#pragma once
#include <boost\filesystem.hpp>
namespace stl_tools {

class current_path_lock
{
public:
    current_path_lock(boost::filesystem::path path);
    ~current_path_lock();

    current_path_lock(const current_path_lock&) = delete;
    current_path_lock& operator = (const current_path_lock&) = delete;

private:
    boost::filesystem::path m_prev_path;
};

} //namespace stl_tools {