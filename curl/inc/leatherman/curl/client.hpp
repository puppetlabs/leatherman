/**
* @file
* Declares the HTTP client.
*/
#pragma once

#include <leatherman/util/scoped_resource.hpp>
#include "request.hpp"
#include "response.hpp"
#include <curl/curl.h>
#include "export.h"


namespace leatherman { namespace curl {

    /**
     * Resource for a cURL handle.
     */
    struct LEATHERMAN_CURL_EXPORT curl_handle : util::scoped_resource<CURL*>
    {
        /**
         * Constructs a cURL handle.
         */
        curl_handle();

     private:
        static void cleanup(CURL* curl);
    };

    /**
     * Resource for a cURL linked-list.
     */
    struct LEATHERMAN_CURL_EXPORT curl_list : util::scoped_resource<curl_slist*>
    {
        /**
         * Constructs a curl_list.
         */
        curl_list();

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
    struct LEATHERMAN_CURL_EXPORT curl_escaped_string : util::scoped_resource<char const*>
    {
        /**
         * Constructs a cURL escaped string.
         * @param handle The cURL handle to use to perform the escape.
         * @param str The string to escape.
         */
        curl_escaped_string(curl_handle const& handle, std::string const& str);

     private:
        static void cleanup(char const* str);
    };

    /**
     * The exception for HTTP.
     */
    struct LEATHERMAN_CURL_EXPORT http_exception : std::runtime_error
    {
        /**
         * Constructs an http_exception.
         * @param message The exception message.
         */
        http_exception(std::string const& message) :
            runtime_error(message)
        {
        }
    };

    /**
     * The exception for HTTP requests.
     */
    struct LEATHERMAN_CURL_EXPORT http_request_exception : http_exception
    {
        /**
         * Constructs an http_request_exception.
         * @param req The HTTP request that caused the exception.
         * @param message The exception message.
         */
        http_request_exception(request req, std::string const &message) :
            http_exception(message),
            _req(std::move(req))
        {
        }

        /**
         * Gets the request associated with the exception
         * @return Returns the request associated with the exception.
         */
        request const& req() const
        {
            return _req;
        }

     private:
        request _req;
    };

    /**
     * Implements a client for HTTP.
     * Note: this class is not thread-safe.
     */
    struct LEATHERMAN_CURL_EXPORT client
    {
        /**
         * Constructs an HTTP client.
         */
        client();

        /**
         * Moves the given client into this client.
         * @param other The client to move into this client.
         */
        client(client&& other);

        /**
         * Moves the given client into this client.
         * @param other The client to move into this client.
         * @return Returns this client.
         */
        client& operator=(client&& other);

        /**
         * Performs a GET with the given request.
         * @param req The HTTP request to perform.
         * @return Returns the HTTP response.
         */
        response get(request const& req);

        /**
         * Performs a POST with the given request.
         * @param req The HTTP request to perform.
         * @return Returns the HTTP response.
         */
        response post(request const& req);

        /**
         * Performs a PUT with the given request.
         * @param req The HTTP request to perform.
         * @return Returns the HTTP response.
         */
        response put(request const& req);

        /**
         * Sets the path to the CA certificate file.
         * @param cert_file The path to the CA certificate file.
         */
        void set_ca_cert(std::string const& cert_file);

        /**
         * Set client SSL certificate and key.
         * @param client_cert The path to the client's certificate file.
         * @param client_key The path to the client's key file.
         */
        void set_client_cert(std::string const& client_cert, std::string const& client_key);

        /**
         * Set and limit what protocols curl will support
         * @param client_protocols bitmask of CURLPROTO_*
         *        (see more: http://curl.haxx.se/libcurl/c/CURLOPT_PROTOCOLS.html)
         */
        void set_supported_protocols(long client_protocols);

     private:
        client(client const&) = delete;
        client& operator=(client const&) = delete;

        enum struct http_method
        {
            get,
            put,
            post
        };

        struct context
        {
            context(request const& req, response& res) :
                req(req),
                res(res),
                read_offset(0)
            {
            }

            request const& req;
            response& res;
            size_t read_offset;
            curl_list request_headers;
            std::string response_buffer;
        };

        std::string _ca_cert;
        std::string _client_cert;
        std::string _client_key;
        long _client_protocols = CURLPROTO_ALL;

        response perform(http_method method, request const& req);
        LEATHERMAN_CURL_NO_EXPORT void set_method(context& ctx, http_method method);
        LEATHERMAN_CURL_NO_EXPORT void set_url(context& ctx);
        LEATHERMAN_CURL_NO_EXPORT void set_headers(context& ctx);
        LEATHERMAN_CURL_NO_EXPORT void set_cookies(context& ctx);
        LEATHERMAN_CURL_NO_EXPORT void set_body(context& ctx, http_method method);
        LEATHERMAN_CURL_NO_EXPORT void set_timeouts(context& ctx);
        LEATHERMAN_CURL_NO_EXPORT void set_write_callbacks(context& ctx);
        LEATHERMAN_CURL_NO_EXPORT void set_client_info(context &ctx);
        LEATHERMAN_CURL_NO_EXPORT void set_ca_info(context& ctx);
        LEATHERMAN_CURL_NO_EXPORT void set_client_protocols(context& ctx);

        static size_t read_body(char* buffer, size_t size, size_t count, void* ptr);
        static int seek_body(void* ptr, curl_off_t offset, int origin);
        static size_t write_header(char* buffer, size_t size, size_t count, void* ptr);
        static size_t write_body(char* buffer, size_t size, size_t count, void* ptr);
        static int debug(CURL* handle, curl_infotype type, char* data, size_t size, void* ptr);

        curl_handle _handle;

    protected:
        /**
         * Returns a reference to a cURL handle resource used in the request.
         * This is primarily exposed for testing.
         * @return Returns a const reference to the cURL handle resource.
         */
        curl_handle const& get_handle();
    };

}}  // namespace leatherman::curl
