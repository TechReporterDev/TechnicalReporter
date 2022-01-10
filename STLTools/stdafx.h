#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// STL
#include <memory>
#include <functional>
#include <vector>
#include <iostream>
#include <string>
#include <type_traits>
#include <algorithm>
#include <numeric>
#include <mutex>

// BOOST
#include <boost\format.hpp>
#include <boost\uuid\uuid.hpp>
#include <boost\uuid\uuid_generators.hpp>
#include <boost\optional.hpp>
#include <boost\lexical_cast.hpp>
#include <boost\uuid\uuid_io.hpp>
#include <boost\range\adaptors.hpp>

// TestAssist
#include "TestAssist\test_assist.h"