#pragma once
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define BOOST_THREAD_VERSION 3

// TestAssist
#include "TestAssist\test_assist.h"

// STLTools
#include "STLTools\stl_tools.h"

// XMLTools
#include "XMLTools\xml_tools.h"

// TRXML
#include "TRXML\TRXML.h"

// ODB
#include <odb\database.hxx>
#include <odb\transaction.hxx>
#include <odb\mysql\database.hxx>
#include <odb\schema-catalog.hxx>
#include "ODBTools.h"

// Exception
#include "Exception.h"

// STD
#include <boost\lexical_cast.hpp>
#include "stddecl.h"

