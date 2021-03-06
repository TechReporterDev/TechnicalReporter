// This file was generated by ODB, object-relational mapping (ORM)
// compiler for C++.
//

namespace odb
{
}

namespace odb
{
  // SourceKey
  //

  inline
  bool access::composite_value_traits< ::TR::Core::SourceKey, id_mysql >::
  get_null (const image_type& i)
  {
    bool r (true);
    r = r && composite_value_traits< ::TR::Core::KeyRestriction, id_mysql >::get_null (i);
    return r;
  }

  inline
  void access::composite_value_traits< ::TR::Core::SourceKey, id_mysql >::
  set_null (image_type& i,
            mysql::statement_kind sk)
  {
    ODB_POTENTIALLY_UNUSED (sk);

    using namespace mysql;

    composite_value_traits< ::TR::Core::KeyRestriction, id_mysql >::set_null (i, sk);
  }

  // RoleKey
  //

  inline
  bool access::composite_value_traits< ::TR::Core::RoleKey, id_mysql >::
  get_null (const image_type& i)
  {
    bool r (true);
    r = r && composite_value_traits< ::TR::Core::KeyRestriction, id_mysql >::get_null (i);
    return r;
  }

  inline
  void access::composite_value_traits< ::TR::Core::RoleKey, id_mysql >::
  set_null (image_type& i,
            mysql::statement_kind sk)
  {
    ODB_POTENTIALLY_UNUSED (sk);

    using namespace mysql;

    composite_value_traits< ::TR::Core::KeyRestriction, id_mysql >::set_null (i, sk);
  }

  // SubjectIdentity
  //

  inline
  bool access::composite_value_traits< ::TR::Core::SubjectIdentity, id_mysql >::
  get_null (const image_type& i)
  {
    bool r (true);
    r = r && composite_value_traits< ::TR::Core::SourceKey, id_mysql >::get_null (i.m_value1_value);
    r = r && composite_value_traits< ::TR::Core::RoleKey, id_mysql >::get_null (i.m_value2_value);
    return r;
  }

  inline
  void access::composite_value_traits< ::TR::Core::SubjectIdentity, id_mysql >::
  set_null (image_type& i,
            mysql::statement_kind sk)
  {
    ODB_POTENTIALLY_UNUSED (sk);

    using namespace mysql;

    composite_value_traits< ::TR::Core::SourceKey, id_mysql >::set_null (i.m_value1_value, sk);
    composite_value_traits< ::TR::Core::RoleKey, id_mysql >::set_null (i.m_value2_value, sk);
  }
}

