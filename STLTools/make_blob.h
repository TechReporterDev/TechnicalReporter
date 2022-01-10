#pragma once
#include <boost\serialization\serialization.hpp>
#include <boost\serialization\vector.hpp>
#include <boost\serialization\utility.hpp>
#include <boost\archive\binary_iarchive.hpp>
#include <boost\archive\binary_oarchive.hpp>
#include <boost\iostreams\stream.hpp>

#pragma warning(push)
#pragma warning(disable:4244)
#include "boost\example\container_device.hpp"
#pragma warning(pop)

namespace stl_tools {

namespace boost_ios = boost::iostreams;
typedef std::vector<char> blob;
typedef boost::iostreams::example::container_device<blob> blob_sink;
typedef boost::iostreams::example::container_device<blob> blob_source;
typedef boost::archive::binary_oarchive binary_oarchive;
typedef boost::archive::binary_iarchive binary_iarchive;

template<class Archive, class Arg>
void serialize(Archive& archive, const Arg& arg)
{
    archive << arg;
}

template<class Archive, class Arg, class... Args>
void serialize(Archive& archive, const Arg& arg, const Args&... args)
{
    serialize(archive, arg);
    serialize(archive, args...);
}

template<class... Args>
blob make_blob(const Args&... args)
{
    blob data;
    boost_ios::stream<blob_sink> data_strm(data);
    binary_oarchive archive(data_strm);
    serialize(archive, args...);    
    data_strm.flush();
    return data;
}

template<class Archive, class Arg>
void unserialize(Archive& archive, Arg& arg)
{
    archive >> arg;
}

template<class Archive, class Arg, class... Args>
void unserialize(Archive& archive, Arg& arg, Args&... args)
{
    unserialize(archive, arg);
    unserialize(archive, args...);
}

template<class... Args>
void read_blob(const blob& bb, Args&... args)
{
    boost_ios::stream<blob_source> data_strm(const_cast<blob&>(bb));
    binary_iarchive archive(data_strm);
    unserialize(archive, args...);  
}

} //namespace stl_tools {