#include <leatherman/json_container/json_container.hpp>
#include <leatherman/locale/locale.hpp>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/writer.h>
#include <rapidjson/allocators.h>
#include <rapidjson/rapidjson.h>

// Mark string for translation (alias for leatherman::locale::format)
using leatherman::locale::_;

namespace leatherman { namespace json_container {

    const size_t LEFT_PADDING_INCREMENT { 2 };

    //
    // free functions
    //

    std::string valueToString(const json_value& jval) {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer { buffer };
        jval.Accept(writer);
        return buffer.GetString();
    }

    //
    // public interface
    //

    JsonContainer::JsonContainer() : document_root_ { new json_document() } {
        document_root_->SetObject();
    }

    JsonContainer::JsonContainer(const std::string& json_text) : JsonContainer() {
        document_root_->Parse(json_text.data());

        if (document_root_->HasParseError()) {
            throw data_parse_error { _("invalid json") };
        }
    }

    JsonContainer::JsonContainer(const json_value& value) : JsonContainer() {
        // Because rapidjson disallows the use of copy constructors we pass
        // the json by const reference and recreate it by explicitly copying
        document_root_->CopyFrom(value, document_root_->GetAllocator());
    }

    JsonContainer::JsonContainer(const JsonContainer& data) : JsonContainer(){
        document_root_->CopyFrom(*data.document_root_, document_root_->GetAllocator());
    }

    JsonContainer::JsonContainer(const JsonContainer&& data) : JsonContainer() {
        document_root_->CopyFrom(*data.document_root_, document_root_->GetAllocator());
    }

    JsonContainer& JsonContainer::operator=(JsonContainer other) {
        std::swap(document_root_, other.document_root_);
        return *this;
    }

    // unique_ptr requires a complete type at time of destruction. this forces us to
    // either have an empty destructor or use a shared_ptr instead.
    JsonContainer::~JsonContainer() {}

    // representation

    const json_document& JsonContainer::getRaw() const {
        return *document_root_;
    }

    std::string JsonContainer::toString() const {
        return valueToString(*document_root_);
    }

    std::string JsonContainer::toString(const JsonContainerKey& key) const {
        auto jval = getValueInJson({ key });
        return valueToString(*jval);
    }

    std::string JsonContainer::toString(const std::vector<JsonContainerKey>& keys) const {
        auto jval = getValueInJson(keys);
        return valueToString(*jval);
    }

    std::string JsonContainer::toPrettyString(size_t left_padding) const {
        if (empty()) {
            switch (type()) {
                case DataType::Object:
                    return "{}";
                case DataType::Array:
                    return "[]";
                default:
                    return "\"\"";
            }
        }

        std::string formatted_data {};

        if (type() == DataType::Object) {
            for (const auto& key : keys()) {
                formatted_data += std::string(left_padding, ' ');
                formatted_data += key + " : ";
                switch (type(key)) {
                    case DataType::Object:
                        // Inner object: add new line, increment padding
                        formatted_data += "\n";
                        formatted_data += get<JsonContainer>(key).toPrettyString(
                                            left_padding + LEFT_PADDING_INCREMENT);
                        break;
                    case DataType::Array:
                        // Array: add raw string, regardless of its items
                        formatted_data += toString(key);
                        break;
                    case DataType::String:
                        formatted_data += get<std::string>(key);
                        break;
                    case DataType::Int:
                        formatted_data += std::to_string(get<int>(key));
                        break;
                    case DataType::Bool:
                        if (get<bool>(key)) {
                            formatted_data += "true";
                        } else {
                            formatted_data += "false";
                        }
                        break;
                    case DataType::Double:
                        formatted_data += std::to_string(get<double>(key));
                        break;
                    default:
                        formatted_data += "NULL";
                }
                formatted_data += "\n";
            }
        } else {
            formatted_data += toString();
        }

        return formatted_data;
    }

    std::string JsonContainer::toPrettyString() const {
        return toPrettyString(DEFAULT_LEFT_PADDING);
    }

    std::string JsonContainer::toPrettyJson(size_t left_padding) const {
        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer { buffer };
        writer.SetIndent(' ', left_padding);
        auto& jval = *getValueInJson();
        jval.Accept(writer);
        return buffer.GetString();
    }

    // capacity

