#pragma once
#include "Values.hxx"
#include "BasisValues.hxx"
#include "AppTrivialTypes.h"
#include "Variant.h"
namespace TR { namespace Core {
#pragma warning(push)
#pragma warning(disable:4521)
#pragma warning(disable:4068)

typedef Variant<PeriodicReloadIdentity, PeriodicLaunchIdentity> PeriodicOperationVariant;

#pragma db value(PeriodicReloadIdentity) definition
#pragma db value(PeriodicLaunchIdentity) definition
#pragma db value(PeriodicOperationVariant) definition
#pragma warning(pop)

}} //namespace TR { namespace Core {
