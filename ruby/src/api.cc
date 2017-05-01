#include <leatherman/ruby/api.hpp>
#include <leatherman/util/environment.hpp>
#include <leatherman/execution/execution.hpp>
#include <leatherman/logging/logging.hpp>
#include <leatherman/locale/locale.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <sstream>

// Mark string for translation (alias for leatherman::locale::format)
using leatherman::locale::_;

using namespace std;
using namespace leatherman::util;
using namespace leatherman::execution;
using namespace boost::filesystem;

namespace lth_lib = leatherman::dynamic_library;

namespace leatherman { namespace ruby {

    string api::ruby_lib_location = "";

    set<VALUE> api::_data_objects;

    library_not_loaded_exception::library_not_loaded_exception(string const& message) :
        runtime_error(message)
    {
    }

    invalid_conversion::invalid_conversion(string const& message) :
        runtime_error(message)
    {
    }

#define LOAD_SYMBOL(x) x(reinterpret_cast<decltype(x)>(library.find_symbol(#x, true)))
#define LOAD_ALIASED_SYMBOL(x, y) x(reinterpret_cast<decltype(x)>(library.find_symbol(#x, true, #y)))
#define LOAD_OPTIONAL_SYMBOL(x) x(reinterpret_cast<decltype(x)>(library.find_symbol(#x)))

    api::api(lth_lib::dynamic_library library) :
        LOAD_SYMBOL(rb_intern),
        LOAD_SYMBOL(rb_const_get),
        LOAD_SYMBOL(rb_const_set),
        LOAD_SYMBOL(rb_const_remove),
        LOAD_SYMBOL(rb_const_defined),
        LOAD_SYMBOL(rb_define_module),
        LOAD_SYMBOL(rb_define_module_under),
        LOAD_SYMBOL(rb_define_class_under),
        LOAD_SYMBOL(rb_define_method),
        LOAD_SYMBOL(rb_define_singleton_method),
        LOAD_SYMBOL(rb_class_new_instance),
        LOAD_SYMBOL(rb_gv_get),
        LOAD_SYMBOL(rb_eval_string),
        LOAD_SYMBOL(rb_funcall),
        LOAD_ALIASED_SYMBOL(rb_funcallv, rb_funcall2),
        LOAD_SYMBOL(rb_proc_new),
        LOAD_SYMBOL(rb_block_call),
        LOAD_SYMBOL(rb_funcall_passing_block),
        LOAD_SYMBOL(rb_num2ull),
        LOAD_SYMBOL(rb_num2ll),
        LOAD_SYMBOL(rb_num2dbl),
        LOAD_SYMBOL(rb_string_value_ptr),
        LOAD_SYMBOL(rb_rescue2),
        LOAD_SYMBOL(rb_protect),
        LOAD_SYMBOL(rb_jump_tag),
        LOAD_SYMBOL(rb_int2inum),
        LOAD_SYMBOL(rb_ll2inum),
        LOAD_SYMBOL(rb_enc_str_new),
        LOAD_SYMBOL(rb_utf8_encoding),
        LOAD_SYMBOL(rb_str_encode),
        LOAD_SYMBOL(rb_load),
        LOAD_SYMBOL(rb_raise),
        LOAD_SYMBOL(rb_block_proc),
        LOAD_SYMBOL(rb_block_given_p),
        LOAD_SYMBOL(rb_gc_register_address),
        LOAD_SYMBOL(rb_gc_unregister_address),
        LOAD_SYMBOL(rb_hash_foreach),
        LOAD_SYMBOL(rb_define_attr),
        LOAD_SYMBOL(rb_ivar_set),
        LOAD_SYMBOL(rb_ivar_get),
        LOAD_ALIASED_SYMBOL(rb_float_new_in_heap, rb_float_new),
        LOAD_ALIASED_SYMBOL(rb_ary_new_capa, rb_ary_new2),
        LOAD_SYMBOL(rb_ary_push),
        LOAD_SYMBOL(rb_ary_entry),
        LOAD_SYMBOL(rb_hash_new),
        LOAD_SYMBOL(rb_hash_aset),
        LOAD_SYMBOL(rb_hash_lookup),
        LOAD_SYMBOL(rb_hash_lookup2),
        LOAD_SYMBOL(rb_sym_to_s),
        LOAD_SYMBOL(rb_to_id),
        LOAD_SYMBOL(rb_id2name),
        LOAD_SYMBOL(rb_define_alloc_func),
        LOAD_ALIASED_SYMBOL(rb_data_object_alloc, rb_data_object_wrap),
        LOAD_SYMBOL(rb_gc_mark),
        LOAD_SYMBOL(rb_yield_values),
        LOAD_SYMBOL(rb_require),
        LOAD_SYMBOL(rb_last_status_set),
        LOAD_SYMBOL(rb_cObject),
        LOAD_SYMBOL(rb_cArray),
        LOAD_SYMBOL(rb_cHash),
        LOAD_SYMBOL(rb_cString),
        LOAD_SYMBOL(rb_cSymbol),
        LOAD_SYMBOL(rb_cFloat),
        LOAD_SYMBOL(rb_cInteger),
        LOAD_SYMBOL(rb_eException),
        LOAD_SYMBOL(rb_eArgError),
        LOAD_SYMBOL(rb_eTypeError),
        LOAD_SYMBOL(rb_eStandardError),
        LOAD_SYMBOL(rb_eRuntimeError),
        LOAD_SYMBOL(rb_eLoadError),
        LOAD_OPTIONAL_SYMBOL(ruby_setup),
        LOAD_SYMBOL(ruby_init),
        LOAD_SYMBOL(ruby_options),
        LOAD_SYMBOL(ruby_cleanup),
        _library(move(library))
    {
    }

