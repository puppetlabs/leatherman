#include "logging.hpp"

using namespace leatherman::test;

SCENARIO("formatting with a TRACE level macro") {
    logging_format_context context(log_level::trace, LOG_NAMESPACE);
    REQUIRE(LOG_IS_TRACE_ENABLED());

    LOG_TRACE("testing {1} {2} {3}", 1, "2", 3.0);
    CAPTURE(context.message());
    REQUIRE(context.expected().size() == context.tokens().size());
    for (auto tup : zip_view(context.tokens(), context.expected())) {
        REQUIRE(get<0>(tup) == get<1>(tup));
    }
    REQUIRE_FALSE(error_has_been_logged());
}

SCENARIO("formatting with a TRACE level directly") {
    logging_format_context context(log_level::trace, "test");
    REQUIRE(LOG_IS_TRACE_ENABLED());

    log("test", log_level::trace, 0, "testing {1} {2} {3}", 1, "2", 3.0);
    CAPTURE(context.message());
    REQUIRE(context.expected().size() == context.tokens().size());
    for (auto tup : zip_view(context.tokens(), context.expected())) {
        REQUIRE(get<0>(tup) == get<1>(tup));
    }
    REQUIRE_FALSE(error_has_been_logged());
}

SCENARIO("formatting with a DEBUG level macro") {
    logging_format_context context(log_level::debug, LOG_NAMESPACE);
    REQUIRE(LOG_IS_DEBUG_ENABLED());

    LOG_DEBUG("testing {1} {2} {3}", 1, "2", 3.0);
    CAPTURE(context.message());
    REQUIRE(context.expected().size() == context.tokens().size());
    for (auto tup : zip_view(context.tokens(), context.expected())) {
        REQUIRE(get<0>(tup) == get<1>(tup));
    }
    REQUIRE_FALSE(error_has_been_logged());
}

SCENARIO("formatting with a DEBUG level directly") {
    logging_format_context context(log_level::debug, "test");
    REQUIRE(LOG_IS_DEBUG_ENABLED());

    log("test", log_level::debug, 0, "testing {1} {2} {3}", 1, "2", 3.0);
    CAPTURE(context.message());
    REQUIRE(context.expected().size() == context.tokens().size());
    for (auto tup : zip_view(context.tokens(), context.expected())) {
        REQUIRE(get<0>(tup) == get<1>(tup));
    }
    REQUIRE_FALSE(error_has_been_logged());
}

SCENARIO("formatting with a INFO level macro") {
    logging_format_context context(log_level::info, LOG_NAMESPACE);
    REQUIRE(LOG_IS_INFO_ENABLED());

    LOG_INFO("testing {1} {2} {3}", 1, "2", 3.0);
    CAPTURE(context.message());
    REQUIRE(context.expected().size() == context.tokens().size());
    for (auto tup : zip_view(context.tokens(), context.expected())) {
        REQUIRE(get<0>(tup) == get<1>(tup));
    }
    REQUIRE_FALSE(error_has_been_logged());
}

SCENARIO("formatting with a INFO level directly") {
    logging_format_context context(log_level::info, "test");
    REQUIRE(LOG_IS_INFO_ENABLED());

    log("test", log_level::info, 0, "testing {1} {2} {3}", 1, "2", 3.0);
    CAPTURE(context.message());
    REQUIRE(context.expected().size() == context.tokens().size());
    for (auto tup : zip_view(context.tokens(), context.expected())) {
        REQUIRE(get<0>(tup) == get<1>(tup));
    }
    REQUIRE_FALSE(error_has_been_logged());
}

SCENARIO("formatting with a WARNING level macro") {
    logging_format_context context(log_level::warning, LOG_NAMESPACE);
    REQUIRE(LOG_IS_WARNING_ENABLED());

    LOG_WARNING("testing {1} {2} {3}", 1, "2", 3.0);
    CAPTURE(context.message());
    REQUIRE(context.expected().size() == context.tokens().size());
    for (auto tup : zip_view(context.tokens(), context.expected())) {
        REQUIRE(get<0>(tup) == get<1>(tup));
    }
    REQUIRE_FALSE(error_has_been_logged());
}

SCENARIO("formatting with a WARNING level directly") {
    logging_format_context context(log_level::warning, "test");
    REQUIRE(LOG_IS_WARNING_ENABLED());

    log("test", log_level::warning, 0, "testing {1} {2} {3}", 1, "2", 3.0);
    CAPTURE(context.message());
    REQUIRE(context.expected().size() == context.tokens().size());
    for (auto tup : zip_view(context.tokens(), context.expected())) {
        REQUIRE(get<0>(tup) == get<1>(tup));
    }
    REQUIRE_FALSE(error_has_been_logged());
}

SCENARIO("formatting with a ERROR level macro") {
    logging_format_context context(log_level::error, LOG_NAMESPACE);
    REQUIRE(LOG_IS_ERROR_ENABLED());

    LOG_ERROR("testing {1} {2} {3}", 1, "2", 3.0);
    CAPTURE(context.message());
    REQUIRE(context.expected().size() == context.tokens().size());
    for (auto tup : zip_view(context.tokens(), context.expected())) {
        REQUIRE(get<0>(tup) == get<1>(tup));
    }
    REQUIRE(error_has_been_logged());
}

SCENARIO("formatting with a ERROR level directly") {
    logging_format_context context(log_level::error, "test");
    REQUIRE(LOG_IS_ERROR_ENABLED());

    log("test", log_level::error, 0, "testing {1} {2} {3}", 1, "2", 3.0);
    CAPTURE(context.message());
    REQUIRE(context.expected().size() == context.tokens().size());
    for (auto tup : zip_view(context.tokens(), context.expected())) {
        REQUIRE(get<0>(tup) == get<1>(tup));
    }
    REQUIRE(error_has_been_logged());
}

SCENARIO("formatting with a FATAL level macro") {
    logging_format_context context(log_level::fatal, LOG_NAMESPACE);
    REQUIRE(LOG_IS_FATAL_ENABLED());

    LOG_FATAL("testing {1} {2} {3}", 1, "2", 3.0);
    CAPTURE(context.message());
    REQUIRE(context.expected().size() == context.tokens().size());
    for (auto tup : zip_view(context.tokens(), context.expected())) {
        REQUIRE(get<0>(tup) == get<1>(tup));
    }
    REQUIRE(error_has_been_logged());
}

SCENARIO("formatting with a FATAL level directly") {
    logging_format_context context(log_level::fatal, "test");
    REQUIRE(LOG_IS_FATAL_ENABLED());

    log("test", log_level::fatal, 0, "testing {1} {2} {3}", 1, "2", 3.0);
    CAPTURE(context.message());
    REQUIRE(context.expected().size() == context.tokens().size());
    for (auto tup : zip_view(context.tokens(), context.expected())) {
        REQUIRE(get<0>(tup) == get<1>(tup));
    }
    REQUIRE(error_has_been_logged());
}
