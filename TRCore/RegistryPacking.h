#pragma once
#include "Packing.h"
#include "RegistryFwd.h"
#include "RegistryRefs.h"
#include "RegistryValues.hxx"
#include "stddecl.h"
namespace TR { namespace Core {

template<>
struct Packing<SourceRef>
{
    using packed_type = SourceKey;
    static SourceKey pack(SourceRef source_ref);
};

template<>
struct Unpacking<SourceKey>
{
    using unpacked_type = SourceRef;
    static SourceRef unpack(SourceKey source_key, const Sources* sources);
};

template<>
struct Packing<RoleRef>
{
    using packed_type = RoleKey;
    static RoleKey pack(RoleRef role_ref);
};

template<>
struct Unpacking<RoleKey>
{
    using unpacked_type = RoleRef;
    static RoleRef unpack(RoleKey role_key, const Roles* roles);
};

///////////////////
// subject packing
///////////////////

struct SubjectPacking: boost::static_visitor<SubjectIdentity>
{
    template<class U>
    SubjectIdentity operator()(const U& u) const
    {
        return pack(u);
    }
};

template<>
struct Packing<SubjectRef>
{
    using packed_type = SubjectIdentity;
    static SubjectIdentity pack(SubjectRef subject_ref)
    {
        return boost::apply_visitor(SubjectPacking(), subject_ref);
    }
};

struct SubjectUnpacking: boost::static_visitor<SubjectRef>
{
    SubjectUnpacking(Sources* sources, Roles* roles):
        m_sources(sources),
        m_roles(roles)
    {
    }

    SubjectRef operator()(const SourceKey& source_key) const
    {
        return unpack(source_key, m_sources);
    }

    RoleRef operator()(const RoleKey& role_key) const
    {
        return unpack(role_key, m_roles);
    }

    Sources* m_sources;
    Roles* m_roles;
};

template<>
struct Unpacking<SubjectIdentity>
{
    using unpacked_type = SubjectRef;
    static SubjectRef unpack(SubjectIdentity subject_key, Sources* sources, Roles* roles)
    {
        return boost::apply_visitor(SubjectUnpacking(sources, roles), subject_key.get_boost_variant());
    }
};

}} //namespace TR { namespace Core {
