#define LEATHERMAN_LOGGING_LINE_NUMBERS
#include "logging.hpp"

using namespace leatherman::test;

SCENARIO("formatting with lines with a TRACE level macro") {
    int line_num = __LINE__ + 4;
    logging_format_context context(LogLevel::trace, LOG_NAMESPACE, line_num);
    REQUIRE(LOG_IS_TRACE_ENABLED());

    LOG_TRACE("testing %1% %2% %3%", 1, "2", 3.0);
    CAPTURE(context.message());
    REQUIRE(context.expected().size() == context.tokens().size());
    for (auto tup : zip_view(context.tokens(), context.expected())) {
        REQUIRE(get<0>(tup) == get<1>(tup));
    }
    REQUIRE_FALSE(error_has_been_logged());
}

SCENARIO("formatting with lines with a TRACE level directly") {
    int line_num = __LINE__ + 4;
    logging_format_context context(LogLevel::trace, "test", line_num);
    REQUIRE(LOG_IS_TRACE_ENABLED());

    log("test", LogLevel::trace, line_num, "testing %1% %2% %3%", 1, "2", 3.0);
    CAPTURE(context.message());
    REQUIRE(context.expected().size() == context.tokens().size());
    for (auto tup : zip_view(context.tokens(), context.expected())) {
        REQUIRE(get<0>(tup) == get<1>(tup));
    }
    REQUIRE_FALSE(error_has_been_logged());
}

SCENARIO("formatting with lines with a DEBUG level macro") {
    int line_num = __LINE__ + 4;
    logging_format_context context(LogLevel::debug, LOG_NAMESPACE, line_num);
    REQUIRE(LOG_IS_DEBUG_ENABLED());

    LOG_DEBUG("testing %1% %2% %3%", 1, "2", 3.0);
    CAPTURE(context.message());
    REQUIRE(context.expected().size() == context.tokens().size());
    for (auto tup : zip_view(context.tokens(), context.expected())) {
        REQUIRE(get<0>(tup) == get<1>(tup));
    }
    REQUIRE_FALSE(error_has_been_logged());
}

SCENARIO("formatting with lines with a DEBUG level directly") {
    int line_num = __LINE__ + 4;
    logging_format_context context(LogLevel::debug, "test", line_num);
    REQUIRE(LOG_IS_DEBUG_ENABLED());

    log("test", LogLevel::debug, line_num, "testing %1% %2% %3%", 1, "2", 3.0);
    CAPTURE(context.message());
    REQUIRE(context.expected().size() == context.tokens().size());
    for (auto tup : zip_view(context.tokens(), context.expected())) {
        REQUIRE(get<0>(tup) == get<1>(tup));
    }
    REQUIRE_FALSE(error_has_been_logged());
}

SCENARIO("formatting with lines with a INFO level macro") {
    int line_num = __LINE__ + 4;
    logging_format_context context(LogLevel::info, LOG_NAMESPACE, line_num);
    REQUIRE(LOG_IS_INFO_ENABLED());

    LOG_INFO("testing %1% %2% %3%", 1, "2", 3.0);
    CAPTURE(context.message());
    REQUIRE(context.expected().size() == context.tokens().size());
    for (auto tup : zip_view(context.tokens(), context.expected())) {
        REQUIRE(get<0>(tup) == get<1>(tup));
    }
    REQUIRE_FALSE(error_has_been_logged());
}

SCENARIO("formatting with lines with a INFO level directly") {
    int line_num = __LINE__ + 4;
    logging_format_context context(LogLevel::info, "test", line_num);
    REQUIRE(LOG_IS_INFO_ENABLED());

    log("test", LogLevel::info, line_num, "testing %1% %2% %3%", 1, "2", 3.0);
    CAPTURE(context.message());
    REQUIRE(context.expected().size() == context.tokens().size());
    for (auto tup : zip_view(context.tokens(), context.expected())) {
        REQUIRE(get<0>(tup) == get<1>(tup));
    }
    REQUIRE_FALSE(error_has_been_logged());
}

