#pragma once
#include "TRWebServer.h"
#include "CommonProperty.h"

CheckPropertyFactory        createPropertyFactory(const TR::XML::XmlCheckDef& checkDef);
NumberPropertyFactory       createPropertyFactory(const TR::XML::XmlNumberDef& numberDef);
StringPropertyFactory       createPropertyFactory(const TR::XML::XmlStringLineDef& stringLineDef);
TextPropertyFactory         createPropertyFactory(const TR::XML::XmlTextAreaDef& textAreaDef);
SelectorPropertyFactory     createPropertyFactory(const TR::XML::XmlSelectorDef& selectorDef);
CommonPropertyFactory       createPropertyFactory(const TR::XML::XmlPropertyCollectionDef& collectionDef);
CommonPropertyArrayFactory  createPropertyFactory(const TR::XML::XmlPropertyListDef& listDef);