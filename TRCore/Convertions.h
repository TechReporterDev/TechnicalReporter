#pragma once
#include "stddecl.h"
#include "BackgndFwd.h"
#include "BasisFwd.h"
#include "BasisRefs.h"
#include "StreamMessage.h"
namespace TR { namespace Core {
class SourceRef;

struct ConvertionDirection
{
    ConvertionDirection(StreamTypeRef input_ref, StreamTypeRef output_ref);

    StreamTypeRef m_input_ref;
    StreamTypeRef m_output_ref;
};

inline bool operator < (const ConvertionDirection& left, const ConvertionDirection& right);
inline bool operator == (const ConvertionDirection& left, const ConvertionDirection& right);

class Convertion
{
public:
    using Functor = std::function<std::vector<StreamMessage>(const std::vector<StreamMessage>&)>;

    virtual ~Convertion() = default;
    virtual ConvertionDirection     get_direction() const = 0;
    virtual Functor                 prepare_functor(SourceRef source_ref) const = 0;
};

class Convertions
{
public:
    static const Priority CONVERTIONS_PRIORITY = 600;

    Convertions();
    ~Convertions();
    void                                        set_stream_types(StreamTypes* stream_types);

    void                                        add_convertion(std::shared_ptr<Convertion> convertion, Transaction& t);
    void                                        remove_convertion(const ConvertionDirection& direction, Transaction& t);
    std::shared_ptr<Convertion>                 get_convertion(const ConvertionDirection& direction) const;
    std::shared_ptr<Convertion>                 find_convertion(const ConvertionDirection& direction) const;
    std::vector<std::shared_ptr<Convertion>>    find_by_input(StreamTypeRef input_ref) const;
    std::vector<std::shared_ptr<Convertion>>    find_by_output(StreamTypeRef output_ref) const;

    template<class T>
    void connect_add_convertion(T slot, Priority priority) { m_add_convertion_sig.connect(slot, priority); }

    template<class T>
    void connect_remove_convertion(T slot, Priority priority) { m_remove_convertion_sig.connect(slot, priority); }

private:
    class Storage;

    OrderedSignal<void(ConvertionDirection direction, Transaction& t)> m_add_convertion_sig;
    OrderedSignal<void(ConvertionDirection direction, Transaction& t)> m_remove_convertion_sig;

    StreamTypes* m_stream_types;
    std::unique_ptr<Storage> m_storage;
};

}} //namespace TR { namespace Core {