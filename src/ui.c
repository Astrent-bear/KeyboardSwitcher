#include "app.h"

static HWND g_settings_window = NULL;
static AppSettings g_pending_settings = { 0 };
static SettingsControls g_settings_controls = { 0 };
static CaptureTarget g_capture_target = CAPTURE_NONE;
static NOTIFYICONDATAW g_tray = { 0 };
static HFONT g_settings_link_font = NULL;
static HICON g_icon_en = NULL;
static HICON g_icon_ru = NULL;
static HICON g_icon_uk = NULL;
static HICON g_icon_unknown = NULL;
static HICON g_icon_brand = NULL;
static wchar_t g_last_badge[8] = L"";
static BOOL g_settings_syncing = FALSE;

static const wchar_t *GITHUB_REPOSITORY_URL = L"https://github.com/Astrent-bear/KeyboardSwitcher";

typedef enum UiTextId {
    TXT_WINDOW_TITLE = 0,
    TXT_MENU_SETTINGS,
    TXT_MENU_SOUND,
    TXT_MENU_EXIT,
    TXT_CHECKBOX_SOUND,
    TXT_CHECKBOX_AUTOSTART,
    TXT_CHECKBOX_LOGGING,
    TXT_LABEL_LANGUAGE,
    TXT_GROUP_SELECTED,
    TXT_GROUP_LASTWORD,
    TXT_BUTTON_RECORD,
    TXT_BUTTON_CLEAR,
    TXT_BUTTON_RESET,
    TXT_STATUS_IDLE,
    TXT_STATUS_CAPTURE_SELECTED,
    TXT_STATUS_CAPTURE_LASTWORD,
    TXT_GITHUB_LINK,
    TXT_VERSION_LABEL,
    TXT_STORE_BUTTON,
    TXT_STORE_PLACEHOLDER_MESSAGE,
    TXT_STORE_PLACEHOLDER_TITLE,
    TXT_LANGUAGE_ENGLISH,
    TXT_LANGUAGE_UKRAINIAN,
    TXT_HOTKEY_NOT_ASSIGNED,
    TXT_AUTOSTART_ERROR_TITLE,
    TXT_AUTOSTART_ERROR_MESSAGE
} UiTextId;

static UiLanguage GetActiveUiLanguage(void) {
    int language = g_settings.ui_language;

    if (g_settings_window != NULL) {
        language = g_pending_settings.ui_language;
    }

    if (language == UI_LANGUAGE_UK) {
        return UI_LANGUAGE_UK;
    }

    return UI_LANGUAGE_EN;
}

static const wchar_t *GetUiText(UiTextId id) {
    if (GetActiveUiLanguage() == UI_LANGUAGE_UK) {
        switch (id) {
        case TXT_WINDOW_TITLE: return L"KeyboardSwitcher Налаштування";
        case TXT_MENU_SETTINGS: return L"Налаштування...";
        case TXT_MENU_SOUND: return L"Увімкнути звук перемикання";
        case TXT_MENU_EXIT: return L"Вихід";
        case TXT_CHECKBOX_SOUND: return L"Увімкнути звук перемикання";
        case TXT_CHECKBOX_AUTOSTART: return L"Запускати разом із Windows";
        case TXT_CHECKBOX_LOGGING: return L"Увімкнути debug log";
        case TXT_LABEL_LANGUAGE: return L"Мова інтерфейсу";
        case TXT_GROUP_SELECTED: return L"Гаряча клавіша для виділеного тексту";
        case TXT_GROUP_LASTWORD: return L"Гаряча клавіша для останнього слова";
        case TXT_BUTTON_RECORD: return L"Запис";
        case TXT_BUTTON_CLEAR: return L"Очистити";
        case TXT_BUTTON_RESET: return L"Скинути";
        case TXT_STATUS_IDLE: return L"Використовуйте «Запис» для нової комбінації. Зміни зберігаються автоматично.";
        case TXT_STATUS_CAPTURE_SELECTED: return L"Натисніть нову комбінацію для виділеного тексту. Esc скасовує запис.";
        case TXT_STATUS_CAPTURE_LASTWORD: return L"Натисніть нову комбінацію для останнього слова. Esc скасовує запис.";
        case TXT_GITHUB_LINK: return L"GitHub: github.com/Astrent-bear/KeyboardSwitcher";
        case TXT_VERSION_LABEL: return L"Версія: " APP_VERSION;
        case TXT_STORE_BUTTON: return L"Microsoft Store (незабаром)";
        case TXT_STORE_PLACEHOLDER_MESSAGE: return L"Посилання на Microsoft Store буде додано пізніше.";
        case TXT_STORE_PLACEHOLDER_TITLE: return L"Microsoft Store";
        case TXT_LANGUAGE_ENGLISH: return L"English";
        case TXT_LANGUAGE_UKRAINIAN: return L"Українська";
        case TXT_HOTKEY_NOT_ASSIGNED: return L"Не призначено";
        case TXT_AUTOSTART_ERROR_TITLE: return L"Автозапуск";
        case TXT_AUTOSTART_ERROR_MESSAGE: return L"Не вдалося змінити параметр автозапуску Windows.";
        default: return L"";
        }
    }

    switch (id) {
    case TXT_WINDOW_TITLE: return L"KeyboardSwitcher Settings";
    case TXT_MENU_SETTINGS: return L"Settings...";
    case TXT_MENU_SOUND: return L"Enable switch sound";
    case TXT_MENU_EXIT: return L"Exit";
    case TXT_CHECKBOX_SOUND: return L"Enable switch sound";
    case TXT_CHECKBOX_AUTOSTART: return L"Start with Windows";
    case TXT_CHECKBOX_LOGGING: return L"Enable debug log";
    case TXT_LABEL_LANGUAGE: return L"Interface language";
    case TXT_GROUP_SELECTED: return L"Selected Text Hotkey";
    case TXT_GROUP_LASTWORD: return L"Last Word Hotkey";
    case TXT_BUTTON_RECORD: return L"Record";
    case TXT_BUTTON_CLEAR: return L"Clear";
    case TXT_BUTTON_RESET: return L"Reset";
    case TXT_STATUS_IDLE: return L"Use Record to capture a new shortcut. Changes are saved automatically.";
    case TXT_STATUS_CAPTURE_SELECTED: return L"Press a new shortcut for selected text. Esc cancels capture.";
    case TXT_STATUS_CAPTURE_LASTWORD: return L"Press a new shortcut for last word. Esc cancels capture.";
    case TXT_GITHUB_LINK: return L"GitHub: github.com/Astrent-bear/KeyboardSwitcher";
    case TXT_VERSION_LABEL: return L"Version: " APP_VERSION;
    case TXT_STORE_BUTTON: return L"Microsoft Store (coming soon)";
    case TXT_STORE_PLACEHOLDER_MESSAGE: return L"The Microsoft Store link will be added later.";
    case TXT_STORE_PLACEHOLDER_TITLE: return L"Microsoft Store";
    case TXT_LANGUAGE_ENGLISH: return L"English";
    case TXT_LANGUAGE_UKRAINIAN: return L"Ukrainian";
    case TXT_HOTKEY_NOT_ASSIGNED: return L"Not assigned";
    case TXT_AUTOSTART_ERROR_TITLE: return L"Autostart";
    case TXT_AUTOSTART_ERROR_MESSAGE: return L"Failed to update the Windows startup setting.";
    default: return L"";
    }
}

