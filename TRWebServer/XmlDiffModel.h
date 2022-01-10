#pragma once
#include "TRWebServer.h"
#include "BaseTreeModel.h"

class XmlDiffNodeRow: private BaseTreeModelRow
{
public:
    friend class XmlDiffModel;
    friend class XmlDiffColumn;

    enum State { STATE_NONE = 0, STATE_INSERTED, STATE_REMOVED, STATE_MODIFIED };
    XmlDiffNodeRow(const TR::XML::XmlNode* newNode, const TR::XML::XmlNode* oldNode = nullptr, State state = STATE_NONE);
    
    //private inheritance woraround
    template<class D, class B> D& dynamicDowncast(B& base_ref);

    //dynamicDowncast support
    virtual bool checkType(const std::type_info& tpi) const;
    
    XmlDiffModel*                   getXmlDiffModel() const;    
    const TR::XML::XmlNode*         getNewNode() const;
    const TR::XML::XmlNode*         getOldNode() const;
    State                           getDiffState() const;   

protected:  
    XmlDiffNodeRow&                 addChildRow(std::unique_ptr<XmlDiffNodeRow> diffNodeRow, size_t position = MAX_ROW_POS);

    const TR::XML::XmlNode* m_newNode;
    const TR::XML::XmlNode* m_oldNode;
    State m_state;
};

class XmlDiffColumn: public BaseTreeModelColumn
{
public:
    virtual std::wstring    getString(const XmlDiffNodeRow&) const = 0;
    virtual bool            less(const XmlDiffNodeRow& left, const XmlDiffNodeRow& right) const;

    //BaseTreeModelColumn override
    virtual std::wstring    getString(const BaseTreeModelRow& row) const override;
    virtual bool            less(const BaseTreeModelRow& left, const BaseTreeModelRow& right) const override;
};

class XmlDiffNameColumn: public XmlDiffColumn
{
public:
    virtual std::wstring    getName() const override;
    virtual std::wstring    getString(const XmlDiffNodeRow& diffRow) const override;
};

class XmlDiffNewValueColumn: public XmlDiffColumn
{
public:
    virtual std::wstring    getName() const override;
    virtual std::wstring    getString(const XmlDiffNodeRow& diffRow) const override;
};

class XmlDiffOldValueColumn: public XmlDiffColumn
{
public:
    virtual std::wstring    getName() const override;
    virtual std::wstring    getString(const XmlDiffNodeRow& diffRow) const override;
};

class XmlDiffModel: private BaseTreeModel
{
public:
    friend XmlDiffModel* XmlDiffNodeRow::getXmlDiffModel() const;
    friend class XmlDiffTreeView;

    XmlDiffModel(const TR::XML::XmlDoc& newDoc, const TR::XML::XmlDoc& oldDoc, const std::vector<TR::Core::XmlDiffPoint>& diffPoints);
    virtual ~XmlDiffModel() = default;

    XmlDiffModel(const XmlDiffModel&) = delete;
    XmlDiffModel& operator = (const XmlDiffModel&) = delete;

    void                            addColumn(std::unique_ptr<XmlDiffColumn> column);
    XmlDiffColumn&                  getColumn(size_t position) const;

protected:
    XmlDiffModel();
    void build(const TR::XML::XmlDoc& newDoc, const TR::XML::XmlDoc& oldDoc, const std::vector<TR::Core::XmlDiffPoint>& diffPoints);

private:
    virtual std::unique_ptr<XmlDiffNodeRow> createDiffNodeRow(const TR::XML::XmlNode* newNode, const TR::XML::XmlNode* oldNode = nullptr);
    std::unique_ptr<XmlDiffNodeRow> buildDiffNodeRow(const TR::XML::XmlNode* newNode, const TR::XML::XmlNode* oldNode = nullptr);
    void                            updateState(XmlDiffNodeRow& diffNodeRow, XmlDiffNodeRow::State state);

    void                            addDiffPoint(const TR::Core::XmlDiffPoint& diffPoint);
    XmlDiffNodeRow&                 getDiffRow(TR::Core::XmlDiffPoint::Path::const_iterator first, TR::Core::XmlDiffPoint::Path::const_iterator postLast);
    const TR::XML::XmlNode&         getOldNode(TR::Core::XmlDiffPoint::Path::const_iterator first, TR::Core::XmlDiffPoint::Path::const_iterator postLast) const;

    const TR::XML::XmlDoc* m_newDoc;
    const TR::XML::XmlDoc* m_oldDoc;
};