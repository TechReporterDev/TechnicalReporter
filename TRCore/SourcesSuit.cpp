#include "stdafx.h"
#include "ReportTypesSuit.h"
#include "CoreTestAssist.h"
namespace {

void add_source_test()
{
    CLEAN_AND_INSTALL();

    int add_counter = 0;
    sources.connect_add_source([&](SourceRef ref, Transaction& t) {
        ++add_counter;
    }, 0);

    auto& source_type = source_types.add_source_type(std::make_unique<SourceTypeMockup>());
    Source source(L"source_1", root.get_ref(), default_role, source_type, source_type.get_default_config());
    TEST_EXCEPTION(source.get_ref());
    
    TEST_ASSERT(boost::distance(sources.get_sources()) == 1); // only root

    auto source_ref = sources.add_source(source);
    
    TEST_ASSERT(boost::distance(sources.get_sources()) == 2);   
    TEST_ASSERT(add_counter == 1);
    TEST_ASSERT(sources.has_source(source_ref.get_key()));
        
    auto loaded_source = sources.get_source(source_ref.get_key());
    
    TEST_ASSERT(loaded_source.get_ref() == source_ref);
    TEST_ASSERT(loaded_source.get_name() == L"source_1");
    TEST_ASSERT(loaded_source.has_parent());
    TEST_ASSERT(loaded_source.get_parent() == root);
    TEST_ASSERT(loaded_source.get_role() == default_role);
    TEST_ASSERT(loaded_source.get_source_type() == source_type);
    TEST_ASSERT(xml_tools::as_string(*loaded_source.get_config()) == xml_tools::as_string(*source_type.get_default_config()));
}

void update_source_test()
{
    CLEAN_AND_INSTALL();

    auto source = *sources.add_source({L"source", root.get_ref(), default_role, group_source_type, group_source_type.get_default_config()});
    auto group = sources.add_source({L"group", root.get_ref(), default_role, group_source_type, group_source_type.get_default_config()});
    auto role = roles.add_role({L"role", default_role});
    auto& source_type = source_types.add_source_type(std::make_unique<SourceTypeMockup>());
    
    int update_counter = 0;
    sources.connect_update_source([&](SourceRef ref, Transaction& t) {      
        ++update_counter;
    }, 0);

    int replace_counter = 0;
    sources.connect_replace_source([&](SourceRef ref, SourceRef prev_parent, SourceRef next_parent, Transaction& t)
    {
        _ASSERT(prev_parent == root && next_parent == group);
        ++replace_counter;
    }, 0);

    int rerole_counter = 0;
    sources.connect_rerole_source([&](SourceRef ref, RoleRef prev_role, RoleRef next_role, Transaction& t)
    {
        _ASSERT(prev_role == default_role && next_role == role);
        ++rerole_counter;
    }, 0);

    source.set_name(L"updated_source");
    source.set_parent(group);
    source.set_role(role);
    source.set_source_type(source_type);
    source.set_config(source_type.get_default_config());

    sources.update_source(source);
    auto updated_source = sources.get_source(source.get_ref().get_key());

    TEST_ASSERT(boost::distance(sources.get_sources()) == 3); //source, root and group
    TEST_ASSERT(update_counter == 1);
    TEST_ASSERT(replace_counter == 1);
    TEST_ASSERT(rerole_counter == 1);
    TEST_ASSERT(updated_source.get_ref() == source.get_ref());
    TEST_ASSERT(updated_source.get_name() == L"updated_source");
    TEST_ASSERT(updated_source.get_parent() == group);
    TEST_ASSERT(updated_source.get_role() == role);
    TEST_ASSERT(updated_source.get_source_type() == source_type);
    TEST_ASSERT(xml_tools::as_string(*updated_source.get_config()) == xml_tools::as_string(*source_type.get_default_config()));
}

void remove_source_test()
{
    CLEAN_AND_INSTALL();

    auto source_ref = sources.add_source({L"source", root.get_ref(), default_role, group_source_type, group_source_type.get_default_config()});
    auto child_ref = sources.add_source({L"source", source_ref, default_role, group_source_type, group_source_type.get_default_config()});
    auto subchild_ref = sources.add_source({L"source", child_ref, default_role, group_source_type, group_source_type.get_default_config()});
    
    int remove_counter = 0;
    sources.connect_remove_source([&](SourceKey source_key, const std::vector<SourceKey> removed_sources, Transaction& t) {
        TEST_ASSERT(source_ref.get_key() == source_key);
        TEST_ASSERT(removed_sources.size() == 3);
        for (auto removed_key : removed_sources)
        {
            TEST_ASSERT(removed_key == source_ref.get_key() || removed_key == child_ref.get_key() || removed_key == subchild_ref.get_key());
        }
        ++remove_counter;
    }, 0);

    TEST_ASSERT(boost::distance(sources.get_sources()) == 4);

    sources.remove_source(source_ref.get_key());

    TEST_ASSERT(boost::distance(sources.get_sources()) == 1);
    TEST_ASSERT(remove_counter == 1);
}

void sources_tree_test()
{
    CLEAN_AND_INSTALL();

    auto& source_type = source_types.add_source_type(std::make_unique<SourceTypeMockup>());

    TEST_ASSERT(boost::distance(sources.get_sources(default_role)) == 1); // only root

    auto role_1 = roles.add_role({L"role_1", default_role});
    TEST_ASSERT(boost::distance(sources.get_sources(role_1)) == 0);

    TEST_ASSERT(!root.has_parent());
    TEST_EXCEPTION(root.get_parent());
    TEST_ASSERT(boost::distance(sources.get_sources()) == 1); // only root
    TEST_ASSERT(boost::distance(sources.get_sources(default_role)) == 1); // only root

    auto source_1 = *sources.add_source({L"source_1", root.get_ref(), default_role, source_type, source_type.get_default_config()});
    
    TEST_ASSERT(boost::distance(sources.get_sources()) == 2); // root and source_1
    TEST_ASSERT(boost::distance(sources.get_sources(root)) == 1); // source_1 only
    TEST_ASSERT(boost::distance(sources.get_sources(default_role)) == 2); // root and source_1  
    TEST_ASSERT(boost::distance(sources.get_sources(role_1)) == 0);

    auto source_2 = *sources.add_source({L"source_2", root.get_ref(), role_1, source_type, source_type.get_default_config()});
    
    TEST_ASSERT(boost::distance(sources.get_sources()) == 3); // root, source_1 and source_2
    TEST_ASSERT(boost::distance(sources.get_sources(root)) == 2); // source_1 and source_2
    TEST_ASSERT(boost::distance(sources.get_sources(default_role)) == 2); // root and source_1
    TEST_ASSERT(boost::distance(sources.get_sources(role_1)) == 1); // only source_2

    Source child(L"child", source_1.get_ref(), role_1, source_type, source_type.get_default_config());
    child = *sources.add_source(child);
    
    TEST_ASSERT(boost::distance(sources.get_sources()) == 4); // root, source_1, source_2 and child
    TEST_ASSERT(boost::distance(sources.get_sources(root)) == 2); // source_1 and source_2
    TEST_ASSERT(boost::distance(sources.get_sources(source_1)) == 1); // only child
    TEST_ASSERT(boost::distance(sources.get_sources(source_2)) == 0);
    TEST_ASSERT(boost::distance(sources.get_sources(default_role)) == 2); // root and source_1
    TEST_ASSERT(boost::distance(sources.get_sources(role_1)) == 2); // source_2 and child

    child.set_parent(source_2);
    sources.update_source(child);
    
    TEST_ASSERT(boost::distance(sources.get_sources()) == 4); // root, source_1, source_2 and child
    TEST_ASSERT(boost::distance(sources.get_sources(root)) == 2); // source_1 and source_2
    TEST_ASSERT(boost::distance(sources.get_sources(source_1)) == 0);
    TEST_ASSERT(boost::distance(sources.get_sources(source_2)) == 1); // only child
    TEST_ASSERT(boost::distance(sources.get_sources(default_role)) == 2); // root and source_1
    TEST_ASSERT(boost::distance(sources.get_sources(role_1)) == 2); // source_2 and child
    
    sources.remove_source(source_2.get_ref().get_key());
    
    TEST_ASSERT(boost::distance(sources.get_sources()) == 2); // root and source_1
    TEST_ASSERT(boost::distance(sources.get_sources(root)) == 1); // source_1
    TEST_ASSERT(boost::distance(sources.get_sources(default_role)) == 2); // root and source_1
    TEST_ASSERT(boost::distance(sources.get_sources(role_1)) == 0);
}

void cyclic_dependency_test()
{
    CLEAN_AND_INSTALL();

    auto& source_type = source_types.add_source_type(std::make_unique<SourceTypeMockup>());

    auto source_1 = *sources.add_source({L"source_1", root.get_ref(), default_role, source_type, source_type.get_default_config()});
    source_1.set_parent(source_1);
    TEST_EXCEPTION(sources.update_source(source_1));

    auto source_2 = *sources.add_source({L"source_2", source_1.get_ref(), default_role, source_type, source_type.get_default_config()});
    source_1.set_parent(source_2);
    TEST_EXCEPTION(sources.update_source(source_1));    
}

void break_remove_source_type_test()
{
    CLEAN_AND_INSTALL();

    auto& source_type = source_types.add_source_type(std::make_unique<SourceTypeMockup>());
    auto source_ref = sources.add_source({L"source_1", root.get_ref(), default_role, source_type, source_type.get_default_config()});
    TEST_EXCEPTION(source_types.remove_source_type(fake_source_type_uuid));

    sources.remove_source(source_ref.get_key());
    TEST_NO_EXCEPTION(source_types.remove_source_type(fake_source_type_uuid))
}

void break_remove_role_test()
{
    CLEAN_AND_INSTALL();

    auto role_ref = roles.add_role({L"role_1", roles.get_default_role()});
    auto source_ref = sources.add_source({L"source_1", root.get_ref(), role_ref, group_source_type, group_source_type.get_default_config()});
    TEST_EXCEPTION(roles.remove_role(role_ref.get_key()));

    sources.remove_source(source_ref.get_key());
    TEST_NO_EXCEPTION(roles.remove_role(role_ref.get_key()));
}
} //namespace {

test_assist::test_suit get_sources_suit()
{
    test_assist::test_suit ts(L"sources_suit");
    ts
        .add({ L"add_source_test", &add_source_test })
        .add({ L"update_source_test", &update_source_test })
        .add({ L"remove_source_test", &remove_source_test })
        .add({ L"sources_tree_test", &sources_tree_test })
        .add({ L"cyclic_dependency_test", &cyclic_dependency_test})
        .add({ L"break_remove_source_type_test", &break_remove_source_type_test })
        .add({ L"break_remove_role_test", &break_remove_role_test });

    return ts;
}