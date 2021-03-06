// This file was generated by ODB, object-relational mapping (ORM)
// compiler for C++.
//

#include <odb/pre.hxx>

#include "ServicesValues-odb.hxx"

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
  // QueryJoiningKey
  //

  bool access::composite_value_traits< ::TR::Core::QueryJoiningKey, id_mysql >::
  grow (image_type& i,
        my_bool* t)
  {
    ODB_POTENTIALLY_UNUSED (i);
    ODB_POTENTIALLY_UNUSED (t);

    bool grew (false);

    // KeyRestriction base
    //
    if (composite_value_traits< ::TR::Core::KeyRestriction, id_mysql >::grow (
          i, t + 0UL))
      grew = true;

    return grew;
  }

  void access::composite_value_traits< ::TR::Core::QueryJoiningKey, id_mysql >::
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

    // KeyRestriction base
    //
    composite_value_traits< ::TR::Core::KeyRestriction, id_mysql >::bind (b + n, i, sk);
    n += 1UL;
  }

  bool access::composite_value_traits< ::TR::Core::QueryJoiningKey, id_mysql >::
  init (image_type& i,
        const value_type& o,
        mysql::statement_kind sk)
  {
    ODB_POTENTIALLY_UNUSED (i);
    ODB_POTENTIALLY_UNUSED (o);
    ODB_POTENTIALLY_UNUSED (sk);

    using namespace mysql;

    bool grew (false);

    // KeyRestriction base
    //
    if (composite_value_traits< ::TR::Core::KeyRestriction, id_mysql >::init (i, o, sk))
      grew = true;

    return grew;
  }

  void access::composite_value_traits< ::TR::Core::QueryJoiningKey, id_mysql >::
  init (value_type& o,
        const image_type&  i,
        database* db)
  {
    ODB_POTENTIALLY_UNUSED (o);
    ODB_POTENTIALLY_UNUSED (i);
    ODB_POTENTIALLY_UNUSED (db);

    // KeyRestriction base
    //
    composite_value_traits< ::TR::Core::KeyRestriction, id_mysql >::init (o, i, db);
  }

  // ComplianceKey
  //

  bool access::composite_value_traits< ::TR::Core::ComplianceKey, id_mysql >::
  grow (image_type& i,
        my_bool* t)
  {
    ODB_POTENTIALLY_UNUSED (i);
    ODB_POTENTIALLY_UNUSED (t);

    bool grew (false);

    // KeyRestriction base
    //
    if (composite_value_traits< ::TR::Core::KeyRestriction, id_mysql >::grow (
          i, t + 0UL))
      grew = true;

    return grew;
  }

  void access::composite_value_traits< ::TR::Core::ComplianceKey, id_mysql >::
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

    // KeyRestriction base
    //
    composite_value_traits< ::TR::Core::KeyRestriction, id_mysql >::bind (b + n, i, sk);
    n += 1UL;
  }

  bool access::composite_value_traits< ::TR::Core::ComplianceKey, id_mysql >::
  init (image_type& i,
        const value_type& o,
        mysql::statement_kind sk)
  {
    ODB_POTENTIALLY_UNUSED (i);
    ODB_POTENTIALLY_UNUSED (o);
    ODB_POTENTIALLY_UNUSED (sk);

    using namespace mysql;

    bool grew (false);

    // KeyRestriction base
    //
    if (composite_value_traits< ::TR::Core::KeyRestriction, id_mysql >::init (i, o, sk))
      grew = true;

    return grew;
  }

  void access::composite_value_traits< ::TR::Core::ComplianceKey, id_mysql >::
  init (value_type& o,
        const image_type&  i,
        database* db)
  {
    ODB_POTENTIALLY_UNUSED (o);
    ODB_POTENTIALLY_UNUSED (i);
    ODB_POTENTIALLY_UNUSED (db);

    // KeyRestriction base
    //
    composite_value_traits< ::TR::Core::KeyRestriction, id_mysql >::init (o, i, db);
  }

  // ValidationKey
  //

  bool access::composite_value_traits< ::TR::Core::ValidationKey, id_mysql >::
  grow (image_type& i,
        my_bool* t)
  {
    ODB_POTENTIALLY_UNUSED (i);
    ODB_POTENTIALLY_UNUSED (t);

    bool grew (false);

    // KeyRestriction base
    //
    if (composite_value_traits< ::TR::Core::KeyRestriction, id_mysql >::grow (
          i, t + 0UL))
      grew = true;

    return grew;
  }

  void access::composite_value_traits< ::TR::Core::ValidationKey, id_mysql >::
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

    // KeyRestriction base
    //
    composite_value_traits< ::TR::Core::KeyRestriction, id_mysql >::bind (b + n, i, sk);
    n += 1UL;
  }

  bool access::composite_value_traits< ::TR::Core::ValidationKey, id_mysql >::
  init (image_type& i,
        const value_type& o,
        mysql::statement_kind sk)
  {
    ODB_POTENTIALLY_UNUSED (i);
    ODB_POTENTIALLY_UNUSED (o);
    ODB_POTENTIALLY_UNUSED (sk);

    using namespace mysql;

    bool grew (false);

    // KeyRestriction base
    //
    if (composite_value_traits< ::TR::Core::KeyRestriction, id_mysql >::init (i, o, sk))
      grew = true;

    return grew;
  }

  void access::composite_value_traits< ::TR::Core::ValidationKey, id_mysql >::
  init (value_type& o,
        const image_type&  i,
        database* db)
  {
    ODB_POTENTIALLY_UNUSED (o);
    ODB_POTENTIALLY_UNUSED (i);
    ODB_POTENTIALLY_UNUSED (db);

    // KeyRestriction base
    //
    composite_value_traits< ::TR::Core::KeyRestriction, id_mysql >::init (o, i, db);
  }

  // ArchivedReportKey
  //

  bool access::composite_value_traits< ::TR::Core::ArchivedReportKey, id_mysql >::
  grow (image_type& i,
        my_bool* t)
  {
    ODB_POTENTIALLY_UNUSED (i);
    ODB_POTENTIALLY_UNUSED (t);

    bool grew (false);

    // KeyRestriction base
    //
    if (composite_value_traits< ::TR::Core::KeyRestriction, id_mysql >::grow (
          i, t + 0UL))
      grew = true;

    return grew;
  }

  void access::composite_value_traits< ::TR::Core::ArchivedReportKey, id_mysql >::
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

    // KeyRestriction base
    //
    composite_value_traits< ::TR::Core::KeyRestriction, id_mysql >::bind (b + n, i, sk);
    n += 1UL;
  }

  bool access::composite_value_traits< ::TR::Core::ArchivedReportKey, id_mysql >::
  init (image_type& i,
        const value_type& o,
        mysql::statement_kind sk)
  {
    ODB_POTENTIALLY_UNUSED (i);
    ODB_POTENTIALLY_UNUSED (o);
    ODB_POTENTIALLY_UNUSED (sk);

    using namespace mysql;

    bool grew (false);

    // KeyRestriction base
    //
    if (composite_value_traits< ::TR::Core::KeyRestriction, id_mysql >::init (i, o, sk))
      grew = true;

    return grew;
  }

  void access::composite_value_traits< ::TR::Core::ArchivedReportKey, id_mysql >::
  init (value_type& o,
        const image_type&  i,
        database* db)
  {
    ODB_POTENTIALLY_UNUSED (o);
    ODB_POTENTIALLY_UNUSED (i);
    ODB_POTENTIALLY_UNUSED (db);

    // KeyRestriction base
    //
    composite_value_traits< ::TR::Core::KeyRestriction, id_mysql >::init (o, i, db);
  }
}

#include <odb/post.hxx>