SCENARIO("formatting with lines with a WARNING level macro") {
    int line_num = __LINE__ + 4;
    logging_format_context context(LogLevel::warning, LOG_NAMESPACE, line_num);
    REQUIRE(LOG_IS_WARNING_ENABLED());

    LOG_WARNING("testing %1% %2% %3%", 1, "2", 3.0);
    CAPTURE(context.message());
    REQUIRE(context.expected().size() == context.tokens().size());
    for (auto tup : zip_view(context.tokens(), context.expected())) {
        REQUIRE(get<0>(tup) == get<1>(tup));
    }
    REQUIRE_FALSE(error_has_been_logged());
}

SCENARIO("formatting with lines with a WARNING level directly") {
    int line_num = __LINE__ + 4;
    logging_format_context context(LogLevel::warning, "test", line_num);
    REQUIRE(LOG_IS_WARNING_ENABLED());

    log("test", LogLevel::warning, line_num, "testing %1% %2% %3%", 1, "2", 3.0);
    CAPTURE(context.message());
    REQUIRE(context.expected().size() == context.tokens().size());
    for (auto tup : zip_view(context.tokens(), context.expected())) {
        REQUIRE(get<0>(tup) == get<1>(tup));
    }
    REQUIRE_FALSE(error_has_been_logged());
}

SCENARIO("formatting with lines with a ERROR level macro") {
    int line_num = __LINE__ + 4;
    logging_format_context context(LogLevel::error, LOG_NAMESPACE, line_num);
    REQUIRE(LOG_IS_ERROR_ENABLED());

    LOG_ERROR("testing %1% %2% %3%", 1, "2", 3.0);
    CAPTURE(context.message());
    REQUIRE(context.expected().size() == context.tokens().size());
    for (auto tup : zip_view(context.tokens(), context.expected())) {
        REQUIRE(get<0>(tup) == get<1>(tup));
    }
    REQUIRE(error_has_been_logged());
}

SCENARIO("formatting with lines with a ERROR level directly") {
    int line_num = __LINE__ + 4;
    logging_format_context context(LogLevel::error, "test", line_num);
    REQUIRE(LOG_IS_ERROR_ENABLED());

    log("test", LogLevel::error, line_num, "testing %1% %2% %3%", 1, "2", 3.0);
    CAPTURE(context.message());
    REQUIRE(context.expected().size() == context.tokens().size());
    for (auto tup : zip_view(context.tokens(), context.expected())) {
        REQUIRE(get<0>(tup) == get<1>(tup));
    }
    REQUIRE(error_has_been_logged());
}

SCENARIO("formatting with lines with a FATAL level macro") {
    int line_num = __LINE__ + 4;
    logging_format_context context(LogLevel::fatal, LOG_NAMESPACE, line_num);
    REQUIRE(LOG_IS_FATAL_ENABLED());

    LOG_FATAL("testing %1% %2% %3%", 1, "2", 3.0);
    CAPTURE(context.message());
    REQUIRE(context.expected().size() == context.tokens().size());
    for (auto tup : zip_view(context.tokens(), context.expected())) {
        REQUIRE(get<0>(tup) == get<1>(tup));
    }
    REQUIRE(error_has_been_logged());
}

SCENARIO("formatting with lines with a FATAL level directly") {
    int line_num = __LINE__ + 4;
    logging_format_context context(LogLevel::fatal, "test", line_num);
    REQUIRE(LOG_IS_FATAL_ENABLED());

    log("test", LogLevel::fatal, line_num, "testing %1% %2% %3%", 1, "2", 3.0);
    CAPTURE(context.message());
    REQUIRE(context.expected().size() == context.tokens().size());
    for (auto tup : zip_view(context.tokens(), context.expected())) {
        REQUIRE(get<0>(tup) == get<1>(tup));
    }
    REQUIRE(error_has_been_logged());
}
