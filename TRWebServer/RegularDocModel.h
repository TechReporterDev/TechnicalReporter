#pragma once
#include "TRWebServer.h"
#include "XmlDocModel.h"

class RegularNodeRow: private XmlNodeRow
{
public:
    friend class RegularDocModel;
    friend class RegularListNodeRow;
    friend class RegularStructNodeRow;
    friend class RegularTrivialNodeRow;

    //dynamicDowncast support
    virtual bool                            checkType(const std::type_info& tpi) const;

    RegularDocModel*                        getRegularDocModel();
    RegularNodeRow*                         getParentRow();
    size_t                                  getPosition() const;
    const TR::XML::XmlRegularNode&          getRegNode() const;

protected:
    RegularNodeRow(TR::XML::XmlRegularNode* regNode);
    RegularNodeRow(std::unique_ptr<TR::XML::XmlRegularNode> regNode);
};

class RegularListNodeRow: public RegularNodeRow
{
public:
    friend class RegularDocModel;
    
    RegularNodeRow&                         addItemRow();
    void                                    moveItemRowUp(size_t position);
    void                                    moveItemRowDown(size_t position);
    void                                    removeItemRow(size_t position);
    size_t                                  getItemRowCount() const;
    RegularNodeRow&                         getItemRow(size_t position);

    const TR::XML::XmlGenericListNode&      getListNode() const;

protected:
    RegularListNodeRow(TR::XML::XmlGenericListNode* listNode);
    RegularListNodeRow(std::unique_ptr<TR::XML::XmlGenericListNode> listNode);
};

class RegularStructNodeRow: public RegularNodeRow
{
public:
    friend class RegularDocModel;
    size_t                                  getMemberRowCount() const;
    RegularNodeRow&                         getMemberRow(size_t position);

    const TR::XML::XmlStructNode&           getStructNode() const;

protected:
    RegularStructNodeRow(TR::XML::XmlStructNode* structNode);
    RegularStructNodeRow(std::unique_ptr<TR::XML::XmlStructNode> structNode);
};

class RegularTrivialNodeRow: public RegularNodeRow
{
public:
    friend class RegularDocModel;
    
    std::wstring    getValue() const;
    void            setValue(const std::wstring& value);

protected:
    RegularTrivialNodeRow(TR::XML::XmlGenericTrivialNode* trivialNode);
    RegularTrivialNodeRow(std::unique_ptr<TR::XML::XmlGenericTrivialNode> trivialNode);
};

class RegularDocColumn: public XmlDocColumn
{
public:
    virtual std::wstring    getString(const TR::XML::XmlNode&) const override;
    virtual bool            less(const TR::XML::XmlNode& left, const TR::XML::XmlNode& right) const override;

    virtual std::wstring    getString(const TR::XML::XmlRegularNode&) const = 0;
    virtual bool            less(const TR::XML::XmlRegularNode& left, const TR::XML::XmlRegularNode& right) const;
};

class RegularNameColumn: public RegularDocColumn
{
public:
    virtual std::wstring    getName() const override;
    virtual std::wstring    getString(const TR::XML::XmlRegularNode& regNode) const override;
};

class RegularActionColumn: public RegularDocColumn
{
public:
    virtual std::wstring    getName() const override;
    virtual std::wstring    getString(const TR::XML::XmlRegularNode& regNode) const override;
};

class RegularValueColumn: public RegularDocColumn
{
public:
    virtual std::wstring    getName() const override;
    virtual std::wstring    getString(const TR::XML::XmlRegularNode& regNode) const override;
};

class RegularDescriptionColumn: public RegularDocColumn
{
public:
    virtual std::wstring    getName() const;
    virtual std::wstring    getString(const TR::XML::XmlRegularNode& regNode) const;
};

class RegularDocModel: private XmlDocModel
{
public:
    struct RegularNodeRowCreator;
    friend RegularDocModel* RegularNodeRow::getRegularDocModel();
    friend RegularNodeRow& RegularListNodeRow::addItemRow();
    friend class RegularDocTreeView;
    friend class RegularDocEditor;

    RegularDocModel(TR::XML::XmlRegularDoc& doc);
    virtual ~RegularDocModel(){}

    RegularDocModel(const RegularDocModel&) = delete;
    RegularDocModel& operator = (const RegularDocModel&) = delete;
    
    void                                            addColumn(std::unique_ptr<RegularDocColumn> column, size_t position = MAX_COL_POS);
    const RegularDocColumn&                         getColumn(size_t position) const;
    const TR::XML::XmlRegularDoc&                   getRegularDoc() const;  
    RegularNodeRow&                                 getRootNode();

protected:
    RegularDocModel();
    void build(TR::XML::XmlRegularDoc& doc);

private:
    std::unique_ptr<RegularNodeRow>                 buildRegularNodeRow(std::unique_ptr<TR::XML::XmlRegularNode> regNode);
    virtual std::unique_ptr<RegularListNodeRow>     createListNodeRow(TR::XML::XmlGenericListNode* listNode);
    virtual std::unique_ptr<RegularStructNodeRow>   createStructNodeRow(TR::XML::XmlStructNode* structNode);
    virtual std::unique_ptr<RegularTrivialNodeRow>  createTrivialNodeRow(TR::XML::XmlGenericTrivialNode* trivialNode);
    virtual std::unique_ptr<XmlNodeRow>             createXmlNodeRow(TR::XML::XmlNode* xmlNode) override final;
};