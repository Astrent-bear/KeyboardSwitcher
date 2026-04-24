#include "app.h"

static const int SETTINGS_SCHEMA_VERSION = 3;
static const wchar_t *AUTOSTART_REGISTRY_PATH = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";

static BOOL BuildSettingsDirectoryPath(wchar_t *path, size_t capacity) {
    DWORD length;

    length = GetEnvironmentVariableW(L"APPDATA", path, (DWORD)capacity);
    if (length == 0 || length >= capacity) {
        if (GetModuleFileNameW(NULL, path, (DWORD)capacity) == 0) {
            return FALSE;
        }

        wchar_t *slash = wcsrchr(path, L'\\');
        if (slash != NULL) {
            *slash = L'\0';
        }
    } else {
        if (FAILED(StringCchCatW(path, capacity, L"\\KeyboardSwitcherC"))) {
            return FALSE;
        }
        CreateDirectoryW(path, NULL);
    }

    return TRUE;
}

static BOOL BuildSettingsFilePath(wchar_t *path, size_t capacity) {
    if (!BuildSettingsDirectoryPath(path, capacity)) {
        return FALSE;
    }

    if (wcsstr(path, L"KeyboardSwitcherC") == NULL) {
        if (FAILED(StringCchCatW(path, capacity, L"\\KeyboardSwitcherC"))) {
            return FALSE;
        }
        CreateDirectoryW(path, NULL);
    }

    return SUCCEEDED(StringCchCatW(path, capacity, L"\\settings.ini"));
}

static UiLanguage GetDefaultUiLanguage(void) {
    LANGID lang_id = GetUserDefaultUILanguage();
    if (PRIMARYLANGID(lang_id) == LANG_UKRAINIAN) {
        return UI_LANGUAGE_UK;
    }
    return UI_LANGUAGE_EN;
}

static BOOL BuildAutostartCommandLine(wchar_t *buffer, size_t capacity) {
    wchar_t module_path[MAX_PATH];

    if (GetModuleFileNameW(NULL, module_path, MAX_PATH) == 0) {
        return FALSE;
    }

    return SUCCEEDED(StringCchPrintfW(buffer, capacity, L"\"%ls\"", module_path));
}

static BOOL IsExtendedVirtualKey(UINT vk) {
    switch (vk) {
    case VK_INSERT:
    case VK_DELETE:
    case VK_HOME:
    case VK_END:
    case VK_PRIOR:
    case VK_NEXT:
    case VK_LEFT:
    case VK_RIGHT:
    case VK_UP:
    case VK_DOWN:
    case VK_DIVIDE:
    case VK_NUMLOCK:
    case VK_RCONTROL:
    case VK_RMENU:
        return TRUE;
    default:
        return FALSE;
    }
}

static void GetVirtualKeyDisplayName(UINT vk, wchar_t *buffer, size_t capacity) {
    LONG scan_code;

    if (vk == 0) {
        StringCchCopyW(buffer, capacity, L"Not assigned");
        return;
    }

    switch (vk) {
    case VK_PAUSE: StringCchCopyW(buffer, capacity, L"Pause"); return;
    case VK_SPACE: StringCchCopyW(buffer, capacity, L"Space"); return;
    case VK_RETURN: StringCchCopyW(buffer, capacity, L"Enter"); return;
    case VK_TAB: StringCchCopyW(buffer, capacity, L"Tab"); return;
    case VK_ESCAPE: StringCchCopyW(buffer, capacity, L"Esc"); return;
    case VK_BACK: StringCchCopyW(buffer, capacity, L"Backspace"); return;
    default:
        break;
    }

    scan_code = (LONG)MapVirtualKeyW(vk, MAPVK_VK_TO_VSC) << 16;
    if (IsExtendedVirtualKey(vk)) {
        scan_code |= 1 << 24;
    }

    if (GetKeyNameTextW(scan_code, buffer, (int)capacity) == 0) {
        StringCchPrintfW(buffer, capacity, L"VK %u", vk);
    }
}

