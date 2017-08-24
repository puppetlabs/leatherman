#pragma once

#include <string>
#include <functional>
#include <curl/curl.h>
#ifdef _WIN32
    #include "export.h"
#else
    #define MOCK_CURL_EXPORT
#endif

struct curl_impl
{
    enum struct http_method
    {
        get,
        put,
        post
    };

    enum struct error_mode
    {
        success,
        easy_perform_error,
        easy_perform_write_error,
        http_post_error,
        http_put_error,
        set_url_error,
        set_header_error,
        set_cookie_error,
        header_function_error,
        header_context_error,
        write_body_function_error,
        write_body_context_error,
        read_body_function_error,
        read_body_context_error,
        connect_timeout_error,
        request_timeout_error,
        ca_bundle_error,
        ssl_cert_error,
        ssl_key_error,
        protocol_error,
    };

    error_mode test_failure_mode = error_mode::success;

    // Pointer for client::write_header as a callback function in curl_easy_setopt
    std::function<size_t(char*, size_t, size_t, void*)> write_header;
    void* header_context; // Where to write the header part of the received data to

    // Pointer for client::write_body as a callback function in curl_easy_setopt
    std::function<size_t(char*, size_t, size_t, void*)> write_body;
    void* body_context; // Where to write the body part of the received data to

    // Pointer for client::read_body as a callback function in curl_easy_setopt
    std::function<size_t(char*, size_t, size_t, void*)> read_function;
    void* read_data; // Where to read the request body from

    std::string request_url, cookie, cacert, client_cert, client_key;
    long protocols;
    long connect_timeout;
    http_method method = http_method::get;

    curl_slist* header; // List of custom request headers to be passed to the server

    std::string read_buffer; // Buffer to test reading the request body
    std::string resp_body;   // Response body which should be written to a context using the write_body function callback

    char* errbuf = 0;
    // Pointer to trigger failure callbacks
    std::function<void()> trigger_external_failure;
};

enum error_mode
{
    success,
    easy_init_error,
    global_init_error
};

struct MOCK_CURL_EXPORT curl_fail_init
{
    curl_fail_init(error_mode mode);
    ~curl_fail_init();
};
