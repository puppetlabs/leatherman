#include <catch.hpp>
#include <leatherman/ruby/api.hpp>
#include <limits>

using namespace std;
using namespace leatherman::ruby;

TEST_CASE("api::eval", "[ruby-api]") {
    SECTION("can load api and evaluate ruby code") {
        auto& ruby = api::instance();
        ruby.initialize();
        REQUIRE(ruby.initialized());

        REQUIRE(ruby.get_load_path().size() > 0u);

        REQUIRE(ruby.to_string(ruby.eval("'foo'")) == "foo");
    }
}

TEST_CASE("api::is_*", "[ruby-api]") {
    auto& ruby = api::instance();
    ruby.initialize();
    REQUIRE(ruby.initialized());

    SECTION("can correctly identify nil values") {
        REQUIRE(ruby.is_nil(ruby.nil_value()));
        REQUIRE_FALSE(ruby.is_nil(ruby.true_value()));
    }

    SECTION("can correctly identify true and false values") {
        REQUIRE(ruby.is_true(ruby.true_value()));
        REQUIRE_FALSE(ruby.is_true(ruby.false_value()));
        REQUIRE(ruby.is_false(ruby.false_value()));
        REQUIRE_FALSE(ruby.is_false(ruby.true_value()));
    }

    SECTION("can correctly identify strings") {
        REQUIRE(ruby.is_string(ruby.utf8_value("'I'm a string'")));
        REQUIRE_FALSE(ruby.is_string(ruby.true_value()));
    }

    SECTION("can correctly identify symbols") {
        REQUIRE(ruby.is_symbol(ruby.to_symbol("mysymbol")));
        REQUIRE_FALSE(ruby.is_symbol(ruby.false_value()));
    }

    SECTION("can correctly identify numbers") {
        REQUIRE(ruby.is_float(ruby.eval("1.5")));
        REQUIRE_FALSE(ruby.is_float(ruby.utf8_value("foo")));

        REQUIRE(ruby.is_integer(ruby.eval("2")));
        REQUIRE_FALSE(ruby.is_integer(ruby.eval("1.5")));
    }

    SECTION("can correctly identify hashes") {
        REQUIRE(ruby.is_hash(ruby.eval("{ 'red' => 2 }")));
        REQUIRE_FALSE(ruby.is_hash(ruby.utf8_value("foo")));
    }

    SECTION("can correctly identify type") {
        REQUIRE(ruby.is_a(ruby.eval("1"), ruby.eval("Integer")));
        REQUIRE_FALSE(ruby.is_a(ruby.eval("'1'"), ruby.eval("Integer")));
    }

    SECTION("can correctly identify arrays") {
        REQUIRE(ruby.is_array(ruby.eval("[1, 2, 3]")));
        REQUIRE_FALSE(ruby.is_array(ruby.false_value()));
    }
}

TEST_CASE("api::equals", "[ruby-api]") {
    auto& ruby = api::instance();
    ruby.initialize();
    REQUIRE(ruby.initialized());
    SECTION("can correctly test boolean values for equality") {
        REQUIRE(ruby.equals(ruby.true_value(), ruby.true_value()));
        REQUIRE_FALSE(ruby.equals(ruby.true_value(), ruby.false_value()));
    }

    SECTION("can correctly test strings for equality") {
        REQUIRE(ruby.equals(ruby.utf8_value("foo"), ruby.utf8_value("foo")));
        REQUIRE_FALSE(ruby.equals(ruby.utf8_value("foo"), ruby.utf8_value("bar")));
    }

    SECTION("can correctly test numbers for equality") {
        REQUIRE(ruby.equals(ruby.eval("1"), ruby.eval("1")));
        REQUIRE_FALSE(ruby.equals(ruby.eval("1"), ruby.eval("3")));
        REQUIRE(ruby.equals(ruby.eval("1.5"), ruby.eval("1.5")));
        REQUIRE_FALSE(ruby.equals(ruby.eval("1.5"), ruby.eval("1")));
    }

    SECTION("can correctly test Ruby hashes for equality") {
        REQUIRE(ruby.equals(ruby.eval("{ 'red' => 'blue' }"), ruby.eval("{ 'red' => 'blue' }")));
        REQUIRE_FALSE(ruby.equals(ruby.eval("{ 'red' => 'blue' }"), ruby.eval("{ 'red' => 'green' }")));
    }

    SECTION("can correctly test symbols for equality") {
        REQUIRE(ruby.equals(ruby.to_symbol("mysymbol"), ruby.eval(":mysymbol")));
        REQUIRE_FALSE(ruby.equals(ruby.to_symbol("mysymbol"), ruby.to_symbol("notmysymbol")));
    }
}

