#pragma once
#include "TextDiff.h"
#include "XMLDiff.h"
#include "TrivialTypes.h"
#include "TRXML\TRXMLFwd.h"

namespace TR { namespace Core {

class Diff
{
public:
    virtual ~Diff() = default;
    virtual Blob as_blob() const = 0;
};

class PlainTextDiff: public Diff
{
public: 
    using DiffLines = std::vector<DiffLine>;
    using ConstIterator = DiffLines::const_iterator;

    PlainTextDiff(Blob blob);
    PlainTextDiff(DiffLines diff_lines);

    // Diff override
    virtual Blob        as_blob() const;

    // PlainTextDiff methods    
    const DiffLines&    get_diff_lines() const;


private:
    DiffLines m_diff_lines; 
};

class PlainXmlDiff: public Diff
{
public: 
    using DiffPoints = std::vector<XmlDiffPoint>;

    PlainXmlDiff(Blob blob);
    PlainXmlDiff(DiffPoints diff_points);

    // Diff override
    virtual Blob        as_blob() const;

    // PlainXmlDiff methods 
    const DiffPoints&   get_diff_points() const;

protected:
    DiffPoints m_diff_points;
};

class RegularDiff: public PlainXmlDiff
{
public:
    using XmlDefDoc = XML::XmlDefDoc;

    RegularDiff(Blob blob, std::shared_ptr<const XmlDefDoc> def_doc);
    RegularDiff(DiffPoints diff_points, std::shared_ptr<const XmlDefDoc> def_doc);
    
    // RegularDiff methods  
    std::wstring format_path(size_t diff_point_pos) const;

private:
    std::shared_ptr<const XmlDefDoc> m_def_doc;
};

}} //namespace TR {