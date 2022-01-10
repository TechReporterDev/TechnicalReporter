#pragma once
#define BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#define BOOST_MPL_LIMIT_LIST_SIZE 40

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// STL
#include <memory>
#include <vector>
#include <list>
#include <string>
#include <set>

// BOOST

#define BOOST_THREAD_VERSION 3

#include <boost\lexical_cast.hpp>
#include <boost\range.hpp>
#include <boost\range\any_range.hpp>
#include <boost\range\algorithm.hpp>
#include <boost\regex.hpp>
#include <boost\algorithm\string.hpp>

// TestAssist
#include "TestAssist\test_assist.h"

// STLTools
#include "StlTools\stl_tools.h"

// XMLTools
#include "XmlTools\xml_tools.h"
