#pragma once
#include "ContentEditor.h"
#include "TRWebServer.h"
#include "MainFrame.h"

class PatternEditor: public MainFrameWidget
{
public:
    PatternEditor(TR::SubjectKey subjectKey, TR::ReportTypeUUID reportTypeUUID);
    PatternEditor(const PatternEditor&) = delete;
    PatternEditor& operator = (const PatternEditor&) = delete;

    void apply();

protected:
    TR::SubjectKey m_subjectKey;
    TR::ReportTypeUUID m_reportTypeUUID;
    ContentEditor* m_contentEditor;
};