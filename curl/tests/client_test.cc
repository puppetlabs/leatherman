#include <catch.hpp>
#include "mock_curl.hpp"
#include <leatherman/curl/client.hpp>
#include <leatherman/curl/request.hpp>
#include <leatherman/curl/response.hpp>

using namespace std;

namespace leatherman { namespace curl {

    struct mock_client : client {
        curl_handle const& get_handle() { return client::get_handle(); }
    };

    TEST_CASE("curl::client HTTP methods") {
        mock_client test_client;
        request test_request {"http://valid.com/"};

        SECTION("GET succeeds on a given URL") {
            auto resp = test_client.get(test_request);
            REQUIRE(resp.status_code() == 200);
        }

        SECTION("POST succeeds on a given URL") {
            auto resp = test_client.post(test_request);
            REQUIRE(resp.status_code() == 200);
        }

        SECTION("PUT succeeds on a given URL") {
            auto resp = test_client.put(test_request);
            REQUIRE(resp.status_code() == 200);
        }

        SECTION("Request returns status code 404 on invalid URL") {
            request invalid_test_request {"http://invalid.com/"};
            auto resp = test_client.get(invalid_test_request);
            REQUIRE(resp.status_code() == 404);
        }
    }

    TEST_CASE("curl::client HTTP request setup") {
        mock_client test_client;
        request test_request {"http://valid.com"};

        SECTION("HTTP method is set to GET given a GET request") {
            auto resp = test_client.get(test_request);
            CURL* const& handle = test_client.get_handle();
            auto test_impl = static_cast<curl_impl* const>(handle);
            REQUIRE(test_impl->method == curl_impl::http_method::get);
        }

        SECTION("HTTP method is set to POST given a POST request") {
            auto resp = test_client.post(test_request);
            CURL* const& handle = test_client.get_handle();
            auto test_impl = static_cast<curl_impl* const>(handle);
            REQUIRE(test_impl->method == curl_impl::http_method::post);
        }

        SECTION("HTTP method is set to PUT given a PUT request") {
            auto resp = test_client.put(test_request);
            CURL* const& handle = test_client.get_handle();
            auto test_impl = static_cast<curl_impl* const>(handle);
            REQUIRE(test_impl->method == curl_impl::http_method::put);
        }

        SECTION("cURL should receive the URL specified in the request") {
            auto resp = test_client.get(test_request);
            CURL* const& handle = test_client.get_handle();
            auto test_impl = static_cast<curl_impl* const>(handle);
            REQUIRE(test_impl->request_url == "http://valid.com");
        }
    }

    TEST_CASE("curl::client header and body writing and reading") {
        mock_client test_client;

        /*
         * Header writing and reading tests
         */
        SECTION("Custom request headers should be honored in the request to the server") {
            request test_request {"http://valid.com"};
            test_request.add_header("header_name", "header_value");
            auto resp = test_client.get(test_request);
            CURL* const& handle = test_client.get_handle();
            auto test_impl = static_cast<curl_impl* const>(handle);
            REQUIRE(test_impl->header);
            REQUIRE(test_impl->header->data == string("header_name: header_value"));
        }

        SECTION("The header response delimiter should be ignored") {
            request test_request {"http://response-delimiter.com/"};
            auto resp = test_client.get(test_request);
            CURL* const& handle = test_client.get_handle();
            auto test_impl = static_cast<curl_impl* const>(handle);
            int headers = 0;
            resp.each_header([&](string const& name, string const& value) {
                ++headers;
                return true;
            });
            REQUIRE(headers == 0);
        }

        SECTION("Non-standard response header should be parsed for name and value") {
            request test_request {"http://nonstd-header.com/"};
            auto resp = test_client.get(test_request);
            CURL* const& handle = test_client.get_handle();
            auto test_impl = static_cast<curl_impl* const>(handle);
            REQUIRE(resp.header("nonstd_header_name"));
            REQUIRE(*(resp.header("nonstd_header_name")) == "nonstd_header_value");
        }

        SECTION("Invalid headers should not be parsed or returned in the response") {
            request test_request {"http://invalid-header.com/"};
            auto resp = test_client.get(test_request);
            CURL* const& handle = test_client.get_handle();
            auto test_impl = static_cast<curl_impl* const>(handle);
            int headers = 0;
            resp.each_header([&](string const& name, string const& value) {
                ++headers;
                return true;
            });
            REQUIRE(headers == 0);
        }

        /*
         * Body writing and reading tests
         */
        SECTION("Request body should be settable and readable") {
            request test_request {"http://valid.com"};
            test_request.body("Hello, I am a request body!", "message");
            auto resp = test_client.get(test_request);
            CURL* const& handle = test_client.get_handle();
            auto test_impl = static_cast<curl_impl* const>(handle);
            REQUIRE(test_impl->read_buffer == "Hello, I am a request body!");
        }

        SECTION("Response body should be what is in the data part of the cURL response") {
            CURL* const& handle = test_client.get_handle();
            auto test_impl = static_cast<curl_impl* const>(handle);
            test_impl->resp_body = "Hello, I am a response body!";
            request test_request {"http://valid.com"};
            auto resp = test_client.get(test_request);
            REQUIRE(resp.body() == "Hello, I am a response body!");
        }
    }

