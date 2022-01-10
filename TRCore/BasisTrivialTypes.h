#pragma once
#include <boost\variant.hpp>
#include "TrivialTypes.h"

namespace TR {  namespace Core {

struct ReportTypeUUID: UUIDRestriction
{
    ReportTypeUUID() = default;

    explicit ReportTypeUUID(UUID uuid):
        UUIDRestriction(uuid)
    {
    }

    static ReportTypeUUID create_invalid()
    {
        return ReportTypeUUID(INVALID_UUID);
    }
};

struct StreamTypeUUID: UUIDRestriction
{
    StreamTypeUUID() = default;

    explicit StreamTypeUUID(UUID uuid):
        UUIDRestriction(uuid)
    {
    }

    static StreamTypeUUID create_invalid()
    {
        return StreamTypeUUID(INVALID_UUID);
    }
};

struct SourceTypeUUID: UUIDRestriction
{
    SourceTypeUUID() = default;

    explicit SourceTypeUUID(UUID uuid):
        UUIDRestriction(uuid)
    {
    }   
};

struct ActionUUID: UUIDRestriction
{
    ActionUUID() = default;

    explicit ActionUUID(UUID uuid):
        UUIDRestriction(uuid)
    {
    }   
};

typedef boost::variant<ReportTypeUUID, StreamTypeUUID> ResourceUUID;

}} //namespace TR { namespace Core {
