# IotvexCore

Shared primitives for [Iotvex](https://github.com/Iotvex) ESP-IDF libraries: typed errors (`Status`) and structured logging.

**Version:** `0.1.0`

| | |
|---|---|
| License | MIT |
| Language | C++17 |
| Framework | ESP-IDF `>= 5.1.0` |
| Platforms | ESP32 (espressif32) |

## What it provides

| Module | Namespace | Role |
|--------|-----------|------|
| **Errors** | `Iotvex::Core::Errors` | Unified `Error` codes + `Status` with short/detail messages |
| **Log** | `Iotvex::Core::Log` | Structured one-line logs with node name, node id, UTC, sequence |

Consumers include a single umbrella header:

```cpp
#include <IotvexCore.h>
```

## Install

### ESP-IDF — local path

In the project's or dependent component's `idf_component.yml`:

```yaml
dependencies:
  iotvex/core:
    path: ../core
```

### ESP-IDF — git tag

```yaml
dependencies:
  iotvex/core:
    git: https://github.com/Iotvex/core.git
    version: v0.1.0
```

In the consuming component's `CMakeLists.txt`:

```cmake
REQUIRES iotvex__core
```

### PlatformIO

```ini
lib_deps =
  https://github.com/Iotvex/core.git#v0.1.0
```

## Quick start

```cpp
#include <IotvexCore.h>

using Iotvex::Core::Errors;

void boot() {
  Iotvex::Core::Log::set_node_name("c6-kitchen");
  Iotvex::Core::Log::set_node_id("22323");
}

Errors::Status open_config() {
  IX_LOGI("node.storage", "loading config");

  return Errors::fail(
      Errors::Error::NotFound,
      Errors::ErrorContext::with_str("config.json"));
}

Errors::Status init() {
  IX_RETURN_IF_ERROR(open_config());
  return Errors::ok();
}
```

Filter ESP-IDF logs by tag, for example `iotvex.node.storage`:

```c
esp_log_level_set("iotvex.node.storage", ESP_LOG_DEBUG);
```

## Errors

Typed error layer shared across Iotvex libraries.

**Two text levels:**

| Level | API | Example |
|-------|-----|---------|
| Short | `error_name(code)` | `"NotFound"` |
| Detail | `status_detail(status)` | `"config.json not found"` |

**Create a status:**

```cpp
// Generic built-in description
Errors::fail(Errors::Error::NotFound);
// -> "Requested item was not found"

// Built-in template with context
Errors::fail(Errors::Error::NotFound,
             Errors::ErrorContext::with_str("config.json"));
// -> "config.json not found"

// Fully custom printf-style detail
Errors::fail(Errors::Error::NotFound, "missing key %s in %s", "pin", "profile");
```

**Helpers:**

| Macro / API | Purpose |
|-------------|---------|
| `IX_RETURN_IF_ERROR(expr)` | Early-return if `Status` is not ok |
| `IX_FAIL(code, fmt, ...)` | `return fail(...)` |
| `IX_LOGE_STATUS(channel, status)` | Log a failed status on a channel |
| `annotate(inner, fmt, ...)` | Wrap an inner status with extra context |
| `format_status(status, buf, len)` | Format `"NotFound: detail"` into a buffer |

`Status` is truthy on success (`explicit operator bool` / `.ok()`). Message buffer is 96 bytes.

Common codes include `NotFound`, `Timeout`, `OutOfMemory`, `NetworkError`, `ConfigError`, `HardwareError`, and others — see `src/errors/errors.h`.

## Logging

Structured single-line format on top of ESP-IDF `log`:

```text
(seq) [UTC] [LEVEL] [name] iotvex.<channel>: message (node-id:id)
```

Example:

```text
(42) [2026-07-11T15:30:22.123Z] [INFO] [c6-kitchen] iotvex.node.lifecycle: started (node-id:22323)
```

| Field | Source |
|-------|--------|
| `seq` | Auto-incrementing counter (atomic) |
| `UTC` | ISO 8601 timestamp (needs SNTP for a real clock) |
| `LEVEL` | `DEBUG` / `INFO` / `WARN` / `ERROR` |
| `name` | `Log::set_node_name()` — max 47 chars |
| `channel` | Module path **without** the `iotvex.` prefix |
| `node-id` | `Log::set_node_id()` — max 47 chars |

**Macros (preferred at call sites):**

```cpp
IX_LOGD("node.lifecycle", "boot step %d", step);
IX_LOGI("node.lifecycle", "started");
IX_LOGW("light.devices", "retry %d", n);
IX_LOGE("light.drivers.pixel", "init failed on pin %d", pin);
```

Call `set_node_name` / `set_node_id` once at boot before other tasks log. Unset values print as `<unset>`. Only `IX_LOG*` / `Log::{debug,info,warn,error}` use this format — raw `ESP_LOGI()` is unchanged.

ESP-IDF filter tag: `iotvex.<channel>`.

## Layout

```text
include/IotvexCore.h   # public umbrella header
src/errors/            # Status / Error / macros
src/log/               # structured logging
scripts/               # release helpers (version sync)
.github/workflows/     # Conventional Commits + release CI
```

Implementation headers (`errors.h`, `log.h`) sit next to sources and are on the component include path via `CMakeLists.txt` / PlatformIO flags.

## Versioning & releases

Automated SemVer via [Cocogitto](https://docs.cocogitto.io/) (no Node).

| Branch | Trigger | Result |
|--------|---------|--------|
| `dev` | push | pre-release `vX.Y.Z-dev.N` |
| `main` | push (prefer via PR from `dev`) | stable `vX.Y.Z` |

| Change | Commit | Bump |
|--------|--------|------|
| Breaking API | `feat!: ...` or `BREAKING CHANGE:` footer | MAJOR |
| Backward-compatible feature | `feat: ...` | MINOR |
| Bug fix | `fix: ...` | PATCH |

CI syncs the version into `idf_component.yml` + `library.json` (+ README), updates `CHANGELOG.md`, tags the repo, and publishes a GitHub Release (`dev` tags are marked prerelease).

### Workflow

1. Develop on `dev` (or feature → PR into `dev`)
2. Each releasable push to `dev` → `v0.2.0-dev.N`
3. When stable: open PR `dev` → `main` (protect `main` from direct pushes)
4. Merge → stable `v0.2.0`

### Commit style (required)

```text
feat: add Error::BusyTimeout
fix: guard null log channel
feat!: resize Status message buffer
docs: clarify git install
chore: tweak clang-format
```

PR checks enforce Conventional Commits since the latest tag.

## Requirements

- ESP-IDF `>= 5.1.0`
- C++17
- Component dependency: ESP-IDF `log`

## License

[MIT](LICENSE) © 2026 Xlebp Rjanoi
