#include "stdafx.h"
#include "TrackingStreams.h"
#include "Basis.h"
#include "Registry.h"
#include "Transaction.h"
#include "ServicesPacking.h"
#include "TrackingStreamData-odb.hxx"
namespace TR { namespace Core {

class GeneralTracking: public Tracking
{
public:
    GeneralTracking(const TrackingStream& tracking_stream):
        m_direction({tracking_stream.get_input_ref(), tracking_stream.get_output_ref()}),
        m_input_xpath(tracking_stream.get_input_xpath())
    {
    }

    virtual TrackingDirection get_direction() const override
    {
        return m_direction;
    }

    virtual Functor prepare_functor(SourceRef /*source_ref*/) const override
    {
        auto& output = dynamic_cast<const StreamType&>(*m_direction.m_output_ref);
        auto message_def = output.get_message_def();

        return[message_def, input_xpath = m_input_xpath](std::shared_ptr<Content> content){
            
            std::vector<StreamMessage> messages;

            if (auto regular_content = std::dynamic_pointer_cast<RegularContent>(content))
            {
                if (auto target_node = xml_tools::find_xpath_node(regular_content->as_xml_doc(), input_xpath.c_str()))
                {
                    auto body = xml_tools::create_doc();
                    auto& root = xml_tools::set_root(*body, xml_tools::create_element("tracking"));                 

                    auto& value_node = xml_tools::add_element(root, "value");
                    xml_tools::add_text_node(value_node, xml_tools::get_node_text(*target_node).c_str());

                    XML::bind_definition(*body, *message_def);

                    messages.push_back({
                        StreamMessage::INFO,
                        L"",
                        time(nullptr),
                        std::make_unique<GeneralRegularContent>(stl_tools::static_pointer_cast<XmlRegularDoc>(std::move(body)), message_def)
                    });
                }
            }

            return messages;
        };
    }   

private:
    TrackingDirection m_direction;
    std::string m_input_xpath;
};

TrackingStream::TrackingStream(std::wstring name, ReportTypeRef input_ref, std::string input_xpath):
    TrackingStream(0, std::move(name), input_ref, std::move(input_xpath), boost::none)
{
}

TrackingStream::TrackingStream(Key key, std::wstring name, ReportTypeRef input_ref, std::string input_xpath, boost::optional<StreamTypeRef> output_ref):
    m_key(key),
    m_name(std::move(name)),
    m_input_ref(input_ref),
    m_input_xpath(std::move(input_xpath)),
    m_output_ref(output_ref)
{
}

Key TrackingStream::get_key() const
{
    if (!m_key)
    {
        throw Exception(L"TrackingStream is not persist.");
    }
    return m_key;
}

std::wstring TrackingStream::get_name() const
{
    return m_name;
}

ReportTypeRef TrackingStream::get_input_ref() const
{
    return m_input_ref;
}

std::string TrackingStream::get_input_xpath() const
{
    return m_input_xpath;
}

StreamTypeRef TrackingStream::get_output_ref() const
{
    _ASSERT(m_output_ref);
    if (!m_output_ref)
    {
        throw Exception(L"TrackingStream is not persist.");
    }
    return *m_output_ref;
}

TrackingStreams::TrackingStreams(Database& db, Basis& basis):
    m_db(db),
    m_report_types(basis.m_report_types),
    m_stream_types(basis.m_stream_types),
    m_trackings(basis.m_transformations.m_trackings),
    m_running(false)
{
    m_report_types.connect_add_report_type([&](ReportTypeRef ref, Transaction& t){
        if (m_running)
        {
            return;
        }

        for (auto& tracking_stream: find_by_input(ref))
        {
            install_tracking_stream(tracking_stream, t);
        }

    }, TRACKING_STREAMS_PRIORITY);
}

void TrackingStreams::run(Executive*, Transaction& t)
{
    transact_assign(m_running, true, t);
}

void TrackingStreams::add_tracking_stream(TrackingStream& tracking_stream, Transaction& t)
{
    _ASSERT(!tracking_stream.m_key);
    tracking_stream.m_output_ref = unpack(StreamTypeUUID(stl_tools::gen_uuid()), &m_stream_types);
    tracking_stream.m_key = odb::persist(m_db, pack_tracking_stream(tracking_stream));
    install_tracking_stream(tracking_stream, t);
}

TrackingStream TrackingStreams::get_tracking_stream(Key tracking_stream_key) const
{
    ReadOnlyTransaction t(m_db);
    TrackingStreamData tracking_stream_data;
    m_db.load<TrackingStreamData>(tracking_stream_key, tracking_stream_data);
    return unpack_tracking_stream(tracking_stream_data);
}

void TrackingStreams::remove_tracking_stream(Key tracking_stream_key, Transaction& t)
{
    TrackingStreamData tracking_stream_data;
    m_db.load<TrackingStreamData>(tracking_stream_key, tracking_stream_data);

    auto tracking_stream = unpack_tracking_stream(tracking_stream_data);
    uninstall_tracking_stream(tracking_stream, t);
    m_db.erase(tracking_stream_data);
}

std::vector<TrackingStream> TrackingStreams::find_by_input(ReportTypeRef input_ref) const
{
    ReadOnlyTransaction t(m_db);
    std::vector<TrackingStream> tracking_streams;
    for (auto& tracking_stream_data : m_db.query<TrackingStreamData>(odb::query<TrackingStreamData>::input_uuid.value == input_ref.get_uuid()))
    {
        tracking_streams.push_back(unpack_tracking_stream(tracking_stream_data));
    }
    return tracking_streams;
}

boost::optional<TrackingStream> TrackingStreams::find_by_output(StreamTypeRef output_ref) const
{
    ReadOnlyTransaction t(m_db);
    auto query_result = m_db.query<TrackingStreamData>(odb::query<TrackingStreamData>::output_uuid.value == output_ref.get_uuid());
    if (query_result.empty())
    {
        return boost::none;
    }
    return unpack_tracking_stream(*query_result.begin());
}

TrackingStreamData TrackingStreams::pack_tracking_stream(const TrackingStream& tracking_stream) const
{
    return TrackingStreamData(
        tracking_stream.m_key,
        pack(tracking_stream.m_name),
        pack(tracking_stream.m_input_ref),
        tracking_stream.m_input_xpath,
        pack(*tracking_stream.m_output_ref)
    );
}

TrackingStream TrackingStreams::unpack_tracking_stream(const TrackingStreamData& tracking_stream_data) const
{
    return TrackingStream(
        tracking_stream_data.m_key,
        unpack(tracking_stream_data.m_name),
        unpack(tracking_stream_data.m_input_uuid, &m_report_types),
        tracking_stream_data.m_input_xpath,
        unpack(tracking_stream_data.m_output_uuid, &m_stream_types)
    );
}

static std::string general_tracking_stream_def(
    "<?xml version=\"1.0\"?>"
    "<tracking class='STRUCT' caption='Tracking'>"
        "<value class='STRING' caption='Value'/>"
    "</tracking>");

void TrackingStreams::install_tracking_stream(const TrackingStream& tracking_stream, Transaction& t)
{
    _ASSERT(tracking_stream.m_key);

    m_stream_types.add_stream_type(std::make_unique<StreamType>(
        tracking_stream.m_output_ref->get_uuid(),
        tracking_stream.get_name(),
        XML::parse_definition(general_tracking_stream_def.c_str())), t);

    m_trackings.add_tracking(std::make_shared<GeneralTracking>(tracking_stream), t);
}

void TrackingStreams::uninstall_tracking_stream(const TrackingStream& tracking_stream, Transaction& t)
{
    _ASSERT(tracking_stream.m_key);

    m_trackings.remove_tracking({tracking_stream.m_input_ref, *tracking_stream.m_output_ref}, t);
    m_stream_types.remove_stream_type(tracking_stream.m_output_ref->get_uuid(), t);
}

}} //namespace TR { namespace Core {