// This file was generated by ODB, object-relational mapping (ORM)
// compiler for C++.
//

#ifndef MAPPING_ODB_HXX
#define MAPPING_ODB_HXX

#include <odb/version.hxx>

#if (ODB_VERSION != 20300UL)
#error ODB runtime version mismatch
#endif

#include <odb/pre.hxx>

// Begin prologue.
//
#include <odb/boost/version.hxx>
#if ODB_BOOST_VERSION != 2030000 // 2.3.0
#  error ODB and C++ compilers see different libodb-boost interface versions
#endif
#include <boost/shared_ptr.hpp>
#include <odb/boost/smart-ptr/pointer-traits.hxx>
#include <odb/boost/smart-ptr/wrapper-traits.hxx>
#include <odb/boost/optional/wrapper-traits.hxx>
#include <odb/boost/unordered/container-traits.hxx>
#include <odb/boost/date-time/mysql/gregorian-traits.hxx>
#include <odb/boost/date-time/mysql/posix-time-traits.hxx>
#include <odb/boost/multi-index/container-traits.hxx>
#include <odb/boost/uuid/mysql/uuid-traits.hxx>
//
// End prologue.

#include "Mapping.hxx"

#include <memory>
#include <cstddef>

#include <odb/core.hxx>
#include <odb/traits.hxx>
#include <odb/callback.hxx>
#include <odb/wrapper-traits.hxx>
#include <odb/pointer-traits.hxx>
#include <odb/container-traits.hxx>
#include <odb/no-op-cache-traits.hxx>
#include <odb/result.hxx>

#include <odb/details/unused.hxx>
#include <odb/details/shared-ptr.hxx>

namespace odb
{
}

#include <odb/details/buffer.hxx>

#include <odb/mysql/version.hxx>
#include <odb/mysql/forward.hxx>
#include <odb/mysql/binding.hxx>
#include <odb/mysql/mysql-types.hxx>
#include <odb/mysql/query.hxx>

namespace odb
{
  // XmlFilterDocText
  //
  template <>
  class access::composite_value_traits< ::TR::Core::XmlFilterDocText, id_mysql >
  {
    public:
    typedef ::TR::Core::XmlFilterDocText value_type;

    struct image_type
    {
      // m_text
      //
      details::buffer m_text_value;
      unsigned long m_text_size;
      my_bool m_text_null;
    };

    static bool
    grow (image_type&,
          my_bool*);

    static void
    bind (MYSQL_BIND*,
          image_type&,
          mysql::statement_kind);

    static bool
    init (image_type&,
          const value_type&,
          mysql::statement_kind);

    static void
    init (value_type&,
          const image_type&,
          database*);

    static bool
    get_null (const image_type&);

    static void
    set_null (image_type&,
              mysql::statement_kind);
  };

  // XmlDefDocText
  //
  template <>
  class access::composite_value_traits< ::TR::Core::XmlDefDocText, id_mysql >
  {
    public:
    typedef ::TR::Core::XmlDefDocText value_type;

    struct image_type
    {
      // m_text
      //
      details::buffer m_text_value;
      unsigned long m_text_size;
      my_bool m_text_null;
    };

    static bool
    grow (image_type&,
          my_bool*);

    static void
    bind (MYSQL_BIND*,
          image_type&,
          mysql::statement_kind);

    static bool
    init (image_type&,
          const value_type&,
          mysql::statement_kind);

    static void
    init (value_type&,
          const image_type&,
          database*);

    static bool
    get_null (const image_type&);

    static void
    set_null (image_type&,
              mysql::statement_kind);
  };
}

#include "Mapping-odb.ixx"

#include <odb/post.hxx>

#endif // MAPPING_ODB_HXX
