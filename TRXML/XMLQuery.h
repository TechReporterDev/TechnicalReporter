#pragma once
#include "XMLRegular.h"
#include "XMLDefinition.h"
#include "TRXMLStd.h"
namespace TR { namespace XML {
struct XmlQueryNode;

struct XmlQueryDoc: XmlDoc
{
    const XmlDefDoc&        get_bound_def() const;
    XmlQueryNode*           get_root();
    const XmlQueryNode*     get_root() const;
};

struct XmlConditionNode: XmlNode
{   
    enum class Action { ACCEPT = 0, REJECT };
    enum class Predicate { EQUAL = 0, MATCH, LESS, MORE };
    
    boost::optional<std::string>    get_name() const;
    void                            set_name(const std::string& name);
    boost::optional<std::string>    get_comment() const;
    void                            set_comment(const std::string& name);

    Action                          get_action() const;
    void                            set_action(Action action);
    Predicate                       get_predicate() const;
    void                            set_predicate(Predicate predicate);
    std::vector<std::wstring>       get_patterns() const;
    void                            set_patterns(const std::vector<std::wstring>& patterns);
};

std::unique_ptr<XmlConditionNode> create_condition(XmlConditionNode::Action action, XmlConditionNode::Predicate predicate, const std::vector<std::wstring>& patterns);
std::unique_ptr<XmlConditionNode> create_condition(const std::string& name, XmlConditionNode::Action action, XmlConditionNode::Predicate predicate, const std::vector<std::wstring>& patterns);

struct XmlQueryNode: XmlNode
{
    enum class Action { ACCEPT, REJECT };

    const XmlDefNode&       get_bound_def() const;
    bool                    is_action_specified() const;
    void                    set_action(Action action);  
    Action                  get_action() const; 
    void                    reset_action();
};

struct XmlCompositeQueryNode: XmlQueryNode
{
    static XmlQueryNode* next_query_node(XmlQueryNode*);
    using QueryNodeIterator = xml_tools::node_iterator<XmlQueryNode, next_query_node>;

    static const XmlQueryNode* next_query_node(const XmlQueryNode*);
    using ConstQueryNodeIterator = xml_tools::node_iterator<const XmlQueryNode, next_query_node>;

    using QueryNodeIteratorRange = boost::iterator_range<QueryNodeIterator>;
    using ConstQueryNodeIteratorRange = boost::iterator_range<ConstQueryNodeIterator>;

    QueryNodeIterator               begin_child_node();
    QueryNodeIterator               end_child_node();
    ConstQueryNodeIterator          begin_child_node() const;
    ConstQueryNodeIterator          end_child_node() const;

    QueryNodeIteratorRange          get_child_nodes();
    ConstQueryNodeIteratorRange     get_child_nodes() const;
};

struct XmlConditionalQueryNode: XmlQueryNode
{
    static XmlConditionNode* next_condition(XmlConditionNode*);
    using ConditionIterator = xml_tools::node_iterator<XmlConditionNode, next_condition>;

    static const XmlConditionNode* next_conditon(const XmlConditionNode*);
    using ConstConditionIterator = xml_tools::node_iterator<const XmlConditionNode, next_conditon>;

    using ConditionIteratorRange = boost::iterator_range<ConditionIterator>;
    using ConstConditionIteratorRange = boost::iterator_range<ConstConditionIterator>;

    bool                            empty() const;  
    
    void                            set_default(Action action, const boost::optional<std::string>& name = boost::optional<std::string>(), const boost::optional<std::string>& comment = boost::optional<std::string>());
    boost::optional<Action>         get_default_action() const;
    boost::optional<std::string>    get_default_name() const;
    boost::optional<std::string>    get_default_comment() const;

    ConditionIterator               begin_condition();
    ConditionIterator               end_condition();
    ConstConditionIterator          begin_condition() const;
    ConstConditionIterator          end_condition() const;

    ConditionIteratorRange          get_conditions();
    ConstConditionIteratorRange     get_conditions() const;
    
    ConditionIterator               add_condition(std::unique_ptr<XmlConditionNode> condition_node);
    ConditionIterator               push_condition_front(std::unique_ptr<XmlConditionNode> condition_node);
    ConditionIterator               remove_condition(ConditionIterator position);   
};

// create functions
std::unique_ptr<XmlQueryDoc>        parse_query(const std::string& xml_query, const XmlDefDoc& def_doc);
XmlQueryDoc&                        bind_query_definition(XmlDoc& doc, const XmlDefDoc& def_doc);
std::unique_ptr<XmlQueryDoc>        create_query(const XmlDefDoc& def_doc);
std::unique_ptr<XmlQueryDoc>        join_queries(const XmlQueryDoc& parent_doc, const XmlQueryDoc& child_doc);

// doc function
std::unique_ptr<XmlRegularDoc>      do_query(const XmlQueryDoc& query_doc, const XmlRegularDoc& reg_doc);
std::unique_ptr<XmlDefDoc>          get_output_def(const XmlQueryDoc& query_doc);

//boost::variant support
typedef boost::variant<XmlCompositeQueryNode&, XmlConditionalQueryNode&> VariantQueryNodeRef;
typedef boost::variant<const XmlCompositeQueryNode&, const XmlConditionalQueryNode&> ConstVariantQueryNodeRef;

VariantQueryNodeRef                 make_variant(XmlQueryNode& query_node);
ConstVariantQueryNodeRef            make_variant(const XmlQueryNode& query_node);

template<class T>
T* cast_query_node(XmlQueryNode* query_node)
{
    auto var = make_variant(*query_node);
    return boost::get<T>(&var);
}

template<class T>
const T* cast_query_node(const XmlQueryNode* query_node)
{
    return cast_query_node<T>(const_cast<XmlQueryNode*>(query_node));
}

template<class Visitor>
typename Visitor::result_type visit_query_node(Visitor visitor, XmlQueryNode& query_node)
{
    return boost::apply_visitor(visitor, make_variant(query_node));
}

template<class Visitor>
typename Visitor::result_type visit_query_node(Visitor visitor, const XmlQueryNode& query_node)
{
    return boost::apply_visitor(visitor, make_variant(query_node));
}

template<class Visitor>
typename Visitor::result_type visit_query_nodes(Visitor visitor, const XmlQueryNode& left_node, const XmlQueryNode& right_node)
{
    return boost::apply_visitor(visitor, make_variant(left_node), make_variant(right_node));
}

}} //namespace TR { namespace XML {