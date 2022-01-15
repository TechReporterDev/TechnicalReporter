#pragma once
#include "BackgndFwd.h"
#include "BasisFwd.h"
#include "BasisRefs.h"
#include "stddecl.h"
namespace TR { namespace Core {
class SourceRef;

class Generator
{
public:
    using Functor = std::function<std::unique_ptr<Content>()>;
    
    Generator(UUID uuid):
        m_uuid(std::move(uuid))
    {
    }
    virtual ~Generator() = default;

    UUID                        get_uuid() const { return m_uuid; }
    virtual SourceTypeRef       get_source_type() const = 0;
    virtual ReportTypeRef       get_report_type() const = 0;

    virtual Functor             prepare_functor(SourceRef source_ref) const = 0;

private:
    UUID m_uuid;
};

class Generators
{
public:
    static const Priority GENERATORS_PRIORITY = 650;
    
    Generators();
    ~Generators();

    void                                        set_source_types(SourceTypes* source_types);
    void                                        set_report_types(ReportTypes* report_types);

    void                                        add_generator(std::shared_ptr<Generator> generator, Transaction& t);
    void                                        remove_generator(const UUID& uuid, Transaction& t);
    std::shared_ptr<Generator>                  get_generator(const UUID& uuid) const;
    std::vector<std::shared_ptr<Generator>>     find_by_source_type(SourceTypeRef source_type_ref) const;
    std::vector<std::shared_ptr<Generator>>     find_by_output(ReportTypeRef report_type_ref) const;

    template<class T>
    void connect_add_generator(T slot, Priority priority) { m_add_generator_sig.connect(slot, priority); }

    template<class T>
    void connect_remove_generator(T slot, Priority priority) { m_remove_generator_sig.connect(slot, priority); }

private:
    class Storage;  

    OrderedSignal<void(UUID uuid, Transaction& t)> m_add_generator_sig;
    OrderedSignal<void(UUID uuid, Transaction& t)> m_remove_generator_sig;

    ReportTypes* m_report_types;
    SourceTypes* m_source_types;
    std::unique_ptr<Storage> m_storage;
};

}} //namespace TR { namespace Core {
