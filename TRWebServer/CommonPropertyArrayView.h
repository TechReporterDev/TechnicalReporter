#pragma once
#include "TRWebServer.h"
#include "CommonPropertyView.h"
#include "StandardTable.h"

class CommonPropertyArrayView: public DetailedPropertyView
{
public:
    CommonPropertyArrayView(const PropertyArray<CommonPropertyFactory>& propertyArray);

private:
    void showEditDlg(size_t row);

    PropertyArray<CommonPropertyFactory> m_propertyArray;
    StandardTable<nullptr_t>* m_previewTable;
};