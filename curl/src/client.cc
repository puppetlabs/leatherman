#include <leatherman/curl/client.hpp>
#include <leatherman/curl/request.hpp>
#include <leatherman/curl/response.hpp>
#include <leatherman/util/regex.hpp>
#include <leatherman/file_util/file.hpp>
#include <leatherman/logging/logging.hpp>
#include <boost/utility/string_ref.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/nowide/iostream.hpp>
#include <boost/nowide/fstream.hpp>
#include <sstream>

// Mark string for translation (alias for leatherman::locale::format)
using leatherman::locale::_;

using namespace std;
namespace fs = boost::filesystem;

namespace leatherman { namespace curl {

    // Helper for globally initializing curl
    struct curl_init_helper
    {
        curl_init_helper()
        {
            _result = curl_global_init(CURL_GLOBAL_DEFAULT);
        }

        ~curl_init_helper()
        {
            if (_result == CURLE_OK) {
                curl_global_cleanup();
            }
        }

        CURLcode result() const
        {
            return _result;
        }

     private:
        CURLcode _result;
    };

    curl_handle::curl_handle() :
        scoped_resource(nullptr, cleanup)
    {
        // Perform initialization
        static curl_init_helper init_helper;
        if (init_helper.result() != CURLE_OK) {
            throw http_exception(curl_easy_strerror(init_helper.result()));
        }

        _resource = curl_easy_init();
    }

    void curl_handle::cleanup(CURL* curl)
    {
        if (curl) {
            curl_easy_cleanup(curl);
        }
    }

    curl_list::curl_list() :
        scoped_resource(nullptr, cleanup)
    {
    }

    void curl_list::append(string const& value)
    {
        _resource = curl_slist_append(_resource, value.c_str());
    }

    void curl_list::cleanup(curl_slist* list)
    {
        if (list) {
            curl_slist_free_all(list);
        }
    }

    curl_escaped_string::curl_escaped_string(curl_handle const& handle, string const& str) :
        scoped_resource(nullptr, cleanup)
    {
        _resource = curl_easy_escape(handle, str.c_str(), str.size());
        if (!_resource) {
            throw http_exception(_("curl_easy_escape failed to escape string."));
        }
    }

    void curl_escaped_string::cleanup(char const* str)
    {
        if (str) {
            curl_free(const_cast<char*>(str));
        }
    }

    static std::string make_file_err_msg(std::string const& reason) {
        return _("File operation error: {1}", reason);
    }

    download_temp_file::download_temp_file(request const& req, std::string const& file_path, boost::optional<boost::filesystem::perms> perms) :
      _req(req),
      _file_path(file_path)
    {
        try {
            _temp_path = fs::path(file_path).parent_path() / fs::unique_path("temp_file_%%%%-%%%%-%%%%-%%%%");
            _fp = boost::nowide::fopen(_temp_path.string().c_str(), "wb");
            if (!_fp) {
                throw http_file_operation_exception(_req, _file_path, make_file_err_msg(_("failed to open temporary file for writing")));
            }
            if (!perms) {
                return;
            }

            boost::system::error_code ec;
            fs::permissions(_temp_path.string(), *perms, ec);
            if (ec) {
                cleanup();
                throw http_file_operation_exception(_req, _file_path, make_file_err_msg(_("failed to modify permissions of temporary file")));
            }
        } catch (fs::filesystem_error& e) {
            throw http_file_operation_exception(_req, _file_path, make_file_err_msg(e.what()));
        }
    }

    download_temp_file::~download_temp_file() {
        cleanup();
    }

    FILE* download_temp_file::get_fp() {
        return _fp;
    }

    void download_temp_file::write() {
        LOG_DEBUG("Download completed, now writing result to file {1}", _file_path);
        close_fp();
        boost::system::error_code ec;
        fs::rename(_temp_path, _file_path, ec);
        if (ec) {
            LOG_WARNING("Failed to write the results of the temporary file to the actual file {1}", _file_path);
            throw http_file_operation_exception(_req, _file_path, make_file_err_msg(_("failed to move over the temporary file's downloaded contents")));
        }
    }

    void download_temp_file::write(response& res) {
        LOG_DEBUG("Writing the temp file's contents to the response body");
        close_fp();
        string res_body;
        if (!leatherman::file_util::read(_temp_path.string(), res_body)) {
            LOG_WARNING("Failed to write the contents of the temporary file to the response body.");
            throw http_file_operation_exception(_req, _file_path, make_file_err_msg(_("failed to write the temporary file's contents to the response body")));
        }
        res.body(res_body);
    }

