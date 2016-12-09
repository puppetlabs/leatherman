#include <catch.hpp>
#include <leatherman/execution/execution.hpp>
#include <leatherman/util/strings.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace leatherman::execution;

SCENARIO("executing detached commands with execution::execute") {
    auto get_ctids = [](string const& input) {
        vector<int> ctids;
        leatherman::util::each_line(input, [&ctids](string& line) {
            try {
                boost::algorithm::trim(line);
                ctids.push_back(boost::lexical_cast<int>(line));
            } catch(boost::bad_lexical_cast const&) {
                ctids.clear();
                return false;
            }
            return true;
        });
        return ctids;
    };
    GIVEN("the detached process creation is requested") {
        THEN("the command is executed in a different process contract than its parent") {
            auto exec = execute("/bin/sh", { "-c", "ps -o ctid= -p $EXECUTOR_PID,$$" },
                                { { "EXECUTOR_PID", to_string(getpid()) } },
                                0,
                                { execution_options::create_detached_process,
                                  execution_options::merge_environment,
                                  execution_options::redirect_stderr_to_null });
            REQUIRE(exec.success);
            REQUIRE(exec.exit_code == 0);
            auto ctids = get_ctids(exec.output);
            REQUIRE(ctids.size() == 2); // the ps command returned two CTIDs
            REQUIRE(ctids[0] != ctids[1]); // the contract IDs are different
        }
    }
    GIVEN("the detached process creation is NOT requested") {
        THEN("the command is executed in the same process contract as its parent") {
            auto exec = execute("/bin/sh", { "-c", "ps -o ctid= -p $EXECUTOR_PID,$$" },
                                { { "EXECUTOR_PID", to_string(getpid()) } },
                                0,
                                { execution_options::merge_environment,
                                  execution_options::redirect_stderr_to_null });
            REQUIRE(exec.success);
            REQUIRE(exec.exit_code == 0);
            auto ctids = get_ctids(exec.output);
            REQUIRE(ctids.size() == 2); // the ps command returned two CTIDs
            REQUIRE(ctids[0] == ctids[1]); // the contract IDs are the same
        }
    }
}
