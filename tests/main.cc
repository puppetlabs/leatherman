#define CATCH_CONFIG_RUNNER
#include <catch.hpp>

// To enable log messages:
// #define ENABLE_LOGGING

#ifdef ENABLE_LOGGING
#include <leatherman/logging/logging.hpp>
#endif

int main(int argc, char **argv)
{
#ifdef ENABLE_LOGGING
    leatherman::logging::set_level(leatherman::logging::log_level::debug);
#endif
    return Catch::Session().run( argc, argv );
}
