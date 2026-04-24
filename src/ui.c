#include "app.h"
#include <objbase.h>
#include <wincodec.h>

typedef struct UiBitmapAsset {
    HBITMAP bitmap;
    UINT width;
    UINT height;
} UiBitmapAsset;

static HWND g_settings_window = NULL;
static AppSettings g_pending_settings = { 0 };
static SettingsControls g_settings_controls = { 0 };
static CaptureTarget g_capture_target = CAPTURE_NONE;
static NOTIFYICONDATAW g_tray = { 0 };
static HFONT g_settings_link_font = NULL;
static HFONT g_settings_title_font = NULL;
static HFONT g_settings_brand_font = NULL;
static HFONT g_settings_heading_font = NULL;
static HFONT g_settings_body_font = NULL;
static HFONT g_settings_small_font = NULL;
static HFONT g_settings_button_font = NULL;
static HFONT g_settings_icon_font = NULL;
static HBRUSH g_settings_background_brush = NULL;
static HBRUSH g_settings_card_brush = NULL;
static HICON g_icon_en = NULL;
static HICON g_icon_ru = NULL;
static HICON g_icon_uk = NULL;
static HICON g_icon_unknown = NULL;
static HICON g_icon_brand = NULL;
static HICON g_icon_github = NULL;
static BOOL g_ui_com_initialized = FALSE;
static IWICImagingFactory *g_wic_factory = NULL;
static UiBitmapAsset g_ui_asset_language = { 0 };
static UiBitmapAsset g_ui_asset_keyboard = { 0 };
static UiBitmapAsset g_ui_asset_autostart = { 0 };
static UiBitmapAsset g_ui_asset_sound = { 0 };
static UiBitmapAsset g_ui_asset_info = { 0 };
static UiBitmapAsset g_ui_asset_log = { 0 };
static UiBitmapAsset g_ui_asset_save_white = { 0 };
static UiBitmapAsset g_ui_asset_delete = { 0 };
static UiBitmapAsset g_ui_asset_reset = { 0 };
static UiBitmapAsset g_ui_asset_store = { 0 };
static UiBitmapAsset g_ui_asset_logo_wordmark_header = { 0 };
static UiBitmapAsset g_ui_asset_logo_wordmark_brand = { 0 };
static UiBitmapAsset g_ui_asset_logo_icon = { 0 };
static UiBitmapAsset g_ui_asset_logo_small = { 0 };
static UiBitmapAsset g_ui_asset_logo_large = { 0 };
static wchar_t g_last_badge[8] = L"";
static BOOL g_settings_syncing = FALSE;

static const wchar_t *GITHUB_REPOSITORY_URL = L"https://github.com/Astrent-bear/KeyboardSwitcher";

#define SETTINGS_LANGUAGE_MENU_EN 42001
#define SETTINGS_LANGUAGE_MENU_UK 42002

#define SETTINGS_CLIENT_WIDTH 720
#define SETTINGS_CLIENT_HEIGHT 780
#define UI_MARGIN 28
#define UI_CARD_RADIUS 16
#define UI_CONTROL_RADIUS 12
#define UI_BADGE_SIZE 42
#define UI_BADGE_ICON_SIZE 26
#define UI_BUTTON_ICON_SIZE 20
#define UI_HOTKEY_CONTROL_HEIGHT 32
#define UI_STORE_BUTTON_SIZE 68
#define UI_BACKGROUND RGB(248, 250, 252)
#define UI_CARD RGB(255, 255, 255)
#define UI_BORDER RGB(229, 235, 243)
#define UI_TEXT RGB(27, 38, 59)
#define UI_MUTED RGB(130, 141, 158)
#define UI_ACCENT RGB(0, 166, 153)
#define UI_ACCENT_DARK RGB(0, 137, 126)
#define UI_ACCENT_LIGHT RGB(224, 251, 248)
#define UI_ICON_MUTED RGB(100, 116, 139)

#define UI_CARD_LEFT UI_MARGIN
#define UI_CARD_RIGHT (SETTINGS_CLIENT_WIDTH - UI_MARGIN)
#define UI_CARD_WIDTH (UI_CARD_RIGHT - UI_CARD_LEFT)
#define UI_TEXT_LEFT 112
#define UI_ICON_LEFT 52

typedef enum UiGlyph {
    UI_GLYPH_NONE = 0,
    UI_GLYPH_GLOBE,
    UI_GLYPH_KEYBOARD,
    UI_GLYPH_INFO,
    UI_GLYPH_SOUND,
    UI_GLYPH_ROCKET,
    UI_GLYPH_DISK,
    UI_GLYPH_TRASH,
    UI_GLYPH_RESET,
    UI_GLYPH_STORE,
    UI_GLYPH_GITHUB,
    UI_GLYPH_DOCUMENT
} UiGlyph;

typedef enum UiTextId {
    TXT_WINDOW_TITLE = 0,
    TXT_MENU_SETTINGS,
    TXT_MENU_SOUND,
    TXT_MENU_EXIT,
    TXT_CHECKBOX_SOUND,
    TXT_CHECKBOX_AUTOSTART,
    TXT_CHECKBOX_LOGGING,
    TXT_DESC_SOUND,
    TXT_DESC_AUTOSTART,
    TXT_DESC_LOGGING,
    TXT_LABEL_LANGUAGE,
    TXT_GROUP_SELECTED,
    TXT_GROUP_LASTWORD,
    TXT_DESC_SELECTED,
    TXT_DESC_LASTWORD,
    TXT_BRAND_TITLE,
    TXT_BRAND_TAGLINE,
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
    TXT_LOG_DELETE,
    TXT_LANGUAGE_ENGLISH,
    TXT_LANGUAGE_UKRAINIAN,
    TXT_HOTKEY_NOT_ASSIGNED,
    TXT_AUTOSTART_ERROR_TITLE,
    TXT_AUTOSTART_ERROR_MESSAGE
} UiTextId;

static BOOL ShouldShowDebugLogLinks(void);
static void RefreshSettingsWindow(void);
static void ApplyPendingSettings(void);

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
        case TXT_WINDOW_TITLE: return L"Cwitcher Налаштування";
        case TXT_MENU_SETTINGS: return L"Налаштування...";
        case TXT_MENU_SOUND: return L"Увімкнути звук перемикання";
        case TXT_MENU_EXIT: return L"Вихід";
        case TXT_CHECKBOX_SOUND: return L"Увімкнути звук перемикання";
        case TXT_CHECKBOX_AUTOSTART: return L"Запускати разом із Windows";
        case TXT_CHECKBOX_LOGGING: return L"Увімкнути журнал налагодження";
        case TXT_DESC_SOUND: return L"Відтворювати звук після успішної зміни розкладки";
        case TXT_DESC_AUTOSTART: return L"Автоматично запускати Cwitcher при вході в систему";
        case TXT_DESC_LOGGING: return L"Записувати технічні події у файл для діагностики";
        case TXT_LABEL_LANGUAGE: return L"Мова інтерфейсу";
        case TXT_GROUP_SELECTED: return L"Гаряча клавіша для виділеного тексту";
        case TXT_GROUP_LASTWORD: return L"Гаряча клавіша для останнього слова";
        case TXT_DESC_SELECTED: return L"Перетворює виділений текст і перемикає розкладку";
        case TXT_DESC_LASTWORD: return L"Перетворює останнє введене слово і перемикає розкладку";
        case TXT_BRAND_TITLE: return L"Cwitcher";
        case TXT_BRAND_TAGLINE: return L"Cwitcher - це швидке перетворення тексту між розкладками";
        case TXT_BUTTON_RECORD: return L"Запис";
        case TXT_BUTTON_CLEAR: return L"Очистити";
        case TXT_BUTTON_RESET: return L"Скинути";
        case TXT_STATUS_IDLE: return L"Натисніть «Запис», щоб задати нову комбінацію. Зміни зберігаються автоматично.";
        case TXT_STATUS_CAPTURE_SELECTED: return L"Натисніть нову комбінацію для виділеного тексту. Esc скасовує запис.";
        case TXT_STATUS_CAPTURE_LASTWORD: return L"Натисніть нову комбінацію для останнього слова. Esc скасовує запис.";
        case TXT_GITHUB_LINK: return L"GitHub: github.com/Astrent-bear/KeyboardSwitcher";
        case TXT_VERSION_LABEL: return L"Версія: " APP_VERSION;
        case TXT_STORE_BUTTON: return L"Microsoft\r\nStore";
        case TXT_STORE_PLACEHOLDER_MESSAGE: return L"Посилання на Microsoft Store буде додано пізніше.";
        case TXT_STORE_PLACEHOLDER_TITLE: return L"Microsoft Store";
        case TXT_LOG_DELETE: return L"Видалити";
        case TXT_LANGUAGE_ENGLISH: return L"English";
        case TXT_LANGUAGE_UKRAINIAN: return L"Українська";
        case TXT_HOTKEY_NOT_ASSIGNED: return L"Не призначено";
        case TXT_AUTOSTART_ERROR_TITLE: return L"Автозапуск";
        case TXT_AUTOSTART_ERROR_MESSAGE: return L"Не вдалося змінити параметр автозапуску Windows.";
        default: return L"";
        }
    }

    switch (id) {
    case TXT_WINDOW_TITLE: return L"Cwitcher Settings";
    case TXT_MENU_SETTINGS: return L"Settings...";
    case TXT_MENU_SOUND: return L"Enable switch sound";
    case TXT_MENU_EXIT: return L"Exit";
    case TXT_CHECKBOX_SOUND: return L"Enable switch sound";
    case TXT_CHECKBOX_AUTOSTART: return L"Start with Windows";
    case TXT_CHECKBOX_LOGGING: return L"Enable debug log";
    case TXT_DESC_SOUND: return L"Play a sound after a successful layout change";
    case TXT_DESC_AUTOSTART: return L"Automatically start Cwitcher when you sign in";
    case TXT_DESC_LOGGING: return L"Write technical events to a file for diagnostics";
    case TXT_LABEL_LANGUAGE: return L"Interface language";
    case TXT_GROUP_SELECTED: return L"Selected Text Hotkey";
    case TXT_GROUP_LASTWORD: return L"Last Word Hotkey";
    case TXT_DESC_SELECTED: return L"Converts selected text and switches the keyboard layout";
    case TXT_DESC_LASTWORD: return L"Converts the last typed word and switches the keyboard layout";
    case TXT_BRAND_TITLE: return L"Cwitcher";
    case TXT_BRAND_TAGLINE: return L"Cwitcher is fast text conversion between keyboard layouts";
    case TXT_BUTTON_RECORD: return L"Record";
    case TXT_BUTTON_CLEAR: return L"Clear";
    case TXT_BUTTON_RESET: return L"Reset";
    case TXT_STATUS_IDLE: return L"Use Record to capture a new shortcut. Changes are saved automatically.";
    case TXT_STATUS_CAPTURE_SELECTED: return L"Press a new shortcut for selected text. Esc cancels capture.";
    case TXT_STATUS_CAPTURE_LASTWORD: return L"Press a new shortcut for last word. Esc cancels capture.";
    case TXT_GITHUB_LINK: return L"GitHub: github.com/Astrent-bear/KeyboardSwitcher";
    case TXT_VERSION_LABEL: return L"Version: " APP_VERSION;
    case TXT_STORE_BUTTON: return L"Microsoft\r\nStore";
    case TXT_STORE_PLACEHOLDER_MESSAGE: return L"The Microsoft Store link will be added later.";
    case TXT_STORE_PLACEHOLDER_TITLE: return L"Microsoft Store";
    case TXT_LOG_DELETE: return L"Delete";
    case TXT_LANGUAGE_ENGLISH: return L"English";
    case TXT_LANGUAGE_UKRAINIAN: return L"Українська";
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
    HGDIOBJ old_bitmap;
    RECT rc;
    HICON icon;
    void *bits;
    HDC mem = NULL;
    HGDIOBJ old_mem_bitmap = NULL;
    BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };

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
    if (bits != NULL) {
        ZeroMemory(bits, 32 * 32 * 4);
    }

    if (g_ui_asset_logo_icon.bitmap != NULL) {
        mem = CreateCompatibleDC(dc);
        if (mem != NULL) {
            old_mem_bitmap = SelectObject(mem, g_ui_asset_logo_icon.bitmap);
            AlphaBlend(dc, 0, 0, 32, 32, mem, 0, 0, (int)g_ui_asset_logo_icon.width, (int)g_ui_asset_logo_icon.height, blend);
        }
    } else if (g_ui_asset_logo_small.bitmap != NULL) {
        mem = CreateCompatibleDC(dc);
        if (mem != NULL) {
            old_mem_bitmap = SelectObject(mem, g_ui_asset_logo_small.bitmap);
            AlphaBlend(dc, 0, 0, 32, 32, mem, 0, 0, (int)g_ui_asset_logo_small.width, (int)g_ui_asset_logo_small.height, blend);
        }
    }

    ZeroMemory(&ii, sizeof(ii));
    ii.fIcon = TRUE;
    ii.hbmColor = color;
    ii.hbmMask = mask;
    icon = CreateIconIndirect(&ii);

    SelectObject(dc, old_bitmap);
    if (mem != NULL) {
        if (old_mem_bitmap != NULL) {
            SelectObject(mem, old_mem_bitmap);
        }
        DeleteDC(mem);
    }
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

