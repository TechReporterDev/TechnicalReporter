#pragma once
#include "TRWebServer.h"
#include "CommonProperty.h"

std::shared_ptr<SingleValue<bool>>              createValue(TR::XML::XmlCheckNode& checkNode);
std::shared_ptr<SingleValue<int>>               createValue(TR::XML::XmlNumberNode& numberNode);
std::shared_ptr<SingleValue<std::wstring>>      createValue(TR::XML::XmlStringLineNode& stringLineNode);
std::shared_ptr<SingleValue<std::wstring>>      createValue(TR::XML::XmlTextAreaNode& textAreaNode);
std::shared_ptr<SingleValue<int>>               createValue(TR::XML::XmlSelectorNode& selectorNode);
std::shared_ptr<ValueCollection>                createValue(TR::XML::XmlPropertyCollectionNode& collectionNode);
std::shared_ptr<ValueArray<ValueCollection>>    createValue(TR::XML::XmlPropertyListNode& listNode);