#include "stdafx.h"
#include "RegularContentEditor.h"
#include "RegularDocEditor.h"
#include "Application.h"

RegularContentEditor::RegularContentEditor(std::shared_ptr<TR::RegularContent> content):
    m_doc(xml_tools::clone_doc(content->as_regular_doc())),
    m_defDoc(content->get_def_doc())
{
    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->addWidget(std::make_unique<RegularDocEditor>(*m_doc));
}

std::shared_ptr<TR::Content> RegularContentEditor::getContent()
{
    return std::make_shared<TR::Core::GeneralRegularContent>(xml_tools::clone_doc(*m_doc), m_defDoc);
}

void RegularContentEditor::setContent(std::shared_ptr<TR::Content> content)
{
    auto regularContent = std::dynamic_pointer_cast<TR::RegularContent>(content);
    m_doc = xml_tools::clone_doc(regularContent->as_regular_doc());
    m_defDoc = regularContent->get_def_doc();

    clear();
    auto vbox = setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->addWidget(std::make_unique<RegularDocEditor>(*m_doc));
}