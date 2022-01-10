#pragma once
#include "Packing.h"
#include "BasisFwd.h"
#include "BasisRefs.h"
#include "BasisValues.hxx"
#include "stddecl.h"

namespace TR { namespace Core {

template<>
struct Packing<ReportTypeRef>
{
    using packed_type = ReportTypeUUID;
    static ReportTypeUUID pack(ReportTypeRef report_type_ref);
};

template<>
struct Unpacking<ReportTypeUUID>
{
    using unpacked_type = ReportTypeRef;
    static ReportTypeRef unpack(ReportTypeUUID report_type_uuid, const ReportTypes* report_types);
};

template<>
struct Packing<StreamTypeRef>
{
    using packed_type = StreamTypeUUID;
    static StreamTypeUUID pack(StreamTypeRef stream_type_ref);
};

template<>
struct Unpacking<StreamTypeUUID>
{
    using unpacked_type = StreamTypeRef;
    static StreamTypeRef unpack(StreamTypeUUID stream_type_uuid, const StreamTypes* stream_types);
};

template<>
struct Packing<SourceTypeRef>
{
    using packed_type = SourceTypeUUID;
    static SourceTypeUUID pack(SourceTypeRef source_type_ref);
};

template<>
struct Unpacking<SourceTypeUUID>
{
    using unpacked_type = SourceTypeRef;
    static SourceTypeRef unpack(SourceTypeUUID source_type_uuid, const SourceTypes* source_types);
};

template<>
struct Packing<ActionRef>
{
    using packed_type = ActionUUID;
    static ActionUUID pack(ActionRef action_ref);
};

template<>
struct Unpacking<ActionUUID>
{
    using unpacked_type = ActionRef;
    static ActionRef unpack(ActionUUID action_uuid, const Actions* actions);
};

///////////////////
// resource packing
///////////////////

struct ResourcePacking: boost::static_visitor<ResourceIdentity>
{
    template<class U>
    ResourceIdentity operator()(const U& u) const
    {
        return pack(u);
    }
};

template<>
struct Packing<ResourceRef>
{
    using packed_type = ResourceIdentity;
    static ResourceIdentity pack(ResourceRef resource_ref)
    {
        return boost::apply_visitor(ResourcePacking(), resource_ref);
    }
};

struct ResourceUnpacking: boost::static_visitor<ResourceRef>
{
    ResourceUnpacking(ReportTypes* report_types, StreamTypes* stream_types):
        m_report_types(report_types),
        m_stream_types(stream_types)
    {
    }

    ResourceRef operator()(const ReportTypeUUID& report_type_uuid) const
    {
        return unpack(report_type_uuid, m_report_types);
    }

    ResourceRef operator()(const StreamTypeUUID& stream_type_uuid) const
    {
        return unpack(stream_type_uuid, m_stream_types);
    }

    ReportTypes* m_report_types;
    StreamTypes* m_stream_types;
};

template<>
struct Unpacking<ResourceIdentity>
{
    using unpacked_type = ResourceRef;
    static ResourceRef unpack(ResourceIdentity resource_identity, ReportTypes* report_types, StreamTypes* stream_types)
    {
        return boost::apply_visitor(ResourceUnpacking(report_types, stream_types), resource_identity.get_boost_variant());
    }
};

}} //namespace TR { namespace Core {
