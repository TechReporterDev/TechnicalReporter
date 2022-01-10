#pragma once
#include "Values.hxx"
#include "BasisTrivialTypes.h"
#include "Variant.h"
namespace TR { namespace Core {
#pragma warning(push)
#pragma warning(disable:4521)
#pragma warning(disable:4068)

typedef Variant<ReportTypeUUID, StreamTypeUUID> ResourceIdentity;

#pragma db value(ResourceIdentity) definition
#pragma db value(ReportTypeUUID) definition
#pragma db value(StreamTypeUUID) definition
#pragma db value(SourceTypeUUID) definition
#pragma db value(ActionUUID) definition

#pragma warning(pop)

}} //namespace TR { namespace Core {
