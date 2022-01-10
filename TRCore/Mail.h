#pragma once
#include <string>
#include <vector>
namespace TR {namespace Core {

struct Attachment
{
    std::wstring m_name;
    std::vector<char> m_content;
};

struct Mail
{
    Mail(std::wstring from = L"", std::vector<std::wstring> to = {}, std::wstring title = L"", std::wstring body = L""):
        m_from(std::move
        (from)),
        m_to(std::move(to)),
        m_title(std::move(title)),
        m_body(std::move(body))
    {
    }

    bool is_valid() const
    {
        return !m_from.empty() && !m_to.empty() && !m_title.empty() && !m_body.empty();
    }

    std::wstring                m_from;
    std::vector<std::wstring>   m_to;
    std::wstring                m_title;
    std::wstring                m_body;
    std::vector<Attachment>     m_attachments;
};

struct SMTP
{
    enum Security { SECURITY_NORMAL = 1, SECURITY_SSL, SECURITY_TLS };

    SMTP(std::wstring server = L"", std::wstring user = L"", std::wstring password = L"", Security security = SECURITY_NORMAL):
        m_server(std::move(server)),
        m_user(std::move(user)),
        m_password(std::move(password)),
        m_security(m_security)
    {
    }

    std::wstring    m_server;
    std::wstring    m_user;
    std::wstring    m_password;
    Security        m_security; 
};

void send_mail(const SMTP& smtp, const Mail& mail);

}} //namespace TR { namespace Core {