#define BUILDING_LIBCURL
#include <cstring>
#include <stdarg.h>
#include <stdio.h>
#include <array>
#include <algorithm>
#include "mock_curl.hpp"

using namespace std;

/*
 * Global error_mode struct implementation. This is needed to test
 * cURL global and easy initialization, before the client object exists.
 */
static error_mode test_failure_mode = success;


curl_fail_init::curl_fail_init(error_mode mode)
{
   test_failure_mode = mode;
}

curl_fail_init::~curl_fail_init()
{
    test_failure_mode = success;
}

/*
 * libcurl implementations below. We are mocking necessary methods
 * of the CURL API to ensure that our wrapper is making the calls
 * to libcurl that we expect.
 */

/*
 * Sets up the program environment that libcurl needs. The mock
 * implementation simply returns successfully, unless we are
 * specifically testing global initialization failure.
 */
CURLcode curl_global_init(long flags)
{
    if (test_failure_mode == global_init_error) {
        return CURLE_FAILED_INIT;
    } else {
        return CURLE_OK;
    }
}

/*
 * Reclaim memory obtained from a libcurl call by deleting
 * a mock curl object.
 */
void curl_free(void *p)
{
    delete reinterpret_cast<curl_impl*>(p);
}

/*
 * End a libcurl easy handle. The mock implementation simply
 * calls curl_free to delete the mock curl object argument.
 */
void curl_easy_cleanup(CURL * handle)
{
    curl_free(handle);
}

/*
 * Mock implementation of curl_easy_escape which simply returns a
 * nullptr. URL encoding the given string is not necessary for
 * testing.
 */
char *curl_easy_escape(CURL * curl, const char * string, int length)
{
    return nullptr;
}

/*
 * Start a libcurl easy session. The mock implementation simply returns
 * a new mock curl object, unless we are specifcally testing easy_init
 * exception handling, in which case we return nullptr.
 */
CURL *curl_easy_init()
{
    if (test_failure_mode == easy_init_error) {
        return nullptr;
    } else {
        return reinterpret_cast<CURL*>(new curl_impl());
    }
}

/*
 * Set options for an easy curl handle. We use this method in the mock
 * implementation to ensure that the correct CURL API calls are being
 * made. Given a particular CURL option, we store received data from
 * the varargs parameter in the mock curl object. The data is
 * then verified in the tests.
 *
 * Each option has the potential to fail while being set, which is
 * covered by a suite of exception tests. If the given test error
 * option is set, we'll return CURLE_COULDNT_CONNECT for the current
 * option.
 */
