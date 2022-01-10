#pragma once
#include "Values.hxx"
#include "Variant.h"
#include "stddecl.h"
namespace TR { namespace Core {

template<class U>
struct Packing;

template<class P>
struct Unpacking;

template<class U, class... Args>
typename Packing<U>::packed_type pack(const U& unpacked, Args&&... args)
{
    return Packing<U>::pack(unpacked, std::forward<Args>(args)...);
}

template<class U, class... Args>
typename Packing<U*>::packed_type pack(const U* unpacked, Args&&... args)
{
    return Packing<U*>::pack(unpacked, std::forward<Args>(args)...);
}

template<class U, class... Args>
typename Packing<typename std::remove_const<U>::type*>::packed_type pack(std::shared_ptr<U> unpacked, Args&&... args)
{
    return Packing<typename std::remove_const<U>::type*>::pack(unpacked.get(), std::forward<Args>(args)...);
}

template<class P, class... Args>
typename Unpacking<P>::unpacked_type unpack(const P& packed, Args&&... args)
{
    return Unpacking<P>::unpack(packed, std::forward<Args>(args)...);
}

template<class U>
struct Packing<boost::optional<U>>
{
    using packed_type = boost::optional<typename Packing<typename std::remove_const<U>::type>::packed_type>;
    
    template<class... Args>
    static packed_type pack(const boost::optional<U>& unpacked, Args&&... args)
    {
        if (!unpacked)
        {
            return boost::none;
        }
        return Packing<typename std::remove_const<U>::type>::pack(*unpacked, std::forward<Args>(args)...);
    }   
};

template<class P>
struct Unpacking<boost::optional<P>>
{
    using unpacked_type = boost::optional<typename Unpacking<typename std::remove_const<P>::type>::unpacked_type>;

    template<class... Args>
    static unpacked_type unpack(const boost::optional<P>& packed, Args&&... args)
    {
        if (!packed)
        {
            return boost::none;
        }
        return Unpacking<typename std::remove_const<P>::type>::unpack(*packed, std::forward<Args>(args)...);
    }
};

template<class U>
struct Packing<std::vector<U>>
{
    using packed_type = std::vector<typename Packing<typename std::remove_const<U>::type>::packed_type>;

    template<class... Args>
    static packed_type pack(const std::vector<U>& unpacked, Args&&... args)
    {
        packed_type packed;     
        for (auto& u : unpacked)
        {
            packed.push_back(Packing<typename std::remove_const<U>::type>::pack(u, std::forward<Args>(args)...));
        }
        return packed;
    }
};

template<class P>
struct Unpacking<std::vector<P>>
{
    using unpacked_type = std::vector<typename Unpacking<typename std::remove_const<P>::type>::unpacked_type>;

    template<class... Args>
    static unpacked_type unpack(const std::vector<P>& packed, Args&&... args)
    {
        unpacked_type unpacked;
        for (auto& p : packed)
        {
            unpacked.push_back(Unpacking<typename std::remove_const<P>::type>::unpack(p, std::forward<Args>(args)...));
        }   
        return unpacked;
    }
};

template<>
struct Packing<std::wstring>
{
    using packed_type = std::string;
    static std::string pack(const std::wstring& wstr);
};

template<>
struct Unpacking<std::string>
{
    using unpacked_type = std::wstring;
    static std::wstring unpack(const std::string& str);
};

template<class... Args>
struct VariantPacking: boost::static_visitor<boost::variant<typename Packing<Args>::packed_type...>>
{
    template<class U>
    boost::variant<typename Packing<Args>::packed_type...> operator()(U u)
    {
        return pack(u);
    }
};

template<class... Args>
struct Packing<Variant<Args...>>
{
    using packed_type = Variant<typename Packing<Args>::packed_type...>;
    static packed_type pack(const Variant<Args...>& unpacked)
    {
        return boost::apply_visitor(VariantPacking(), unpacked);
    }
};

template<class... Args>
struct VariantUnpacking: boost::static_visitor<boost::variant<typename Unpacking<Args>::unpacked_type...>>
{
    template<class P>
    boost::variant<typename Unpacking<Args>::unpacked_type...> operator()(P p)
    {
        return unpack(p);
    }
};

template<class... Args>
struct Unpacking<Variant<Args...>>
{
    using unpacked_type = Variant<typename Unpacking<Args>::unpacked_type...>;
    static unpacked_type unpack(const Variant<Args...>& packed)
    {
        return boost::apply_visitor(VariantUnpacking(), packed);
    }
};

template<>
struct Packing<XmlQueryDoc*>
{
    using packed_type = boost::optional<XmlQueryDocText>;
    static boost::optional<XmlQueryDocText> pack(const XmlQueryDoc* query_doc);
};

template<>
struct Unpacking<boost::optional<XmlQueryDocText>>
{
    using unpacked_type = std::unique_ptr<XmlQueryDoc>;
    static std::unique_ptr<XmlQueryDoc> unpack(const boost::optional<XmlQueryDocText>& query_xml, const XmlDefDoc& def_doc);
};

template<>
struct Packing<XmlQueryDoc>
{
    using packed_type = XmlQueryDocText;
    static XmlQueryDocText pack(const XmlQueryDoc& query_doc);
};

template<>
struct Unpacking<XmlQueryDocText>
{
    using unpacked_type = std::unique_ptr<XmlQueryDoc>;
    static std::unique_ptr<XmlQueryDoc> unpack(const XmlQueryDocText& query_xml, const XmlDefDoc& def_doc);
};

template<>
struct Packing<XmlDefDoc*>
{
    using packed_type = boost::optional<XmlDefDocText>;
    static boost::optional<XmlDefDocText> pack(const XmlDefDoc* def_doc);
};

template<>
struct Unpacking<boost::optional<XmlDefDocText>>
{
    using unpacked_type = std::unique_ptr<XmlDefDoc>;
    static std::unique_ptr<XmlDefDoc> unpack(const boost::optional<XmlDefDocText>& def_xml);
};

template<>
struct Packing<XmlDefDoc>
{
    using packed_type = XmlDefDocText;
    static XmlDefDocText pack(const XmlDefDoc& def_doc);
};

template<>
struct Unpacking<XmlDefDocText>
{
    using unpacked_type = std::unique_ptr<XmlDefDoc>;
    static std::unique_ptr<XmlDefDoc> unpack(const XmlDefDocText& def_xml);
};

}} //namespace TR { namespace Core {


