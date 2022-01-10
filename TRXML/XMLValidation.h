#pragma once
#include "XMLDefinition.h"
#include "XMLRegular.h"
#include "XMLQuery.h"
#include "TRXMLStd.h"
namespace TR { namespace XML {

struct XmlValidatedNode: XmlRegularNode
{
    bool            is_valid() const;
    std::wstring    get_comment() const;
};

struct XmlValidationNode: XmlConditionNode
{
    enum Decision {VALID, INVALID};
    void            set_decision(Decision decision);
    Decision        get_decision() const;
};

std::unique_ptr<XmlValidationNode> create_validation(XmlValidationNode::Decision desition, XmlConditionNode::Predicate predicate, const std::vector<std::wstring>& patterns, const std::string& comment);

// doc function
std::unique_ptr<XmlQueryDoc> create_validator(const XmlDefDoc& def_doc);
std::unique_ptr<XmlQueryDoc> parse_validator(const std::string& xml_validator, const XmlDefDoc& def_doc);

}} //namespace TR { namespace XML {