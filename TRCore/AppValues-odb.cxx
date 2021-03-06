// This file was generated by ODB, object-relational mapping (ORM)
// compiler for C++.
//

#include <odb/pre.hxx>

#include "AppValues-odb.hxx"

#include <cassert>
#include <cstring>  // std::memcpy

#include <odb/schema-catalog-impl.hxx>

#include <odb/mysql/traits.hxx>
#include <odb/mysql/database.hxx>
#include <odb/mysql/transaction.hxx>
#include <odb/mysql/connection.hxx>
#include <odb/mysql/statement.hxx>
#include <odb/mysql/statement-cache.hxx>
#include <odb/mysql/container-statements.hxx>
#include <odb/mysql/exceptions.hxx>
#include <odb/mysql/enum.hxx>

namespace odb
{
  // PeriodicReloadIdentity
  //

  bool access::composite_value_traits< ::TR::Core::PeriodicReloadIdentity, id_mysql >::
  grow (image_type& i,
        my_bool* t)
  {
    ODB_POTENTIALLY_UNUSED (i);
    ODB_POTENTIALLY_UNUSED (t);

    bool grew (false);

    // m_report_type_uuid
    //
    if (composite_value_traits< ::TR::Core::ReportTypeUUID, id_mysql >::grow (
          i.m_report_type_uuid_value, t + 0UL))
      grew = true;

    return grew;
  }

  void access::composite_value_traits< ::TR::Core::PeriodicReloadIdentity, id_mysql >::
  bind (MYSQL_BIND* b,
        image_type& i,
        mysql::statement_kind sk)
  {
    ODB_POTENTIALLY_UNUSED (b);
    ODB_POTENTIALLY_UNUSED (i);
    ODB_POTENTIALLY_UNUSED (sk);

    using namespace mysql;

    std::size_t n (0);
    ODB_POTENTIALLY_UNUSED (n);

    // m_report_type_uuid
    //
    composite_value_traits< ::TR::Core::ReportTypeUUID, id_mysql >::bind (
      b + n, i.m_report_type_uuid_value, sk);
    n += 1UL;
  }

  bool access::composite_value_traits< ::TR::Core::PeriodicReloadIdentity, id_mysql >::
  init (image_type& i,
        const value_type& o,
        mysql::statement_kind sk)
  {
    ODB_POTENTIALLY_UNUSED (i);
    ODB_POTENTIALLY_UNUSED (o);
    ODB_POTENTIALLY_UNUSED (sk);

    using namespace mysql;

    bool grew (false);

    // m_report_type_uuid
    //
    {
      ::TR::Core::ReportTypeUUID const& v =
        o.m_report_type_uuid;

      if (composite_value_traits< ::TR::Core::ReportTypeUUID, id_mysql >::init (
            i.m_report_type_uuid_value,
            v,
            sk))
        grew = true;
    }

    return grew;
  }

  void access::composite_value_traits< ::TR::Core::PeriodicReloadIdentity, id_mysql >::
  init (value_type& o,
        const image_type&  i,
        database* db)
  {
    ODB_POTENTIALLY_UNUSED (o);
    ODB_POTENTIALLY_UNUSED (i);
    ODB_POTENTIALLY_UNUSED (db);

    // m_report_type_uuid
    //
    {
      ::TR::Core::ReportTypeUUID& v =
        o.m_report_type_uuid;

      composite_value_traits< ::TR::Core::ReportTypeUUID, id_mysql >::init (
        v,
        i.m_report_type_uuid_value,
        db);
    }
  }

  // PeriodicLaunchIdentity
  //

  bool access::composite_value_traits< ::TR::Core::PeriodicLaunchIdentity, id_mysql >::
  grow (image_type& i,
        my_bool* t)
  {
    ODB_POTENTIALLY_UNUSED (i);
    ODB_POTENTIALLY_UNUSED (t);

    bool grew (false);

    // m_action_shortcut_uuid
    //
    if (t[0UL])
    {
      i.m_action_shortcut_uuid_value.capacity (i.m_action_shortcut_uuid_size);
      grew = true;
    }

    return grew;
  }

  void access::composite_value_traits< ::TR::Core::PeriodicLaunchIdentity, id_mysql >::
  bind (MYSQL_BIND* b,
        image_type& i,
        mysql::statement_kind sk)
  {
    ODB_POTENTIALLY_UNUSED (b);
    ODB_POTENTIALLY_UNUSED (i);
    ODB_POTENTIALLY_UNUSED (sk);

    using namespace mysql;

    std::size_t n (0);
    ODB_POTENTIALLY_UNUSED (n);

    // m_action_shortcut_uuid
    //
    b[n].buffer_type = MYSQL_TYPE_BLOB;
    b[n].buffer = i.m_action_shortcut_uuid_value.data ();
    b[n].buffer_length = static_cast<unsigned long> (
      i.m_action_shortcut_uuid_value.capacity ());
    b[n].length = &i.m_action_shortcut_uuid_size;
    b[n].is_null = &i.m_action_shortcut_uuid_null;
    n++;
  }

