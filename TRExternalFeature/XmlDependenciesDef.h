#pragma once
#include "TRCore\BasisTrivialTypes.h"
#include "stddecl.h"
namespace TR {namespace External {

struct XmlResourceDef : XmlNode
{
    UUID            get_resource_uuid() const;
    std::wstring    get_resource_name() const;
};

struct XmlDependencyDef: XmlNode
{
    static const XmlResourceDef* next_resource_def(const XmlResourceDef* resource_def);
    using ConstResourceDefIterator = xml_tools::node_iterator<const XmlResourceDef, next_resource_def>;
    using ConstResourceDefRange = boost::iterator_range<ConstResourceDefIterator>;

    UUID                    get_feature_uuid() const;
    std::wstring            get_feature_name() const;
    ConstResourceDefRange   get_resources() const;
};

struct XmlDependenciesDef: XmlNode
{
    static const XmlDependencyDef* next_dependency_def(const XmlDependencyDef* dependency_def);
    using ConstDependencyDefIterator = xml_tools::node_iterator<const XmlDependencyDef, next_dependency_def>;
    using ConstDependencyDefRange = boost::iterator_range<ConstDependencyDefIterator>;

    ConstDependencyDefRange get_dependencies() const;
};

}}// namespace TR { namespace External {