// This file was generated by ODB, object-relational mapping (ORM)
// compiler for C++.
//

#ifndef ARCHIVING_POLICY_DATA_ODB_HXX
#define ARCHIVING_POLICY_DATA_ODB_HXX

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

#include "ArchivingPolicyData.hxx"

#include "BasisValues-odb.hxx"
#include "Values-odb.hxx"

#include <memory>
#include <cstddef>

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
  // ArchivingPolicyData
  //
  template <>
  struct class_traits< ::TR::Core::ArchivingPolicyData >
  {
    static const class_kind kind = class_object;
  };

  template <>
  class access::object_traits< ::TR::Core::ArchivingPolicyData >
  {
    public:
    typedef ::TR::Core::ArchivingPolicyData object_type;
    typedef ::boost::shared_ptr< ::TR::Core::ArchivingPolicyData > pointer_type;
    typedef odb::pointer_traits<pointer_type> pointer_traits;

    static const bool polymorphic = false;

    typedef ::TR::Core::ArchivingPolicyIdentity id_type;

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
  // ArchivingPolicyIdentity
  //
  template <>
  class access::composite_value_traits< ::TR::Core::ArchivingPolicyIdentity, id_mysql >
  {
    public:
    typedef ::TR::Core::ArchivingPolicyIdentity value_type;

    struct image_type
    {
      // m_subject
      //
      composite_value_traits< ::TR::Core::SourceRoleSubject, id_mysql >::image_type m_subject_value;

      // m_report_type_uuid
      //
      composite_value_traits< ::TR::Core::ReportTypeUUID, id_mysql >::image_type m_report_type_uuid_value;
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

  // ArchivingPolicyData
  //
  template <typename A>
  struct query_columns< ::TR::Core::ArchivingPolicyData, id_mysql, A >
  {
    // identity
    //
    struct identity_type_
    {
      identity_type_ ()
      {
      }

      // subject
      //
      struct subject_type_
      {
        subject_type_ ()
        {
        }

        // value1
        //
        struct value1_type_
        {
          value1_type_ ()
          {
          }

          // value
          //
          typedef
          mysql::query_column<
            mysql::value_traits<
              int,
              mysql::id_long >::query_type,
            mysql::id_long >
          value_type_;

          static const value_type_ value;
        };

        static const value1_type_ value1;

        // value2
        //
        struct value2_type_
        {
          value2_type_ ()
          {
          }

          // value
          //
          typedef
          mysql::query_column<
            mysql::value_traits<
              int,
              mysql::id_long >::query_type,
            mysql::id_long >
          value_type_;

          static const value_type_ value;
        };

        static const value2_type_ value2;
      };

      static const subject_type_ subject;

      // report_type_uuid
      //
      struct report_type_uuid_type_
      {
        report_type_uuid_type_ ()
        {
        }

        // value
        //
        typedef
        mysql::query_column<
          mysql::value_traits<
            ::boost::uuids::uuid,
            mysql::id_blob >::query_type,
          mysql::id_blob >
        value_type_;

        static const value_type_ value;
      };

      static const report_type_uuid_type_ report_type_uuid;
    };

    static const identity_type_ identity;

    // archiving_policy
    //
    typedef
    mysql::query_column<
      mysql::value_traits<
        ::TR::Core::ArchivingPolicy,
        mysql::id_long >::query_type,
      mysql::id_long >
    archiving_policy_type_;

    static const archiving_policy_type_ archiving_policy;

    // archived_expiration
    //
    typedef
    mysql::query_column<
      mysql::value_traits<
        ::TR::Core::ArchivedExpiration,
        mysql::id_long >::query_type,
      mysql::id_long >
    archived_expiration_type_;

    static const archived_expiration_type_ archived_expiration;
  };

  template <typename A>
  const typename query_columns< ::TR::Core::ArchivingPolicyData, id_mysql, A >::identity_type_::subject_type_::value1_type_::value_type_
  query_columns< ::TR::Core::ArchivingPolicyData, id_mysql, A >::identity_type_::subject_type_::value1_type_::
  value (A::table_name, "`identity_subject_value1_value`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::ArchivingPolicyData, id_mysql, A >::identity_type_::subject_type_::value1_type_
  query_columns< ::TR::Core::ArchivingPolicyData, id_mysql, A >::identity_type_::subject_type_::value1;

  template <typename A>
  const typename query_columns< ::TR::Core::ArchivingPolicyData, id_mysql, A >::identity_type_::subject_type_::value2_type_::value_type_
  query_columns< ::TR::Core::ArchivingPolicyData, id_mysql, A >::identity_type_::subject_type_::value2_type_::
  value (A::table_name, "`identity_subject_value2_value`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::ArchivingPolicyData, id_mysql, A >::identity_type_::subject_type_::value2_type_
  query_columns< ::TR::Core::ArchivingPolicyData, id_mysql, A >::identity_type_::subject_type_::value2;

  template <typename A>
  const typename query_columns< ::TR::Core::ArchivingPolicyData, id_mysql, A >::identity_type_::subject_type_
  query_columns< ::TR::Core::ArchivingPolicyData, id_mysql, A >::identity_type_::subject;

  template <typename A>
  const typename query_columns< ::TR::Core::ArchivingPolicyData, id_mysql, A >::identity_type_::report_type_uuid_type_::value_type_
  query_columns< ::TR::Core::ArchivingPolicyData, id_mysql, A >::identity_type_::report_type_uuid_type_::
  value (A::table_name, "`identity_report_type_uuid_value`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::ArchivingPolicyData, id_mysql, A >::identity_type_::report_type_uuid_type_
  query_columns< ::TR::Core::ArchivingPolicyData, id_mysql, A >::identity_type_::report_type_uuid;

  template <typename A>
  const typename query_columns< ::TR::Core::ArchivingPolicyData, id_mysql, A >::identity_type_
  query_columns< ::TR::Core::ArchivingPolicyData, id_mysql, A >::identity;

  template <typename A>
  const typename query_columns< ::TR::Core::ArchivingPolicyData, id_mysql, A >::archiving_policy_type_
  query_columns< ::TR::Core::ArchivingPolicyData, id_mysql, A >::
  archiving_policy (A::table_name, "`archiving_policy`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::ArchivingPolicyData, id_mysql, A >::archived_expiration_type_
  query_columns< ::TR::Core::ArchivingPolicyData, id_mysql, A >::
  archived_expiration (A::table_name, "`archived_expiration`", 0);

  template <typename A>
  struct pointer_query_columns< ::TR::Core::ArchivingPolicyData, id_mysql, A >:
    query_columns< ::TR::Core::ArchivingPolicyData, id_mysql, A >
  {
  };

  template <>
  class access::object_traits_impl< ::TR::Core::ArchivingPolicyData, id_mysql >:
    public access::object_traits< ::TR::Core::ArchivingPolicyData >
  {
    public:
    struct id_image_type
    {
      composite_value_traits< ::TR::Core::ArchivingPolicyIdentity, id_mysql >::image_type id_value;

      std::size_t version;
    };

    struct image_type
    {
      // m_identity
      //
      composite_value_traits< ::TR::Core::ArchivingPolicyIdentity, id_mysql >::image_type m_identity_value;

      // m_archiving_policy
      //
      int m_archiving_policy_value;
      my_bool m_archiving_policy_null;

      // m_archived_expiration
      //
      int m_archived_expiration_value;
      my_bool m_archived_expiration_null;

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

    static const std::size_t column_count = 5UL;
    static const std::size_t id_column_count = 3UL;
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
  class access::object_traits_impl< ::TR::Core::ArchivingPolicyData, id_common >:
    public access::object_traits_impl< ::TR::Core::ArchivingPolicyData, id_mysql >
  {
  };

  // ArchivingPolicyData
  //
}

#include "ArchivingPolicyData-odb.ixx"

#include <odb/post.hxx>

#endif // ARCHIVING_POLICY_DATA_ODB_HXX