static HICON CreateBadgeIcon(const wchar_t *text, COLORREF background) {
    BITMAPV5HEADER bi;
    ICONINFO ii;
    HDC screen;
    HDC dc;
    HBITMAP color;
    HBITMAP mask;
    HFONT font;
    HGDIOBJ old_bitmap;
    HGDIOBJ old_font;
    HBRUSH brush;
    RECT rc;
    RECT text_rc;
    HICON icon;
    void *bits;

    ZeroMemory(&bi, sizeof(bi));
    bi.bV5Size = sizeof(bi);
    bi.bV5Width = 32;
    bi.bV5Height = -32;
    bi.bV5Planes = 1;
    bi.bV5BitCount = 32;
    bi.bV5Compression = BI_BITFIELDS;
    bi.bV5RedMask = 0x00FF0000;
    bi.bV5GreenMask = 0x0000FF00;
    bi.bV5BlueMask = 0x000000FF;
    bi.bV5AlphaMask = 0xFF000000;

    rc.left = 0;
    rc.top = 0;
    rc.right = 32;
    rc.bottom = 32;
    icon = NULL;
    bits = NULL;

    screen = GetDC(NULL);
    dc = CreateCompatibleDC(screen);
    color = CreateDIBSection(screen, (BITMAPINFO *)&bi, DIB_RGB_COLORS, &bits, NULL, 0);
    mask = CreateBitmap(32, 32, 1, 1, NULL);
    if (dc == NULL || color == NULL || mask == NULL) {
        if (screen != NULL) {
            ReleaseDC(NULL, screen);
        }
        if (dc != NULL) {
            DeleteDC(dc);
        }
        if (color != NULL) {
            DeleteObject(color);
        }
        if (mask != NULL) {
            DeleteObject(mask);
        }
        return NULL;
    }

    old_bitmap = SelectObject(dc, color);
    brush = CreateSolidBrush(background);
    FillRect(dc, &rc, brush);
    FrameRect(dc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));

    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(0, 0, 0));
    text_rc = rc;
    text_rc.left += 1;
    text_rc.right -= 1;

    font = CreateFontW(-18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    old_font = SelectObject(dc, font);
    DrawTextW(dc, text, -1, &text_rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    ZeroMemory(&ii, sizeof(ii));
    ii.fIcon = TRUE;
    ii.hbmColor = color;
    ii.hbmMask = mask;
    icon = CreateIconIndirect(&ii);

    SelectObject(dc, old_font);
    SelectObject(dc, old_bitmap);
    DeleteObject(font);
    DeleteObject(brush);
    DeleteObject(color);
    DeleteObject(mask);
    DeleteDC(dc);
    ReleaseDC(NULL, screen);
    return icon;
}

static HICON CreateBrandIcon(void) {
    BITMAPV5HEADER bi;
    ICONINFO ii;
    HDC screen;
    HDC dc;
    HBITMAP color;
    HBITMAP mask;
    HFONT font;
    HGDIOBJ old_bitmap;
    HGDIOBJ old_font;
    HBRUSH brush;
    RECT rc;
    HICON icon;
    void *bits;

    ZeroMemory(&bi, sizeof(bi));
    bi.bV5Size = sizeof(bi);
    bi.bV5Width = 32;
    bi.bV5Height = -32;
    bi.bV5Planes = 1;
    bi.bV5BitCount = 32;
    bi.bV5Compression = BI_BITFIELDS;
    bi.bV5RedMask = 0x00FF0000;
    bi.bV5GreenMask = 0x0000FF00;
    bi.bV5BlueMask = 0x000000FF;
    bi.bV5AlphaMask = 0xFF000000;

    rc.left = 0;
    rc.top = 0;
    rc.right = 32;
    rc.bottom = 32;
    icon = NULL;
    bits = NULL;

    screen = GetDC(NULL);
    dc = CreateCompatibleDC(screen);
    color = CreateDIBSection(screen, (BITMAPINFO *)&bi, DIB_RGB_COLORS, &bits, NULL, 0);
    mask = CreateBitmap(32, 32, 1, 1, NULL);
    if (dc == NULL || color == NULL || mask == NULL) {
        if (screen != NULL) {
            ReleaseDC(NULL, screen);
        }
        if (dc != NULL) {
            DeleteDC(dc);
        }
        if (color != NULL) {
            DeleteObject(color);
        }
        if (mask != NULL) {
            DeleteObject(mask);
        }
        return NULL;
    }

    old_bitmap = SelectObject(dc, color);
    brush = CreateSolidBrush(RGB(255, 255, 255));
    FillRect(dc, &rc, brush);
    FrameRect(dc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
    SetBkMode(dc, TRANSPARENT);

    font = CreateFontW(-17, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    old_font = SelectObject(dc, font);

    SetTextColor(dc, RGB(29, 202, 210));
    TextOutW(dc, 4, 6, L"C", 1);
    SetTextColor(dc, RGB(36, 46, 72));
    TextOutW(dc, 14, 6, L"W", 1);

    ZeroMemory(&ii, sizeof(ii));
    ii.fIcon = TRUE;
    ii.hbmColor = color;
    ii.hbmMask = mask;
    icon = CreateIconIndirect(&ii);

    SelectObject(dc, old_font);
    SelectObject(dc, old_bitmap);
    DeleteObject(font);
    DeleteObject(brush);
    DeleteObject(color);
    DeleteObject(mask);
    DeleteDC(dc);
    ReleaseDC(NULL, screen);
    return icon;
}

static HICON GetTrayIconForLayout(const LayoutDef *layout) {
    if (layout == NULL) {
        return g_icon_unknown;
    }
    if (wcscmp(layout->badge, L"EN") == 0) {
        return g_icon_en;
    }
    if (wcscmp(layout->badge, L"RU") == 0) {
        return g_icon_ru;
    }
    if (wcscmp(layout->badge, L"UK") == 0) {
        return g_icon_uk;
    }
    return g_icon_unknown;
}

static void ApplyDefaultGuiFont(HWND control) {
    SendMessageW(control, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
}

static HFONT GetSettingsLinkFont(void) {
    LOGFONTW lf;
    HFONT base_font;

    if (g_settings_link_font != NULL) {
        return g_settings_link_font;
    }

    base_font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    if (base_font == NULL) {
        return NULL;
    }

    ZeroMemory(&lf, sizeof(lf));
    if (GetObjectW(base_font, sizeof(lf), &lf) == 0) {
        return NULL;
    }

    lf.lfUnderline = TRUE;
    g_settings_link_font = CreateFontIndirectW(&lf);
    return g_settings_link_font;
}

static void ApplyLinkFont(HWND control) {
    HFONT link_font = GetSettingsLinkFont();
    if (link_font != NULL) {
        SendMessageW(control, WM_SETFONT, (WPARAM)link_font, TRUE);
    } else {
        ApplyDefaultGuiFont(control);
    }
}

static void DrawBrandLogo(const DRAWITEMSTRUCT *draw_item) {
    HDC dc = draw_item->hDC;
    RECT rc = draw_item->rcItem;
    RECT text_rect;
    HFONT brand_font = CreateFontW(-24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    HFONT old_font = SelectObject(dc, brand_font);

    FillRect(dc, &rc, GetSysColorBrush(COLOR_WINDOW));
    SetBkMode(dc, TRANSPARENT);

    SelectObject(dc, brand_font);
    text_rect = rc;
    text_rect.left += 4;
    SetTextColor(dc, RGB(29, 202, 210));
    TextOutW(dc, text_rect.left, rc.top + 3, L"C", 1);
    SetTextColor(dc, RGB(36, 46, 72));
    TextOutW(dc, text_rect.left + 15, rc.top + 3, L"witcher", 7);

    SelectObject(dc, old_font);
    DeleteObject(brand_font);
}

static void SetCaptureStatusText(const wchar_t *text) {
    if (g_settings_controls.status_value != NULL) {
        SetWindowTextW(g_settings_controls.status_value, text);
    }
}

static void FormatLocalizedHotkeyBinding(const HotkeyBinding *binding, wchar_t *buffer, size_t capacity) {
    if (binding->vk == 0) {
        StringCchCopyW(buffer, capacity, GetUiText(TXT_HOTKEY_NOT_ASSIGNED));
        return;
    }

    FormatHotkeyBinding(binding, buffer, capacity);
}

static void PopulateLanguageCombo(void) {
    int selection;

    if (g_settings_controls.language_combo == NULL) {
        return;
    }

    g_settings_syncing = TRUE;
    ComboBox_ResetContent(g_settings_controls.language_combo);
    ComboBox_AddString(g_settings_controls.language_combo, GetUiText(TXT_LANGUAGE_ENGLISH));
    ComboBox_AddString(g_settings_controls.language_combo, GetUiText(TXT_LANGUAGE_UKRAINIAN));

    selection = (g_pending_settings.ui_language == UI_LANGUAGE_UK) ? 1 : 0;
    ComboBox_SetCurSel(g_settings_controls.language_combo, selection);
    g_settings_syncing = FALSE;
}

static void UpdateSettingsWindowState(void) {
    wchar_t selected_text[128];
    wchar_t lastword_text[128];

    if (g_settings_window == NULL) {
        return;
    }

    FormatLocalizedHotkeyBinding(&g_pending_settings.selected_hotkey, selected_text, sizeof(selected_text) / sizeof(selected_text[0]));
    FormatLocalizedHotkeyBinding(&g_pending_settings.lastword_hotkey, lastword_text, sizeof(lastword_text) / sizeof(lastword_text[0]));

    g_settings_syncing = TRUE;

    if (g_settings_controls.logging_checkbox != NULL) {
        Button_SetCheck(g_settings_controls.logging_checkbox, g_pending_settings.logging_enabled ? BST_CHECKED : BST_UNCHECKED);
    }

    if (g_settings_controls.sound_checkbox != NULL) {
        Button_SetCheck(g_settings_controls.sound_checkbox, g_pending_settings.sound_enabled ? BST_CHECKED : BST_UNCHECKED);
    }

    if (g_settings_controls.autostart_checkbox != NULL) {
        Button_SetCheck(g_settings_controls.autostart_checkbox, g_pending_settings.autostart_enabled ? BST_CHECKED : BST_UNCHECKED);
    }

    if (g_settings_controls.selected_value != NULL) {
        SetWindowTextW(g_settings_controls.selected_value, selected_text);
    }

    if (g_settings_controls.lastword_value != NULL) {
        SetWindowTextW(g_settings_controls.lastword_value, lastword_text);
    }

    if (g_settings_controls.version_value != NULL) {
        SetWindowTextW(g_settings_controls.version_value, GetUiText(TXT_VERSION_LABEL));
    }

    if (g_settings_controls.language_combo != NULL) {
        ComboBox_SetCurSel(g_settings_controls.language_combo, g_pending_settings.ui_language == UI_LANGUAGE_UK ? 1 : 0);
    }

    g_settings_syncing = FALSE;

    switch (g_capture_target) {
    case CAPTURE_SELECTED:
        SetCaptureStatusText(GetUiText(TXT_STATUS_CAPTURE_SELECTED));
        break;
    case CAPTURE_LASTWORD:
        SetCaptureStatusText(GetUiText(TXT_STATUS_CAPTURE_LASTWORD));
        break;
    default:
        SetCaptureStatusText(GetUiText(TXT_STATUS_IDLE));
        break;
    }
}

static void ApplyLocalizedControlText(void) {
    if (g_settings_window == NULL) {
        return;
    }

    SetWindowTextW(g_settings_window, GetUiText(TXT_WINDOW_TITLE));

    if (g_settings_controls.sound_checkbox != NULL) {
        SetWindowTextW(g_settings_controls.sound_checkbox, GetUiText(TXT_CHECKBOX_SOUND));
    }

    if (g_settings_controls.autostart_checkbox != NULL) {
        SetWindowTextW(g_settings_controls.autostart_checkbox, GetUiText(TXT_CHECKBOX_AUTOSTART));
    }

    if (g_settings_controls.logging_checkbox != NULL) {
        SetWindowTextW(g_settings_controls.logging_checkbox, GetUiText(TXT_CHECKBOX_LOGGING));
    }

    if (g_settings_controls.github_link != NULL) {
        SetWindowTextW(g_settings_controls.github_link, GetUiText(TXT_GITHUB_LINK));
    }

    if (g_settings_controls.store_button != NULL) {
        SetWindowTextW(g_settings_controls.store_button, GetUiText(TXT_STORE_BUTTON));
    }

    SetWindowTextW(GetDlgItem(g_settings_window, IDC_SETTINGS_GROUP_SELECTED), GetUiText(TXT_GROUP_SELECTED));
    SetWindowTextW(GetDlgItem(g_settings_window, IDC_SETTINGS_GROUP_LASTWORD), GetUiText(TXT_GROUP_LASTWORD));
    SetWindowTextW(GetDlgItem(g_settings_window, IDC_SELECTED_RECORD), GetUiText(TXT_BUTTON_RECORD));
    SetWindowTextW(GetDlgItem(g_settings_window, IDC_SELECTED_CLEAR), GetUiText(TXT_BUTTON_CLEAR));
    SetWindowTextW(GetDlgItem(g_settings_window, IDC_SELECTED_RESET), GetUiText(TXT_BUTTON_RESET));
    SetWindowTextW(GetDlgItem(g_settings_window, IDC_LASTWORD_RECORD), GetUiText(TXT_BUTTON_RECORD));
    SetWindowTextW(GetDlgItem(g_settings_window, IDC_LASTWORD_CLEAR), GetUiText(TXT_BUTTON_CLEAR));
    SetWindowTextW(GetDlgItem(g_settings_window, IDC_LASTWORD_RESET), GetUiText(TXT_BUTTON_RESET));
    SetWindowTextW(GetDlgItem(g_settings_window, IDC_SETTINGS_LANGUAGE_LABEL), GetUiText(TXT_LABEL_LANGUAGE));

    PopulateLanguageCombo();
}

static void RefreshSettingsWindow(void) {
    ApplyLocalizedControlText();
    UpdateSettingsWindowState();
    InvalidateRect(g_settings_window, NULL, TRUE);
}

static void ApplyPendingSettings(void) {
    NormalizeHotkeyBinding(&g_pending_settings.selected_hotkey);
    NormalizeHotkeyBinding(&g_pending_settings.lastword_hotkey);

    if (!SetAutostartEnabled(g_pending_settings.autostart_enabled)) {
        g_pending_settings.autostart_enabled = IsAutostartEnabled();
        MessageBoxW(
            g_settings_window,
            GetUiText(TXT_AUTOSTART_ERROR_MESSAGE),
            GetUiText(TXT_AUTOSTART_ERROR_TITLE),
            MB_OK | MB_ICONWARNING);
    }

    g_settings = g_pending_settings;
    g_settings.autostart_enabled = IsAutostartEnabled();
    SaveSettings(&g_settings);
    g_pending_settings = g_settings;

    if (g_settings_window != NULL) {
        RefreshSettingsWindow();
    }
}

static void ResetPendingHotkeyToDefault(CaptureTarget target) {
    if (target == CAPTURE_SELECTED) {
        g_pending_settings.selected_hotkey.vk = DEFAULT_SELECTED_HOTKEY_VK;
        g_pending_settings.selected_hotkey.modifiers = DEFAULT_SELECTED_HOTKEY_MODIFIERS;
    } else if (target == CAPTURE_LASTWORD) {
        g_pending_settings.lastword_hotkey.vk = DEFAULT_LASTWORD_HOTKEY_VK;
        g_pending_settings.lastword_hotkey.modifiers = DEFAULT_LASTWORD_HOTKEY_MODIFIERS;
    }

    ApplyPendingSettings();
}

static void ClearPendingHotkey(CaptureTarget target) {
    HotkeyBinding empty_binding = { 0 };

    if (target == CAPTURE_SELECTED) {
        g_pending_settings.selected_hotkey = empty_binding;
    } else if (target == CAPTURE_LASTWORD) {
        g_pending_settings.lastword_hotkey = empty_binding;
    }

    ApplyPendingSettings();
}

static void StartHotkeyCapture(CaptureTarget target) {
    g_capture_target = target;
    UpdateSettingsWindowState();
}

static void CancelHotkeyCapture(void) {
    g_capture_target = CAPTURE_NONE;
    UpdateSettingsWindowState();
}

static void CommitCapturedHotkey(UINT vk, UINT modifiers) {
    HotkeyBinding binding;

    binding.vk = vk;
    binding.modifiers = modifiers;
    NormalizeHotkeyBinding(&binding);

    if (g_capture_target == CAPTURE_SELECTED) {
        g_pending_settings.selected_hotkey = binding;
    } else if (g_capture_target == CAPTURE_LASTWORD) {
        g_pending_settings.lastword_hotkey = binding;
    }

    g_capture_target = CAPTURE_NONE;
    ApplyPendingSettings();
}

static void CreateSettingsControls(HWND hwnd) {
    int x = SETTINGS_MARGIN;
    int width = 430;
    HWND control;

    control = CreateWindowExW(0, L"BUTTON", L"", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        x, 14, width, 22, hwnd, (HMENU)(INT_PTR)IDC_SETTINGS_SOUND, g_instance, NULL);
    g_settings_controls.sound_checkbox = control;
    ApplyDefaultGuiFont(control);

    control = CreateWindowExW(0, L"BUTTON", L"", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        x, 40, width, 22, hwnd, (HMENU)(INT_PTR)IDC_SETTINGS_AUTOSTART, g_instance, NULL);
    g_settings_controls.autostart_checkbox = control;
    ApplyDefaultGuiFont(control);

    control = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_LEFT,
        x, 70, 120, 20, hwnd, (HMENU)(INT_PTR)IDC_SETTINGS_LANGUAGE_LABEL, g_instance, NULL);
    ApplyDefaultGuiFont(control);

    control = CreateWindowExW(0, L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | CBS_HASSTRINGS,
        x + 132, 66, 170, 300, hwnd, (HMENU)(INT_PTR)IDC_SETTINGS_LANGUAGE_COMBO, g_instance, NULL);
    g_settings_controls.language_combo = control;
    ApplyDefaultGuiFont(control);

    control = CreateWindowExW(0, L"BUTTON", L"", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        x, 100, width, 78, hwnd, (HMENU)(INT_PTR)IDC_SETTINGS_GROUP_SELECTED, g_instance, NULL);
    ApplyDefaultGuiFont(control);

    control = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_READONLY,
        x + 12, 125, 190, 24, hwnd, (HMENU)(INT_PTR)IDC_SELECTED_VALUE, g_instance, NULL);
    g_settings_controls.selected_value = control;
    ApplyDefaultGuiFont(control);

    control = CreateWindowExW(0, L"BUTTON", L"", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        x + 212, 125, 64, 24, hwnd, (HMENU)(INT_PTR)IDC_SELECTED_RECORD, g_instance, NULL);
    ApplyDefaultGuiFont(control);

    control = CreateWindowExW(0, L"BUTTON", L"", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        x + 284, 125, 56, 24, hwnd, (HMENU)(INT_PTR)IDC_SELECTED_CLEAR, g_instance, NULL);
    ApplyDefaultGuiFont(control);

    control = CreateWindowExW(0, L"BUTTON", L"", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        x + 348, 125, 60, 24, hwnd, (HMENU)(INT_PTR)IDC_SELECTED_RESET, g_instance, NULL);
    ApplyDefaultGuiFont(control);

    control = CreateWindowExW(0, L"BUTTON", L"", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        x, 188, width, 78, hwnd, (HMENU)(INT_PTR)IDC_SETTINGS_GROUP_LASTWORD, g_instance, NULL);
    ApplyDefaultGuiFont(control);

    control = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_READONLY,
        x + 12, 213, 190, 24, hwnd, (HMENU)(INT_PTR)IDC_LASTWORD_VALUE, g_instance, NULL);
    g_settings_controls.lastword_value = control;
    ApplyDefaultGuiFont(control);

    control = CreateWindowExW(0, L"BUTTON", L"", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        x + 212, 213, 64, 24, hwnd, (HMENU)(INT_PTR)IDC_LASTWORD_RECORD, g_instance, NULL);
    ApplyDefaultGuiFont(control);

    control = CreateWindowExW(0, L"BUTTON", L"", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        x + 284, 213, 56, 24, hwnd, (HMENU)(INT_PTR)IDC_LASTWORD_CLEAR, g_instance, NULL);
    ApplyDefaultGuiFont(control);

    control = CreateWindowExW(0, L"BUTTON", L"", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        x + 348, 213, 60, 24, hwnd, (HMENU)(INT_PTR)IDC_LASTWORD_RESET, g_instance, NULL);
    ApplyDefaultGuiFont(control);

    control = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_LEFT,
        x, 282, width, 34, hwnd, (HMENU)(INT_PTR)IDC_SETTINGS_STATUS, g_instance, NULL);
    g_settings_controls.status_value = control;
    ApplyDefaultGuiFont(control);

    control = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_LEFT,
        x, 326, 180, 20, hwnd, (HMENU)(INT_PTR)IDC_SETTINGS_VERSION, g_instance, NULL);
    g_settings_controls.version_value = control;
    ApplyDefaultGuiFont(control);

    control = CreateWindowExW(0, L"BUTTON", L"", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        x + 268, 322, 162, 26, hwnd, (HMENU)(INT_PTR)IDC_SETTINGS_STORE, g_instance, NULL);
    g_settings_controls.store_button = control;
    ApplyDefaultGuiFont(control);

    control = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_OWNERDRAW,
        x, 354, 260, 32, hwnd, (HMENU)(INT_PTR)IDC_SETTINGS_BRAND, g_instance, NULL);
    g_settings_controls.brand_logo = control;

    control = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOTIFY,
        x, 392, width, 22, hwnd, (HMENU)(INT_PTR)IDC_SETTINGS_GITHUB, g_instance, NULL);
    g_settings_controls.github_link = control;
    ApplyLinkFont(control);

    control = CreateWindowExW(0, L"BUTTON", L"", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        x, 420, width, 22, hwnd, (HMENU)(INT_PTR)IDC_SETTINGS_LOGGING, g_instance, NULL);
    g_settings_controls.logging_checkbox = control;
    ApplyDefaultGuiFont(control);
}