    TEST_CASE("curl::client cookies") {
        mock_client test_client;
        request test_request {"http://valid.com"};

        SECTION("There should be no cookies in the request by default") {
            auto resp = test_client.get(test_request);
            CURL* const& handle = test_client.get_handle();
            auto test_impl = static_cast<curl_impl* const>(handle);
            REQUIRE(test_impl->cookie == "");
        }

        SECTION("Cookies should be present in the request when added") {
            test_request.add_cookie("cookie_name", "cookie_val");
            auto resp = test_client.get(test_request);
            CURL* const& handle = test_client.get_handle();
            auto test_impl = static_cast<curl_impl* const>(handle);
            REQUIRE(test_impl->cookie == "cookie_name=cookie_val");
        }

        SECTION("Cookies should be removable from the request") {
            test_request.add_cookie("cookie_0", "cookie_val_0");
            test_request.add_cookie("cookie_1", "cookie_val_1");
            test_request.remove_cookie("cookie_1");
            auto resp = test_client.get(test_request);
            CURL* const& handle = test_client.get_handle();
            auto test_impl = static_cast<curl_impl* const>(handle);
            REQUIRE(test_impl->cookie == "cookie_0=cookie_val_0");
        }

        SECTION("cURL should receieve cookies specified in the request") {
            test_request.add_cookie("cookie_0", "cookie_val_0");
            test_request.add_cookie("cookie_1", "cookie_val_1");
            auto resp = test_client.get(test_request);
            CURL* const& handle = test_client.get_handle();
            auto test_impl = static_cast<curl_impl* const>(handle);
            REQUIRE(test_impl->cookie == "cookie_0=cookie_val_0; cookie_1=cookie_val_1");
        }
    }

    TEST_CASE("curl::client CA bundle and SSL setup") {
        mock_client test_client;
        request test_request {"http://valid.com"};

        SECTION("Path to CA certificate should be unspecified by default") {
            auto resp = test_client.get(test_request);
            CURL* const& handle = test_client.get_handle();
            auto test_impl = static_cast<curl_impl* const>(handle);
            REQUIRE(test_impl->cacert == "");
        }

        SECTION("cURL should receive the path to the CA certificate specified in the request") {
            test_client.set_ca_cert("cacert");
            auto resp = test_client.get(test_request);
            CURL* const& handle = test_client.get_handle();
            auto test_impl = static_cast<curl_impl* const>(handle);
            REQUIRE(test_impl->cacert == "cacert");
        }

        SECTION("Client cert name should be unspecified by default") {
            auto resp = test_client.get(test_request);
            CURL* const& handle = test_client.get_handle();
            auto test_impl = static_cast<curl_impl* const>(handle);
            REQUIRE(test_impl->client_cert == "");
        }

        SECTION("cURL should receive the client cert name specified in the request") {
            test_client.set_client_cert("cert", "key");
            auto resp = test_client.get(test_request);
            CURL* const& handle = test_client.get_handle();
            auto test_impl = static_cast<curl_impl* const>(handle);
            REQUIRE(test_impl->client_cert == "cert");
        }

        SECTION("Private keyfile name should be unspecified by default") {
            auto resp = test_client.get(test_request);
            CURL* const& handle = test_client.get_handle();
            auto test_impl = static_cast<curl_impl* const>(handle);
            REQUIRE(test_impl->client_key == "");
        }

        SECTION("cURL should receive the private keyfile name specified in the request") {
            test_client.set_client_cert("cert", "key");
            auto resp = test_client.get(test_request);
            CURL* const& handle = test_client.get_handle();
            auto test_impl = static_cast<curl_impl* const>(handle);
            REQUIRE(test_impl->client_key == "key");
        }

        SECTION("cURL should make an HTTP request with the specified HTTP protocol") {
            test_client.set_supported_protocols(CURLPROTO_HTTP);
            auto resp = test_client.get(test_request);
            CURL* const& handle = test_client.get_handle();
            auto test_impl = static_cast<curl_impl* const>(handle);
            REQUIRE(test_impl->protocols == CURLPROTO_HTTP);
        }
    }

