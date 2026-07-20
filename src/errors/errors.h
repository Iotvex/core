#pragma once

#include <cstddef>

// Unified error layer for all Iotvex libraries.
//
// Two text levels:
//   short  - error_name(code)              e.g. "NotFound"
//   detail - status_detail(status)         e.g. "config.json not found"
//
// Built-in detail for every code:
//   fail(Error::NotFound)
//       -> "Requested item was not found"
//   fail(Error::NotFound, ErrorContext::with_str("config.json"))
//       -> "config.json not found"
//
// Fully custom detail:
//   fail(Error::NotFound, "missing key %s in %s", "pin", "profile")
//
// Log:
//   IX_LOGE_STATUS("node.storage", st);

#if defined(__GNUC__)
#define IX_ERRORS_PRINTF(fmt_idx, args_idx)                                    \
    __attribute__((format(printf, fmt_idx, args_idx)))
#else
#define IX_ERRORS_PRINTF(fmt_idx, args_idx)
#endif

namespace Iotvex::Core::Errors {

enum class Error {
    Ok = 0,

    Unknown,
    InvalidState,
    InvalidArgument,
    NotInitialized,
    NotReady,
    Cancelled,
    InternalError,

    NotFound,
    AlreadyExists,
    Conflict,

    Timeout,
    BusyTimeout, // wait expired while resource stayed busy (pre-release tags only on dev)
    Expired,

    OutOfMemory,
    ResourceBusy,
    ResourceError,
    BufferFull,

    StorageError,
    ReadError,
    WriteError,
    Corrupted,

    NetworkError,
    NetworkUnavailable,
    ConnectionLost,
    ConnectionRefused,
    Unauthorized,
    Forbidden,

    HardwareError,
    DriverError,
    PinConflict,

    ConfigError,
    ValidationError,
    PermissionDenied,
    Unsupported,

    CapabilityError,
    SafeModeBlocked,
};

struct Status {
    Error code = Error::Ok;
    char message[96]{};

    [[nodiscard]] constexpr bool ok() const { return code == Error::Ok; }

    // True means success (ok). Prefer status.ok() at call sites.
    [[nodiscard]] explicit constexpr operator bool() const { return ok(); }
};

// Optional args for built-in detailed templates.
// Unset int = -1, unset string = nullptr -> generic description is used.
struct ErrorContext {
    const char *custom = nullptr;
    int i0 = -1;
    int i1 = -1;
    const char *s0 = nullptr;

    [[nodiscard]] static constexpr ErrorContext with_int(int value) {
        ErrorContext context;
        context.i0 = value;
        return context;
    }

    [[nodiscard]] static constexpr ErrorContext with_ints(int first,
                                                          int second) {
        ErrorContext context;
        context.i0 = first;
        context.i1 = second;
        return context;
    }

    [[nodiscard]] static constexpr ErrorContext with_str(const char *value) {
        ErrorContext context;
        context.s0 = value;
        return context;
    }

    [[nodiscard]] static constexpr ErrorContext with_str_int(const char *value,
                                                             int number) {
        ErrorContext context;
        context.s0 = value;
        context.i0 = number;
        return context;
    }
};

[[nodiscard]] constexpr bool is_ok(Error error) { return error == Error::Ok; }

[[nodiscard]] constexpr bool is_ok(const Status &status) { return status.ok(); }

[[nodiscard]] const char *error_name(Error error);
[[nodiscard]] const char *error_description(Error error);
[[nodiscard]] const char *status_detail(const Status &status);

[[nodiscard]] Status ok();
[[nodiscard]] Status fail(Error code);
[[nodiscard]] Status fail(Error code, ErrorContext context);
[[nodiscard]] Status fail(Error code, const char *fmt, ...)
    IX_ERRORS_PRINTF(2, 3);

[[nodiscard]] Status annotate(const Status &inner, const char *fmt, ...)
    IX_ERRORS_PRINTF(2, 3);

char *format_status(const Status &status, char *out, size_t out_len);
void log_status(const char *channel, const Status &status);

} // namespace Iotvex::Core::Errors

#define IX_RETURN_IF_ERROR(status_expr)                                        \
    do {                                                                       \
        const Iotvex::Core::Errors::Status _ix_status = (status_expr);        \
        if (!_ix_status.ok()) {                                                \
            return _ix_status;                                                 \
        }                                                                      \
    } while (0)

#define IX_FAIL(code, fmt, ...)                                                \
    return Iotvex::Core::Errors::fail(code, fmt, ##__VA_ARGS__)

#define IX_LOGE_STATUS(channel, status)                                        \
    Iotvex::Core::Errors::log_status(channel, status)
