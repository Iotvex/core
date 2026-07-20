# IotvexCore

Общие примитивы для библиотек Iotvex на ESP-IDF: типизированные ошибки (`Status`) и структурированное логирование.

**Version:** `0.1.0`

Требования: ESP-IDF `>= 5.1.0`, C++17.

## Как работает

Один публичный заголовок:

```cpp
#include <IotvexCore.h>
```

### Errors

Единый слой ошибок для всех библиотек Iotvex. Функции возвращают `Status` с кодом (`Error`) и коротким текстом (до 96 байт).

```cpp
using Iotvex::Core::Errors;

Errors::Status open_config() {
  return Errors::fail(
      Errors::Error::NotFound,
      Errors::ErrorContext::with_str("config.json"));
  // detail: "config.json not found"
}

Errors::Status init() {
  IX_RETURN_IF_ERROR(open_config());
  return Errors::ok();
}
```

- `fail(code)` — встроенное описание кода
- `fail(code, ErrorContext::...)` — шаблон с контекстом (`with_str`, `with_int`, …)
- `fail(code, fmt, ...)` — свой printf-текст
- `IX_RETURN_IF_ERROR(expr)` — ранний выход при ошибке
- `IX_LOGE_STATUS(channel, status)` — залогировать failed status

### Log

Структурированные однострочные логи поверх ESP-IDF `log`:

```text
(seq) [UTC] [LEVEL] [name] iotvex.<channel>: message (node-id:id)
```

```cpp
Iotvex::Core::Log::set_node_name("c6-kitchen");
Iotvex::Core::Log::set_node_id("22323");

IX_LOGI("node.lifecycle", "started");
IX_LOGE("light.drivers.pixel", "init failed on pin %d", pin);
```

Канал — путь модуля **без** префикса `iotvex.`. Фильтр ESP-IDF: `iotvex.<channel>`:

```c
esp_log_level_set("iotvex.node.lifecycle", ESP_LOG_DEBUG);
```

Имя и id узла задаются один раз при старте. Без них в логе будет `<unset>`. Сырой `ESP_LOGI()` не меняется.

## Установка

### ESP-IDF (локальный путь)

В `idf_component.yml` проекта или зависимого компонента:

```yaml
dependencies:
  iotvex/core:
    path: ../core
```

### ESP-IDF (git / тег)

```yaml
dependencies:
  iotvex/core:
    git: https://github.com/Iotvex/core.git
    version: v0.1.0
```

В `CMakeLists.txt` потребляющего компонента:

```cmake
REQUIRES iotvex__core
```

### PlatformIO

```ini
lib_deps =
  https://github.com/Iotvex/core.git#v0.1.0
```

## Вклад в разработку

В репозитории две ветки:

| Ветка | Назначение |
|-------|------------|
| `dev` | разработка и pre-release (`vX.Y.Z-dev.N`) |
| `main` | стабильные релизы (`vX.Y.Z`) |

### Процесс

1. Работаешь в `dev` (или PR в `dev`)
2. Пуш в `dev` → CI может сделать pre-release через Cocogitto
3. Когда готово — PR `dev` → `main`
4. Мерж в `main` → стабильный релиз

Прямые пуши в `main` нежелательны.

### Коммиты

Только [Conventional Commits](https://www.conventionalcommits.org/). CI проверяет их с последнего тега.

```text
feat: add Error::BusyTimeout
fix: guard null log channel
feat!: resize Status message buffer
docs: clarify git install
chore: tweak clang-format
```

| Тип | Бамп версии |
|-----|-------------|
| `feat!:` / `BREAKING CHANGE:` | MAJOR |
| `feat:` | MINOR |
| `fix:` | PATCH |

Релизы автоматизированы [Cocogitto](https://docs.cocogitto.io/): версия синкается в `idf_component.yml`, `library.json` и README, обновляется `CHANGELOG.md`, ставится тег и GitHub Release.

## License

[MIT](LICENSE) © 2026 Xlebp Rjanoi
