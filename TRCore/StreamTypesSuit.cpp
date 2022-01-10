#include "stdafx.h"
#include "StreamTypesSuit.h"
#include "CoreTestAssist.h"
namespace {

void add_stream_type_test()
{
    CLEAN_AND_INSTALL();

    auto stream_type_uuid = StreamTypeUUID(stl_tools::gen_uuid());      
    auto stream_type = std::make_unique<StreamType>(stream_type_uuid, L"stream_type", nullptr);
    TEST_EXCEPTION(stream_type->get_ref());

    int add_counter = 0;
    stream_types.connect_add_stream_type([&](StreamTypeRef ref, Transaction& t){
        TEST_ASSERT(ref.get_uuid() == stream_type_uuid);
        ++add_counter;
    }, 0);

    TEST_ASSERT(! stream_types.has_stream_type(stream_type_uuid));
    TEST_EXCEPTION(stream_types.get_stream_type(stream_type_uuid));
    TEST_ASSERT(boost::distance(stream_types.get_stream_types()) == 0);

    Transaction t1(db);
    auto& stream_type_ref = stream_types.add_stream_type(std::move(stream_type), t1);
    TEST_NO_EXCEPTION(stream_type_ref.get_ref());
    t1.commit();
    

    TEST_ASSERT(add_counter == 1);
    TEST_ASSERT(stream_types.has_stream_type(stream_type_uuid));
    TEST_NO_EXCEPTION(stream_types.get_stream_type(stream_type_uuid));
    TEST_ASSERT(boost::distance(stream_types.get_stream_types()) == 1);
}

void rollback_add_stream_type_test()
{
    CLEAN_AND_INSTALL();
    auto stream_type_uuid = StreamTypeUUID(stl_tools::gen_uuid());

    Transaction t1(db);
    stream_types.add_stream_type(std::make_unique<StreamType>(stream_type_uuid, L"stream_type", nullptr), t1);
    TEST_ASSERT(stream_types.has_stream_type(stream_type_uuid));
    
    t1.rollback();
    TEST_ASSERT(! stream_types.has_stream_type(stream_type_uuid));
}

void duplicate_stream_type_test()
{
    CLEAN_AND_INSTALL();

    auto stream_type_uuid = StreamTypeUUID(stl_tools::gen_uuid());
    
    Transaction t1(db);
    stream_types.add_stream_type(std::make_unique<StreamType>(stream_type_uuid, L"stream_type", nullptr), t1);
    t1.commit();

    TEST_ASSERT(stream_types.has_stream_type(stream_type_uuid));

    Transaction t2(db);
    TEST_EXCEPTION(stream_types.add_stream_type(std::make_unique<StreamType>(stream_type_uuid, L"stream_type", nullptr), t2));
    t2.rollback();
}

void remove_stream_type_test()
{
    CLEAN_AND_INSTALL();

    auto stream_type_uuid = StreamTypeUUID(stl_tools::gen_uuid());
    auto stream_type = std::make_unique<StreamType>(stream_type_uuid, L"stream_type", nullptr);

    Transaction t1(db);
    auto& stream_type_ref = stream_types.add_stream_type(std::move(stream_type), t1);
    t1.commit();

    TEST_ASSERT(stream_types.has_stream_type(stream_type_uuid));

    int remove_counter = 0;
    stream_types.connect_remove_stream_type([&](UUID removed_uuid, Transaction& t){
        TEST_ASSERT(stream_type_uuid == removed_uuid);
        ++remove_counter;
    }, 0);

    Transaction t2(db);
    stream_types.remove_stream_type(stream_type_uuid, t2);
    t2.commit();

    TEST_ASSERT(remove_counter == 1);
    TEST_ASSERT(! stream_types.has_stream_type(stream_type_uuid));
}

void rollback_remove_stream_type_test()
{
    CLEAN_AND_INSTALL();

    auto stream_type_uuid = StreamTypeUUID(stl_tools::gen_uuid());
    auto stream_type = std::make_unique<StreamType>(stream_type_uuid, L"stream_type", nullptr);

    Transaction t1(db);
    auto& stream_type_ref = stream_types.add_stream_type(std::move(stream_type), t1);
    t1.commit();

    TEST_ASSERT(stream_types.has_stream_type(stream_type_uuid));

    Transaction t2(db);
    stream_types.remove_stream_type(stream_type_uuid, t2);
    TEST_ASSERT(! stream_types.has_stream_type(stream_type_uuid));
    t2.rollback();

    TEST_ASSERT(stream_types.has_stream_type(stream_type_uuid));
}

void collect_stream_types_test()
{
    CLEAN_AND_INSTALL();

    auto stream_type_uuid_1 = StreamTypeUUID(stl_tools::gen_uuid());
    auto stream_type_uuid_2 = StreamTypeUUID(stl_tools::gen_uuid());

    TEST_ASSERT(boost::distance(stream_types.get_stream_types()) == 0);
    TEST_ASSERT(! stream_types.has_stream_type(stream_type_uuid_1));
    TEST_ASSERT(! stream_types.has_stream_type(stream_type_uuid_2));
    
    Transaction t1(db);
    stream_types.add_stream_type(std::make_unique<StreamType>(stream_type_uuid_1, L"stream_type_1", nullptr), t1);
    t1.commit();

    TEST_ASSERT(boost::distance(stream_types.get_stream_types()) == 1);
    TEST_ASSERT(stream_types.has_stream_type(stream_type_uuid_1));
    TEST_ASSERT(! stream_types.has_stream_type(stream_type_uuid_2));
    
    Transaction t2(db);
    stream_types.add_stream_type(std::make_unique<StreamType>(stream_type_uuid_2, L"stream_type_2", nullptr), t2);
    t2.commit();

    TEST_ASSERT(boost::distance(stream_types.get_stream_types()) == 2);
    TEST_ASSERT(stream_types.has_stream_type(stream_type_uuid_1));
    TEST_ASSERT(stream_types.has_stream_type(stream_type_uuid_2));

    Transaction t3(db);
    stream_types.remove_stream_type(stream_type_uuid_1, t3);
    t3.commit();

    TEST_ASSERT(boost::distance(stream_types.get_stream_types()) == 1);
    TEST_ASSERT(! stream_types.has_stream_type(stream_type_uuid_1));
    TEST_ASSERT(stream_types.has_stream_type(stream_type_uuid_2));
    
    Transaction t4(db);
    stream_types.remove_stream_type(stream_type_uuid_2, t4);
    t4.commit();

    TEST_ASSERT(boost::distance(stream_types.get_stream_types()) == 0);
    TEST_ASSERT(! stream_types.has_stream_type(stream_type_uuid_1));
    TEST_ASSERT(! stream_types.has_stream_type(stream_type_uuid_2));
}

} //namespace {

test_assist::test_suit get_stream_types_suit()
{
    test_assist::test_suit ts(L"stream_types_suit");
    ts
        .add({L"add_stream_type_test", &add_stream_type_test})
        .add({L"rollback_add_stream_type_test", &rollback_add_stream_type_test})
        .add({L"duplicate_stream_type_test", &duplicate_stream_type_test})
        .add({L"remove_stream_type_test", &remove_stream_type_test})
        .add({L"rollback_remove_stream_type_test", &rollback_remove_stream_type_test})
        .add({L"collect_stream_types_test", &collect_stream_types_test});

    return ts;
}