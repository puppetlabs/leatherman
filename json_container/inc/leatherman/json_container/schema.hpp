#pragma once

#include <leatherman/json_container/json_container.hpp>

#include <map>
#include <string>
#include <set>


// boost forward declarations used in valijson forward declarations
namespace boost {
    template<typename K, typename T, typename C, typename CA, typename A>
    class ptr_map;
    struct heap_clone_allocator;
}

// valijson forward declarations
namespace valijson {
    class Schema;

    namespace constraints {
        struct TypeConstraint;
        typedef boost::ptr_map<std::string, Schema,
                               std::less<std::string>,
                               boost::heap_clone_allocator,
                               std::allocator< std::pair<const std::string,
                                                         void*>>> PropertySchemaMap;
        typedef std::set<std::string> RequiredProperties;
    }
}

namespace leatherman { namespace json_container {

namespace V_C = valijson::constraints;

enum class TypeConstraint { Object, Array, String, Int, Bool, Double, Null, Any };
enum class ContentType { Json, Binary };

class schema_error : public std::runtime_error  {
  public:
    explicit schema_error(std::string const& msg)
            : std::runtime_error(msg) {}
};

class Schema {
  public:
    Schema() = delete;

    // The following constructors instantiate an empty Schema with no
    // constraint

    Schema(std::string name,
           ContentType content_type,
           TypeConstraint type);

    Schema(std::string name,
           ContentType content_type);

    Schema(std::string name,
           TypeConstraint type);

    Schema(std::string name);

    Schema(const Schema& schema);

    // Instantiate a Schema of type ContentType::Json by parsing the
    // JSON schema passed as a JsonContainer object.
    // It won't be possible to add further constraints to such schema
    // Throw a schema_error in case of parsing failure.
    Schema(std::string name, const JsonContainer& json_schema);

    ~Schema();

    // Add constraints to a JSON object schema.
    // Throw a schema_error in case the Schema instance is not of
    // TypeConstraint::Object type or if it was constructed by parsing
    // a JSON schema.
    void addConstraint(std::string field, TypeConstraint type, bool required = false);
    void addConstraint(std::string field, Schema sub_schema, bool required = false);

    const std::string getName() const;
    ContentType getContentType() const;
    const valijson::Schema getRaw() const;

  private:
    std::string name_;

    // To distinguish between binary and JSON data
    const ContentType content_type_;

    // To add a global type constraint; default to Object (see ctors)
    const TypeConstraint type_;

    // Stores the schema parsed by the parsing ctor
    const std::unique_ptr<valijson::Schema> parsed_json_schema_;

    // Flag; set in case the used ctor was the parsing one
    const bool parsed_;

    // Store constraints for validjson
    std::unique_ptr<V_C::PropertySchemaMap> properties_;
    std::unique_ptr<V_C::PropertySchemaMap> pattern_properties_;
    std::unique_ptr<V_C::RequiredProperties> required_properties_;

    // Convert our TypeConstraint to the validjson ones
    V_C::TypeConstraint getConstraint(TypeConstraint type) const;

    // Check if it's possible to add constraints
    void checkAddConstraint();
};

}}  // namespace leatherman::json_container
