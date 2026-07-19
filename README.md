# IotvexCore

Shared primitives for Iotvex ESP-IDF libraries: typed errors (`Status`) and structured logging.

**Version:** `0.1.0`

## Install

### ESP-IDF (local path)

In your project's or dependent component's `idf_component.yml`:

```yaml
dependencies:
  iotvex/core:
    path: ../core
```

### ESP-IDF (git tag / origin)

```yaml
dependencies:
  iotvex/core:
    git: https://github.com/Iotvex/core.git
    version: v0.1.0
```

In `CMakeLists.txt` of the consuming component:

```cmake
REQUIRES iotvex__core
```

### PlatformIO

```ini
lib_deps =
  https://github.com/Iotvex/core.git#v0.1.0
```

## Usage

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

Filter ESP-IDF logs by tag, e.g. `iotvex.node.storage`:

```c
esp_log_level_set("iotvex.node.storage", ESP_LOG_DEBUG);
```

## Public API

Consumers include only the umbrella header:

```cpp
#include <IotvexCore.h>
```

Implementation headers live next to sources (`src/errors/errors.h`, `src/log/log.h`) and are on the component include path.

## Versioning & releases

Automated SemVer via **[Cocogitto](https://docs.cocogitto.io/)** (no Node).

| Branch | Trigger | Result |
|--------|---------|--------|
| `dev` | push | pre-release `vX.Y.Z-dev.N` |
| `main` | push (prefer via PR from `dev`) | stable `vX.Y.Z` |

| Change | Commit | Bump |
|--------|--------|------|
| Breaking API | `feat!: ...` or `BREAKING CHANGE:` footer | MAJOR |
| Backward-compatible feature | `feat: ...` | MINOR |
| Bugfix | `fix: ...` | PATCH |

CI syncs version into `idf_component.yml` + `library.json`, updates `CHANGELOG.md`, tags, and publishes a GitHub Release (`dev` tags are marked prerelease).

### Workflow

1. Develop on `dev` (or feature → PR into `dev`)
2. Each releasable push to `dev` → `v0.2.0-dev.N`
3. When stable: open PR `dev` → `main` (protect `main` from direct pushes)
4. Merge → stable `v0.2.0`

### Commit style (required)

```
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