    void download_temp_file::close_fp() {
      fclose(_fp);
      _fp = nullptr;
    }

    void download_temp_file::cleanup() {
        if (_fp) {
            fclose(_fp);
        }
        boost::system::error_code ec;
        fs::remove(_temp_path, ec);
        if (ec) {
            LOG_WARNING("Failed to properly clean-up the temporary file {1}", _temp_path);
        }
    }

    client::client()
    {
        if (!_handle) {
            throw http_exception(_("failed to create cURL handle."));
        }
    }

    client::client(client && other)
    {
        *this = move(other);
    }

    client &client::operator=(client && other)
    {
        _handle = move(other._handle);
        return *this;
    }

    response client::get(request const& req)
    {
        return perform(http_method::get, req);
    }

    response client::post(request const& req)
    {
        return perform(http_method::post, req);
    }

    response client::put(request const& req)
    {
        return perform(http_method::put, req);
    }

    response client::perform(http_method method, request const& req)
    {
        response res;
        context ctx(req, res);

        // Reset the options
        curl_easy_reset(_handle);

        // Set common options
        curl_easy_setopt_maybe(ctx, CURLOPT_NOPROGRESS, 1);
        curl_easy_setopt_maybe(ctx, CURLOPT_FOLLOWLOCATION, 1);

        // Set tracing from libcurl if enabled (we don't care if this fails)
        if (LOG_IS_DEBUG_ENABLED()) {
            curl_easy_setopt(_handle, CURLOPT_DEBUGFUNCTION, debug);
            curl_easy_setopt(_handle, CURLOPT_VERBOSE, 1);
        }

        // Setup the request
        set_method(ctx, method);
        set_url(ctx);
        set_headers(ctx);
        set_cookies(ctx);
        set_body(ctx, method);
        set_timeouts(ctx);
        set_write_callbacks(ctx);
        set_ca_info(ctx);
        set_client_info(ctx);
        set_client_protocols(ctx);

        // Perform the request
        auto result = curl_easy_perform(_handle);
        if (result != CURLE_OK) {
            throw http_request_exception(req, curl_easy_strerror(result));
        }

        LOG_DEBUG("request completed (status {1}).", res.status_code());

        // Set the body of the response
        res.body(move(ctx.response_buffer));
        return res;
    }

    void client::download_file(request const& req, std::string const& file_path, boost::optional<fs::perms> perms)
    {
        download_file_helper(req, file_path, {}, perms);
    }

    void client::download_file(request const& req, std::string const& file_path, response& res, boost::optional<fs::perms> perms)
    {
        download_file_helper(req, file_path, res, perms);
    }

    void client::download_file_helper(request const& req, std::string const& file_path, boost::optional<response&> res, boost::optional<fs::perms> perms)
    {
        response _res;
        context ctx(req, _res);

        // Reset the options
        curl_easy_reset(_handle);

        char errbuf[CURL_ERROR_SIZE] = { '\0' };
        download_temp_file temp_file(req, file_path, perms);
        curl_easy_setopt_maybe(ctx, CURLOPT_NOPROGRESS, 1);

        // Setup the remaining request
        set_url(ctx);
        set_headers(ctx);
        set_timeouts(ctx);
        set_write_callbacks(ctx, temp_file.get_fp());
        set_ca_info(ctx);
        set_client_info(ctx);
        set_client_protocols(ctx);

        // More detailed error messages
        curl_easy_setopt_maybe(ctx, CURLOPT_ERRORBUFFER, errbuf);

        // Perform the request
        auto result = curl_easy_perform(_handle);
        if (result == CURLE_WRITE_ERROR) {
            throw http_file_operation_exception(req, file_path, make_file_err_msg(_("failed to write to the temporary file during download")));
        } else if (result != CURLE_OK) {
            throw http_file_download_exception(req, file_path, _("File download server side error: {1}", errbuf));
        }

        // Check the status code. If 400+, fill in the response
        LOG_DEBUG("request completed (status {1}).", _res.status_code());
        if (_res.status_code() >= 400 && res) {
            temp_file.write(_res);
        } else {
            temp_file.write();
        }

        if (res) {
            (*res) = move(_res);
        }
    }

    void client::set_ca_cert(string const& cert_file)
    {
        _ca_cert = cert_file;
    }

