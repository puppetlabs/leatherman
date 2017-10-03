#include <catch.hpp>
#include <leatherman/json_container/json_container.hpp>
#include <iostream>

static const std::string JSON = "{\"foo\" : {\"bar\" : 2},"
                                " \"goo\" : 1,"
                                " \"bool\" : true,"
                                " \"string\" : \"a string\","
                                " \"string_with_null\" : \"a string\\u0000with\\u0000null\","
                                " \"null\" : null,"
                                " \"real\" : 3.1415,"
                                " \"vec\" : [1, 2], "
                                " \"string_vec\" : [\"one\", \"two\\u0000null\"], "
                                " \"nested\" : {"
                                "                  \"foo\" : \"bar\""
                                "               }"
                                "}";

using namespace leatherman::json_container;

TEST_CASE("JsonContainer::JsonContainer - passing JSON string", "[data]") {
    std::string json_value {};

    SECTION("it should instantiate by passing any JSON value") {
        SECTION("object") {
            json_value = JSON;
        }

        SECTION("array") {
            SECTION("of numbers") {
                json_value = "[1, 2, 3]";
            }

            SECTION("of booleans") {
                json_value = "[true, true]";
            }

            SECTION("of strings") {
                json_value = "[\"spam\", \"eggs\", \"foo\"]";
            }

            SECTION("of objects") {
                json_value = "[" + JSON + ",\n" + JSON + "]";
            }

            SECTION("of arrays") {
                json_value = "[[1, 2, 3], [\"spam\", \"eggs\", \"foo\"]]";
            }

            SECTION("of values of different types") {
                json_value = "[1, \"spam\",\n" + JSON + "]";
            }
        }

        SECTION("std::string instance containing an empty JSON string") {
            json_value = "\"\"";
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

        REQUIRE_NOTHROW(JsonContainer { json_value });
    }

    SECTION("it should throw a data_parse_error in case of empty string") {
        json_value = "";
        REQUIRE_THROWS_AS(JsonContainer { json_value }, data_parse_error);
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

        REQUIRE_THROWS_AS(JsonContainer { json_value }, data_parse_error);
    }
}

TEST_CASE("JsonContainer::get for object entries", "[data]") {
    JsonContainer data { JSON };

    SECTION("it can get a root value") {
        REQUIRE(data.get<int>("goo") == 1);
    }

    SECTION("it can get a nested value") {
        REQUIRE(data.get<int>({"foo", "bar"}) == 2);
    }

    SECTION("it can get a bool value") {
        REQUIRE(data.get<bool>("bool") == true);
    }

    SECTION("it can get a string value") {
        REQUIRE(data.get<std::string>("string") == "a string");
    }

    SECTION("it can get a string value containing null character(s)") {
        REQUIRE(data.get<std::string>("string_with_null") == std::string("a string\0with\0null", 18));
    }

    SECTION("it can get a double value") {
        REQUIRE(data.get<double>("real") == 3.1415);
    }

    SECTION("it can get a vector") {
        std::vector<int> tmp { 1, 2 };
        std::vector<int> result { data.get<std::vector<int>>("vec") };
        REQUIRE(tmp.size() == result.size());
        REQUIRE(tmp[0] == result[0]);
        REQUIRE(tmp[1] == result[1]);
    }

    SECTION("it can get a string vector") {
        std::vector<std::string> tmp { "one", { "two\0null", 8 } };
        std::vector<std::string> result { data.get<std::vector<std::string>>("string_vec") };
        REQUIRE(tmp.size() == result.size());
        REQUIRE(tmp[0] == result[0]);
        REQUIRE(tmp[1] == result[1]);
    }

    SECTION("it can get the root object") {
        REQUIRE(data.get<JsonContainer>().get<int>("goo") == 1);
    }

    SECTION("it should behave correctly given a null value") {
        REQUIRE(data.get<std::string>("null") == "");
        REQUIRE(data.get<int>("null") == 0);
        REQUIRE(data.get<bool>("null") == false);
    }

    SECTION("it can get the root entry") {
        SECTION("array of numbers") {
            JsonContainer data_array { "[1, 2, 3]" };
            auto array = data_array.get<std::vector<int>>();
            std::vector<int> expected_array { 1, 2, 3 };

            REQUIRE(array == expected_array);
        }

        SECTION("object") {
            auto object = data.get<JsonContainer>();

            REQUIRE(object.get<int>("goo") == 1);
        }

        SECTION("number") {
            JsonContainer data_number { "42" };
            auto number = data_number.get<int>();

            REQUIRE(number == 42);
        }
    }

    SECTION("it throws a data_key_error in case of unknown object entry") {
        SECTION("unknown root object entry") {
            REQUIRE_THROWS_AS(data.get<int>("unknown"), data_key_error);
        }

        SECTION("unknown nested object entry") {
            REQUIRE_THROWS_AS(data.get<int>({ "nested", "unknown" }),
                              data_key_error);
        }
    }

    SECTION("it throws a data_type_error in case of mismatch") {
        SECTION("root entry") {
            SECTION("not a boolean") {
                REQUIRE_THROWS_AS(data.get<bool>("string"), data_type_error);
            }

            SECTION("not an integer") {
                REQUIRE_THROWS_AS(data.get<int>("real"), data_type_error);
            }

            SECTION("not a double") {
                REQUIRE_THROWS_AS(data.get<double>("goo"), data_type_error);
            }

            SECTION("not a string") {
                REQUIRE_THROWS_AS(data.get<std::string>("real"), data_type_error);
            }

            SECTION("array mismatches") {
                SECTION("not an array") {
                    REQUIRE_THROWS_AS(data.get<std::vector<int>>("goo"),
                                      data_type_error);
                }

                SECTION("mismatch type on array entry") {
                    REQUIRE_THROWS_AS(data.get<double>("goo"), data_type_error);
                }
            }
        }

        SECTION("nested entry") {
            data.set<JsonContainer>({ "foo", "spam" }, JsonContainer { JSON });

            SECTION("not a boolean") {
                REQUIRE_THROWS_AS(data.get<bool>({ "foo", "spam", "string" }),
                                  data_type_error);
            }

            SECTION("not an integer") {
                REQUIRE_THROWS_AS(data.get<int>({ "foo", "spam", "real" }),
                                  data_type_error);
            }

            SECTION("not a double") {
                REQUIRE_THROWS_AS(data.get<double>({ "foo", "spam", "goo" }),
                                  data_type_error);
            }

            SECTION("not a string") {
                REQUIRE_THROWS_AS(data.get<std::string>({ "foo", "spam", "real" }),
                                  data_type_error);
            }

            SECTION("array mismatches") {
                SECTION("not an array") {
                    REQUIRE_THROWS_AS(
                        data.get<std::vector<int>>({ "foo", "spam", "goo" }),
                        data_type_error);
                }

                SECTION("mismatch type on array entry") {
                    REQUIRE_THROWS_AS(data.get<double>({ "foo", "spam", "goo" }),
                                      data_type_error);
                }
            }
        }
    }

    SECTION("it can always return a JsonContainer instance of an entry") {
        SECTION("scalars") {
            SECTION("boolean") {
                REQUIRE(data.get<JsonContainer>("bool").get<bool>() == true);
            }

            SECTION("integer") {
                REQUIRE(data.get<JsonContainer>("goo").get<int>() == 1);
            }

            SECTION("double") {
                REQUIRE(data.get<JsonContainer>("real").get<double>() == 3.1415);
            }

            SECTION("string") {
                REQUIRE(data.get<JsonContainer>("string").get<std::string>()
                        == "a string");
            }
        }

        SECTION("object") {
            REQUIRE(data.get<JsonContainer>("nested").get<std::string>("foo")
                    == "bar");
        }

        SECTION("array") {
            std::vector<int> expected_array {1, 2};
            REQUIRE(data.get<JsonContainer>("vec").get<std::vector<int>>()
                    == expected_array);
        }
    }

    SECTION("it can access array entries") {
        SECTION("it throws a data_type_error in case of type mismatch") {
            SECTION("root entry") {
                JsonContainer a { "[1, 2, 3]" };
                REQUIRE_THROWS_AS(a.get<std::string>(1), data_type_error);
            }

            SECTION("object entry") {
                REQUIRE_THROWS_AS(data.get<std::string>("vec", 1),
                                  data_type_error);
            }
        }

        SECTION("it throws a data_index_error in case of index out of bounds") {
            SECTION("root entry") {
                JsonContainer a { "[1, 2, 3]" };
                REQUIRE_THROWS_AS(a.get<int>(10), data_index_error);
            }

            SECTION("object entry") {
                REQUIRE_THROWS_AS(data.get<int>("vec", 10), data_index_error);
            }
        }

        SECTION("it can get a value") {
            SECTION("boolean") {
                JsonContainer b { "[false, false, true, false]" };
                REQUIRE_FALSE(b.get<bool>(3));
            }

            SECTION("integer") {
                JsonContainer i { "[1, 2, 3]" };
                REQUIRE(i.get<int>(1) == 2);
            }

            SECTION("double") {
                JsonContainer d { "[3.14, 2.718]" };
                REQUIRE(d.get<double>(1) == 2.718);
            }

            SECTION("string") {
                JsonContainer s { "[\"one\", \"two\"]" };
                REQUIRE(s.get<std::string>(1) == "two");
            }

            SECTION("object") {
                JsonContainer o { "[ {\"spam\":\"eggs\"}, {\"foo\":\"bar\"} ]" };
                auto retrieved_o = o.get<JsonContainer>(0);

                REQUIRE(retrieved_o.size() == 1u);
                REQUIRE(retrieved_o.get<std::string>("spam") == "eggs");
            }

            SECTION("array") {
                JsonContainer a { "[ [1, 2], [false, true], [\"ab\", \"cd\"] ]" };
                std::vector<bool> e_a { false, true };

                REQUIRE(a.get<std::vector<bool>>(1) == e_a);
            }
        }

        SECTION("array with values of different types") {
            JsonContainer a { "[ 1, \"foo\", true, [2.718, 3.14], 42, 42.0, "
                "{\"spam\":\"eggs\"} ]" };

            SECTION("boolean") {
                REQUIRE(a.get<bool>(2));
            }

            SECTION("integer") {
                REQUIRE(a.get<int>(0) == 1);
            }

            SECTION("double") {
                REQUIRE(a.get<double>(5) == 42.0);
            }

            SECTION("string") {
                REQUIRE(a.get<std::string>(1) == "foo");
            }

            SECTION("object") {
                auto retrieved_o = a.get<JsonContainer>(6);

                REQUIRE(retrieved_o.size() == 1u);
                REQUIRE(retrieved_o.get<std::string>("spam") == "eggs");
            }

            SECTION("array") {
                std::vector<double> expected_array { 2.718, 3.14 };

                REQUIRE(a.get<std::vector<double>>(3) == expected_array);
            }
        }
    }
}

TEST_CASE("JsonContainer::getWithDefault", "[data]") {
    JsonContainer data { JSON };
    JsonContainer data_a { "[1, 2, 3]" };
    std::vector<int> ints { 1, 2, 3 };
    std::vector<double> doubles { 1.0, 2.0, 3.0 };
    std::vector<bool> bools { false, true, false };
    std::vector<std::string> strings { "foo", "bar", "baz" };

    SECTION("it can provide a default value if a root entry key is not found") {
        REQUIRE(data.getWithDefault<int>("dne", 42) == 42);
        REQUIRE(data.getWithDefault<double>("dne", 42.0) == 42.0);
        REQUIRE(data.getWithDefault<bool>("dne", true) == true);
        REQUIRE(data.getWithDefault<std::string>("dne", "foo") == "foo");
        REQUIRE(data.getWithDefault<std::vector<int>>("dne", ints) == ints);
        REQUIRE(data.getWithDefault<std::vector<double>>("dne", doubles) == doubles);
        REQUIRE(data.getWithDefault<std::vector<bool>>("dne", bools) == bools);
        REQUIRE(data.getWithDefault<std::vector<std::string>>("dne", strings) == strings);
    }

    SECTION("throw a data_type_error if the root entry is not an object") {
        REQUIRE_THROWS_AS(data_a.getWithDefault<int>("foo", 42), data_type_error);
    }

    SECTION("it can provide a default value if a nested key is not found") {
        JsonContainer lv_2 {};
        lv_2.set<JsonContainer>("entry_3", data);
        JsonContainer lv_1 {};
        lv_1.set<JsonContainer>("entry_2", lv_2);
        std::vector<JsonContainerKey> missing_entry { "entry_2", "entry_3", "dne" };

        REQUIRE(lv_1.getWithDefault<int>(missing_entry, 42) == 42);
        REQUIRE(lv_1.getWithDefault<double>(missing_entry, 42.0) == 42.0);
        REQUIRE(lv_1.getWithDefault<bool>(missing_entry, true) == true);
        REQUIRE(lv_1.getWithDefault<std::string>(missing_entry, "foo")
                == "foo");
        REQUIRE(lv_1.getWithDefault<std::vector<int>>(missing_entry, ints)
                == ints);
        REQUIRE(lv_1.getWithDefault<std::vector<double>>(missing_entry, doubles)
                == doubles);
        REQUIRE(lv_1.getWithDefault<std::vector<bool>>(missing_entry, bools)
                == bools);
        REQUIRE(lv_1.getWithDefault<std::vector<std::string>>(missing_entry, strings)
                == strings);
    }

    SECTION("throw a data_type_error if the parent of a nested entry is not an object") {
        JsonContainer more_data_a {};
        more_data_a.set<std::vector<int>>("ints_entry", ints);

        REQUIRE_THROWS_AS(more_data_a.getWithDefault<int>({ "ints_entry", "foo" }, 42),
                          data_type_error);
    }
}

TEST_CASE("JsonContainer::toString", "[data]") {
    SECTION("root entry") {
        SECTION("object") {
            JsonContainer o {};
            o.set<std::string>("spam", "eggs");
            REQUIRE(o.toString() == "{\"spam\":\"eggs\"}");
        }

        SECTION("array") {
            JsonContainer a { "[1, 2, 3]" };
            REQUIRE(a.toString() == "[1,2,3]");
        }

        SECTION("multi type array") {
            JsonContainer mt_a { "[1, false, \"s\"]" };
            REQUIRE(mt_a.toString() == "[1,false,\"s\"]");
        }

        SECTION("scalar") {
            JsonContainer s { "42" };
            REQUIRE(s.toString() == "42");
        }
    }

    JsonContainer data { JSON };

    SECTION("root object entry") {
        REQUIRE(data.toString("goo") == "1");
    }

    SECTION("nested object entry") {
        REQUIRE(data.toString({ "nested", "foo" }) == "\"bar\"");
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

TEST_CASE("JsonContainer::toPrettyJson", "[data]") {
    SECTION("it pretty prints valid json") {
        SECTION("a null value") {
            std::string EXPECTED_OUTPUT = "null";

            JsonContainer data(EXPECTED_OUTPUT);
            auto pretty_json = data.toPrettyJson();
            REQUIRE(pretty_json == EXPECTED_OUTPUT);
        }

        SECTION("a double") {
            std::string EXPECTED_OUTPUT = "42.5";

            JsonContainer data(EXPECTED_OUTPUT);
            auto pretty_json = data.toPrettyJson();
            REQUIRE(pretty_json == EXPECTED_OUTPUT);
        }

        SECTION("a bool") {
            std::string EXPECTED_OUTPUT = "true";

            JsonContainer data(EXPECTED_OUTPUT);
            auto pretty_json = data.toPrettyJson();
            REQUIRE(pretty_json == EXPECTED_OUTPUT);
        }

        SECTION("an int") {
            std::string EXPECTED_OUTPUT = "42";

            JsonContainer data(EXPECTED_OUTPUT);
            auto pretty_json = data.toPrettyJson();
            REQUIRE(pretty_json == EXPECTED_OUTPUT);
        }

        SECTION("a string") {
            std::string EXPECTED_OUTPUT = "\"string\"";

            JsonContainer data(EXPECTED_OUTPUT);
            auto pretty_json = data.toPrettyJson();
            REQUIRE(pretty_json == EXPECTED_OUTPUT);
        }

        SECTION("a simple array") {
            std::string EXPECTED_OUTPUT =
              "[\n"
              "    null,\n"
              "    42.5,\n"
              "    true,\n"
              "    42,\n"
              "    \"string\"\n"
              "]";

            JsonContainer data(EXPECTED_OUTPUT);
            auto pretty_json = data.toPrettyJson();
            REQUIRE(pretty_json == EXPECTED_OUTPUT);
        }

        SECTION("a simple object") {
            std::string EXPECTED_OUTPUT =
              "{\n"
              "    \"null-key\": null,\n"
              "    \"double-key\": 42.5,\n"
              "    \"bool-key\": true,\n"
              "    \"int-key\": 42,\n"
              "    \"string-key\": \"string\"\n"
              "}";

            JsonContainer data(EXPECTED_OUTPUT);
            auto pretty_json = data.toPrettyJson();
            REQUIRE(pretty_json == EXPECTED_OUTPUT);
        }

        SECTION("a nested object") {
            std::string EXPECTED_OUTPUT =
              "{\n"
              "    \"object-key\": {\n"
              "        \"null-key\": null,\n"
              "        \"double-key\": 42.5,\n"
              "        \"bool-key\": true,\n"
              "        \"int-key\": 42,\n"
              "        \"string-key\": \"string\"\n"
              "    },\n"
              "    \"array-key\": [\n"
              "        null,\n"
              "        42.5,\n"
              "        true,\n"
              "        42,\n"
              "        \"string\"\n"
              "    ]\n"
              "}";

            JsonContainer data(EXPECTED_OUTPUT);
            auto pretty_json = data.toPrettyJson();
            REQUIRE(pretty_json == EXPECTED_OUTPUT);
        }

        SECTION("a nested array") {
            std::string EXPECTED_OUTPUT =
              "[\n"
              "    {\n"
              "        \"null-key\": null,\n"
              "        \"double-key\": 42.5,\n"
              "        \"bool-key\": true,\n"
              "        \"int-key\": 42,\n"
              "        \"string-key\": \"string\"\n"
              "    },\n"
              "    [\n"
              "        null,\n"
              "        42.5,\n"
              "        true,\n"
              "        42,\n"
              "        \"string\"\n"
              "    ]\n"
              "]";

            JsonContainer data(EXPECTED_OUTPUT);
            auto pretty_json = data.toPrettyJson();
            REQUIRE(pretty_json == EXPECTED_OUTPUT);
        }
    }

    SECTION("handles variable padding") {
        std::string EXPECTED_OUTPUT =
          "{\n"
          "  \"object-key\": {\n"
          "    \"null-key\": null,\n"
          "    \"double-key\": 42.5,\n"
          "    \"bool-key\": true,\n"
          "    \"int-key\": 42,\n"
          "    \"string-key\": \"string\"\n"
          "  },\n"
          "  \"array-key\": [\n"
          "    null,\n"
          "    42.5,\n"
          "    true,\n"
          "    42,\n"
          "    \"string\"\n"
          "  ]\n"
          "}";

        JsonContainer data(EXPECTED_OUTPUT);
        auto pretty_json = data.toPrettyJson(2);
        REQUIRE(pretty_json == EXPECTED_OUTPUT);
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

TEST_CASE("JsonContainer::size", "[data]") {
    SECTION("works correctly on the root (no key argument)") {
        SECTION("empty object") {
            JsonContainer data {};
            REQUIRE(data.size() == 0u);
        }

        SECTION("the root is an empty array") {
            JsonContainer data {  "[]" };
            REQUIRE(data.size() == 0u);
        }

        SECTION("non-empty singleton object") {
            JsonContainer data {};
            data.set<int>("spam", 1);
            REQUIRE(data.size() == 1u);
        }

        SECTION("non-empty multi element object") {
            JsonContainer bigger_data { JSON };
            REQUIRE(bigger_data.size() == 10u);
        }

        SECTION("non-empty array") {
            JsonContainer data {  "[1, 2, 3]" };
            REQUIRE(data.size() == 3u);
        }
    }

    JsonContainer data { JSON };

    SECTION("works correctly on an object entry") {
        SECTION("entry is a scalar") {
            REQUIRE(data.size("goo") == 0u);
        }

        SECTION("entry is an object") {
            REQUIRE(data.size("foo") == 1u);
        }

        SECTION("entry is an array") {
            REQUIRE(data.size("vec") == 2u);
        }
    }

    SECTION("works correctly on a nested entry") {
        data.set<JsonContainer>({ "foo", "spam" }, JsonContainer { JSON });

        SECTION("entry is a scalar") {
            REQUIRE(data.size({ "foo", "spam", "goo" }) == 0u);
        }

        SECTION("entry is an object") {
            REQUIRE(data.size({ "foo", "spam", "nested" }) == 1u);
        }

        SECTION("entry is an array") {
            REQUIRE(data.size({ "foo", "spam", "vec" }) == 2u);
        }
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

        // Expecting msg = { "d_c entry" : {"i" : 1} }
        REQUIRE(msg.includes("d_c entry"));
        REQUIRE(i_entry.get<int>("i") == 1);

        JsonContainer d_other {};
        d_other.set<bool>("b", true);

        msg.set<JsonContainer>("d_c entry", d_other);
        auto b_entry = msg.get<JsonContainer>("d_c entry");

        // Expecting msg = { "d_c entry" : {"b" : true} }
        REQUIRE(b_entry.get<bool>("b"));
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
            " \"b\" : 2, "
                " \"c\\u0000null\" : 2}" };
        std::vector<std::string> expected_keys { "a", "b", { "c\0null", 6 } };

        REQUIRE(data.keys() == expected_keys);
    }

    SECTION("It returns an empty vector when the JsonContainer is empty") {
        JsonContainer data {};
        REQUIRE(data.keys().size() == 0u);
    }

    SECTION("It returns an empty vector when the JsonContainer is an array") {
        JsonContainer data_array { "[1, 2, 3]" };
        REQUIRE(data_array.keys().size() == 0u);
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

        SECTION("integer") {
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
            SECTION("defined by set") {
                data.set<double>("d_entry", 2.71828);
                REQUIRE(data.type("d_entry") == DataType::Double);
            }

            SECTION("defined by JSON string given to the ctor") {
                JsonContainer data_number { "2.71828" };
                REQUIRE(data_number.type() == DataType::Double);
            }
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

TEST_CASE("JsonContainer::type for arrays entries", "[data]") {
    JsonContainer data { "[false, -42, 3.14, \"spam\", {\"foo\" : [3, true]}, "
        "[1, 2, 3, 4] ]" };

    SECTION("root entry") {
        SECTION("array") {
            JsonContainer not_an_aray { JSON };
            REQUIRE_THROWS_AS(not_an_aray.type(1), data_type_error);
        }

        SECTION("array with values of different types") {
            JsonContainer data_array { "[1, \"spam\", false]" };
            REQUIRE(data_array.type() == DataType::Array);
        }

        SECTION("boolean") {
            REQUIRE(data.type(0) == DataType::Bool);
        }

        SECTION("integer") {
            REQUIRE(data.type(1) == DataType::Int);
        }

        SECTION("double") {
            REQUIRE(data.type(2) == DataType::Double);
        }

        SECTION("string") {
            REQUIRE(data.type(3) == DataType::String);
        }

        SECTION("object") {
            REQUIRE(data.type(4) == DataType::Object);
        }

        SECTION("array") {
            REQUIRE(data.type(5) == DataType::Array);
        }
    }

    SECTION("object entry") {
        JsonContainer o { JSON };
        o.set<JsonContainer>("multi type array", data);

        SECTION("container") {
            REQUIRE(o.type("multi type array") == DataType::Array);
        }

        SECTION("double") {
            REQUIRE(o.type("multi type array", 2) == DataType::Double);
        }

        SECTION("string") {
            REQUIRE(o.type("multi type array", 3) == DataType::String);
        }
    }

    SECTION("nested entry") {
        JsonContainer o { JSON };
        o.set<JsonContainer>({ "nested", "multi type array" }, data);

        SECTION("container") {
            REQUIRE(o.type({ "nested", "multi type array" }) == DataType::Array);
        }

        SECTION("double") {
            REQUIRE(o.type({ "nested", "multi type array" }, 2)
                    == DataType::Double);
        }

        SECTION("string") {
            REQUIRE(o.type({ "nested", "multi type array" }, 3)
                    == DataType::String);
        }
    }
}
