#include "stdafx.h"
#include "Archive.h"
#include "Basis.h"
#include "Registry.h"
#include "Transaction.h"
#include "Executive.h"
#include "DateTime.h"
#include "BasisPacking.h"
#include "RegistryPacking.h"
#include "ArchivedReportData-odb.hxx"
#include "ArchivingDataset.h"
namespace TR {namespace Core {

ArchivedReport::ArchivedReport(boost::optional<ArchivedReportRef> ref, SourceRef source_ref, ReportTypeRef report_type_ref, stl_tools::lazy_shared_ptr<Content> content, std::shared_ptr<Diff> diff, time_t time, time_t off_time):
    Report(source_ref, report_type_ref, content, time),
    m_ref(ref),
    m_diff(diff),
    m_off_time(off_time)
{
}

std::shared_ptr<Diff> ArchivedReport::get_diff() const
{
    return m_diff;
}

ArchivedReportRef ArchivedReport::get_ref() const
{
    _ASSERT(m_ref);
    if (!m_ref)
    {
        throw Exception(L"ArchivedReport is not persist.");
    }
    return *m_ref;
}

ArchivedReport::operator ArchivedReportRef() const
{
    return get_ref();
}

Archive::Archive(Database& db, Basis& basis, Registry& registry):
    m_db(db),
    m_basis(basis),
    m_registry(registry),
    m_report_types(basis.m_report_types),
    m_source_types(basis.m_source_types),
    m_sources(registry.m_sources),
    m_collector(registry.m_collector),
    m_source_resources(registry.m_source_resources),
    m_archiving_dataset(std::make_unique<ArchivingDataset>(m_db)),
    m_executive(nullptr)
{
    m_report_types.connect_remove_report_type([this](UUID uuid, Transaction& t){
        on_remove_report_type(uuid, t);
    }, ARCHIVE_PRIORITY);

    m_sources.connect_remove_source([this](SourceKey source_key, const std::vector<SourceKey>& removed_sources, Transaction& t){
        on_remove_sources(removed_sources, t);
    }, ARCHIVE_PRIORITY);

    m_collector.connect_update_current_report([this](CurrentReportRef current_report_ref, bool content_changed, Transaction& t){
        if (content_changed)
        {
            on_update_current_report(current_report_ref, t);
        }
    }, ARCHIVE_PRIORITY);

    m_source_resources.add_service_request([this](SourceRef source_ref, ResourceRef resource_ref)
    {
        if (auto report_type_ref = boost::get<ReportTypeRef>(&resource_ref))
        {
            return is_archiving_enabled(source_ref, *report_type_ref);
        }
        return false;       
    });
}

Archive::~Archive()
{
}

void Archive::run(Executive* executive, Transaction& t)
{
    _ASSERT(!m_executive);
    transact_assign(m_executive, executive, t);

    t.connect_commit([this]()
    {
        archivate_one_by_one();
        remove_expired_one_by_one();
    });
}

// archiving policy
void Archive::set_archiving_policy(SubjectRef subject_ref, ReportTypeRef report_type_ref, ArchivingPolicy archiving_policy, Transaction& t)
{
    auto archiving_data = m_archiving_dataset->get(pack(subject_ref), pack(report_type_ref));
    if (archiving_data.m_archiving_policy == archiving_policy)
    {
        return;
    }

    archiving_data.m_archiving_policy = archiving_policy;
    m_archiving_dataset->put(archiving_data, t);
    
    if (auto source_ref = boost::get<SourceRef>(&subject_ref))
    {
        m_source_resources.check_service_request(*source_ref, report_type_ref, t);
        return;
    }

    _ASSERT(boost::get<RoleRef>(&subject_ref));
    for (auto derived_ref : get_derived_refs(m_registry, subject_ref))
    {
        forward_archiving_policy(subject_ref, report_type_ref, t);
    }
}

ArchivingPolicy Archive::get_archiving_policy(SubjectRef subject_ref, ReportTypeRef report_type_ref) const
{
    ReadOnlyTransaction t(m_db);
    auto archiving_data = m_archiving_dataset->get(pack(subject_ref), pack(report_type_ref));
    return archiving_data.m_archiving_policy;
}

bool Archive::is_archiving_enabled(SubjectRef subject_ref, ReportTypeRef report_type_ref) const
{
    auto archiving_policy = get_archiving_policy(subject_ref, report_type_ref);

    if (archiving_policy == ArchivingPolicy::ENABLE_ARCHIVING)
    {
        return true;
    }

    if (archiving_policy == ArchivingPolicy::DISABLE_ARCHIVING)
    {
        return false;
    }

    if (refer_default_role(subject_ref))
    {
        return false;
    }

    return is_archiving_enabled(*get_base_ref(subject_ref), report_type_ref);
}

// expiration policy
void Archive::set_archived_expiration(SubjectRef subject_ref, ReportTypeRef report_type_ref, ArchivedExpiration archived_expiration, Transaction& t)
{
    auto archiving_data = m_archiving_dataset->get(pack(subject_ref), pack(report_type_ref));
    if (archiving_data.m_archived_expiration == archived_expiration)
    {
        return;
    }

    archiving_data.m_archived_expiration = archived_expiration;
    m_archiving_dataset->put(archiving_data, t);
}

ArchivedExpiration Archive::get_archived_expiration(SubjectRef subject_ref, ReportTypeRef report_type_ref) const
{
    return m_archiving_dataset->get(pack(subject_ref), pack(report_type_ref)).m_archived_expiration;
}

ArchivedExpiration Archive::get_effective_archived_expiration(SubjectRef subject_ref, ReportTypeRef report_type_ref) const
{
    auto archived_expiration = get_archived_expiration(subject_ref, report_type_ref);
    if (archived_expiration != ArchivedExpiration::EXPIRED_DEFAULT)
    {
        return archived_expiration;
    }

    if (refer_default_role(subject_ref))
    {
        return ArchivedExpiration::EXPIRED_NEVER;
    }

    return get_effective_archived_expiration(*get_base_ref(subject_ref), report_type_ref);
}

ArchivedReport Archive::get_archived_report(ArchivedReportKey archived_report_key) const
{
    ReadOnlyTransaction t(m_db);
    ArchivedReportData archived_report_data;
    m_db.load<ArchivedReportData>(archived_report_key, archived_report_data);
    return unpack_archived_report(archived_report_data);
}

std::vector<ArchivedReport> Archive::get_archived_reports(SourceRef source_ref) const
{
    ReadOnlyTransaction t(m_db);
    std::vector<ArchivedReport> archived_reports;
    for (auto& archived_report_data : m_db.query<ArchivedReportData>(odb::query<ArchivedReportData>::source_key.value == source_ref.get_key()))
    {
        archived_reports.push_back(unpack_archived_report(archived_report_data));
    }
    return archived_reports;
}

boost::optional<ArchivedReport> Archive::last_archived_report(SourceRef source_ref, ReportTypeRef report_type_ref) const
{
    ReadOnlyTransaction t(m_db);
    auto last_archive_report_data = m_db.query<ArchivedReportData>(
        odb::query<ArchivedReportData>::source_key.value == source_ref.get_key() &&
        odb::query<ArchivedReportData>::report_type_uuid.value == report_type_ref.get_uuid() &&
        odb::query<ArchivedReportData>::off_time == 0);

    if (last_archive_report_data.empty())
    {
        return boost::none;
    }
        
    return unpack_archived_report(*last_archive_report_data.begin());
}

boost::optional<ArchivedReport> Archive::find_archived_report(SourceRef source_ref, ReportTypeRef report_type_ref, time_t time) const
{
    ReadOnlyTransaction t(m_db);
    auto found_archive_report_data = m_db.query<ArchivedReportData>(
        odb::query<ArchivedReportData>::source_key.value == source_ref.get_key() &&
        odb::query<ArchivedReportData>::report_type_uuid.value == report_type_ref.get_uuid() &&
        odb::query<ArchivedReportData>::time <= time &&
        (odb::query<ArchivedReportData>::off_time > time || odb::query<ArchivedReportData>::off_time == 0));

    if (found_archive_report_data.empty())
    {
        return boost::none;
    }

    return unpack_archived_report(*found_archive_report_data.begin());
}

void Archive::forward_archiving_policy(SubjectRef subject_ref, ReportTypeRef report_type_ref, Transaction& t)
{
    auto archiving_policy = get_archiving_policy(subject_ref, report_type_ref);
    if (archiving_policy != ArchivingPolicy::DEFAULT_ARCHIVING)
    {
        return;
    }

    if (auto source_ref = boost::get<SourceRef>(&subject_ref))
    {
        m_source_resources.check_service_request(*source_ref, report_type_ref, t);
        return;
    }

    _ASSERT(boost::get<RoleRef>(&subject_ref));
    for (auto derived_ref : get_derived_refs(m_registry, subject_ref))
    {
        forward_archiving_policy(subject_ref, report_type_ref, t);
    }
}

ArchivedReportData Archive::pack_archived_report(const ArchivedReport& archived_report) const
{
    return ArchivedReportData(
        archived_report.m_ref ? archived_report.m_ref->get_key(): 0,
        pack(archived_report.m_source_ref),
        pack(archived_report.m_report_type_ref),
        archived_report.m_diff ? boost::make_optional(archived_report.m_diff->as_blob()): boost::none,
        archived_report.m_time,
        archived_report.m_off_time);
}

ArchivedReport Archive::unpack_archived_report(const ArchivedReportData& archived_report_data) const
{
    auto report_type_uuid = archived_report_data.m_report_type_uuid;
    auto archived_report_key = archived_report_data.m_key;
    auto& report_type = m_report_types.get_report_type(archived_report_data.m_report_type_uuid);

    return ArchivedReport(
        ArchivedReportRef(this, ArchivedReportKey(archived_report_data.m_key)),
        unpack(archived_report_data.m_source_key, &m_sources),
        unpack(archived_report_data.m_report_type_uuid, &m_report_types),
        [this, report_type_uuid, archived_report_key]{
            ReadOnlyTransaction t(m_db);
            ArchivedReportContentData archived_report_content_data;
            m_db.load<ArchivedReportContentData>(archived_report_key, archived_report_content_data);
            auto& report_type = m_report_types.get_report_type(report_type_uuid);
            return report_type.make_content(archived_report_content_data.m_content);
        },
        archived_report_data.m_diff ? report_type.make_diff(*archived_report_data.m_diff): nullptr,
        archived_report_data.m_time,
        archived_report_data.m_off_time);
}

void Archive::archivate_current_report(const CurrentReport& current_report, Transaction& t)
{
    ArchivedReport archived_report(
        boost::none,
        current_report.get_source_ref(),
        current_report.get_report_type_ref(),
        current_report.get_content(),
        current_report.get_diff(),
        current_report.get_time(),
        0
    );

    if (auto prev_archived_report = last_archived_report(archived_report.get_source_ref(), archived_report.get_report_type_ref()))
    {
        _ASSERT(archived_report.get_time() != prev_archived_report->get_time());
        prev_archived_report->m_off_time = archived_report.get_time();
        m_db.update(pack_archived_report(*prev_archived_report));
    }

    archived_report.m_ref = ArchivedReportRef(this, ArchivedReportKey(odb::persist(m_db, pack_archived_report(archived_report))));
    m_db.persist(ArchivedReportContentData(archived_report.m_ref->get_key(), archived_report.get_content()->as_blob()));

    m_add_archived_report_sig(archived_report, t);
}

bool Archive::check_archivate_condition(const CurrentReport& current_report) const
{
    if (!is_archiving_enabled(current_report.get_source_ref(), current_report.get_report_type_ref()))
    {
        return false;
    }

    auto archived_report = last_archived_report(current_report.get_source_ref(), current_report.get_report_type_ref());
    if (!archived_report)
    {
        return true;
    }

    return archived_report->get_time() != current_report.get_time();
}

void Archive::archivate_one_by_one()
{
    no_except([&]{
        std::vector<std::pair<SourceKey, ReportTypeUUID>> targets;
        for (auto& source : m_sources.get_sources())
        {
            for (auto& source_resource : m_source_resources.get_source_resources(source))
            {
                if (auto report_type_ref = boost::get<ReportTypeRef>(&source_resource.m_resource_ref))
                {
                    targets.push_back({source.get_ref().get_key(), report_type_ref->get_uuid()});
                }
            }
        }

        m_executive->async(std::bind(&Archive::archivate_next, this, stl_tools::force_move(targets)));
    });
}

void Archive::archivate_next(std::vector<std::pair<SourceKey, ReportTypeUUID>> targets)
{
    no_except([&]{
        if (targets.empty())
        {
            m_executive->async(std::bind(&Archive::archivate_one_by_one, this), time(nullptr) + 10);
            return;
        }

        auto source_key = targets.back().first;
        auto report_type_uuid = targets.back().second;
        targets.pop_back();

        archivate_one(source_key, report_type_uuid);
        m_executive->async(std::bind(&Archive::archivate_next, this, stl_tools::force_move(targets)));
    });
}

void Archive::archivate_one(SourceKey source_key, ReportTypeUUID report_type_uuid)
{
    try
    {
        Transaction t(m_db);
        if (!m_sources.has_source(source_key) || !m_report_types.has_report_type(report_type_uuid))
        {
            return;
        }

        auto current_report = m_collector.find_current_report(m_sources.get_source(source_key), m_report_types.get_report_type(report_type_uuid));
        if (!current_report)
        {
            return;
        }

        if (check_archivate_condition(*current_report))
        {
            archivate_current_report(*current_report, t);
        }
        t.commit();
    }
    catch (std::exception&)
    {
    }   
}

void Archive::remove_expired_one_by_one()
{
    no_except([&]{
        std::vector<std::pair<SourceKey, ReportTypeUUID>> targets;
        for (auto& source : m_sources.get_sources())
        {
            for (auto& report_type_ref : m_report_types.get_report_types())
            {
                targets.push_back({source.get_ref().get_key(), report_type_ref.get_uuid()});
            }
        }

        m_executive->async(std::bind(&Archive::remove_next_expired, this, stl_tools::force_move(targets)));
    });
}

void Archive::remove_next_expired(std::vector<std::pair<SourceKey, ReportTypeUUID>> targets)
{
    no_except([&]{
        if (targets.empty())
        {
            m_executive->async(std::bind(&Archive::remove_expired_one_by_one, this), time(nullptr) + 10);
            return;
        }

        auto source_key = targets.back().first;
        auto report_type_uuid = targets.back().second;
        targets.pop_back();

        remove_expired(source_key, report_type_uuid);
        m_executive->async(std::bind(&Archive::remove_next_expired, this, stl_tools::force_move(targets)));
    });
}

void Archive::remove_expired(SourceKey source_key, ReportTypeUUID report_type_uuid)
{
    try
    {
        Transaction t(m_db);
        auto source = m_sources.get_source(source_key);
        auto& report_type = m_report_types.get_report_type(report_type_uuid);

        auto remove_time = time(nullptr);
        switch (get_effective_archived_expiration(source, report_type))
        {
        case ArchivedExpiration::EXPIRED_NEVER:
            remove_time = 0;
            break;

        case ArchivedExpiration::EXPIRED_IN_HOUR:
            remove_time -= ONE_HOUR;
            break;

        case ArchivedExpiration::EXPIRED_IN_DAY:
            remove_time -= ONE_DAY;
            break;

        case ArchivedExpiration::EXPIRED_IN_MONTH:
            remove_time -= ONE_MONTH;
            break;

        case ArchivedExpiration::EXPIRED_IN_YEAR:
            remove_time -= ONE_YEAR;
            break;

        default:
            _ASSERT(false);
        }

        std::vector<Key> remove_keys;
        for (auto& archived_report_data : m_db.query<ArchivedReportData>(
            odb::query<ArchivedReportData>::report_type_uuid.value == report_type_uuid && 
            odb::query<ArchivedReportData>::source_key.value == source_key &&
            odb::query<ArchivedReportData>::time < remove_time
        ))
        {
            remove_keys.push_back(archived_report_data.m_key);
        }

        if (!remove_keys.empty())
        {
            m_db.erase_query<ArchivedReportData>(odb::query<ArchivedReportData>::key.in_range(remove_keys.begin(), remove_keys.end()));
            m_db.erase_query<ArchivedReportContentData>(odb::query<ArchivedReportContentData>::key.in_range(remove_keys.begin(), remove_keys.end()));
        }
        
        t.commit();
    }
    catch (std::exception&)
    {
        // write to log
    }
}

void Archive::on_update_current_report(CurrentReportRef current_report_ref, Transaction& t)
{
    auto current_report = *current_report_ref;
    if (check_archivate_condition(current_report))
    {
        archivate_current_report(current_report, t);
    }
}

void Archive::on_remove_report_type(UUID report_type_uuid, Transaction& t)
{
    m_archiving_dataset->erase_report_type_equal(ReportTypeUUID(report_type_uuid), t);

    std::vector<Key> remove_keys;
    for (auto& archived_report_data : m_db.query<ArchivedReportData>(odb::query<ArchivedReportData>::report_type_uuid.value == report_type_uuid))
    {
        remove_keys.push_back(archived_report_data.m_key);
    }

    if (!remove_keys.empty())
    {
        m_db.erase_query<ArchivedReportData>(odb::query<ArchivedReportData>::key.in_range(remove_keys.begin(), remove_keys.end()));
        m_db.erase_query<ArchivedReportContentData>(odb::query<ArchivedReportContentData>::key.in_range(remove_keys.begin(), remove_keys.end()));
    }
}

void Archive::on_remove_sources(const std::vector<SourceKey>& removed_sources, Transaction& t)
{
    m_archiving_dataset->erase_source_equal(removed_sources, t);

    std::vector<Key> remove_keys;
    for (auto& archived_report_data : m_db.query<ArchivedReportData>(odb::query<ArchivedReportData>::source_key.value.in_range(removed_sources.begin(), removed_sources.end())))
    {
        remove_keys.push_back(archived_report_data.m_key);
    }

    if (!remove_keys.empty())
    {
        m_db.erase_query<ArchivedReportData>(odb::query<ArchivedReportData>::key.in_range(remove_keys.begin(), remove_keys.end()));
        m_db.erase_query<ArchivedReportContentData>(odb::query<ArchivedReportContentData>::key.in_range(remove_keys.begin(), remove_keys.end()));
    }   
}

}} //namespace TR { namespace Core {
