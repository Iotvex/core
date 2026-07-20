#include "log.h"

#include "esp_log.h"

#include <atomic>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <sys/time.h>
#include <time.h>

// Implementation of Iotvex::Core::Log. See log.h for public API.

namespace Iotvex::Core::Log {

namespace {

// Printed line counter. Incremented only when a line is actually output.
std::atomic<uint32_t> g_log_counter{0};

// Copies of boot-time values from set_node_name() / set_node_id().
char g_node_name[48] = {};
char g_node_id[48] = {};

constexpr size_t k_line_size = 512;
constexpr size_t k_timestamp_size = 32;
constexpr size_t k_tag_size = 48;
constexpr size_t k_message_size = 256;

// Maps ESP-IDF level to the string shown in [LEVEL].
const char *level_name(esp_log_level_t level)
{
    switch (level) {
    case ESP_LOG_VERBOSE:
        return "VERBOSE";
    case ESP_LOG_DEBUG:
        return "DEBUG";
    case ESP_LOG_WARN:
        return "WARN";
    case ESP_LOG_ERROR:
        return "ERROR";
    default:
        return "INFO";
    }
}

// Writes current UTC time as ISO 8601 with milliseconds, e.g.
// 2026-07-11T15:30:22.123Z Without SNTP, tv_sec stays near epoch (1970-01-01).
void format_timestamp_iso8601(char *out, size_t out_size)
{
    struct timeval tv {};
    gettimeofday(&tv, nullptr);

    struct tm tm_utc {};
    gmtime_r(&tv.tv_sec, &tm_utc);

    snprintf(out, out_size, "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
             tm_utc.tm_year + 1900, tm_utc.tm_mon + 1, tm_utc.tm_mday,
             tm_utc.tm_hour, tm_utc.tm_min, tm_utc.tm_sec,
             static_cast<int>(tv.tv_usec / 1000));
}

// Core output path for all log levels.
// 1. Build tag "iotvex.<channel>"
// 2. Drop if above esp_log_level_get(tag)
// 3. Format message via vsnprintf
// 4. Assemble full line and emit via esp_log_write (IDF lock / sinks)
void write(esp_log_level_t level, const char *channel, const char *fmt,
           va_list args)
{
    if (channel == nullptr || channel[0] == '\0' || fmt == nullptr ||
        fmt[0] == '\0') {
        return;
    }

    char tag[k_tag_size];
    snprintf(tag, sizeof(tag), "iotvex.%s", channel);

    if (level > esp_log_level_get(tag)) {
        return;
    }

    char timestamp[k_timestamp_size];
    format_timestamp_iso8601(timestamp, sizeof(timestamp));

    char message[k_message_size];
    vsnprintf(message, sizeof(message), fmt, args);
    message[sizeof(message) - 1] = '\0';

    const char *node_name = g_node_name[0] != '\0' ? g_node_name : "<unset>";
    const char *node_id = g_node_id[0] != '\0' ? g_node_id : "<unset>";
    const uint32_t seq =
        g_log_counter.fetch_add(1, std::memory_order_relaxed) + 1;

    char line[k_line_size];
    snprintf(line, sizeof(line), "(%u) [%s] [%s] [%s] %s: %s (node-id:%s)", seq,
             timestamp, level_name(level), node_name, tag, message, node_id);
    line[sizeof(line) - 1] = '\0';

    // Custom line body; IDF still serializes sinks / locks around the write.
    esp_log_write(level, tag, "%s\n", line);
}

} // namespace

void set_node_name(const char *name)
{
    if (name == nullptr) {
        g_node_name[0] = '\0';
        return;
    }

    // Copy into owned buffer; source string may be temporary.
    strncpy(g_node_name, name, sizeof(g_node_name) - 1);
    g_node_name[sizeof(g_node_name) - 1] = '\0';
}

void set_node_id(const char *id)
{
    if (id == nullptr) {
        g_node_id[0] = '\0';
        return;
    }

    strncpy(g_node_id, id, sizeof(g_node_id) - 1);
    g_node_id[sizeof(g_node_id) - 1] = '\0';
}

void info(const char *channel, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    write(ESP_LOG_INFO, channel, fmt, args);
    va_end(args);
}

void debug(const char *channel, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    write(ESP_LOG_DEBUG, channel, fmt, args);
    va_end(args);
}

void warn(const char *channel, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    write(ESP_LOG_WARN, channel, fmt, args);
    va_end(args);
}

void error(const char *channel, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    write(ESP_LOG_ERROR, channel, fmt, args);
    va_end(args);
}

} // namespace Iotvex::Core::Log