static int GetFontHeightForPointSize(HWND hwnd, int points) {
    HDC dc = GetDC(hwnd);
    int dpi = 96;

    if (dc != NULL) {
        dpi = GetDeviceCaps(dc, LOGPIXELSY);
        ReleaseDC(hwnd, dc);
    }

    return -MulDiv(points, dpi, 72);
}

static HFONT CreateSettingsFont(HWND hwnd, int points, int weight, BOOL underline) {
    return CreateFontW(
        GetFontHeightForPointSize(hwnd, points),
        0,
        0,
        0,
        weight,
        FALSE,
        underline,
        FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_NATURAL_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        L"Segoe UI");
}

static void DestroySettingsUiResources(void) {
    if (g_settings_link_font != NULL) DeleteObject(g_settings_link_font);
    if (g_settings_title_font != NULL) DeleteObject(g_settings_title_font);
    if (g_settings_brand_font != NULL) DeleteObject(g_settings_brand_font);
    if (g_settings_heading_font != NULL) DeleteObject(g_settings_heading_font);
    if (g_settings_body_font != NULL) DeleteObject(g_settings_body_font);
    if (g_settings_small_font != NULL) DeleteObject(g_settings_small_font);
    if (g_settings_button_font != NULL) DeleteObject(g_settings_button_font);
    if (g_settings_icon_font != NULL) DeleteObject(g_settings_icon_font);
    if (g_settings_background_brush != NULL) DeleteObject(g_settings_background_brush);
    if (g_settings_card_brush != NULL) DeleteObject(g_settings_card_brush);

    g_settings_link_font = NULL;
    g_settings_title_font = NULL;
    g_settings_brand_font = NULL;
    g_settings_heading_font = NULL;
    g_settings_body_font = NULL;
    g_settings_small_font = NULL;
    g_settings_button_font = NULL;
    g_settings_icon_font = NULL;
    g_settings_background_brush = NULL;
    g_settings_card_brush = NULL;
}

static void EnsureSettingsUiResources(HWND hwnd) {
    if (g_settings_title_font == NULL) {
        g_settings_title_font = CreateSettingsFont(hwnd, 20, FW_SEMIBOLD, FALSE);
    }
    if (g_settings_brand_font == NULL) {
        g_settings_brand_font = CreateSettingsFont(hwnd, 28, FW_SEMIBOLD, FALSE);
    }
    if (g_settings_heading_font == NULL) {
        g_settings_heading_font = CreateSettingsFont(hwnd, 11, FW_SEMIBOLD, FALSE);
    }
    if (g_settings_body_font == NULL) {
        g_settings_body_font = CreateSettingsFont(hwnd, 11, FW_NORMAL, FALSE);
    }
    if (g_settings_small_font == NULL) {
        g_settings_small_font = CreateSettingsFont(hwnd, 10, FW_NORMAL, FALSE);
    }
    if (g_settings_button_font == NULL) {
        g_settings_button_font = CreateSettingsFont(hwnd, 10, FW_NORMAL, FALSE);
    }
    if (g_settings_icon_font == NULL) {
        g_settings_icon_font = CreateFontW(
            GetFontHeightForPointSize(hwnd, 16),
            0,
            0,
            0,
            FW_NORMAL,
            FALSE,
            FALSE,
            FALSE,
            DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE,
            L"Segoe Fluent Icons");
    }
    if (g_settings_link_font == NULL) {
        g_settings_link_font = CreateSettingsFont(hwnd, 10, FW_NORMAL, TRUE);
    }
    if (g_settings_background_brush == NULL) {
        g_settings_background_brush = CreateSolidBrush(UI_BACKGROUND);
    }
    if (g_settings_card_brush == NULL) {
        g_settings_card_brush = CreateSolidBrush(UI_CARD);
    }
}

static void ApplyFont(HWND control, HFONT font) {
    if (control == NULL) {
        return;
    }

    SendMessageW(control, WM_SETFONT, (WPARAM)(font != NULL ? font : GetStockObject(DEFAULT_GUI_FONT)), TRUE);
}

static void ApplyDefaultGuiFont(HWND control) {
    ApplyFont(control, g_settings_body_font);
}

static void ApplyButtonFont(HWND control) {
    ApplyFont(control, g_settings_button_font);
}

static void ApplyLinkFont(HWND control) {
    ApplyFont(control, g_settings_link_font);
}

static void DrawTextWithFont(HDC dc, const wchar_t *text, RECT rect, HFONT font, COLORREF color, UINT format) {
    HFONT old_font = NULL;

    if (font != NULL) {
        old_font = (HFONT)SelectObject(dc, font);
    }

    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, color);
    DrawTextW(dc, text, -1, &rect, format | DT_NOPREFIX);

    if (old_font != NULL) {
        SelectObject(dc, old_font);
    }
}

static void ReleaseUiBitmapAsset(UiBitmapAsset *asset) {
    if (asset == NULL) {
        return;
    }

    if (asset->bitmap != NULL) {
        DeleteObject(asset->bitmap);
        asset->bitmap = NULL;
    }

    asset->width = 0;
    asset->height = 0;
}

static BOOL BuildPreviewAssetPath(const wchar_t *file_name, wchar_t *path, size_t capacity) {
    wchar_t module_path[MAX_PATH];
    wchar_t module_dir[MAX_PATH];
    wchar_t *slash = NULL;

    if (file_name == NULL || path == NULL || capacity == 0) {
        return FALSE;
    }

    if (GetModuleFileNameW(NULL, module_path, ARRAYSIZE(module_path)) == 0) {
        return FALSE;
    }

    if (FAILED(StringCchCopyW(module_dir, ARRAYSIZE(module_dir), module_path))) {
        return FALSE;
    }

    slash = wcsrchr(module_dir, L'\\');
    if (slash == NULL) {
        return FALSE;
    }

    *slash = L'\0';
    if (wcschr(file_name, L'\\') != NULL || wcschr(file_name, L'/') != NULL) {
        return SUCCEEDED(StringCchPrintfW(path, capacity, L"%s\\resources\\icons\\%s", module_dir, file_name));
    }
    return SUCCEEDED(StringCchPrintfW(path, capacity, L"%s\\resources\\icons\\preview\\%s", module_dir, file_name));
}

static BOOL EnsureWicFactory(void) {
    HRESULT hr;

    if (g_wic_factory != NULL) {
        return TRUE;
    }

    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (SUCCEEDED(hr)) {
        g_ui_com_initialized = TRUE;
    } else if (hr != RPC_E_CHANGED_MODE) {
        LogDebug(L"CoInitializeEx failed for UI assets. HRESULT=0x%08lx.", (unsigned long)hr);
        return FALSE;
    }

    hr = CoCreateInstance(&CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER,
        &IID_IWICImagingFactory, (LPVOID *)&g_wic_factory);
    if (FAILED(hr)) {
        LogDebug(L"CoCreateInstance(CLSID_WICImagingFactory) failed. HRESULT=0x%08lx.", (unsigned long)hr);
        if (g_ui_com_initialized) {
            CoUninitialize();
            g_ui_com_initialized = FALSE;
        }
        return FALSE;
    }

    return TRUE;
}

static BOOL LoadUiBitmapAsset(UiBitmapAsset *asset, const wchar_t *file_name) {
    wchar_t path[MAX_PATH];
    IWICBitmapDecoder *decoder = NULL;
    IWICBitmapFrameDecode *frame = NULL;
    IWICFormatConverter *converter = NULL;
    HDC screen = NULL;
    HBITMAP bitmap = NULL;
    void *bits = NULL;
    UINT width = 0;
    UINT height = 0;
    UINT stride = 0;
    UINT image_size = 0;
    BITMAPINFO info;
    HRESULT hr;
    BOOL success = FALSE;

    if (asset == NULL) {
        return FALSE;
    }

    if (asset->bitmap != NULL) {
        return TRUE;
    }

    if (!EnsureWicFactory()) {
        return FALSE;
    }

    if (!BuildPreviewAssetPath(file_name, path, ARRAYSIZE(path))) {
        return FALSE;
    }

    hr = g_wic_factory->lpVtbl->CreateDecoderFromFilename(
        g_wic_factory, path, NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder);
    if (FAILED(hr)) {
        LogDebug(L"Failed to open UI PNG asset %ls. HRESULT=0x%08lx.", path, (unsigned long)hr);
        goto cleanup;
    }

    hr = decoder->lpVtbl->GetFrame(decoder, 0, &frame);
    if (FAILED(hr)) {
        LogDebug(L"Failed to read UI PNG frame %ls. HRESULT=0x%08lx.", path, (unsigned long)hr);
        goto cleanup;
    }

    hr = g_wic_factory->lpVtbl->CreateFormatConverter(g_wic_factory, &converter);
    if (FAILED(hr)) {
        LogDebug(L"Failed to create WIC format converter for %ls. HRESULT=0x%08lx.", path, (unsigned long)hr);
        goto cleanup;
    }

    hr = converter->lpVtbl->Initialize(
        converter,
        (IWICBitmapSource *)frame,
        &GUID_WICPixelFormat32bppPBGRA,
        WICBitmapDitherTypeNone,
        NULL,
        0.0f,
        WICBitmapPaletteTypeCustom);
    if (FAILED(hr)) {
        LogDebug(L"Failed to convert UI PNG %ls to 32bppPBGRA. HRESULT=0x%08lx.", path, (unsigned long)hr);
        goto cleanup;
    }

    hr = converter->lpVtbl->GetSize(converter, &width, &height);
    if (FAILED(hr) || width == 0 || height == 0) {
        LogDebug(L"Failed to read UI PNG dimensions for %ls. HRESULT=0x%08lx.", path, (unsigned long)hr);
        goto cleanup;
    }

    ZeroMemory(&info, sizeof(info));
    info.bmiHeader.biSize = sizeof(info.bmiHeader);
    info.bmiHeader.biWidth = (LONG)width;
    info.bmiHeader.biHeight = -(LONG)height;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;

    screen = GetDC(NULL);
    if (screen == NULL) {
        goto cleanup;
    }

    bitmap = CreateDIBSection(screen, &info, DIB_RGB_COLORS, &bits, NULL, 0);
    if (bitmap == NULL || bits == NULL) {
        LogDebug(L"CreateDIBSection failed for UI PNG %ls. Win32=%lu.", path, GetLastError());
        goto cleanup;
    }

    stride = width * 4;
    image_size = stride * height;
    hr = converter->lpVtbl->CopyPixels(converter, NULL, stride, image_size, (BYTE *)bits);
    if (FAILED(hr)) {
        LogDebug(L"CopyPixels failed for UI PNG %ls. HRESULT=0x%08lx.", path, (unsigned long)hr);
        goto cleanup;
    }

    asset->bitmap = bitmap;
    asset->width = width;
    asset->height = height;
    bitmap = NULL;
    success = TRUE;

cleanup:
    if (screen != NULL) {
        ReleaseDC(NULL, screen);
    }
    if (bitmap != NULL) {
        DeleteObject(bitmap);
    }
    if (converter != NULL) {
        converter->lpVtbl->Release(converter);
    }
    if (frame != NULL) {
        frame->lpVtbl->Release(frame);
    }
    if (decoder != NULL) {
        decoder->lpVtbl->Release(decoder);
    }

    return success;
}

