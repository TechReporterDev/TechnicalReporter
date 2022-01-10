#include "stdafx.h"
#include "UploadDialog.h"
#include "Application.h"

UploadDialog::UploadDialog():
    WDialog(L"Upload file"),
    m_fileUpload(nullptr)
{
    rejectWhenEscapePressed();
    setWidth(300);

    m_fileUpload = contents()->addWidget(std::make_unique<Wt::WFileUpload>());
    m_fileUpload->setProgressBar(new Wt::WProgressBar());

    m_fileUpload->uploaded().connect([this](Wt::NoClass){
        if (m_onUploaded)
        {
            m_onUploaded(m_fileUpload->spoolFileName());
        }
    });

    auto btnOk = footer()->addWidget(std::make_unique<Wt::WPushButton>("Upload"));
    btnOk->setDefault(true);
    btnOk->clicked().connect([this](Wt::WMouseEvent)
    {
        if (m_fileUpload->canUpload())
        {
            m_fileUpload->upload();
        }
    });

    auto btnCancel = footer()->addWidget(std::make_unique<Wt::WPushButton>("Close"));
    btnCancel->clicked().connect(this, &Wt::WDialog::reject);

    finished().connect(std::bind([this]() {
        delete this;
    }));
}

void UploadDialog::connectUploaded(OnUploaded onUploaded)
{
    m_onUploaded = onUploaded;
}

UploadButton::UploadButton(const Wt::WString& text):
    Wt::WPushButton(text)
{
    clicked().connect([this](Wt::WMouseEvent){
        auto uploadDlg = new UploadDialog();
        uploadDlg->connectUploaded(m_onUploaded);
        uploadDlg->show();
    });
}

void UploadButton::connectUploaded(OnUploaded onUploaded)
{
    m_onUploaded = onUploaded;
}