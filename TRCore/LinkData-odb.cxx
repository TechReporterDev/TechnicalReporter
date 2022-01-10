// This file was generated by ODB, object-relational mapping (ORM)
// compiler for C++.
//

#include <odb/pre.hxx>

#include "LinkData-odb.hxx"

#include <cassert>
#include <cstring>  // std::memcpy

#include <odb/schema-catalog-impl.hxx>

#include <odb/mysql/traits.hxx>
#include <odb/mysql/database.hxx>
#include <odb/mysql/transaction.hxx>
#include <odb/mysql/connection.hxx>
#include <odb/mysql/statement.hxx>
#include <odb/mysql/statement-cache.hxx>
#include <odb/mysql/simple-object-statements.hxx>
#include <odb/mysql/container-statements.hxx>
#include <odb/mysql/exceptions.hxx>
#include <odb/mysql/simple-object-result.hxx>
#include <odb/mysql/enum.hxx>

namespace odb
{
  // LinkID
  //

  bool access::composite_value_traits< ::TR::Core::LinkID, id_mysql >::
  grow (image_type& i,
        my_bool* t)
  {
    ODB_POTENTIALLY_UNUSED (i);
    ODB_POTENTIALLY_UNUSED (t);

    bool grew (false);

    // first
    //
    if (composite_value_traits< ::TR::Core::SourceKey, id_mysql >::grow (
          i.first_value, t + 0UL))
      grew = true;

    // second
    //
    if (composite_value_traits< ::TR::Core::SourceKey, id_mysql >::grow (
          i.second_value, t + 1UL))
      grew = true;

    return grew;
  }

  void access::composite_value_traits< ::TR::Core::LinkID, id_mysql >::
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

    // first
    //
    composite_value_traits< ::TR::Core::SourceKey, id_mysql >::bind (
      b + n, i.first_value, sk);
    n += 1UL;

