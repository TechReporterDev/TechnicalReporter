#pragma once
#include "TRWebServer.h"
#include "MainFrame.h"

class StreamFilterWidget: public MainFrameWidget
{
public:
    StreamFilterWidget(std::shared_ptr<LazyStreamQuery> previewStream);
    StreamFilterWidget(const StreamFilterWidget&) = delete;

protected:
    void            initialize();
    virtual void    onPersistClick() = 0;
    void            onPreviewClick();
    void            onCloseClick();

    TR::StreamFilterSetup m_filterSetup;
    std::shared_ptr<LazyStreamQuery> m_previewStream;
};

class CreateStreamFilterWidget: public StreamFilterWidget
{
public:
    CreateStreamFilterWidget(TR::StreamTypeInfo inputInfo, std::shared_ptr<LazyStreamQuery> previewStream);
    CreateStreamFilterWidget(const CreateStreamFilterWidget&) = delete;

protected:
    // override FilterSetupWidget
    virtual void onPersistClick() override;

    TR::StreamTypeInfo m_inputInfo;
    std::shared_ptr<LazyStreamQuery> m_previewStream;
};

class CreateStreamFilterPage: public MainFramePage
{
public:
    CreateStreamFilterPage(TR::StreamTypeInfo inputInfo, std::shared_ptr<LazyStreamQuery> previewStream = nullptr);
    virtual MainFrameWidget* getWidget() override;

private:
    CreateStreamFilterWidget m_widget;
};

class ModifyStreamFilterWidget: public StreamFilterWidget
{
public:
    ModifyStreamFilterWidget(TR::StreamFilterInfo filterInfo, std::shared_ptr<LazyStreamQuery> previewStream);
    ModifyStreamFilterWidget(const StreamFilterWidget&) = delete;

protected:
    // override FilterSetupWidget
    virtual void onPersistClick() override;

    TR::StreamFilterInfo m_filterInfo;
};

class ModifyStreamFilterPage: public MainFramePage
{
public:
    ModifyStreamFilterPage(TR::StreamFilterInfo filterInfo, std::shared_ptr<LazyStreamQuery> previewStream);
    virtual MainFrameWidget* getWidget() override;

private:
    ModifyStreamFilterWidget m_widget;
};