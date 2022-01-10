#pragma once
#include "AppDecl.h"
#include "Mail.h"
namespace TR {  namespace Core {

class Mailing
{
public:
    static const Priority MAILING_PRIORITY = 2100;

    Mailing(Database& db, CoreDomain& core_domain);
    ~Mailing();

    void                            set_security(Security* security);
    void                            set_notifier(Notifier* notifier);
    void                            set_summary_collector(SummaryCollector* m_summary_collector);
    void                            run(Executive* executive, Transaction& t);

    void                            setup(SMTP smtp, std::wstring email_from);
    void                            set_email(Key user_key, boost::optional<std::wstring> user_email, Transaction& t);
    boost::optional<std::wstring>   get_email(Key user_key) const;

private:
    void                            on_remove_user(Key user_key, Transaction& t);
    void                            send_summary();
    time_t                          get_sent_summary_time(Key user_key) const;
    void                            set_sent_summary_time(Key user_key, time_t time);

    friend class SendMailTask;
    friend class SendSummaryMail;
    class MailSender;   
    class SummarySender;
    
    Database& m_db;
    CoreDomain& m_core_domain;    
    Notifier* m_notifier;
    Security* m_security;
    SummaryCollector* m_summary_collector;
    Executive* m_executive;

    boost::optional<SMTP> m_smtp;
    std::wstring m_email_from;

    std::unique_ptr<SummarySender> m_summary_sender;
};

}} //namespace TR { namespace Core {