    bool JsonContainer::empty() const {
        auto jval = getValueInJson();
        auto data_type = getValueType(*jval);

        if (data_type == DataType::Object) {
            return jval->ObjectEmpty();
        } else if (data_type == DataType::Array) {
            return jval->Empty();
        } else {
            return false;
        }
    }

    size_t JsonContainer::size() const {
        auto jval = getValueInJson();
        return getSize(*jval);
    }

    size_t JsonContainer::size(const JsonContainerKey& key) const {
        auto jval = getValueInJson({ key });
        return getSize(*jval);
    }

    size_t JsonContainer::size(const std::vector<JsonContainerKey>& keys) const {
        auto jval = getValueInJson(keys);
        return getSize(*jval);
    }

    // keys

    std::vector<std::string> JsonContainer::keys() const {
        std::vector<std::string> k;
        auto jval = getValueInJson();

        if (jval->IsObject()) {
            for (json_value::ConstMemberIterator itr = jval->MemberBegin();
                 itr != jval->MemberEnd(); ++itr) {
                k.emplace_back(itr->name.GetString(), itr->name.GetStringLength());
            }
        }

        // Return an empty vector if the document type isn't an object
        return k;
    }

    // includes

    bool JsonContainer::includes(const JsonContainerKey& key) const {
        auto jval = getValueInJson();

        if (hasKey(*jval, key.data())) {
            return true;
        } else {
            return false;
        }
    }

    bool JsonContainer::includes(const std::vector<JsonContainerKey>& keys) const {
        auto jval = getValueInJson();

        for (const auto& key : keys) {
            if (!hasKey(*jval, key.data())) {
                return false;
            }
            jval = getValueInJson(*jval, key.data());
        }

        return true;
    }

    // type

    DataType JsonContainer::type() const {
        auto jval = getValueInJson();
        return getValueType(*jval);
    }

    DataType JsonContainer::type(const JsonContainerKey& key) const {
        auto jval = getValueInJson({ key });
        return getValueType(*jval);
    }

    DataType JsonContainer::type(const std::vector<JsonContainerKey>& keys) const {
        auto jval = getValueInJson(keys);
        return getValueType(*jval);
    }

    DataType JsonContainer::type(const size_t idx) const {
        auto jval = getValueInJson(std::vector<JsonContainerKey> {}, true, idx);
        return getValueType(*jval);
    }

    DataType JsonContainer::type(const JsonContainerKey& key, const size_t idx) const {
        auto jval = getValueInJson({ key }, true, idx);
        return getValueType(*jval);
    }

    DataType JsonContainer::type(const std::vector<JsonContainerKey>& keys,
                                 const size_t idx) const {
        auto jval = getValueInJson(keys, true, idx);
        return getValueType(*jval);
    }

    //
    // Private functions
    //

    size_t JsonContainer::getSize(const json_value& jval) const {
        switch (getValueType(jval)) {
            case DataType::Array:
                return jval.Size();
            case DataType::Object:
                return jval.MemberCount();
            default:
                return 0;
        }
    }

    DataType JsonContainer::getValueType(const json_value& jval) const {
        switch (jval.GetType()) {
            case rapidjson::Type::kNullType:
                return DataType::Null;
            case rapidjson::Type::kFalseType:
                return DataType::Bool;
            case rapidjson::Type::kTrueType:
                return DataType::Bool;
            case rapidjson::Type::kObjectType:
                return DataType::Object;
            case rapidjson::Type::kArrayType:
                return DataType::Array;
            case rapidjson::Type::kStringType:
                return DataType::String;
            case rapidjson::Type::kNumberType:
                if (jval.IsDouble()) {
                    return DataType::Double;
                } else {
                    return DataType::Int;
                }
            default:
                // This is unexpected as for rapidjson docs
                return DataType::Null;
        }
    }

    // Internal key / index manipulation methods

    bool JsonContainer::hasKey(const json_value& jval, const char* key) const {
        return (jval.IsObject() && jval.HasMember(key));
    }

    bool JsonContainer::isObject(const json_value& jval) const {
        return jval.IsObject();
    }

    json_value* JsonContainer::getValueInJson(const json_value& jval,
                                                    const char* key) const {
        if (!jval.IsObject()) {
            throw data_type_error { _("not an object") };
        }

        if (!jval.HasMember(key)) {
            throw data_key_error { _("unknown object entry with key: {1}", key) };
        }

        return const_cast<json_value*>(&jval[key]);
    }