static void LoadUiBitmapAssets(void) {
    LoadUiBitmapAsset(&g_ui_asset_language, L"language_24.png");
    LoadUiBitmapAsset(&g_ui_asset_keyboard, L"keyboard_28.png");
    LoadUiBitmapAsset(&g_ui_asset_autostart, L"autostart_25.png");
    LoadUiBitmapAsset(&g_ui_asset_sound, L"sound_29.png");
    LoadUiBitmapAsset(&g_ui_asset_info, L"info_26.png");
    LoadUiBitmapAsset(&g_ui_asset_log, L"log_29.png");
    LoadUiBitmapAsset(&g_ui_asset_save_white, L"save_white_21.png");
    LoadUiBitmapAsset(&g_ui_asset_delete, L"delete_21.png");
    LoadUiBitmapAsset(&g_ui_asset_reset, L"reset_21.png");
    LoadUiBitmapAsset(&g_ui_asset_store, L"store_24.png");
    LoadUiBitmapAsset(&g_ui_asset_logo_wordmark_header, L"logo_wordmark_header.png");
    LoadUiBitmapAsset(&g_ui_asset_logo_wordmark_brand, L"logo_wordmark_brand.png");
    LoadUiBitmapAsset(&g_ui_asset_logo_icon, L"logo_32.png");
    LoadUiBitmapAsset(&g_ui_asset_logo_small, L"logo_40.png");
    LoadUiBitmapAsset(&g_ui_asset_logo_large, L"logo_56.png");
}

static void DestroyUiBitmapAssets(void) {
    ReleaseUiBitmapAsset(&g_ui_asset_language);
    ReleaseUiBitmapAsset(&g_ui_asset_keyboard);
    ReleaseUiBitmapAsset(&g_ui_asset_autostart);
    ReleaseUiBitmapAsset(&g_ui_asset_sound);
    ReleaseUiBitmapAsset(&g_ui_asset_info);
    ReleaseUiBitmapAsset(&g_ui_asset_log);
    ReleaseUiBitmapAsset(&g_ui_asset_save_white);
    ReleaseUiBitmapAsset(&g_ui_asset_delete);
    ReleaseUiBitmapAsset(&g_ui_asset_reset);
    ReleaseUiBitmapAsset(&g_ui_asset_store);
    ReleaseUiBitmapAsset(&g_ui_asset_logo_wordmark_header);
    ReleaseUiBitmapAsset(&g_ui_asset_logo_wordmark_brand);
    ReleaseUiBitmapAsset(&g_ui_asset_logo_icon);
    ReleaseUiBitmapAsset(&g_ui_asset_logo_small);
    ReleaseUiBitmapAsset(&g_ui_asset_logo_large);

    if (g_wic_factory != NULL) {
        g_wic_factory->lpVtbl->Release(g_wic_factory);
        g_wic_factory = NULL;
    }

    if (g_ui_com_initialized) {
        CoUninitialize();
        g_ui_com_initialized = FALSE;
    }
}

static const UiBitmapAsset *GetBitmapAssetForGlyph(UiGlyph glyph) {
    switch (glyph) {
    case UI_GLYPH_GLOBE:
        return &g_ui_asset_language;
    case UI_GLYPH_KEYBOARD:
        return &g_ui_asset_keyboard;
    case UI_GLYPH_INFO:
        return &g_ui_asset_info;
    case UI_GLYPH_SOUND:
        return &g_ui_asset_sound;
    case UI_GLYPH_ROCKET:
        return &g_ui_asset_autostart;
    case UI_GLYPH_DISK:
        return &g_ui_asset_save_white;
    case UI_GLYPH_TRASH:
        return &g_ui_asset_delete;
    case UI_GLYPH_RESET:
        return &g_ui_asset_reset;
    case UI_GLYPH_STORE:
        return &g_ui_asset_store;
    case UI_GLYPH_DOCUMENT:
        return &g_ui_asset_log;
    default:
        return NULL;
    }
}

static BOOL DrawBitmapAsset(HDC dc, const UiBitmapAsset *asset, int x, int y, int size) {
    HDC mem;
    HGDIOBJ old_bitmap;
    BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
    int old_mode;
    BOOL drawn;

    if (asset == NULL || asset->bitmap == NULL || size <= 0) {
        return FALSE;
    }

    mem = CreateCompatibleDC(dc);
    if (mem == NULL) {
        return FALSE;
    }

    old_bitmap = SelectObject(mem, asset->bitmap);
    old_mode = SetStretchBltMode(dc, HALFTONE);
    drawn = AlphaBlend(dc, x, y, size, size, mem, 0, 0, (int)asset->width, (int)asset->height, blend);
    SetStretchBltMode(dc, old_mode);
    SelectObject(mem, old_bitmap);
    DeleteDC(mem);
    return drawn;
}

static BOOL DrawBitmapAssetRect(HDC dc, const UiBitmapAsset *asset, int x, int y, int width, int height) {
    HDC mem;
    HGDIOBJ old_bitmap;
    BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
    int old_mode;
    BOOL drawn;

    if (asset == NULL || asset->bitmap == NULL || width <= 0 || height <= 0) {
        return FALSE;
    }

    mem = CreateCompatibleDC(dc);
    if (mem == NULL) {
        return FALSE;
    }

    old_bitmap = SelectObject(mem, asset->bitmap);
    old_mode = SetStretchBltMode(dc, HALFTONE);
    drawn = AlphaBlend(dc, x, y, width, height, mem, 0, 0, (int)asset->width, (int)asset->height, blend);
    SetStretchBltMode(dc, old_mode);
    SelectObject(mem, old_bitmap);
    DeleteDC(mem);
    return drawn;
}

static void DrawRoundedCard(HDC dc, int left, int top, int right, int bottom, int radius) {
    HBRUSH old_brush;
    HPEN old_pen;
    HBRUSH brush = CreateSolidBrush(UI_CARD);
    HPEN pen = CreatePen(PS_SOLID, 1, UI_BORDER);

    old_brush = (HBRUSH)SelectObject(dc, brush);
    old_pen = (HPEN)SelectObject(dc, pen);
    RoundRect(dc, left, top, right, bottom, radius, radius);
    SelectObject(dc, old_pen);
    SelectObject(dc, old_brush);
    DeleteObject(pen);
    DeleteObject(brush);
}

static void StrokeLine(HDC dc, int x1, int y1, int x2, int y2, COLORREF color, int width) {
    HPEN old_pen;
    HPEN pen = CreatePen(PS_SOLID, width, color);

    old_pen = (HPEN)SelectObject(dc, pen);
    MoveToEx(dc, x1, y1, NULL);
    LineTo(dc, x2, y2);
    SelectObject(dc, old_pen);
    DeleteObject(pen);
}

static const wchar_t *GetFluentGlyph(UiGlyph glyph) {
    (void)glyph;
    return NULL;
}