#pragma clang diagnostic push
// This function signature is required to mock curl, so disable a warning generated from it.
#pragma clang diagnostic ignored "-Wvarargs"
CURLcode curl_easy_setopt(CURL *handle, CURLoption option, ...)
{
    auto h = reinterpret_cast<curl_impl*>(handle);
    va_list vl;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wvarargs"
    va_start(vl, option);
#pragma clang diagnostic pop

    switch (option) {
        case CURLOPT_HEADERFUNCTION:
            // Set client::write_header as the function to be called as soon as mock curl has received header data.
            if (h->test_failure_mode == curl_impl::error_mode::header_function_error) {
                va_end(vl);
                return CURLE_UNKNOWN_OPTION;
            }
            h->write_header = va_arg(vl, size_t (*)(char*, size_t, size_t, void*));
            break;
        case CURLOPT_HEADERDATA:
            // Pointer to the context to write the header part of the received data to.
            if (h->test_failure_mode == curl_impl::error_mode::header_context_error) {
                va_end(vl);
                return CURLE_COULDNT_CONNECT;
            }
            h->header_context = va_arg(vl, void*);
            break;
        case CURLOPT_WRITEFUNCTION:
            // Set client::write_body as the function to be called as soon as mock curl has received data.
            if (h->test_failure_mode == curl_impl::error_mode::write_body_function_error) {
                va_end(vl);
                return CURLE_COULDNT_CONNECT;
            }
            h->write_body = va_arg(vl, size_t (*)(char*, size_t, size_t, void*));
            break;
        case CURLOPT_WRITEDATA:
            // Pointer to the context to write the body part of the received data to.
            if (h->test_failure_mode == curl_impl::error_mode::write_body_context_error) {
                va_end(vl);
                return CURLE_COULDNT_CONNECT;
            }
            h->body_context = va_arg(vl, void*);
            break;
        case CURLOPT_READFUNCTION:
            // Set client::read_body as the function to be called for mock curl to read the request body.
            if (h->test_failure_mode == curl_impl::error_mode::read_body_function_error) {
                va_end(vl);
                return CURLE_COULDNT_CONNECT;
            }
            h->read_function = va_arg(vl, size_t (*)(char*, size_t, size_t, void*));
            break;
        case CURLOPT_READDATA:
            // Pointer to the context to read the request body from by the READFUNCTION callback.
            if (h->test_failure_mode == curl_impl::error_mode::read_body_context_error) {
                va_end(vl);
                return CURLE_COULDNT_CONNECT;
            }
            h->read_data = va_arg(vl, void*);
            break;
        case CURLOPT_URL:
            // Set the mock curl URL as the URL specified in the request.
            if (h->test_failure_mode == curl_impl::error_mode::set_url_error) {
                va_end(vl);
                return CURLE_OUT_OF_MEMORY;
            }
            h->request_url = va_arg(vl, char*);
            break;
        case CURLOPT_POST:
            // Set the mock curl HTTP method as POST
            if (h->test_failure_mode == curl_impl::error_mode::http_post_error) {
                va_end(vl);
                return CURLE_COULDNT_CONNECT;
            }
            h->method = curl_impl::http_method::post;
            break;
        case CURLOPT_UPLOAD:
        case CURLOPT_PUT:
            // Set the mock curl HTTP method as PUT
            if (h->test_failure_mode == curl_impl::error_mode::http_put_error) {
                va_end(vl);
                return CURLE_COULDNT_CONNECT;
            }
            h->method = curl_impl::http_method::put;
            break;
        case CURLOPT_HTTPHEADER:
            // Set the mock curl list of custom headers to that which was passed in the request.
            if (h->test_failure_mode == curl_impl::error_mode::set_header_error) {
                va_end(vl);
                return CURLE_COULDNT_CONNECT;
            }
            h->header = va_arg(vl, curl_slist*);
            break;
        case CURLOPT_COOKIE:
            // Set the mock curl Cookie header to that which was passed in the request.
            if (h->test_failure_mode == curl_impl::error_mode::set_cookie_error) {
                va_end(vl);
                return CURLE_OUT_OF_MEMORY;
            }
            h->cookie = va_arg(vl, char*);
            break;
        case CURLOPT_CAINFO:
            // Set the mock curl Certificate Authority path to that which was passed in the request.
            if (h->test_failure_mode == curl_impl::error_mode::ca_bundle_error) {
                va_end(vl);
                return CURLE_OUT_OF_MEMORY;
            }
            h->cacert = va_arg(vl, char*);
            break;
        case CURLOPT_SSLCERT:
            // Set the mock curl SSL client cert name to that which was passed in the request.
            if (h->test_failure_mode == curl_impl::error_mode::ssl_cert_error) {
                va_end(vl);
                return CURLE_OUT_OF_MEMORY;
            }
            h->client_cert = va_arg(vl, char*);
            break;
        case CURLOPT_SSLKEY:
            // Set the mock curl private keyfile name to that which was passed in the request.
            if (h->test_failure_mode == curl_impl::error_mode::ssl_key_error) {
                va_end(vl);
                return CURLE_OUT_OF_MEMORY;
            }
            h->client_key = va_arg(vl, char*);
            break;
        case CURLOPT_CONNECTTIMEOUT_MS:
            if (h->test_failure_mode == curl_impl::error_mode::connect_timeout_error) {
                va_end(vl);
                return CURLE_COULDNT_CONNECT;
            }
            h->connect_timeout = va_arg(vl, long);
            break;
        case CURLOPT_TIMEOUT_MS:
            if (h->test_failure_mode == curl_impl::error_mode::request_timeout_error) {
                va_end(vl);
                return CURLE_COULDNT_CONNECT;
            }
            break;
        case CURLOPT_PROTOCOLS:
            if (h->test_failure_mode == curl_impl::error_mode::protocol_error) {
                va_end(vl);
                return CURLE_COULDNT_CONNECT;
            }
            h->protocols = va_arg(vl, long);
            break;
        case CURLOPT_ERRORBUFFER:
            h->errbuf = va_arg(vl, char*); 
            break;
        default:
            break;
    }
    va_end(vl);
    return CURLE_OK;
}

/*
 * Perform a cURL transfer. the mock implementation uses this method to
 * write the response header and body.
 */