    // second
    //
    composite_value_traits< ::TR::Core::SourceKey, id_mysql >::bind (
      b + n, i.second_value, sk);
    n += 1UL;
  }

  bool access::composite_value_traits< ::TR::Core::LinkID, id_mysql >::
  init (image_type& i,
        const value_type& o,
        mysql::statement_kind sk)
  {
    ODB_POTENTIALLY_UNUSED (i);
    ODB_POTENTIALLY_UNUSED (o);
    ODB_POTENTIALLY_UNUSED (sk);

    using namespace mysql;

    bool grew (false);

    // first
    //
    {
      ::TR::Core::SourceKey const& v =
        o.first;

      composite_value_traits< ::TR::Core::SourceKey, id_mysql >::init (
        i.first_value,
        v,
        sk);
    }

    // second
    //
    {
      ::TR::Core::SourceKey const& v =
        o.second;

      composite_value_traits< ::TR::Core::SourceKey, id_mysql >::init (
        i.second_value,
        v,
        sk);
    }

    return grew;
  }

  void access::composite_value_traits< ::TR::Core::LinkID, id_mysql >::
  init (value_type& o,
        const image_type&  i,
        database* db)
  {
    ODB_POTENTIALLY_UNUSED (o);
    ODB_POTENTIALLY_UNUSED (i);
    ODB_POTENTIALLY_UNUSED (db);

    // first
    //
    {
      ::TR::Core::SourceKey& v =
        o.first;

      composite_value_traits< ::TR::Core::SourceKey, id_mysql >::init (
        v,
        i.first_value,
        db);
    }

    // second
    //
    {
      ::TR::Core::SourceKey& v =
        o.second;

      composite_value_traits< ::TR::Core::SourceKey, id_mysql >::init (
        v,
        i.second_value,
        db);
    }
  }

  // LinkData
  //

  struct access::object_traits_impl< ::TR::Core::LinkData, id_mysql >::extra_statement_cache_type
  {
    extra_statement_cache_type (
      mysql::connection&,
      image_type&,
      id_image_type&,
      mysql::binding&,
      mysql::binding&)
    {
    }
  };

  access::object_traits_impl< ::TR::Core::LinkData, id_mysql >::id_type
  access::object_traits_impl< ::TR::Core::LinkData, id_mysql >::
  id (const image_type& i)
  {
    mysql::database* db (0);
    ODB_POTENTIALLY_UNUSED (db);

    id_type id;
    {
      composite_value_traits< ::TR::Core::LinkID, id_mysql >::init (
        id,
        i.ID_value,
        db);
    }

    return id;
  }

  bool access::object_traits_impl< ::TR::Core::LinkData, id_mysql >::
  grow (image_type& i,
        my_bool* t)
  {
    ODB_POTENTIALLY_UNUSED (i);
    ODB_POTENTIALLY_UNUSED (t);

    bool grew (false);

    // ID
    //
    if (composite_value_traits< ::TR::Core::LinkID, id_mysql >::grow (
          i.ID_value, t + 0UL))
      grew = true;

    return grew;
  }

  void access::object_traits_impl< ::TR::Core::LinkData, id_mysql >::
  bind (MYSQL_BIND* b,
        image_type& i,
        mysql::statement_kind sk)
  {
    ODB_POTENTIALLY_UNUSED (sk);

    using namespace mysql;

    std::size_t n (0);

    // ID
    //
    if (sk != statement_update)
    {
      composite_value_traits< ::TR::Core::LinkID, id_mysql >::bind (
        b + n, i.ID_value, sk);
      n += 2UL;
    }
  }

  void access::object_traits_impl< ::TR::Core::LinkData, id_mysql >::
  bind (MYSQL_BIND* b, id_image_type& i)
  {
    std::size_t n (0);
    mysql::statement_kind sk (mysql::statement_select);
    composite_value_traits< ::TR::Core::LinkID, id_mysql >::bind (
      b + n, i.id_value, sk);
  }

  bool access::object_traits_impl< ::TR::Core::LinkData, id_mysql >::
  init (image_type& i,
        const object_type& o,
        mysql::statement_kind sk)
  {
    ODB_POTENTIALLY_UNUSED (i);
    ODB_POTENTIALLY_UNUSED (o);
    ODB_POTENTIALLY_UNUSED (sk);

    using namespace mysql;

    bool grew (false);

    // ID
    //
    if (sk == statement_insert)
    {
      // From LinkData.hxx:34:14
      ::TR::Core::LinkID const& v =
        ::TR::Core::LinkID (o.m_parent_key, o.m_child_key);

      composite_value_traits< ::TR::Core::LinkID, id_mysql >::init (
        i.ID_value,
        v,
        sk);
    }

    return grew;
  }

  void access::object_traits_impl< ::TR::Core::LinkData, id_mysql >::
  init (object_type& o,
        const image_type& i,
        database* db)
  {
    ODB_POTENTIALLY_UNUSED (o);
    ODB_POTENTIALLY_UNUSED (i);
    ODB_POTENTIALLY_UNUSED (db);

    // ID
    //
    {
      // From LinkData.hxx:35:14
      ::TR::Core::LinkID v;

      composite_value_traits< ::TR::Core::LinkID, id_mysql >::init (
        v,
        i.ID_value,
        db);

      // From LinkData.hxx:35:14
      o.m_parent_key = (v).first;
      o.m_child_key = (v).second;
    }
  }

  void access::object_traits_impl< ::TR::Core::LinkData, id_mysql >::
  init (id_image_type& i, const id_type& id)
  {
    mysql::statement_kind sk (mysql::statement_select);
    {
      composite_value_traits< ::TR::Core::LinkID, id_mysql >::init (
        i.id_value,
        id,
        sk);
    }
  }

  const char access::object_traits_impl< ::TR::Core::LinkData, id_mysql >::persist_statement[] =
  "INSERT INTO `LinkData` "
  "(`ID_first_value`, "
  "`ID_second_value`) "
  "VALUES "
  "(?, ?)";

  const char access::object_traits_impl< ::TR::Core::LinkData, id_mysql >::find_statement[] =
  "SELECT "
  "`LinkData`.`ID_first_value`, "
  "`LinkData`.`ID_second_value` "
  "FROM `LinkData` "
  "WHERE `LinkData`.`ID_first_value`=? AND `LinkData`.`ID_second_value`=?";

  const char access::object_traits_impl< ::TR::Core::LinkData, id_mysql >::erase_statement[] =
  "DELETE FROM `LinkData` "
  "WHERE `ID_first_value`=? AND `ID_second_value`=?";

  const char access::object_traits_impl< ::TR::Core::LinkData, id_mysql >::query_statement[] =
  "SELECT "
  "`LinkData`.`ID_first_value`, "
  "`LinkData`.`ID_second_value` "
  "FROM `LinkData`";

  const char access::object_traits_impl< ::TR::Core::LinkData, id_mysql >::erase_query_statement[] =
  "DELETE FROM `LinkData`";

  const char access::object_traits_impl< ::TR::Core::LinkData, id_mysql >::table_name[] =
  "`LinkData`";

  void access::object_traits_impl< ::TR::Core::LinkData, id_mysql >::
  persist (database& db, const object_type& obj)
  {
    ODB_POTENTIALLY_UNUSED (db);

    using namespace mysql;

    mysql::connection& conn (
      mysql::transaction::current ().connection ());
    statements_type& sts (
      conn.statement_cache ().find_object<object_type> ());

    callback (db,
              obj,
              callback_event::pre_persist);

    image_type& im (sts.image ());
    binding& imb (sts.insert_image_binding ());

    if (init (im, obj, statement_insert))
      im.version++;

    if (im.version != sts.insert_image_version () ||
        imb.version == 0)
    {
      bind (imb.bind, im, statement_insert);
      sts.insert_image_version (im.version);
      imb.version++;
    }

    insert_statement& st (sts.persist_statement ());
    if (!st.execute ())
      throw object_already_persistent ();

    callback (db,
              obj,
              callback_event::post_persist);
  }

  void access::object_traits_impl< ::TR::Core::LinkData, id_mysql >::
  update (database& db, const object_type& obj)
  {
    ODB_POTENTIALLY_UNUSED (db);

    using namespace mysql;
    using mysql::update_statement;

    callback (db, obj, callback_event::pre_update);

    callback (db, obj, callback_event::post_update);
    pointer_cache_traits::update (db, obj);
  }

  void access::object_traits_impl< ::TR::Core::LinkData, id_mysql >::
  erase (database& db, const id_type& id)
  {
    using namespace mysql;

    ODB_POTENTIALLY_UNUSED (db);

    mysql::connection& conn (
      mysql::transaction::current ().connection ());
    statements_type& sts (
      conn.statement_cache ().find_object<object_type> ());

    id_image_type& i (sts.id_image ());
    init (i, id);

    binding& idb (sts.id_image_binding ());
    if (i.version != sts.id_image_version () || idb.version == 0)
    {
      bind (idb.bind, i);
      sts.id_image_version (i.version);
      idb.version++;
    }

    if (sts.erase_statement ().execute () != 1)
      throw object_not_persistent ();

    pointer_cache_traits::erase (db, id);
  }

  access::object_traits_impl< ::TR::Core::LinkData, id_mysql >::pointer_type
  access::object_traits_impl< ::TR::Core::LinkData, id_mysql >::
  find (database& db, const id_type& id)
  {
    using namespace mysql;

    {
      pointer_type p (pointer_cache_traits::find (db, id));

      if (!pointer_traits::null_ptr (p))
        return p;
    }

    mysql::connection& conn (
      mysql::transaction::current ().connection ());
    statements_type& sts (
      conn.statement_cache ().find_object<object_type> ());

    statements_type::auto_lock l (sts);

    if (l.locked ())
    {
      if (!find_ (sts, &id))
        return pointer_type ();
    }

    pointer_type p (
      access::object_factory<object_type, pointer_type>::create ());
    pointer_traits::guard pg (p);

    pointer_cache_traits::insert_guard ig (
      pointer_cache_traits::insert (db, id, p));

    object_type& obj (pointer_traits::get_ref (p));

    if (l.locked ())
    {
      select_statement& st (sts.find_statement ());
      ODB_POTENTIALLY_UNUSED (st);

      callback (db, obj, callback_event::pre_load);
      init (obj, sts.image (), &db);
      load_ (sts, obj, false);
      sts.load_delayed (0);
      l.unlock ();
      callback (db, obj, callback_event::post_load);
      pointer_cache_traits::load (ig.position ());
    }
    else
      sts.delay_load (id, obj, ig.position ());

    ig.release ();
    pg.release ();
    return p;
  }

  bool access::object_traits_impl< ::TR::Core::LinkData, id_mysql >::
  find (database& db, const id_type& id, object_type& obj)
  {
    using namespace mysql;

    mysql::connection& conn (
      mysql::transaction::current ().connection ());
    statements_type& sts (
      conn.statement_cache ().find_object<object_type> ());

    statements_type::auto_lock l (sts);

    if (!find_ (sts, &id))
      return false;

    select_statement& st (sts.find_statement ());
    ODB_POTENTIALLY_UNUSED (st);

    reference_cache_traits::position_type pos (
      reference_cache_traits::insert (db, id, obj));
    reference_cache_traits::insert_guard ig (pos);

    callback (db, obj, callback_event::pre_load);
    init (obj, sts.image (), &db);
    load_ (sts, obj, false);
    sts.load_delayed (0);
    l.unlock ();
    callback (db, obj, callback_event::post_load);
    reference_cache_traits::load (pos);
    ig.release ();
    return true;
  }

  bool access::object_traits_impl< ::TR::Core::LinkData, id_mysql >::
  reload (database& db, object_type& obj)
  {
    using namespace mysql;

    mysql::connection& conn (
      mysql::transaction::current ().connection ());
    statements_type& sts (
      conn.statement_cache ().find_object<object_type> ());

    statements_type::auto_lock l (sts);

    // From LinkData.hxx:34:14
    const id_type& id  (
      ::TR::Core::LinkID (obj.m_parent_key, obj.m_child_key));

    if (!find_ (sts, &id))
      return false;

    select_statement& st (sts.find_statement ());
    ODB_POTENTIALLY_UNUSED (st);

    callback (db, obj, callback_event::pre_load);
    init (obj, sts.image (), &db);
    load_ (sts, obj, true);
    sts.load_delayed (0);
    l.unlock ();
    callback (db, obj, callback_event::post_load);
    return true;
  }

  bool access::object_traits_impl< ::TR::Core::LinkData, id_mysql >::
  find_ (statements_type& sts,
         const id_type* id)
  {
    using namespace mysql;

    id_image_type& i (sts.id_image ());
    init (i, *id);

    binding& idb (sts.id_image_binding ());
    if (i.version != sts.id_image_version () || idb.version == 0)
    {
      bind (idb.bind, i);
      sts.id_image_version (i.version);
      idb.version++;
    }

    image_type& im (sts.image ());
    binding& imb (sts.select_image_binding ());

    if (im.version != sts.select_image_version () ||
        imb.version == 0)
    {
      bind (imb.bind, im, statement_select);
      sts.select_image_version (im.version);
      imb.version++;
    }

    select_statement& st (sts.find_statement ());

    st.execute ();
    auto_result ar (st);
    select_statement::result r (st.fetch ());

    return r != select_statement::no_data;
  }

  result< access::object_traits_impl< ::TR::Core::LinkData, id_mysql >::object_type >
  access::object_traits_impl< ::TR::Core::LinkData, id_mysql >::
  query (database&, const query_base_type& q)
  {
    using namespace mysql;
    using odb::details::shared;
    using odb::details::shared_ptr;

    mysql::connection& conn (
      mysql::transaction::current ().connection ());

    statements_type& sts (
      conn.statement_cache ().find_object<object_type> ());

    image_type& im (sts.image ());
    binding& imb (sts.select_image_binding ());

    if (im.version != sts.select_image_version () ||
        imb.version == 0)
    {
      bind (imb.bind, im, statement_select);
      sts.select_image_version (im.version);
      imb.version++;
    }

    std::string text (query_statement);
    if (!q.empty ())
    {
      text += " ";
      text += q.clause ();
    }

    q.init_parameters ();
    shared_ptr<select_statement> st (
      new (shared) select_statement (
        conn,
        text,
        false,
        true,
        q.parameters_binding (),
        imb));

    st->execute ();

    shared_ptr< odb::object_result_impl<object_type> > r (
      new (shared) mysql::object_result_impl<object_type> (
        q, st, sts, 0));

    return result<object_type> (r);
  }

  unsigned long long access::object_traits_impl< ::TR::Core::LinkData, id_mysql >::
  erase_query (database&, const query_base_type& q)
  {
    using namespace mysql;

    mysql::connection& conn (
      mysql::transaction::current ().connection ());

    std::string text (erase_query_statement);
    if (!q.empty ())
    {
      text += ' ';
      text += q.clause ();
    }

    q.init_parameters ();
    delete_statement st (
      conn,
      text,
      q.parameters_binding ());

    return st.execute ();
  }
}

namespace odb
{
  static bool
  create_schema (database& db, unsigned short pass, bool drop)
  {
    ODB_POTENTIALLY_UNUSED (db);
    ODB_POTENTIALLY_UNUSED (pass);
    ODB_POTENTIALLY_UNUSED (drop);

    if (drop)
    {
      switch (pass)
      {
        case 1:
        {
          return true;
        }
        case 2:
        {
          db.execute ("DROP TABLE IF EXISTS `LinkData`");
          return false;
        }
      }
    }
    else
    {
      switch (pass)
      {
        case 1:
        {
          db.execute ("CREATE TABLE `LinkData` (\n"
                      "  `ID_first_value` INT NOT NULL,\n"
                      "  `ID_second_value` INT NOT NULL,\n"
                      "  PRIMARY KEY (`ID_first_value`,\n"
                      "               `ID_second_value`))\n"
                      " ENGINE=InnoDB");
          return false;
        }
      }
    }

    return false;
  }

  static const schema_catalog_create_entry
  create_schema_entry_ (
    id_mysql,
    "",
    &create_schema);
}

#include <odb/post.hxx>
