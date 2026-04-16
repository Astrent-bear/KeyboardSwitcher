# KeyboardSwitcherC

Open-source Windows tray utility for keyboard layout switching and text transformation, inspired by Punto Switcher and implemented as a small native Win32 app in plain C.

The app converts text by key-position mapping, not dictionary translation.

---

## Українська

### Що це

`KeyboardSwitcherC` — це невелика Win32-утиліта для Windows, яка живе в треї та допомагає:

- перетворювати виділений текст між розкладками
- перетворювати останнє набране слово
- перемикати активну розкладку в тому ж порядку, що й Windows

Це не словниковий переклад. Програма працює через мапінг символів по клавішах.

### Швидкий старт

1. Завантажте останній portable-пакет зі сторінки [Releases](https://github.com/Astrent-bear/KeyboardSwitcher/releases/latest).
2. Розпакуйте zip-архів.
3. Запустіть `KeyboardSwitcherC.exe`.
4. Програма стартує в системному треї.
5. Двічі натисніть лівою кнопкою по іконці в треї, щоб відкрити налаштування, або натисніть правою кнопкою для меню.

### Поведінка за замовчуванням

- Гаряча клавіша за замовчуванням: `Pause/Break`
- За замовчуванням одна й та сама клавіша призначена для:
  - перетворення виділеного тексту
  - перетворення останнього слова
- Якщо обидві дії використовують одну гарячу клавішу, програма пробує:
  1. перетворити останнє слово, якщо трекер слова ще валідний
  2. якщо слово було скинуте або не підходить, перетворити виділений текст
- Звук перемикання за замовчуванням вимкнений
- Debug log за замовчуванням вимкнений
- Налаштування зберігаються в `%APPDATA%\KeyboardSwitcherC\settings.ini`

### Поточні можливості

- прихована Win32-програма з іконкою в треї без головного вікна
- перетворення виділеного тексту по гарячій клавіші
- швидке перетворення останнього набраного слова по гарячій клавіші
- безпечніше перетворення виділеного тексту з full-format snapshot буфера обміну
- перемикання розкладки за поточним порядком мов введення Windows
- резервний внутрішній цикл, якщо системний порядок визначити не вдалося
- live-apply вікно налаштувань без `Save/Cancel`
- налаштовувані гарячі клавіші для:
  - перетворення виділеного тексту
  - перетворення останнього слова
- опційний звук перемикання
- опційний debug log
- опційний автозапуск разом із Windows
- інтерфейс англійською та українською
- версія, посилання на GitHub і placeholder-кнопка для Microsoft Store в налаштуваннях

### Підтримувані мапінги розкладок

Зараз таблиці мапінгу позицій клавіш реалізовані для 37 розкладок:

- `en`, `en-gb`
- `ru`, `uk`, `be`, `kk`, `bg`, `sr`, `mk`, `ky`, `tg`, `uz`
- `de`, `fr`, `es`, `it`, `pl`, `nl`, `pt`, `pt-br`, `tr`
- `cs`, `sk`, `sl`, `hu`, `ro`, `hr`, `sv`, `fi`, `no`, `da`, `is`
- `el`
- `ar`, `he`, `fa`
- `ka`

### Самоперевірка без зміни системи

Щоб перевірити таблиці мапінгу без встановлення десятків мов у Windows, у `Debug`-збірці є режим self-test:

```powershell
.\bin\Debug\KeyboardSwitcherC.exe --self-test
```

Після запуску створюється звіт:

```text
bin\Debug\layout-selftest-report.txt
```

Self-test перевіряє:

- цілісність таблиць розкладок
- наявність порожніх токенів
- дублікати токенів усередині розкладки
- round-trip перетворення між парами розкладок

Частина попереджень для окремих мов очікувана: деякі реальні розкладки мають неоднозначні або повторювані символи.

### Важлива поведінка та обмеження

- Перетворення тексту виконується через мапінг клавіш, а не словниковий переклад.
- У перетворенні беруть участь лише ті розкладки, які:
  - підтримуються в коді
  - і водночас встановлені у вашій системі Windows
- Порядок циклу береться зі списку мов введення Windows. Якщо його не вдається використати, програма переходить на внутрішній резервний цикл.
- Для однакової гарячої клавіші першим виконується сценарій останнього слова. Сценарій виділеного тексту використовується як fallback, коли слово було скинуте або контекст введення змінився.
- Сценарій виділеного тексту тимчасово використовує буфер обміну, але намагається зберегти всі підтримувані формати та не відновлювати старий snapshot, якщо clipboard уже змінила інша програма.
- Debug log не записує сам текст користувача, лише технічні події, довжини рядків і коди розкладок.
- Деякі програми та ігри працюють із введенням нестандартно, тому поведінка може відрізнятися залежно від цільового застосунку.
- Для частини екзотичніших або правосторонніх розкладок практичне перетворення сильно залежить від того, як конкретний застосунок обробляє Unicode-ввід і clipboard.

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

- `src\main.c` — старт застосунку та глобальний стан
- `src\ui.c` — трей-інтерфейс і вікно налаштувань
- `src\input.c` — keyboard/mouse hook, маршрутизація hotkey, трекінг останнього слова
- `src\transform.c` — визначення розкладок, перетворення, clipboard-сценарій, self-test
- `src\settings.c` — налаштування, логування, звук, автозапуск, форматування hotkey
- `src\layout_data.inc` — таблиці розкладок, нормалізовані для C-коду
- `src\app.h` — спільні оголошення

### Для спільноти

- Як долучитися: [CONTRIBUTING.md](./CONTRIBUTING.md)
- Політика безпеки: [SECURITY.md](./SECURITY.md)
- Історія змін: [CHANGELOG.md](./CHANGELOG.md)
- Ліцензія: [LICENSE](./LICENSE)

---

## English

### What it is

`KeyboardSwitcherC` is a small Win32 tray utility for Windows that can:

- convert selected text between keyboard layouts
- convert the last typed word
- switch the active layout using the same effective order as Windows

This is not dictionary translation. The app works through key-position mapping.

### Quick start

1. Download the latest portable package from [Releases](https://github.com/Astrent-bear/KeyboardSwitcher/releases/latest).
2. Extract the zip archive.
3. Run `KeyboardSwitcherC.exe`.
4. The app starts in the system tray.
5. Double-click the tray icon to open settings, or right-click it to open the tray menu.

### Default behavior

- Default hotkey: `Pause/Break`
- By default, the same hotkey is assigned to both:
  - selected text conversion
  - last word conversion
- If both actions use the same hotkey, the app tries:
  1. last word conversion when the word tracker is still valid
  2. selected text conversion as fallback when the word was reset or the input context changed
- Switch sound is disabled by default
- Debug logging is disabled by default
- Settings are stored in `%APPDATA%\KeyboardSwitcherC\settings.ini`

### Current features

- hidden Win32 application with tray icon and no main window
- selected-text conversion by hotkey
- fast last typed word conversion by hotkey
- safer selected-text conversion with full-format clipboard snapshots
- layout switching by the current Windows input-language order
- fallback internal cycle if the system order cannot be resolved
- live-apply settings window without `Save/Cancel`
- configurable hotkeys for:
  - selected text conversion
  - last word conversion
- optional switch sound
- optional debug log
- optional start with Windows
- English and Ukrainian UI
- version label, GitHub link, and Microsoft Store placeholder button in settings

### Supported layout mappings

The current key-position mapping set covers 37 layouts:

- `en`, `en-gb`
- `ru`, `uk`, `be`, `kk`, `bg`, `sr`, `mk`, `ky`, `tg`, `uz`
- `de`, `fr`, `es`, `it`, `pl`, `nl`, `pt`, `pt-br`, `tr`
- `cs`, `sk`, `sl`, `hu`, `ro`, `hr`, `sv`, `fi`, `no`, `da`, `is`
- `el`
- `ar`, `he`, `fa`
- `ka`

### Self-test without changing your Windows setup

To validate the mapping tables without installing a large set of Windows layouts, the `Debug` build includes a self-test mode:

```powershell
.\bin\Debug\KeyboardSwitcherC.exe --self-test
```

It writes a report to:

```text
bin\Debug\layout-selftest-report.txt
```

The self-test checks:

- mapping-table integrity
- empty tokens
- duplicate tokens inside a layout
- round-trip conversion across layout pairs

Some warnings are expected for certain real-world layouts because a few keyboards contain repeated or ambiguous visible symbols.

### Important behavior and limitations

- Text conversion is based on key-position mapping, not dictionary translation.
- Only layouts that are both:
  - implemented in code
  - and installed in Windows
  can participate in actual text conversion.
- The layout cycle follows the Windows input-language order when possible. If that order cannot be used, the app falls back to an internal cycle.
- When both actions share the same hotkey, last-word conversion is tried first. Selected-text conversion is used as fallback when the tracked word is invalid or the input context changed.
- Selected-text conversion temporarily uses the clipboard, but it tries to preserve all supported clipboard formats and avoids restoring an old snapshot if another app has already changed the clipboard.
- Debug logs do not store the user's actual text; they only contain technical events, string lengths, and layout codes.
- Some applications and games expose text input in non-standard ways, so behavior may still vary by target app.
- For some more exotic or right-to-left layouts, real-world behavior also depends on how the target application handles Unicode input and clipboard operations.

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

- `src\main.c` — app bootstrap and global state
- `src\ui.c` — tray UI and settings window
- `src\input.c` — keyboard and mouse hooks, hotkey routing, last-word tracking
- `src\transform.c` — layout detection, conversion, clipboard flow, self-test
- `src\settings.c` — settings, logging, sound, autostart, hotkey formatting
- `src\layout_data.inc` — layout tables normalized for C code
- `src\app.h` — shared declarations

### Community

- Contribution guide: [CONTRIBUTING.md](./CONTRIBUTING.md)
- Security policy: [SECURITY.md](./SECURITY.md)
- Changelog: [CHANGELOG.md](./CHANGELOG.md)
- License: [LICENSE](./LICENSE)
