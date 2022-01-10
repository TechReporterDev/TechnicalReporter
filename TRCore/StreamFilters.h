#pragma once
#include "XmlNodeFilter.h"
#include "ServicesDecl.h"
namespace TR { namespace Core {
struct StreamFilterData;

class StreamFilter
{
public:
    friend class StreamFilters;

    Key                                 get_key() const;
    std::wstring                        get_name() const;
    StreamTypeRef                       get_input_ref() const;
    StreamTypeRef                       get_output_ref() const;

    const XmlQueryDoc&                  get_filter_doc() const;
    void                                set_filter_doc(std::unique_ptr<XmlQueryDoc> filter_doc);
    std::shared_ptr<const XmlDefDoc>    get_filter_def() const;
    
private:
    StreamFilter(Key key, std::wstring name, StreamTypeRef input_ref,
        std::unique_ptr<XmlQueryDoc> filter_doc, std::shared_ptr<const XmlDefDoc> filter_def,
        boost::optional<StreamTypeRef> output_ref = boost::none);

    Key m_key;
    std::wstring m_name;
    StreamTypeRef m_input_ref;
    std::shared_ptr<const XmlQueryDoc> m_filter_doc;
    std::shared_ptr<const XmlDefDoc> m_filter_def;
    boost::optional<StreamTypeRef> m_output_ref;
};

class StreamFilters
{
public:
    static const Priority STREAM_FILTERS_PRIORITY = 1900;

    StreamFilters(Database& db, Basis& basis, Registry& registry);
    void                                run(Executive* executive, Transaction& t);

    StreamFilter                        create_stream_filter(std::wstring name, StreamTypeRef input_ref) const;
    void                                add_stream_filter(StreamFilter& stream_filter, Transaction& t);
    void                                update_stream_filter(const StreamFilter& stream_filter, Transaction& t);

    StreamFilter                        get_stream_filter(Key stream_filter_key) const;
    void                                remove_stream_filter(Key stream_filter_key, Transaction& t);
    
    std::vector<StreamFilter>           find_by_input(StreamTypeRef input_ref) const;
    boost::optional<StreamFilter>       find_by_output(StreamTypeRef output_ref) const;

private:
    StreamFilterData                    pack_stream_filter(const StreamFilter& stream_filter) const;
    StreamFilter                        unpack_stream_filter(const StreamFilterData& stream_filter_data) const;

    void                                install_stream_filter(const StreamFilter& stream_filter, Transaction& t);
    void                                uninstall_stream_filter(const StreamFilter& stream_filter, Transaction& t);

    Database& m_db;
    Roles& m_roles;
    Sources& m_sources; 
    Reservoir& m_reservoir;
    StreamTypes& m_stream_types;
    Convertions& m_convertions;
    Conveyor& m_conveyor;
    bool m_running;
};

}} // namespace TR { namespace Core {