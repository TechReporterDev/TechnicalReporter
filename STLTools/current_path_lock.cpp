#include "stdafx.h"
#include "current_path_lock.h"
namespace stl_tools {

current_path_lock::current_path_lock(boost::filesystem::path path)
{
    m_prev_path = boost::filesystem::current_path();
    boost::filesystem::current_path(path);
}

current_path_lock::~current_path_lock()
{
    boost::filesystem::current_path(m_prev_path);
}

} //namespace stl_tools {