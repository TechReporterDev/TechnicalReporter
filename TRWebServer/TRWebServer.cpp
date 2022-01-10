#include "stdafx.h"
#include "TRWebServer.h"
#include "Application.h"
#include "TRCore\FakeCiscoFeature.h"
#include "TRCore\SuitsFeature.h"
#include "TRCore\DirectClient.h"
#include "TRExternalFeature\TRExternalFeature.h"
#include <fstream>
#include <boost\optional.hpp>
#include <boost\filesystem.hpp>
#include <boost\dll\runtime_symbol_info.hpp>
namespace bfs = boost::filesystem;

std::unique_ptr<TR::Core::Core> serverCore;

void scan_features(TR::Core::Core& core, const bfs::path& path)
{
    for (auto& feature_dir : boost::make_iterator_range(bfs::directory_iterator(path), bfs::directory_iterator()))
    {
        if (bfs::exists(feature_dir.path() / L"Feature.xml"))
        {
            core.add_feature(std::make_unique<TR::External::ExternalFeature>(feature_dir.path().wstring()));
        }
        else
        {
            scan_features(core, feature_dir.path());
        }
    }
}

void scan_features(TR::Core::Core& core)
{
    auto features_path = bfs::current_path() / "Features";
    scan_features(core, features_path);
}

bool install_feature(TR::Core::Frontend& frontend, TR::Core::Key sessionKey)
{
    for (auto& feature_info : frontend.get_features_info(sessionKey))
    {
        if (feature_info.m_installed)
        {           
            continue;
        }

        bool ready_to_install = true;
        for (auto& dependency : feature_info.m_dependencies)
        {
            if (!frontend.get_feature_info(sessionKey, dependency).m_installed)
            {
                ready_to_install = false;
                break;
            }
        }

        if (!ready_to_install)
        {
            continue;
        }

        frontend.install_feature(sessionKey, feature_info.m_uuid);
        return true;
    }

    return false;
}

TR::Core::Core getDebugCore()
{
    using namespace TR;
    if (!TR::Core::verify(L"root", L"root", L"localhost"))
    {
        TR::Core::install(L"root", L"root", L"localhost");
    }

    Core::Core core; 
    core.connect_mysql(L"root", L"root", L"localhost");
    core.setup_mailing(Core::SMTP(L"localhost", L"admin@sed.local", L"admin"), L"admin2@sed.local");
    connect_direct_client_dispatcher(core);
    core.add_feature(std::make_unique<Core::SuitsFeature>());
    core.add_feature(std::make_unique<Core::FakeCiscoFeature>());
    scan_features(core);
    core.run();
    
    auto& frontend = core.get_frontend();
    auto sessionKey = frontend.open_session(L"admin");
    while (install_feature(frontend, sessionKey));
   
    if (frontend.get_users(sessionKey).size() == 1)
    {
        frontend.add_user(sessionKey, {L"user", L"user@sed.local"});
    }

    return core;
}

TR::Core::Core* getServerCore()
{
    return serverCore.get();
}

void setServerCore(std::unique_ptr<TR::Core::Core> core)
{
    serverCore = std::move(core);
}

int _tmain(int argc, _TCHAR* argv[])
{
    auto program_location = boost::dll::program_location();
    boost::filesystem::current_path(program_location.parent_path());

    serverCore = std::make_unique<TR::Core::Core>(getDebugCore());  
    Wt::WString::setDefaultEncoding(Wt::CharEncoding::UTF8);

    static const auto docroot_path = (boost::dll::program_location().parent_path() / "docroot").string();
    char* env[] { "app", "--docroot", (char*)docroot_path.c_str(), "--http-port", "8081", "--http-addr", "0.0.0.0" };
    return Wt::WRun(_countof(env), env, &createApplication);
}