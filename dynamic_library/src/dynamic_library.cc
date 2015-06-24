#include <leatherman/dynamic_library/dynamic_library.hpp>

using namespace std;

namespace leatherman { namespace dynamic_library {

    missing_import_exception::missing_import_exception(string const& message) :
        runtime_error(message)
    {
    }

    DynamicLibrary::DynamicLibrary() :
        _handle(nullptr),
        _first_load(false)
    {
    }

    DynamicLibrary::~DynamicLibrary()
    {
        close();
    }

    DynamicLibrary::DynamicLibrary(DynamicLibrary && other) :
        _handle(nullptr),
        _first_load(false)
    {
        *this = move(other);
    }

    DynamicLibrary &DynamicLibrary::operator=(DynamicLibrary && other)
    {
        close();
        _handle = other._handle;
        _name = other._name;
        _first_load = other._first_load;
        other._handle = nullptr;
        other._name.clear();
        other._first_load = false;
        return *this;
    }

    bool DynamicLibrary::loaded() const
    {
        return _handle != nullptr;
    }

    bool DynamicLibrary::first_load() const
    {
        return _first_load;
    }

    string const& DynamicLibrary::name() const
    {
        return _name;
    }

}}  // namespace leatherman::dynamic_library
