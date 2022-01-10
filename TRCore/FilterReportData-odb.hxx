// This file was generated by ODB, object-relational mapping (ORM)
// compiler for C++.
//

#ifndef FILTER_REPORT_DATA_ODB_HXX
#define FILTER_REPORT_DATA_ODB_HXX

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

#include "FilterReportData.hxx"

#include "BasisValues-odb.hxx"
#include "ServicesValues-odb.hxx"
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
  // FilterReportData
  //
  template <>
  struct class_traits< ::TR::Core::FilterReportData >
  {
    static const class_kind kind = class_object;
  };

  template <>
  class access::object_traits< ::TR::Core::FilterReportData >
  {
    public:
    typedef ::TR::Core::FilterReportData object_type;
    typedef ::boost::shared_ptr< ::TR::Core::FilterReportData > pointer_type;
    typedef odb::pointer_traits<pointer_type> pointer_traits;

    static const bool polymorphic = false;

    typedef ::TR::Core::FilterReportUUID id_type;

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

  // FilterReportStatusData
  //
  template <>
  struct class_traits< ::TR::Core::FilterReportStatusData >
  {
    static const class_kind kind = class_object;
  };

  template <>
  class access::object_traits< ::TR::Core::FilterReportStatusData >
  {
    public:
    typedef ::TR::Core::FilterReportStatusData object_type;
    typedef ::boost::shared_ptr< ::TR::Core::FilterReportStatusData > pointer_type;
    typedef odb::pointer_traits<pointer_type> pointer_traits;

    static const bool polymorphic = false;

    typedef ::TR::Core::FilterReportStatusIdentity id_type;

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
  // FilterReportData
  //
  template <typename A>
  struct query_columns< ::TR::Core::FilterReportData, id_mysql, A >
  {
    // filter_report_uuid
    //
    struct filter_report_uuid_type_
    {
      filter_report_uuid_type_ ()
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

    static const filter_report_uuid_type_ filter_report_uuid;

    // name
    //
    typedef
    mysql::query_column<
      mysql::value_traits<
        ::std::string,
        mysql::id_string >::query_type,
      mysql::id_string >
    name_type_;

    static const name_type_ name;

    // input_def
    //
    struct input_def_type_
    {
      input_def_type_ ()
      {
      }

      // value
      //
      typedef
      mysql::query_column<
        mysql::value_traits<
          ::std::basic_string< char >,
          mysql::id_string >::query_type,
        mysql::id_string >
      value_type_;

      static const value_type_ value;
    };

    static const input_def_type_ input_def;
  };

  template <typename A>
  const typename query_columns< ::TR::Core::FilterReportData, id_mysql, A >::filter_report_uuid_type_::value_type_
  query_columns< ::TR::Core::FilterReportData, id_mysql, A >::filter_report_uuid_type_::
  value (A::table_name, "`filter_report_uuid_value`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::FilterReportData, id_mysql, A >::filter_report_uuid_type_
  query_columns< ::TR::Core::FilterReportData, id_mysql, A >::filter_report_uuid;

  template <typename A>
  const typename query_columns< ::TR::Core::FilterReportData, id_mysql, A >::name_type_
  query_columns< ::TR::Core::FilterReportData, id_mysql, A >::
  name (A::table_name, "`name`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::FilterReportData, id_mysql, A >::input_def_type_::value_type_
  query_columns< ::TR::Core::FilterReportData, id_mysql, A >::input_def_type_::
  value (A::table_name, "`input_def_value`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::FilterReportData, id_mysql, A >::input_def_type_
  query_columns< ::TR::Core::FilterReportData, id_mysql, A >::input_def;

  template <typename A>
  struct pointer_query_columns< ::TR::Core::FilterReportData, id_mysql, A >:
    query_columns< ::TR::Core::FilterReportData, id_mysql, A >
  {
  };

  template <>
  class access::object_traits_impl< ::TR::Core::FilterReportData, id_mysql >:
    public access::object_traits< ::TR::Core::FilterReportData >
  {
    public:
    struct id_image_type
    {
      composite_value_traits< ::TR::Core::FilterReportUUID, id_mysql >::image_type id_value;

      std::size_t version;
    };

    struct image_type
    {
      // m_filter_report_uuid
      //
      composite_value_traits< ::TR::Core::FilterReportUUID, id_mysql >::image_type m_filter_report_uuid_value;

      // m_name
      //
      details::buffer m_name_value;
      unsigned long m_name_size;
      my_bool m_name_null;

      // m_input_def
      //
      composite_value_traits< ::TR::Core::XmlDefDocText, id_mysql >::image_type m_input_def_value;

      std::size_t version;
    };

    struct extra_statement_cache_type;

    // m_input_uuids
    //
    struct input_uuids_traits
    {
      static const std::size_t id_column_count = 1UL;
      static const std::size_t data_column_count = 3UL;

      static const bool versioned = false;

      static const char insert_statement[];
      static const char select_statement[];
      static const char delete_statement[];

      typedef ::std::vector< ::TR::Core::InputUUID > container_type;
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
        composite_value_traits< value_type, id_mysql >::image_type value_value;

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
  class access::object_traits_impl< ::TR::Core::FilterReportData, id_common >:
    public access::object_traits_impl< ::TR::Core::FilterReportData, id_mysql >
  {
  };

  // FilterReportStatusIdentity
  //
  template <>
  class access::composite_value_traits< ::TR::Core::FilterReportStatusIdentity, id_mysql >
  {
    public:
    typedef ::TR::Core::FilterReportStatusIdentity value_type;

    struct image_type
    {
      // m_subject
      //
      composite_value_traits< ::TR::Core::FilterReportSubject, id_mysql >::image_type m_subject_value;

      // m_filter_report_uuid
      //
      composite_value_traits< ::TR::Core::FilterReportUUID, id_mysql >::image_type m_filter_report_uuid_value;
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

  // FilterReportStatusData
  //
  template <typename A>
  struct query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >
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

      // filter_report_uuid
      //
      struct filter_report_uuid_type_
      {
        filter_report_uuid_type_ ()
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

      static const filter_report_uuid_type_ filter_report_uuid;
    };

    static const identity_type_ identity;

    // filter_xml
    //
    struct filter_xml_type_
    {
      filter_xml_type_ ()
      {
      }

      // value
      //
      typedef
      mysql::query_column<
        mysql::value_traits<
          ::std::basic_string< char >,
          mysql::id_string >::query_type,
        mysql::id_string >
      value_type_;

      static const value_type_ value;
    };

    static const filter_xml_type_ filter_xml;

    // enable_policy
    //
    typedef
    mysql::query_column<
      mysql::value_traits<
        ::TR::Core::EnableReportPolicy,
        mysql::id_enum >::query_type,
      mysql::id_enum >
    enable_policy_type_;

    static const enable_policy_type_ enable_policy;

    // effective_filter_xml
    //
    struct effective_filter_xml_type_
    {
      effective_filter_xml_type_ ()
      {
      }

      // value
      //
      typedef
      mysql::query_column<
        mysql::value_traits<
          ::std::basic_string< char >,
          mysql::id_string >::query_type,
        mysql::id_string >
      value_type_;

      static const value_type_ value;
    };

    static const effective_filter_xml_type_ effective_filter_xml;

    // effective_enable_policy
    //
    typedef
    mysql::query_column<
      mysql::value_traits<
        ::TR::Core::EnableReportPolicy,
        mysql::id_enum >::query_type,
      mysql::id_enum >
    effective_enable_policy_type_;

    static const effective_enable_policy_type_ effective_enable_policy;

    // base_enable_policy_override
    //
    typedef
    mysql::query_column<
      mysql::value_traits<
        ::TR::Core::EnableReportPolicy,
        mysql::id_enum >::query_type,
      mysql::id_enum >
    base_enable_policy_override_type_;

    static const base_enable_policy_override_type_ base_enable_policy_override;

    // filtrate_uuid
    //
    struct filtrate_uuid_type_
    {
      filtrate_uuid_type_ ()
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

    static const filtrate_uuid_type_ filtrate_uuid;

    // filtrate_def
    //
    struct filtrate_def_type_
    {
      filtrate_def_type_ ()
      {
      }

      // value
      //
      typedef
      mysql::query_column<
        mysql::value_traits<
          ::std::basic_string< char >,
          mysql::id_string >::query_type,
        mysql::id_string >
      value_type_;

      static const value_type_ value;
    };

    static const filtrate_def_type_ filtrate_def;

    // base_filter_doc
    //
    struct base_filter_doc_type_
    {
      base_filter_doc_type_ ()
      {
      }

      // value
      //
      typedef
      mysql::query_column<
        mysql::value_traits<
          ::std::basic_string< char >,
          mysql::id_string >::query_type,
        mysql::id_string >
      value_type_;

      static const value_type_ value;
    };

    static const base_filter_doc_type_ base_filter_doc;

    // base_enable_policy
    //
    typedef
    mysql::query_column<
      mysql::value_traits<
        ::TR::Core::EnableReportPolicy,
        mysql::id_enum >::query_type,
      mysql::id_enum >
    base_enable_policy_type_;

    static const base_enable_policy_type_ base_enable_policy;

    // base_filtarte_uuid
    //
    struct base_filtarte_uuid_type_
    {
      base_filtarte_uuid_type_ ()
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

    static const base_filtarte_uuid_type_ base_filtarte_uuid;
  };

  template <typename A>
  const typename query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::identity_type_::subject_type_::value1_type_::value_type_
  query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::identity_type_::subject_type_::value1_type_::
  value (A::table_name, "`identity_subject_value1_value`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::identity_type_::subject_type_::value1_type_
  query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::identity_type_::subject_type_::value1;

  template <typename A>
  const typename query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::identity_type_::subject_type_::value2_type_::value_type_
  query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::identity_type_::subject_type_::value2_type_::
  value (A::table_name, "`identity_subject_value2_value`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::identity_type_::subject_type_::value2_type_
  query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::identity_type_::subject_type_::value2;

  template <typename A>
  const typename query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::identity_type_::subject_type_
  query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::identity_type_::subject;

  template <typename A>
  const typename query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::identity_type_::filter_report_uuid_type_::value_type_
  query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::identity_type_::filter_report_uuid_type_::
  value (A::table_name, "`identity_filter_report_uuid_value`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::identity_type_::filter_report_uuid_type_
  query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::identity_type_::filter_report_uuid;

  template <typename A>
  const typename query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::identity_type_
  query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::identity;

  template <typename A>
  const typename query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::filter_xml_type_::value_type_
  query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::filter_xml_type_::
  value (A::table_name, "`filter_xml_value`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::filter_xml_type_
  query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::filter_xml;

  template <typename A>
  const typename query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::enable_policy_type_
  query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::
  enable_policy (A::table_name, "`enable_policy`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::effective_filter_xml_type_::value_type_
  query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::effective_filter_xml_type_::
  value (A::table_name, "`effective_filter_xml_value`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::effective_filter_xml_type_
  query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::effective_filter_xml;

  template <typename A>
  const typename query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::effective_enable_policy_type_
  query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::
  effective_enable_policy (A::table_name, "`effective_enable_policy`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::base_enable_policy_override_type_
  query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::
  base_enable_policy_override (A::table_name, "`base_enable_policy_override`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::filtrate_uuid_type_::value_type_
  query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::filtrate_uuid_type_::
  value (A::table_name, "`filtrate_uuid_value`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::filtrate_uuid_type_
  query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::filtrate_uuid;

  template <typename A>
  const typename query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::filtrate_def_type_::value_type_
  query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::filtrate_def_type_::
  value (A::table_name, "`filtrate_def_value`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::filtrate_def_type_
  query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::filtrate_def;

  template <typename A>
  const typename query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::base_filter_doc_type_::value_type_
  query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::base_filter_doc_type_::
  value (A::table_name, "`base_filter_doc_value`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::base_filter_doc_type_
  query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::base_filter_doc;

  template <typename A>
  const typename query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::base_enable_policy_type_
  query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::
  base_enable_policy (A::table_name, "`base_enable_policy`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::base_filtarte_uuid_type_::value_type_
  query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::base_filtarte_uuid_type_::
  value (A::table_name, "`base_filtarte_uuid_value`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::base_filtarte_uuid_type_
  query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >::base_filtarte_uuid;

  template <typename A>
  struct pointer_query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >:
    query_columns< ::TR::Core::FilterReportStatusData, id_mysql, A >
  {
  };

  template <>
  class access::object_traits_impl< ::TR::Core::FilterReportStatusData, id_mysql >:
    public access::object_traits< ::TR::Core::FilterReportStatusData >
  {
    public:
    struct id_image_type
    {
      composite_value_traits< ::TR::Core::FilterReportStatusIdentity, id_mysql >::image_type id_value;

      std::size_t version;
    };

    struct image_type
    {
      // m_identity
      //
      composite_value_traits< ::TR::Core::FilterReportStatusIdentity, id_mysql >::image_type m_identity_value;

      // m_filter_xml
      //
      composite_value_traits< ::TR::Core::XmlFilterDocText, id_mysql >::image_type m_filter_xml_value;

      // m_enable_policy
      //
      mysql::value_traits< ::TR::Core::EnableReportPolicy, mysql::id_enum >::image_type m_enable_policy_value;
      unsigned long m_enable_policy_size;
      my_bool m_enable_policy_null;

      // m_effective_filter_xml
      //
      composite_value_traits< ::TR::Core::XmlFilterDocText, id_mysql >::image_type m_effective_filter_xml_value;

      // m_effective_enable_policy
      //
      mysql::value_traits< ::TR::Core::EnableReportPolicy, mysql::id_enum >::image_type m_effective_enable_policy_value;
      unsigned long m_effective_enable_policy_size;
      my_bool m_effective_enable_policy_null;

      // m_base_enable_policy_override
      //
      mysql::value_traits< ::TR::Core::EnableReportPolicy, mysql::id_enum >::image_type m_base_enable_policy_override_value;
      unsigned long m_base_enable_policy_override_size;
      my_bool m_base_enable_policy_override_null;

      // m_filtrate_uuid
      //
      composite_value_traits< ::TR::Core::OutputUUID, id_mysql >::image_type m_filtrate_uuid_value;

      // m_filtrate_def
      //
      composite_value_traits< ::TR::Core::XmlDefDocText, id_mysql >::image_type m_filtrate_def_value;

      // m_base_filter_doc
      //
      composite_value_traits< ::TR::Core::XmlFilterDocText, id_mysql >::image_type m_base_filter_doc_value;

      // m_base_enable_policy
      //
      mysql::value_traits< ::TR::Core::EnableReportPolicy, mysql::id_enum >::image_type m_base_enable_policy_value;
      unsigned long m_base_enable_policy_size;
      my_bool m_base_enable_policy_null;

      // m_base_filtarte_uuid
      //
      composite_value_traits< ::TR::Core::OutputUUID, id_mysql >::image_type m_base_filtarte_uuid_value;

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

    static const std::size_t column_count = 13UL;
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
  class access::object_traits_impl< ::TR::Core::FilterReportStatusData, id_common >:
    public access::object_traits_impl< ::TR::Core::FilterReportStatusData, id_mysql >
  {
  };

  // FilterReportData
  //
  // FilterReportStatusData
  //
}

#include "FilterReportData-odb.ixx"

#include <odb/post.hxx>

#endif // FILTER_REPORT_DATA_ODB_HXX