void SetDefaultSettings(AppSettings *settings) {
    ZeroMemory(settings, sizeof(*settings));
    settings->logging_enabled = FALSE;
    settings->sound_enabled = FALSE;
    settings->autostart_enabled = FALSE;
    settings->ui_language = GetDefaultUiLanguage();
    settings->selected_hotkey.vk = DEFAULT_SELECTED_HOTKEY_VK;
    settings->selected_hotkey.modifiers = DEFAULT_SELECTED_HOTKEY_MODIFIERS;
    settings->lastword_hotkey.vk = DEFAULT_LASTWORD_HOTKEY_VK;
    settings->lastword_hotkey.modifiers = DEFAULT_LASTWORD_HOTKEY_MODIFIERS;
}

void NormalizeHotkeyBinding(HotkeyBinding *binding) {
    if (binding->vk == 0) {
        binding->modifiers = 0;
    }
}

BOOL IsModifierVirtualKey(UINT vk) {
    return vk == VK_SHIFT || vk == VK_LSHIFT || vk == VK_RSHIFT
        || vk == VK_CONTROL || vk == VK_LCONTROL || vk == VK_RCONTROL
        || vk == VK_MENU || vk == VK_LMENU || vk == VK_RMENU
        || vk == VK_LWIN || vk == VK_RWIN;
}

UINT CaptureCurrentModifiers(void) {
    UINT modifiers = 0;

    if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0) modifiers |= MOD_SHIFT;
    if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0) modifiers |= MOD_CONTROL;
    if ((GetAsyncKeyState(VK_MENU) & 0x8000) != 0) modifiers |= MOD_ALT;
    if ((GetAsyncKeyState(VK_LWIN) & 0x8000) != 0 || (GetAsyncKeyState(VK_RWIN) & 0x8000) != 0) modifiers |= MOD_WIN;

    return modifiers;
}

void LoadSettings(AppSettings *settings) {
    wchar_t path[MAX_PATH];
    int settings_version;

    SetDefaultSettings(settings);
    if (!BuildSettingsFilePath(path, sizeof(path) / sizeof(path[0]))) {
        settings->autostart_enabled = IsAutostartEnabled();
        return;
    }

    settings_version = GetPrivateProfileIntW(L"General", L"SettingsVersion", 0, path);
    if (settings_version >= SETTINGS_SCHEMA_VERSION) {
        settings->sound_enabled = GetPrivateProfileIntW(L"General", L"SoundEnabled", settings->sound_enabled, path) != 0;
        settings->logging_enabled = GetPrivateProfileIntW(L"General", L"LoggingEnabled", settings->logging_enabled, path) != 0;
    } else {
        settings->sound_enabled = FALSE;
        settings->logging_enabled = FALSE;
    }
    settings->ui_language = GetPrivateProfileIntW(L"General", L"UiLanguage", settings->ui_language, path);
    settings->selected_hotkey.vk = (UINT)GetPrivateProfileIntW(L"Hotkeys", L"SelectedVk", (int)settings->selected_hotkey.vk, path);
    settings->selected_hotkey.modifiers = (UINT)GetPrivateProfileIntW(L"Hotkeys", L"SelectedModifiers", (int)settings->selected_hotkey.modifiers, path);
    settings->lastword_hotkey.vk = (UINT)GetPrivateProfileIntW(L"Hotkeys", L"LastWordVk", (int)settings->lastword_hotkey.vk, path);
    settings->lastword_hotkey.modifiers = (UINT)GetPrivateProfileIntW(L"Hotkeys", L"LastWordModifiers", (int)settings->lastword_hotkey.modifiers, path);
    settings->autostart_enabled = IsAutostartEnabled();

    if (settings->ui_language != UI_LANGUAGE_EN && settings->ui_language != UI_LANGUAGE_UK) {
        settings->ui_language = GetDefaultUiLanguage();
    }

    NormalizeHotkeyBinding(&settings->selected_hotkey);
    NormalizeHotkeyBinding(&settings->lastword_hotkey);
}

