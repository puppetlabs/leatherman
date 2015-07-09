#include <leatherman/ruby/api.hpp>

using namespace std;

namespace lth_lib = leatherman::dynamic_library;

namespace leatherman { namespace ruby {

    lth_lib::dynamic_library api::find_loaded_library()
    {
        return lth_lib::dynamic_library::find_by_symbol("ruby_init");
    }

}}  // namespace leatherman::ruby