    json_value* JsonContainer::getValueInJson(const json_value& jval,
                                                    const size_t& idx) const {
        if (getValueType(jval) != DataType::Array) {
            throw data_type_error { _("not an array") };
        }

        if (idx >= jval.Size()) {
            throw data_index_error { _("array index out of bounds") };
        }

        return const_cast<json_value*>(&jval[idx]);
    }

    json_value* JsonContainer::getValueInJson(std::vector<JsonContainerKey>::const_iterator begin,
                                              std::vector<JsonContainerKey>::const_iterator end,
                                                    const bool is_array,
                                                    const size_t idx) const {
        auto jval = dynamic_cast<json_value*>(document_root_.get());

        for (auto it = begin; it != end; ++it) {
            jval = getValueInJson(*jval, it->data());
        }

        if (is_array) {
            jval = getValueInJson(*jval, idx);
        }

        return jval;
    }

    void JsonContainer::createKeyInJson(const char* key,
                                        json_value& jval) {
        jval.AddMember(json_value(key, document_root_->GetAllocator()).Move(),
                       json_value(rapidjson::kObjectType).Move(),
                       document_root_->GetAllocator());
    }

    // getValue specialisations

    template<>
    int JsonContainer::getValue<>(const json_value& value) const {
        if (value.IsNull()) {
            return 0;
        }

        if (!value.IsInt()) {
            throw data_type_error { _("not an integer") };
        }

        return value.GetInt();
    }

    template<>
    bool JsonContainer::getValue<>(const json_value& value) const {
        if (value.IsNull()) {
            return false;
        }

        if (!value.IsBool()) {
            throw data_type_error { _("not a boolean") };
        }

        return value.GetBool();
    }

    template<>
    std::string JsonContainer::getValue<>(const json_value& value) const {
        if (value.IsNull()) {
            return "";
        }

        if (!value.IsString()) {
            throw data_type_error { _("not a string") };
        }

        return std::string(value.GetString(), value.GetStringLength());
    }

    template<>
    double JsonContainer::getValue<>(const json_value& value) const {
        if (value.IsNull()) {
            return 0.0;
        }

        if (!value.IsDouble()) {
            throw data_type_error { _("not a double") };
        }

        return value.GetDouble();
    }

    template<>
    JsonContainer JsonContainer::getValue<>(const json_value& value) const {
        if (value.IsNull()) {
            JsonContainer container {};
            return container;
        }

        // HERE(ale): we don't do any type check

        // rvalue return (implicitly)
        JsonContainer container { value };
        return container;
    }

    template<>
    json_value JsonContainer::getValue<>(const json_value& value) const {
        JsonContainer* tmp_this = const_cast<JsonContainer*>(this);
        json_value v { value, tmp_this->document_root_->GetAllocator() };
        return v;
    }

    template<>
    std::vector<std::string> JsonContainer::getValue<>(const json_value& value) const {
        std::vector<std::string> tmp {};

        if (value.IsNull()) {
            return tmp;
        }

        if (!value.IsArray()) {
            throw data_type_error { _("not an array") };
        }

        for (json_value::ConstValueIterator itr = value.Begin();
             itr != value.End();
             itr++) {
            if (!itr->IsString()) {
                throw data_type_error { _("not a string") };
            }

            tmp.emplace_back(itr->GetString(), itr->GetStringLength());
        }

        return tmp;
    }

    template<>
    std::vector<bool> JsonContainer::getValue<>(const json_value& value) const {
        std::vector<bool> tmp {};

        if (value.IsNull()) {
            return tmp;
        }

        if (!value.IsArray()) {
            throw data_type_error { _("not an array") };
        }

        for (json_value::ConstValueIterator itr = value.Begin();
             itr != value.End();
             itr++) {
            if (!itr->IsBool()) {
                throw data_type_error { _("not a boolean") };
            }

            tmp.push_back(itr->GetBool());
        }

        return tmp;
    }

    template<>
    std::vector<int> JsonContainer::getValue<>(const json_value& value) const {
        std::vector<int> tmp {};

        if (value.IsNull()) {
            return tmp;
        }

        if (!value.IsArray()) {
            throw data_type_error { _("not an array") };
        }

        for (json_value::ConstValueIterator itr = value.Begin();
             itr != value.End();
             itr++) {
            if (!itr->IsInt()) {
                throw data_type_error { _("not an integer") };
            }

            tmp.push_back(itr->GetInt());
        }

        return tmp;
    }

