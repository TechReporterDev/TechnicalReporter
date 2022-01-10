#include "stdafx.h"
#include "BasisPacking.h"
#include "Basis.h"
namespace TR { namespace Core {

ReportTypeUUID Packing<ReportTypeRef>::pack(ReportTypeRef report_type_ref)
{
    return report_type_ref.get_uuid();
}

ReportTypeRef Unpacking<ReportTypeUUID>::unpack(ReportTypeUUID report_type_uuid, const ReportTypes* report_types)
{
    return ReportTypeRef(report_types, report_type_uuid);
}

StreamTypeUUID Packing<StreamTypeRef>::pack(StreamTypeRef stream_type_ref)
{
    return stream_type_ref.get_uuid();
}

StreamTypeRef Unpacking<StreamTypeUUID>::unpack(StreamTypeUUID stream_type_uuid, const StreamTypes* stream_types)
{
    return StreamTypeRef(stream_types, stream_type_uuid);
}

SourceTypeUUID Packing<SourceTypeRef>::pack(SourceTypeRef source_type_ref)
{
    return SourceTypeUUID(source_type_ref.get_uuid());
}

SourceTypeRef Unpacking<SourceTypeUUID>::unpack(SourceTypeUUID source_type_uuid, const SourceTypes* source_types)
{
    return SourceTypeRef(source_types, source_type_uuid);
}

ActionUUID Packing<ActionRef>::pack(ActionRef action_ref)
{
    return action_ref.get_uuid();
}

ActionRef Unpacking<ActionUUID>::unpack(ActionUUID action_uuid, const Actions* actions)
{
    return ActionRef(actions, action_uuid);
}

}} //namespace TR { namespace Core {