    api::~api()
    {
        uninitialize();
    }

    api& api::instance()
    {
        static api instance { create() };
        return instance;
    }

    lth_lib::dynamic_library api::create()
    {
        lth_lib::dynamic_library library = find_library();
        if (!library.loaded()) {
            throw library_not_loaded_exception(_("could not locate a ruby library"));
        } else if (library.first_load()) {
            LOG_INFO("ruby loaded from \"{1}\".", library.name());
        } else {
            LOG_INFO("ruby was already loaded.");
        }
        return library;
    }

    void api::initialize()
    {
        if (_initialized) {
            return;
        }

        // Prefer ruby_setup over ruby_init if present (2.0+)
        // If ruby is already initialized, this is a no-op
        if (ruby_setup) {
            ruby_setup();
        } else {
            ruby_init();
        }

        if (_library.first_load()) {
            // Run an empty script evaluation
            // ruby_options is a required call as it sets up some important stuff (unfortunately)
            char const* opts[] = {
                "ruby",
                "-e",
                ""
            };

            // Check for bundler; this is the only ruby option we support
            string ruby_opt;
            if (environment::get("RUBYOPT", ruby_opt) && boost::starts_with(ruby_opt, "-rbundler/setup")) {
                environment::set("RUBYOPT", "-rbundler/setup");
            } else {
                // Clear RUBYOPT so that only our options are used.
                environment::set("RUBYOPT", "");
            }

            ruby_options(sizeof(opts) / sizeof(opts[0]), const_cast<char**>(opts));
        }

        // Get the values for nil, true, and false
        // We do this because these are not constant across ruby versions
        _nil = rb_ivar_get(*rb_cObject, rb_intern("@expected_to_be_nil"));
        _true = rb_funcall(_nil, rb_intern("nil?"), 0);
        _false = rb_funcall(_true, rb_intern("nil?"), 0);

        // Delay logging until now; to_string depends on _nil.
        LOG_INFO("using ruby version {1}", to_string(rb_const_get(*rb_cObject, rb_intern("RUBY_VERSION"))));

        // Set SIGINT handling to system default
        // This prevents ruby from raising an interrupt exception.
        rb_funcall(*rb_cObject, rb_intern("trap"), 2, utf8_value("INT"), utf8_value("SYSTEM_DEFAULT"));

        _initialized = true;
    }

    bool api::initialized() const
    {
        return _initialized;
    }

    void api::uninitialize()
    {
        if (_initialized && _library.first_load()) {
            ruby_cleanup(0);
            _initialized = false;
        }

        // API is shutting down; free all remaining data objects
        // Destructors may unregister the data object, so increment the iterator before freeing
        for (auto it = _data_objects.begin(); it != _data_objects.end();) {
            auto data = reinterpret_cast<RData*>(*it);
            ++it;
            if (data->dfree) {
                data->dfree(data->data);
                data->dfree = nullptr;
                data->dmark = nullptr;
            }
        }
        _data_objects.clear();
    }