    template<>
    std::vector<double> JsonContainer::getValue<>(const json_value& value) const {
        std::vector<double> tmp {};

        if (value.IsNull()) {
            return tmp;
        }

        if (!value.IsArray()) {
            throw data_type_error { _("not an array") };
        }

        for (json_value::ConstValueIterator itr = value.Begin();
             itr != value.End();
             itr++) {
            if (!itr->IsDouble()) {
                throw data_type_error { _("not a double") };
            }

            tmp.push_back(itr->GetDouble());
        }

        return tmp;
    }

    template<>
    std::vector<JsonContainer> JsonContainer::getValue<>(const json_value& value) const {
        std::vector<JsonContainer> tmp {};

        if (value.IsNull()) {
            return tmp;
        }

        if (!value.IsArray()) {
            throw data_type_error { _("not an array") };
        }

        for (json_value::ConstValueIterator itr = value.Begin();
             itr != value.End();
             itr++) {
            if (!itr->IsObject()) {
                throw data_type_error { _("not an object") };
            }

            JsonContainer* tmp_this = const_cast<JsonContainer*>(this);
            const json_value tmpvalue(*itr, tmp_this->document_root_->GetAllocator());
            JsonContainer tmp_data { tmpvalue };
            tmp.push_back(tmp_data);
        }

        return tmp;
    }

    // setValue specialisations

    template<>
    void JsonContainer::setValue<>(json_value& jval, bool new_value) {
        jval.SetBool(new_value);
    }

    template<>
    void JsonContainer::setValue<>(json_value& jval, int new_value) {
        jval.SetInt(new_value);
    }

    template<>
    void JsonContainer::setValue<>(json_value& jval, const std::string new_value) {
        jval.SetString(new_value.data(), new_value.size(), document_root_->GetAllocator());
    }

    template<>
    void JsonContainer::setValue<>(json_value& jval, const char * new_value) {
        jval.SetString(new_value, std::string(new_value).size(), document_root_->GetAllocator());
    }

    template<>
    void JsonContainer::setValue<>(json_value& jval, double new_value) {
        jval.SetDouble(new_value);
    }

    template<>
    void JsonContainer::setValue<>(json_value& jval, std::vector<std::string> new_value ) {
        jval.SetArray();

        for (const auto& value : new_value) {
            // rapidjson doesn't like std::string...
            json_value s;
            s.SetString(value.data(), value.size(), document_root_->GetAllocator());
            jval.PushBack(s, document_root_->GetAllocator());
        }
    }

    template<>
    void JsonContainer::setValue<>(json_value& jval, std::vector<bool> new_value ) {
        jval.SetArray();

        for (const auto& value : new_value) {
            json_value tmp_val;
            tmp_val.SetBool(value);
            jval.PushBack(tmp_val, document_root_->GetAllocator());
        }
    }

    template<>
    void JsonContainer::setValue<>(json_value& jval, std::vector<int> new_value ) {
        jval.SetArray();

        for (const auto& value : new_value) {
            json_value tmp_val;
            tmp_val.SetInt(value);
            jval.PushBack(tmp_val, document_root_->GetAllocator());
        }
    }

    template<>
    void JsonContainer::setValue<>(json_value& jval, std::vector<double> new_value ) {
        jval.SetArray();

        for (const auto& value : new_value) {
            json_value tmp_val;
            tmp_val.SetDouble(value);
            jval.PushBack(tmp_val, document_root_->GetAllocator());
        }
    }

    template<>
    void JsonContainer::setValue<>(json_value& jval, std::vector<JsonContainer> new_value ) {
        jval.SetArray();

        for (auto value : new_value) {
            json_document tmp_value;
            tmp_value.CopyFrom(*value.document_root_, document_root_->GetAllocator());
            jval.PushBack(tmp_value, document_root_->GetAllocator());
        }
    }

    template<>
    void JsonContainer::setValue<>(json_value& jval, JsonContainer new_value ) {
        jval.CopyFrom(new_value.getRaw(), document_root_->GetAllocator());
    }

}}  // namespace leatherman::json_container
