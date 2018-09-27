//
// Created by Darien Pardinas Diaz on 9/23/18.
//

#pragma once

#include <regex>
#include <string>

#include <unordered_map>

class MultiPartFormParser
{

public:
    template <class KeyValueMap>
    bool parse( const KeyValueMap & headers, const std::string & body);

    int contentStartOffset() const;

    int contentSize() const;

    std::string getContentFilename() const;
    std::string getContentName() const;

private:
    std::string m_boundary;

    std::string m_filename;

    std::string m_contentTypeFile;

    int m_contentStart = 0;

    int m_contentSize = 0;

    std::string m_name;
};
