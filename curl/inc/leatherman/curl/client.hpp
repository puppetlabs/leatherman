/**
* @file
* Declares the HTTP client.
*/
#pragma once

#include <leatherman/util/scoped_resource.hpp>
#include "request.hpp"
#include "response.hpp"
#include <curl/curl.h>


namespace leatherman { namespace curl {

    /**
     * Resource for a cURL handle.
     */
    struct CurlHandle : util::scoped_resource<CURL*>
    {
        /**
         * Constructs a cURL handle.
         */
        CurlHandle();

     private:
        static void cleanup(CURL* curl);
    };

    /**
     * Resource for a cURL linked-list.
     */
    struct CurlList : util::scoped_resource<curl_slist*>
    {
        /**
         * Constructs a curl_list.
         */
        CurlList();

        /**
         * Appends the given string onto the list.
         * @param value The string to append onto the list.
         */
        void append(std::string const& value);

     private:
        static void cleanup(curl_slist* list);
    };

    /**
     * Resource for a cURL escaped string.
     */
    struct CurlEscapedString : util::scoped_resource<char const*>
    {
        /**
         * Constructs a cURL escaped string.
         * @param handle The cURL handle to use to perform the escape.
         * @param str The string to escape.
         */
        CurlEscapedString(CurlHandle const& handle, std::string const& str);

     private:
        static void cleanup(char const* str);
    };

    /**
     * The exception for HTTP.
     */
    struct http_exception : std::runtime_error
    {
        /**
         * Constructs an http_exception.
         * @param message The exception message.
         */
        http_exception(std::string const &message) :
            runtime_error(message)
        {
        }
    };

    /**
     * The exception for HTTP requests.
     */
    struct http_request_exception : http_exception
    {
        /**
         * Constructs an http_request_exception.
         * @param req The HTTP request that caused the exception.
         * @param message The exception message.
         */
        http_request_exception(Request req, std::string const &message) :
            http_exception(message),
            _req(std::move(req))
        {
        }

        /**
         * Gets the request associated with the exception
         * @return Returns the request associated with the exception.
         */
        Request const& req() const
        {
            return _req;
        }

     private:
        Request _req;
    };

    /**
     * Implements a client for HTTP.
     * Note: this class is not thread-safe.
     */
    struct Client
    {
        /**
         * Constructs an HTTP client.
         */
        Client();

        /**
         * Moves the given client into this client.
         * @param other The client to move into this client.
         */
        Client(Client&& other);

        /**
         * Moves the given client into this client.
         * @param other The client to move into this client.
         * @return Returns this client.
         */
        Client& operator=(Client&& other);

        /**
         * Performs a GET with the given request.
         * @param req The HTTP request to perform.
         * @return Returns the HTTP response.
         */
        Response get(Request const& req);

        /**
         * Performs a POST with the given request.
         * @param req The HTTP request to perform.
         * @return Returns the HTTP response.
         */
        Response post(Request const& req);

        /**
         * Performs a PUT with the given request.
         * @param req The HTTP request to perform.
         * @return Returns the HTTP response.
         */
        Response put(Request const& req);

     private:
        Client(Client const&) = delete;
        Client& operator=(Client const&) = delete;

        enum struct http_method
        {
            get,
            put,
            post
        };

        struct context
        {
            context(Request const& req, Response& res) :
                req(req),
                res(res),
                read_offset(0)
            {
            }

            Request const& req;
            Response& res;
            size_t read_offset;
            CurlList request_headers;
            std::string response_buffer;
        };

        Response perform(http_method method, Request const& req);
        void set_method(context& ctx, http_method method);
        void set_url(context& ctx);
        void set_headers(context& ctx);
        void set_cookies(context& ctx);
        void set_body(context& ctx);
        void set_timeouts(context& ctx);
        void set_write_callbacks(context& ctx);

        static size_t read_body(char* buffer, size_t size, size_t count, void* ptr);
        static size_t write_header(char* buffer, size_t size, size_t count, void* ptr);
        static size_t write_body(char* buffer, size_t size, size_t count, void* ptr);
        static int debug(CURL* handle, curl_infotype type, char* data, size_t size, void* ptr);

        CurlHandle _handle;
    };

}}  // namespace leatherman::curl
