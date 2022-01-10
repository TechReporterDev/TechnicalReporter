#pragma once
#include "Dialog.h"

class BrowseNodeDlg: public Dialog
{
public:
    BrowseNodeDlg(const TR::XML::XmlDefDoc& defDoc, const TR::XML::XmlDefNode* selectedNode = nullptr);
    void show(std::function<void(const TR::XML::XmlDefNode*)> onApply = nullptr);

protected:
    // Dialog override
    virtual void apply() override;

private:
    const TR::XML::XmlDefDoc& m_defDoc;
    const TR::XML::XmlDefNode* m_selectedNode;
    std::function<void(const TR::XML::XmlDefNode*)> m_onApply;
};