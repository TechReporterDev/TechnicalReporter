#include "stdafx.h"
#include "TRWebServer.h"
#include "ValidatedDocModel.h"

bool ValidatedAdditive::isRecursiveValid() const
{
    return m_recursiveValid;
};

void ValidatedListNodeRow::checkRecursiveValidation()
{
    m_recursiveValid = static_cast<const TR::XML::XmlValidatedNode&>(getRegNode()).is_valid();
    for (size_t position = 0; position < getItemRowCount(); ++position)
    {
        auto& validatedItem = dynamic_cast<ValidatedAdditive&>(getItemRow(position));
        validatedItem.checkRecursiveValidation();
        if (m_recursiveValid)
        {
            m_recursiveValid = validatedItem.isRecursiveValid();
        }
    }   
}

ValidatedListNodeRow::ValidatedListNodeRow(TR::XML::XmlGenericListNode* listNode):
    RegularListNodeRow(listNode)
{
}

void ValidatedStructNodeRow::checkRecursiveValidation()
{
    m_recursiveValid = static_cast<const TR::XML::XmlValidatedNode&>(getRegNode()).is_valid();
    for (size_t position = 0; position < getMemberRowCount(); ++position)
    {
        auto& validatedItem = dynamic_cast<ValidatedAdditive&>(getMemberRow(position));
        validatedItem.checkRecursiveValidation();
        if (m_recursiveValid)
        {
            m_recursiveValid = validatedItem.isRecursiveValid();
        }
    }   
}

ValidatedStructNodeRow::ValidatedStructNodeRow(TR::XML::XmlStructNode* structNode):
    RegularStructNodeRow(structNode)
{   
}

void ValidatedTrivialNodeRow::checkRecursiveValidation()
{
    m_recursiveValid = static_cast<const TR::XML::XmlValidatedNode&>(getRegNode()).is_valid();
}

ValidatedTrivialNodeRow::ValidatedTrivialNodeRow(TR::XML::XmlGenericTrivialNode* trivialNode):
    RegularTrivialNodeRow(trivialNode)
{
}

std::wstring ValidatedCommentColumn::getName() const
{
    return L"Comment";
}

std::wstring ValidatedCommentColumn::getString(const TR::XML::XmlRegularNode& regNode) const
{
    auto& validatedNode = static_cast<const TR::XML::XmlValidatedNode&>(regNode);
    return validatedNode.get_comment();
}

ValidatedDocModel::ValidatedDocModel(TR::XML::XmlRegularDoc& doc)
{
    build(doc);
    auto& validatedRoot = dynamic_cast<ValidatedAdditive&>(getRootNode());
    validatedRoot.checkRecursiveValidation();
}

std::unique_ptr<RegularListNodeRow> ValidatedDocModel::createListNodeRow(TR::XML::XmlGenericListNode* listNode)
{
    return std::unique_ptr<RegularListNodeRow>(new ValidatedListNodeRow(listNode));
}

std::unique_ptr<RegularStructNodeRow> ValidatedDocModel::createStructNodeRow(TR::XML::XmlStructNode* structNode)
{
    return std::unique_ptr<RegularStructNodeRow>(new ValidatedStructNodeRow(structNode));
}

std::unique_ptr<RegularTrivialNodeRow> ValidatedDocModel::createTrivialNodeRow(TR::XML::XmlGenericTrivialNode* trivialNode)
{
    return std::unique_ptr<RegularTrivialNodeRow>(new ValidatedTrivialNodeRow(trivialNode));
}