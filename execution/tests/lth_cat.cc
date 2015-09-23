#include "lth_cat.hpp"
#include <boost/nowide/iostream.hpp>
#include <set>
namespace nw = boost::nowide;
using namespace std;

static string prompt(set<string> const& codes) {
    if (codes.count("overwhelm")) {
        return lth_cat::overwhelm;
    }
    return {};
}

int main(int argc, char** argv)
{
    // Enable special testing modes
    set<string> codes(argv+1, argv+argc);

    if (codes.count("prefix")) {
        nw::cout << lth_cat::prefix << flush;
    }

    string buf;
    nw::cout << prompt(codes) << flush;
    while (getline(nw::cin, buf)) {
        nw::cout << buf << endl;
        if (codes.count("stderr")) {
            nw::cerr << buf << endl;
        }
        nw::cout << prompt(codes) << flush;
    }

    if (codes.count("suffix")) {
        nw::cout << lth_cat::suffix << flush;
    }

    return 0;
}