static void PositionWindowNearTray(HWND hwnd) {
    RECT work_area;
    RECT window_rect;
    int x;
    int y;

    if (!SystemParametersInfoW(SPI_GETWORKAREA, 0, &work_area, 0)) {
        work_area.left = 0;
        work_area.top = 0;
        work_area.right = GetSystemMetrics(SM_CXSCREEN);
        work_area.bottom = GetSystemMetrics(SM_CYSCREEN);
    }

    if (!GetWindowRect(hwnd, &window_rect)) {
        return;
    }

    x = work_area.right - (window_rect.right - window_rect.left) - 18;
    y = work_area.bottom - (window_rect.bottom - window_rect.top) - 18;

    if (x < work_area.left) x = work_area.left + 18;
    if (y < work_area.top) y = work_area.top + 18;

    SetWindowPos(hwnd, NULL, x, y, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
}

static BOOL IsSettingsLinkControl(HWND control) {
    return control != NULL && control == g_settings_controls.github_link;
}

static BOOL IsSettingsValueControl(HWND control) {
    return control != NULL
        && (control == g_settings_controls.selected_value
            || control == g_settings_controls.lastword_value
            || control == g_settings_controls.status_value
            || control == g_settings_controls.version_value
            || control == GetDlgItem(g_settings_window, IDC_SETTINGS_LANGUAGE_LABEL));
}

static LRESULT CALLBACK SettingsWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        CreateSettingsControls(hwnd);
        RefreshSettingsWindow();
        return 0;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_SELECTED_RECORD:
            StartHotkeyCapture(CAPTURE_SELECTED);
            return 0;
        case IDC_SELECTED_CLEAR:
            ClearPendingHotkey(CAPTURE_SELECTED);
            return 0;
        case IDC_SELECTED_RESET:
            ResetPendingHotkeyToDefault(CAPTURE_SELECTED);
            return 0;
        case IDC_LASTWORD_RECORD:
            StartHotkeyCapture(CAPTURE_LASTWORD);
            return 0;
        case IDC_LASTWORD_CLEAR:
            ClearPendingHotkey(CAPTURE_LASTWORD);
            return 0;
        case IDC_LASTWORD_RESET:
            ResetPendingHotkeyToDefault(CAPTURE_LASTWORD);
            return 0;
        case IDC_SETTINGS_SOUND:
            if (!g_settings_syncing && HIWORD(wParam) == BN_CLICKED) {
                g_pending_settings.sound_enabled = Button_GetCheck(g_settings_controls.sound_checkbox) == BST_CHECKED;
                ApplyPendingSettings();
            }
            return 0;
        case IDC_SETTINGS_AUTOSTART:
            if (!g_settings_syncing && HIWORD(wParam) == BN_CLICKED) {
                g_pending_settings.autostart_enabled = Button_GetCheck(g_settings_controls.autostart_checkbox) == BST_CHECKED;
                ApplyPendingSettings();
            }
            return 0;
        case IDC_SETTINGS_LOGGING:
            if (!g_settings_syncing && HIWORD(wParam) == BN_CLICKED) {
                g_pending_settings.logging_enabled = Button_GetCheck(g_settings_controls.logging_checkbox) == BST_CHECKED;
                ApplyPendingSettings();
            }
            return 0;
        case IDC_SETTINGS_LANGUAGE_COMBO:
            if (!g_settings_syncing && HIWORD(wParam) == CBN_SELCHANGE) {
                int selection = ComboBox_GetCurSel(g_settings_controls.language_combo);
                g_pending_settings.ui_language = (selection == 1) ? UI_LANGUAGE_UK : UI_LANGUAGE_EN;
                ApplyPendingSettings();
            }
            return 0;
        case IDC_SETTINGS_GITHUB:
            ShellExecuteW(hwnd, L"open", GITHUB_REPOSITORY_URL, NULL, NULL, SW_SHOWNORMAL);
            return 0;
        case IDC_SETTINGS_STORE:
            MessageBoxW(
                hwnd,
                GetUiText(TXT_STORE_PLACEHOLDER_MESSAGE),
                GetUiText(TXT_STORE_PLACEHOLDER_TITLE),
                MB_OK | MB_ICONINFORMATION);
            return 0;
        default:
            break;
        }
        break;

    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;

    case WM_CTLCOLOREDIT:
    {
        HWND control = (HWND)lParam;
        HDC dc = (HDC)wParam;

        if (control == g_settings_controls.selected_value || control == g_settings_controls.lastword_value) {
            SetTextColor(dc, RGB(0, 0, 0));
            SetBkColor(dc, GetSysColor(COLOR_WINDOW));
            return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
        }
        break;
    }

    case WM_CTLCOLORSTATIC:
    {
        HWND control = (HWND)lParam;
        HDC dc = (HDC)wParam;

        SetBkMode(dc, TRANSPARENT);

        if (IsSettingsLinkControl(control)) {
            SetTextColor(dc, RGB(0, 102, 204));
            return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
        }

        if (IsSettingsValueControl(control)) {
            SetTextColor(dc, RGB(0, 0, 0));
            return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
        }
        break;
    }

    case WM_DRAWITEM:
        if (wParam == IDC_SETTINGS_BRAND) {
            DrawBrandLogo((const DRAWITEMSTRUCT *)lParam);
            return TRUE;
        }
        break;

    case WM_SETCURSOR:
        if ((HWND)wParam == g_settings_controls.github_link) {
            SetCursor(LoadCursorW(NULL, IDC_HAND));
            return TRUE;
        }
        break;

    case WM_NCDESTROY:
        ZeroMemory(&g_settings_controls, sizeof(g_settings_controls));
        g_capture_target = CAPTURE_NONE;
        g_settings_window = NULL;
        g_settings_syncing = FALSE;
        if (g_settings_link_font != NULL) {
            DeleteObject(g_settings_link_font);
            g_settings_link_font = NULL;
        }
        return 0;
    }

    return DefWindowProcW(hwnd, message, wParam, lParam);
}

