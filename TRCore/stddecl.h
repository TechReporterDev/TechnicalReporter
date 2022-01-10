#pragma once

// STL
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <set>
#include <mutex>
#include <thread>
#include <future>
#include <functional>
#include <queue>
#include <utility>

// BOOST
#include <boost\uuid\uuid.hpp>
#include <boost\range.hpp>
#include <boost\range\adaptor\map.hpp>
#include <boost\range\adaptor\indirected.hpp>
#include <boost\range\adaptor\reversed.hpp>
#include <boost\range\adaptor\filtered.hpp>
#include <boost\optional.hpp>
#include <boost\iterator\indirect_iterator.hpp>
#include <boost\range\algorithm.hpp>
#include <boost\range\any_range.hpp>
#include <boost\signals2.hpp>
#include <boost\any.hpp>
#include <boost\variant\polymorphic_get.hpp>

// TRXML
#include "TRXML\TRXMLFwd.h"

// STLTools
#include "STLTools\uuid.h"
#include "STLTools\ordered_signal.h"
#include "STLTools\lazy.h"
#include "STLTools\no_except.h"

#undef max
#undef min

namespace TR { namespace Core {

template <typename Signature>
using Signal = boost::signals2::signal<Signature>;

template <typename Signature>
using OrderedSignal = stl_tools::ordered_signal<Signature>;

using stl_tools::no_except;

using Priority = int;

template <typename Signature>
using SlotFunctionType = typename Signal<Signature>::slot_function_type;

using XML::XmlDoc;
using XML::XmlNode;
using XML::XmlDefDoc;
using XML::XmlRegularDoc;
using XML::XmlQueryDoc;
using XML::XmlComplianceDef;
using XML::XmlComplianceDoc;
using XML::XmlComplianceSetup;

using XML::XmlPropertiesDef;
using XML::XmlPropertiesDoc;

}} //namespace TR {
