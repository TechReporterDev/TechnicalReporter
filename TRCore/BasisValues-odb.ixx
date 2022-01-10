// This file was generated by ODB, object-relational mapping (ORM)
// compiler for C++.
//

namespace odb
{
}

namespace odb
{
  // ReportTypeUUID
  //

  inline
  bool access::composite_value_traits< ::TR::Core::ReportTypeUUID, id_mysql >::
  get_null (const image_type& i)
  {
    bool r (true);
    r = r && composite_value_traits< ::TR::Core::UUIDRestriction, id_mysql >::get_null (i);
    return r;
  }

  inline
  void access::composite_value_traits< ::TR::Core::ReportTypeUUID, id_mysql >::
  set_null (image_type& i,
            mysql::statement_kind sk)
  {
    ODB_POTENTIALLY_UNUSED (sk);

    using namespace mysql;

    composite_value_traits< ::TR::Core::UUIDRestriction, id_mysql >::set_null (i, sk);
  }

  // StreamTypeUUID
  //

  inline
  bool access::composite_value_traits< ::TR::Core::StreamTypeUUID, id_mysql >::
  get_null (const image_type& i)
  {
    bool r (true);
    r = r && composite_value_traits< ::TR::Core::UUIDRestriction, id_mysql >::get_null (i);
    return r;
  }

  inline
  void access::composite_value_traits< ::TR::Core::StreamTypeUUID, id_mysql >::
  set_null (image_type& i,
            mysql::statement_kind sk)
  {
    ODB_POTENTIALLY_UNUSED (sk);

    using namespace mysql;

    composite_value_traits< ::TR::Core::UUIDRestriction, id_mysql >::set_null (i, sk);
  }

  // SourceTypeUUID
  //

  inline
  bool access::composite_value_traits< ::TR::Core::SourceTypeUUID, id_mysql >::
  get_null (const image_type& i)
  {
    bool r (true);
    r = r && composite_value_traits< ::TR::Core::UUIDRestriction, id_mysql >::get_null (i);
    return r;
  }

  inline
  void access::composite_value_traits< ::TR::Core::SourceTypeUUID, id_mysql >::
  set_null (image_type& i,
            mysql::statement_kind sk)
  {
    ODB_POTENTIALLY_UNUSED (sk);

    using namespace mysql;

    composite_value_traits< ::TR::Core::UUIDRestriction, id_mysql >::set_null (i, sk);
  }

  // ActionUUID
  //

  inline
  bool access::composite_value_traits< ::TR::Core::ActionUUID, id_mysql >::
  get_null (const image_type& i)
  {
    bool r (true);
    r = r && composite_value_traits< ::TR::Core::UUIDRestriction, id_mysql >::get_null (i);
    return r;
  }

  inline
  void access::composite_value_traits< ::TR::Core::ActionUUID, id_mysql >::
  set_null (image_type& i,
            mysql::statement_kind sk)
  {
    ODB_POTENTIALLY_UNUSED (sk);

    using namespace mysql;

    composite_value_traits< ::TR::Core::UUIDRestriction, id_mysql >::set_null (i, sk);
  }

  // ResourceIdentity
  //

  inline
  bool access::composite_value_traits< ::TR::Core::ResourceIdentity, id_mysql >::
  get_null (const image_type& i)
  {
    bool r (true);
    r = r && composite_value_traits< ::TR::Core::ReportTypeUUID, id_mysql >::get_null (i.m_value1_value);
    r = r && composite_value_traits< ::TR::Core::StreamTypeUUID, id_mysql >::get_null (i.m_value2_value);
    return r;
  }

  inline
  void access::composite_value_traits< ::TR::Core::ResourceIdentity, id_mysql >::
  set_null (image_type& i,
            mysql::statement_kind sk)
  {
    ODB_POTENTIALLY_UNUSED (sk);

    using namespace mysql;

    composite_value_traits< ::TR::Core::ReportTypeUUID, id_mysql >::set_null (i.m_value1_value, sk);
    composite_value_traits< ::TR::Core::StreamTypeUUID, id_mysql >::set_null (i.m_value2_value, sk);
  }
}
