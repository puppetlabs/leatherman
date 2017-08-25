#include <catch.hpp>
#include "mock_curl.hpp"
#include <leatherman/curl/client.hpp>
#include <leatherman/curl/request.hpp>

using namespace std;
using namespace leatherman::curl;

struct mock_client : client {
    curl_handle const& get_handle() { return client::get_handle(); }
};

TEST_CASE("curl::request") {
    request test_request {"http://valid.com"};

    SECTION("Headers should be addable and retrievable from the request") {
        test_request.add_header("header_name", "header_value");
        auto header = test_request.header("header_name");
        REQUIRE(header);
        REQUIRE(*(header) == "header_value");
    }

    SECTION("Headers should be removable from the request") {
        test_request.add_header("header_name", "header_value");
        test_request.remove_header("header_name");
        auto header = test_request.header("header_name");
        REQUIRE(header == nullptr);
    }

    SECTION("Headers should be enumerable") {
        int i = 0;
        string expected_name, expected_value;
        test_request.add_header("header_0", "header_value_0");
        test_request.add_header("header_1", "header_value_1");
        test_request.add_header("header_2", "header_value_2");

        test_request.each_header([&](string const& name, string const& value) {
            expected_name  = "header_" + to_string(i);
            expected_value = "header_value_" + to_string(i);
            REQUIRE(name == expected_name);
            REQUIRE(value == expected_value);
            ++i;
            return true;
        });
    }

    SECTION("A cookie should be retrievable by name") {
        test_request.add_cookie("cookie_0", "cookie_val_0");
        test_request.add_cookie("cookie_1", "cookie_val_1");
        REQUIRE(*(test_request.cookie("cookie_0")) == "cookie_val_0");
    }

    SECTION("Cookies should be enumerable") {
        int i = 0;
        string expected_name, expected_value;
        test_request.add_cookie("cookie_0", "cookie_value_0");
        test_request.add_cookie("cookie_1", "cookie_value_1");
        test_request.add_cookie("cookie_2", "cookie_value_2");

        test_request.each_cookie([&](string const& name, string const& value) {
            expected_name  = "cookie_" + to_string(i);
            expected_value = "cookie_value_" + to_string(i);
            REQUIRE(name == expected_name);
            REQUIRE(value == expected_value);
            ++i;
            return true;
        });
    }

    SECTION("Request body should be addable and retrievable") {
        test_request.body("Hello, I am a request body!", "message");
        auto body = test_request.body();
        REQUIRE(body == "Hello, I am a request body!");
    }

    SECTION("Overall request timeout should be configurable and retrievable") {
        test_request.timeout(100);
        REQUIRE(test_request.timeout() == 100);
    }

    SECTION("Connection timeout should be configurable and retrievable") {
        test_request.connection_timeout(100);
        REQUIRE(test_request.connection_timeout() == 100);
    }
}
