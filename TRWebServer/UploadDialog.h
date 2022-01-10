#pragma once

class UploadDialog: public Wt::WDialog
{
public:
    using OnUploaded = std::function<void(const Wt::WString& path)>;
    UploadDialog();
    void connectUploaded(OnUploaded onUploaded);

private:
    Wt::WFileUpload* m_fileUpload;
    OnUploaded m_onUploaded;
};

class UploadButton: public Wt::WPushButton
{
public:
    using OnUploaded = UploadDialog::OnUploaded;
    UploadButton(const Wt::WString& text);
    void connectUploaded(OnUploaded onUploaded);

private:
    OnUploaded m_onUploaded;
};
