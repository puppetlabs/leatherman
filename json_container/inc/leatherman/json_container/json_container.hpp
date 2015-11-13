#pragma once

#include <vector>
#include <cstdarg>
#include <iostream>
#include <tuple>
#include <typeinfo>
#include <memory>

// Forward declarations for rapidjson
namespace rapidjson {
    class CrtAllocator;
    template <typename Encoding, typename Allocator> class GenericValue;
    template <typename CharType> struct UTF8;
    template <typename Encoding, typename Allocator, typename StackAllocator> class GenericDocument;
}  // namespace rapidjson

namespace leatherman { namespace json_container {
    // Errors

    /// Parent error class.
    class data_error : public std::runtime_error  {
    public:
        explicit data_error(std::string const& msg) : std::runtime_error(msg) {}
    };

    /// Error thrown when trying to parse an invalid JSON string.
    class data_parse_error : public data_error  {
    public:
        explicit data_parse_error(std::string const& msg) : data_error(msg) {}
    };

    /// Error due to an operation involving a key.
    class data_key_error : public data_error  {
    public:
        explicit data_key_error(std::string const& msg) : data_error(msg) {}
    };

    /// Error due to an operation involving an array index.
    class data_index_error : public data_error  {
    public:
        explicit data_index_error(std::string const& msg) : data_error(msg) {}
    };

    /// Error due to wrongly specified type.
    class data_type_error : public data_error  {
    public:
        explicit data_type_error(std::string const& msg) : data_error(msg) {}
    };

    // Types

    enum DataType { Object, Array, String, Int, Bool, Double, Null };

    struct JsonContainerKey : public std::string {
        JsonContainerKey(const std::string& value) : std::string(value) {}
        JsonContainerKey(const char* value) : std::string(value) {}
        JsonContainerKey(std::initializer_list<char> il) = delete;
    };

    /**
     * Typedef for RapidJSON allocator.
     */
    using json_allocator = rapidjson::CrtAllocator;
    /**
     * Typedef for RapidJSON value.
     */
    using json_value = rapidjson::GenericValue<rapidjson::UTF8<char>, json_allocator>;
    /**
     * Typedef for RapidJSON document.
     */
    using json_document = rapidjson::GenericDocument<rapidjson::UTF8<char>, json_allocator, json_allocator>;

    // Usage:
    //
    // SUPPORTED SCALARS:
    //    int, float, double, bool, std::string, nullptr
    //
    // To set a key to a scalar value in object x
    //    x.set<int>("foo", 1);
    //    x.set<std::string>(foo", "bar");
    //
    // To set a nested key to a scalar value in object x
    //    x.set<bool>({ "foo", "bar", "baz" }, true);
    //
    // To set a key to a vector value in object x
    //    std::vector<int> tmp { 1, 2, 3 };
    //    x.set<std::vector<int>>("foo", tmp);
    //
    // To get a scalar value from a key in object x
    //    x.get<std::string>("foo");
    //    x.get<int>("bar");
    //
    // To get a vector from a key in object x
    //    x.get<std::vector<float>>("foo");
    //
    // To get the int entry with index i from the array a in object x
    //    x.get<int>("a", 1);
    //
    // To get a result object (json object) from object x
    //    x.get<Data>("foo");
    //
    // To get a null value from a key in object x
    //    x.get<std::string>("foo") == "";
    //    x.get<int>("foo") == 0;
    //    x.get<bool>("foo") == false;
    //    x.get<float>("foo") == 0.0f;
    //    x.get<double>("foo") == 0.0;
    //
    // To get a json string representation of object x
    //    x.toString();
    //
    // To check if a key is set in object x
    //    x.includes("foo");
    //    x.includes({ "foo", "bar", "baz" });

    class JsonContainer {
    public:
        JsonContainer();
        explicit JsonContainer(const std::string& json_txt);
        explicit JsonContainer(const json_value& value);
        JsonContainer(const JsonContainer& data);
        JsonContainer(const JsonContainer&& data);
        JsonContainer& operator=(JsonContainer other);

        ~JsonContainer();

        const json_document& getRaw() const;

        std::string toString() const;

        /// Throw a data_key_error in case the specified key is unknown.
        std::string toString(const JsonContainerKey& key) const;

        /// Throw a data_key_error in case the specified key is unknown.
        std::string toString(const std::vector<JsonContainerKey>& keys) const;

        std::string toPrettyString(size_t left_padding) const;
        std::string toPrettyString() const;

        /// Return true if the root is an empty JSON array or an empty
        /// JSON object, false otherwise.
        bool empty() const;

        /// Return the number of entries of the root element in case
        /// is an object or array; returns 0 in case of a scalar
        size_t size() const;

