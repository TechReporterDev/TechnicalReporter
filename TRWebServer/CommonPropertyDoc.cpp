#include "stdafx.h"
#include "CommonPropertyDoc.h"
#include "PropertyNodeFactory.h"
#include "PropertyNodeValue.h"
#include "Application.h"

CommonProperty createProperty(TR::XML::XmlPropertiesDoc& propsDoc)
{
    auto& propsDef = propsDoc.get_properties_def();
    auto propertyFactory = createPropertyFactory(propsDef.get_root_def());
    return propertyFactory.createProperty(createValue(propsDoc.get_root()));
}