#pragma once

#include <functional>
#include <regex>
#include <string>
#include <unordered_set>

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <fields_alloc.hpp>

#include "utility.h"

namespace ip = boost::asio::ip; // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio.hpp>
namespace http = boost::beast::http; // from <boost/beast/http.hpp>

using request_body_t = http::string_body;
using alloc_t = fields_alloc<char>;

using Request = http::request<request_body_t, http::basic_fields<alloc_t>>;
using StringResponse = http::response<http::string_body, http::basic_fields<alloc_t>>;
using FileResponse = http::response<http::file_body, http::basic_fields<alloc_t>>;

class Response
{

public:
    Response(StringResponse & res)
    : _res(res)
    {
    }

    void setHeader(const std::string & key, const std::string & value);

    void write(const std::string & content);

    //    template<typename T>
    //    friend Response & operator<<(const Response &res, const T & content){
    //        res.write()
    //        return res;
    //    }
    void end() {

    }
private:
    StringResponse & _res;
};

using RouteHandler = std::function<void(const Request & req, Response & res)>;

FWD_DECL(RouteDefinition);
class RouteDefinition
{

private:
    std::string _routeString;

    std::regex _routeRegex;

    RouteHandler & _handler;

    std::unordered_set<http::verb> _verbs;

    RouteDefinition(std::string routeString, std::initializer_list<http::verb> verbs, RouteHandler routeHandler);

public:
    static RouteDefinitionSPtr create(std::string routeString,
                                      std::initializer_list<http::verb> verbs,
                                      RouteHandler routeHandler);

    bool handle(const Request & req, Response & res) const;

private:
};

class HttpWorker
{
public:
    HttpWorker(HttpWorker const &) = delete;
    HttpWorker & operator=(HttpWorker const &) = delete;

    explicit HttpWorker(tcp::acceptor & acceptor)
    : m_acceptor(acceptor)
    {
    }

    void addRoute(RouteDefinitionSPtr route)
    {
        m_routeDefinitions.push_back(route);
    }

    void start()
    {
        accept();
        checkDeadline();
    }

private:
    // using request_body_t = http::basic_dynamic_body<boost::beast::flat_static_buffer<1024 * 1024>>;

    // The acceptor used to listen for incoming connections.
    tcp::acceptor & m_acceptor;

    // The path to the root of the document directory.
    std::string m_docRoot;

    // The socket for the currently connected client.
    tcp::socket m_socket{ m_acceptor.get_executor().context() };

    // The buffer for performing reads
    boost::beast::flat_static_buffer<8192> m_buffer;

    // The allocator used for the fields in the request and reply.
    alloc_t m_alloc{ 8192 };

    // The parser for reading the requests
    boost::optional<http::request_parser<request_body_t, alloc_t>> m_parser;

    // The timer putting a time limit on requests.
    boost::asio::basic_waitable_timer<std::chrono::steady_clock>
        m_requestDeadline{ m_acceptor.get_executor().context(), (std::chrono::steady_clock::time_point::max)() };

    boost::optional<http::response<http::string_body, http::basic_fields<alloc_t>>> m_stringResponse;
    boost::optional<http::response_serializer<http::string_body, http::basic_fields<alloc_t>>> m_stringSerializer;
    boost::optional<http::response<http::file_body, http::basic_fields<alloc_t>>> m_fileResponse;
    boost::optional<http::response_serializer<http::file_body, http::basic_fields<alloc_t>>> m_fileSerializer;

    std::vector<RouteDefinitionSPtr> m_routeDefinitions;

    void accept();

    void readRequest();

    void processRequest(http::request<request_body_t, http::basic_fields<alloc_t>> const & req);

    void sendBadResponse(http::status status, std::string const & error);

    bool sendFile(boost::beast::string_view target);

    void checkDeadline();
};

class HttpServer
{
private:
    std::string _address = "0.0.0.0";
    uint16_t _port = 4000;
    int _numWorkers = 1;

    std::vector<std::string> _staticPaths;

    std::vector<RouteDefinitionSPtr> _routeDefinitions;

public:
    void configure(uint16_t port, int numWorkers)
    {
    }

    bool addStatic(const std::string & pathPrefix)
    {
        _staticPaths.push_back(pathPrefix);
        return true;
    }

    void addRoute(RouteDefinitionSPtr route)
    {
        _routeDefinitions.push_back(route);
    }

    int run();
};