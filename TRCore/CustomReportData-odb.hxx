// This file was generated by ODB, object-relational mapping (ORM)
// compiler for C++.
//

#ifndef CUSTOM_REPORT_DATA_ODB_HXX
#define CUSTOM_REPORT_DATA_ODB_HXX

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

#include "CustomReportData.hxx"

#include "BasisValues-odb.hxx"
#include "RegistryValues-odb.hxx"
#include "ServicesValues-odb.hxx"
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
#include <odb/view-image.hxx>
#include <odb/view-result.hxx>

#include <odb/details/unused.hxx>
#include <odb/details/shared-ptr.hxx>

namespace odb
{
  // QueryJoiningData
  //
  template <>
  struct class_traits< ::TR::Core::QueryJoiningData >
  {
    static const class_kind kind = class_object;
  };

  template <>
  class access::object_traits< ::TR::Core::QueryJoiningData >
  {
    public:
    typedef ::TR::Core::QueryJoiningData object_type;
    typedef ::boost::shared_ptr< ::TR::Core::QueryJoiningData > pointer_type;
    typedef odb::pointer_traits<pointer_type> pointer_traits;

    static const bool polymorphic = false;

    typedef ::TR::Core::Key id_type;

    static const bool auto_id = true;

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

  // SubjectQueryData
  //
  template <>
  struct class_traits< ::TR::Core::SubjectQueryData >
  {
    static const class_kind kind = class_object;
  };

  template <>
  class access::object_traits< ::TR::Core::SubjectQueryData >
  {
    public:
    typedef ::TR::Core::SubjectQueryData object_type;
    typedef ::boost::shared_ptr< ::TR::Core::SubjectQueryData > pointer_type;
    typedef odb::pointer_traits<pointer_type> pointer_traits;

    static const bool polymorphic = false;

    typedef ::TR::Core::SubjectQueryIdentity id_type;

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

  // SubjectQueryView
  //
  template <>
  struct class_traits< ::TR::Core::SubjectQueryView >
  {
    static const class_kind kind = class_view;
  };

  template <>
  class access::view_traits< ::TR::Core::SubjectQueryView >
  {
    public:
    typedef ::TR::Core::SubjectQueryView view_type;
    typedef ::boost::shared_ptr< ::TR::Core::SubjectQueryView > pointer_type;

    static void
    callback (database&, view_type&, callback_event);
  };

  // SubjectJoinedQueryData
  //
  template <>
  struct class_traits< ::TR::Core::SubjectJoinedQueryData >
  {
    static const class_kind kind = class_object;
  };

  template <>
  class access::object_traits< ::TR::Core::SubjectJoinedQueryData >
  {
    public:
    typedef ::TR::Core::SubjectJoinedQueryData object_type;
    typedef ::boost::shared_ptr< ::TR::Core::SubjectJoinedQueryData > pointer_type;
    typedef odb::pointer_traits<pointer_type> pointer_traits;

    static const bool polymorphic = false;

    typedef ::TR::Core::SubjectQueryIdentity id_type;

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

  // SubjectJoinedQueryView
  //
  template <>
  struct class_traits< ::TR::Core::SubjectJoinedQueryView >
  {
    static const class_kind kind = class_view;
  };

  template <>
  class access::view_traits< ::TR::Core::SubjectJoinedQueryView >
  {
    public:
    typedef ::TR::Core::SubjectJoinedQueryView view_type;
    typedef ::boost::shared_ptr< ::TR::Core::SubjectJoinedQueryView > pointer_type;

    static void
    callback (database&, view_type&, callback_event);
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
  // QueryJoiningData
  //
  template <typename A>
  struct query_columns< ::TR::Core::QueryJoiningData, id_mysql, A >
  {
    // key
    //
    typedef
    mysql::query_column<
      mysql::value_traits<
        ::TR::Core::Key,
        mysql::id_long >::query_type,
      mysql::id_long >
    key_type_;

    static const key_type_ key;

