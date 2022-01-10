#pragma once

// STD
#include <memory>
#include <vector>

// XML
#include "XMLTools\xml_tools.h"
#include "XSLTTools\xslt_tools.h"
#include "TRXML\XMLDefinition.h"
#include "TRXML\XMLRegular.h"
#include "TRXML\XMLCompliance.h"
#include "TRXML\XMLProperties.h"

// STLTOOLS
#include "STLTools\stl_tools.h"

// Core
#include "TRCore\CoreFwd.h"
#include "TRCore\BasisFwd.h"

namespace TR { namespace External {
using namespace Core;

using UUID = boost::uuids::uuid;
using Blob = std::vector<char>;
using XML::XmlRegularDoc;
using XML::XmlRegularNode;
using XML::XmlDefDoc;
using XML::XmlDefNode;
using xml_tools::XmlDoc;
using xml_tools::XmlNode;
using xml_tools::XsltStylesheet;
using xml_tools::XmlGrammarDoc;

}} //namespace TR { namespace External {