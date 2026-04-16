# Changelog

Історія змін ведеться за версіями. Український опис іде першим, англійський дубль нижче.

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
