#include "stdafx.h"
#include "XMLQuery.h"
namespace TR { namespace XML {
using namespace xml_tools;

namespace {
void bind_query_def(XmlNode& query_node, const XmlDefNode& def_node)
{
    if (!node_name_equal(query_node, def_node))
    {
        throw std::logic_error("Document does not match to definition");
    }

    query_node._private = (void*)&def_node;

    if (def_node.is_trivial())
        return;
    
    stl_tools::associate(get_child_nodes(query_node), get_child_defs(def_node), [](XmlNode* child_node, const XmlDefNode* child_def){
        if (child_node == nullptr || child_def == nullptr)
        {
            throw std::logic_error("Document does not match to definition");
        }
        bind_query_def(*child_node, *child_def);
    });
}

void bind_query_def(XmlDoc& query_doc, const XmlDefDoc& def_doc)
{
    query_doc._private = (void*)&def_doc;

    auto root = get_root(query_doc);
    if (!root)
    {
        throw std::logic_error("Document has no root element");
    }

    auto def_root = def_doc.get_root_def();
    _ASSERT(def_root);

    bind_query_def(*root, *def_root);
}

const XmlQueryNode& resolve_link(const XmlQueryNode& query_node)
{
    auto& def_node = query_node.get_bound_def();
    if (!def_node.is_link())
    {
        return query_node;
    }

    auto link_target_node = find_xpath_node(*static_cast<XmlDoc*>(query_node.doc), def_node.get_xpath().c_str());
    if (!link_target_node)
    {
        throw std::logic_error("Document does not match to definition");
    }

    return static_cast<const XmlQueryNode&>(*link_target_node);
}
}

const XmlDefDoc& XmlQueryDoc::get_bound_def() const
{
    auto def_doc = (const XmlDefDoc*)_private;
    if (!def_doc)
    {
        throw std::logic_error("Query document has no bound definition");
    }
    return *def_doc;
}

XmlQueryNode* XmlQueryDoc::get_root()
{
    return static_cast<XmlQueryNode*>(xml_tools::get_root(*this));
}

const XmlQueryNode* XmlQueryDoc::get_root() const
{
    return const_cast<XmlQueryDoc*>(this)->get_root();
}

boost::optional<std::string> XmlConditionNode::get_name() const
{
    auto name_attr = find_attribute(*this, "name");
    if (!name_attr)
    {
        return boost::none;
    }
    return get_attr_value(*name_attr);
}

void XmlConditionNode::set_name(const std::string& name)
{
    set_attr_value(*this, "name", name.c_str());
}

boost::optional<std::string> XmlConditionNode::get_comment() const
{
    auto comment_attr = find_attribute(*this, "comment");
    if (!comment_attr)
    {
        return boost::none;
    }
    return get_attr_value(*comment_attr);
}

void XmlConditionNode::set_comment(const std::string& comment)
{
    set_attr_value(*this, "comment", comment.c_str());
}

XmlConditionNode::Action XmlConditionNode::get_action() const
{
    auto action_attr = find_attribute(*this, "action");
    if (!action_attr)
    {
        return Action::ACCEPT;
    }
    else if (attr_value_equal(*action_attr, "ACCEPT"))
    {
        return Action::ACCEPT;
    }
    else if (attr_value_equal(*action_attr, "REJECT"))
    {
        return Action::REJECT;
    }   
    else
    {
        _ASSERT(false);
        throw std::logic_error("Invalid value of 'action' attribute of 'condition' node");
    }
}

void XmlConditionNode::set_action(Action action)
{
    switch (action)
    {
    case Action::ACCEPT:
        set_attr_value(*this, "action", "ACCEPT");
        break;

    case Action::REJECT:
        set_attr_value(*this, "action", "REJECT");
        break;  

    default:
        _ASSERT(false);
        throw std::logic_error("Unknown XML condition action");
    }
}

XmlConditionNode::Predicate XmlConditionNode::get_predicate() const
{
    auto predicate_attr = find_attribute(*this, "predicate");
    if (!predicate_attr)
    {
        return Predicate::EQUAL;
    }
    else if (attr_value_equal(*predicate_attr, "EQUAL"))
    {
        return Predicate::EQUAL;
    }
    else if (attr_value_equal(*predicate_attr, "MATCH"))
    {
        return Predicate::MATCH;
    }
    else if (attr_value_equal(*predicate_attr, "LESS"))
    {
        return Predicate::LESS;
    }
    else if (attr_value_equal(*predicate_attr, "MORE"))
    {
        return Predicate::MORE;
    }
    else
    {
        _ASSERT(false);
        throw std::logic_error("Invalid value of 'predicate' attribute of 'condition' node");
    }
}

void XmlConditionNode::set_predicate(Predicate predicate)
{
    switch (predicate)
    {
    case Predicate::EQUAL:
        set_attr_value(*this, "predicate", "EQUAL");
        break;

    case Predicate::MATCH:
        set_attr_value(*this, "predicate", "MATCH");
        break;

    case Predicate::LESS:
        set_attr_value(*this, "predicate", "LESS");
        break;

    case Predicate::MORE:
        set_attr_value(*this, "predicate", "MORE");
        break;

    default:
        _ASSERT(false);
        throw std::logic_error("Unknown XML condition predicate");
    }
}

std::vector<std::wstring> XmlConditionNode::get_patterns() const
{
    std::vector<std::wstring> patterns;
    for (auto& child_node : get_child_nodes(*this))
    {
        _ASSERT(node_name_equal(child_node, "pattern"));
        patterns.push_back(get_node_wtext(child_node));
    }
    return patterns;
}

void XmlConditionNode::set_patterns(const std::vector<std::wstring>& patterns)
{
    remove_children(*this);
    for (auto& pattern : patterns)
    {
        auto& pattern_node = add_element(*this, "pattern");
        add_text_node(pattern_node, pattern.c_str());   
    }
}

std::unique_ptr<XmlConditionNode> create_condition(XmlConditionNode::Action action, XmlConditionNode::Predicate predicate, const std::vector<std::wstring>& patterns)
{
    auto condition_node = stl_tools::static_pointer_cast<XmlConditionNode>(create_element("condition"));
    condition_node->set_action(action);
    condition_node->set_predicate(predicate);
    condition_node->set_patterns(patterns);
    return condition_node;
}

std::unique_ptr<XmlConditionNode> create_condition(const std::string& name, XmlConditionNode::Action action, XmlConditionNode::Predicate predicate, const std::vector<std::wstring>& patterns)
{
    auto condition_node = create_condition(action, predicate, patterns);
    condition_node->set_name(name);
    return condition_node;
}

const XmlDefNode& XmlQueryNode::get_bound_def() const
{
    auto def_node = (const XmlDefNode*)_private;
    if (!def_node)
    {
        throw std::logic_error("Query node has no bound definition");
    }
    return *def_node;
}

bool XmlQueryNode::is_action_specified() const
{
    return find_attribute(*this, "action") != nullptr;
}

void XmlQueryNode::set_action(Action action)
{
    switch (action)
    {
    case Action::ACCEPT:
        set_attr_value(*this, "action", "ACCEPT");
        break;

    case Action::REJECT:
        set_attr_value(*this, "action", "REJECT");
        break;
    
    default:
        _ASSERT(false);
        throw std::logic_error("Unknown XML query action found");
    }   
}

XmlQueryNode::Action XmlQueryNode::get_action() const
{
    auto action_attr = find_attribute(*this, "action");
    if (!action_attr)
    {
        return Action::ACCEPT;
    }
    else if (attr_value_equal(*action_attr, "ACCEPT"))
    {
        return Action::ACCEPT;
    }
    else if (attr_value_equal(*action_attr, "REJECT"))
    {
        return Action::REJECT;
    }
    else 
    {
        _ASSERT(false);
        throw std::logic_error("Invalid value of 'action' attribute of query node");        
    }
}

void XmlQueryNode::reset_action()
{
    unset_attr(*this, "action");
}

XmlQueryNode* XmlCompositeQueryNode::next_query_node(XmlQueryNode* query_node)
{
    _ASSERT(query_node);
    return static_cast<XmlQueryNode*>(next_child(query_node));
}


const XmlQueryNode* XmlCompositeQueryNode::next_query_node(const XmlQueryNode* query_node)
{
    _ASSERT(query_node);
    return next_query_node(const_cast<XmlQueryNode*>(query_node));
}

XmlCompositeQueryNode::QueryNodeIterator XmlCompositeQueryNode::begin_child_node()
{
    return QueryNodeIterator(static_cast<XmlQueryNode*>(begin_child(*this)));
}

XmlCompositeQueryNode::QueryNodeIterator XmlCompositeQueryNode::end_child_node()
{
    return QueryNodeIterator();
}

XmlCompositeQueryNode::ConstQueryNodeIterator XmlCompositeQueryNode::begin_child_node() const
{
    return ConstQueryNodeIterator(static_cast<const XmlQueryNode*>(begin_child(*this)));
}

XmlCompositeQueryNode::ConstQueryNodeIterator XmlCompositeQueryNode::end_child_node() const
{
    return ConstQueryNodeIterator();
}

XmlCompositeQueryNode::QueryNodeIteratorRange XmlCompositeQueryNode::get_child_nodes()
{
    return QueryNodeIteratorRange(begin_child_node(), end_child_node());
}

XmlCompositeQueryNode::ConstQueryNodeIteratorRange XmlCompositeQueryNode::get_child_nodes() const
{
    return ConstQueryNodeIteratorRange(begin_child_node(), end_child_node());
}


XmlConditionNode* XmlConditionalQueryNode::next_condition(XmlConditionNode* condition_node)
{
    _ASSERT(condition_node);
    return static_cast<XmlConditionNode*>(next_child(condition_node));
}

const XmlConditionNode* XmlConditionalQueryNode::next_conditon(const XmlConditionNode* condition_node)
{
    _ASSERT(condition_node);
    return static_cast<const XmlConditionNode*>(next_child(condition_node));
}

bool XmlConditionalQueryNode::empty() const
{
    return begin_condition() == end_condition();
}

void XmlConditionalQueryNode::set_default(Action action, const boost::optional<std::string>& name, const boost::optional<std::string>& comment)
{
    switch (action)
    {
    case Action::ACCEPT:
        set_attr_value(*this, "default_action", "ACCEPT");
        break;

    case Action::REJECT:
        set_attr_value(*this, "default_action", "REJECT");
        break;

    default:
        _ASSERT(false);
        throw std::logic_error("Unknown XML query action found");
    }

    if (name)
    {
        set_attr_value(*this, "default_name", name->c_str());
    }
    else
    {
        unset_attr(*this, "default_name");
    }

    if (comment)
    {
        set_attr_value(*this, "default_comment", comment->c_str());
    }
    else
    {
        unset_attr(*this, "default_comment");
    }
}

boost::optional<XmlConditionalQueryNode::Action> XmlConditionalQueryNode::get_default_action() const
{
    auto action_attr = find_attribute(*this, "default_action");
    if (!action_attr)
    {
        return boost::none;
    }
    else if (attr_value_equal(*action_attr, "ACCEPT"))
    {
        return Action::ACCEPT;
    }
    else if (attr_value_equal(*action_attr, "REJECT"))
    {
        return Action::REJECT;
    }
    else
    {
        _ASSERT(false);
        throw std::logic_error("Invalid value of 'default_action' attribute of query node");
    }
}

boost::optional<std::string> XmlConditionalQueryNode::get_default_name() const
{   
    if (auto default_name_attr = find_attribute(*this, "default_name"))
    {
        return get_attr_value(*default_name_attr);
    }   
    return boost::none;
}

boost::optional<std::string> XmlConditionalQueryNode::get_default_comment() const
{   
    if (auto default_comment_attr = find_attribute(*this, "default_comment"))
    {
        return get_attr_value(*default_comment_attr);
    }
    return boost::none;
}

XmlConditionalQueryNode::ConditionIterator XmlConditionalQueryNode::begin_condition()
{
    return ConditionIterator(static_cast<XmlConditionNode*>(begin_child(*this)));
}

XmlConditionalQueryNode::ConditionIterator XmlConditionalQueryNode::end_condition()
{
    return ConditionIterator();
}

XmlConditionalQueryNode::ConstConditionIterator XmlConditionalQueryNode::begin_condition() const
{
    return ConstConditionIterator(static_cast<const XmlConditionNode*>(begin_child(*this)));
}

XmlConditionalQueryNode::ConstConditionIterator XmlConditionalQueryNode::end_condition() const
{
    return ConstConditionIterator();
}

XmlConditionalQueryNode::ConditionIteratorRange XmlConditionalQueryNode::get_conditions()
{
    return ConditionIteratorRange(begin_condition(), end_condition());
}

XmlConditionalQueryNode::ConstConditionIteratorRange XmlConditionalQueryNode::get_conditions() const
{
    return ConstConditionIteratorRange(begin_condition(), end_condition());
}


XmlConditionalQueryNode::ConditionIterator  XmlConditionalQueryNode::add_condition(std::unique_ptr<XmlConditionNode> condition_node)
{
    return ConditionIterator(&add_node(*this, std::move(condition_node)));
}

XmlConditionalQueryNode::ConditionIterator XmlConditionalQueryNode::push_condition_front(std::unique_ptr<XmlConditionNode> condition_node)
{
    return ConditionIterator(&push_node_front(*this, std::move(condition_node)));
}

XmlConditionalQueryNode::ConditionIterator  XmlConditionalQueryNode::remove_condition(ConditionIterator condition_iterator)
{
    _ASSERT(condition_iterator != end_condition());
    auto next = std::next(condition_iterator);
    remove_node(*condition_iterator);   
    return next;
}

std::unique_ptr<XmlQueryDoc> parse_query(const std::string& xml_query, const XmlDefDoc& def_doc)
{
    auto query_doc = xml_tools::parse(xml_query);
    bind_query_def(*query_doc, def_doc);
    return stl_tools::static_pointer_cast<XmlQueryDoc>(std::move(query_doc));
}

XmlQueryDoc& bind_query_definition(XmlDoc& doc, const XmlDefDoc& def_doc)
{
    bind_query_def(doc, def_doc);
    return static_cast<XmlQueryDoc&>(doc);
}

static std::unique_ptr<XmlQueryNode> create_query_node(const XmlDefNode& def_node)
{
    auto query_node = stl_tools::static_pointer_cast<XmlQueryNode>(clone_node(def_node, XmlCloneOpt::NONE));
    query_node->_private = (void*)&def_node;
    for (auto& child_def : get_child_defs(def_node))
    {
        add_node(*query_node, create_query_node(child_def));
    }
    return query_node;
}

std::unique_ptr<XmlQueryDoc> create_query(const XmlDefDoc& def_doc)
{
    auto query_doc = stl_tools::static_pointer_cast<XmlQueryDoc>(create_doc());
    set_root(*query_doc, create_query_node(*def_doc.get_root_def()));
    query_doc->_private = (void*)&def_doc;
    return query_doc;
}

boost::optional<XmlQueryNode::Action> get_redifined_action(const XmlQueryNode& parent_node, const XmlQueryNode& child_node)
{
    if (child_node.is_action_specified() == false && parent_node.is_action_specified())
    {
        return parent_node.get_action();
    }
    return boost::none;
}

std::unique_ptr<XmlQueryNode> join_nodes(const XmlQueryNode& parent_node, const XmlQueryNode& child_node);
void join_composed_nodes(const XmlCompositeQueryNode& parent_node, const XmlCompositeQueryNode& child_node, XmlCompositeQueryNode& joined_node)
{
    stl_tools::associate(parent_node.get_child_nodes(), child_node.get_child_nodes(), [&](const XmlQueryNode* left_node, const XmlQueryNode* right_node){
        if (left_node == nullptr || right_node == nullptr)
        {
            throw std::logic_error("Joined queries does not match");
        }
        add_node(joined_node, join_nodes(*left_node, *right_node));
    });
}

void join_conditions(const XmlConditionalQueryNode& parent_node, const XmlConditionalQueryNode& child_node, XmlConditionalQueryNode& joined_node)
{
    for (auto& condition : child_node.get_conditions())
    {
        add_node(joined_node, clone_node(condition));
    }

    for (auto& condition : parent_node.get_conditions())
    {
        add_node(joined_node, clone_node(condition));
    }
}

struct JoinQueryNodes: boost::static_visitor<void>
{
    JoinQueryNodes(XmlQueryNode& joined_node):
        m_joined_node(joined_node)
    {
    }