void OpenSettingsWindow(void) {
    static BOOL class_registered = FALSE;
    WNDCLASSEXW wc;
    RECT rect;

    if (g_settings_window != NULL) {
        PositionWindowNearTray(g_settings_window);
        ShowWindow(g_settings_window, SW_SHOWNORMAL);
        SetForegroundWindow(g_settings_window);
        return;
    }

    if (!class_registered) {
        ZeroMemory(&wc, sizeof(wc));
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = SettingsWindowProc;
        wc.hInstance = g_instance;
        wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.hIcon = g_icon_brand != NULL ? g_icon_brand : g_icon_unknown;
        wc.hIconSm = g_icon_brand != NULL ? g_icon_brand : g_icon_unknown;
        wc.lpszClassName = SETTINGS_CLASS_NAME;
        if (!RegisterClassExW(&wc)) {
            return;
        }
        class_registered = TRUE;
    }

    g_pending_settings = g_settings;
    g_pending_settings.autostart_enabled = IsAutostartEnabled();
    g_capture_target = CAPTURE_NONE;

    rect.left = 0;
    rect.top = 0;
    rect.right = 460;
    rect.bottom = 490;
    AdjustWindowRectEx(&rect, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE, WS_EX_APPWINDOW);

    g_settings_window = CreateWindowExW(
        WS_EX_APPWINDOW,
        SETTINGS_CLASS_NAME,
        GetUiText(TXT_WINDOW_TITLE),
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        0,
        0,
        rect.right - rect.left,
        rect.bottom - rect.top,
        NULL,
        NULL,
        g_instance,
        NULL);

    if (g_settings_window == NULL) {
        return;
    }

    RefreshSettingsWindow();
    PositionWindowNearTray(g_settings_window);
    ShowWindow(g_settings_window, SW_SHOWNORMAL);
    UpdateWindow(g_settings_window);
    SetForegroundWindow(g_settings_window);
}

