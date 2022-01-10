#pragma once
#include "Values.hxx"
#include "RegistryTrivialTypes.h"
#include "Variant.h"
namespace TR { namespace Core {
#pragma warning(push)
#pragma warning(disable:4521)
#pragma warning(disable:4068)

typedef Variant<SourceKey, RoleKey> SubjectIdentity;

#pragma db value(SubjectIdentity) definition
#pragma db value(SourceKey) definition
#pragma db value(RoleKey) definition
#pragma warning(pop)

}} //namespace TR { namespace Core {