    TEST_CASE("curl::client errors") {
        mock_client test_client;
        request test_request {"http://valid.com/"};
        CURL* const& handle = test_client.get_handle();
        auto test_impl = static_cast<curl_impl* const>(handle);

        /*
         * Note: we do not currently test the case where cURL errors
         * on curl_global_init, as the global init is done as part of
         * static initialization in the cURL helper, and there is
         * currently no way to force it to be reinitialized.
         */

        SECTION("client fails to initialize a libcurl easy session") {
            curl_fail_init mock_error {easy_init_error};
            REQUIRE_THROWS_AS(mock_client test_client, http_exception);
        }

        SECTION("client fails to perform a cURL request") {
            test_impl->test_failure_mode = curl_impl::error_mode::easy_perform_error;
            REQUIRE_THROWS_AS(test_client.get(test_request), http_request_exception);
        }

        SECTION("client fails to set HTTP method to POST") {
            test_impl->test_failure_mode = curl_impl::error_mode::http_post_error;
            REQUIRE_THROWS_AS(test_client.post(test_request), http_request_exception);
        }

        SECTION("client fails to set HTTP method to PUT") {
            test_impl->test_failure_mode = curl_impl::error_mode::http_put_error;
            REQUIRE_THROWS_AS(test_client.put(test_request), http_request_exception);
        }

        SECTION("client fails to set the request URL") {
            test_impl->test_failure_mode = curl_impl::error_mode::set_url_error;
            REQUIRE_THROWS_AS(test_client.get(test_request), http_request_exception);
        }

        SECTION("client fails to set the request headers") {
            test_impl->test_failure_mode = curl_impl::error_mode::set_header_error;
            REQUIRE_THROWS_AS(test_client.get(test_request), http_request_exception);
        }

        SECTION("client fails to set cookies in the request") {
            test_impl->test_failure_mode = curl_impl::error_mode::set_cookie_error;
            REQUIRE_THROWS_AS(test_client.get(test_request), http_request_exception);
        }

        SECTION("client fails to set the header callback function") {
            test_impl->test_failure_mode = curl_impl::error_mode::header_function_error;
            REQUIRE_THROWS_AS(test_client.get(test_request), http_request_exception);
        }

        SECTION("client fails to set the header write location") {
            test_impl->test_failure_mode = curl_impl::error_mode::header_context_error;
            REQUIRE_THROWS_AS(test_client.get(test_request), http_request_exception);
        }

        SECTION("client fails to set the body writing callback function") {
            test_impl->test_failure_mode = curl_impl::error_mode::write_body_function_error;
            REQUIRE_THROWS_AS(test_client.get(test_request), http_request_exception);
        }

        SECTION("client fails to set the body write location") {
            test_impl->test_failure_mode = curl_impl::error_mode::write_body_context_error;
            REQUIRE_THROWS_AS(test_client.get(test_request), http_request_exception);
        }
        SECTION("client fails to set the read_body callback function") {
            test_impl->test_failure_mode = curl_impl::error_mode::read_body_function_error;
            REQUIRE_THROWS_AS(test_client.get(test_request), http_request_exception);
        }

        SECTION("client fails to set the read_body data source") {
            test_impl->test_failure_mode = curl_impl::error_mode::read_body_context_error;
            REQUIRE_THROWS_AS(test_client.get(test_request), http_request_exception);
        }

        SECTION("client fails to set the connection timeout") {
            test_impl->test_failure_mode = curl_impl::error_mode::connect_timeout_error;
            REQUIRE_THROWS_AS(test_client.get(test_request), http_request_exception);
        }

        SECTION("client fails to set the request timeout") {
            test_impl->test_failure_mode = curl_impl::error_mode::request_timeout_error;
            REQUIRE_THROWS_AS(test_client.get(test_request), http_request_exception);
        }

        SECTION("client fails to set certificate authority info") {
            test_client.set_ca_cert("certfile");
            test_impl->test_failure_mode = curl_impl::error_mode::ca_bundle_error;
            REQUIRE_THROWS_AS(test_client.get(test_request), http_request_exception);
        }

        SECTION("client fails to set SSL cert info") {
            test_client.set_client_cert("cert", "key");
            test_impl->test_failure_mode = curl_impl::error_mode::ssl_cert_error;
            REQUIRE_THROWS_AS(test_client.get(test_request), http_request_exception);
        }

        SECTION("client fails to set SSL key info") {
            test_client.set_client_cert("cert", "key");
            test_impl->test_failure_mode = curl_impl::error_mode::ssl_key_error;
            REQUIRE_THROWS_AS(test_client.get(test_request), http_request_exception);
        }

        SECTION("client fails to make http call with https protocol only enabled") {
            test_client.set_supported_protocols(CURLPROTO_HTTPS);
            test_impl->test_failure_mode = curl_impl::error_mode::protocol_error;
            REQUIRE_THROWS_AS(test_client.get(test_request), http_request_exception);
        }
    }
}}
