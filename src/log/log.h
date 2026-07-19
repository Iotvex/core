#pragma once

// Structured logging for Iotvex ESP-IDF libraries.
//
// Output (single line):
//   (seq) [UTC] [LEVEL] [name] iotvex.<channel>: message (node-id:id)
//
// Example:
//   (42) [2026-07-11T15:30:22.123Z] [INFO] [c6-kitchen]
//       iotvex.node.lifecycle: started (node-id:22323)
//
// Fields:
//   seq      - auto-incrementing log counter (atomic)
//   UTC      - ISO 8601 timestamp (needs SNTP for a real clock)
//   LEVEL    - DEBUG, INFO, WARN, or ERROR
//   name     - set_node_name(), e.g. c6-kitchen
//   channel  - module path without "iotvex." prefix
//   message  - printf-style fmt and arguments
//   node-id  - set_node_id()
//
// Boot setup (call once before other tasks log):
//   Iotvex::Core::Log::set_node_name("c6-kitchen");
//   Iotvex::Core::Log::set_node_id("22323");
//
// Usage:
//   IX_LOGI("node.lifecycle", "started");
//   IX_LOGE("light.drivers.pixel", "init failed on pin %d", pin);
//
// Channel examples: node.lifecycle, light.devices, light.drivers.pixel
// ESP-IDF filter tag: iotvex.<channel>
//
// Only IX_LOG* / debug/info/warn/error use this format.
// Raw ESP_LOGI() output is unchanged.
//
// Missing set_node_name() or set_node_id() prints "<unset>".

#if defined(__GNUC__)
#define IX_LOG_PRINTF(fmt_idx, args_idx)                                       \
    __attribute__((format(printf, fmt_idx, args_idx)))
#else
#define IX_LOG_PRINTF(fmt_idx, args_idx)
#endif

namespace Iotvex::Core::Log {

// Human-readable node label in brackets. Max 47 characters.
// Intended to be set once at boot; not internally locked.
void set_node_name(const char *name);

// Stable node identifier in (node-id:...). Max 47 characters.
// Intended to be set once at boot; not internally locked.
void set_node_id(const char *id);

// DEBUG level. Filter: esp_log_level_set("iotvex.<channel>", ESP_LOG_DEBUG).
void debug(const char *channel, const char *fmt, ...) IX_LOG_PRINTF(2, 3);

// INFO level.
void info(const char *channel, const char *fmt, ...) IX_LOG_PRINTF(2, 3);

// WARN level.
void warn(const char *channel, const char *fmt, ...) IX_LOG_PRINTF(2, 3);

// ERROR level.
void error(const char *channel, const char *fmt, ...) IX_LOG_PRINTF(2, 3);

} // namespace Iotvex::Core::Log

// IX_LOGD/I/W/E - preferred call-site macros.
// channel: module path without "iotvex." prefix.
// fmt:     printf-style format string.

#define IX_LOGD(channel, fmt, ...)                                             \
    Iotvex::Core::Log::debug(channel, fmt, ##__VA_ARGS__)
#define IX_LOGI(channel, fmt, ...)                                             \
    Iotvex::Core::Log::info(channel, fmt, ##__VA_ARGS__)
#define IX_LOGW(channel, fmt, ...)                                             \
    Iotvex::Core::Log::warn(channel, fmt, ##__VA_ARGS__)
#define IX_LOGE(channel, fmt, ...)                                             \
    Iotvex::Core::Log::error(channel, fmt, ##__VA_ARGS__)
