// This file was generated by ODB, object-relational mapping (ORM)
// compiler for C++.
//

#ifndef FEATURE_DATA_ODB_HXX
#define FEATURE_DATA_ODB_HXX

// Begin prologue.
//
#include <odb/boost/version.hxx>
#if ODB_BOOST_VERSION != 2040000 // 2.4.0
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

#include <odb/version.hxx>

#if (ODB_VERSION != 20400UL)
#error ODB runtime version mismatch
#endif

#include <odb/pre.hxx>

#include "FeatureData.hxx"

#include <memory>
#include <cstddef>
#include <utility>

#include <odb/core.hxx>
#include <odb/traits.hxx>
#include <odb/callback.hxx>
#include <odb/wrapper-traits.hxx>
#include <odb/pointer-traits.hxx>
#ifdef BOOST_TR1_MEMORY_HPP_INCLUDED
#  include <odb/tr1/wrapper-traits.hxx>
#  include <odb/tr1/pointer-traits.hxx>
#endif
#include <odb/container-traits.hxx>
#include <odb/no-op-cache-traits.hxx>
#include <odb/result.hxx>
#include <odb/simple-object-result.hxx>

#include <odb/details/unused.hxx>
#include <odb/details/shared-ptr.hxx>

namespace odb
{
  // FeatureData
  //
  template <>
  struct class_traits< ::TR::Core::FeatureData >
  {
    static const class_kind kind = class_object;
  };

  template <>
  class access::object_traits< ::TR::Core::FeatureData >
  {
    public:
    typedef ::TR::Core::FeatureData object_type;
    typedef ::boost::shared_ptr< ::TR::Core::FeatureData > pointer_type;
    typedef odb::pointer_traits<pointer_type> pointer_traits;

    static const bool polymorphic = false;

    typedef ::TR::Core::UUID id_type;

    static const bool auto_id = false;

    static const bool abstract = false;

    static id_type
    id (const object_type&);

    typedef
    no_op_pointer_cache_traits<pointer_type>
    pointer_cache_traits;

    typedef
    no_op_reference_cache_traits<object_type>
    reference_cache_traits;

    static void
    callback (database&, object_type&, callback_event);

    static void
    callback (database&, const object_type&, callback_event);
  };
}

#include <odb/details/buffer.hxx>

#include <odb/mysql/version.hxx>
#include <odb/mysql/forward.hxx>
#include <odb/mysql/binding.hxx>
#include <odb/mysql/mysql-types.hxx>
#include <odb/mysql/query.hxx>

namespace odb
{
  // FeatureData
  //
  template <typename A>
  struct query_columns< ::TR::Core::FeatureData, id_mysql, A >
  {
    // uuid
    //
    typedef
    mysql::query_column<
      mysql::value_traits<
        ::TR::Core::UUID,
        mysql::id_blob >::query_type,
      mysql::id_blob >
    uuid_type_;

    static const uuid_type_ uuid;

    // installed
    //
    typedef
    mysql::query_column<
      mysql::value_traits<
        bool,
        mysql::id_tiny >::query_type,
      mysql::id_tiny >
    installed_type_;

    static const installed_type_ installed;
  };

  template <typename A>
  const typename query_columns< ::TR::Core::FeatureData, id_mysql, A >::uuid_type_
  query_columns< ::TR::Core::FeatureData, id_mysql, A >::
  uuid (A::table_name, "`uuid`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::FeatureData, id_mysql, A >::installed_type_
  query_columns< ::TR::Core::FeatureData, id_mysql, A >::
  installed (A::table_name, "`installed`", 0);

  template <typename A>
  struct pointer_query_columns< ::TR::Core::FeatureData, id_mysql, A >:
    query_columns< ::TR::Core::FeatureData, id_mysql, A >
  {
  };

  template <>
  class access::object_traits_impl< ::TR::Core::FeatureData, id_mysql >:
    public access::object_traits< ::TR::Core::FeatureData >
  {
    public:
    struct id_image_type
    {
      details::buffer id_value;
      unsigned long id_size;
      my_bool id_null;

      std::size_t version;
    };

    struct image_type
    {
      // m_uuid
      //
      details::buffer m_uuid_value;
      unsigned long m_uuid_size;
      my_bool m_uuid_null;

      // m_installed
      //
      signed char m_installed_value;
      my_bool m_installed_null;

      std::size_t version;
    };

    struct extra_statement_cache_type;

    using object_traits<object_type>::id;

    static id_type
    id (const image_type&);

    static bool
    grow (image_type&,
          my_bool*);

    static void
    bind (MYSQL_BIND*,
          image_type&,
          mysql::statement_kind);

    static void
    bind (MYSQL_BIND*, id_image_type&);

    static bool
    init (image_type&,
          const object_type&,
          mysql::statement_kind);

    static void
    init (object_type&,
          const image_type&,
          database*);

    static void
    init (id_image_type&, const id_type&);

    typedef mysql::object_statements<object_type> statements_type;

    typedef mysql::query_base query_base_type;

    static const std::size_t column_count = 2UL;
    static const std::size_t id_column_count = 1UL;
    static const std::size_t inverse_column_count = 0UL;
    static const std::size_t readonly_column_count = 0UL;
    static const std::size_t managed_optimistic_column_count = 0UL;

    static const std::size_t separate_load_column_count = 0UL;
    static const std::size_t separate_update_column_count = 0UL;

    static const bool versioned = false;

    static const char persist_statement[];
    static const char find_statement[];
    static const char update_statement[];
    static const char erase_statement[];
    static const char query_statement[];
    static const char erase_query_statement[];

    static const char table_name[];

    static void
    persist (database&, const object_type&);

    static pointer_type
    find (database&, const id_type&);

    static bool
    find (database&, const id_type&, object_type&);

    static bool
    reload (database&, object_type&);

    static void
    update (database&, const object_type&);

    static void
    erase (database&, const id_type&);

    static void
    erase (database&, const object_type&);

    static result<object_type>
    query (database&, const query_base_type&);

    static unsigned long long
    erase_query (database&, const query_base_type&);

    public:
    static bool
    find_ (statements_type&,
           const id_type*);

    static void
    load_ (statements_type&,
           object_type&,
           bool reload);
  };

  template <>
  class access::object_traits_impl< ::TR::Core::FeatureData, id_common >:
    public access::object_traits_impl< ::TR::Core::FeatureData, id_mysql >
  {
  };

  // FeatureData
  //
}

#include "FeatureData-odb.ixx"

#include <odb/post.hxx>

#endif // FEATURE_DATA_ODB_HXX
