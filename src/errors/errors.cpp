#include "errors.h"

#include "log.h"

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace Iotvex::Core::Errors {

namespace {

enum class Arg : uint8_t {
    None = 0,
    I0 = 1,
    I1 = 2,
    S0 = 4,
    I0_I1 = I0 | I1,
    S0_I0 = S0 | I0,
};

constexpr Arg operator|(Arg left, Arg right) {
    return static_cast<Arg>(static_cast<uint8_t>(left) |
                            static_cast<uint8_t>(right));
}

constexpr Arg operator&(Arg left, Arg right) {
    return static_cast<Arg>(static_cast<uint8_t>(left) &
                            static_cast<uint8_t>(right));
}

struct ErrorMeta {
    const char *name;
    const char *generic;
    const char *param_fmt;
    Arg args;
};

constexpr ErrorMeta k_error_meta[] = {
    {"Ok", "Success", nullptr, Arg::None},

    {"Unknown", "Unknown error", "Unknown error in %s", Arg::S0},
    {"InvalidState", "Operation is not allowed in the current state",
     "Invalid state: %s", Arg::S0},
    {"InvalidArgument", "Invalid argument", "Invalid argument: %s", Arg::S0},
    {"NotInitialized", "Component is not initialized", "%s is not initialized",
     Arg::S0},
    {"NotReady", "Component is not ready yet", "%s is not ready yet", Arg::S0},
    {"Cancelled", "Operation was cancelled", "Operation cancelled: %s",
     Arg::S0},
    {"InternalError", "Internal error", "Internal error in %s", Arg::S0},

    {"NotFound", "Requested item was not found", "%s not found", Arg::S0},
    {"AlreadyExists", "Item already exists", "%s already exists", Arg::S0},
    {"Conflict", "Conflicting state or resource", "Conflict: %s", Arg::S0},

    {"Timeout", "Operation timed out", "Operation timed out after %d ms",
     Arg::I0},
    {"BusyTimeout", "Timed out while waiting for a busy resource",
     "%s busy timeout after %d ms", Arg::S0_I0},
    {"Expired", "Resource or session expired", "%s expired", Arg::S0},

    {"OutOfMemory", "Out of memory", "Out of memory (requested %d bytes)",
     Arg::I0},
    {"ResourceBusy", "Resource is busy", "%s is busy", Arg::S0},
    {"ResourceError", "Resource error", "Resource error: %s", Arg::S0},
    {"BufferFull", "Buffer is full", "Buffer full (%d/%d bytes)", Arg::I0_I1},

    {"StorageError", "Storage operation failed", "Storage error: %s", Arg::S0},
    {"ReadError", "Read operation failed", "Read failed: %s", Arg::S0},
    {"WriteError", "Write operation failed", "Write failed: %s", Arg::S0},
    {"Corrupted", "Data is corrupted", "Corrupted data: %s", Arg::S0},

    {"NetworkError", "Network error", "Network error: %s", Arg::S0},
    {"NetworkUnavailable", "Network is unavailable", "Network unavailable: %s",
     Arg::S0},
    {"ConnectionLost", "Connection was lost", "Connection lost: %s", Arg::S0},
    {"ConnectionRefused", "Connection was refused", "Connection refused: %s:%d",
     Arg::S0_I0},
    {"Unauthorized", "Unauthorized", "Unauthorized: %s", Arg::S0},
    {"Forbidden", "Forbidden", "Forbidden: %s", Arg::S0},

    {"HardwareError", "Hardware error", "Hardware error: %s", Arg::S0},
    {"DriverError", "Driver error", "Driver error: %s", Arg::S0},
    {"PinConflict", "GPIO pin conflict", "GPIO pin %d conflict", Arg::I0},

    {"ConfigError", "Configuration error", "Config error: %s", Arg::S0},
    {"ValidationError", "Validation failed", "Validation failed: %s", Arg::S0},
    {"PermissionDenied", "Permission denied", "Permission denied: %s", Arg::S0},
    {"Unsupported", "Operation is not supported", "Unsupported: %s", Arg::S0},

    {"CapabilityError", "Capability error", "Capability error: %s", Arg::S0},
    {"SafeModeBlocked", "Operation blocked while node is in safe mode",
     "Blocked in safe mode: %s", Arg::S0},
};

constexpr size_t k_error_meta_count =
    sizeof(k_error_meta) / sizeof(k_error_meta[0]);

static_assert(k_error_meta_count ==
                  static_cast<size_t>(Error::SafeModeBlocked) + 1,
              "k_error_meta must stay 1:1 with Error enumerators");

const ErrorMeta &meta_for(Error error) {
    const auto index = static_cast<size_t>(error);
    if (index >= k_error_meta_count) {
        return k_error_meta[static_cast<size_t>(Error::Unknown)];
    }
    return k_error_meta[index];
}

bool has_required_args(Arg args, const ErrorContext &context) {
    if ((args & Arg::I0) != Arg::None && context.i0 < 0) {
        return false;
    }
    if ((args & Arg::I1) != Arg::None && context.i1 < 0) {
        return false;
    }
    if ((args & Arg::S0) != Arg::None &&
        (context.s0 == nullptr || context.s0[0] == '\0')) {
        return false;
    }
    return true;
}

void copy_message(Status &status, const char *text) {
    if (text == nullptr) {
        status.message[0] = '\0';
        return;
    }

    strncpy(status.message, text, sizeof(status.message) - 1);
    status.message[sizeof(status.message) - 1] = '\0';
}

void format_message(Status &status, const char *fmt, va_list args) {
    if (fmt == nullptr) {
        status.message[0] = '\0';
        return;
    }

    vsnprintf(status.message, sizeof(status.message), fmt, args);
    status.message[sizeof(status.message) - 1] = '\0';
}

void write_builtin_detail(Status &status, ErrorContext context) {
    const ErrorMeta &meta = meta_for(status.code);

    if (context.custom != nullptr && context.custom[0] != '\0') {
        copy_message(status, context.custom);
        return;
    }

    if (meta.param_fmt != nullptr && has_required_args(meta.args, context)) {
        switch (meta.args) {
        case Arg::I0:
            snprintf(status.message, sizeof(status.message), meta.param_fmt,
                     context.i0);
            break;
        case Arg::I0_I1:
            snprintf(status.message, sizeof(status.message), meta.param_fmt,
                     context.i0, context.i1);
            break;
        case Arg::S0:
            snprintf(status.message, sizeof(status.message), meta.param_fmt,
                     context.s0);
            break;
        case Arg::S0_I0:
            snprintf(status.message, sizeof(status.message), meta.param_fmt,
                     context.s0, context.i0);
            break;
        default:
            copy_message(status, meta.generic);
            return;
        }

        status.message[sizeof(status.message) - 1] = '\0';
        return;
    }

    copy_message(status, meta.generic);
}

} // namespace

const char *error_name(Error error) { return meta_for(error).name; }

const char *error_description(Error error) { return meta_for(error).generic; }

const char *status_detail(const Status &status) {
    if (status.message[0] != '\0') {
        return status.message;
    }

    return error_description(status.code);
}

Status ok() { return {}; }

Status fail(Error code) {
    Status status;
    status.code = code;
    copy_message(status, error_description(code));
    return status;
}

Status fail(Error code, ErrorContext context) {
    Status status;
    status.code = code;
    write_builtin_detail(status, context);
    return status;
}

Status fail(Error code, const char *fmt, ...) {
    Status status;
    status.code = code;

    va_list args;
    va_start(args, fmt);
    format_message(status, fmt, args);
    va_end(args);

    if (status.message[0] == '\0') {
        copy_message(status, error_description(code));
    }

    return status;
}

Status annotate(const Status &inner, const char *fmt, ...) {
    if (inner.ok()) {
        return ok();
    }

    Status status = inner;

    char prefix[48];
    va_list args;
    va_start(args, fmt);
    vsnprintf(prefix, sizeof(prefix), fmt == nullptr ? "" : fmt, args);
    va_end(args);
    prefix[sizeof(prefix) - 1] = '\0';

    snprintf(status.message, sizeof(status.message), "%s: %s", prefix,
             status_detail(inner));
    status.message[sizeof(status.message) - 1] = '\0';

    return status;
}

char *format_status(const Status &status, char *out, size_t out_len) {
    if (out == nullptr || out_len == 0) {
        return out;
    }

    if (status.ok()) {
        snprintf(out, out_len, "Ok");
        return out;
    }

    snprintf(out, out_len, "%s: %s", error_name(status.code),
             status_detail(status));
    out[out_len - 1] = '\0';
    return out;
}

void log_status(const char *channel, const Status &status) {
    if (status.ok()) {
        return;
    }

    char buffer[128];
    format_status(status, buffer, sizeof(buffer));
    Log::error(channel, "%s", buffer);
}

} // namespace Iotvex::Core::Errors
// ensure BusyTimeout stays listed after SafeModeBlocked meta sync
