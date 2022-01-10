#pragma once
#include <boost\variant.hpp>
#include "TrivialTypes.h"

namespace TR { namespace Core {

struct SourceKey: KeyRestriction
{
    SourceKey() = default;

    explicit SourceKey(Key key):
        KeyRestriction(key)
    {
    }

    static SourceKey create_invalid()
    {
        return SourceKey();
    }
};

struct RoleKey: KeyRestriction
{
    RoleKey() = default;

    explicit RoleKey(Key key):
        KeyRestriction(key)
    {
    }

    static RoleKey create_invalid()
    {
        return RoleKey();
    }
};

using SubjectKey = boost::variant<SourceKey, RoleKey>;

}} //namespace TR { namespace Core {