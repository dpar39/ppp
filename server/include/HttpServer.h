#pragma once

#include <string>

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <fields_alloc.hpp>
#include <utility>

namespace ip = boost::asio::ip; // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio.hpp>
namespace http = boost::beast::http; // from <boost/beast/http.hpp>

class HttpWorker
{
public:
    HttpWorker(HttpWorker const &) = delete;
    HttpWorker & operator=(HttpWorker const &) = delete;

    HttpWorker(tcp::acceptor & acceptor, std::string doc_root)
    : m_acceptor(acceptor)
    , m_docRoot(doc_root)
    {
    }

    void start()
    {
        accept();
        checkDeadline();
    }

private:
    using alloc_t = fields_alloc<char>;
    // using request_body_t = http::basic_dynamic_body<boost::beast::flat_static_buffer<1024 * 1024>>;
    using request_body_t = http::string_body;

    // The acceptor used to listen for incoming connections.
    tcp::acceptor & m_acceptor;

    // The path to the root of the document directory.
    std::string m_docRoot;

    // The socket for the currently connected client.
    tcp::socket m_socket { m_acceptor.get_executor().context() };

    // The buffer for performing reads
    boost::beast::flat_static_buffer<8192> m_buffer;

    // The allocator used for the fields in the request and reply.
    alloc_t alloc_ { 8192 };

    // The parser for reading the requests
    boost::optional<http::request_parser<request_body_t, alloc_t>> _parser;

    // The timer putting a time limit on requests.
    boost::asio::basic_waitable_timer<std::chrono::steady_clock> m_requestDeadline {
        m_acceptor.get_executor().context(),
        (std::chrono::steady_clock::time_point::max)()
    };

    // The string-based response message.
    boost::optional<http::response<http::string_body, http::basic_fields<alloc_t>>> m_stringResponse;

    // The string-based response serializer.
    boost::optional<http::response_serializer<http::string_body, http::basic_fields<alloc_t>>> m_stringSerializer;

    // The file-based response message.
    boost::optional<http::response<http::file_body, http::basic_fields<alloc_t>>> m_fileResponse;

    // The file-based response serializer.
    boost::optional<http::response_serializer<http::file_body, http::basic_fields<alloc_t>>> m_fileSerializer;

    void accept();

    void readRequest();

    void processRequest(http::request<request_body_t, http::basic_fields<alloc_t>> const & req);

    void sendBadResponse(http::status status, std::string const & error);

    void sendFile(boost::beast::string_view target);

    void checkDeadline();
};
