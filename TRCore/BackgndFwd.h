#pragma once
#include "odb/forward.hxx"
namespace TR { namespace Core {

extern const wchar_t*   default_schema;
typedef odb::database Database;
class Transaction;
struct Executive;
class Executor;
struct RoutineRunner;

template <class _ID, class... _Routines>
class RoutineManager;

class Job;

template <class StrandID>
class JobManager;

struct SMTP;
}} //namespace TR { namespace Core {