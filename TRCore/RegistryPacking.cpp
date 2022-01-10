#include "stdafx.h"
#include "RegistryPacking.h"
#include "Registry.h"
namespace TR { namespace Core {

SourceKey Packing<SourceRef>::pack(SourceRef source_ref)
{
    return SourceKey(source_ref.get_key());
}

SourceRef Unpacking<SourceKey>::unpack(SourceKey source_key, const Sources* sources)
{
    return SourceRef(sources, source_key);
}

RoleKey Packing<RoleRef>::pack(RoleRef role_ref)
{
    return RoleKey(role_ref.get_key());
}

RoleRef Unpacking<RoleKey>::unpack(RoleKey role_key, const Roles* roles)
{
    return RoleRef(roles, role_key);
}

}} //namespace TR { namespace Core {