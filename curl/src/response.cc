#include <leatherman/curl/response.hpp>

using namespace std;

namespace leatherman { namespace curl {

    Response::Response() :
        _status_code(0)
    {
    }

    void Response::add_header(string name, string value)
    {
        _headers.emplace(make_pair(move(name), move(value)));
    }

    void Response::each_header(function<bool(string const&, string const&)> callback) const
    {
        if (!callback) {
            return;
        }
        for (auto const& kvp : _headers) {
            if (!callback(kvp.first, kvp.second)) {
                return;
            }
        }
    }

    string* Response::header(string const& name)
    {
        auto header = _headers.find(name);
        if (header == _headers.end()) {
            return nullptr;
        }
        return &header->second;
    }

    void Response::remove_header(string const& name)
    {
        _headers.erase(name);
    }

    void Response::body(string body)
    {
        _body = move(body);
    }

    string const& Response::body() const
    {
        return _body;
    }

    int Response::status_code() const
    {
        return _status_code;
    }

    void Response::status_code(int status)
    {
        _status_code = status;
    }
}}  // leatherman::curl