static void DrawGlyph(HDC dc, UiGlyph glyph, int x, int y, int size, COLORREF color) {
    const UiBitmapAsset *bitmap_asset = GetBitmapAssetForGlyph(glyph);
    HPEN old_pen;
    HBRUSH old_brush;
    int line = 1;
    int pad = 2;
    int left = x + pad;
    int top = y + pad;
    int right = x + size - pad;
    int bottom = y + size - pad;
    int cx = x + size / 2;
    int cy = y + size / 2;
    HPEN pen = CreatePen(PS_SOLID, line, color);
    HBRUSH hollow = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
    RECT rc;
    POINT points[8];
    const wchar_t *fluent_glyph = GetFluentGlyph(glyph);

    if (bitmap_asset != NULL && DrawBitmapAsset(dc, bitmap_asset, x, y, size)) {
        return;
    }

    if (glyph == UI_GLYPH_GITHUB) {
        RECT glyph_rect;
        HBRUSH brush = CreateSolidBrush(color);
        HPEN github_pen = CreatePen(PS_SOLID, 1, color);
        HGDIOBJ old_github_brush = SelectObject(dc, brush);
        HGDIOBJ old_github_pen = SelectObject(dc, github_pen);

        Ellipse(dc, x + 1, y + 1, x + size - 1, y + size - 1);
        SelectObject(dc, old_github_pen);
        SelectObject(dc, old_github_brush);
        glyph_rect.left = x;
        glyph_rect.top = y + 1;
        glyph_rect.right = x + size;
        glyph_rect.bottom = y + size - 1;
        DrawTextWithFont(dc, L"GH", glyph_rect, g_settings_small_font, RGB(255, 255, 255), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        DeleteObject(github_pen);
        DeleteObject(brush);
        return;
    }

    if (fluent_glyph != NULL && g_settings_icon_font != NULL) {
        RECT glyph_rect;
        HFONT old_font;
        HFONT icon_font;

        icon_font = CreateFontW(
            GetFontHeightForPointSize(g_settings_window != NULL ? g_settings_window : NULL, size),
            0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE, L"Segoe Fluent Icons");
        old_font = (HFONT)SelectObject(dc, icon_font != NULL ? icon_font : g_settings_icon_font);
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, color);
        glyph_rect.left = x;
        glyph_rect.top = y;
        glyph_rect.right = x + size;
        glyph_rect.bottom = y + size;
        DrawTextW(dc, fluent_glyph, -1, &glyph_rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        SelectObject(dc, old_font);
        if (icon_font != NULL) {
            DeleteObject(icon_font);
        }
        return;
    }

    old_pen = (HPEN)SelectObject(dc, pen);
    old_brush = (HBRUSH)SelectObject(dc, hollow);

    switch (glyph) {
    case UI_GLYPH_GLOBE:
        Ellipse(dc, left + 1, top + 1, right - 1, bottom - 1);
        Ellipse(dc, x + size / 4, top + 1, x + (size * 3) / 4, bottom - 1);
        StrokeLine(dc, left + 2, cy, right - 2, cy, color, line);
        StrokeLine(dc, cx, top + 2, cx, bottom - 2, color, line);
        break;
    case UI_GLYPH_KEYBOARD:
        RoundRect(dc, left + 1, y + size / 4, right - 1, y + (size * 3) / 4 + 2, 4, 4);
        StrokeLine(dc, left + 4, cy - 3, right - 4, cy - 3, color, line);
        StrokeLine(dc, left + 4, cy + 1, right - 4, cy + 1, color, line);
        StrokeLine(dc, left + 5, cy - 5, left + 5, cy - 1, color, line);
        StrokeLine(dc, cx, cy - 5, cx, cy - 1, color, line);
        StrokeLine(dc, right - 5, cy - 5, right - 5, cy - 1, color, line);
        StrokeLine(dc, left + 6, cy + 5, right - 6, cy + 5, color, line);
        break;
    case UI_GLYPH_INFO:
        Ellipse(dc, left + 1, top + 1, right - 1, bottom - 1);
        StrokeLine(dc, cx, cy, cx, bottom - 6, color, line);
        StrokeLine(dc, cx, top + 6, cx, top + 6, color, 2);
        break;
    case UI_GLYPH_SOUND:
        points[0].x = left + 2; points[0].y = cy - 4;
        points[1].x = left + 6; points[1].y = cy - 4;
        points[2].x = cx; points[2].y = top + 4;
        points[3].x = cx; points[3].y = bottom - 4;
        points[4].x = left + 6; points[4].y = cy + 4;
        points[5].x = left + 2; points[5].y = cy + 4;
        Polyline(dc, points, 6);
        StrokeLine(dc, left + 2, cy - 4, left + 2, cy + 4, color, line);
        Arc(dc, cx - 2, cy - 8, right - 4, cy + 8, cx + 4, cy - 7, cx + 4, cy + 7);
        Arc(dc, cx + 1, cy - 11, right, cy + 11, cx + 8, cy - 10, cx + 8, cy + 10);
        break;
    case UI_GLYPH_ROCKET:
        Ellipse(dc, cx - 4, top + 2, cx + 5, top + 11);
        StrokeLine(dc, cx, top + 11, left + 6, bottom - 5, color, line);
        StrokeLine(dc, cx + 4, top + 9, right - 5, bottom - 9, color, line);
        StrokeLine(dc, left + 8, bottom - 8, left + 4, bottom - 4, color, line);
        StrokeLine(dc, left + 10, bottom - 7, left + 7, bottom - 3, color, line);
        StrokeLine(dc, right - 8, bottom - 10, right - 4, bottom - 7, color, line);
        StrokeLine(dc, cx - 1, bottom - 5, cx - 3, bottom - 1, color, line);
        StrokeLine(dc, cx + 2, bottom - 6, cx + 1, bottom - 1, color, line);
        break;
    case UI_GLYPH_DISK:
        RoundRect(dc, left + 1, top + 1, right - 1, bottom - 1, 3, 3);
        Rectangle(dc, left + 4, top + 2, right - 5, top + size / 3);
        Rectangle(dc, left + 4, bottom - size / 3, right - 4, bottom - 3);
        break;
    case UI_GLYPH_TRASH:
        Rectangle(dc, left + 4, top + 5, right - 4, bottom - 2);
        StrokeLine(dc, left + 2, top + 4, right - 2, top + 4, color, line);
        StrokeLine(dc, left + 6, top + 2, right - 6, top + 2, color, line);
        StrokeLine(dc, left + 6, top + 8, left + 6, bottom - 4, color, 1);
        StrokeLine(dc, right - 6, top + 8, right - 6, bottom - 4, color, 1);
        break;
    case UI_GLYPH_RESET:
        Arc(dc, left + 2, top + 3, right - 2, bottom - 1, right - 4, top + 7, left + 5, bottom - 4);
        StrokeLine(dc, left + 5, top + 4, left + 5, top + 9, color, line);
        StrokeLine(dc, left + 5, top + 4, left + 10, top + 4, color, line);
        break;
    case UI_GLYPH_STORE:
        RoundRect(dc, left + 1, top + size / 3, right - 1, bottom - 1, 3, 3);
        Arc(dc, left + 5, top + 1, right - 5, top + (size * 2) / 3, left + 5, top + size / 3 + 1, right - 5, top + size / 3 + 1);
        break;
    case UI_GLYPH_GITHUB:
        Ellipse(dc, left, top, right, bottom);
        rc.left = left;
        rc.top = top;
        rc.right = right;
        rc.bottom = bottom;
        DrawTextWithFont(dc, L"GH", rc, g_settings_small_font, color, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        break;
    case UI_GLYPH_DOCUMENT:
        Rectangle(dc, left + 4, top + 2, right - 4, bottom - 2);
        StrokeLine(dc, left + 7, top + size / 3, right - 7, top + size / 3, color, 1);
        StrokeLine(dc, left + 7, top + size / 2, right - 7, top + size / 2, color, 1);
        StrokeLine(dc, left + 7, top + (size * 2) / 3, right - 8, top + (size * 2) / 3, color, 1);
        break;
    default:
        break;
    }

    SelectObject(dc, old_brush);
    SelectObject(dc, old_pen);
    DeleteObject(pen);
}

static const wchar_t *GetCheckboxDescription(int id) {
    switch (id) {
    case IDC_SETTINGS_SOUND:
        return GetUiText(TXT_DESC_SOUND);
    case IDC_SETTINGS_AUTOSTART:
        return GetUiText(TXT_DESC_AUTOSTART);
    case IDC_SETTINGS_LOGGING:
        return GetUiText(TXT_DESC_LOGGING);
    default:
        return L"";
    }
}

static BOOL IsPrimarySettingsButton(int id) {
    return id == IDC_SELECTED_RECORD || id == IDC_LASTWORD_RECORD;
}

static BOOL IsStoreSettingsButton(int id) {
    return id == IDC_SETTINGS_STORE;
}

static BOOL IsSettingsCheckboxChecked(int id) {
    switch (id) {
    case IDC_SETTINGS_SOUND:
        return g_pending_settings.sound_enabled;
    case IDC_SETTINGS_AUTOSTART:
        return g_pending_settings.autostart_enabled;
    case IDC_SETTINGS_LOGGING:
        return g_pending_settings.logging_enabled;
    default:
        return FALSE;
    }
}

static BOOL IsSettingsCheckboxId(int id) {
    return id == IDC_SETTINGS_SOUND
        || id == IDC_SETTINGS_AUTOSTART
        || id == IDC_SETTINGS_LOGGING;
}

static BOOL IsCursorInCheckboxSquare(HWND control) {
    POINT pt;
    RECT rc;
    RECT box_rect;

    if (control == NULL || !GetCursorPos(&pt)) {
        return FALSE;
    }

    ScreenToClient(control, &pt);
    GetClientRect(control, &rc);

    box_rect.left = rc.left + 8;
    box_rect.top = rc.top + ((rc.bottom - rc.top) - 28) / 2;
    box_rect.right = box_rect.left + 28;
    box_rect.bottom = box_rect.top + 28;

    return PtInRect(&box_rect, pt);
}

static void ToggleSettingsCheckboxById(int id) {
    if (g_settings_syncing) {
        return;
    }

    switch (id) {
    case IDC_SETTINGS_SOUND:
        g_pending_settings.sound_enabled = !g_pending_settings.sound_enabled;
        ApplyPendingSettings();
        break;
    case IDC_SETTINGS_AUTOSTART:
        g_pending_settings.autostart_enabled = !g_pending_settings.autostart_enabled;
        ApplyPendingSettings();
        break;
    case IDC_SETTINGS_LOGGING:
        g_pending_settings.logging_enabled = !g_pending_settings.logging_enabled;
        ApplyPendingSettings();
        break;
    default:
        break;
    }
}

static UiGlyph GetCheckboxGlyph(int id) {
    switch (id) {
    case IDC_SETTINGS_SOUND:
        return UI_GLYPH_SOUND;
    case IDC_SETTINGS_AUTOSTART:
        return UI_GLYPH_ROCKET;
    case IDC_SETTINGS_LOGGING:
        return UI_GLYPH_DOCUMENT;
    default:
        return UI_GLYPH_NONE;
    }
}

static UiGlyph GetButtonGlyph(int id) {
    switch (id) {
    case IDC_SELECTED_RECORD:
    case IDC_LASTWORD_RECORD:
        return UI_GLYPH_DISK;
    case IDC_SELECTED_CLEAR:
    case IDC_LASTWORD_CLEAR:
        return UI_GLYPH_TRASH;
    case IDC_SELECTED_RESET:
    case IDC_LASTWORD_RESET:
        return UI_GLYPH_RESET;
    case IDC_SETTINGS_STORE:
        return UI_GLYPH_STORE;
    default:
        return UI_GLYPH_NONE;
    }
}

static int GetBadgeGlyphSize(UiGlyph glyph) {
    switch (glyph) {
    case UI_GLYPH_GLOBE:
        return 24;
    case UI_GLYPH_KEYBOARD:
        return 28;
    case UI_GLYPH_INFO:
        return 26;
    case UI_GLYPH_SOUND:
        return 27;
    case UI_GLYPH_ROCKET:
        return 25;
    case UI_GLYPH_DOCUMENT:
        return 27;
    default:
        return UI_BADGE_ICON_SIZE;
    }
}

static int GetBadgeGlyphOffsetX(UiGlyph glyph) {
    switch (glyph) {
    case UI_GLYPH_DOCUMENT:
        return 2;
    default:
        return 0;
    }
}

static int GetBadgeGlyphOffsetY(UiGlyph glyph) {
    switch (glyph) {
    case UI_GLYPH_INFO:
        return 1;
    default:
        return 0;
    }
}

static int GetButtonGlyphSize(UiGlyph glyph) {
    switch (glyph) {
    case UI_GLYPH_STORE:
        return 24;
    case UI_GLYPH_DISK:
    case UI_GLYPH_TRASH:
    case UI_GLYPH_RESET:
        return 21;
    default:
        return UI_BUTTON_ICON_SIZE;
    }
}

static void DrawCheckMark(HDC dc, int x, int y, COLORREF color) {
    HPEN old_pen;
    HPEN pen = CreatePen(PS_SOLID, 4, color);

    old_pen = (HPEN)SelectObject(dc, pen);
    MoveToEx(dc, x, y + 8, NULL);
    LineTo(dc, x + 7, y + 15);
    LineTo(dc, x + 20, y + 2);
    SelectObject(dc, old_pen);
    DeleteObject(pen);
}

static void DrawModernCheckbox(const DRAWITEMSTRUCT *draw_item) {
    int id = (int)draw_item->CtlID;
    HDC dc = draw_item->hDC;
    RECT rc = draw_item->rcItem;
    RECT title_rect;
    RECT desc_rect;
    RECT box_rect;
    RECT icon_circle;
    wchar_t text[128];
    BOOL checked = IsSettingsCheckboxChecked(id);
    BOOL selected = (draw_item->itemState & ODS_SELECTED) != 0;
    BOOL show_log_links = id == IDC_SETTINGS_LOGGING && ShouldShowDebugLogLinks();
    HBRUSH fill_brush = CreateSolidBrush(UI_CARD);
    HBRUSH box_brush = CreateSolidBrush(checked ? (selected ? UI_ACCENT_DARK : UI_ACCENT) : RGB(255, 255, 255));
    HPEN border_pen = CreatePen(PS_SOLID, 1, checked ? UI_ACCENT : UI_BORDER);
    HBRUSH icon_brush = CreateSolidBrush(id == IDC_SETTINGS_LOGGING ? RGB(241, 245, 249) : UI_ACCENT_LIGHT);
    HPEN icon_pen = CreatePen(PS_SOLID, 1, id == IDC_SETTINGS_LOGGING ? RGB(226, 232, 240) : RGB(203, 246, 241));
    HGDIOBJ old_brush;
    HGDIOBJ old_pen;
    int glyph_size = GetBadgeGlyphSize(GetCheckboxGlyph(id));

    GetWindowTextW(draw_item->hwndItem, text, sizeof(text) / sizeof(text[0]));

    FillRect(dc, &rc, fill_brush);

    box_rect.left = rc.left + 8;
    box_rect.top = rc.top + ((rc.bottom - rc.top) - 28) / 2;
    box_rect.right = box_rect.left + 28;
    box_rect.bottom = box_rect.top + 28;

    old_brush = SelectObject(dc, box_brush);
    old_pen = SelectObject(dc, border_pen);
    RoundRect(dc, box_rect.left, box_rect.top, box_rect.right, box_rect.bottom, 7, 7);
    SelectObject(dc, old_pen);
    SelectObject(dc, old_brush);

    if (checked) {
        DrawCheckMark(dc, box_rect.left + 4, box_rect.top + 5, RGB(255, 255, 255));
    }

    title_rect.left = rc.left + 56;
    title_rect.top = rc.top + (show_log_links ? 4 : 6);
    title_rect.right = rc.right - 72;
    title_rect.bottom = title_rect.top + 22;
    DrawTextWithFont(dc, text, title_rect, g_settings_heading_font, UI_TEXT, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

    desc_rect.left = title_rect.left;
    desc_rect.top = title_rect.bottom - (show_log_links ? 4 : 1);
    desc_rect.right = title_rect.right;
    desc_rect.bottom = show_log_links ? desc_rect.top + 14 : rc.bottom - 4;
    if (desc_rect.bottom < desc_rect.top + 12) {
        desc_rect.bottom = desc_rect.top + 12;
    }
    DrawTextWithFont(dc, GetCheckboxDescription(id), desc_rect, g_settings_small_font, UI_MUTED, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);

    icon_circle.left = rc.right - 60;
    icon_circle.top = rc.top + ((rc.bottom - rc.top) - UI_BADGE_SIZE) / 2;
    icon_circle.right = icon_circle.left + UI_BADGE_SIZE;
    icon_circle.bottom = icon_circle.top + UI_BADGE_SIZE;
    old_brush = SelectObject(dc, icon_brush);
    old_pen = SelectObject(dc, icon_pen);
    Ellipse(dc, icon_circle.left, icon_circle.top, icon_circle.right, icon_circle.bottom);
    SelectObject(dc, old_pen);
    SelectObject(dc, old_brush);
    DrawGlyph(
        dc,
        GetCheckboxGlyph(id),
        icon_circle.left + (UI_BADGE_SIZE - glyph_size) / 2,
        icon_circle.top + (UI_BADGE_SIZE - glyph_size) / 2,
        glyph_size,
        id == IDC_SETTINGS_LOGGING ? UI_ICON_MUTED : UI_ACCENT_DARK);

    DeleteObject(icon_pen);
    DeleteObject(icon_brush);
    DeleteObject(border_pen);
    DeleteObject(box_brush);
    DeleteObject(fill_brush);
}

static void DrawModernButton(const DRAWITEMSTRUCT *draw_item) {
    int id = (int)draw_item->CtlID;
    HDC dc = draw_item->hDC;
    RECT rc = draw_item->rcItem;
    RECT text_rect = rc;
    wchar_t text[96];
    BOOL primary = IsPrimarySettingsButton(id);
    BOOL store = IsStoreSettingsButton(id);
    BOOL selected = (draw_item->itemState & ODS_SELECTED) != 0;
    BOOL disabled = (draw_item->itemState & ODS_DISABLED) != 0;
    UiGlyph glyph = GetButtonGlyph(id);
    COLORREF fill = RGB(255, 255, 255);
    COLORREF border = UI_BORDER;
    COLORREF text_color = UI_TEXT;
    int border_width = 1;
    HBRUSH fill_brush;
    HPEN border_pen;
    HGDIOBJ old_brush;
    HGDIOBJ old_pen;

    if (primary) {
        fill = selected ? UI_ACCENT_DARK : UI_ACCENT;
        border = fill;
        text_color = RGB(255, 255, 255);
    } else if (store) {
        fill = selected ? UI_ACCENT_LIGHT : RGB(255, 255, 255);
        border = UI_ACCENT;
        text_color = UI_ACCENT_DARK;
        border_width = 2;
    } else if (selected) {
        fill = RGB(241, 245, 249);
        border = RGB(203, 213, 225);
    }

    if (disabled) {
        text_color = RGB(148, 163, 184);
    }

    GetWindowTextW(draw_item->hwndItem, text, sizeof(text) / sizeof(text[0]));
    fill_brush = CreateSolidBrush(fill);
    border_pen = CreatePen(PS_SOLID, border_width, border);
    if (store) {
        RECT inner = rc;
        HBRUSH border_brush = CreateSolidBrush(border);

        old_brush = SelectObject(dc, border_brush);
        old_pen = SelectObject(dc, GetStockObject(NULL_PEN));
        RoundRect(dc, rc.left, rc.top, rc.right, rc.bottom, UI_CONTROL_RADIUS, UI_CONTROL_RADIUS);
        SelectObject(dc, old_pen);
        SelectObject(dc, old_brush);

        InflateRect(&inner, -2, -2);
        old_brush = SelectObject(dc, fill_brush);
        old_pen = SelectObject(dc, GetStockObject(NULL_PEN));
        RoundRect(dc, inner.left, inner.top, inner.right, inner.bottom, UI_CONTROL_RADIUS - 2, UI_CONTROL_RADIUS - 2);
        SelectObject(dc, old_pen);
        SelectObject(dc, old_brush);

        DeleteObject(border_brush);
    } else {
        old_brush = SelectObject(dc, fill_brush);
        old_pen = SelectObject(dc, border_pen);
        RoundRect(dc, rc.left, rc.top, rc.right, rc.bottom, UI_CONTROL_RADIUS, UI_CONTROL_RADIUS);
        SelectObject(dc, old_pen);
        SelectObject(dc, old_brush);
    }

    InflateRect(&text_rect, -8, 0);
    if (store && glyph != UI_GLYPH_NONE) {
        RECT line_one_rect;
        RECT line_two_rect;
        int icon_size = GetButtonGlyphSize(glyph);
        int icon_left = rc.left + ((rc.right - rc.left) - icon_size) / 2;
        int icon_top = rc.top + 5;

        DrawGlyph(dc, glyph, icon_left, icon_top, icon_size, text_color);

        line_one_rect.left = rc.left + 4;
        line_one_rect.right = rc.right - 4;
        line_one_rect.top = rc.top + 29;
        line_one_rect.bottom = rc.top + 42;

        line_two_rect.left = rc.left + 4;
        line_two_rect.right = rc.right - 4;
        line_two_rect.top = rc.top + 42;
        line_two_rect.bottom = rc.bottom - 6;

        DrawTextWithFont(dc, L"Microsoft", line_one_rect, g_settings_small_font, text_color, DT_CENTER | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
        DrawTextWithFont(dc, L"Store", line_two_rect, g_settings_small_font, text_color, DT_CENTER | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
    } else if (glyph != UI_GLYPH_NONE) {
        SIZE text_size;
        int icon_size = GetButtonGlyphSize(glyph);
        int gap = 6;
        int content_width;
        int content_left;
        int icon_top;
        HFONT old_font = NULL;

        if (g_settings_button_font != NULL) {
            old_font = (HFONT)SelectObject(dc, g_settings_button_font);
        }
        GetTextExtentPoint32W(dc, text, (int)wcslen(text), &text_size);
        if (old_font != NULL) {
            SelectObject(dc, old_font);
        }
        content_width = icon_size + gap + text_size.cx;
        content_left = rc.left + ((rc.right - rc.left) - content_width) / 2;
        icon_top = rc.top + ((rc.bottom - rc.top) - icon_size) / 2;
        DrawGlyph(dc, glyph, content_left, icon_top, icon_size, text_color);
        text_rect.left = content_left + icon_size + gap;
        text_rect.right = rc.right - 6;
        DrawTextWithFont(dc, text, text_rect, g_settings_button_font, text_color, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
    } else {
        DrawTextWithFont(dc, text, text_rect, g_settings_button_font, text_color, DT_CENTER | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
    }

    DeleteObject(border_pen);
    DeleteObject(fill_brush);
}

static void DrawChevron(HDC dc, int x, int y, COLORREF color) {
    StrokeLine(dc, x, y, x + 5, y + 5, color, 1);
    StrokeLine(dc, x + 5, y + 5, x + 10, y, color, 1);
}

static void DrawLanguageSelector(const DRAWITEMSTRUCT *draw_item) {
    HDC dc = draw_item->hDC;
    RECT rc = draw_item->rcItem;
    RECT text_rect = rc;
    wchar_t text[64];
    BOOL selected = (draw_item->itemState & ODS_SELECTED) != 0;
    HBRUSH fill_brush = CreateSolidBrush(selected ? RGB(248, 250, 252) : RGB(255, 255, 255));
    HPEN border_pen = CreatePen(PS_SOLID, 1, selected ? RGB(203, 213, 225) : UI_BORDER);
    HGDIOBJ old_brush;
    HGDIOBJ old_pen;

    GetWindowTextW(draw_item->hwndItem, text, sizeof(text) / sizeof(text[0]));

    old_brush = SelectObject(dc, fill_brush);
    old_pen = SelectObject(dc, border_pen);
    RoundRect(dc, rc.left, rc.top, rc.right, rc.bottom, UI_CONTROL_RADIUS, UI_CONTROL_RADIUS);
    SelectObject(dc, old_pen);
    SelectObject(dc, old_brush);

    text_rect.left += 20;
    text_rect.right -= 44;
    DrawTextWithFont(dc, text, text_rect, g_settings_body_font, UI_TEXT, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
    DrawChevron(dc, rc.right - 30, rc.top + ((rc.bottom - rc.top) / 2) - 2, RGB(148, 163, 184));

    DeleteObject(border_pen);
    DeleteObject(fill_brush);
}

static void DrawHotkeyValue(const DRAWITEMSTRUCT *draw_item) {
    HDC dc = draw_item->hDC;
    RECT rc = draw_item->rcItem;
    RECT text_rect = rc;
    wchar_t text[128];
    HBRUSH fill_brush = CreateSolidBrush(RGB(255, 255, 255));
    HPEN border_pen = CreatePen(PS_SOLID, 1, UI_BORDER);
    HGDIOBJ old_brush;
    HGDIOBJ old_pen;

    GetWindowTextW(draw_item->hwndItem, text, sizeof(text) / sizeof(text[0]));

    old_brush = SelectObject(dc, fill_brush);
    old_pen = SelectObject(dc, border_pen);
    RoundRect(dc, rc.left, rc.top, rc.right, rc.bottom, UI_CONTROL_RADIUS, UI_CONTROL_RADIUS);
    SelectObject(dc, old_pen);
    SelectObject(dc, old_brush);

    text_rect.left += 20;
    text_rect.right -= 20;
    DrawTextWithFont(dc, text, text_rect, g_settings_body_font, UI_TEXT, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);

    DeleteObject(border_pen);
    DeleteObject(fill_brush);
}

static HFONT CreateAccentWordFont(HFONT base_font) {
    LOGFONTW lf;

    if (base_font == NULL) {
        return NULL;
    }

    ZeroMemory(&lf, sizeof(lf));
    if (GetObjectW(base_font, sizeof(lf), &lf) == 0) {
        return NULL;
    }

    lf.lfWeight = min(FW_BOLD, lf.lfWeight + 120);
    return CreateFontIndirectW(&lf);
}

static void DrawCwitcherWord(HDC dc, int x, int y, HFONT font, COLORREF accent, COLORREF text_color) {
    SIZE c_size;
    HFONT old_font = NULL;
    HFONT accent_font = NULL;

    accent_font = CreateAccentWordFont(font);
    if (accent_font != NULL) {
        old_font = (HFONT)SelectObject(dc, accent_font);
    } else if (font != NULL) {
        old_font = (HFONT)SelectObject(dc, font);
    }

    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, accent);
    TextOutW(dc, x, y, L"C", 1);
    GetTextExtentPoint32W(dc, L"C", 1, &c_size);

    if (font != NULL) {
        SelectObject(dc, font);
    }
    SetTextColor(dc, text_color);
    TextOutW(dc, x + c_size.cx - 2, y, L"witcher", 7);

    if (old_font != NULL) {
        SelectObject(dc, old_font);
    }
    if (accent_font != NULL) {
        DeleteObject(accent_font);
    }
}

static int DrawTopWordmark(HDC dc, int x, int y) {
    SIZE text_size = { 0 };
    HFONT old_font = NULL;

    if (g_ui_asset_logo_wordmark_header.bitmap != NULL) {
        DrawBitmapAssetRect(dc, &g_ui_asset_logo_wordmark_header, x, y - 6, 156, 48);
        return x + 156;
    }

    DrawCwitcherWord(dc, x, y + 1, g_settings_title_font, UI_ACCENT, UI_TEXT);
    old_font = (HFONT)SelectObject(dc, g_settings_title_font);
    GetTextExtentPoint32W(dc, L"Cwitcher", 8, &text_size);
    if (old_font != NULL) {
        SelectObject(dc, old_font);
    }
    return x + text_size.cx;
}

static void DrawCwMonogram(HDC dc, RECT rect) {
    HFONT font = CreateSettingsFont(g_settings_window, 24, FW_SEMIBOLD, FALSE);
    HFONT old_font = NULL;
    SIZE c_size;
    SIZE w_size;
    int overlap = 2;
    int total_width;
    int x;
    int y;

    if (font != NULL) {
        old_font = (HFONT)SelectObject(dc, font);
    }

    SetBkMode(dc, TRANSPARENT);
    GetTextExtentPoint32W(dc, L"C", 1, &c_size);
    GetTextExtentPoint32W(dc, L"W", 1, &w_size);
    total_width = c_size.cx + w_size.cx - overlap;
    x = rect.left + ((rect.right - rect.left) - total_width) / 2;
    y = rect.top + ((rect.bottom - rect.top) - c_size.cy) / 2 - 1;

    SetTextColor(dc, UI_ACCENT);
    TextOutW(dc, x, y, L"C", 1);
    SetTextColor(dc, UI_TEXT);
    TextOutW(dc, x + c_size.cx - overlap, y, L"W", 1);

    if (old_font != NULL) {
        SelectObject(dc, old_font);
    }
    if (font != NULL) {
        DeleteObject(font);
    }
}

static void DrawBrandLogo(const DRAWITEMSTRUCT *draw_item) {
    HDC dc = draw_item->hDC;
    RECT rc = draw_item->rcItem;
    RECT text_rect = rc;

    FillRect(dc, &rc, g_settings_card_brush != NULL ? g_settings_card_brush : GetSysColorBrush(COLOR_WINDOW));
    SetBkMode(dc, TRANSPARENT);

    if (g_ui_asset_logo_wordmark_brand.bitmap != NULL) {
        DrawBitmapAssetRect(dc, &g_ui_asset_logo_wordmark_brand, rc.left, rc.top + 1, 164, 50);
    } else {
        DrawCwitcherWord(dc, rc.left + 2, rc.top - 10, g_settings_brand_font, UI_ACCENT, UI_TEXT);
    }

    text_rect.left = rc.left + 2;
    text_rect.top = rc.top + 52;
    text_rect.right = rc.right;
    text_rect.bottom = rc.bottom;
    DrawTextWithFont(dc, GetUiText(TXT_BRAND_TAGLINE), text_rect, g_settings_small_font, UI_MUTED, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS);
}

static void DrawAccentBadge(HDC dc, int left, int top, const wchar_t *text) {
    RECT badge;
    RECT text_rect;
    HBRUSH old_brush;
    HPEN old_pen;
    HBRUSH brush = CreateSolidBrush(UI_ACCENT_LIGHT);
    HPEN pen = CreatePen(PS_SOLID, 1, RGB(203, 246, 241));

    badge.left = left;
    badge.top = top;
    badge.right = left + UI_BADGE_SIZE;
    badge.bottom = top + UI_BADGE_SIZE;

    old_brush = (HBRUSH)SelectObject(dc, brush);
    old_pen = (HPEN)SelectObject(dc, pen);
    Ellipse(dc, badge.left, badge.top, badge.right, badge.bottom);
    SelectObject(dc, old_pen);
    SelectObject(dc, old_brush);

    text_rect = badge;
    DrawTextWithFont(dc, text, text_rect, g_settings_button_font, UI_ACCENT_DARK, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    DeleteObject(pen);
    DeleteObject(brush);
}

static void DrawGlyphBadge(HDC dc, int left, int top, UiGlyph glyph) {
    RECT badge;
    HBRUSH old_brush;
    HPEN old_pen;
    HBRUSH brush = CreateSolidBrush(UI_ACCENT_LIGHT);
    HPEN pen = CreatePen(PS_SOLID, 1, RGB(203, 246, 241));
    int glyph_size = GetBadgeGlyphSize(glyph);

    badge.left = left;
    badge.top = top;
    badge.right = left + UI_BADGE_SIZE;
    badge.bottom = top + UI_BADGE_SIZE;

    old_brush = (HBRUSH)SelectObject(dc, brush);
    old_pen = (HPEN)SelectObject(dc, pen);
    Ellipse(dc, badge.left, badge.top, badge.right, badge.bottom);
    SelectObject(dc, old_pen);
    SelectObject(dc, old_brush);

    DrawGlyph(
        dc,
        glyph,
        left + (UI_BADGE_SIZE - glyph_size) / 2 + GetBadgeGlyphOffsetX(glyph),
        top + (UI_BADGE_SIZE - glyph_size) / 2 + GetBadgeGlyphOffsetY(glyph),
        glyph_size,
        UI_ACCENT_DARK);

    DeleteObject(pen);
    DeleteObject(brush);
}

static void DrawHotkeyCardText(HDC dc, int top, const wchar_t *title, const wchar_t *description) {
    RECT title_rect;
    RECT desc_rect;

    DrawGlyphBadge(dc, UI_ICON_LEFT, top + 22, UI_GLYPH_KEYBOARD);

    title_rect.left = UI_TEXT_LEFT;
    title_rect.top = top + 14;
    title_rect.right = UI_CARD_RIGHT - 28;
    title_rect.bottom = top + 42;
    DrawTextWithFont(dc, title, title_rect, g_settings_heading_font, UI_TEXT, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS);

    desc_rect.left = title_rect.left;
    desc_rect.top = top + 40;
    desc_rect.right = title_rect.right;
    desc_rect.bottom = top + 62;
    DrawTextWithFont(dc, description, desc_rect, g_settings_small_font, UI_MUTED, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS);
}

static void DrawSettingsChrome(HWND hwnd, HDC dc) {
    RECT rc;
    RECT text_rect;
    int wordmark_right;

    GetClientRect(hwnd, &rc);
    FillRect(dc, &rc, g_settings_background_brush != NULL ? g_settings_background_brush : GetSysColorBrush(COLOR_WINDOW));

    wordmark_right = DrawTopWordmark(dc, UI_MARGIN, 21);
    text_rect.left = wordmark_right + 12;
    text_rect.top = 28;
    text_rect.right = SETTINGS_CLIENT_WIDTH - UI_MARGIN;
    text_rect.bottom = 52;
    DrawTextWithFont(dc, GetActiveUiLanguage() == UI_LANGUAGE_UK ? L"Налаштування" : L"Settings", text_rect, g_settings_body_font, RGB(82, 92, 108), DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

    DrawRoundedCard(dc, UI_CARD_LEFT, 74, UI_CARD_RIGHT, 194, UI_CARD_RADIUS);
    StrokeLine(dc, UI_CARD_LEFT + 1, 134, UI_CARD_RIGHT - 1, 134, UI_BORDER, 1);
    DrawRoundedCard(dc, UI_CARD_LEFT, 250, UI_CARD_RIGHT, 486, UI_CARD_RADIUS);
    StrokeLine(dc, UI_CARD_LEFT + 24, 370, UI_CARD_RIGHT - 24, 370, UI_BORDER, 1);
    DrawRoundedCard(dc, UI_CARD_LEFT, 494, UI_CARD_RIGHT, 550, UI_CARD_RADIUS);
    DrawRoundedCard(dc, UI_CARD_LEFT, 558, UI_CARD_RIGHT, 682, UI_CARD_RADIUS);
    DrawRoundedCard(dc, UI_CARD_LEFT, 690, UI_CARD_RIGHT, 748, UI_CARD_RADIUS);

    DrawGlyphBadge(dc, UI_ICON_LEFT, 202, UI_GLYPH_GLOBE);
    text_rect.left = UI_TEXT_LEFT;
    text_rect.top = 204;
    text_rect.right = 360;
    text_rect.bottom = 240;
    DrawTextWithFont(dc, GetUiText(TXT_LABEL_LANGUAGE), text_rect, g_settings_heading_font, UI_TEXT, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

    DrawHotkeyCardText(dc, 250, GetUiText(TXT_GROUP_SELECTED), GetUiText(TXT_DESC_SELECTED));
    DrawHotkeyCardText(dc, 364, GetUiText(TXT_GROUP_LASTWORD), GetUiText(TXT_DESC_LASTWORD));

    DrawGlyphBadge(dc, UI_ICON_LEFT, 501, UI_GLYPH_INFO);

    text_rect.left = UI_MARGIN;
    text_rect.top = 762;
    text_rect.right = 220;
    text_rect.bottom = SETTINGS_CLIENT_HEIGHT - 5;
    DrawTextWithFont(dc, GetUiText(TXT_VERSION_LABEL), text_rect, g_settings_small_font, RGB(166, 176, 191), DT_LEFT | DT_BOTTOM | DT_SINGLELINE | DT_END_ELLIPSIS);

    if (g_icon_github != NULL) {
        DrawIconEx(dc, 60, 643, g_icon_github, 20, 20, 0, NULL, DI_NORMAL);
    } else {
        DrawGlyph(dc, UI_GLYPH_GITHUB, 60, 643, 20, RGB(17, 24, 39));
    }
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
    if (g_settings_controls.language_combo == NULL) {
        return;
    }

    g_settings_syncing = TRUE;
    SetWindowTextW(
        g_settings_controls.language_combo,
        g_pending_settings.ui_language == UI_LANGUAGE_UK ? GetUiText(TXT_LANGUAGE_UKRAINIAN) : GetUiText(TXT_LANGUAGE_ENGLISH));
    g_settings_syncing = FALSE;
}

static BOOL ShouldShowDebugLogLinks(void) {
    return DebugLogFileExists();
}

static void ShortenMiddlePath(const wchar_t *path, wchar_t *buffer, size_t capacity, size_t max_chars) {
    size_t length;
    size_t prefix_count;
    size_t suffix_count;

    if (buffer == NULL || capacity == 0) {
        return;
    }

    buffer[0] = L'\0';
    if (path == NULL) {
        return;
    }

    length = wcslen(path);
    if (length <= max_chars || capacity <= max_chars) {
        StringCchCopyW(buffer, capacity, path);
        return;
    }

    if (max_chars < 8) {
        StringCchCopyW(buffer, capacity, L"...");
        return;
    }

    prefix_count = (max_chars - 3) / 2;
    suffix_count = max_chars - 3 - prefix_count;

    wcsncpy_s(buffer, capacity, path, prefix_count);
    StringCchCatW(buffer, capacity, L"...");
    StringCchCatW(buffer, capacity, path + length - suffix_count);
}

static void UpdateDebugLogLinkControls(void) {
    wchar_t path[MAX_PATH];
    wchar_t display_path[64];
    BOOL visible;
    HDC dc;
    HFONT old_font = NULL;
    SIZE path_size = { 0 };
    int path_width;
    int delete_left;

    visible = ShouldShowDebugLogLinks();

    if (visible && BuildDebugLogFilePath(path, sizeof(path) / sizeof(path[0]))) {
        ShortenMiddlePath(path, display_path, sizeof(display_path) / sizeof(display_path[0]), 50);
        if (g_settings_controls.log_path_link != NULL) {
            SetWindowTextW(g_settings_controls.log_path_link, display_path);
        }
        if (g_settings_controls.log_delete_link != NULL) {
            SetWindowTextW(g_settings_controls.log_delete_link, GetUiText(TXT_LOG_DELETE));
        }

        if (g_settings_window != NULL) {
            dc = GetDC(g_settings_window);
            if (dc != NULL) {
                if (g_settings_link_font != NULL) {
                    old_font = (HFONT)SelectObject(dc, g_settings_link_font);
                }
                GetTextExtentPoint32W(dc, display_path, (int)wcslen(display_path), &path_size);
                if (old_font != NULL) {
                    SelectObject(dc, old_font);
                }
                ReleaseDC(g_settings_window, dc);
            }
        }

        path_width = min(path_size.cx + 4, 360);
        delete_left = 100 + path_width + 10;
        if (g_settings_controls.log_path_link != NULL) {
            MoveWindow(g_settings_controls.log_path_link, 100, 726, path_width, 18, TRUE);
            SetWindowPos(g_settings_controls.log_path_link, HWND_TOP, 0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        }
        if (g_settings_controls.log_delete_link != NULL) {
            MoveWindow(g_settings_controls.log_delete_link, delete_left, 726, 80, 18, TRUE);
            SetWindowPos(g_settings_controls.log_delete_link, HWND_TOP, 0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        }
    }

    if (g_settings_controls.log_path_link != NULL) {
        ShowWindow(g_settings_controls.log_path_link, visible ? SW_SHOW : SW_HIDE);
    }
    if (g_settings_controls.log_delete_link != NULL) {
        ShowWindow(g_settings_controls.log_delete_link, visible ? SW_SHOW : SW_HIDE);
    }
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

    PopulateLanguageCombo();
    UpdateDebugLogLinkControls();

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
    if (g_settings_controls.log_delete_link != NULL) {
        SetWindowTextW(g_settings_controls.log_delete_link, GetUiText(TXT_LOG_DELETE));
    }

    SetWindowTextW(GetDlgItem(g_settings_window, IDC_SELECTED_RECORD), GetUiText(TXT_BUTTON_RECORD));
    SetWindowTextW(GetDlgItem(g_settings_window, IDC_SELECTED_CLEAR), GetUiText(TXT_BUTTON_CLEAR));
    SetWindowTextW(GetDlgItem(g_settings_window, IDC_SELECTED_RESET), GetUiText(TXT_BUTTON_RESET));
    SetWindowTextW(GetDlgItem(g_settings_window, IDC_LASTWORD_RECORD), GetUiText(TXT_BUTTON_RECORD));
    SetWindowTextW(GetDlgItem(g_settings_window, IDC_LASTWORD_CLEAR), GetUiText(TXT_BUTTON_CLEAR));
    SetWindowTextW(GetDlgItem(g_settings_window, IDC_LASTWORD_RESET), GetUiText(TXT_BUTTON_RESET));

    PopulateLanguageCombo();
}

static void InvalidateSettingsControl(HWND control) {
    if (control != NULL) {
        InvalidateRect(control, NULL, TRUE);
    }
}

static void RefreshSettingsWindow(void) {
    ApplyLocalizedControlText();
    UpdateSettingsWindowState();
    InvalidateRect(g_settings_window, NULL, TRUE);
    InvalidateSettingsControl(g_settings_controls.sound_checkbox);
    InvalidateSettingsControl(g_settings_controls.autostart_checkbox);
    InvalidateSettingsControl(g_settings_controls.logging_checkbox);
    InvalidateSettingsControl(g_settings_controls.store_button);
    InvalidateSettingsControl(g_settings_controls.brand_logo);
    InvalidateSettingsControl(g_settings_controls.language_combo);
    InvalidateSettingsControl(g_settings_controls.selected_value);
    InvalidateSettingsControl(g_settings_controls.lastword_value);
    InvalidateSettingsControl(GetDlgItem(g_settings_window, IDC_SELECTED_RECORD));
    InvalidateSettingsControl(GetDlgItem(g_settings_window, IDC_SELECTED_CLEAR));
    InvalidateSettingsControl(GetDlgItem(g_settings_window, IDC_SELECTED_RESET));
    InvalidateSettingsControl(GetDlgItem(g_settings_window, IDC_LASTWORD_RECORD));
    InvalidateSettingsControl(GetDlgItem(g_settings_window, IDC_LASTWORD_CLEAR));
    InvalidateSettingsControl(GetDlgItem(g_settings_window, IDC_LASTWORD_RESET));
    InvalidateSettingsControl(g_settings_controls.log_path_link);
    InvalidateSettingsControl(g_settings_controls.log_delete_link);
}

static void ApplyPendingSettings(void) {
    BOOL logging_was_enabled = g_settings.logging_enabled;

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

    if (!logging_was_enabled && g_settings.logging_enabled) {
        LogDebug(L"Debug logging enabled.");
    }

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

static void ShowLanguagePopupMenu(void) {
    HMENU menu;
    RECT rect;
    UINT command;

    if (g_settings_window == NULL || g_settings_controls.language_combo == NULL) {
        return;
    }

    menu = CreatePopupMenu();
    if (menu == NULL) {
        return;
    }

    AppendMenuW(menu,
        MF_STRING | (g_pending_settings.ui_language == UI_LANGUAGE_EN ? MF_CHECKED : 0),
        SETTINGS_LANGUAGE_MENU_EN,
        GetUiText(TXT_LANGUAGE_ENGLISH));
    AppendMenuW(menu,
        MF_STRING | (g_pending_settings.ui_language == UI_LANGUAGE_UK ? MF_CHECKED : 0),
        SETTINGS_LANGUAGE_MENU_UK,
        GetUiText(TXT_LANGUAGE_UKRAINIAN));

    GetWindowRect(g_settings_controls.language_combo, &rect);
    SetForegroundWindow(g_settings_window);
    command = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN, rect.left, rect.bottom + 4, 0, g_settings_window, NULL);
    DestroyMenu(menu);

    if (command == SETTINGS_LANGUAGE_MENU_EN || command == SETTINGS_LANGUAGE_MENU_UK) {
        int new_language = command == SETTINGS_LANGUAGE_MENU_UK ? UI_LANGUAGE_UK : UI_LANGUAGE_EN;
        if (g_pending_settings.ui_language != new_language) {
            g_pending_settings.ui_language = new_language;
            ApplyPendingSettings();
        }
    }
}

static void OpenDebugLogInExplorer(HWND hwnd) {
    wchar_t path[MAX_PATH];
    wchar_t parameters[MAX_PATH + 16];

    if (!BuildDebugLogFilePath(path, sizeof(path) / sizeof(path[0])) || !DebugLogFileExists()) {
        RefreshSettingsWindow();
        return;
    }

    if (FAILED(StringCchPrintfW(parameters, sizeof(parameters) / sizeof(parameters[0]), L"/select,\"%ls\"", path))) {
        return;
    }

    ShellExecuteW(hwnd, L"open", L"explorer.exe", parameters, NULL, SW_SHOWNORMAL);
}

static void DeleteDebugLogFromSettings(HWND hwnd) {
    if (!DeleteDebugLogFile()) {
        MessageBoxW(hwnd, L"Failed to delete the debug log file.", L"Cwitcher", MB_OK | MB_ICONWARNING);
    }

    RefreshSettingsWindow();
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
    HWND control;

    control = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_OWNERDRAW | SS_NOTIFY,
        44, 84, 632, 48, hwnd, (HMENU)(INT_PTR)IDC_SETTINGS_SOUND, g_instance, NULL);
    g_settings_controls.sound_checkbox = control;
    ApplyButtonFont(control);

    control = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_OWNERDRAW | SS_NOTIFY,
        44, 140, 632, 48, hwnd, (HMENU)(INT_PTR)IDC_SETTINGS_AUTOSTART, g_instance, NULL);
    g_settings_controls.autostart_checkbox = control;
    ApplyButtonFont(control);

    control = CreateWindowExW(0, L"BUTTON", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW,
        390, 200, 302, 44, hwnd, (HMENU)(INT_PTR)IDC_SETTINGS_LANGUAGE_COMBO, g_instance, NULL);
    g_settings_controls.language_combo = control;
    ApplyDefaultGuiFont(control);

    control = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_OWNERDRAW,
        64, 324, 270, UI_HOTKEY_CONTROL_HEIGHT, hwnd, (HMENU)(INT_PTR)IDC_SELECTED_VALUE, g_instance, NULL);
    g_settings_controls.selected_value = control;
    ApplyDefaultGuiFont(control);

    control = CreateWindowExW(0, L"BUTTON", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW,
        348, 324, 112, UI_HOTKEY_CONTROL_HEIGHT, hwnd, (HMENU)(INT_PTR)IDC_SELECTED_RECORD, g_instance, NULL);
    ApplyButtonFont(control);

    control = CreateWindowExW(0, L"BUTTON", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW,
        466, 324, 96, UI_HOTKEY_CONTROL_HEIGHT, hwnd, (HMENU)(INT_PTR)IDC_SELECTED_CLEAR, g_instance, NULL);
    ApplyButtonFont(control);

    control = CreateWindowExW(0, L"BUTTON", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW,
        568, 324, 104, UI_HOTKEY_CONTROL_HEIGHT, hwnd, (HMENU)(INT_PTR)IDC_SELECTED_RESET, g_instance, NULL);
    ApplyButtonFont(control);

    control = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_OWNERDRAW,
        64, 432, 270, UI_HOTKEY_CONTROL_HEIGHT, hwnd, (HMENU)(INT_PTR)IDC_LASTWORD_VALUE, g_instance, NULL);
    g_settings_controls.lastword_value = control;
    ApplyDefaultGuiFont(control);

    control = CreateWindowExW(0, L"BUTTON", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW,
        348, 432, 112, UI_HOTKEY_CONTROL_HEIGHT, hwnd, (HMENU)(INT_PTR)IDC_LASTWORD_RECORD, g_instance, NULL);
    ApplyButtonFont(control);

    control = CreateWindowExW(0, L"BUTTON", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW,
        466, 432, 96, UI_HOTKEY_CONTROL_HEIGHT, hwnd, (HMENU)(INT_PTR)IDC_LASTWORD_CLEAR, g_instance, NULL);
    ApplyButtonFont(control);

    control = CreateWindowExW(0, L"BUTTON", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW,
        568, 432, 104, UI_HOTKEY_CONTROL_HEIGHT, hwnd, (HMENU)(INT_PTR)IDC_LASTWORD_RESET, g_instance, NULL);
    ApplyButtonFont(control);

    control = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE,
        106, 512, 552, 24, hwnd, (HMENU)(INT_PTR)IDC_SETTINGS_STATUS, g_instance, NULL);
    g_settings_controls.status_value = control;
    ApplyFont(control, g_settings_small_font);

    control = CreateWindowExW(0, L"BUTTON", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW,
        596, 586, UI_STORE_BUTTON_SIZE, UI_STORE_BUTTON_SIZE, hwnd, (HMENU)(INT_PTR)IDC_SETTINGS_STORE, g_instance, NULL);
    g_settings_controls.store_button = control;
    ApplyButtonFont(control);

    control = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_OWNERDRAW,
        60, 572, 430, 72, hwnd, (HMENU)(INT_PTR)IDC_SETTINGS_BRAND, g_instance, NULL);
    g_settings_controls.brand_logo = control;

    control = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOTIFY,
        88, 643, 356, 20, hwnd, (HMENU)(INT_PTR)IDC_SETTINGS_GITHUB, g_instance, NULL);
    g_settings_controls.github_link = control;
    ApplyLinkFont(control);

    control = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_OWNERDRAW | SS_NOTIFY,
        44, 692, 632, 46, hwnd, (HMENU)(INT_PTR)IDC_SETTINGS_LOGGING, g_instance, NULL);
    g_settings_controls.logging_checkbox = control;
    ApplyButtonFont(control);

    control = CreateWindowExW(WS_EX_TRANSPARENT, L"STATIC", L"", WS_CHILD | SS_LEFT | SS_NOTIFY,
        100, 726, 360, 18, hwnd, (HMENU)(INT_PTR)IDC_SETTINGS_LOG_PATH, g_instance, NULL);
    g_settings_controls.log_path_link = control;
    ApplyLinkFont(control);

    control = CreateWindowExW(WS_EX_TRANSPARENT, L"STATIC", L"", WS_CHILD | SS_LEFT | SS_NOTIFY,
        470, 726, 80, 18, hwnd, (HMENU)(INT_PTR)IDC_SETTINGS_LOG_DELETE, g_instance, NULL);
    g_settings_controls.log_delete_link = control;
    ApplyLinkFont(control);
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
    return control != NULL
        && (control == g_settings_controls.github_link
            || control == g_settings_controls.log_path_link
            || control == g_settings_controls.log_delete_link);
}

static BOOL IsSettingsCheckboxControl(HWND control) {
    return control != NULL
        && (control == g_settings_controls.sound_checkbox
            || control == g_settings_controls.autostart_checkbox
            || control == g_settings_controls.logging_checkbox);
}

static BOOL IsInteractiveSettingsControl(HWND control) {
    if (control == NULL) {
        return FALSE;
    }

    return control == g_settings_controls.language_combo
        || control == g_settings_controls.store_button
        || control == GetDlgItem(g_settings_window, IDC_SELECTED_RECORD)
        || control == GetDlgItem(g_settings_window, IDC_SELECTED_CLEAR)
        || control == GetDlgItem(g_settings_window, IDC_SELECTED_RESET)
        || control == GetDlgItem(g_settings_window, IDC_LASTWORD_RECORD)
        || control == GetDlgItem(g_settings_window, IDC_LASTWORD_CLEAR)
        || control == GetDlgItem(g_settings_window, IDC_LASTWORD_RESET);
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
        EnsureSettingsUiResources(hwnd);
        CreateSettingsControls(hwnd);
        RefreshSettingsWindow();
        return 0;

    case WM_ERASEBKGND:
        return 1;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(hwnd, &ps);
        EnsureSettingsUiResources(hwnd);
        DrawSettingsChrome(hwnd, dc);
        EndPaint(hwnd, &ps);
        return 0;
    }

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
            if (!g_settings_syncing && HIWORD(wParam) == STN_CLICKED
                && IsCursorInCheckboxSquare(g_settings_controls.sound_checkbox)) {
                ToggleSettingsCheckboxById(IDC_SETTINGS_SOUND);
            }
            return 0;
        case IDC_SETTINGS_AUTOSTART:
            if (!g_settings_syncing && HIWORD(wParam) == STN_CLICKED
                && IsCursorInCheckboxSquare(g_settings_controls.autostart_checkbox)) {
                ToggleSettingsCheckboxById(IDC_SETTINGS_AUTOSTART);
            }
            return 0;
        case IDC_SETTINGS_LOGGING:
            if (!g_settings_syncing && HIWORD(wParam) == STN_CLICKED
                && IsCursorInCheckboxSquare(g_settings_controls.logging_checkbox)) {
                ToggleSettingsCheckboxById(IDC_SETTINGS_LOGGING);
            }
            return 0;
        case IDC_SETTINGS_LANGUAGE_COMBO:
            if (!g_settings_syncing && HIWORD(wParam) == BN_CLICKED) {
                ShowLanguagePopupMenu();
            }
            return 0;
        case IDC_SETTINGS_GITHUB:
            ShellExecuteW(hwnd, L"open", GITHUB_REPOSITORY_URL, NULL, NULL, SW_SHOWNORMAL);
            return 0;
        case IDC_SETTINGS_LOG_PATH:
            OpenDebugLogInExplorer(hwnd);
            return 0;
        case IDC_SETTINGS_LOG_DELETE:
            DeleteDebugLogFromSettings(hwnd);
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
            SetTextColor(dc, UI_TEXT);
            SetBkColor(dc, RGB(255, 255, 255));
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
            SetTextColor(dc, UI_ACCENT_DARK);
            if (control == g_settings_controls.log_path_link || control == g_settings_controls.log_delete_link) {
                return (LRESULT)GetStockObject(HOLLOW_BRUSH);
            }
            return (LRESULT)(g_settings_card_brush != NULL ? g_settings_card_brush : GetSysColorBrush(COLOR_WINDOW));
        }

        if (IsSettingsValueControl(control)) {
            SetTextColor(dc, control == g_settings_controls.status_value ? UI_MUTED : UI_TEXT);
            return (LRESULT)(g_settings_card_brush != NULL ? g_settings_card_brush : GetSysColorBrush(COLOR_WINDOW));
        }
        break;
    }

    case WM_MEASUREITEM:
    {
        MEASUREITEMSTRUCT *measure = (MEASUREITEMSTRUCT *)lParam;
        if (measure != NULL && measure->CtlID == IDC_SETTINGS_LANGUAGE_COMBO) {
            measure->itemHeight = 30;
            return TRUE;
        }
        break;
    }

    case WM_DRAWITEM:
        if (wParam == IDC_SETTINGS_BRAND) {
            DrawBrandLogo((const DRAWITEMSTRUCT *)lParam);
            return TRUE;
        }
        if (wParam == IDC_SETTINGS_LANGUAGE_COMBO) {
            DrawLanguageSelector((const DRAWITEMSTRUCT *)lParam);
            return TRUE;
        }
        if (wParam == IDC_SELECTED_VALUE || wParam == IDC_LASTWORD_VALUE) {
            DrawHotkeyValue((const DRAWITEMSTRUCT *)lParam);
            return TRUE;
        }
        if (wParam == IDC_SETTINGS_SOUND || wParam == IDC_SETTINGS_AUTOSTART || wParam == IDC_SETTINGS_LOGGING) {
            DrawModernCheckbox((const DRAWITEMSTRUCT *)lParam);
            return TRUE;
        }
        if (wParam == IDC_SELECTED_RECORD
            || wParam == IDC_SELECTED_CLEAR
            || wParam == IDC_SELECTED_RESET
            || wParam == IDC_LASTWORD_RECORD
            || wParam == IDC_LASTWORD_CLEAR
            || wParam == IDC_LASTWORD_RESET
            || wParam == IDC_SETTINGS_STORE) {
            DrawModernButton((const DRAWITEMSTRUCT *)lParam);
            return TRUE;
        }
        break;

    case WM_SETCURSOR:
        if (IsSettingsLinkControl((HWND)wParam)) {
            SetCursor(LoadCursorW(NULL, IDC_HAND));
            return TRUE;
        }
        if (IsSettingsCheckboxControl((HWND)wParam) && IsCursorInCheckboxSquare((HWND)wParam)) {
            SetCursor(LoadCursorW(NULL, IDC_HAND));
            return TRUE;
        }
        if (IsInteractiveSettingsControl((HWND)wParam)) {
            SetCursor(LoadCursorW(NULL, IDC_HAND));
            return TRUE;
        }
        break;

    case WM_NCDESTROY:
        ZeroMemory(&g_settings_controls, sizeof(g_settings_controls));
        g_capture_target = CAPTURE_NONE;
        g_settings_window = NULL;
        g_settings_syncing = FALSE;
        DestroySettingsUiResources();
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
        wc.hbrBackground = g_settings_background_brush != NULL ? g_settings_background_brush : (HBRUSH)(COLOR_WINDOW + 1);
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
    rect.right = SETTINGS_CLIENT_WIDTH;
    rect.bottom = SETTINGS_CLIENT_HEIGHT;
    AdjustWindowRectEx(&rect, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN, FALSE, WS_EX_APPWINDOW);

    g_settings_window = CreateWindowExW(
        WS_EX_APPWINDOW,
        SETTINGS_CLASS_NAME,
        GetUiText(TXT_WINDOW_TITLE),
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN,
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
    LoadUiBitmapAssets();
    g_icon_brand = (HICON)LoadImageW(g_instance, MAKEINTRESOURCEW(IDI_APP_ICON), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
    if (g_icon_brand == NULL) {
        g_icon_brand = CreateBrandIcon();
    }
    g_icon_github = (HICON)LoadImageW(g_instance, MAKEINTRESOURCEW(IDI_GITHUB_ICON), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
}

void DestroyTrayIcons(void) {
    DestroyUiBitmapAssets();
    if (g_icon_en != NULL) DestroyIcon(g_icon_en);
    if (g_icon_ru != NULL) DestroyIcon(g_icon_ru);
    if (g_icon_uk != NULL) DestroyIcon(g_icon_uk);
    if (g_icon_unknown != NULL) DestroyIcon(g_icon_unknown);
    if (g_icon_brand != NULL) DestroyIcon(g_icon_brand);
    if (g_icon_github != NULL) DestroyIcon(g_icon_github);
    g_icon_en = NULL;
    g_icon_ru = NULL;
    g_icon_uk = NULL;
    g_icon_unknown = NULL;
    g_icon_brand = NULL;
    g_icon_github = NULL;
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
