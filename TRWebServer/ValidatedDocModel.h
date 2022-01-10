#pragma once
#include "TRWebServer.h"
#include "RegularDocModel.h"

class ValidatedDocModel;
class ValidatedAdditive
{
public:
    virtual ~ValidatedAdditive() = default;
    bool                    isRecursiveValid() const;
    virtual void            checkRecursiveValidation() = 0;

protected:
    bool m_recursiveValid;
};

class ValidatedListNodeRow: public RegularListNodeRow, public ValidatedAdditive
{
public:
    friend class ValidatedDocModel; 
    virtual void checkRecursiveValidation() override;

protected:
    ValidatedListNodeRow(TR::XML::XmlGenericListNode* listNode);
};

class ValidatedStructNodeRow: public RegularStructNodeRow, public ValidatedAdditive
{
public:
    friend class ValidatedDocModel;
    virtual void checkRecursiveValidation() override;

protected:
    ValidatedStructNodeRow(TR::XML::XmlStructNode* structNode);
};

class ValidatedTrivialNodeRow: public RegularTrivialNodeRow, public ValidatedAdditive
{
public:
    friend class ValidatedDocModel;
    virtual void checkRecursiveValidation() override;

protected:
    ValidatedTrivialNodeRow(TR::XML::XmlGenericTrivialNode* trivialNode);
};

template<class RegularNodeRowType>
class ValidatedNodeRow: public RegularNodeRowType, public ValidatedAdditive
{
public:
    friend class ValidatedDocModel;
    const TR::XML::XmlValidatedNode& getValidatedNode() const
    {
        return static_cast<TR::XML::XmlValidatedNode&>(*getRegNode());
    }

    bool isRecursiveValid() const
    {
        return m_recursive_valid;
    }

    ValidatedDocModel* getValidatedDocModel() const
    {
        return static_cast<ValidatedDocModel*>(getRegularDocModel());
    }

protected:
    ValidatedNodeRow(typename RegularNodeRowType::XmlRegularNodeType& node):
        RegularNodeRowType(node),
        m_recursive_valid(true)
    {
        auto& regularNode = static_cast<TR::XML::XmlRegularNode&>(node);
        m_recursive_valid = static_cast<TR::XML::XmlValidatedNode&>(regularNode).is_valid();
    }

    bool m_recursive_valid;
}; 

class ValidatedCommentColumn: public RegularDocColumn
{
public:
    virtual std::wstring    getName() const override;
    virtual std::wstring    getString(const TR::XML::XmlRegularNode& regNode) const override;
};

class ValidatedDocModel: public RegularDocModel
{
public:
    ValidatedDocModel(TR::XML::XmlRegularDoc& doc);
    ValidatedDocModel(const ValidatedDocModel&) = delete;
    ValidatedDocModel& operator = (const ValidatedDocModel&) = delete;

private:
    virtual std::unique_ptr<RegularListNodeRow> createListNodeRow(TR::XML::XmlGenericListNode* listNode) override final;
    virtual std::unique_ptr<RegularStructNodeRow> createStructNodeRow(TR::XML::XmlStructNode* structNode) override final;
    virtual std::unique_ptr<RegularTrivialNodeRow> createTrivialNodeRow(TR::XML::XmlGenericTrivialNode* trivialNode) override final;
};