        /// Return the number of entries of the specified element;
        /// returns 0 in case it's scalar
        /// Throw a data_key_error in case the specified key is unknown.
        size_t size(const JsonContainerKey& key) const;

        /// Return the number of entries of the specified element;
        /// return 0 in case it's scalar
        /// Throw a data_key_error in case of unknown keys.
        size_t size(const std::vector<JsonContainerKey>& keys) const;

        /// In case the root entry is an object, returns its keys,
        /// otherwise an empty vector.
        std::vector<std::string> keys() const;

        /// Whether the specified entry exists.
        bool includes(const JsonContainerKey& key) const;

        /// Whether the specified entry exists.
        bool includes(const std::vector<JsonContainerKey>& keys) const;

        DataType type() const;

        /// Throw a data_key_error in case the specified key is unknown.
        DataType type(const JsonContainerKey& key) const;

        /// Throw a data_key_error in case of unknown keys.
        DataType type(const std::vector<JsonContainerKey>& keys) const;

        /// Throw a data_type_error in case the root entry is not an array.
        /// Throw a data_index_error in case the index is out of bounds.
        DataType type(const size_t idx) const;

        /// Throw a data_key_error in case the specified key is unknown.
        /// Throw a data_type_error in case the specified entry is not an array.
        /// Throw a data_index_error in case the index is out of bound.
        DataType type(const JsonContainerKey& key, const size_t idx) const;

        /// Throw a data_key_error in case of unknown keys.
        /// Throw a data_type_error in case the specified entry is not an array.
        /// Throw a data_index_error in case the index is out of bound.
        DataType type(const std::vector<JsonContainerKey>& keys, const size_t idx) const;

        /// Return the value of the root entry.
        /// Throw a data_type_error in case the type of the root entry
        /// does not match the specified one.
        template <typename T>
        T get() const {
            return getValue<T>(*getValueInJson());
        }

        /// Return the value of the specified entry of the root object.
        /// Throw a data_key_error in case the entry does not exist.
        /// Throw a data_type_error in case the type T doesn't match
        /// the specified one.
        template <typename T>
        T get(const JsonContainerKey& key) const {
            return getValue<T>(*getValueInJson(std::vector<JsonContainerKey> { key }));
        }

        /// Return the value of the specified nested entry.
        /// Throw a data_key_error in case the entry does not exist.
        /// Throw a data_type_error in case the type T doesn't match
        /// the specified one.
        template <typename T>
        T get(std::vector<JsonContainerKey> keys) const {
            return getValue<T>(*getValueInJson(keys));
        }

        /// Return the indexed value of root array.
        /// Throw a data_index_error in case the index is out of bound.
        /// Throw a data_type_error in case the type T doesn't match
        /// the one of the specified value or if the root entry is not
        /// an array.
        template <typename T>
        T get(const size_t idx) const {
            return getValue<T>(*getValueInJson(std::vector<JsonContainerKey> {},
                                               true, idx));
        }

        /// Return the indexed value of the specified array entry.
        /// Throw a data_key_error in case the array entry is unknown.
        /// Throw a data_index_error in case the index is out of bound.
        /// Throw a data_type_error in case the type T doesn't match
        /// the one of the specified entry or in case the specified
        /// entry is not an array.
        template <typename T>
        T get(const JsonContainerKey& key, const size_t idx) const {
            return getValue<T>(*getValueInJson(std::vector<JsonContainerKey> { key },
                                               true, idx));
        }

        /// Return the indexed value of the specified nested array
        /// entry.
        /// Throw a data_key_error in case the array entry is unknown.
        /// Throw a data_index_error in case the index is out of bound.
        /// Throw a data_type_error in case the type T doesn't match
        /// the one of the specified entry or in case the specified
        /// entry is not an array.
        template <typename T>
        T get(std::vector<JsonContainerKey> keys, const size_t idx) const {
            return getValue<T>(*getValueInJson(keys, true, idx));
        }

        /// Return the value of the specified entry of the root object,
        /// or default_value if the entry doesn't exist.
        /// Throw a data_type_error in case the type T doesn't match
        /// the specified one or if the root entry is not an object.
        template <typename T>
        T getWithDefault(const JsonContainerKey& key, const T default_value) const {
            auto jval = getValueInJson();
            auto key_data = key.data();

            if (!isObject(*jval)) {
                throw data_type_error { "not an object" };
            }

            if (!hasKey(*jval, key_data)) {
                return default_value;
            }

            return getValue<T>(*getValueInJson(*jval, key_data));
        }

