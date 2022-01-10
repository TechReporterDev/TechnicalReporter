// This file was generated by ODB, object-relational mapping (ORM)
// compiler for C++.
//

#ifndef CURRENT_REPORT_DATA_ODB_HXX
#define CURRENT_REPORT_DATA_ODB_HXX

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

#include "CurrentReportData.hxx"

#include "BasisValues-odb.hxx"
#include "RegistryValues-odb.hxx"
#include "Values-odb.hxx"

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
  // CurrentReportData
  //
  template <>
  struct class_traits< ::TR::Core::CurrentReportData >
  {
    static const class_kind kind = class_object;
  };

  template <>
  class access::object_traits< ::TR::Core::CurrentReportData >
  {
    public:
    typedef ::TR::Core::CurrentReportData object_type;
    typedef ::boost::shared_ptr< ::TR::Core::CurrentReportData > pointer_type;
    typedef odb::pointer_traits<pointer_type> pointer_traits;

    static const bool polymorphic = false;

    typedef ::TR::Core::CurrentReportIdentity id_type;

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

  // CurrentReportContentData
  //
  template <>
  struct class_traits< ::TR::Core::CurrentReportContentData >
  {
    static const class_kind kind = class_object;
  };

  template <>
  class access::object_traits< ::TR::Core::CurrentReportContentData >
  {
    public:
    typedef ::TR::Core::CurrentReportContentData object_type;
    typedef ::boost::shared_ptr< ::TR::Core::CurrentReportContentData > pointer_type;
    typedef odb::pointer_traits<pointer_type> pointer_traits;

    static const bool polymorphic = false;

    typedef ::TR::Core::CurrentReportIdentity id_type;

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
  // CurrentReportIdentity
  //
  template <>
  class access::composite_value_traits< ::TR::Core::CurrentReportIdentity, id_mysql >
  {
    public:
    typedef ::TR::Core::CurrentReportIdentity value_type;

    struct image_type
    {
      // m_source_key
      //
      composite_value_traits< ::TR::Core::SourceKey, id_mysql >::image_type m_source_key_value;

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

    static const std::size_t column_count = 2UL;
  };

  // TransformationCircumstancesData
  //
  template <>
  class access::composite_value_traits< ::TR::Core::TransformationCircumstancesData, id_mysql >
  {
    public:
    typedef ::TR::Core::TransformationCircumstancesData value_type;

    struct image_type
    {
      // m_transformation_uuid
      //
      details::buffer m_transformation_uuid_value;
      unsigned long m_transformation_uuid_size;
      my_bool m_transformation_uuid_null;

      // m_setup_uuid
      //
      details::buffer m_setup_uuid_value;
      unsigned long m_setup_uuid_size;
      my_bool m_setup_uuid_null;
    };

    // m_input_uuids
    //
    struct input_uuids_traits
    {
      typedef ::std::vector< ::boost::uuids::uuid > container_type;
      typedef
      odb::access::container_traits<container_type>
      container_traits_type;
      typedef container_traits_type::index_type index_type;
      typedef container_traits_type::value_type value_type;

      typedef ordered_functions<index_type, value_type> functions_type;
      typedef mysql::container_statements< input_uuids_traits > statements_type;

      struct data_image_type
      {
        // index
        //
        unsigned long long index_value;
        my_bool index_null;

        // value
        //
        details::buffer value_value;
        unsigned long value_size;
        my_bool value_null;

        std::size_t version;
      };

      static void
      bind (MYSQL_BIND*,
            const MYSQL_BIND* id,
            std::size_t id_size,
            data_image_type&);

      static void
      grow (data_image_type&,
            my_bool*);

      static void
      init (data_image_type&,
            index_type*,
            const value_type&);

      static void
      init (index_type&,
            value_type&,
            const data_image_type&,
            database*);

      static void
      insert (index_type, const value_type&, void*);

      static bool
      select (index_type&, value_type&, void*);

      static void
      delete_ (void*);

      static void
      persist (const container_type&,
               statements_type&);

      static void
      load (container_type&,
            statements_type&);

      static void
      update (const container_type&,
              statements_type&);

      static void
      erase (statements_type&);
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

    static const std::size_t column_count = 2UL;
  };

  // CurrentReportData
  //
  template <typename A>
  struct query_columns< ::TR::Core::CurrentReportData, id_mysql, A >
  {
    // identity
    //
    struct identity_class_
    {
      identity_class_ ()
      {
      }

      // source_key
      //
      struct source_key_class_1_
      {
        source_key_class_1_ ()
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

      static const source_key_class_1_ source_key;

      // report_type_uuid
      //
      struct report_type_uuid_class_1_
      {
        report_type_uuid_class_1_ ()
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

      static const report_type_uuid_class_1_ report_type_uuid;
    };

    static const identity_class_ identity;

    // diff
    //
    typedef
    mysql::query_column<
      mysql::value_traits<
        ::std::vector< char >,
        mysql::id_blob >::query_type,
      mysql::id_blob >
    diff_type_;

    static const diff_type_ diff;

    // time
    //
    typedef
    mysql::query_column<
      mysql::value_traits<
        ::time_t,
        mysql::id_longlong >::query_type,
      mysql::id_longlong >
    time_type_;

    static const time_type_ time;

    // check_time
    //
    typedef
    mysql::query_column<
      mysql::value_traits<
        ::time_t,
        mysql::id_longlong >::query_type,
      mysql::id_longlong >
    check_time_type_;

    static const check_time_type_ check_time;

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

    // transformation_circumstances
    //
    struct transformation_circumstances_class_
    {
      transformation_circumstances_class_ ()
      {
      }

      // transformation_uuid
      //
      typedef
      mysql::query_column<
        mysql::value_traits<
          ::TR::Core::UUID,
          mysql::id_blob >::query_type,
        mysql::id_blob >
      transformation_uuid_type_;

      static const transformation_uuid_type_ transformation_uuid;

      // setup_uuid
      //
      typedef
      mysql::query_column<
        mysql::value_traits<
          ::TR::Core::UUID,
          mysql::id_blob >::query_type,
        mysql::id_blob >
      setup_uuid_type_;

      static const setup_uuid_type_ setup_uuid;
    };

    static const transformation_circumstances_class_ transformation_circumstances;
  };

  template <typename A>
  const typename query_columns< ::TR::Core::CurrentReportData, id_mysql, A >::identity_class_::source_key_class_1_::value_type_
  query_columns< ::TR::Core::CurrentReportData, id_mysql, A >::identity_class_::source_key_class_1_::
  value (A::table_name, "`identity_source_key_value`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::CurrentReportData, id_mysql, A >::identity_class_::source_key_class_1_
  query_columns< ::TR::Core::CurrentReportData, id_mysql, A >::identity_class_::source_key;

  template <typename A>
  const typename query_columns< ::TR::Core::CurrentReportData, id_mysql, A >::identity_class_::report_type_uuid_class_1_::value_type_
  query_columns< ::TR::Core::CurrentReportData, id_mysql, A >::identity_class_::report_type_uuid_class_1_::
  value (A::table_name, "`identity_report_type_uuid_value`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::CurrentReportData, id_mysql, A >::identity_class_::report_type_uuid_class_1_
  query_columns< ::TR::Core::CurrentReportData, id_mysql, A >::identity_class_::report_type_uuid;

  template <typename A>
  const typename query_columns< ::TR::Core::CurrentReportData, id_mysql, A >::identity_class_
  query_columns< ::TR::Core::CurrentReportData, id_mysql, A >::identity;

  template <typename A>
  const typename query_columns< ::TR::Core::CurrentReportData, id_mysql, A >::diff_type_
  query_columns< ::TR::Core::CurrentReportData, id_mysql, A >::
  diff (A::table_name, "`diff`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::CurrentReportData, id_mysql, A >::time_type_
  query_columns< ::TR::Core::CurrentReportData, id_mysql, A >::
  time (A::table_name, "`time`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::CurrentReportData, id_mysql, A >::check_time_type_
  query_columns< ::TR::Core::CurrentReportData, id_mysql, A >::
  check_time (A::table_name, "`check_time`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::CurrentReportData, id_mysql, A >::uuid_type_
  query_columns< ::TR::Core::CurrentReportData, id_mysql, A >::
  uuid (A::table_name, "`uuid`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::CurrentReportData, id_mysql, A >::transformation_circumstances_class_::transformation_uuid_type_
  query_columns< ::TR::Core::CurrentReportData, id_mysql, A >::transformation_circumstances_class_::
  transformation_uuid (A::table_name, "`transformation_circumstances_transformation_uuid`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::CurrentReportData, id_mysql, A >::transformation_circumstances_class_::setup_uuid_type_
  query_columns< ::TR::Core::CurrentReportData, id_mysql, A >::transformation_circumstances_class_::
  setup_uuid (A::table_name, "`transformation_circumstances_setup_uuid`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::CurrentReportData, id_mysql, A >::transformation_circumstances_class_
  query_columns< ::TR::Core::CurrentReportData, id_mysql, A >::transformation_circumstances;

  template <typename A>
  struct pointer_query_columns< ::TR::Core::CurrentReportData, id_mysql, A >:
    query_columns< ::TR::Core::CurrentReportData, id_mysql, A >
  {
  };

  template <>
  class access::object_traits_impl< ::TR::Core::CurrentReportData, id_mysql >:
    public access::object_traits< ::TR::Core::CurrentReportData >
  {
    public:
    struct id_image_type
    {
      composite_value_traits< ::TR::Core::CurrentReportIdentity, id_mysql >::image_type id_value;

      std::size_t version;
    };

    struct image_type
    {
      // m_identity
      //
      composite_value_traits< ::TR::Core::CurrentReportIdentity, id_mysql >::image_type m_identity_value;

      // m_diff
      //
      details::buffer m_diff_value;
      unsigned long m_diff_size;
      my_bool m_diff_null;

      // m_time
      //
      long long m_time_value;
      my_bool m_time_null;

      // m_check_time
      //
      long long m_check_time_value;
      my_bool m_check_time_null;

      // m_uuid
      //
      details::buffer m_uuid_value;
      unsigned long m_uuid_size;
      my_bool m_uuid_null;

      // m_transformation_circumstances
      //
      composite_value_traits< ::TR::Core::TransformationCircumstancesData, id_mysql >::image_type m_transformation_circumstances_value;

      std::size_t version;
    };

    struct extra_statement_cache_type;

    // m_input_uuids
    //
    struct transformation_circumstances_input_uuids_traits: access::composite_value_traits< ::TR::Core::TransformationCircumstancesData, id_mysql >::input_uuids_traits
    {
      static const std::size_t id_column_count = 2UL;
      static const std::size_t data_column_count = 4UL;

      static const bool versioned = false;

      static const char insert_statement[];
      static const char select_statement[];
      static const char delete_statement[];
    };

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

    static const std::size_t column_count = 8UL;
    static const std::size_t id_column_count = 2UL;
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
  class access::object_traits_impl< ::TR::Core::CurrentReportData, id_common >:
    public access::object_traits_impl< ::TR::Core::CurrentReportData, id_mysql >
  {
  };

  // CurrentReportContentData
  //
  template <typename A>
  struct query_columns< ::TR::Core::CurrentReportContentData, id_mysql, A >
  {
    // identity
    //
    struct identity_class_
    {
      identity_class_ ()
      {
      }

      // source_key
      //
      struct source_key_class_1_
      {
        source_key_class_1_ ()
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

      static const source_key_class_1_ source_key;

      // report_type_uuid
      //
      struct report_type_uuid_class_1_
      {
        report_type_uuid_class_1_ ()
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

      static const report_type_uuid_class_1_ report_type_uuid;
    };

    static const identity_class_ identity;

    // content
    //
    typedef
    mysql::query_column<
      mysql::value_traits<
        ::TR::Core::Blob,
        mysql::id_blob >::query_type,
      mysql::id_blob >
    content_type_;

    static const content_type_ content;
  };

  template <typename A>
  const typename query_columns< ::TR::Core::CurrentReportContentData, id_mysql, A >::identity_class_::source_key_class_1_::value_type_
  query_columns< ::TR::Core::CurrentReportContentData, id_mysql, A >::identity_class_::source_key_class_1_::
  value (A::table_name, "`identity_source_key_value`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::CurrentReportContentData, id_mysql, A >::identity_class_::source_key_class_1_
  query_columns< ::TR::Core::CurrentReportContentData, id_mysql, A >::identity_class_::source_key;

  template <typename A>
  const typename query_columns< ::TR::Core::CurrentReportContentData, id_mysql, A >::identity_class_::report_type_uuid_class_1_::value_type_
  query_columns< ::TR::Core::CurrentReportContentData, id_mysql, A >::identity_class_::report_type_uuid_class_1_::
  value (A::table_name, "`identity_report_type_uuid_value`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::CurrentReportContentData, id_mysql, A >::identity_class_::report_type_uuid_class_1_
  query_columns< ::TR::Core::CurrentReportContentData, id_mysql, A >::identity_class_::report_type_uuid;

  template <typename A>
  const typename query_columns< ::TR::Core::CurrentReportContentData, id_mysql, A >::identity_class_
  query_columns< ::TR::Core::CurrentReportContentData, id_mysql, A >::identity;

  template <typename A>
  const typename query_columns< ::TR::Core::CurrentReportContentData, id_mysql, A >::content_type_
  query_columns< ::TR::Core::CurrentReportContentData, id_mysql, A >::
  content (A::table_name, "`content`", 0);

  template <typename A>
  struct pointer_query_columns< ::TR::Core::CurrentReportContentData, id_mysql, A >:
    query_columns< ::TR::Core::CurrentReportContentData, id_mysql, A >
  {
  };

  template <>
  class access::object_traits_impl< ::TR::Core::CurrentReportContentData, id_mysql >:
    public access::object_traits< ::TR::Core::CurrentReportContentData >
  {
    public:
    struct id_image_type
    {
      composite_value_traits< ::TR::Core::CurrentReportIdentity, id_mysql >::image_type id_value;

      std::size_t version;
    };

    struct image_type
    {
      // m_identity
      //
      composite_value_traits< ::TR::Core::CurrentReportIdentity, id_mysql >::image_type m_identity_value;

      // m_content
      //
      details::buffer m_content_value;
      unsigned long m_content_size;
      my_bool m_content_null;

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

    static const std::size_t column_count = 3UL;
    static const std::size_t id_column_count = 2UL;
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
  class access::object_traits_impl< ::TR::Core::CurrentReportContentData, id_common >:
    public access::object_traits_impl< ::TR::Core::CurrentReportContentData, id_mysql >
  {
  };

  // CurrentReportData
  //
  // CurrentReportContentData
  //
}

#include "CurrentReportData-odb.ixx"

#include <odb/post.hxx>

#endif // CURRENT_REPORT_DATA_ODB_HXX