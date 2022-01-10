#include "stdafx.h"
#include "StreamFilters.h"
#include "Basis.h"
#include "Registry.h"
#include "Transaction.h"
#include "ServicesPacking.h"
#include "StreamFilterData-odb.hxx"

namespace TR { namespace Core {

class StreamFilterConvertion: public Convertion
{
public:
    StreamFilterConvertion(const StreamFilters& stream_filters, const StreamFilter& stream_filter):
        m_stream_filters(stream_filters),
        m_direction({stream_filter.get_input_ref(), stream_filter.get_output_ref()}),
        m_stream_filter_key(stream_filter.get_key())
    {
    }

    virtual ConvertionDirection get_direction() const override
    {
        return m_direction;
    }

    virtual Functor prepare_functor(SourceRef /*source_ref*/) const override
    {
        auto stream_filter = m_stream_filters.get_stream_filter(m_stream_filter_key);
        auto filter_doc = std::shared_ptr<XmlQueryDoc>(xml_tools::clone_doc(stream_filter.get_filter_doc()));
        auto filter_def = stream_filter.get_filter_def();

        return [filter_doc, filter_def](const std::vector<StreamMessage>& istream) {
            std::vector<StreamMessage> ostream;
            for (auto& input_message : istream)
            {
                if (do_query(*filter_doc, input_message.m_body->as_regular_doc()))
                {
                    ostream.push_back(input_message);                       
                }               
            }
            return ostream;
        };
    }   

private:
    ConvertionDirection m_direction;
    const StreamFilters& m_stream_filters;  
    Key m_stream_filter_key;
};

StreamFilter::StreamFilter(Key key, std::wstring name, StreamTypeRef input_ref,
    std::unique_ptr<XmlQueryDoc> filter_doc, std::shared_ptr<const XmlDefDoc> filter_def,
    boost::optional<StreamTypeRef> output_ref):
    m_key(key),
    m_name(std::move(name)),
    m_input_ref(input_ref),
    m_filter_doc(std::move(filter_doc)),
    m_filter_def(std::move(filter_def)),
    m_output_ref(output_ref)
{
}

Key StreamFilter::get_key() const
{
    return m_key;
}

std::wstring StreamFilter::get_name() const
{
    return m_name;
}

StreamTypeRef StreamFilter::get_input_ref() const
{
    return m_input_ref;
}

StreamTypeRef StreamFilter::get_output_ref() const
{
    _ASSERT(m_output_ref);
    if (!m_output_ref)
    {
        throw Exception(L"Stream filter is not persist.");
    }
    return *m_output_ref;
}

const XmlQueryDoc& StreamFilter::get_filter_doc() const
{
    _ASSERT(m_filter_doc);
    return *m_filter_doc;
}

void StreamFilter::set_filter_doc(std::unique_ptr<XmlQueryDoc> filter_doc)
{
    m_filter_doc = std::move(filter_doc);
}

std::shared_ptr<const XmlDefDoc> StreamFilter::get_filter_def() const
{
    _ASSERT(m_filter_def);
    return m_filter_def;
}

StreamFilters::StreamFilters(Database& db, Basis& basis, Registry& registry):
    m_db(db),
    m_roles(registry.m_roles),
    m_sources(registry.m_sources),
    m_reservoir(registry.m_reservoir),
    m_stream_types(basis.m_stream_types),
    m_convertions(basis.m_transformations.m_convertions),
    m_conveyor(registry.m_conveyor),
    m_running(false)
{
    m_stream_types.connect_add_stream_type([&](StreamTypeRef ref, Transaction& t){
        if (m_running)
        {
            return;
        }

        for (auto& stream_filter : find_by_input(ref))
        {
            install_stream_filter(stream_filter, t);
        }

    }, STREAM_FILTERS_PRIORITY);
}

void StreamFilters::run(Executive*, Transaction& t)
{
    transact_assign(m_running, true, t);
}

StreamFilter StreamFilters::create_stream_filter(std::wstring name, StreamTypeRef input_ref) const
{
    auto filter_def = (*input_ref).get_message_def();
    auto filter_doc = XML::create_query(*filter_def);
    
    return StreamFilter(0, std::move(name), input_ref, std::move(filter_doc), std::move(filter_def));
}

void StreamFilters::add_stream_filter(StreamFilter& stream_filter, Transaction& t)
{
    _ASSERT(!stream_filter.get_key());

    stream_filter.m_output_ref = unpack(StreamTypeUUID(stl_tools::gen_uuid()), &m_stream_types);
    stream_filter.m_key = odb::persist(m_db, pack_stream_filter(stream_filter));
    install_stream_filter(stream_filter, t);    
    //m_reservoir.set_stream_length(m_roles.get_default_role(), *stream_filter.m_output_ref, StreamLength::NONE, t);
}

void StreamFilters::update_stream_filter(const StreamFilter& stream_filter, Transaction& t)
{
    _ASSERT(stream_filter.get_key());
    m_db.update(pack_stream_filter(stream_filter));
}

StreamFilter StreamFilters::get_stream_filter(Key stream_filter_key) const
{
    ReadOnlyTransaction t(m_db);
    StreamFilterData stream_filter_data;
    m_db.load<StreamFilterData>(stream_filter_key, stream_filter_data);
    return unpack_stream_filter(stream_filter_data);
}

void StreamFilters::remove_stream_filter(Key stream_filter_key, Transaction& t)
{
    StreamFilterData stream_filter_data;
    m_db.load<StreamFilterData>(stream_filter_key, stream_filter_data);

    auto stream_filter = unpack_stream_filter(stream_filter_data);
    uninstall_stream_filter(stream_filter, t);
    m_db.erase(stream_filter_data);
}


std::vector<StreamFilter> StreamFilters::find_by_input(StreamTypeRef input_ref) const
{
    ReadOnlyTransaction t(m_db);
    std::vector<StreamFilter> stream_filters;
    for (auto& stream_filter_data : m_db.query<StreamFilterData>(odb::query<StreamFilterData>::input_uuid.value == input_ref.get_uuid()))
    {
        stream_filters.push_back(unpack_stream_filter(stream_filter_data));
    }
    return stream_filters;
}

boost::optional<StreamFilter> StreamFilters::find_by_output(StreamTypeRef output_ref) const
{
    ReadOnlyTransaction t(m_db);
    auto query_result = m_db.query<StreamFilterData>(odb::query<StreamFilterData>::output_uuid.value == output_ref.get_uuid());
    if (query_result.empty())
    {
        return boost::none;
    }

    //_ASSERT(std::distance(query_result.begin(), query_result.end()) == 1);
    return unpack_stream_filter(*query_result.begin());
}

StreamFilterData StreamFilters::pack_stream_filter(const StreamFilter& stream_filter) const
{
    return StreamFilterData(
        stream_filter.m_key,
        pack(stream_filter.m_name),
        pack(stream_filter.m_input_ref),
        pack(*stream_filter.m_output_ref),
        pack(*stream_filter.m_filter_doc),
        pack(*stream_filter.m_filter_def)
    );
}

StreamFilter StreamFilters::unpack_stream_filter(const StreamFilterData& stream_filter_data) const
{
    auto filter_def = unpack(stream_filter_data.m_filter_def);
    auto filter_doc = unpack(stream_filter_data.m_filter_doc, *filter_def);

    return StreamFilter(
        stream_filter_data.m_key,
        unpack(stream_filter_data.m_name),
        unpack(stream_filter_data.m_input_uuid, &m_stream_types),       
        std::move(filter_doc),
        std::move(filter_def),
        unpack(stream_filter_data.m_output_uuid, &m_stream_types)
    );
}

void StreamFilters::install_stream_filter(const StreamFilter& stream_filter, Transaction& t)
{
    _ASSERT(stream_filter.m_output_ref);

    m_stream_types.add_stream_type(std::make_unique<StreamType>(
        stream_filter.m_output_ref->get_uuid(),
        stream_filter.get_name(),
        xml_tools::clone_doc(*stream_filter.get_filter_def())), t);

    m_convertions.add_convertion(std::make_unique<StreamFilterConvertion>(*this, stream_filter), t);
}

void StreamFilters::uninstall_stream_filter(const StreamFilter& stream_filter, Transaction& t)
{
    m_convertions.remove_convertion({stream_filter.get_input_ref(), stream_filter.get_output_ref()}, t);
    m_stream_types.remove_stream_type(stream_filter.get_output_ref().get_uuid(), t);
}

}} //namespace TR { namespace Core {