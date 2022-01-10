#pragma once
#include "stddecl.h"
#include <boost\regex.hpp>
namespace TR { namespace External {
using Preprocessor = std::function<std::string(const std::string&)>;

struct ComposedPreprocessor
{
    void add_regex_replace(const std::string& pattern, const std::string& replacement)
    {
        boost::regex regex(pattern);
        m_preprocessors.push_back([regex, replacement](const std::string& input){
            return boost::regex_replace(input, regex, replacement);
        });
    }

    std::string operator()(const std::string& input) const
    {
        std::string output = input;
        for (auto& preprocessor : m_preprocessors)
        {
            output = preprocessor(output);
        }
        return output;
    }

    bool empty() const
    {
        return m_preprocessors.empty();
    }

    std::vector<Preprocessor> m_preprocessors;
};

}} //namespace TR { namespace External {