    void client::set_client_cert(string const& client_cert, string const& client_key)
    {
        _client_cert = client_cert;
        _client_key = client_key;
    }

    void client::set_supported_protocols(long client_protocols) {
        _client_protocols = client_protocols;
    }

    void client::set_method(context& ctx, http_method method)
    {
        switch (method) {
            case http_method::get:
                // Unnecessary since we're resetting the handle before calling this function
                return;

            case http_method::post: {
                curl_easy_setopt_maybe(ctx, CURLOPT_POST, 1);
                break;
            }

            case http_method::put: {
                curl_easy_setopt_maybe(ctx, CURLOPT_UPLOAD, 1);
                break;
            }

            default:
                throw http_request_exception(ctx.req, _("unexpected HTTP method specified."));
        }
    }

    void client::set_url(context& ctx)
    {
        // TODO: support an easy interface for setting escaped query parameters
        curl_easy_setopt_maybe(ctx, CURLOPT_URL, ctx.req.url().c_str());
        LOG_DEBUG("requesting {1}.", ctx.req.url());
    }

    void client::set_headers(context& ctx)
    {
        ctx.req.each_header([&](string const& name, string const& value) {
            ctx.request_headers.append(name + ": " + value);
            return true;
        });
        curl_easy_setopt_maybe(ctx, CURLOPT_HTTPHEADER, static_cast<curl_slist*>(ctx.request_headers));
    }

    void client::set_cookies(context& ctx)
    {
        ostringstream cookies;
        ctx.req.each_cookie([&](string const& name, string const& value) {
            if (cookies.tellp() != 0) {
                cookies << "; ";
            }
            cookies << name << "=" << value;
            return true;
        });
        curl_easy_setopt_maybe(ctx, CURLOPT_COOKIE, cookies.str().c_str());
    }

    void client::set_body(context& ctx, http_method method)
    {
        curl_easy_setopt_maybe(ctx, CURLOPT_READFUNCTION, read_body);
        curl_easy_setopt_maybe(ctx, CURLOPT_READDATA, &ctx);
        curl_easy_setopt_maybe(ctx, CURLOPT_SEEKFUNCTION, seek_body);
        curl_easy_setopt_maybe(ctx, CURLOPT_SEEKDATA, &ctx);

        switch (method) {
            case http_method::post: {
                curl_easy_setopt_maybe(ctx, CURLOPT_POSTFIELDSIZE_LARGE, ctx.req.body().size());
                break;
            }
            case http_method::put: {
                curl_easy_setopt_maybe(ctx, CURLOPT_INFILESIZE_LARGE, ctx.req.body().size());
                break;
            }
            default:
                break;
        }
    }

    void client::set_timeouts(context& ctx)
    {
        curl_easy_setopt_maybe(ctx, CURLOPT_CONNECTTIMEOUT_MS, ctx.req.connection_timeout());
        curl_easy_setopt_maybe(ctx, CURLOPT_TIMEOUT_MS, ctx.req.timeout());
    }

    void client::set_header_write_callbacks(context& ctx)
    {
        curl_easy_setopt_maybe(ctx, CURLOPT_HEADERFUNCTION, write_header);
        curl_easy_setopt_maybe(ctx, CURLOPT_HEADERDATA, &ctx);
    }

    void client::set_write_callbacks(context& ctx)
    {
        set_header_write_callbacks(ctx);
        curl_easy_setopt_maybe(ctx, CURLOPT_WRITEFUNCTION, write_body);
        curl_easy_setopt_maybe(ctx, CURLOPT_WRITEDATA, &ctx);
    }

    void client::set_write_callbacks(context& ctx, FILE* fp)
    {
        set_header_write_callbacks(ctx);
        curl_easy_setopt_maybe(ctx, CURLOPT_WRITEFUNCTION, write_file);
        curl_easy_setopt_maybe(ctx, CURLOPT_WRITEDATA, fp);
    }

    void client::set_ca_info(context& ctx){
        if (_ca_cert == "") {
            return;
        }

        curl_easy_setopt_maybe(ctx, CURLOPT_CAINFO, _ca_cert.c_str());
    }

    void client::set_client_info(context &ctx) {
        if (_client_cert == "" || _client_key == "") {
          return;
        }

        curl_easy_setopt_maybe(ctx, CURLOPT_SSLCERT, _client_cert.c_str());
        curl_easy_setopt_maybe(ctx, CURLOPT_SSLKEY, _client_key.c_str());
    }