BOOL HandleSettingsHotkeyCapture(const KBDLLHOOKSTRUCT *kbd, WPARAM wParam) {
    if (g_capture_target == CAPTURE_NONE) {
        return FALSE;
    }

    if (IsKeyDownMessage(wParam)) {
        if (kbd->vkCode == VK_ESCAPE) {
            CancelHotkeyCapture();
            return TRUE;
        }

        if (!IsModifierVirtualKey((UINT)kbd->vkCode)) {
            CommitCapturedHotkey((UINT)kbd->vkCode, CaptureCurrentModifiers());
            return TRUE;
        }
    }

    return TRUE;
}

void NotifySettingsSoundToggled(void) {
    if (g_settings_window != NULL) {
        g_pending_settings = g_settings;
        g_pending_settings.autostart_enabled = IsAutostartEnabled();
        RefreshSettingsWindow();
    }
}

void CreateTrayIcons(void) {
    g_icon_en = CreateBadgeIcon(L"EN", RGB(255, 255, 255));
    g_icon_ru = CreateBadgeIcon(L"RU", RGB(255, 255, 255));
    g_icon_uk = CreateBadgeIcon(L"UK", RGB(255, 255, 255));
    g_icon_unknown = CreateBadgeIcon(L"KS", RGB(255, 255, 255));
    g_icon_brand = (HICON)LoadImageW(g_instance, MAKEINTRESOURCEW(IDI_APP_ICON), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
    if (g_icon_brand == NULL) {
        g_icon_brand = CreateBrandIcon();
    }
}

void DestroyTrayIcons(void) {
    if (g_icon_en != NULL) DestroyIcon(g_icon_en);
    if (g_icon_ru != NULL) DestroyIcon(g_icon_ru);
    if (g_icon_uk != NULL) DestroyIcon(g_icon_uk);
    if (g_icon_unknown != NULL) DestroyIcon(g_icon_unknown);
    if (g_icon_brand != NULL) DestroyIcon(g_icon_brand);
    g_icon_en = NULL;
    g_icon_ru = NULL;
    g_icon_uk = NULL;
    g_icon_unknown = NULL;
    g_icon_brand = NULL;
}

BOOL AddTrayIcon(HWND hwnd) {
    ZeroMemory(&g_tray, sizeof(g_tray));
    g_tray.cbSize = sizeof(g_tray);
    g_tray.hWnd = hwnd;
    g_tray.uID = TRAY_UID;
    g_tray.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    g_tray.uCallbackMessage = WMAPP_TRAYICON;
    g_tray.hIcon = g_icon_unknown;
    wcscpy(g_tray.szTip, APP_NAME);

    if (!Shell_NotifyIconW(NIM_ADD, &g_tray)) {
        return FALSE;
    }

    g_tray.uVersion = NOTIFYICON_VERSION_4;
    Shell_NotifyIconW(NIM_SETVERSION, &g_tray);
    UpdateTrayLayoutIndicator(TRUE);
    return TRUE;
}

void RemoveTrayIcon(void) {
    if (g_tray.hWnd != NULL) {
        Shell_NotifyIconW(NIM_DELETE, &g_tray);
        ZeroMemory(&g_tray, sizeof(g_tray));
    }
}

void UpdateTrayLayoutIndicator(BOOL force) {
    const LayoutDef *layout = GetCurrentLayoutDef();
    const wchar_t *badge = layout ? layout->badge : L"??";

    if (!force && wcscmp(g_last_badge, badge) == 0) {
        return;
    }

    g_tray.uFlags = NIF_ICON | NIF_TIP;
    g_tray.hIcon = GetTrayIconForLayout(layout);
    swprintf(g_tray.szTip, sizeof(g_tray.szTip) / sizeof(g_tray.szTip[0]), L"%s [%s]", APP_NAME, badge);
    Shell_NotifyIconW(NIM_MODIFY, &g_tray);
    wcsncpy(g_last_badge, badge, (sizeof(g_last_badge) / sizeof(g_last_badge[0])) - 1);
    g_last_badge[(sizeof(g_last_badge) / sizeof(g_last_badge[0])) - 1] = L'\0';
}

void ShowTrayMenu(void) {
    HMENU menu;
    POINT pt;
    UINT sound_flags = MF_STRING;

    menu = CreatePopupMenu();
    if (menu == NULL) {
        return;
    }

    if (g_settings.sound_enabled) {
        sound_flags |= MF_CHECKED;
    }

    AppendMenuW(menu, MF_STRING, MENU_OPEN_SETTINGS, GetUiText(TXT_MENU_SETTINGS));
    AppendMenuW(menu, sound_flags, MENU_TOGGLE_SOUND, GetUiText(TXT_MENU_SOUND));
    AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(menu, MF_STRING, MENU_EXIT_APP, GetUiText(TXT_MENU_EXIT));
    GetCursorPos(&pt);
    SetForegroundWindow(g_main_window);
    TrackPopupMenu(menu, TPM_RIGHTBUTTON | TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, g_main_window, NULL);
    DestroyMenu(menu);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        AddClipboardFormatListener(hwnd);
        SetTimer(hwnd, TIMER_LAYOUT, LAYOUT_TIMER_INTERVAL_MS, NULL);
        return 0;

    case WM_CLIPBOARDUPDATE:
        NotifyClipboardUpdated();
        return 0;

    case WM_TIMER:
        if (wParam == TIMER_LAYOUT) {
            UpdateTrayLayoutIndicator(FALSE);
        }
        return 0;

    case WM_COMMAND:
        if (LOWORD(wParam) == MENU_EXIT_APP) {
            DestroyWindow(hwnd);
        } else if (LOWORD(wParam) == MENU_OPEN_SETTINGS) {
            OpenSettingsWindow();
        } else if (LOWORD(wParam) == MENU_TOGGLE_SOUND) {
            g_settings.sound_enabled = !g_settings.sound_enabled;
            SaveSettings(&g_settings);
            NotifySettingsSoundToggled();
        }
        return 0;

    case WMAPP_TRAYICON:
        if (LOWORD(lParam) == WM_LBUTTONDBLCLK) {
            OpenSettingsWindow();
        } else if (LOWORD(lParam) == WM_RBUTTONUP || LOWORD(lParam) == WM_CONTEXTMENU) {
            ShowTrayMenu();
        }
        return 0;

    case WMAPP_TRANSFORM:
    {
        HANDLE thread;
        if (InterlockedCompareExchange(&g_transform_running, 1, 0) != 0) {
            return 0;
        }

        thread = CreateThread(NULL, 0, TransformWorkerThread, (LPVOID)(INT_PTR)wParam, 0, NULL);
        if (thread != NULL) {
            CloseHandle(thread);
        } else {
            LogDebug(L"Failed to start transform worker. Win32=%lu.", GetLastError());
            InterlockedExchange(&g_transform_running, 0);
        }
        return 0;
    }

    case WM_DESTROY:
        KillTimer(hwnd, TIMER_LAYOUT);
        RemoveClipboardFormatListener(hwnd);
        RemoveTrayIcon();
        RemoveKeyboardHook();
        RemoveMouseHook();
        DestroyTrayIcons();
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, message, wParam, lParam);
}