    void operator()(const XmlCompositeQueryNode& parent_node, const XmlCompositeQueryNode& child_node)
    {
        return join_composed_nodes(parent_node, child_node, static_cast<XmlCompositeQueryNode&>(m_joined_node));
    }

    void operator()(const XmlConditionalQueryNode& parent_node, const XmlConditionalQueryNode& child_node)
    {
        auto& joined_node = static_cast<XmlConditionalQueryNode&>(m_joined_node);
        if (child_node.get_default_action() != parent_node.get_default_action())
        {
            throw std::logic_error("Default action does not match");
        }
        return join_conditions(parent_node, child_node, joined_node);
    }

    template<class T1, class T2> 
    void operator()(const T1& parent_node, const T2& child_node)
    {
        _ASSERT(false);
    }

    XmlQueryNode& m_joined_node;
};

std::unique_ptr<XmlQueryNode> join_nodes(const XmlQueryNode& parent_node, const XmlQueryNode& child_node)
{
    auto joined_node = stl_tools::static_pointer_cast<XmlQueryNode>(clone_node(child_node, stl_tools::flag | XmlCloneOpt::ATTRIBUTE | XmlCloneOpt::PRIVATE));
    if (joined_node->is_action_specified() == false && parent_node.is_action_specified())
    {
        joined_node->set_action(parent_node.get_action());
    }

    visit_query_nodes(JoinQueryNodes(*joined_node), parent_node, child_node);
    return joined_node;
}

std::unique_ptr<XmlQueryDoc> join_queries(const XmlQueryDoc& parent_query, const XmlQueryDoc& child_query)
{
    auto parent_root = parent_query.get_root();
    if (!parent_root)
    {
        throw std::logic_error("Parent query has no root element");
    }

    auto child_root = child_query.get_root();
    if (!child_root)
    {
        throw std::logic_error("Child query has no root element");
    }

    auto joined_query = stl_tools::static_pointer_cast<XmlQueryDoc>(create_doc());
    set_root(*joined_query, join_nodes(*parent_root, *child_root));
    return joined_query;
}

std::unique_ptr<XmlRegularNode> do_query(const XmlQueryNode& query_node, const XmlRegularNode& reg_node);

std::unique_ptr<XmlGenericListNode> do_query(const XmlCompositeQueryNode& query_node, const XmlGenericListNode& list_node)
{
    if (query_node.get_action() == XmlQueryNode::Action::REJECT)
    {
        return nullptr;
    }

    auto child_query_nodes = query_node.get_child_nodes();
    auto child_query_nodes_count = boost::distance(child_query_nodes);
    if (child_query_nodes_count > 1)
    {
        throw std::logic_error("Invalid query node found");
    }

    if (child_query_nodes_count == 0 && list_node.empty() == false)
    {
        throw std::logic_error("Query does not match regular document");
    }

    auto queried_list_node = stl_tools::static_pointer_cast<XmlGenericListNode>(clone_node(list_node, stl_tools::flag | XmlCloneOpt::ATTRIBUTE | XmlCloneOpt::PRIVATE));
    for (auto& list_item : list_node.get_items())
    {
        if (auto queried_item_node = do_query(*child_query_nodes.begin(), list_item))
        {
            add_node(*queried_list_node, std::move(queried_item_node));
        }
    }

    return queried_list_node;
}

std::unique_ptr<XmlStructNode> do_query(const XmlCompositeQueryNode& query_node, const XmlStructNode& struct_node)
{
    if (query_node.get_action() == XmlQueryNode::Action::REJECT)
    {
        return nullptr;
    }

    auto queried_struct_node = stl_tools::static_pointer_cast<XmlStructNode>(clone_node(struct_node, stl_tools::flag | XmlCloneOpt::ATTRIBUTE | XmlCloneOpt::PRIVATE));

    stl_tools::associate(struct_node.get_members(), query_node.get_child_nodes(), [&](const XmlRegularNode* child_reg_node, const XmlQueryNode* child_query_node){
        if (child_reg_node != nullptr && child_query_node == nullptr)
        {
            throw std::logic_error("Query does not match regular document");
        }

        if (child_reg_node == nullptr || node_name_equal(child_reg_node->get_bound_def(), child_query_node->get_bound_def()) == false)
        {
            if (child_query_node->get_bound_def().is_optional())
            {
                return stl_tools::loop_stmt::CONTINUE_RIGHT;                
            }
            throw std::logic_error("Query does not match regular document");            
        }

        if (child_query_node->get_action() == XmlQueryNode::Action::REJECT)
        {
            return stl_tools::loop_stmt::CONTINUE;
        }

        if (auto child_queried_node = do_query(*child_query_node, *child_reg_node))
        {
            add_node(*queried_struct_node, std::move(child_queried_node));
            return stl_tools::loop_stmt::CONTINUE;
        }
        else
        {
            queried_struct_node.reset();
            return stl_tools::loop_stmt::BREAK;
        }

        return stl_tools::loop_stmt::BREAK; // to supress warning
    });
    return queried_struct_node;
}

namespace {

template<class T>
bool check_equal(const std::vector<std::wstring>& patterns, const T& value)
{
    for (auto& pattern : patterns)
    {
        if (boost::lexical_cast<T>(pattern) == value)
        {
            return true;
        }       
    }
    return false;
}

template<>
bool check_equal<std::wstring>(const std::vector<std::wstring>& patterns, const std::wstring& value)
{
    return boost::find(patterns, value) != patterns.end();
}

template<class T>
bool check_match(const std::vector<std::wstring>& patterns, const T& value)
{
    for (auto& pattern : patterns)
    {
        if (boost::regex_match(boost::lexical_cast<std::wstring>(value), boost::wregex(pattern)))
        {
            return true;
        }   
    }
    return false;
}

template<class T>
bool check_less(const std::vector<std::wstring>& patterns, const T& value)
{
    for (auto& pattern : patterns)
    {
        if (boost::lexical_cast<T>(pattern) > value)
        {
            return true;
        }
    }
    return false;
}

template<class T>
bool check_more(const std::vector<std::wstring>& patterns, const T& value)
{
    for (auto& pattern : patterns)
    {
        if (boost::lexical_cast<T>(pattern) < value)
        {
            return true;
        }
    }
    return false;
}

template<class T>
boost::optional<XmlConditionNode::Action> check_condition(const XmlConditionNode& condition_node, const XmlTrivialNode<T>& trivial_node)
{
    auto patterns = condition_node.get_patterns();
    _ASSERT(!patterns.empty()); //condition without patterns? it is ok but starange.

    auto value = trivial_node.get_value();

    switch (condition_node.get_predicate())
    {
    case XmlConditionNode::Predicate::MATCH:
        if (check_match(patterns, value))
        {
            return condition_node.get_action();
        }
        break;      

    case XmlConditionNode::Predicate::EQUAL:
        if (check_equal(patterns, value))
        {
            return condition_node.get_action();
        }
        break;

    case XmlConditionNode::Predicate::LESS:
        if (check_less(patterns, value))
        {
            return condition_node.get_action();
        }
        break;

    case XmlConditionNode::Predicate::MORE:
        if (check_more(patterns, value))
        {
            return condition_node.get_action();
        }
        break;

    default:
        _ASSERT(false);
    }

    return boost::none;
}

XmlQueryNode::Action get_actual_default_action(const XmlConditionalQueryNode& query_node)
{
    if (auto default_action = query_node.get_default_action())
    {
        return *default_action;
    }

    if (query_node.empty())
    {
        return XmlQueryNode::Action::ACCEPT;
    }

    auto last_condition = query_node.begin_condition();
    while (std::next(last_condition) != query_node.end_condition())
    {
        ++last_condition;
    }
    
    switch (last_condition->get_action())
    {
    case XmlConditionNode::Action::ACCEPT:
        return XmlQueryNode::Action::REJECT;

    case XmlConditionNode::Action::REJECT:
        return XmlQueryNode::Action::ACCEPT;

    default:
        _ASSERT(false);
    }

    return XmlQueryNode::Action::ACCEPT; // to supress warning
}

} //namespace {

template <class T>
std::unique_ptr<XmlTrivialNode<T>> do_query(const XmlConditionalQueryNode& query_node, const XmlTrivialNode<T>& trivial_node)
{
    if (query_node.get_action() == XmlQueryNode::Action::REJECT)
    {
        return nullptr;
    }

    if (query_node.empty())
    {
        return stl_tools::static_pointer_cast<XmlTrivialNode<T>>(clone_node(trivial_node));
    }

    for (auto& condition : query_node.get_conditions())
    {
        if (auto action = check_condition(condition, trivial_node))
        {
            if (*action == XmlConditionNode::Action::ACCEPT)
            {
                auto clone = clone_node(trivial_node);
                if (auto condition_name = condition.get_name())
                {
                    set_attr_value(*clone, "condition", condition_name->c_str());
                }

                if (auto comment = condition.get_comment())
                {
                    set_attr_value(*clone, "comment", comment->c_str());
                }

                return clone;
            }

            _ASSERT(*action == XmlConditionNode::Action::REJECT);
            return nullptr;
        }       
    }

    if (get_actual_default_action(query_node) == XmlQueryNode::Action::ACCEPT)
    {
        auto clone = clone_node(trivial_node);
        if (auto default_name = query_node.get_default_name())
        {
            set_attr_value(*clone, "condition", default_name->c_str());
        }

        if (auto default_comment = query_node.get_default_comment())
        {
            set_attr_value(*clone, "comment", default_comment->c_str());
        }
        
        return clone;
    }

    return nullptr;
}

struct DoQuery: boost::static_visitor<std::unique_ptr<XmlRegularNode>>
{
    DoQuery(const XmlQueryNode& query_node):
        m_query_node(query_node)
    {
    }

