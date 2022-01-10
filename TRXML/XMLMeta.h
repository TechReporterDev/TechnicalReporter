#pragma once
#include "XMLDefinition.h"
#include "TRXMLStd.h"

namespace TR { namespace XML { namespace META {

using UUID = stl_tools::uuid;
using Key = int;
using Blob = std::vector<char>;

struct XmlActionNode: XmlMetaNode
{ 
    static XmlActionNode*           cast_meta_node(XmlMetaNode* meta_node);
    static const XmlActionNode*     cast_meta_node(const XmlMetaNode* meta_node);

    UUID                            get_action_uuid() const;
    std::wstring                    get_caption() const;
    std::unique_ptr<XmlDoc>         get_params(const XmlNode& context) const;   
};

template<class T>
T* cast_meta_node(XmlMetaNode* meta_node)
{
    return T::cast_meta_node(meta_node);
}

template<class T>
const T* cast_meta_node(const XmlMetaNode* meta_node)
{
    return T::cast_meta_node(meta_node);
}

}}}