    bool api::include_stack_trace() const
    {
        return _include_stack_trace;
    }

    void api::include_stack_trace(bool value)
    {
        _include_stack_trace = value;
    }

    vector<string> api::get_load_path() const
    {
        vector<string> directories;

        array_for_each(rb_gv_get("$LOAD_PATH"), [&](VALUE value) {
            string path = to_string(value);
            // Ignore "." as a load path (present in 1.8.7)
            if (path == ".") {
                return false;
            }
            directories.emplace_back(move(path));
            return true;
        });

        return directories;
    }

    size_t api::num2size_t(VALUE v) const
    {
        auto size = rb_num2ull(v);
        if (size > numeric_limits<size_t>::max()) {
            throw invalid_conversion(_("size_t maximum exceeded, requested size was {1}", to_string(size)));
        }
        return static_cast<size_t>(size);
    }

    string api::to_string(VALUE v) const
    {
        v = rb_funcall(v, rb_intern("to_s"), 0);
        v = rb_str_encode(v, utf8_value("UTF-8"), 0, _nil);
        return string(rb_string_value_ptr(&v), num2size_t(rb_funcall(v, rb_intern("bytesize"), 0)));
    }

    VALUE api::to_symbol(string const& s) const
    {
        return rb_funcall(utf8_value(s), rb_intern("to_sym"), 0);
    }

    VALUE api::utf8_value(char const* s, size_t len) const
    {
        return rb_enc_str_new(s, len, rb_utf8_encoding());
    }

    VALUE api::utf8_value(char const* s) const
    {
        return utf8_value(s, strlen(s));
    }

    VALUE api::utf8_value(std::string const& s) const
    {
        return utf8_value(s.c_str(), s.size());
    }

    VALUE api::rescue(function<VALUE()> callback, function<VALUE(VALUE)> rescue) const
    {
        return rb_rescue2(
            RUBY_METHOD_FUNC(callback_thunk),
            reinterpret_cast<VALUE>(&callback),
            RUBY_METHOD_FUNC(rescue_thunk),
            reinterpret_cast<VALUE>(&rescue),
            *rb_eException,
            0);
    }

    VALUE api::protect(int& tag, function<VALUE()> callback) const
    {
        return rb_protect(
            callback_thunk,
            reinterpret_cast<VALUE>(&callback),
            &tag);
    }

    VALUE api::callback_thunk(VALUE parameter)
    {
        auto callback = reinterpret_cast<function<VALUE()>*>(parameter);
        return (*callback)();
    }

    VALUE api::rescue_thunk(VALUE parameter, VALUE exception)
    {
        auto rescue = reinterpret_cast<function<VALUE(VALUE)>*>(parameter);
        return (*rescue)(exception);
    }

    void api::array_for_each(VALUE array, std::function<bool(VALUE)> callback) const
    {
        long size = array_len(array);

        for (long i = 0; i < size; ++i) {
            if (!callback(rb_ary_entry(array, i))) {
                break;
            }
        }
    }

    void api::hash_for_each(VALUE hash, function<bool(VALUE, VALUE)> callback) const
    {
        rb_hash_foreach(hash, reinterpret_cast<int(*)(...)>(hash_for_each_thunk), reinterpret_cast<VALUE>(&callback));
    }

    int api::hash_for_each_thunk(VALUE key, VALUE value, VALUE arg)
    {
        auto callback = reinterpret_cast<function<bool(VALUE, VALUE)>*>(arg);
        return (*callback)(key, value) ? 0 /* continue */ : 1 /* stop */;
    }

    string api::exception_to_string(VALUE ex, string const& message) const
    {
        ostringstream result;

        if (message.empty()) {
            result << to_string(ex);
        } else {
            result << message;
        }

        if (_include_stack_trace) {
            result << "\nbacktrace:\n";

            // Append ex.backtrace.join('\n')
            result << to_string(rb_funcall(rb_funcall(ex, rb_intern("backtrace"), 0), rb_intern("join"), 1, utf8_value("\n")));
        }

        return result.str();
    }

    bool api::is_a(VALUE value, VALUE klass) const
    {
        return rb_funcall(value, rb_intern("is_a?"), 1, klass) != 0;
    }