void SaveSettings(const AppSettings *settings) {
    wchar_t path[MAX_PATH];
    wchar_t number[32];

    if (!BuildSettingsFilePath(path, sizeof(path) / sizeof(path[0]))) {
        return;
    }

    StringCchPrintfW(number, sizeof(number) / sizeof(number[0]), L"%d", SETTINGS_SCHEMA_VERSION);
    WritePrivateProfileStringW(L"General", L"SettingsVersion", number, path);

    StringCchPrintfW(number, sizeof(number) / sizeof(number[0]), L"%d", settings->logging_enabled ? 1 : 0);
    WritePrivateProfileStringW(L"General", L"LoggingEnabled", number, path);

    StringCchPrintfW(number, sizeof(number) / sizeof(number[0]), L"%d", settings->sound_enabled ? 1 : 0);
    WritePrivateProfileStringW(L"General", L"SoundEnabled", number, path);

    StringCchPrintfW(number, sizeof(number) / sizeof(number[0]), L"%d", settings->ui_language);
    WritePrivateProfileStringW(L"General", L"UiLanguage", number, path);

    StringCchPrintfW(number, sizeof(number) / sizeof(number[0]), L"%u", settings->selected_hotkey.vk);
    WritePrivateProfileStringW(L"Hotkeys", L"SelectedVk", number, path);

    StringCchPrintfW(number, sizeof(number) / sizeof(number[0]), L"%u", settings->selected_hotkey.modifiers);
    WritePrivateProfileStringW(L"Hotkeys", L"SelectedModifiers", number, path);

    StringCchPrintfW(number, sizeof(number) / sizeof(number[0]), L"%u", settings->lastword_hotkey.vk);
    WritePrivateProfileStringW(L"Hotkeys", L"LastWordVk", number, path);

    StringCchPrintfW(number, sizeof(number) / sizeof(number[0]), L"%u", settings->lastword_hotkey.modifiers);
    WritePrivateProfileStringW(L"Hotkeys", L"LastWordModifiers", number, path);
}

void FormatHotkeyBinding(const HotkeyBinding *binding, wchar_t *buffer, size_t capacity) {
    wchar_t key_name[64];
    wchar_t prefix[128] = L"";

    if (binding->vk == 0) {
        StringCchCopyW(buffer, capacity, L"Not assigned");
        return;
    }

    if ((binding->modifiers & MOD_CONTROL) != 0) StringCchCatW(prefix, sizeof(prefix) / sizeof(prefix[0]), L"Ctrl+");
    if ((binding->modifiers & MOD_ALT) != 0) StringCchCatW(prefix, sizeof(prefix) / sizeof(prefix[0]), L"Alt+");
    if ((binding->modifiers & MOD_SHIFT) != 0) StringCchCatW(prefix, sizeof(prefix) / sizeof(prefix[0]), L"Shift+");
    if ((binding->modifiers & MOD_WIN) != 0) StringCchCatW(prefix, sizeof(prefix) / sizeof(prefix[0]), L"Win+");

    GetVirtualKeyDisplayName(binding->vk, key_name, sizeof(key_name) / sizeof(key_name[0]));
    StringCchPrintfW(buffer, capacity, L"%ls%ls", prefix, key_name);
}

BOOL HotkeyBindingsEqual(const HotkeyBinding *left, const HotkeyBinding *right) {
    return left->vk == right->vk && left->modifiers == right->modifiers;
}

BOOL HotkeyOwnsEvent(const HotkeyBinding *binding, const KBDLLHOOKSTRUCT *kbd) {
    if (binding->vk == 0) {
        return FALSE;
    }

    if (kbd->vkCode != binding->vk) {
        return FALSE;
    }

    return CaptureCurrentModifiers() == binding->modifiers;
}

void PlaySwitchSound(void) {
    if (!g_settings.sound_enabled) {
        return;
    }

    MessageBeep(MB_OK);
}