TEST_CASE("api::case_equals", "[ruby-api]") {
    auto& ruby = api::instance();
    ruby.initialize();
    REQUIRE(ruby.initialized());

    SECTION("can detect class membership") {
        REQUIRE(ruby.case_equals(ruby.eval("Integer"), ruby.eval("1")));
        REQUIRE_FALSE(ruby.case_equals(ruby.eval("String"), ruby.eval("4")));
    }
}

VALUE test_func(VALUE self) {
    auto& ruby = api::instance();
    ruby.initialize();
    return ruby.utf8_value("test function");
}

TEST_CASE("api::rb_define_singleton_method", "[ruby-api]") {
    SECTION("can define a new module with a new method") {
        auto& ruby = api::instance();
        ruby.initialize();
        REQUIRE(ruby.initialized());

        auto module = ruby.rb_define_module("Test");
        REQUIRE(module);
        ruby.rb_define_singleton_method(module, "test_func", RUBY_METHOD_FUNC(test_func), 0);
        REQUIRE(ruby.to_string(ruby.eval("Test.test_func")) == "test function");
    }
}

TEST_CASE("api::exception_to_string", "[ruby-api]") {
    auto& ruby = api::instance();
    ruby.initialize();
    REQUIRE(ruby.initialized());

    SECTION("can print exception details") {
        try {
            ruby.eval("raise 'test_exception'");
        } catch (runtime_error exc) {
            REQUIRE(string(exc.what()) == "test_exception");
        }
    }

    SECTION("can print exception details with stack trace") {
        ruby.include_stack_trace(true);
        try {
            ruby.eval("raise 'test_exception'");
        } catch (runtime_error exc) {
            REQUIRE(string(exc.what()).find("backtrace") != string::npos);
        }
    }
}

TEST_CASE("api::lookup", "[ruby-api]") {
    auto& ruby = api::instance();
    ruby.initialize();
    REQUIRE(ruby.initialized());

    SECTION("can find module by name") {
        auto foo_module = ruby.rb_define_module("Foo");
        ruby.rb_define_module_under(foo_module, "Bar");
        REQUIRE(ruby.to_string(ruby.lookup({ "Foo", "Bar" })) == "Foo::Bar");
    }
}

TEST_CASE("api::to_string", "[ruby-api]") {
    auto& ruby = api::instance();
    ruby.initialize();
    REQUIRE(ruby.initialized());

    SECTION("can normalize encodings") {
        string john {"J\xc3\xb6hn"};
        auto obj = ruby.utf8_value(john);
        auto encoded = ruby.rb_funcall(obj, ruby.rb_intern("encode"), 1, ruby.utf8_value("Windows-1252"));
        REQUIRE(ruby.to_string(encoded) == john);
    }
}

TEST_CASE("api::num2size_t", "[ruby-api]") {
    auto& ruby = api::instance();
    ruby.initialize();
    REQUIRE(ruby.initialized());

    SECTION("can convert Ruby number to size_t") {
        auto fixednum = ruby.eval("1");
        auto num = ruby.num2size_t(fixednum);
        REQUIRE(1u == num);
    }

    SECTION("can convert large Ruby number to size_t") {
        auto expected = numeric_limits<size_t>::max();
        auto largenum = ruby.eval(to_string(expected));
        auto num = ruby.num2size_t(largenum);
        REQUIRE(expected == num);
    }

#if 0
    // Can't use this test yet, because Ruby SIGSEGVs on calling rb_num2ull.
    SECTION("throws exception on Ruby numbers exceeding size_t") {
        auto largenum = ruby.eval("184467440737095516150");
        REQUIRE_THROWS_AS(ruby.num2size_t(largenum), runtime_error);
    }
#endif
}