        /// Return the value of the specified nested entry or
        /// default_value if the entry doesn't exist but its parent is
        /// an object.
        /// Throw a data_type_error in case the type T doesn't match
        /// the specified one or in case the parent of the specified
        /// entry is not an object.
        template <typename T>
        T getWithDefault(const std::vector<JsonContainerKey>& keys, const T& default_value) const {
            auto key_data = keys.back().data();
            auto jval_obj = getValueInJson(keys.cbegin(), keys.cend()-1);

            if (!isObject(*jval_obj)) {
                throw data_type_error { "not an object" };
            }

            if (!hasKey(*jval_obj, key_data)) {
                return default_value;
            }

            return getValue<T>(*getValueInJson(*jval_obj, key_data));
        }

        /// Throw a data_key_error in case the root is not a valid JSON
        /// object, so that is not possible to set the entry.
        template <typename T>
        void set(const JsonContainerKey& key, T value) {
            auto jval = getValueInJson();
            auto key_data = key.data();

            if (!isObject(*jval)) {
                throw data_key_error { "root is not a valid JSON object" };
            }

            if (!hasKey(*jval, key_data)) {
                createKeyInJson(key_data, *jval);
            }

            setValue<T>(*getValueInJson(*jval, key_data), value);
        }

        /// Throw a data_key_error if a known nested key is not associated
        /// with a valid JSON object, so that it is not  possible to
        /// iterate the remaining keys.
        template <typename T>
        void set(std::vector<JsonContainerKey> keys, T value) {
            auto jval = getValueInJson();

            for (const auto& key : keys) {
                const char* key_data = key.data();

                if (!isObject(*jval)) {
                    throw data_key_error { "invalid key supplied; cannot "
                                           "navigate the provided path" };
                }

                if (!hasKey(*jval, key_data)) {
                    createKeyInJson(key_data, *jval);
                }

                jval = getValueInJson(*jval, key_data);
            }

            setValue<T>(*jval, value);
        }

    private:
        std::unique_ptr<json_document> document_root_;

        size_t getSize(const json_value& jval) const;

        DataType getValueType(const json_value& jval) const;

        bool hasKey(const json_value& jval, const char* key) const;

        // NOTE(ale): we cant' use json_value::IsObject directly
        // since we have forward declarations for rapidjson; otherwise
        // we would have an implicit template instantiation error
        bool isObject(const json_value& jval) const;

        // Root object entry accessor
        // Throws a data_type_error in case the specified value is not
        // an object.
        // Throws a data_key_error or if the key is unknown.
        json_value* getValueInJson(const json_value& jval,
                                   const char* key) const;

        // Root array entry accessor
        // Throws a data_type_error in case the specified value is not
        // an array.
        // Throws a data_index_error in case the arraye index is out
        // of bounds.
        json_value* getValueInJson(const json_value& jval,
                                   const size_t& idx) const;

        // Generic entry accessor
        // In case any key is specified, throws a data_type_error if
        // the specified entry is not an object; throws a
        // data_key_error or if the key is unknown.
        // In case an array element is specified, throws a
        // data_index_error if the index is out of bounds.
        json_value* getValueInJson(
            std::vector<JsonContainerKey>::const_iterator begin,
            std::vector<JsonContainerKey>::const_iterator end,
            const bool is_array = false,
            const size_t idx = 0) const;

        // Generic entry accessor
        // In case any key is specified, throws a data_type_error if
        // the specified entry is not an object; throws a
        // data_key_error or if the key is unknown.
        // In case an array element is specified, throws a
        // data_index_error if the index is out of bounds.
        json_value* getValueInJson(
            const std::vector<JsonContainerKey>& keys = std::vector<JsonContainerKey> {},
            const bool is_array = false,
            const size_t idx = 0) const {
            return getValueInJson(keys.cbegin(), keys.cend(), is_array, idx);
        }

        void createKeyInJson(const char* key, json_value& jval);

        template<typename T>
        T getValue(const json_value& value) const;

        template<typename T>
        void setValue(json_value& jval, T new_value);
    };

    template<>
    void JsonContainer::setValue<>(json_value& jval, const std::string& new_value);

    template<>
    void JsonContainer::setValue<>(json_value& jval, const char* new_value);

    template<>
    void JsonContainer::setValue<>(json_value& jval, bool new_value);

    template<>
    void JsonContainer::setValue<>(json_value& jval, int new_value);

    template<>
    void JsonContainer::setValue<>(json_value& jval, double new_value);

    template<>
    void JsonContainer::setValue<>(json_value& jval, std::vector<std::string> new_value);

    template<>
    void JsonContainer::setValue<>(json_value& jval, std::vector<bool> new_value);

    template<>
    void JsonContainer::setValue<>(json_value& jval, std::vector<int> new_value);

    template<>
    void JsonContainer::setValue<>(json_value& jval, std::vector<double> new_value);

    template<>
    void JsonContainer::setValue<>(json_value& jval, std::vector<JsonContainer> new_value);

    template<>
    void JsonContainer::setValue<>(json_value& jval, JsonContainer new_value);

}}  // namespace leatherman::json_container
