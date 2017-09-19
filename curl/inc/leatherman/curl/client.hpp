/**
* @file
* Declares the HTTP client.
*/
#pragma once

#include <leatherman/util/scoped_resource.hpp>
#include <leatherman/locale/locale.hpp>
#include "request.hpp"
#include "response.hpp"
#include <curl/curl.h>
#include <boost/optional.hpp>
#include <boost/filesystem.hpp>
#include <boost/nowide/cstdio.hpp>
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
     * Resource for a temporary file used during download
     */
    struct LEATHERMAN_CURL_NO_EXPORT download_temp_file
    {
        /**
         * Constructs a temporary file that will be used to store the downloaded file's
         * contents.
         * @param req The HTTP request.
         * @param file_path The file path that this temporary file's contents will be written to.
         * @param perms The (optional) permissions of the downloaded file.
         */
        download_temp_file(request const& req, std::string const& file_path, boost::optional<boost::filesystem::perms> perms);

        ~download_temp_file();

        /*
         * Returns the underlying file pointer.
         */
        FILE* get_fp();

        /*
         * Writes the temporary file's contents to its file path.
         */
        void write();

        /*
         * Writes the temporary file's contents to the body of the
         * given response.
         * @param response The HTTP response to write the contents to
         */
        void write(response& res);

     private:
        void close_fp();
        void cleanup();
        FILE* _fp;
        request _req;
        std::string _file_path;
        boost::filesystem::path _temp_path;
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
     * The exception for curl_easy_setopt errors.
     */
    struct LEATHERMAN_CURL_EXPORT http_curl_setup_exception : http_request_exception
    {
        /**
         * Constructs an http_curl_setup_exception.
         * @param req The HTTP request that caused the exception.
         * @param message The exception message.
         * @param curl_opt The CURL option that failed.
         */
        http_curl_setup_exception(request req, CURLoption curl_opt, std::string const &message) :
            http_request_exception(req, message),
            _curl_opt(std::move(curl_opt))
        {
        }

        /**
         * Gets the CURL option associated with the exception
         * @return Returns the CURL option associated with the exception.
         */
        CURLoption const& curl_opt() const
        {
            return _curl_opt;
        }


     private:
        CURLoption _curl_opt;
    };

    /**
     * The exception for HTTP file download server-side errors.
     */
    struct LEATHERMAN_CURL_EXPORT http_file_download_exception : http_request_exception
    {
        /**
         * Constructs an http_file_download_exception.
         * @param request The request that caused the exception
         * @param file_path The file that was meant to be downloaded
         * @param message The exception message.
         */
        http_file_download_exception(request req, std::string file_path, std::string const &message) :
          http_request_exception(req, message),
          _file_path(std::move(file_path))
        {
        }

        /**
         * Gets the file_path associated with the exception
         * @return Returns the file_path associated with the exception.
         */
        std::string const& file_path() const
        {
            return _file_path;
        }

     private:
        std::string _file_path;
    };

    /**
     * The exception for HTTP file download file operation errors.
     */
    struct LEATHERMAN_CURL_EXPORT http_file_operation_exception : http_request_exception
    {
        /**
         * Constructs an http_file_operation_exception.
         * @param request The request that caused the exception
         * @param file_path The file that was meant to be downloaded
         * @param message The exception message.
         */
        http_file_operation_exception(request req, std::string file_path, std::string const &message) : http_file_operation_exception(req, file_path, "", message)
        {
        }

        /**
         * Constructs an http_file_operation_exception.
         * @param request The request that caused the exception
         * @param file_path The file that was meant to be downloaded
         * @param temp_path The path to the temporary file that wasn't successfully cleaned up.
         * @param message The exception message.
         */
        http_file_operation_exception(request req, std::string file_path, std::string temp_path, std::string const &message) :
            http_request_exception(req, message),
            _file_path(file_path),
            _temp_path(std::move(temp_path))
        {
        }

        /**
         * Gets the file_path associated with the exception
         * @return Returns the file_path associated with the exception.
         */
        std::string const& file_path() const
        {
            return _file_path;
        }

        /**
         * Gets the temp_path associated with the exception
         * @return Returns the temp_path associated with the exception.
         */
        std::string const& temp_path() const
        {
            return _temp_path;
        }

     private:
        std::string _file_path;
        std::string _temp_path;
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
         * Downloads the file from the specified url.
         * Throws http_file_download_exception if anything goes wrong.
         * @param req The HTTP request to perform.
         * @param file_path The file that the downloaded contents will be written to.
         * @param perms The file permissions to apply when writing to file_path.
         *              On Windows this only toggles read-only.
         */
        void download_file(request const& req,
                           std::string const& file_path,
                           boost::optional<boost::filesystem::perms> perms = {});

        /**
         * Downloads the file from the specified url.
         * Throws http_file_download_exception if anything goes wrong.
         * @param req The HTTP request to perform.
         * @param file_path The file that the downloaded contents will be written to.
         * @param response The HTTP response. The body will only be included if the response status is >= 400.
         * @param perms The file permissions to apply when writing to file_path.
         *              On Windows this only toggles read-only.
         */
        void download_file(request const& req,
                           std::string const& file_path,
                           response& res,
                           boost::optional<boost::filesystem::perms> perms = {});

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
        void download_file_helper(request const& req,
                                  std::string const& file_path,
                                  boost::optional<response&> res = {},
                                  boost::optional<boost::filesystem::perms> perms = {});

        LEATHERMAN_CURL_NO_EXPORT void set_method(context& ctx, http_method method);
        LEATHERMAN_CURL_NO_EXPORT void set_url(context& ctx);
        LEATHERMAN_CURL_NO_EXPORT void set_headers(context& ctx);
        LEATHERMAN_CURL_NO_EXPORT void set_cookies(context& ctx);
        LEATHERMAN_CURL_NO_EXPORT void set_body(context& ctx, http_method method);
        LEATHERMAN_CURL_NO_EXPORT void set_timeouts(context& ctx);
        LEATHERMAN_CURL_NO_EXPORT void set_header_write_callbacks(context& ctx);
        LEATHERMAN_CURL_NO_EXPORT void set_write_callbacks(context& ctx);
        LEATHERMAN_CURL_NO_EXPORT void set_write_callbacks(context& ctx, FILE* fp);
        LEATHERMAN_CURL_NO_EXPORT void set_client_info(context &ctx);
        LEATHERMAN_CURL_NO_EXPORT void set_ca_info(context& ctx);
        LEATHERMAN_CURL_NO_EXPORT void set_client_protocols(context& ctx);

        template <typename ParamType>
        LEATHERMAN_CURL_NO_EXPORT void curl_easy_setopt_maybe(
            context &ctx,
            CURLoption option,
            ParamType const& param
        ) {
            auto result = curl_easy_setopt(_handle, option, param);
            if (result != CURLE_OK) {
                throw http_curl_setup_exception(ctx.req, option, leatherman::locale::_("Failed setting up libcurl. Reason: {1}", curl_easy_strerror(result)));
            }
        }

        static size_t read_body(char* buffer, size_t size, size_t count, void* ptr);
        static int seek_body(void* ptr, curl_off_t offset, int origin);
        static size_t write_header(char* buffer, size_t size, size_t count, void* ptr);
        static size_t write_body(char* buffer, size_t size, size_t count, void* ptr);
        static size_t write_file(char *buffer, size_t size, size_t count, void* ptr);
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