    // input_uuid
    //
    struct input_uuid_class_
    {
      input_uuid_class_ ()
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

    static const input_uuid_class_ input_uuid;

    // output_uuid
    //
    struct output_uuid_class_
    {
      output_uuid_class_ ()
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

    static const output_uuid_class_ output_uuid;

    // default_query_xml
    //
    struct default_query_xml_class_
    {
      default_query_xml_class_ ()
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

    static const default_query_xml_class_ default_query_xml;
  };

  template <typename A>
  const typename query_columns< ::TR::Core::QueryJoiningData, id_mysql, A >::key_type_
  query_columns< ::TR::Core::QueryJoiningData, id_mysql, A >::
  key (A::table_name, "`key`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::QueryJoiningData, id_mysql, A >::input_uuid_class_::value_type_
  query_columns< ::TR::Core::QueryJoiningData, id_mysql, A >::input_uuid_class_::
  value (A::table_name, "`input_uuid_value`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::QueryJoiningData, id_mysql, A >::input_uuid_class_
  query_columns< ::TR::Core::QueryJoiningData, id_mysql, A >::input_uuid;

  template <typename A>
  const typename query_columns< ::TR::Core::QueryJoiningData, id_mysql, A >::output_uuid_class_::value_type_
  query_columns< ::TR::Core::QueryJoiningData, id_mysql, A >::output_uuid_class_::
  value (A::table_name, "`output_uuid_value`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::QueryJoiningData, id_mysql, A >::output_uuid_class_
  query_columns< ::TR::Core::QueryJoiningData, id_mysql, A >::output_uuid;

  template <typename A>
  const typename query_columns< ::TR::Core::QueryJoiningData, id_mysql, A >::default_query_xml_class_::value_type_
  query_columns< ::TR::Core::QueryJoiningData, id_mysql, A >::default_query_xml_class_::
  value (A::table_name, "`default_query_xml_value`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::QueryJoiningData, id_mysql, A >::default_query_xml_class_
  query_columns< ::TR::Core::QueryJoiningData, id_mysql, A >::default_query_xml;

  template <typename A>
  struct pointer_query_columns< ::TR::Core::QueryJoiningData, id_mysql, A >:
    query_columns< ::TR::Core::QueryJoiningData, id_mysql, A >
  {
  };

  template <>
  class access::object_traits_impl< ::TR::Core::QueryJoiningData, id_mysql >:
    public access::object_traits< ::TR::Core::QueryJoiningData >
  {
    public:
    struct id_image_type
    {
      int id_value;
      my_bool id_null;

      std::size_t version;
    };

    struct image_type
    {
      // m_key
      //
      int m_key_value;
      my_bool m_key_null;

      // m_input_uuid
      //
      composite_value_traits< ::TR::Core::ReportTypeUUID, id_mysql >::image_type m_input_uuid_value;

      // m_output_uuid
      //
      composite_value_traits< ::TR::Core::ReportTypeUUID, id_mysql >::image_type m_output_uuid_value;

      // m_default_query_xml
      //
      composite_value_traits< ::TR::Core::XmlQueryDocText, id_mysql >::image_type m_default_query_xml_value;

      std::size_t version;
    };

    struct extra_statement_cache_type;

    using object_traits<object_type>::id;

    static id_type
    id (const id_image_type&);

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

    static const std::size_t column_count = 4UL;
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
    persist (database&, object_type&);

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
  class access::object_traits_impl< ::TR::Core::QueryJoiningData, id_common >:
    public access::object_traits_impl< ::TR::Core::QueryJoiningData, id_mysql >
  {
  };

  // SubjectQueryIdentity
  //
  template <>
  class access::composite_value_traits< ::TR::Core::SubjectQueryIdentity, id_mysql >
  {
    public:
    typedef ::TR::Core::SubjectQueryIdentity value_type;

    struct image_type
    {
      // m_subject
      //
      composite_value_traits< ::TR::Core::SubjectIdentity, id_mysql >::image_type m_subject_value;

      // m_query_joining_key
      //
      composite_value_traits< ::TR::Core::QueryJoiningKey, id_mysql >::image_type m_query_joining_key_value;
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

    static const std::size_t column_count = 3UL;
  };

  // SubjectQueryData
  //
  template <typename A>
  struct query_columns< ::TR::Core::SubjectQueryData, id_mysql, A >
  {
    // identity
    //
    struct identity_class_
    {
      identity_class_ ()
      {
      }

      // subject
      //
      struct subject_class_1_
      {
        subject_class_1_ ()
        {
        }

        // value1
        //
        struct value1_class_2_
        {
          value1_class_2_ ()
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

        static const value1_class_2_ value1;

        // value2
        //
        struct value2_class_2_
        {
          value2_class_2_ ()
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

        static const value2_class_2_ value2;
      };

      static const subject_class_1_ subject;

      // query_joining_key
      //
      struct query_joining_key_class_1_
      {
        query_joining_key_class_1_ ()
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

      static const query_joining_key_class_1_ query_joining_key;
    };

    static const identity_class_ identity;

    // query_xml
    //
    struct query_xml_class_
    {
      query_xml_class_ ()
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

    static const query_xml_class_ query_xml;

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
  };

  template <typename A>
  const typename query_columns< ::TR::Core::SubjectQueryData, id_mysql, A >::identity_class_::subject_class_1_::value1_class_2_::value_type_
  query_columns< ::TR::Core::SubjectQueryData, id_mysql, A >::identity_class_::subject_class_1_::value1_class_2_::
  value (A::table_name, "`identity_subject_value1_value`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::SubjectQueryData, id_mysql, A >::identity_class_::subject_class_1_::value1_class_2_
  query_columns< ::TR::Core::SubjectQueryData, id_mysql, A >::identity_class_::subject_class_1_::value1;

  template <typename A>
  const typename query_columns< ::TR::Core::SubjectQueryData, id_mysql, A >::identity_class_::subject_class_1_::value2_class_2_::value_type_
  query_columns< ::TR::Core::SubjectQueryData, id_mysql, A >::identity_class_::subject_class_1_::value2_class_2_::
  value (A::table_name, "`identity_subject_value2_value`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::SubjectQueryData, id_mysql, A >::identity_class_::subject_class_1_::value2_class_2_
  query_columns< ::TR::Core::SubjectQueryData, id_mysql, A >::identity_class_::subject_class_1_::value2;

  template <typename A>
  const typename query_columns< ::TR::Core::SubjectQueryData, id_mysql, A >::identity_class_::subject_class_1_
  query_columns< ::TR::Core::SubjectQueryData, id_mysql, A >::identity_class_::subject;

  template <typename A>
  const typename query_columns< ::TR::Core::SubjectQueryData, id_mysql, A >::identity_class_::query_joining_key_class_1_::value_type_
  query_columns< ::TR::Core::SubjectQueryData, id_mysql, A >::identity_class_::query_joining_key_class_1_::
  value (A::table_name, "`identity_query_joining_key_value`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::SubjectQueryData, id_mysql, A >::identity_class_::query_joining_key_class_1_
  query_columns< ::TR::Core::SubjectQueryData, id_mysql, A >::identity_class_::query_joining_key;

  template <typename A>
  const typename query_columns< ::TR::Core::SubjectQueryData, id_mysql, A >::identity_class_
  query_columns< ::TR::Core::SubjectQueryData, id_mysql, A >::identity;

  template <typename A>
  const typename query_columns< ::TR::Core::SubjectQueryData, id_mysql, A >::query_xml_class_::value_type_
  query_columns< ::TR::Core::SubjectQueryData, id_mysql, A >::query_xml_class_::
  value (A::table_name, "`query_xml_value`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::SubjectQueryData, id_mysql, A >::query_xml_class_
  query_columns< ::TR::Core::SubjectQueryData, id_mysql, A >::query_xml;

  template <typename A>
  const typename query_columns< ::TR::Core::SubjectQueryData, id_mysql, A >::uuid_type_
  query_columns< ::TR::Core::SubjectQueryData, id_mysql, A >::
  uuid (A::table_name, "`uuid`", 0);

  template <typename A>
  struct pointer_query_columns< ::TR::Core::SubjectQueryData, id_mysql, A >:
    query_columns< ::TR::Core::SubjectQueryData, id_mysql, A >
  {
  };

  template <>
  class access::object_traits_impl< ::TR::Core::SubjectQueryData, id_mysql >:
    public access::object_traits< ::TR::Core::SubjectQueryData >
  {
    public:
    struct id_image_type
    {
      composite_value_traits< ::TR::Core::SubjectQueryIdentity, id_mysql >::image_type id_value;

      std::size_t version;
    };

    struct image_type
    {
      // m_identity
      //
      composite_value_traits< ::TR::Core::SubjectQueryIdentity, id_mysql >::image_type m_identity_value;

      // m_query_xml
      //
      composite_value_traits< ::TR::Core::XmlQueryDocText, id_mysql >::image_type m_query_xml_value;

      // m_uuid
      //
      details::buffer m_uuid_value;
      unsigned long m_uuid_size;
      my_bool m_uuid_null;

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
  class access::object_traits_impl< ::TR::Core::SubjectQueryData, id_common >:
    public access::object_traits_impl< ::TR::Core::SubjectQueryData, id_mysql >
  {
  };

  // SubjectQueryView
  //
  template <>
  class access::view_traits_impl< ::TR::Core::SubjectQueryView, id_mysql >:
    public access::view_traits< ::TR::Core::SubjectQueryView >
  {
    public:
    struct image_type
    {
      // m_identity
      //
      composite_value_traits< ::TR::Core::SubjectQueryIdentity, id_mysql >::image_type m_identity_value;

      std::size_t version;
    };

    typedef mysql::view_statements<view_type> statements_type;

    typedef mysql::query_base query_base_type;
    struct query_columns;

    static const bool versioned = false;

    static bool
    grow (image_type&,
          my_bool*);

    static void
    bind (MYSQL_BIND*,
          image_type&);

    static void
    init (view_type&,
          const image_type&,
          database*);

    static const std::size_t column_count = 3UL;

    static query_base_type
    query_statement (const query_base_type&);

    static result<view_type>
    query (database&, const query_base_type&);
  };

  template <>
  class access::view_traits_impl< ::TR::Core::SubjectQueryView, id_common >:
    public access::view_traits_impl< ::TR::Core::SubjectQueryView, id_mysql >
  {
  };

  // SubjectJoinedQueryData
  //
  template <typename A>
  struct query_columns< ::TR::Core::SubjectJoinedQueryData, id_mysql, A >
  {
    // identity
    //
    struct identity_class_
    {
      identity_class_ ()
      {
      }

      // subject
      //
      struct subject_class_1_
      {
        subject_class_1_ ()
        {
        }

        // value1
        //
        struct value1_class_2_
        {
          value1_class_2_ ()
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

        static const value1_class_2_ value1;

        // value2
        //
        struct value2_class_2_
        {
          value2_class_2_ ()
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

        static const value2_class_2_ value2;
      };

      static const subject_class_1_ subject;

      // query_joining_key
      //
      struct query_joining_key_class_1_
      {
        query_joining_key_class_1_ ()
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

      static const query_joining_key_class_1_ query_joining_key;
    };

    static const identity_class_ identity;

    // query_xml
    //
    struct query_xml_class_
    {
      query_xml_class_ ()
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

    static const query_xml_class_ query_xml;

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

    // base_query_uuid
    //
    typedef
    mysql::query_column<
      mysql::value_traits<
        ::TR::Core::UUID,
        mysql::id_blob >::query_type,
      mysql::id_blob >
    base_query_uuid_type_;

    static const base_query_uuid_type_ base_query_uuid;

    // subject_query_uuid
    //
    typedef
    mysql::query_column<
      mysql::value_traits<
        ::TR::Core::UUID,
        mysql::id_blob >::query_type,
      mysql::id_blob >
    subject_query_uuid_type_;

    static const subject_query_uuid_type_ subject_query_uuid;
  };

  template <typename A>
  const typename query_columns< ::TR::Core::SubjectJoinedQueryData, id_mysql, A >::identity_class_::subject_class_1_::value1_class_2_::value_type_
  query_columns< ::TR::Core::SubjectJoinedQueryData, id_mysql, A >::identity_class_::subject_class_1_::value1_class_2_::
  value (A::table_name, "`identity_subject_value1_value`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::SubjectJoinedQueryData, id_mysql, A >::identity_class_::subject_class_1_::value1_class_2_
  query_columns< ::TR::Core::SubjectJoinedQueryData, id_mysql, A >::identity_class_::subject_class_1_::value1;

  template <typename A>
  const typename query_columns< ::TR::Core::SubjectJoinedQueryData, id_mysql, A >::identity_class_::subject_class_1_::value2_class_2_::value_type_
  query_columns< ::TR::Core::SubjectJoinedQueryData, id_mysql, A >::identity_class_::subject_class_1_::value2_class_2_::
  value (A::table_name, "`identity_subject_value2_value`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::SubjectJoinedQueryData, id_mysql, A >::identity_class_::subject_class_1_::value2_class_2_
  query_columns< ::TR::Core::SubjectJoinedQueryData, id_mysql, A >::identity_class_::subject_class_1_::value2;

  template <typename A>
  const typename query_columns< ::TR::Core::SubjectJoinedQueryData, id_mysql, A >::identity_class_::subject_class_1_
  query_columns< ::TR::Core::SubjectJoinedQueryData, id_mysql, A >::identity_class_::subject;

  template <typename A>
  const typename query_columns< ::TR::Core::SubjectJoinedQueryData, id_mysql, A >::identity_class_::query_joining_key_class_1_::value_type_
  query_columns< ::TR::Core::SubjectJoinedQueryData, id_mysql, A >::identity_class_::query_joining_key_class_1_::
  value (A::table_name, "`identity_query_joining_key_value`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::SubjectJoinedQueryData, id_mysql, A >::identity_class_::query_joining_key_class_1_
  query_columns< ::TR::Core::SubjectJoinedQueryData, id_mysql, A >::identity_class_::query_joining_key;

  template <typename A>
  const typename query_columns< ::TR::Core::SubjectJoinedQueryData, id_mysql, A >::identity_class_
  query_columns< ::TR::Core::SubjectJoinedQueryData, id_mysql, A >::identity;

  template <typename A>
  const typename query_columns< ::TR::Core::SubjectJoinedQueryData, id_mysql, A >::query_xml_class_::value_type_
  query_columns< ::TR::Core::SubjectJoinedQueryData, id_mysql, A >::query_xml_class_::
  value (A::table_name, "`query_xml_value`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::SubjectJoinedQueryData, id_mysql, A >::query_xml_class_
  query_columns< ::TR::Core::SubjectJoinedQueryData, id_mysql, A >::query_xml;

  template <typename A>
  const typename query_columns< ::TR::Core::SubjectJoinedQueryData, id_mysql, A >::uuid_type_
  query_columns< ::TR::Core::SubjectJoinedQueryData, id_mysql, A >::
  uuid (A::table_name, "`uuid`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::SubjectJoinedQueryData, id_mysql, A >::base_query_uuid_type_
  query_columns< ::TR::Core::SubjectJoinedQueryData, id_mysql, A >::
  base_query_uuid (A::table_name, "`base_query_uuid`", 0);

  template <typename A>
  const typename query_columns< ::TR::Core::SubjectJoinedQueryData, id_mysql, A >::subject_query_uuid_type_
  query_columns< ::TR::Core::SubjectJoinedQueryData, id_mysql, A >::
  subject_query_uuid (A::table_name, "`subject_query_uuid`", 0);

  template <typename A>
  struct pointer_query_columns< ::TR::Core::SubjectJoinedQueryData, id_mysql, A >:
    query_columns< ::TR::Core::SubjectJoinedQueryData, id_mysql, A >
  {
  };

  template <>
  class access::object_traits_impl< ::TR::Core::SubjectJoinedQueryData, id_mysql >:
    public access::object_traits< ::TR::Core::SubjectJoinedQueryData >
  {
    public:
    struct id_image_type
    {
      composite_value_traits< ::TR::Core::SubjectQueryIdentity, id_mysql >::image_type id_value;

      std::size_t version;
    };

    struct image_type
    {
      // m_identity
      //
      composite_value_traits< ::TR::Core::SubjectQueryIdentity, id_mysql >::image_type m_identity_value;

      // m_query_xml
      //
      composite_value_traits< ::TR::Core::XmlQueryDocText, id_mysql >::image_type m_query_xml_value;

      // m_uuid
      //
      details::buffer m_uuid_value;
      unsigned long m_uuid_size;
      my_bool m_uuid_null;

      // m_base_query_uuid
      //
      details::buffer m_base_query_uuid_value;
      unsigned long m_base_query_uuid_size;
      my_bool m_base_query_uuid_null;

      // m_subject_query_uuid
      //
      details::buffer m_subject_query_uuid_value;
      unsigned long m_subject_query_uuid_size;
      my_bool m_subject_query_uuid_null;

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

    static const std::size_t column_count = 7UL;
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
  class access::object_traits_impl< ::TR::Core::SubjectJoinedQueryData, id_common >:
    public access::object_traits_impl< ::TR::Core::SubjectJoinedQueryData, id_mysql >
  {
  };

  // SubjectJoinedQueryView
  //
  template <>
  class access::view_traits_impl< ::TR::Core::SubjectJoinedQueryView, id_mysql >:
    public access::view_traits< ::TR::Core::SubjectJoinedQueryView >
  {
    public:
    struct image_type
    {
      // m_identity
      //
      composite_value_traits< ::TR::Core::SubjectQueryIdentity, id_mysql >::image_type m_identity_value;

      std::size_t version;
    };

    typedef mysql::view_statements<view_type> statements_type;

    typedef mysql::query_base query_base_type;
    struct query_columns;

    static const bool versioned = false;

    static bool
    grow (image_type&,
          my_bool*);

    static void
    bind (MYSQL_BIND*,
          image_type&);

    static void
    init (view_type&,
          const image_type&,
          database*);

    static const std::size_t column_count = 3UL;

    static query_base_type
    query_statement (const query_base_type&);

    static result<view_type>
    query (database&, const query_base_type&);
  };

  template <>
  class access::view_traits_impl< ::TR::Core::SubjectJoinedQueryView, id_common >:
    public access::view_traits_impl< ::TR::Core::SubjectJoinedQueryView, id_mysql >
  {
  };

  // QueryJoiningData
  //
  // SubjectQueryData
  //
  // SubjectQueryView
  //
  struct access::view_traits_impl< ::TR::Core::SubjectQueryView, id_mysql >::query_columns:
    odb::pointer_query_columns<
      ::TR::Core::SubjectQueryData,
      id_mysql,
      odb::access::object_traits_impl< ::TR::Core::SubjectQueryData, id_mysql > >
  {
  };

  // SubjectJoinedQueryData
  //
  // SubjectJoinedQueryView
  //
  struct access::view_traits_impl< ::TR::Core::SubjectJoinedQueryView, id_mysql >::query_columns:
    odb::pointer_query_columns<
      ::TR::Core::SubjectJoinedQueryData,
      id_mysql,
      odb::access::object_traits_impl< ::TR::Core::SubjectJoinedQueryData, id_mysql > >
  {
  };
}

#include "CustomReportData-odb.ixx"

#include <odb/post.hxx>

#endif // CUSTOM_REPORT_DATA_ODB_HXX
