#pragma once
#include "TrivialTypes.h"
namespace TR { namespace Core {

struct QueryJoiningKey: KeyRestriction
{
    QueryJoiningKey() = default;

    explicit QueryJoiningKey(Key key):
        KeyRestriction(key)
    {
    }
};

struct ComplianceKey: KeyRestriction
{
    ComplianceKey() = default;

    explicit ComplianceKey(Key key):
        KeyRestriction(key)
    {
    }
};

struct ValidationKey: KeyRestriction
{
    ValidationKey() = default;

    explicit ValidationKey(Key key):
        KeyRestriction(key)
    {
    }
};

struct ArchivedReportKey: KeyRestriction
{
    ArchivedReportKey() = default;

    explicit ArchivedReportKey(Key key):
        KeyRestriction(key)
    {
    }
};
    
}} //namespace TR { namespace Core {
