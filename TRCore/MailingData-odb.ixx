// This file was generated by ODB, object-relational mapping (ORM)
// compiler for C++.
//

namespace odb
{
  // MailingData
  //

  inline
  access::object_traits< ::TR::Core::MailingData >::id_type
  access::object_traits< ::TR::Core::MailingData >::
  id (const object_type& o)
  {
    return o.m_user_key;
  }

  inline
  void access::object_traits< ::TR::Core::MailingData >::
  callback (database& db, object_type& x, callback_event e)
  {
    ODB_POTENTIALLY_UNUSED (db);
    ODB_POTENTIALLY_UNUSED (x);
    ODB_POTENTIALLY_UNUSED (e);
  }

  inline
  void access::object_traits< ::TR::Core::MailingData >::
  callback (database& db, const object_type& x, callback_event e)
  {
    ODB_POTENTIALLY_UNUSED (db);
    ODB_POTENTIALLY_UNUSED (x);
    ODB_POTENTIALLY_UNUSED (e);
  }

  // SentMailData
  //

  inline
  access::object_traits< ::TR::Core::SentMailData >::id_type
  access::object_traits< ::TR::Core::SentMailData >::
  id (const object_type& o)
  {
    return o.m_user_key;
  }

  inline
  void access::object_traits< ::TR::Core::SentMailData >::
  callback (database& db, object_type& x, callback_event e)
  {
    ODB_POTENTIALLY_UNUSED (db);
    ODB_POTENTIALLY_UNUSED (x);
    ODB_POTENTIALLY_UNUSED (e);
  }

  inline
  void access::object_traits< ::TR::Core::SentMailData >::
  callback (database& db, const object_type& x, callback_event e)
  {
    ODB_POTENTIALLY_UNUSED (db);
    ODB_POTENTIALLY_UNUSED (x);
    ODB_POTENTIALLY_UNUSED (e);
  }
}

namespace odb
{
  // MailingData
  //

  inline
  void access::object_traits_impl< ::TR::Core::MailingData, id_mysql >::
  erase (database& db, const object_type& obj)
  {
    callback (db, obj, callback_event::pre_erase);
    erase (db, id (obj));
    callback (db, obj, callback_event::post_erase);
  }

  inline
  void access::object_traits_impl< ::TR::Core::MailingData, id_mysql >::
  load_ (statements_type& sts,
         object_type& obj,
         bool)
  {
    ODB_POTENTIALLY_UNUSED (sts);
    ODB_POTENTIALLY_UNUSED (obj);
  }

  // SentMailData
  //

  inline
  void access::object_traits_impl< ::TR::Core::SentMailData, id_mysql >::
  erase (database& db, const object_type& obj)
  {
    callback (db, obj, callback_event::pre_erase);
    erase (db, id (obj));
    callback (db, obj, callback_event::post_erase);
  }

  inline
  void access::object_traits_impl< ::TR::Core::SentMailData, id_mysql >::
  load_ (statements_type& sts,
         object_type& obj,
         bool)
  {
    ODB_POTENTIALLY_UNUSED (sts);
    ODB_POTENTIALLY_UNUSED (obj);
  }
}
