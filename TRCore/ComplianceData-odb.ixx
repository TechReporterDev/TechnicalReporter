// This file was generated by ODB, object-relational mapping (ORM)
// compiler for C++.
//

namespace odb
{
  // ComplianceData
  //

  inline
  access::object_traits< ::TR::Core::ComplianceData >::id_type
  access::object_traits< ::TR::Core::ComplianceData >::
  id (const object_type& o)
  {
    return o.m_key;
  }

  inline
  void access::object_traits< ::TR::Core::ComplianceData >::
  callback (database& db, object_type& x, callback_event e)
  {
    ODB_POTENTIALLY_UNUSED (db);
    ODB_POTENTIALLY_UNUSED (x);
    ODB_POTENTIALLY_UNUSED (e);
  }

  inline
  void access::object_traits< ::TR::Core::ComplianceData >::
  callback (database& db, const object_type& x, callback_event e)
  {
    ODB_POTENTIALLY_UNUSED (db);
    ODB_POTENTIALLY_UNUSED (x);
    ODB_POTENTIALLY_UNUSED (e);
  }

  // SubjectComplianceData
  //

  inline
  access::object_traits< ::TR::Core::SubjectComplianceData >::id_type
  access::object_traits< ::TR::Core::SubjectComplianceData >::
  id (const object_type& o)
  {
    return o.m_identity;
  }

  inline
  void access::object_traits< ::TR::Core::SubjectComplianceData >::
  callback (database& db, object_type& x, callback_event e)
  {
    ODB_POTENTIALLY_UNUSED (db);
    ODB_POTENTIALLY_UNUSED (x);
    ODB_POTENTIALLY_UNUSED (e);
  }

  inline
  void access::object_traits< ::TR::Core::SubjectComplianceData >::
  callback (database& db, const object_type& x, callback_event e)
  {
    ODB_POTENTIALLY_UNUSED (db);
    ODB_POTENTIALLY_UNUSED (x);
    ODB_POTENTIALLY_UNUSED (e);
  }
}

namespace odb
{
  // ComplianceData
  //

  inline
  void access::object_traits_impl< ::TR::Core::ComplianceData, id_mysql >::
  erase (database& db, const object_type& obj)
  {
    callback (db, obj, callback_event::pre_erase);
    erase (db, id (obj));
    callback (db, obj, callback_event::post_erase);
  }

  inline
  void access::object_traits_impl< ::TR::Core::ComplianceData, id_mysql >::
  load_ (statements_type& sts,
         object_type& obj,
         bool)
  {
    ODB_POTENTIALLY_UNUSED (sts);
    ODB_POTENTIALLY_UNUSED (obj);
  }

  // SubjectComplianceIdentity
  //

  inline
  bool access::composite_value_traits< ::TR::Core::SubjectComplianceIdentity, id_mysql >::
  get_null (const image_type& i)
  {
    bool r (true);
    r = r && composite_value_traits< ::TR::Core::SubjectIdentity, id_mysql >::get_null (i.m_subject_value);
    r = r && composite_value_traits< ::TR::Core::ComplianceKey, id_mysql >::get_null (i.m_compliance_key_value);
    return r;
  }

  inline
  void access::composite_value_traits< ::TR::Core::SubjectComplianceIdentity, id_mysql >::
  set_null (image_type& i,
            mysql::statement_kind sk)
  {
    ODB_POTENTIALLY_UNUSED (sk);

    using namespace mysql;

    composite_value_traits< ::TR::Core::SubjectIdentity, id_mysql >::set_null (i.m_subject_value, sk);
    composite_value_traits< ::TR::Core::ComplianceKey, id_mysql >::set_null (i.m_compliance_key_value, sk);
  }

  // SubjectComplianceData
  //

  inline
  void access::object_traits_impl< ::TR::Core::SubjectComplianceData, id_mysql >::
  erase (database& db, const object_type& obj)
  {
    callback (db, obj, callback_event::pre_erase);
    erase (db, id (obj));
    callback (db, obj, callback_event::post_erase);
  }

  inline
  void access::object_traits_impl< ::TR::Core::SubjectComplianceData, id_mysql >::
  load_ (statements_type& sts,
         object_type& obj,
         bool)
  {
    ODB_POTENTIALLY_UNUSED (sts);
    ODB_POTENTIALLY_UNUSED (obj);
  }
}

