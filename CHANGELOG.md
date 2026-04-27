# Changelog

Історія змін ведеться за версіями. Український опис іде першим, англійський дубль нижче.

---

## v0.6.1 - release build cleanup

### Українською

- Оновлено відображення версії до `0.6.1`.
- Виправлено `build.ps1`, щоб він збирав актуальний `Cwitcher.sln` через MSBuild, а не старий однофайловий прототип.
- Очищено релізну збірку від застарілих артефактів зі старою назвою.

### English

- Updated the displayed app version to `0.6.1`.
- Fixed `build.ps1` so it builds the current `Cwitcher.sln` through MSBuild instead of the old single-file prototype.
- Cleaned the release build output from stale artifacts with the old name.

---

## v0.6.0 - Cwitcher naming and Store preparation

### Українською

- Перейменовано продукт, Visual Studio project та вихідний exe на `Cwitcher`.
- Оновлено GitHub-посилання, документацію, шаблони issues та локальні імена файлів під новий бренд.
- Додано міграцію старого `%APPDATA%\KeyboardSwitcherC\settings.ini` у новий `%APPDATA%\Cwitcher\settings.ini`, щоб не втрачати налаштування після перейменування.
- Оновлено ім'я debug log на `cwitcher.log`.
- Дополіровано іконку клавіатури для рівнішого відображення у вікні налаштувань.

### English

- Renamed the product, Visual Studio project, and output executable to `Cwitcher`.
- Updated GitHub links, documentation, issue templates, and local filenames for the new brand.
- Added migration from the old `%APPDATA%\KeyboardSwitcherC\settings.ini` path to `%APPDATA%\Cwitcher\settings.ini` so settings survive the rename.
- Updated the debug log filename to `cwitcher.log`.
- Polished the keyboard icon for cleaner rendering in the settings window.

---

## v0.5.0 - redesigned settings UI

### Українською

- Перероблено вікно налаштувань: компактніша ширина, центрований заголовок, оновлений бренд-блок і вирівняні секції.
- Замінено старі чекбокси на сучасні перемикачі для звуку, автозапуску та debug log.
- Об'єднано перемикачі та вибір мови в один верхній блок.
- Оновлено блок гарячих клавіш: компактніші поля, узгоджені кнопки та менша підказка всередині секції.
- Поліпшено відображення іконок, bitmap-згладжування, скруглення контролів і власний стиль випадаючого меню мови.
- Оновлено відображення версії до `0.5.0`.

### English

- Redesigned the settings window with a narrower layout, centered header, refreshed brand block, and better section alignment.
- Replaced old checkboxes with modern toggles for sound, autostart, and debug log.
- Grouped toggles and interface language selection into one top settings block.
- Refined the hotkey section with shorter value fields, aligned buttons, and a smaller in-section hint.
- Improved icon rendering, bitmap smoothing, rounded control backgrounds, and the custom language dropdown menu style.
- Updated the displayed app version to `0.5.0`.

---

## v0.3.0 - input priority and clipboard safety

### Українською

- Змінено поведінку спільної гарячої клавіші: тепер спочатку пробується перетворення останнього слова, а перетворення виділеного тексту використовується як fallback.
- Посилено скидання трекера останнього слова при зміні focus/caret усередині того самого вікна, function keys, navigation/control keys і non-text діях.
- Додано full-format snapshot буфера обміну для сценарію виділеного тексту: програма намагається зберегти всі підтримувані clipboard-формати, а не лише Unicode text.
- Додано marker/race protection для відновлення clipboard: якщо буфер обміну вже змінила інша програма, старий snapshot не перезаписує новий вміст.
- Очікування зміни clipboard переведено на `WM_CLIPBOARDUPDATE` з timeout fallback.
- Прибрано логування фактичного тексту користувача. Debug log тепер пише технічні події, довжини рядків і коди розкладок.
- Додано відкриття налаштувань подвійним кліком по іконці в треї.

### English

- Changed shared-hotkey behavior: last-word conversion is now tried first, and selected-text conversion is used as fallback.
- Strengthened last-word tracker reset rules for focus/caret changes inside the same window, function keys, navigation/control keys, and non-text actions.
- Added full-format clipboard snapshot support for selected-text conversion, preserving supported clipboard formats instead of Unicode text only.
- Added clipboard marker/race protection so an old snapshot does not overwrite newer clipboard content from another app.
- Moved clipboard-change waiting to `WM_CLIPBOARDUPDATE` with timeout fallback.
- Removed actual user text from debug logs. Debug logs now store technical events, string lengths, and layout codes only.
- Added double-click tray icon behavior to open settings.

---

## v0.2.0 - expanded layout mappings and self-test

### Українською

- Розширено таблиці мапінгу з 4 до 37 підтримуваних розкладок.
- Додано token-based engine для ширшої підтримки розкладок.
- Додано режим `--self-test` для перевірки таблиць без встановлення десятків мов у Windows.
- Оновлено README з українським розділом першим.

### English

- Expanded mapping tables from 4 to 37 supported layouts.
- Added a token-based conversion engine for broader layout coverage.
- Added `--self-test` mode to validate layout tables without installing many Windows languages.
- Refreshed README with Ukrainian documentation first.

---

## v0.1.0 - first public native C build

### Українською

- Перший публічний release native C rewrite.
- Додано tray-утиліту, selected-text conversion, last-word conversion, налаштовувані hotkeys, автозапуск, звук і debug log.

### English

- First public release of the native C rewrite.
- Added tray utility behavior, selected-text conversion, last-word conversion, configurable hotkeys, autostart, sound, and debug log.
