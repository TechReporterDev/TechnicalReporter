#include "stdafx.h"
#include "Mail.h"
#include <iomanip>
//#include <curl\curl.h>
#include <curlpp\cURLpp.hpp>
#include <curlpp\Easy.hpp>
#include <curlpp\Options.hpp>
#include <curlpp\Exception.hpp>
#include <boost\archive\iterators\binary_from_base64.hpp>
#include <boost\archive\iterators\base64_from_binary.hpp>
#include <boost\archive\iterators\transform_width.hpp>

namespace {

std::string format_smtp_server_url(const TR::Core::SMTP& smtp)
{
    std::stringstream url;
    url << "smtp://" << stl_tools::ucs_to_utf8(smtp.m_server);
    return url.str();
}

std::string format_email_list(const std::vector<std::wstring>& emails)
{
    std::stringstream email_list;
    bool count = 0;
    for (const auto& email : emails)
    {
        if (count++)
        {
            email_list << " ";
        }

        email_list << "<" << stl_tools::ucs_to_utf8(email) << ">";
    }
    return email_list.str();
}

std::string to_base64(const std::vector<char>& data)
{
    using namespace boost::archive::iterators;
    using It = base64_from_binary<transform_width<std::vector<char>::const_iterator, 6, 8>>;
    auto tmp = std::string(It(std::begin(data)), It(std::end(data)));
    return tmp.append((3 - data.size() % 3) % 3, '=');

    //return "JVBERi0xLjMKJeLjz9MKNSAwIG9iajw8L0ZpbHRlci9GbGF0ZURlY29kZS9MZW5ndGggNiAwIFI+PgpzdHJlYW0KeJytmtuK5DYQhu/7KfoJJl0qldSGkIuZ7QnkbpKBPEBOEDKBXQKBPH1kuy3LqtpW6cDATK/XdvUv/VJ9Kun5/fTN6z/+bC5Pl8vl/P77CS73j/+d7h9+Oa3/e/LeLH/ffz1/a1+9c+RN+D2Fv/678/ufp9v76bnqhXZ/IaF79eGFDsPr0FzcVwJ8PsHyln/DJ3P+4f7O9PeP359w/RTes/z9OJvJP1Fy5a/zT/KT+Y0fZ3J0eNn86FsIvsWA9bJx64N34V9+O/98/luI8eWP+VnIniXh2a0p4WFTrq86ebg+EeLaN0tTupu3cwNuDfcWfh42FmyKs9abBX/eG6ZFr5mu2cPtgu/vOigm58C9BM1UVJvrqPBGdAI88obCljw0aG3JQ4NoSze123J/ttuWbko6yQZb2rmjtJbcvshuSZAs2aR1s+QIsZslU7WIaivuMtV+2Hp/f1TyQ9mKPPR2pWhFHjp9NLWi77CiH2dFn1jROwip5hZSy815X+6qTEo0ZCo5MWSL4mjIAZKjIRPNeNEb0le7InrAP3KFwpA8tNcakof2siGpw5A0zpCUpOyZdEyYH02FGSk3oxfN2KI2mnGA3GhGajMjVTsi9j89coTCjDw0ac3IQ1Nqxrc6UnYmIeXQGf6yeMWOImXHkMRpkcQxJHEyklAHktA4JKGpl5QpxxInYkmT3m3gjRC8DbxUcQMpE2cELZ4QZ4QqPOGhSYsnPDTJeEIdeELj8IR8DylTDiYkgkmT1mjJgWCSqkWrtyKjg6IfYu8zOqAqMBFCa8FECC2DCXWACY0DE0rBZFpnx0DJxk0h4Uwh4aC7hivonld+Dlca8IVyfCERX5raJFp2IL5QG74QY4iib6JLGENQFb4IobX4IoQm2bLYYVkcZ1k8JnX3KRDSS7AnBINiQK82i2JuURIt2tIG0aID";
}

std::string make_payload(const TR::Core::Mail& mail)
{
    std::stringstream payload;
    auto now = time(nullptr);
    tm now_tm;
    gmtime_s(&now_tm, &now);
    
    /*payload
        << "Date: " << std::put_time(&now_tm, "%a, %d %b %Y %H:%M:%S %z") << std::endl
        << "To: " << format_email_list(mail.m_to) << " " << std::endl
        << "From: " << format_email_list({ mail.m_from }) << " " << std::endl
        << "Message-ID: <dcd7cb36-11db-487a-9f3a-e652a9458efd@rfcpedant.example.org>" << std::endl
        << "Subject: " << stl_tools::ucs_to_utf8(mail.m_title) << std::endl
        << "MIME-Version: 1.0" << std::endl
        << "Content-Type: multipart/mixed;" << std::endl
        << " boundary=\"------------BC4F6F07421062CAFD2D0F20\"" << std::endl
        << std::endl
        << "This is a multi-part message in MIME format." << std::endl
        << "--------------BC4F6F07421062CAFD2D0F20" << std::endl
        << "Content-Type: text/plain; charset=utf-8; format=flowed" << std::endl
        << std::endl
        << stl_tools::ucs_to_utf8(mail.m_body) << std::endl
        << std::endl
        << std::endl
        << "--------------BC4F6F07421062CAFD2D0F20" << std::endl
        << "Content-Type: image/jpeg;" << std::endl
        << " name=\"image.jpeg\"" << std::endl
        << "Content-Transfer-Encoding: base64" << std::endl
        << "Content-Disposition: attachment;" << std::endl
        << " filename=\"image.jpeg\"" << std::endl
        << std::endl
        << "iVBORw0KGgoAAAANSUhEUgAAACwAAAAsCAYAAAAehFoBAAAA0klEQVRYw+3YwQnCQBCF4SkhJQQrSQkpYUuwAy3BEjxYgFc9pQRLSAeKDfwefIs5LOptE/IGHktgDh/DEJYNIJaUMNhgg5cLboEjcONdo77bOYITnzoDe5250pzAGTtomknYATho0hnd1QY3wENrEFoBNOFm0jOqr1NPNXAvYD+Z9PZH301nFfBekBBk/NKbJ5/3uzo4g/4BYzB1y2CDDS7Vs98UA7C7RjEGG2zwmsCL+63F6V4MAJcox2CDDV4R2NdLg2cOTpWwye/DBhts8IrAL/M0tq2fkxMJAAAAAElFTkSuQmCC" << std::endl
        << "--------------BC4F6F07421062CAFD2D0F20--";*/

    payload
        << "Date: " << std::put_time(&now_tm, "%a, %d %b %Y %H:%M:%S %z") << std::endl
        << "To: " << format_email_list(mail.m_to) << " " << std::endl
        << "From: " << format_email_list({ mail.m_from }) << " " << std::endl
        << "Message-ID: <dcd7cb36-11db-487a-9f3a-e652a9458efd@rfcpedant.example.org>" << std::endl
        << "Subject: " << stl_tools::ucs_to_utf8(mail.m_title) << std::endl
        << "MIME-Version: 1.0" << std::endl
        << "Content-Type: multipart/mixed;" << std::endl
        << " boundary=\"------------BC4F6F07421062CAFD2D0F20\"" << std::endl
        << std::endl
        << "This is a multi-part message in MIME format." << std::endl
        << "--------------BC4F6F07421062CAFD2D0F20" << std::endl
        << "Content-Type: text/plain; charset=utf-8; format=flowed" << std::endl
        << std::endl
        << stl_tools::ucs_to_utf8(mail.m_body) << std::endl
        << std::endl
        << std::endl;

    for (auto& attachment : mail.m_attachments)
    {
        payload
            << "--------------BC4F6F07421062CAFD2D0F20" << std::endl
            << "Content-Type: application/pdf;" << std::endl
            << " name=\"" << stl_tools::ucs_to_utf8(attachment.m_name) << "\"" << std::endl
            << "Content-Transfer-Encoding: base64" << std::endl
            << "Content-Disposition: attachment;" << std::endl
            << " filename=\"" << stl_tools::ucs_to_utf8(attachment.m_name) << "\"" << std::endl
            << std::endl;

        auto base64 = to_base64(attachment.m_content);        
        for (size_t first = 0, count = std::min(size_t(1000), base64.size());
            count != 0;  first += count, count = std::min(size_t(1000), base64.size() - first))
        {
            payload.write(&base64[first], count);
            payload << std::endl;
        }
        payload << std::endl;
    }

    payload 
        << "--------------BC4F6F07421062CAFD2D0F20--";

    return payload.str();    
}

static size_t
StreamReadCallback(char* buffer, size_t size, size_t nitems, std::istream* stream)
{
    stream->read(buffer, static_cast<std::streamsize>(size * nitems));
    size_t realread = stream->gcount();
    if (!realread && !(stream->eof()))
        realread = CURL_READFUNC_ABORT;

    return realread;
};

}