    bool api::is_nil(VALUE value) const
    {
        return value == _nil;
    }

    bool api::is_true(VALUE value) const
    {
        return value == _true;
    }

    bool api::is_false(VALUE value) const
    {
        return value == _false;
    }

    bool api::is_hash(VALUE value) const
    {
        return is_a(value, *rb_cHash);
    }

    bool api::is_array(VALUE value) const
    {
        return is_a(value, *rb_cArray);
    }

    bool api::is_string(VALUE value) const
    {
        return is_a(value, *rb_cString);
    }

    bool api::is_symbol(VALUE value) const
    {
        return is_a(value, *rb_cSymbol);
    }

    bool api::is_integer(VALUE value) const
    {
        return is_a(value, *rb_cInteger);
    }

    bool api::is_float(VALUE value) const
    {
        return is_a(value, *rb_cFloat);
    }

    VALUE api::nil_value() const
    {
        return _nil;
    }

    VALUE api::true_value() const
    {
        return _true;
    }

    VALUE api::false_value() const
    {
        return _false;
    }

    long api::array_len(VALUE array) const
    {
        // This is used for rb_ary_entry, which only accepts a 'long'. So we only expect to
        // encounter long values here.
        auto size = rb_num2ull(rb_funcall(array, rb_intern("size"), 0));
        if (size > numeric_limits<long>::max()) {
            throw invalid_conversion(_("maximum array size exceeded, reported size was {1}", to_string(size)));
        }
        return static_cast<long>(size);
    }

    VALUE api::lookup(std::initializer_list<std::string> const& names) const
    {
        volatile VALUE current = *rb_cObject;

        for (auto const& name : names) {
            current = rb_const_get(current, rb_intern(name.c_str()));
        }
        return current;
    }

    bool api::equals(VALUE first, VALUE second) const
    {
        return is_true(rb_funcall(first, rb_intern("eql?"), 1, second));
    }

    bool api::case_equals(VALUE first, VALUE second) const
    {
        return is_true(rb_funcall(first, rb_intern("==="), 1, second));
    }

    VALUE api::eval(const string& code)
    {
        std::string exception;

        VALUE result = rescue(
                [&]() {
                    return rb_eval_string(code.c_str());
                },
                [&](VALUE exc) {
                    exception = exception_to_string(exc);
                    return nil_value();
                });

        if (!exception.empty()) {
            throw runtime_error(exception);
        }
        return result;
    }

    lth_lib::dynamic_library api::find_library() {
        // First search for an already loaded Ruby.
        auto library = find_loaded_library();
        if (library.loaded()) {
            return library;
        }

        if (!ruby_lib_location.empty()) {
            // Ruby lib location was specified by the user, fix to that.
            if (library.load(ruby_lib_location)) {
                return library;
            }
            LOG_WARNING("preferred ruby library \"{1}\" could not be loaded.", ruby_lib_location);
        }

        // Next try an environment variable.
        // This allows users to directly specify the ruby version to use.
        string value;
        if (environment::get("LEATHERMAN_RUBY", value)) {
            if (library.load(value)) {
                return library;
            } else {
                LOG_WARNING("ruby library \"{1}\" could not be loaded.", value);
            }
        }

        // Search the path for ruby.exe and query it for the location of its library.
        string ruby = execution::which("ruby");
        if (ruby.empty()) {
            LOG_DEBUG("ruby could not be found on the PATH.");
            return library;
        }
        LOG_DEBUG("ruby was found at \"{1}\".", ruby);

        auto exec = execute(ruby, { "-e", "print(['libdir', 'archlibdir', 'sitearchlibdir', 'bindir'].find do |name|"
                "dir = RbConfig::CONFIG[name];"
                "next unless dir;"
                "file = File.join(dir, RbConfig::CONFIG['LIBRUBY_SO']);"
                "break file if File.exist? file;"
                "false end)" });
        if (!exec.success) {
            LOG_WARNING("ruby failed to run: {1}", exec.output);
            return library;
        }

        boost::system::error_code ec;
        if (!exists(exec.output, ec) || is_directory(exec.output, ec)) {
            LOG_DEBUG("ruby library \"{1}\" was not found: ensure ruby was built with the --enable-shared configuration option.", exec.output);
            return library;
        }

        library.load(exec.output);
        return library;
    }

}}  // namespace leatherman::ruby
