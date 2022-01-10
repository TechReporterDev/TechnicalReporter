#include "stdafx.h"
#include "Mailing.h"
#include "CoreDomain.h"
#include "Security.h"
#include "Notifier.h"
#include "SummaryCollector.h"
#include "JobManager.h"
#include "Transaction.h"
#include "Tasks.h"
#include "ServicesPacking.h"
#include "MailingData-odb.hxx"
namespace TR { namespace Core {

class SendMailTask: public Task, public std::enable_shared_from_this<SendMailTask>
{
public:
    SendMailTask(Mailing& mailing, Key user_key):
        Task(*mailing.m_executive),
        m_mailing(mailing),
        m_core_domain(mailing.m_core_domain),
        m_user_key(user_key)
    {
    }

protected:
    class Routine
    {
    public:
        Routine(std::shared_ptr<SendMailTask> task):
            m_task(task),
            m_mailing(task->m_mailing),
            m_security(*task->m_mailing.m_security)
        {
        }

        std::function<void()> prepare_functor()
        {
            auto task = m_task.lock();
            if (!task)
            {
                return nullptr;
            }

            auto smtp = m_mailing.m_smtp;
            if (!smtp)
            {
                return nullptr;
            }

            auto mail = task->make_mail();
            if (!mail.is_valid())
            {
                return nullptr;
            }

            return [smtp, mail]{
                send_mail(*smtp, mail);
            };
        }

        void on_completed() const
        {
            if (auto task = m_task.lock())
            {
                task->on_mail_sent();
                task->emit_completed();
            }
        }

        void on_failed(const std::exception& err) const
        {
            if (auto task = m_task.lock())
            {
                task->emit_failed(err);
            }
        }

        void on_cancel() const
        {
            if (auto task = m_task.lock())
            {
                task->emit_completed();
            }
        }

    protected:
        std::weak_ptr<SendMailTask> m_task;
        Mailing& m_mailing;
        Security& m_security;
    };

    // Job override
    virtual void do_run() override
    {
        m_executive.run_processing(Routine(shared_from_this()));
    }

    virtual Mail make_mail()
    {
        auto mail = Mail( 
            m_mailing.m_email_from, 
            {},
            L"",
            L""
        );

        if (auto user_email = m_mailing.get_email(m_user_key))
        {
            mail.m_to.push_back(std::move(*user_email));
        }

        return mail;
    }

    virtual void on_mail_sent()
    {
    }

