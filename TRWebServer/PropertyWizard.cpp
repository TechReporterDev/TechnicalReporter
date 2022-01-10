#include "stdafx.h"
#include "PropertyWizard.h"
#include "Dialog.h"
#include "Application.h"

PropertyWizard::PropertyWizard(const Wt::WString& windowTitle):
    Wt::WDialog(windowTitle),
    m_propertySheet(nullptr),
    m_btnNext(nullptr),
    m_btnBack(nullptr)
{
    rejectWhenEscapePressed();
    setWidth(400);
    setHeight(480);

    contents()->setPadding(10);
    footer()->setMargin(0, Wt::Side::Top);

    auto vbox = contents()->setLayout(std::make_unique<Wt::WVBoxLayout>());
    vbox->setContentsMargins(0, 0, 0, 0);

    m_propertySheet = vbox->addWidget(std::make_unique<PropertySheetWidget>(
        PropertySheetWidget::DEFAULT_ATTRIBUTES ^ PropertySheetWidget::SHOW_BUTTONS ^ PropertySheetWidget::ENABLE_VALIDATION));

    m_btnBack = footer()->addWidget(std::make_unique<Wt::WPushButton>("Back"));
    m_btnBack->setEnabled(false);
    m_btnBack->clicked().connect([this](Wt::WMouseEvent){
        m_propertySheet->prevPage();
        if (m_propertySheet->firstPage())
        {
            m_btnBack->setEnabled(false);
        }
        m_btnNext->setText(L"Next");
    });

    m_btnNext = footer()->addWidget(std::make_unique<Wt::WPushButton>("Next"));
    m_btnNext->setDefault(true);
    m_btnNext->clicked().connect([this](Wt::WMouseEvent){
        auto focused = getFocus(m_propertySheet);
        if (dynamic_cast<Wt::WTextArea*>(focused))
        {
            return;
        }

        auto currentPage = m_propertySheet->getPage();
        if (!currentPage->validate())
        {
            return;
        }

        if (isFinal(currentPage->getIdentity()))
        {
            accept();
            return;
        }

        m_propertySheet->pushPage(nextPage(currentPage->getIdentity()));

        if (isFinal(m_propertySheet->getPage()->getIdentity()))
        {
            m_btnNext->setText(L"Finish");
        }
        m_btnBack->setEnabled(true);
    });

    auto btnCancel = footer()->addWidget(std::make_unique<Wt::WPushButton>("Cancel"));
    btnCancel->clicked().connect(this, &Wt::WDialog::reject);

    finished().connect(std::bind([this]() {
        if (result() == Wt::DialogCode::Accepted)
        {
            onFinish();
        }
        delete this;
    }));
}

void PropertyWizard::init(std::unique_ptr<PropertyPageWidget> firstPage)
{
    m_propertySheet->pushPage(std::move(firstPage));
    if (isFinal(m_propertySheet->getPage()->getIdentity()))
    {
        m_btnNext->setText(L"Finish");
    }
}