  bool access::composite_value_traits< ::TR::Core::PeriodicLaunchIdentity, id_mysql >::
  init (image_type& i,
        const value_type& o,
        mysql::statement_kind sk)
  {
    ODB_POTENTIALLY_UNUSED (i);
    ODB_POTENTIALLY_UNUSED (o);
    ODB_POTENTIALLY_UNUSED (sk);

    using namespace mysql;

    bool grew (false);

    // m_action_shortcut_uuid
    //
    {
      ::TR::Core::UUID const& v =
        o.m_action_shortcut_uuid;

      bool is_null (true);
      std::size_t size (0);
      std::size_t cap (i.m_action_shortcut_uuid_value.capacity ());
      mysql::value_traits<
          ::TR::Core::UUID,
          mysql::id_blob >::set_image (
        i.m_action_shortcut_uuid_value,
        size,
        is_null,
        v);
      i.m_action_shortcut_uuid_null = is_null;
      i.m_action_shortcut_uuid_size = static_cast<unsigned long> (size);
      grew = grew || (cap != i.m_action_shortcut_uuid_value.capacity ());
    }

    return grew;
  }

  void access::composite_value_traits< ::TR::Core::PeriodicLaunchIdentity, id_mysql >::
  init (value_type& o,
        const image_type&  i,
        database* db)
  {
    ODB_POTENTIALLY_UNUSED (o);
    ODB_POTENTIALLY_UNUSED (i);
    ODB_POTENTIALLY_UNUSED (db);

    // m_action_shortcut_uuid
    //
    {
      ::TR::Core::UUID& v =
        o.m_action_shortcut_uuid;

      mysql::value_traits<
          ::TR::Core::UUID,
          mysql::id_blob >::set_value (
        v,
        i.m_action_shortcut_uuid_value,
        i.m_action_shortcut_uuid_size,
        i.m_action_shortcut_uuid_null);
    }
  }

  // PeriodicOperationVariant
  //

  bool access::composite_value_traits< ::TR::Core::PeriodicOperationVariant, id_mysql >::
  grow (image_type& i,
        my_bool* t)
  {
    ODB_POTENTIALLY_UNUSED (i);
    ODB_POTENTIALLY_UNUSED (t);

    bool grew (false);

    // m_value1
    //
    if (composite_value_traits< ::TR::Core::PeriodicReloadIdentity, id_mysql >::grow (
          i.m_value1_value, t + 0UL))
      grew = true;

    // m_value2
    //
    if (composite_value_traits< ::TR::Core::PeriodicLaunchIdentity, id_mysql >::grow (
          i.m_value2_value, t + 1UL))
      grew = true;

    return grew;
  }

  void access::composite_value_traits< ::TR::Core::PeriodicOperationVariant, id_mysql >::
  bind (MYSQL_BIND* b,
        image_type& i,
        mysql::statement_kind sk)
  {
    ODB_POTENTIALLY_UNUSED (b);
    ODB_POTENTIALLY_UNUSED (i);
    ODB_POTENTIALLY_UNUSED (sk);

    using namespace mysql;

    std::size_t n (0);
    ODB_POTENTIALLY_UNUSED (n);

    // m_value1
    //
    composite_value_traits< ::TR::Core::PeriodicReloadIdentity, id_mysql >::bind (
      b + n, i.m_value1_value, sk);
    n += 1UL;

    // m_value2
    //
    composite_value_traits< ::TR::Core::PeriodicLaunchIdentity, id_mysql >::bind (
      b + n, i.m_value2_value, sk);
    n += 1UL;
  }

  bool access::composite_value_traits< ::TR::Core::PeriodicOperationVariant, id_mysql >::
  init (image_type& i,
        const value_type& o,
        mysql::statement_kind sk)
  {
    ODB_POTENTIALLY_UNUSED (i);
    ODB_POTENTIALLY_UNUSED (o);
    ODB_POTENTIALLY_UNUSED (sk);

    using namespace mysql;

    bool grew (false);

    // m_value1
    //
    {
      ::TR::Core::PeriodicReloadIdentity const& v =
        o.m_value1;

      if (composite_value_traits< ::TR::Core::PeriodicReloadIdentity, id_mysql >::init (
            i.m_value1_value,
            v,
            sk))
        grew = true;
    }

    // m_value2
    //
    {
      ::TR::Core::PeriodicLaunchIdentity const& v =
        o.m_value2;

      if (composite_value_traits< ::TR::Core::PeriodicLaunchIdentity, id_mysql >::init (
            i.m_value2_value,
            v,
            sk))
        grew = true;
    }

    return grew;
  }

  void access::composite_value_traits< ::TR::Core::PeriodicOperationVariant, id_mysql >::
  init (value_type& o,
        const image_type&  i,
        database* db)
  {
    ODB_POTENTIALLY_UNUSED (o);
    ODB_POTENTIALLY_UNUSED (i);
    ODB_POTENTIALLY_UNUSED (db);

    // m_value1
    //
    {
      ::TR::Core::PeriodicReloadIdentity& v =
        o.m_value1;

      composite_value_traits< ::TR::Core::PeriodicReloadIdentity, id_mysql >::init (
        v,
        i.m_value1_value,
        db);
    }

    // m_value2
    //
    {
      ::TR::Core::PeriodicLaunchIdentity& v =
        o.m_value2;

      composite_value_traits< ::TR::Core::PeriodicLaunchIdentity, id_mysql >::init (
        v,
        i.m_value2_value,
        db);
    }
  }
}

#include <odb/post.hxx>
