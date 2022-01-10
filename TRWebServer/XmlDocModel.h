#pragma once
#include "TRWebServer.h"
#include "BaseTreeModel.h"

class XmlDocModel;
class XmlNodeRow: private BaseTreeModelRow
{
public:
    friend class XmlDocModel;
    friend class XmlDocColumn;  
    virtual ~XmlNodeRow();

    //dynamicDowncast support
    virtual bool                        checkType(const std::type_info& tpi) const;

    XmlDocModel*                        getXmlDocModel();

    std::wstring                        getText() const;
    void                                setText(const std::wstring& text);
    
    XmlNodeRow*                         getParentRow();
    size_t                              getPosition() const;
    XmlNodeRow&                         addChildRow(std::unique_ptr<XmlNodeRow> xmlNodeRow, size_t position = MAX_ROW_POS);
    std::unique_ptr<XmlNodeRow>         removeChildRow(size_t position);
    size_t                              getChildCount() const;
    XmlNodeRow&                         getChildRow(size_t position);
    void                                swapChildRows(size_t first, size_t second);

    const TR::XML::XmlNode&             getXmlNode() const;
    
protected:
    XmlNodeRow(TR::XML::XmlNode* xmlNode) noexcept;
    XmlNodeRow(std::unique_ptr<TR::XML::XmlNode> xmlNode) noexcept;
    TR::XML::XmlNode* m_xmlNode;
    bool m_owner;
};

class XmlDocColumn: public BaseTreeModelColumn
{
public:
    virtual std::wstring    getString(const BaseTreeModelRow& row) const override;
    virtual bool            less(const BaseTreeModelRow& left, const BaseTreeModelRow& right) const override;

    virtual std::wstring    getString(const TR::XML::XmlNode&) const = 0;
    virtual bool            less(const TR::XML::XmlNode& left, const TR::XML::XmlNode& right) const;
};

class XmlNameColumn: public XmlDocColumn
{
public:
    //XmlDocColumn override
    virtual std::wstring    getName() const override;
    virtual std::wstring    getString(const TR::XML::XmlNode& xmlNode) const override;
};

class XmlTextColumn: public XmlDocColumn
{
public:
    //XmlDocColumn override
    virtual std::wstring    getName() const override;
    virtual std::wstring    getString(const TR::XML::XmlNode& xmlNode) const override;
};

class XmlDocModel: private BaseTreeModel
{
public:
    friend XmlDocModel* XmlNodeRow::getXmlDocModel();
    friend class XmlDocTreeView;

    XmlDocModel(TR::XML::XmlDoc& doc);
    virtual ~XmlDocModel(){}

    XmlDocModel(const XmlDocModel&) = delete;
    XmlDocModel& operator = (const XmlDocModel&) = delete;
    
    void                                addColumn(std::unique_ptr<XmlDocColumn> column, size_t position = MAX_COL_POS);
    const XmlDocColumn&                 getColumn(size_t position) const;
    
    const TR::XML::XmlDoc&              getXmlDoc() const;  
    std::unique_ptr<XmlNodeRow>         buildXmlNodeRow(std::unique_ptr<TR::XML::XmlNode> xmlNode);
    XmlNodeRow&                         getRootNode();

protected:
    XmlDocModel();
    void                                build(TR::XML::XmlDoc& doc);
    std::unique_ptr<XmlNodeRow>         buildXmlNodeRow(TR::XML::XmlNode* xmlNode);
    virtual std::unique_ptr<XmlNodeRow> createXmlNodeRow(TR::XML::XmlNode* xmlNode);

private:
    TR::XML::XmlDoc* m_doc;
};