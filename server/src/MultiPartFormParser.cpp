//
// Created by Darien Pardinas Diaz on 9/23/18.
//

#include "MultiPartFormParser.h"
#include <crow_all.h>
#include <regex>

bool MultiPartFormParser::parse(const crow::request & req)
{

    auto it = req.headers.find("Content-Type");
    if (it == req.headers.end())
    {
        return false;
    }

    const auto & contentType = it->second;
    static const std::regex e("multipart\\/form-data; boundary=(.*)$");

    std::smatch m;
    if (!std::regex_search(contentType, m, e))
    {
        return false;
    }
    m_boundary = m[1].str();

    const auto & body = req.body;

    static const std::string lineBreak("\r\n");
    const auto startBoundary = std::string("--") + m_boundary + lineBreak;
    const auto endBoundary = std::string("--") + m_boundary + std::string("--");

    auto itc = body.begin();
    const auto itcEnd = body.end();

    auto itc2 = std::search(itc, itcEnd, startBoundary.begin(), startBoundary.end());
    if (itc2 == itcEnd)
    {
        return false;
    }
    itc = std::next(itc2, startBoundary.size());

    // Content-Disposition: form-data; name="uploads[]"; filename="image_0001.jpg"
    itc2 = std::search(itc, itcEnd, lineBreak.begin(), lineBreak.end());
    if (itc2 == itcEnd)
    {
        return false;
    }

    static const std::regex e2("^Content-Disposition: form-data; name=\"([^\"]*)\"(?:; filename=\"([^\"]*)\")");
    if (!std::regex_search(itc, itc2, m, e2))
    {
        return false;
    }

    m_name = m[1].str();
    m_filename = m[2].str();

    itc = std::next(itc2, lineBreak.size());

    // Content-Type: image/jpeg
    itc2 = std::search(itc, itcEnd, lineBreak.begin(), lineBreak.end());
    if (itc2 == itcEnd)
    {
        return false;
    }

    static const std::regex e3("^Content-Type: ([^\"]*)$");
    if (!std::regex_search(itc, itc2, m, e3))
    {
        return false;
    }
    m_contentTypeFile = m[1].str();

    itc = std::next(itc2, lineBreak.size());

    // Another line break before content really starts
    itc2 = std::search(itc, itcEnd, lineBreak.begin(), lineBreak.end());
    if (itc2 == itcEnd)
    {
        return false;
    }
    itc = std::next(itc2, lineBreak.size());

    itc2 = std::search(itc, itcEnd, endBoundary.begin(), endBoundary.end());
    if (itc2 == itcEnd)
    {
        return false;
    }

    m_contentStart = std::distance(body.begin(), itc);
    m_contentSize = std::distance(itc, itc2);

    return true;
}

int MultiPartFormParser::contentStartOffset() const
{
    return m_contentStart;
}

int MultiPartFormParser::contentSize() const
{
    return m_contentSize;
}

std::string MultiPartFormParser::getContentFilename() const
{
    return m_filename;
}

std::string MultiPartFormParser::getContentName() const
{
    return m_name;
}