    std::unique_ptr<XmlRegularNode> operator()(const XmlGenericListNode& list_node)
    {
        return do_query(static_cast<const XmlCompositeQueryNode&>(m_query_node), list_node);
    }

    std::unique_ptr<XmlRegularNode> operator()(const XmlStructNode& struct_node)
    {
        return do_query(static_cast<const XmlCompositeQueryNode&>(m_query_node), struct_node);
    }

    template<class T>
    std::unique_ptr<XmlRegularNode> operator()(const XmlTrivialNode<T>& trivial_node)
    {
        return do_query(static_cast<const XmlConditionalQueryNode&>(m_query_node), trivial_node);
    }

    const XmlQueryNode& m_query_node;
};

std::unique_ptr<XmlRegularNode> do_query(const XmlQueryNode& query_node, const XmlRegularNode& reg_node)
{
    if (!node_name_equal(query_node, reg_node))
    {
        throw std::logic_error("Query does not match regular document");
    }
    return visit_regular_node(DoQuery(resolve_link(query_node)), reg_node);
}

std::unique_ptr<XmlRegularDoc> do_query(const XmlQueryDoc& query_doc, const XmlRegularDoc& reg_doc)
{
    auto reg_root = reg_doc.get_root();
    if (!reg_root)
    {
        throw std::logic_error("Regular document has no root element");
    }

    auto query_root = query_doc.get_root();
    if (!query_root)
    {
        throw std::logic_error("Query has no root element");
    }

    std::unique_ptr<XmlRegularDoc> queried_doc;
    if (auto queried_root = do_query(*query_root, *reg_root))
    {
        queried_doc = stl_tools::static_pointer_cast<XmlRegularDoc>(create_doc());
        queried_doc->_private = (void*)&reg_doc.get_bound_def();
        set_root(*queried_doc, std::move(queried_root));
    }       
    return queried_doc;
}

std::unique_ptr<XmlDefNode> get_output_node_def(const XmlQueryNode& query_node)
{
    if (query_node.get_action() == XmlQueryNode::Action::REJECT)
    {
        return nullptr;
    }

    auto output_node_def = stl_tools::static_pointer_cast<XmlDefNode>(clone_node(query_node.get_bound_def(), XmlCloneOpt::ATTRIBUTE));
    if (auto composed_query_node = cast_query_node<XmlCompositeQueryNode>(&query_node))
    {
        for (auto& child_query_node : composed_query_node->get_child_nodes())
        {
            if (auto child_output_def_node = get_output_node_def(child_query_node))
            {
                add_node(*output_node_def, std::move(child_output_def_node));
            }
        }
    }

    XmlNode* meta = nullptr;
    for (auto& meta_node : query_node.get_bound_def().get_meta_nodes())
    {
        if (!meta)
        {
            meta = &add_element(*output_node_def, "META");
        }
        add_node(*meta, clone_node(meta_node));
    }

    return output_node_def;
}

std::unique_ptr<XmlDefDoc> get_output_def(const XmlQueryDoc& query_doc)
{
    auto output_def = stl_tools::static_pointer_cast<XmlDefDoc>(create_doc());
    auto query_root = query_doc.get_root();
    if (!query_root)
    {
        throw std::logic_error("Invalid query doc");
    }
    set_root(*output_def, get_output_node_def(*query_root));
    return output_def;
}

VariantQueryNodeRef make_variant(XmlQueryNode& query_node)
{
    if (query_node.get_bound_def().is_trivial())
    {
        return static_cast<XmlConditionalQueryNode&>(query_node);
    }
    return static_cast<XmlCompositeQueryNode&>(query_node);
}

ConstVariantQueryNodeRef make_variant(const XmlQueryNode& query_node)
{
    return make_variant(const_cast<XmlQueryNode&>(query_node));
}

}} //namespace TR { namespace XML {