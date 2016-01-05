#include <leatherman/ruby/api.hpp>

using namespace std;

namespace lth_lib = leatherman::dynamic_library;

namespace leatherman { namespace ruby {

    lth_lib::dynamic_library api::find_loaded_library()
    {
        // Ruby DLL's follow a pattern of
        //   ruby.dll, libruby.dll, ruby210.dll, libruby210.dll
        //   msvcrt-ruby193.dll, x64-msvcrt-ruby210.dll, etc
        // To avoid detecting leatherman_ruby.dll as a Ruby DLL, look for
        // anything except an underscore.
        const string libruby_pattern = "^[^_]*ruby(\\d)?(\\d)?(\\d)?\\.dll$";
        return lth_lib::dynamic_library::find_by_pattern(libruby_pattern);
    }

}}  // namespace leatherman::ruby
