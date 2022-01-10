#pragma once
#include "XmlNodeFilter.h"
#include "ServicesDecl.h"
namespace TR { namespace Core {
struct TrackingStreamData;

class TrackingStream
{
public:
    friend class TrackingStreams;

    TrackingStream(std::wstring name, ReportTypeRef input_ref, std::string input_xpath);

    Key                                 get_key() const;
    std::wstring                        get_name() const;   
    ReportTypeRef                       get_input_ref() const;
    std::string                         get_input_xpath() const;
    StreamTypeRef                       get_output_ref() const;

private:
    TrackingStream(Key key, std::wstring name, ReportTypeRef input_ref, std::string input_xpath, boost::optional<StreamTypeRef> output_ref);

    Key m_key;
    std::wstring m_name;    
    ReportTypeRef m_input_ref;
    std::string m_input_xpath;
    boost::optional<StreamTypeRef> m_output_ref;
};

class TrackingStreams
{
public:
    static const Priority TRACKING_STREAMS_PRIORITY = 1850;

    TrackingStreams(Database& db, Basis& basis);
    void                                run(Executive* executive, Transaction& t);

    void                                add_tracking_stream(TrackingStream& tracking_stream, Transaction& t);
    TrackingStream                      get_tracking_stream(Key tracking_stream_key) const;
    void                                remove_tracking_stream(Key tracking_stream_key, Transaction& t);
    
    std::vector<TrackingStream>         find_by_input(ReportTypeRef input_ref) const;
    boost::optional<TrackingStream>     find_by_output(StreamTypeRef output_ref) const;

private:
    TrackingStreamData                  pack_tracking_stream(const TrackingStream& tracking_stream) const;
    TrackingStream                      unpack_tracking_stream(const TrackingStreamData& custom_filter_data) const;

    void                                install_tracking_stream(const TrackingStream& tracking_stream, Transaction& t);
    void                                uninstall_tracking_stream(const TrackingStream& tracking_stream, Transaction& t);

    Database& m_db;
    ReportTypes& m_report_types;
    StreamTypes& m_stream_types;
    Trackings& m_trackings;
    bool m_running;
};

}} // namespace TR { namespace Core {