namespace curlpp {
namespace options {
using MailFrom = curlpp::OptionTrait<std::string, CURLOPT_MAIL_FROM>;
using Recipients = curlpp::OptionTrait<std::list<std::string>, CURLOPT_MAIL_RCPT>;
using ReadStreamFunction = curlpp::OptionTrait<void*, CURLOPT_READFUNCTION>;
}
}

namespace TR { namespace Core {

void send_mail(const SMTP& smtp, const Mail& mail)
{
    curlpp::Cleanup cleanup;
    curlpp::Easy curl;

    curl.setOpt(curlpp::options::Url(format_smtp_server_url(smtp)));
    curl.setOpt(curlpp::options::MailFrom(stl_tools::ucs_to_utf8(mail.m_from)));

    std::list<std::string> recipients;
    for (const auto& to : mail.m_to)
    {
        recipients.push_back(stl_tools::ucs_to_utf8(to));
    }
    curl.setOpt(curlpp::options::Recipients(std::move(recipients)));

    std::istringstream payloadStream(make_payload(mail));
    curl.setOpt(curlpp::options::ReadStream(&payloadStream));
    curl.setOpt(curlpp::options::ReadStreamFunction(&StreamReadCallback));
    curl.setOpt(curlpp::options::Upload(true));
    curl.perform();
}

}} //namespace TR { namespace Core {