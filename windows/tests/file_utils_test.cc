#include <catch.hpp>
#include <leatherman/windows/file_util.hpp>

namespace leatherman { namespace windows { namespace file_util {

    TEST_CASE("windows::file_util::get_programdata_dir", "[windows]") {

        SECTION("should return the expected value of C:\\ProgramData") {
            REQUIRE(get_programdata_dir() == "C:\\ProgramData");
        }
    }

}}} // namespace leatherman::windows::file_util
