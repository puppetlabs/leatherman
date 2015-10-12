/**
 * @file
 * Declares the logging functions and macros.
 */
#pragma once

// To use this header, you must:
// - Have Boost on the include path
// - Link in Boost.Log
// - Configure Boost.Log at runtime before any logging takes place
/**
 * See Boost.Log's documentation.
 */
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/format.hpp>
#include <cstdio>
#include <functional>

/**
 * Defines the logging namespace.
 */
#ifndef LEATHERMAN_LOGGING_NAMESPACE
#error "LEATHERMAN_LOGGING_NAMESPACE must be set. This is typically done via CMake."
#else
#define LOG_NAMESPACE LEATHERMAN_LOGGING_NAMESPACE
#endif


/**
 * Logs a message.
 * @param level The logging level for the message.
 * @param line_num The source line number of the logging call.
 * @param format The format message.
 * @param ... The format message parameters.
 */
#ifdef LEATHERMAN_LOGGING_LINE_NUMBERS
#define LOG_MESSAGE(level, line_num, format, ...) \
    if (leatherman::logging::is_enabled(level)) { \
        leatherman::logging::log(LOG_NAMESPACE, level, line_num, format, ##__VA_ARGS__); \
    }
#else
#define LOG_MESSAGE(level, line_num, format, ...) \
    if (leatherman::logging::is_enabled(level)) { \
        leatherman::logging::log(LOG_NAMESPACE, level, 0, format, ##__VA_ARGS__); \
    }
#endif
/**
 * Logs a trace message.
 * @param format The format message.
 * @param ... The format message parameters.
 */
#define LOG_TRACE(format, ...) LOG_MESSAGE(leatherman::logging::log_level::trace, __LINE__, format, ##__VA_ARGS__)
/**
 * Logs a debug message.
 * @param format The format message.
 * @param ... The format message parameters.
 */
#define LOG_DEBUG(format, ...) LOG_MESSAGE(leatherman::logging::log_level::debug, __LINE__, format, ##__VA_ARGS__)
/**
 * Logs an info message.
 * @param format The format message.
 * @param ... The format message parameters.
 */
#define LOG_INFO(format, ...) LOG_MESSAGE(leatherman::logging::log_level::info, __LINE__, format, ##__VA_ARGS__)
/**
 * Logs a warning message.
 * @param format The format message.
 * @param ... The format message parameters.
 */
#define LOG_WARNING(format, ...) LOG_MESSAGE(leatherman::logging::log_level::warning, __LINE__, format, ##__VA_ARGS__)
/**
 * Logs an error message.
 * @param format The format message.
 * @param ... The format message parameters.
 */
#define LOG_ERROR(format, ...) LOG_MESSAGE(leatherman::logging::log_level::error, __LINE__, format, ##__VA_ARGS__)
/**
 * Logs a fatal message.
 * @param format The format message.
 * @param ... The format message parameters.
 */
#define LOG_FATAL(format, ...) LOG_MESSAGE(leatherman::logging::log_level::fatal, __LINE__, format, ##__VA_ARGS__)
/**
 * Determines if the trace logging level is enabled.
 * @returns Returns true if trace logging is enabled or false if it is not enabled.
 */
#define LOG_IS_TRACE_ENABLED() leatherman::logging::is_enabled(leatherman::logging::log_level::trace)
/**
 * Determines if the debug logging level is enabled.
 * @returns Returns true if debug logging is enabled or false if it is not enabled.
 */
#define LOG_IS_DEBUG_ENABLED() leatherman::logging::is_enabled(leatherman::logging::log_level::debug)
/**
 * Determines if the info logging level is enabled.
 * @returns Returns true if info logging is enabled or false if it is not enabled.
 */
#define LOG_IS_INFO_ENABLED() leatherman::logging::is_enabled(leatherman::logging::log_level::info)
/**
 * Determines if the warning logging level is enabled.
 * @returns Returns true if warning logging is enabled or false if it is not enabled.
 */
#define LOG_IS_WARNING_ENABLED() leatherman::logging::is_enabled(leatherman::logging::log_level::warning)
/**
 * Determines if the error logging level is enabled.
 * @returns Returns true if error logging is enabled or false if it is not enabled.
 */
#define LOG_IS_ERROR_ENABLED() leatherman::logging::is_enabled(leatherman::logging::log_level::error)
/**
 * Determines if the fatal logging level is enabled.
 * @returns Returns true if fatal logging is enabled or false if it is not enabled.
 */
#define LOG_IS_FATAL_ENABLED() leatherman::logging::is_enabled(leatherman::logging::log_level::fatal)

namespace leatherman { namespace logging {

    /**
     * Represents the supported logging levels.
     */
    enum class log_level
    {
        none,
        trace,
        debug,
        info,
        warning,
        error,
        fatal
    };

    /**
     * Reads a log level from an input stream.
     * This is used in boost::lexical_cast<log_level>.
     * @param in The input stream.
     * @param level The returned log level.
     * @returns Returns the input stream.
     */
    std::istream& operator>>(std::istream& in, log_level& level);

    /**
     * Produces the printed representation of logging level.
     * @param strm The stream to write.
     * @param level The logging level to print.
     * @return Returns the stream after writing to it.
     */
    std::ostream& operator<<(std::ostream& strm, log_level level);

    /**
     * Sets up logging for the given stream.
     * The logging level is set to warning by default.
     * @param dst Destination stream for logging output.
     * @param locale The locale identifier to use for logging.
     */
    void setup_logging(std::ostream &dst, std::string locale = "");

    /**
     * Sets up logging; it will write records to the specified file.
     * Log file rotation will be configured by file size as follows.
     * The logging level is set to warning by default.
     * @param file_name File name pattern and path; ex. '/var/log/my_app_%3N.log'.
     * @param open_mode File open mode; ex. 'std::ios_base::app'.
     * @param target Path of the directory where rotated files will be stored.
     * @param rotation_size Max file size, after which it rotates (num chars).
     * @param max_size Max size for all rotated files (in bytes).
     * @param min_free_space Min drive space, to prevent logging (in bytes).
     * @param locale The locale identifier to use for logging.
     */
    void setup_logging(std::string file_name,
                       std::ios_base::openmode open_mode,
                       std::string target,
                       int rotation_size,
                       int max_size,
                       int min_free_space,
                       std::string locale = "");

    /**
     * Sets the current log level.
     * @param level The new current log level to set.
     */
    void set_level(log_level level);

    /**
     * Gets the current log level.
     * @return Returns the current log level.
     */
    log_level get_level();

    /**
     * Sets whether or not log output is colorized.
     * If logging was configured to file, colorization will be ignored on Windows.
     * @param color Pass true if log output is colorized or false if it is not colorized.
     */
    void set_colorization(bool color);

    /**
     * Gets whether or not the log output is colorized.
     * @return Returns true if log output is colorized or false if it is not colorized.
     */
    bool get_colorization();

    /**
     * Provides a callback for when a message is logged.
     * If the callback returns false, the message will not be logged.
     * @param callback The callback to call when a message is about to be logged.
     */
    void on_message(std::function<bool(log_level, std::string const&)> callback);

    /**
     * Determines if the given log level is enabled for the given logger.
     * @param level The logging level to check.
     * @return Returns true if the logging level is enabled or false if it is not.
     */
    bool is_enabled(log_level level);

    /**
     * Determine if an error has been logged
     * @return Returns true if an error or critical message has been logged
     */
    bool error_has_been_logged();

    /**
     * Clear the flag that indicates an error has been logged.
     * This is necessary for testing the flagging functionality. This function should
     * not be used by library consumers.
     */
    void clear_error_logged_flag();

    /**
     * Logs a given message to the given logger with the specified line number (if > 0).
     * @param logger The logger to log the message to.
     * @param level The logging level to log with.
     * @param line_num The source line number of the logging call.
     * @param message The message to log.
     */
    void log(const std::string &logger, log_level level, int line_num, std::string const& message);

    /**
     * Logs a given format message to the given logger with the specified line number (if > 0).
     * @param logger The logger to log the message to.
     * @param level The logging level to log with.
     * @param line_num The source line number of the logging call.
     * @param message The message being formatted.
     */
    void log(const std::string &logger, log_level level, int line_num, boost::format& message);

    /**
     * Logs a given format message to the given logger with the specified line number (if > 0).
     * @tparam T The type of the first argument.
     * @tparam TArgs The types of the remaining arguments.
     * @param logger The logger to log to.
     * @param level The logging level to log with.
     * @param line_num The source line number of the logging call.
     * @param message The message being formatted.
     * @param arg The first argument to the message.
     * @param args The remaining arguments to the message.
     */
    template <typename T, typename... TArgs>
    void log(const std::string &logger, log_level level, int line_num, boost::format& message, T arg, TArgs... args)
    {
        message % arg;
        log(logger, level, line_num, message, std::forward<TArgs>(args)...);
    }

    /**
     * Logs a given format message to the given logger with the specified line number (if > 0).
     * @tparam TArgs The types of the arguments to format the message with.
     * @param logger The logger to log to.
     * @param level The logging level to log with.
     * @param line_num The source line number of the logging call.
     * @param format The message format.
     * @param args The remaining arguments to the message.
     */
    template <typename... TArgs>
    void log(const std::string &logger, log_level level, int line_num, std::string const& format, TArgs... args)
    {
        boost::format message(format);
        log(logger, level, line_num, message, std::forward<TArgs>(args)...);
    }

    /**
     * Starts colorizing the output stream for the given log level.
     * This is a no-op on platforms that don't natively support terminal colors.
     * @param dst The stream to colorize.
     * @param level The log level to colorize for. Defaults to none, which resets colorization.
     */
    void colorize(std::ostream &dst, log_level level = log_level::none);

    /**
     * Starts colorizing the formatting output stream for the given log level.
     * This is a no-op on platforms that don't natively support terminal colors.
     * @param strm The formatting output stream stream to colorize.
     * @param level The log level to colorize for. Defaults to none, which resets colorization.
     */
    void colorize(boost::log::formatting_ostream& strm, log_level level = log_level::none);

    /**
     * Returns whether terminal colors are supported.
     * @param dst The stream to check.
     * @return True if terminal colors are supported for the specified stream on this platform, else false.
     */
    bool color_supported(std::ostream &dst);

}}  // namespace leatherman::logging
