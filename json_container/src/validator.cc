#include <leatherman/json_container/validator.hpp>
#include <leatherman/locale/locale.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#pragma GCC diagnostic ignored "-Wreorder"
#include <leatherman/json_container/rapidjson_adapter.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/validation_results.hpp>
#include <valijson/validator.hpp>
#pragma GCC diagnostic pop

namespace leatherman { namespace json_container {

///
/// Auxiliary functions
///

static std::string getValidationError(valijson::ValidationResults& validation_results) {
    std::string err_msg {};
    valijson::ValidationResults::Error error;
    unsigned int err_idx { 0 };
    static std::string err_label { locale::translate("ERROR") };

    while (validation_results.popError(error)) {
        if (!err_msg.empty()) {
            err_msg += "  - ";
        }
        err_idx++;
        err_msg += err_label + std::to_string(err_idx) + ":";
        for (const auto& context_element : error.context) {
            err_msg += " " + context_element;
        }
    }

    return  err_msg;
}

static void validateJsonContainer(const JsonContainer& data,
                                  const Schema& schema,
                                  const std::string& schema_name) {
    valijson::Validator validator { schema.getRaw() };
    valijson::adapters::RapidJsonAdapter adapted_document { data.getRaw() };
    valijson::ValidationResults validation_results;

    auto success = validator.validate(adapted_document, &validation_results);

    if (!success) {
        auto err_msg = getValidationError(validation_results);
        throw validation_error {
            locale::format("'{1}' does not match schema: {2}", schema_name, err_msg) };
    }
}

///
/// Public API
///

Validator::Validator()
        : schema_map_ {},
          lookup_mutex_ {} {
}

Validator::Validator(Validator&& other_validator)
        : schema_map_ { other_validator.schema_map_ },
          lookup_mutex_ {} {
}

void Validator::registerSchema(const Schema& schema) {
    util::lock_guard<util::mutex> lock(lookup_mutex_);
    auto schema_name = schema.getName();
    if (includesSchema(schema_name)) {
        throw schema_redefinition_error {
            locale::format("schema '{1}' already defined", schema_name) };
    }

    auto p = std::pair<std::string, Schema>(schema_name, schema);
    schema_map_.insert(p);
}

void Validator::validate(const JsonContainer& data,
                         std::string schema_name) const {
    util::unique_lock<util::mutex> lock(lookup_mutex_);
    if (!includesSchema(schema_name)) {
        throw schema_not_found_error {
            locale::format("'{1}' is not a registered schema", schema_name) };
    }
    lock.unlock();

    // we can freely unlock. When a schema has been set it cannot be modified

    validateJsonContainer(data, schema_map_.at(schema_name), schema_name);
}

bool Validator::includesSchema(std::string schema_name) const {
    return schema_map_.find(schema_name) != schema_map_.end();
}

ContentType Validator::getSchemaContentType(std::string schema_name) const {
    util::unique_lock<util::mutex> lock(lookup_mutex_);
    if (!includesSchema(schema_name)) {
        throw schema_not_found_error {
            locale::format("'{1}' is not a registered schema", schema_name) };
    }
    lock.unlock();

    return schema_map_.at(schema_name).getContentType();
}

}}  // namespace leatherman::json_container