CURLcode curl_easy_perform(CURL * easy_handle)
{
    auto h = reinterpret_cast<curl_impl*>(easy_handle);
    if (h->test_failure_mode == curl_impl::error_mode::easy_perform_write_error) {
        return CURLE_WRITE_ERROR;
    }
    if (h->test_failure_mode == curl_impl::error_mode::easy_perform_error) {
        if (h->errbuf) {
            strcpy(h->errbuf, "easy perform failed"); 
        }
        return CURLE_COULDNT_CONNECT;
    }

    /*
     * Fill the read buffer in multiple chunks to better simulate real libcurl.
     */
    if (h->read_function) {
        size_t bytes_returned;
        char buf[10] = {};

        while ((bytes_returned = h->read_function(buf, 1, 10, h->read_data))) {
            h->read_buffer.append(buf, bytes_returned);
        }
    }

    static const array<string, 3> VALID_URLS{{
      "http://valid.com/",
      "https://download.com",
      "https://remove_temp_file.com"
    }};

    /*
     * If we pass 'valid.com' in the test, return HTTP status 200. Otherwise, return status 404.
     */
    if (h->write_header) {
        bool is_valid_url = find(VALID_URLS.begin(), VALID_URLS.end(), h->request_url) != VALID_URLS.end();
        if (is_valid_url) {
            string header_content = "HTTP/1.1 200 OK\n"
                                    "Connection: keep-alive\n"
                                    "Date: Thu, 16 Jul 2015 18:41:08 GMT\n"
                                    "Content-Type: text/html;charset=UTF-8\n"
                                    "Server: Jetty(7.x.y-SNAPSHOT)\n"
                                    "Via: 1.1 vegur";

            h->write_header(&header_content[0], 1, header_content.size(), h->header_context);
        } else if (h->request_url == "http://nonstd-header.com/") {
            string header_content = "nonstd_header_name:nonstd_header_value";
            h->write_header(&header_content[0], 1, header_content.size(), h->header_context);
        } else if (h->request_url == "http://response-delimiter.com/") {
            string header_content = "\r\n";
            h->write_header(&header_content[0], 1, header_content.size(), h->header_context);
        } else if (h->request_url == "http://invalid-header.com/") {
            string header_content = "This is an invalid header";
            h->write_header(&header_content[0], 1, header_content.size(), h->header_context);
        } else {
            string header_content = "HTTP/1.1 404 NOT FOUND\n"
                                    "Connection: keep-alive\n"
                                    "Date: Thu, 16 Jul 2015 18:41:08 GMT\n"
                                    "Content-Type: text/html;charset=UTF-8\n"
                                    "Server: Jetty(7.x.y-SNAPSHOT)\n"
                                    "Via: 1.1 vegur";

            h->write_header(&header_content[0], 1, header_content.size(), h->header_context);
        }
    }

    /*
     * For file download. It is OK if exception is thrown if write_body is not set,
     * that means something went wrong in our code's setup so we want our test to
     * fail.
     */
    if (h->request_url == "https://download.com" || h->request_url == "https://download_trigger_404.com") {
        string download_msg = (h->request_url == "https://download.com") ? "successfully downloaded file" : "Not found";
        h->write_body(const_cast<char*>(download_msg.c_str()), 1, reinterpret_cast<size_t>(download_msg.size()), h->body_context);
        if (h->trigger_external_failure) {
           #ifdef _WIN32
           fclose(reinterpret_cast<FILE*>(h->body_context));
           #endif
           h->trigger_external_failure();
        }
        return CURLE_OK;
    }

    /*
     * We set resp_body internally in write_body tests.
     */
    if (h->write_body) {
        h->write_body(&h->resp_body[0], 1, h->resp_body.size(), h->body_context);
    }

    return CURLE_OK;
}

/*
 * Unimplemented, as resetting options is not necessary for testing.
 */
void curl_easy_reset(CURL *handle)
{
}

/*
 * We throw CURLE_FAILED_INIT to test cURL handle initialization,
 * and CURLE_COULDNT_CONNECT for all other possible errors.
 */
const char *curl_easy_strerror(CURLcode errornum)
{
    switch (errornum) {
        case CURLE_OK:
            break;
        case CURLE_FAILED_INIT:
            return "cURL failed with: CURLE_FAILED_INIT";
        case CURLE_COULDNT_CONNECT:
            return "cURL failed with: CURLE_COULDNT_CONNECT";
        case CURLE_OUT_OF_MEMORY:
            return "cURL failed with: CURLE_OUT_OF_MEMORY";
        case CURLE_UNKNOWN_OPTION:
            return "cURL failed with CURLE_UNKNOWN_OPTION";
        default:
            return nullptr;
    }
    return nullptr;
}

/*
 * Unimplemented, as we don't allocate many objects to clean up
 * in tests.
 */
void curl_global_cleanup(void)
{
}

/*
 * Add a string to an slist. If list already includes curl_slist
 * objects, we must traverse the linked list to append the new
 * object at the end. Otherwise, create a new curl_slist linked
 * list.
 */
struct curl_slist *curl_slist_append(struct curl_slist * list, const char * string )
{
    curl_slist* new_slist_obj = new curl_slist();
    new_slist_obj->data = new char[strlen(string) + 1];
    new_slist_obj->data = strcpy(new_slist_obj->data, string);
    if (list) {
        curl_slist* ptr = list;
        while(ptr->next) {
            ptr = ptr->next;
        }
        ptr->next = new_slist_obj;
        return list;
    } else {
        return new_slist_obj;
    }
}

/*
 * Unimplemented, as we allocate very little memory for curl_slist
 * objects in tests. This may have to change if we decide to run
 * memory checks on unit tests.
 */
void curl_slist_free_all(struct curl_slist * list)
{
}
