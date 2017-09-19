#include <leatherman/util/uri.hpp>
#include <algorithm>
#include <sstream>

namespace leatherman { namespace util {
    uri::uri(std::string const& _uri)
    {
        if (_uri.length() == 0) {
            return;
        }

        auto uri_end = _uri.end();

        // get query start
        auto query_start = std::find(_uri.begin(), uri_end, '?');

        // protocol
        auto protocol_start = _uri.begin();
        auto protocol_end = std::find(protocol_start, uri_end, ':');

        if (protocol_end != uri_end) {
            std::string after_prot{protocol_end, uri_end};
            if ((after_prot.length() > 3) && (after_prot.substr(0, 3) == "://")) {
                protocol = std::string(protocol_start, protocol_end);
                protocol_end += 3;
            } else {
                protocol_end = _uri.begin();  // no protocol
            }
        } else {
            protocol_end = _uri.begin();  // no protocol
        }

        // host
        auto host_start = protocol_end;
        auto path_start = std::find(host_start, uri_end, '/');

        auto host_end = std::find(protocol_end,
                                  (path_start != uri_end) ? path_start : query_start,
                                  ':');  // check for port

        host = std::string(host_start, host_end);

        // port
        if ((host_end != uri_end) && (*host_end == ':')) {
            auto port_end = (path_start != uri_end) ? path_start : query_start;
            port = std::string(host_end+1, port_end);
        }

        // path
        if (path_start != uri_end) {
            path = std::string(path_start, query_start);
        }

        // query
        if (query_start != uri_end) {
            query = std::string(query_start, uri_end);
        }
    }

    std::string uri::str() const
    {
        std::stringstream ss;
        if (!protocol.empty()) {
            ss << protocol << "://";
        }

        ss << host;
        if (!port.empty()) {
            ss << ":" << port;
        }

        ss << path << query;
        return ss.str();
    }
}}  // namespace leatherman::util
