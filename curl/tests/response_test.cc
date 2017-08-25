#include <catch.hpp>
#include "mock_curl.hpp"
#include <leatherman/curl/client.hpp>
#include <leatherman/curl/request.hpp>

using namespace std;
using namespace leatherman::curl;

struct mock_client : client {
    curl_handle const& get_handle() { return client::get_handle(); }
};

TEST_CASE("curl::response") {
    response test_response;

    SECTION("Headers should be addable and retrievable from the response") {
        test_response.add_header("header_name", "header_value");
        auto header = test_response.header("header_name");
        REQUIRE(header);
        REQUIRE(*(header) == "header_value");
    }

    SECTION("Headers should be removable from the response") {
        test_response.add_header("header_name", "header_value");
        test_response.remove_header("header_name");
        auto header = test_response.header("header_name");
        REQUIRE(header == nullptr);
    }

    SECTION("Headers should be enumerable") {
        int i = 0;
        string expected_name, expected_value;
        test_response.add_header("header_0", "header_value_0");
        test_response.add_header("header_1", "header_value_1");
        test_response.add_header("header_2", "header_value_2");

        test_response.each_header([&](string const& name, string const& value) {
            expected_name  = "header_" + to_string(i);
            expected_value = "header_value_" + to_string(i);
            REQUIRE(name == expected_name);
            REQUIRE(value == expected_value);
            ++i;
            return true;
        });
    }

    SECTION("Response body should be addable and retrievable") {
        test_response.body("Hello, I am a response body!");
        auto body = test_response.body();
        REQUIRE(body == "Hello, I am a response body!");
    }

    SECTION("Status code should be addable and retrievable") {
        test_response.status_code(200);
        auto code = test_response.status_code();
        REQUIRE(code == 200);
    }
}
