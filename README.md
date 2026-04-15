# KeyboardSwitcherC

Open-source Windows tray utility for keyboard layout switching and text transformation, inspired by Punto Switcher but implemented as a small native Win32 app in plain C.

The app converts text by key-position mapping, not dictionary translation.

## English

### Quick start

1. Download the latest portable package from [Releases](https://github.com/Astrent-bear/KeyboardSwitcher/releases/latest).
2. Extract the zip archive.
3. Run `KeyboardSwitcherC.exe`.
4. The app starts in the system tray.
5. Right-click the tray icon to open `Settings...` or exit the app.

### Default behavior

- Default hotkey: `Pause/Break`
- By default, the same hotkey is assigned to both:
  - selected text conversion
  - last word conversion
- If both actions use the same hotkey, the app tries:
  1. selected text conversion
  2. last word conversion as fallback
- Switch sound is disabled by default
- Debug logging is disabled by default
- Settings are stored in `%APPDATA%\KeyboardSwitcherC\settings.ini`

### Current features

- hidden Win32 application with tray icon and no main window
- tray badge with current layout code: `EN`, `RU`, `UK`, `??`
- selected-text conversion by hotkey
- last typed word conversion by hotkey
- layout switching by the current Windows input-language order
- fallback internal order if the system order cannot be resolved
- settings window from tray menu
- auto-save settings UI without `Save/Cancel`
- configurable hotkeys for:
  - selected text conversion
  - last word conversion
- optional switch sound
- optional debug log
- optional start with Windows
- English and Ukrainian UI
- version label and project links in settings

### Supported layout mappings

Current key-position mapping tables are implemented for:

- `en`
- `en-gb`
- `ru`
- `uk`

### Important behavior and limitations

- Text conversion is based on key-position mapping, not dictionary translation.
- The app uses the Windows list of installed input languages and follows the same effective cycle as system layout switching.
- If the Windows order cannot be used, the app falls back to an internal cycle.
- Only layouts with implemented mapping tables can participate in text conversion.
- Some applications and games expose text input in non-standard ways, so behavior may still vary by target app.

### Build from source

Open the project in Visual Studio:

- `KeyboardSwitcherC.sln`
- `KeyboardSwitcherC.vcxproj`

Or build from PowerShell:

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1
```

### Output binaries

- Debug: `bin\Debug\KeyboardSwitcherC.exe`
- Release: `bin\Release\KeyboardSwitcherC.exe`

### Project structure

- `src\main.c` - app bootstrap and global state
- `src\ui.c` - tray UI and settings window
- `src\input.c` - keyboard and mouse hooks, hotkey routing, last-word tracking
- `src\transform.c` - layout detection, conversion, clipboard flow
- `src\settings.c` - settings, logging, sound, autostart, hotkey formatting
- `src\app.h` - shared declarations

### Community

- Contribution guide: [CONTRIBUTING.md](./CONTRIBUTING.md)
- Security policy: [SECURITY.md](./SECURITY.md)
- License: [LICENSE](./LICENSE)

---

## Українська

### Швидкий старт

1. Завантажте останній portable-пакет зі сторінки [Releases](https://github.com/Astrent-bear/KeyboardSwitcher/releases/latest).
2. Розпакуйте zip-архів.
3. Запустіть `KeyboardSwitcherC.exe`.
4. Програма стартує в системному треї.
5. Натисніть правою кнопкою по іконці в треї, щоб відкрити `Settings...` або завершити роботу.

### Поведінка за замовчуванням

- Гаряча клавіша за замовчуванням: `Pause/Break`
- За замовчуванням одна й та сама клавіша призначена для:
  - перетворення виділеного тексту
  - перетворення останнього слова
- Якщо обидві дії використовують одну гарячу клавішу, програма пробує:
  1. перетворити виділений текст
  2. якщо не вийшло, перетворити останнє слово
- Звук перемикання за замовчуванням вимкнений
- Debug log за замовчуванням вимкнений
- Налаштування зберігаються в `%APPDATA%\KeyboardSwitcherC\settings.ini`

### Поточні можливості

- прихована Win32-програма з іконкою в треї без головного вікна
- бейдж у треї з поточною розкладкою: `EN`, `RU`, `UK`, `??`
- перетворення виділеного тексту по гарячій клавіші
- перетворення останнього набраного слова по гарячій клавіші
- перемикання розкладки за поточним порядком мов введення Windows
- резервний внутрішній порядок, якщо системний порядок визначити не вдалося
- вікно налаштувань із меню трея
- інтерфейс налаштувань з автозбереженням без `Save/Cancel`
- налаштовувані гарячі клавіші для:
  - перетворення виділеного тексту
  - перетворення останнього слова
- опційний звук перемикання
- опційний debug log
- опційний автозапуск разом із Windows
- інтерфейс англійською та українською
- відображення версії та посилань на проєкт у налаштуваннях

### Підтримувані мапінги розкладок

Зараз таблиці мапінгу позицій клавіш реалізовані для:

- `en`
- `en-gb`
- `ru`
- `uk`

### Важлива поведінка та обмеження

- Перетворення тексту виконується через мапінг клавіш, а не словниковий переклад.
- Програма використовує список встановлених мов введення Windows і намагається йти в тому самому ефективному циклі, що й системне перемикання розкладок.
- Якщо використати системний порядок не виходить, застосунок переходить на внутрішній резервний цикл.
- У перетворенні тексту беруть участь лише ті розкладки, для яких уже є таблиці мапінгу.
- Деякі програми та ігри працюють із введенням нестандартно, тому поведінка може відрізнятися залежно від цільового застосунку.

### Збірка з вихідного коду

Відкрити проєкт у Visual Studio:

- `KeyboardSwitcherC.sln`
- `KeyboardSwitcherC.vcxproj`

Або зібрати через PowerShell:

```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1
```

### Готові збірки

- Debug: `bin\Debug\KeyboardSwitcherC.exe`
- Release: `bin\Release\KeyboardSwitcherC.exe`

### Структура проєкту

- `src\main.c` - старт застосунку та глобальний стан
- `src\ui.c` - трей-інтерфейс і вікно налаштувань
- `src\input.c` - keyboard/mouse hook, маршрутизація hotkey, трекінг останнього слова
- `src\transform.c` - визначення розкладок, перетворення, clipboard-сценарій
- `src\settings.c` - налаштування, логування, звук, автозапуск, форматування hotkey
- `src\app.h` - спільні оголошення

### Для спільноти

- Як долучитися: [CONTRIBUTING.md](./CONTRIBUTING.md)
- Політика безпеки: [SECURITY.md](./SECURITY.md)
- Ліцензія: [LICENSE](./LICENSE)
