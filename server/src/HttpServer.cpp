// #include "crow_all.h"

// int main()
// {
//     crow::SimpleApp app;

//     CROW_ROUTE(app, "/")([](){
//         return "Hello world";
//     });

//     app.port(18080).multithreaded().run();
// }
#include "HttpServer.h"
#include "fields_alloc.hpp"

#include <boost/filesystem.hpp>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>

// Return a reasonable mime type based on the extension of a file.
boost::beast::string_view mime_type(boost::beast::string_view path)
{
    using boost::beast::iequals;
    auto const ext = [&path] {
        auto const pos = path.rfind(".");
        if (pos == boost::beast::string_view::npos)
            return boost::beast::string_view {};
        return path.substr(pos);
    }();

    const auto str = ext.to_string();

    static std::unordered_map<std::string, std::string> s_knownMimeTypes = { { "htm", "text/html" },
                                                                             { ".html", "text/html" },
                                                                             { ".css", "text/css" },
                                                                             { ".txt", "text/plain" },
                                                                             { ".js", "application/javascript" },
                                                                             { ".json", "application/json" },
                                                                             { ".xml", "application/xml" },
                                                                             { ".swf", "application/x-shockwave-flash" },
                                                                             { ".flv", "video/x-flv" },
                                                                             { ".png", "image/png" },
                                                                             { ".jpe", "image/jpeg" },
                                                                             { ".jpeg", "image/jpeg" },
                                                                             { ".jpg", "image/jpeg" },
                                                                             { ".gif", "image/gif" },
                                                                             { ".bmp", "image/bmp" },
                                                                             { ".ico", "image/vnd.microsoft.icon" },
                                                                             { ".tiff", "image/tiff" },
                                                                             { ".tif", "image/tiff" },
                                                                             { ".svg", "image/svg+xml" },
                                                                             { ".svgz", "image/svg+xml" } };
    const auto it = s_knownMimeTypes.find(str);
    if (it != s_knownMimeTypes.end())
    {
        return it->second;
    }
    return "application/text";
}

void HttpWorker::accept()
{
    // Clean up any previous connection.
    boost::beast::error_code ec;
    m_socket.close(ec);
    m_buffer.consume(m_buffer.size());
    m_acceptor.async_accept(m_socket, [this](const boost::beast::error_code ec) {
        if (ec)
        {
            accept();
        }
        else
        {
            // Request must be fully processed within 60 seconds.
            m_requestDeadline.expires_after(std::chrono::seconds(60));
            readRequest();
        }
    });
}

void HttpWorker::readRequest()
{
    // On each read the parser needs to be destroyed and
    // recreated. We store it in a boost::optional to
    // achieve that.
    //
    // Arguments passed to the parser constructor are
    // forwarded to the message object. A single argument
    // is forwarded to the body constructor.
    //
    // We construct the dynamic body with a 1MB limit
    // to prevent vulnerability to buffer attacks.
    //
    _parser.emplace(std::piecewise_construct, std::make_tuple(), std::make_tuple(alloc_));

    http::async_read(m_socket, m_buffer, *_parser, [this](boost::beast::error_code ec, std::size_t) {
        if (ec)
            accept();
        else
            processRequest(_parser->get());
    });
}

void HttpWorker::processRequest(http::request<request_body_t, http::basic_fields<alloc_t>> const & req)
{
    const auto & req_target = req.target();
    switch (req.method())
    {
        case http::verb::get:
            sendFile(req_target);
            break;

        default:
            // We return responses indicating an error if
            // we do not recognize the request method.
            sendBadResponse(http::status::bad_request,
                            "Invalid request-method '" + req.method_string().to_string() + "'\r\n");
            break;
    }
}

void HttpWorker::sendBadResponse(const http::status status, std::string const & error)
{
    m_stringResponse.emplace(std::piecewise_construct, std::make_tuple(), std::make_tuple(alloc_));

    m_stringResponse->result(status);
    m_stringResponse->keep_alive(false);
    m_stringResponse->set(http::field::server, "Beast");
    m_stringResponse->set(http::field::content_type, "text/plain");
    m_stringResponse->body() = error;
    m_stringResponse->prepare_payload();

    m_stringSerializer.emplace(*m_stringResponse);

    http::async_write(m_socket, *m_stringSerializer, [this](boost::beast::error_code ec, std::size_t) {
        m_socket.shutdown(tcp::socket::shutdown_send, ec);
        m_stringSerializer.reset();
        m_stringResponse.reset();
        accept();
    });
}

void HttpWorker::sendFile(boost::beast::string_view target)
{
    // Request path must be absolute and not contain "..".
    if (target.empty() || target[0] != '/' || target.find("..") != std::string::npos)
    {
        sendBadResponse(http::status::not_found, "File not found\r\n");
        return;
    }

    auto full_path = m_docRoot;
    full_path.append(target.data(), target.size());

    http::file_body::value_type file;
    boost::beast::error_code ec;
    file.open(full_path.c_str(), boost::beast::file_mode::read, ec);
    if (ec)
    {
        sendBadResponse(http::status::not_found, "File not found\r\n");
        return;
    }

    m_fileResponse.emplace(std::piecewise_construct, std::make_tuple(), std::make_tuple(alloc_));

    m_fileResponse->result(http::status::ok);
    m_fileResponse->keep_alive(false);
    m_fileResponse->set(http::field::server, "Beast");
    m_fileResponse->set(http::field::content_type, mime_type(target.to_string()));
    m_fileResponse->body() = std::move(file);
    m_fileResponse->prepare_payload();

    m_fileSerializer.emplace(*m_fileResponse);

    http::async_write(m_socket, *m_fileSerializer, [this](boost::beast::error_code ec, std::size_t) {
        m_socket.shutdown(tcp::socket::shutdown_send, ec);
        m_fileSerializer.reset();
        m_fileResponse.reset();
        accept();
    });
}

void HttpWorker::checkDeadline()
{
    // The deadline may have moved, so check it has really passed.
    if (m_requestDeadline.expiry() <= std::chrono::steady_clock::now())
    {
        // Close socket to cancel any outstanding operation.
        boost::beast::error_code ec;
        m_socket.close();

        // Sleep indefinitely until we're given a new deadline.
        m_requestDeadline.expires_at(std::chrono::steady_clock::time_point::max());
    }

    m_requestDeadline.async_wait([this](boost::beast::error_code) { checkDeadline(); });
}
