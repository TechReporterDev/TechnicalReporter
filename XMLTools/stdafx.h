#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// LIBXML
#include <libxml/tree.h>
#include <libxml/xpath.h>

// STL
#include <stdexcept>
#include <memory>
#include <functional>
#include <vector>
#include <iostream>
#include <string>
#include <codecvt>
#include <iterator>

// BOOST
#include <boost\range.hpp>
#include <boost\range\algorithm.hpp>
#include <boost\iterator\filter_iterator.hpp>

// STLTools
#include "STLTools\stl_tools.h"

// TestAssist
#include "TestAssist\test_assist.h"