    Mailing& m_mailing;
    CoreDomain& m_core_domain;
    Key m_user_key;
};

class SendIntegrityCheckingMail: public SendMailTask
{
public:
    SendIntegrityCheckingMail(Mailing& mailing, Key user_key, SourceRef source_ref, ReportTypeRef report_type_ref):
        SendMailTask(mailing, user_key),
        m_source_key(source_ref.get_key()),
        m_report_type_uuid(report_type_ref.get_uuid())
    {
    }

protected:
    virtual Mail make_mail()
    {
        auto source = m_core_domain.m_registry.m_sources.get_source(m_source_key);
        auto& report_type = m_core_domain.m_basis.m_report_types.get_report_type(m_report_type_uuid);
        
        auto mail = SendMailTask::make_mail();
        mail.m_title = L"Integrity checking failed";
        mail.m_body = (boost::wformat(L"Integrity checking failed for source:%1% and report:%2%") % source.get_name() % report_type.get_name()).str();
        return mail;
    }   

private:
    SourceKey m_source_key; 
    ReportTypeUUID m_report_type_uuid;
};

class SendComplianceCheckingMail: public SendMailTask
{
public:
    SendComplianceCheckingMail(Mailing& mailing, Key user_key, SourceRef source_ref, ReportTypeRef report_type_ref):
        SendMailTask(mailing, user_key),
        m_source_key(source_ref.get_key()),
        m_report_type_uuid(report_type_ref.get_uuid())
    {
    }

protected:
    virtual Mail make_mail()
    {
        auto source = m_core_domain.m_registry.m_sources.get_source(m_source_key);
        auto& report_type = m_core_domain.m_basis.m_report_types.get_report_type(m_report_type_uuid);
        
        auto mail = SendMailTask::make_mail();
        mail.m_title = L"Compliance checking failed";
        mail.m_body = (boost::wformat(L"Compliance checking failed for source:%1% and report:%2%") % source.get_name() % report_type.get_name()).str();
        return mail;
    }   

private:
    SourceKey m_source_key; 
    ReportTypeUUID m_report_type_uuid;
};

class SendValidationMail: public SendMailTask
{
public:
    SendValidationMail(Mailing& mailing, Key user_key, SourceRef source_ref, ReportTypeRef report_type_ref):
        SendMailTask(mailing, user_key),
        m_source_key(source_ref.get_key()),
        m_report_type_uuid(report_type_ref.get_uuid())
    {
    }

protected:
    virtual Mail make_mail()
    {
        auto source = m_core_domain.m_registry.m_sources.get_source(m_source_key);
        auto& report_type = m_core_domain.m_basis.m_report_types.get_report_type(m_report_type_uuid);
        
        auto mail = SendMailTask::make_mail();
        mail.m_title = L"Validation failed";
        mail.m_body = (boost::wformat(L"Validation failed for source:%1% and report:%2%") % source.get_name() % report_type.get_name()).str();
        return mail;
    }   

private:
    SourceKey m_source_key; 
    ReportTypeUUID m_report_type_uuid;
};

class SendMessageRecievedMail: public SendMailTask
{
public:
    SendMessageRecievedMail(Mailing& mailing, Key user_key, SourceRef source_ref, StreamTypeRef stream_type_ref):
        SendMailTask(mailing, user_key),
        m_source_key(source_ref.get_key()),
        m_stream_type_uuid(stream_type_ref.get_uuid())
    {
    }

protected:
    virtual Mail make_mail()
    {
        auto source = m_core_domain.m_registry.m_sources.get_source(m_source_key);
        auto& stream_type = m_core_domain.m_basis.m_stream_types.get_stream_type(m_stream_type_uuid);
        
        auto mail = SendMailTask::make_mail();
        mail.m_title = L"Stream message recieved";
        mail.m_body = (boost::wformat(L"Message recieved for source:%1% from stream:%2%") % source.get_name() % stream_type.get_name()).str();
        return mail;
    }   

private:
    SourceKey m_source_key; 
    StreamTypeUUID m_stream_type_uuid;
};

class Mailing::MailSender: public NotificatonSender
{
public:
    MailSender(Mailing& mailing):
        m_mailing(mailing)
    {
    }

    virtual void send_integrity_checking_failed(Key user_key, SourceRef source_ref, ReportTypeRef report_type_ref) override
    {
        m_job_manager.run_job(0, std::make_shared<SendIntegrityCheckingMail>(m_mailing, user_key, source_ref, report_type_ref));
    }

    virtual void send_compliance_checking_failed(Key user_key, SourceRef source_ref, ReportTypeRef report_type_ref) override
    {
        m_job_manager.run_job(0, std::make_shared<SendComplianceCheckingMail>(m_mailing, user_key, source_ref, report_type_ref));
    }

    virtual void send_validation_failed(Key user_key, SourceRef source_ref, ReportTypeRef report_type_ref) override
    {
        m_job_manager.run_job(0, std::make_shared<SendValidationMail>(m_mailing, user_key, source_ref, report_type_ref));
    }

    virtual void send_stream_record_recieved(Key user_key, const StreamRecord& stream_record) override
    {
        m_job_manager.run_job(0, std::make_shared<SendMessageRecievedMail>(m_mailing, user_key, stream_record.m_source_ref, stream_record.m_stream_type_ref));
    }

private:
    Mailing& m_mailing;
    JobManager<int> m_job_manager;
};

class SendSummaryMail : public SendMailTask
{
public:
    SendSummaryMail(Mailing& mailing, Key user_key) :
        SendMailTask(mailing, user_key),
        m_sent_summary_time(0)
    {
    }

protected:
    virtual Mail make_mail() override
    {
        auto summary = m_mailing.m_summary_collector->find_last_summary(m_user_key);        
        if(!summary)
        {
            return Mail();
        }

        if (summary->get_time() == m_mailing.get_sent_summary_time(m_user_key))
        {
            return Mail();
        }
        m_sent_summary_time = summary->get_time();
        auto mail = SendMailTask::make_mail();
        mail.m_title = L"Technical reporter summary";
        mail.m_body = L"Look attachment for summary pdf file";
        mail.m_attachments.push_back({ L"Summary.pdf", summary->get_summary_pdf() });
        return mail;
    }

