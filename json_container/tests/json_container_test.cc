#include <catch.hpp>
#include <leatherman/json_container/json_container.hpp>
#include <iostream>

static const std::string JSON = "{\"foo\" : {\"bar\" : 2},"
                                " \"goo\" : 1,"
                                " \"bool\" : true,"
                                " \"string\" : \"a string\","
                                " \"null\" : null,"
                                " \"real\" : 3.1415,"
                                " \"vec\" : [1, 2], "
                                " \"nested\" : {"
                                "                  \"foo\" : \"bar\""
                                "               }"
                                "}";

namespace leatherman { namespace json_container {

auto ctor = [](std::string& json_txt) { JsonContainer d { json_txt }; };  // NOLINT

TEST_CASE("JsonContainer::JsonContainer - passing JSON string", "[data]") {
    std::string json_value {};

    SECTION("it should instantiate by passing any JSON value") {
        SECTION("object") {
            json_value = JSON;
        }

        SECTION("array") {
            json_value = "[1, 2, 3]";
        }

        SECTION("string") {
            json_value = "\"foo\"";
        }

        SECTION("number - int") {
            json_value = "42";
        }

        SECTION("number - float") {
            json_value = "3.14159";
        }

        SECTION("boolean - true") {
            json_value = "true";
        }

        SECTION("boolean - false") {
            json_value = "false";
        }

        SECTION("null") {
            json_value = "null";
        }

        REQUIRE_NOTHROW(ctor(json_value));
    }

    SECTION("it should throw a data_parse_error in case of invalid JSON") {
        SECTION("bad object") {
            json_value = "{\"foo\" : \"bar\", 42}";
        }

        SECTION("bad key") {
            json_value = "{42 : \"bar\"}";
        }

        SECTION("bad array") {
            json_value = "1, 2, 3";
        }

        REQUIRE_THROWS_AS(ctor(json_value), data_parse_error);
    }
}

TEST_CASE("JsonContainer::get", "[data]") {
    JsonContainer msg { JSON };

    SECTION("it can get a root value") {
        REQUIRE(msg.get<int>("goo") == 1);
    }

    SECTION("it can get a nested value") {
        REQUIRE(msg.get<int>({"foo", "bar"}) == 2);
    }

    SECTION("it can get a bool value") {
        REQUIRE(msg.get<bool>("bool") == true);
    }

    SECTION("it can get a string value") {
        REQUIRE(msg.get<std::string>("string") == "a string");
    }

    SECTION("it can get a double value") {
        REQUIRE(msg.get<double>("real") == 3.1415);
    }

    SECTION("it can get a vector") {
        std::vector<int> tmp { 1, 2 };
        std::vector<int> result { msg.get<std::vector<int>>("vec") };
        REQUIRE(tmp[0] == result[0]);
        REQUIRE(tmp[1] == result[1]);
    }

    SECTION("it can get the root object") {
      REQUIRE(msg.get<JsonContainer>().get<int>("goo") == 1);
    }

    SECTION("it should behave correctly given a null value") {
        REQUIRE(msg.get<std::string>("null") == "");
        REQUIRE(msg.get<int>("null") == 0);
        REQUIRE(msg.get<bool>("null") == false);
    }

    SECTION("it returns a null like value when indexing something that "
            "doesn't exist") {
        REQUIRE(msg.get<std::string>("invalid") == "");
        REQUIRE(msg.get<int>({ "goo", "1" }) == 0);
        REQUIRE(msg.get<bool>({ "foo", "baz" }) == false);
    }

    SECTION("it can get the root") {
        SECTION("array") {
            JsonContainer data_array { "[1, 2, 3]" };
            auto array = data_array.get<std::vector<int>>();
            std::vector<int> expected_array { 1, 2, 3 };

            REQUIRE(array == expected_array);
        }

        SECTION("object") {
            auto object = msg.get<JsonContainer>();

            REQUIRE(object.get<int>("goo") == 1);
        }

        SECTION("number") {
            JsonContainer data_number { "42" };
            auto number = data_number.get<int>();

            REQUIRE(number == 42);
        }
    }
}

TEST_CASE("JsonContainer::toPrettyString", "[data]") {
    SECTION("does not throw when the root is") {
        SECTION("a string") {
            JsonContainer data_s { "\"some text\"" };
            REQUIRE_NOTHROW(data_s.toPrettyString());
        }

        SECTION("an array") {
            JsonContainer data_a { "[1, 2, 3]" };
            REQUIRE_NOTHROW(data_a.toPrettyString());
        }

        SECTION("an object") {
            JsonContainer data_o { JSON };
            REQUIRE_NOTHROW(data_o.toPrettyString());
        }

        SECTION("an object containing nested objects with an array") {
            JsonContainer data_ooa {};
            JsonContainer tmp {};
            tmp.set<std::vector<int>>("bar", { 1, 2, 3 });
            JsonContainer tmp_two {};
            tmp_two.set<JsonContainer>("spam", tmp);
            tmp_two.set<std::vector<int>>("beans", { 55, 56, 57 });
            JsonContainer tmp_three {};
            tmp_three.set<JsonContainer>("eggs", tmp_two);
            data_ooa.set<JsonContainer>("foo", tmp_three);

            REQUIRE_NOTHROW(data_ooa.toPrettyString());
        }
    }
}

TEST_CASE("JsonContainer::empty", "[data]") {
    SECTION("works correctly for an empty JsonContainer instance") {
        JsonContainer data {};
        REQUIRE(data.empty());
    }

    SECTION("works correctly if the root is an empty array") {
        JsonContainer data {  "[]" };
        REQUIRE(data.empty());
    }

    SECTION("works correctly for an non-empty JsonContainer instance") {
        JsonContainer data {};
        data.set<int>("spam", 1);
        REQUIRE_FALSE(data.empty());
    }

    SECTION("works correctly if the root is an non-empty array") {
        JsonContainer data {  "[1, 2, 3]" };
        REQUIRE_FALSE(data.empty());
    }
}

TEST_CASE("JsonContainer::includes", "[data]") {
    SECTION("does not throw for an empty JsonContainer instance") {
        JsonContainer data {};
        REQUIRE_FALSE(data.includes("foo"));
    }

    SECTION("it should not throw if the root is not a JSON object") {
        JsonContainer data { "[1, 2, 3]" };
        REQUIRE_FALSE(data.includes("foo"));
    }

    SECTION("Document/object lookups") {
        JsonContainer msg { JSON };
        REQUIRE(msg.includes("foo") == true);
        REQUIRE(msg.includes({ "foo", "bar" }) == true);
        REQUIRE(msg.includes({ "foo", "baz" }) == false);
    }

    SECTION("Non object/document lookups") {
        JsonContainer msg { "\"foo\"" };
        REQUIRE(msg.includes({ "bar", "bar" }) == false);
        REQUIRE(msg.includes("foo") == false);
    }
}

TEST_CASE("JsonContainer::set", "[data]") {
    JsonContainer msg {};

    SECTION("it should add a new pair to the root") {
        msg.set<int>("foo", 4);
        REQUIRE(msg.get<int>("foo") == 4);
    }

    SECTION("it allows the creation of a nested structure") {
        msg.set<int>({"level1", "level21"}, 0);
        msg.set<bool>("bool1", true);
        msg.set<std::string>({"level1", "level22"}, "a string");
        msg.set<std::string>("level11", "different string");
        REQUIRE(msg.get<int>({ "level1", "level21" }) == 0);
        REQUIRE(msg.get<bool>("bool1") == true);
        REQUIRE(msg.get<std::string>({"level1", "level22"}) == "a string");
        REQUIRE(msg.get<std::string>("level11") == "different string");
    }

    SECTION("it allows resetting an integer value") {
        msg.set<int>("i entry", 0);
        REQUIRE(msg.includes("i entry"));
        REQUIRE(msg.get<int>("i entry") == 0);

        msg.set<int>("i entry", 5);
        REQUIRE(msg.get<int>("i entry") == 5);
    }

    SECTION("it allows resetting a double value") {
        msg.set<double>("d entry", 3.14159);
        REQUIRE(msg.includes("d entry"));
        REQUIRE(msg.get<double>("d entry") == 3.14159);

        msg.set<double>("d entry", 2.71828);
        REQUIRE(msg.get<double>("d entry") == 2.71828);
    }

    SECTION("it allows resetting a boolean value") {
        msg.set<bool>("b entry", true);
        REQUIRE(msg.includes("b entry"));
        REQUIRE(msg.get<bool>("b entry") == true);

        msg.set<bool>("b entry", false);
        REQUIRE(msg.get<bool>("b entry") == false);
    }

    SECTION("it allows resetting a string value") {
        msg.set<std::string>("s entry", "bar");
        REQUIRE(msg.includes("s entry"));
        REQUIRE(msg.get<std::string>("s entry") == "bar");

        msg.set<std::string>("s entry", "spam");
        REQUIRE(msg.get<std::string>("s entry") == "spam");
    }

    SECTION("it allows resetting a string vector value") {
        std::vector<std::string> s_v { "foo", "bar" };
        msg.set<std::vector<std::string>>("s_v entry", s_v);
        REQUIRE(msg.includes("s_v entry"));
        REQUIRE(msg.get<std::vector<std::string>>("s_v entry") == s_v);

        std::vector<std::string> s_v_other { "spam", "eggs" };
        msg.set<std::vector<std::string>>("s_v entry", s_v_other);
        REQUIRE(msg.get<std::vector<std::string>>("s_v entry") == s_v_other);
    }

    SECTION("it allows resetting a JsonContainer value") {
        JsonContainer d {};
        d.set<int>("i", 1);
        msg.set<JsonContainer>("d_c entry", d);
        auto i_entry = msg.get<JsonContainer>("d_c entry");
        REQUIRE(msg.includes("d_c entry"));
        REQUIRE(i_entry.get<int>("i") == 1);

        JsonContainer d_other {};
        d_other.set<bool>("b", false);
        auto b_entry = msg.get<JsonContainer>("d_c entry");
        REQUIRE_FALSE(b_entry.get<bool>("b"));
    }

    SECTION("it can set a key to a vector") {
        std::vector<std::string> strings { "foo", "bar" };
        msg.set<std::vector<std::string>>("sv", strings);

        std::vector<int> ints { 4, 2 };
        msg.set<std::vector<int>>("iv", ints);

        std::vector<bool> bools { true, false };
        msg.set<std::vector<bool>>("bv", bools);

        std::vector<double> doubles { 0.00, 9.99 };
        msg.set<std::vector<double>>("dv", doubles);

        REQUIRE(msg.get<std::vector<std::string>>("sv")[0] == "foo");
        REQUIRE(msg.get<std::vector<std::string>>("sv")[1] == "bar");

        REQUIRE(msg.get<std::vector<int>>("iv")[0] == 4);
        REQUIRE(msg.get<std::vector<int>>("iv")[1] == 2);

        REQUIRE(msg.get<std::vector<bool>>("bv")[0] == true);
        REQUIRE(msg.get<std::vector<bool>>("bv")[1] == false);

        REQUIRE(msg.get<std::vector<double>>("dv")[0] == 0.00);
        REQUIRE(msg.get<std::vector<double>>("dv")[1] == 9.99);
    }

    SECTION("it should throw a data_key_error in case root is not an object") {
        std::string json_array { "[1, 2, 3]" };
        JsonContainer data_array { json_array };

        REQUIRE_THROWS_AS(data_array.set<std::string>("foo", "bar"),
                          data_key_error);
    }

    SECTION("it should throw a data_key_error in case a known inner key is not "
            "associated with a JSON object") {
        JsonContainer d_c { JSON };

        REQUIRE_THROWS_AS(d_c.set<std::string>({ "vec", "foo" }, "bar"),
                          data_key_error);
    }
}

TEST_CASE("JsonContainer::keys", "[data]") {
    SECTION("It returns a vector of keys") {
        JsonContainer data { "{ \"a\" : 1, "
                              " \"b\" : 2}"};
        REQUIRE(data.keys().size() == 2);
    }

    SECTION("It returns an empty vector when the JsonContainer is empty") {
        JsonContainer data {};
        REQUIRE(data.keys().size() == 0);
    }
}

TEST_CASE("JsonContainer::type", "[data]") {
    JsonContainer data {};

    SECTION("When no key is passed it retrieves the type of the root value") {
        SECTION("array") {
            JsonContainer data_array { "[1, 2, 3]" };
            REQUIRE(data_array.type() == DataType::Array);
        }

        SECTION("object") {
            data.set<bool>("b_entry", false);
            REQUIRE(data.type() == DataType::Object);
        }

        SECTION("number") {
            JsonContainer data_number { "42" };
            REQUIRE(data_number.type() == DataType::Int);
        }
    }

    SECTION("When a single key is passed") {
        SECTION("it throws a data_key_error if the key is unknown") {
            REQUIRE_THROWS_AS(data.type("foo"),
                              data_key_error);
        }

        SECTION("it can distinguish a Bool (false) value") {
            data.set<bool>("b_entry", false);
            REQUIRE(data.type("b_entry") == DataType::Bool);
        }

        SECTION("it can distinguish a Bool (true) value") {
            data.set<bool>("b_entry", true);
            REQUIRE(data.type("b_entry") == DataType::Bool);
        }

        SECTION("it can distinguish an Object (JsonContainer) value") {
            JsonContainer tmp {};
            tmp.set<std::string>("eggs", "spam");
            data.set<JsonContainer>("obj_entry", tmp);
            REQUIRE(data.type("obj_entry") == DataType::Object);
        }

        SECTION("it can distinguish an Array value") {
            std::vector<std::string> tmp { "one", "two", "three" };
            data.set<std::vector<std::string>>("array_entry", tmp);
            REQUIRE(data.type("array_entry") == DataType::Array);
        }

        SECTION("it can distinguish a String value") {
            data.set<std::string>("eggs", "spam");
            REQUIRE(data.type("eggs") == DataType::String);
        }

        SECTION("it can distinguish an Int value") {
            data.set<int>("int_entry", 42);
            REQUIRE(data.type("int_entry") == DataType::Int);
        }

        SECTION("it can distinguish a Double value") {
            data.set<double>("d_entry", 2.71828);
            REQUIRE(data.type("d_entry") == DataType::Double);
        }

        SECTION("it can distinguish a null value") {
            JsonContainer data_with_null { "{\"the_null\" : null}" };
            REQUIRE(data_with_null.type("the_null") == DataType::Null);
        }
    }

    SECTION("When multiple keys are passed") {
        JsonContainer tmp {};
        data.set<JsonContainer>("stuff", tmp);

        SECTION("it throws a data_key_error if a key is unknown") {
            REQUIRE_THROWS_AS(data.type({ "stuff", "bar" }),
                              data_key_error);
        }

        SECTION("it can distinguish a Bool (false) value") {
            data.set<bool>({ "stuff", "b_entry" }, false);
            REQUIRE(data.type({ "stuff", "b_entry" }) == DataType::Bool);
        }

        SECTION("it can distinguish a Bool (true) value") {
            data.set<bool>({ "stuff", "b_entry" }, true);
            REQUIRE(data.type({ "stuff", "b_entry" }) == DataType::Bool);
        }

        SECTION("it can distinguish an Object (JsonContainer) value") {
            JsonContainer tmp {};
            tmp.set<std::string>("eggs", "spam");
            data.set<JsonContainer>({ "stuff", "obj_entry" }, tmp);
            REQUIRE(data.type({ "stuff", "obj_entry" }) == DataType::Object);
        }

        SECTION("it can distinguish an Array value") {
            std::vector<std::string> tmp { "one", "two", "three" };
            data.set<std::vector<std::string>>({ "stuff", "array_entry" }, tmp);
            REQUIRE(data.type({ "stuff", "array_entry" }) == DataType::Array);
        }

        SECTION("it can distinguish a String value") {
            data.set<std::string>({ "stuff", "eggs" }, "spam");
            REQUIRE(data.type({ "stuff", "eggs" }) == DataType::String);
        }

        SECTION("it can distinguish an Int value") {
            data.set<int>({ "stuff", "int_entry" }, 42);
            REQUIRE(data.type({ "stuff", "int_entry" }) == DataType::Int);
        }

        SECTION("it can distinguish a Double value") {
            data.set<double>({ "stuff", "d_entry" }, 2.71828);
            REQUIRE(data.type({ "stuff", "d_entry" }) == DataType::Double);
        }

        SECTION("it can distinguish a null value") {
            JsonContainer data_with_null { "{\"the_null\" : null}" };
            data.set<JsonContainer>({ "stuff", "more_stuff" }, data_with_null);
            auto data_type = data.type({ "stuff", "more_stuff", "the_null" });
            REQUIRE(data_type == DataType::Null);
        }
    }
}

}}  // namespace leatherman::json_container