BOOL BuildDebugLogFilePath(wchar_t *path, size_t capacity) {
    wchar_t *slash;

    if (path == NULL || capacity == 0) {
        return FALSE;
    }

    if (GetModuleFileNameW(NULL, path, (DWORD)capacity) == 0) {
        return FALSE;
    }

    slash = wcsrchr(path, L'\\');
    if (slash != NULL) {
        slash[1] = L'\0';
    }

    return SUCCEEDED(StringCchCatW(path, capacity, L"keyboard-switcher-c.log"));
}

BOOL DebugLogFileExists(void) {
    wchar_t path[MAX_PATH];
    DWORD attributes;

    if (!BuildDebugLogFilePath(path, sizeof(path) / sizeof(path[0]))) {
        return FALSE;
    }

    attributes = GetFileAttributesW(path);
    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

BOOL DeleteDebugLogFile(void) {
    wchar_t path[MAX_PATH];
    DWORD attributes;

    if (!BuildDebugLogFilePath(path, sizeof(path) / sizeof(path[0]))) {
        return FALSE;
    }

    attributes = GetFileAttributesW(path);
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        return TRUE;
    }
    if ((attributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
        return FALSE;
    }

    return DeleteFileW(path);
}

void LogDebug(const wchar_t *format, ...) {
    wchar_t message[1024];
    wchar_t line[1400];
    wchar_t path[MAX_PATH];
    FILE *file;
    SYSTEMTIME st;
    va_list args;

    if (!g_settings.logging_enabled) {
        return;
    }

    GetLocalTime(&st);
    if (!BuildDebugLogFilePath(path, sizeof(path) / sizeof(path[0]))) {
        return;
    }

    va_start(args, format);
    vswprintf(message, sizeof(message) / sizeof(message[0]), format, args);
    va_end(args);

    swprintf(
        line,
        sizeof(line) / sizeof(line[0]),
        L"%04u-%02u-%02u %02u:%02u:%02u.%03u | %s\n",
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
        message);

    file = _wfopen(path, L"a+, ccs=UTF-8");
    if (file == NULL) {
        return;
    }

    fputws(line, file);
    fclose(file);
}

BOOL IsAutostartEnabled(void) {
    HKEY key;
    LONG result;
    wchar_t value[MAX_PATH * 2];
    DWORD type = 0;
    DWORD size = sizeof(value);

    result = RegOpenKeyExW(HKEY_CURRENT_USER, AUTOSTART_REGISTRY_PATH, 0, KEY_READ, &key);
    if (result != ERROR_SUCCESS) {
        return FALSE;
    }

    result = RegQueryValueExW(key, APP_NAME, NULL, &type, (LPBYTE)value, &size);
    RegCloseKey(key);
    return result == ERROR_SUCCESS && type == REG_SZ && value[0] != L'\0';
}

BOOL SetAutostartEnabled(BOOL enabled) {
    HKEY key;
    LONG result;

    result = RegCreateKeyExW(HKEY_CURRENT_USER, AUTOSTART_REGISTRY_PATH, 0, NULL, 0,
        KEY_SET_VALUE | KEY_QUERY_VALUE, NULL, &key, NULL);
    if (result != ERROR_SUCCESS) {
        return FALSE;
    }

    if (enabled) {
        wchar_t command[MAX_PATH * 2];
        if (!BuildAutostartCommandLine(command, sizeof(command) / sizeof(command[0]))) {
            RegCloseKey(key);
            return FALSE;
        }

        result = RegSetValueExW(key, APP_NAME, 0, REG_SZ,
            (const BYTE *)command, (DWORD)((wcslen(command) + 1) * sizeof(wchar_t)));
    } else {
        result = RegDeleteValueW(key, APP_NAME);
        if (result == ERROR_FILE_NOT_FOUND) {
            result = ERROR_SUCCESS;
        }
    }

    RegCloseKey(key);
    return result == ERROR_SUCCESS;
}