    void client::set_client_protocols(context& ctx) {
        curl_easy_setopt_maybe(ctx, CURLOPT_PROTOCOLS, _client_protocols);
    }

    size_t client::read_body(char* buffer, size_t size, size_t count, void* ptr)
    {
        auto ctx = reinterpret_cast<context*>(ptr);
        size_t requested = size * count;

        auto const& body = ctx->req.body();

        if (requested > (body.size() - ctx->read_offset)) {
            requested = (body.size() - ctx->read_offset);
        }
        if (requested > 0) {
            memcpy(buffer, body.c_str() + ctx->read_offset, requested);
            ctx->read_offset += requested;
        }
        return requested;
    }

    int client::seek_body(void* ptr, curl_off_t offset, int origin)
    {
        auto ctx = reinterpret_cast<context*>(ptr);

        // Only setting offset from the beginning is supported and the CURL docs
        // claim this is the only way this gets called
        if (origin != SEEK_SET) {
            return CURL_SEEKFUNC_FAIL;
        }

        // Since we only support an absolute offset, we should not support
        // negative offsets to prevent reading data from before the buffer
        if (offset < 0) {
            return CURL_SEEKFUNC_FAIL;
        }

        ctx->read_offset = offset;
        return CURL_SEEKFUNC_OK;
    }

    size_t client::write_header(char* buffer, size_t size, size_t count, void* ptr)
    {
        size_t written = size * count;
        boost::string_ref input(buffer, written);

        auto ctx = reinterpret_cast<context*>(ptr);

        // If the header starts with "HTTP/", then we have the response status
        if (input.starts_with("HTTP/")) {
            // Reset the response buffer
            ctx->response_buffer.clear();

            // Parse out the error code
            static boost::regex regex("HTTP/\\d\\.\\d (\\d\\d\\d).*");
            int status_code = 0;
            if (util::re_search(input.to_string(), regex, &status_code)) {
                ctx->res.status_code(status_code);
            }
            return written;
        } else if (input == "\r\n") {
            // Ignore the response delimiter
            return written;
        }

        auto pos = input.find_first_of(':');
        if (pos == boost::string_ref::npos) {
            LOG_WARNING("unexpected HTTP response header: {1}.", input);
            return written;
        }

        auto name = input.substr(0, pos).to_string();
        auto value = input.substr(pos + 1).to_string();
        boost::trim(name);
        boost::trim(value);

        // If this is the "Content-Length" header, reserve the response buffer as an optimization
        if (name == "Content-Length") {
            try {
                ctx->response_buffer.reserve(stoi(value));
            } catch (logic_error&) {
            }
        }

        ctx->res.add_header(move(name), move(value));
        return written;
    }

    size_t client::write_body(char* buffer, size_t size, size_t count, void* ptr)
    {
        size_t written = size * count;

        auto ctx = reinterpret_cast<context*>(ptr);
        if (written > 0) {
            ctx->response_buffer.append(buffer, written);
        }

        return written;
    }

    size_t client::write_file(char *buffer, size_t size, size_t count, void* ptr)
    {
        return fwrite(buffer, size, count, reinterpret_cast<FILE*>(ptr));
    }

    int client::debug(CURL* handle, curl_infotype type, char* data, size_t size, void* ptr)
    {
        if (type > CURLINFO_DATA_OUT) {
            return 0;
        }
        string str(data, size);
        boost::trim(str);
        if (str.empty()) {
            return 0;
        }

        // Only log cURL's text to debug
        if (type == CURLINFO_TEXT) {
            LOG_DEBUG(str);
            return 0;
        } else if (!LOG_IS_TRACE_ENABLED()) {
            return 0;
        }

        ostringstream header;
        if (type == CURLINFO_HEADER_IN) {
            header << "[response headers: " << size << " bytes]\n";
        } else if (type == CURLINFO_HEADER_OUT) {
            header << "[request headers: " << size << " bytes]\n";
        } else if (type == CURLINFO_DATA_IN) {
            header << "[response body: " << size << " bytes]\n";
        } else if (type == CURLINFO_DATA_OUT) {
            header << "[request body: " << size << " bytes]\n";
        }
        LOG_TRACE("{1}{2}", header.str(), str);
        return 0;
    }

    curl_handle const& client::get_handle()
    {
        return _handle;
    }
}}  // leatherman::curl
