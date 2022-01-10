#pragma once
#include "stddecl.h"
namespace TR { namespace External {

Blob exec_command(const std::wstring& file_system_path, const Blob& input);
std::string read_text(const std::wstring& file_system_path);

}} //namespace TR { namespace External {