    virtual void on_mail_sent()
    {
        m_mailing.set_sent_summary_time(m_user_key, m_sent_summary_time);
    }

private:
    SourceKey m_source_key;
    StreamTypeUUID m_stream_type_uuid;
    time_t m_sent_summary_time;
};

class Mailing::SummarySender
{
public:
    SummarySender(Mailing& mailing) :
        m_mailing(mailing)
    {
    }

    bool is_busy()
    {
        return m_job_manager.is_empty(0);
    }

    void send_summary()
    {
        if (!m_job_manager.is_empty(0))
        {
            return;
        }

        for (auto& user : m_mailing.m_security->get_users())
        {
            m_job_manager.run_job(0, std::make_shared<SendSummaryMail>(m_mailing, user.m_key));
        }
    }

private:
    Mailing& m_mailing;
    JobManager<int> m_job_manager;
};

Mailing::Mailing(Database& db, CoreDomain& core_domain):
    m_db(db),    
    m_core_domain(core_domain),
    m_security(nullptr),
    m_summary_collector(nullptr),
    m_notifier(nullptr),
    m_executive(nullptr),
    m_summary_sender(std::make_unique<SummarySender>(*this))
{ 
}

Mailing::~Mailing()
{
}

void Mailing::set_security(Security* security)
{
    _ASSERT(!m_security);
    m_security = security;
    m_security->connect_remove_user([this](Key user_key, Transaction& t){
        on_remove_user(user_key, t);
    }, MAILING_PRIORITY);
}

void Mailing::set_notifier(Notifier* notifier)
{
    _ASSERT(!m_notifier);
    m_notifier = notifier;
    m_notifier->add_sender(std::make_unique<MailSender>(*this));
}

void Mailing::set_summary_collector(SummaryCollector* summary_collector)
{
    _ASSERT(!m_summary_collector);
    m_summary_collector = summary_collector;
}

void Mailing::run(Executive* executive, Transaction& t)
{
    _ASSERT(!m_executive);
    transact_assign(m_executive, executive, t);
 
    t.connect_commit([this]()
    {
        send_summary();
    });   
}

void Mailing::send_summary()
{
    no_except([&] {        
        m_summary_sender->send_summary();
        m_executive->async(std::bind(&Mailing::send_summary, this), time(nullptr) + 60);
    });
}

time_t Mailing::get_sent_summary_time(Key user_key) const
{    
    ReadOnlyTransaction t(m_db);
    SentMailData sent_mail_data;
    if (m_db.find<SentMailData>(user_key, sent_mail_data))
    {
        return sent_mail_data.m_sent_summary_time;
    }

    return 0;
}

void Mailing::set_sent_summary_time(Key user_key, time_t time)
{    
    Transaction t(m_db);
    SentMailData sent_mail_data;
    if (m_db.find(user_key, sent_mail_data))
    {
        sent_mail_data.m_sent_summary_time = time;
        m_db.update(sent_mail_data);
    }
    else
    {
        m_db.persist(SentMailData(user_key, time));
    }    
    t.commit();
}

void Mailing::setup(SMTP smtp, std::wstring email_from)
{
    m_smtp = std::move(smtp);
    m_email_from = std::move(email_from);   
}

void Mailing::set_email(Key user_key, boost::optional<std::wstring> user_email, Transaction& t)
{
    m_db.erase_query<MailingData>(odb::query<MailingData>::user_key == user_key);
    if (user_email)
    {
        m_db.persist(MailingData{user_key, pack(*user_email)});
    }
}

boost::optional<std::wstring> Mailing::get_email(Key user_key) const
{
    ReadOnlyTransaction t(m_db);
    MailingData mailing_data;
    if (m_db.find(user_key, mailing_data))
    {
        return unpack(mailing_data.m_user_email);
    }
    return boost::none;
}

void Mailing::on_remove_user(Key user_key, Transaction& t)
{
    m_db.erase<MailingData>(user_key);
}

}} //namespace TR { namespace Core {