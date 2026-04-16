#ifndef KEYBOARD_SWITCHER_C_APP_H
#define KEYBOARD_SWITCHER_C_APP_H

#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strsafe.h>
#include <wchar.h>
#include <stdarg.h>
#include "resource.h"

#define APP_NAME L"KeyboardSwitcherC"
#define APP_CLASS_NAME L"KeyboardSwitcherCWindow"
#define SETTINGS_CLASS_NAME L"KeyboardSwitcherCSettingsWindow"
#define TRAY_UID 1001
#define TIMER_LAYOUT 2001
#define WMAPP_TRAYICON (WM_APP + 1)
#define WMAPP_TRANSFORM (WM_APP + 2)

#define VK_PAUSE_KEY 0x13
#define VK_C_KEY 0x43
#define VK_V_KEY 0x56
#define PAUSE_DEBOUNCE_MS 120
#define ACTION_DELAY_MS 15
#define LAST_WORD_MAX 128
#define CLIPBOARD_RETRY_COUNT 8
#define CLIPBOARD_RETRY_DELAY_MS 10
#define CLIPBOARD_WAIT_TIMEOUT_MS 500
#define CLIPBOARD_POLL_DELAY_MS 10
#define PASTE_SETTLE_DELAY_MS 100
#define CLIPBOARD_MAX_FORMATS 128
#define LAYOUT_TIMER_INTERVAL_MS 350
#define SETTINGS_MARGIN 14

#define MENU_OPEN_SETTINGS 40001
#define MENU_TOGGLE_SOUND 40002
#define MENU_EXIT_APP 40003

#define IDC_SETTINGS_SOUND 41001
#define IDC_SETTINGS_LOGGING 41002
#define IDC_SELECTED_VALUE 41003
#define IDC_SELECTED_RECORD 41004
#define IDC_SELECTED_CLEAR 41005
#define IDC_SELECTED_RESET 41006
#define IDC_LASTWORD_VALUE 41007
#define IDC_LASTWORD_RECORD 41008
#define IDC_LASTWORD_CLEAR 41009
#define IDC_LASTWORD_RESET 41010
#define IDC_SETTINGS_STATUS 41011
#define IDC_SETTINGS_SAVE 41012
#define IDC_SETTINGS_CANCEL 41013
#define IDC_SETTINGS_GITHUB 41014
#define IDC_SETTINGS_AUTOSTART 41015
#define IDC_SETTINGS_LANGUAGE_LABEL 41016
#define IDC_SETTINGS_LANGUAGE_COMBO 41017
#define IDC_SETTINGS_VERSION 41018
#define IDC_SETTINGS_STORE 41019
#define IDC_SETTINGS_GROUP_SELECTED 41020
#define IDC_SETTINGS_GROUP_LASTWORD 41021
#define IDC_SETTINGS_BRAND 41022

#ifdef _DEBUG
#define APP_VERSION L"0.3.0-debug"
#else
#define APP_VERSION L"0.3.0"
#endif

#define DEFAULT_SELECTED_HOTKEY_VK VK_PAUSE_KEY
#define DEFAULT_SELECTED_HOTKEY_MODIFIERS 0
#define DEFAULT_LASTWORD_HOTKEY_VK VK_PAUSE_KEY
#define DEFAULT_LASTWORD_HOTKEY_MODIFIERS 0

typedef struct KeyPair {
    const wchar_t *normal;
    const wchar_t *shifted;
} KeyPair;

typedef struct LayoutDef {
    const wchar_t *code;
    const wchar_t *badge;
    const wchar_t *name;
    const wchar_t *category;
    WORD lang_id;
    COLORREF tray_color;
    const KeyPair *keys;
    size_t key_count;
} LayoutDef;

typedef enum ClipboardItemKind {
    CLIPBOARD_ITEM_EMPTY = 0,
    CLIPBOARD_ITEM_GLOBAL = 1,
    CLIPBOARD_ITEM_BITMAP = 2,
    CLIPBOARD_ITEM_ENHMETAFILE = 3
} ClipboardItemKind;

typedef struct ClipboardFormatSnapshot {
    UINT format;
    ClipboardItemKind kind;
    union {
        HGLOBAL global;
        HBITMAP bitmap;
        HENHMETAFILE enh_metafile;
    } data;
} ClipboardFormatSnapshot;

typedef struct ClipboardSnapshot {
    ClipboardFormatSnapshot items[CLIPBOARD_MAX_FORMATS];
    size_t count;
    DWORD sequence;
} ClipboardSnapshot;

typedef struct RestoreClipboardContext {
    ClipboardSnapshot snapshot;
    DWORD delay_ms;
    BOOL require_marker;
    UINT marker_format;
    DWORD marker_value;
} RestoreClipboardContext;

typedef struct LastWordTracker {
    HWND hwnd;
    HWND focus_hwnd;
    HWND caret_hwnd;
    wchar_t text[LAST_WORD_MAX];
    int length;
    const LayoutDef *source_layout;
} LastWordTracker;

typedef struct HotkeyBinding {
    UINT vk;
    UINT modifiers;
} HotkeyBinding;

typedef struct AppSettings {
    BOOL logging_enabled;
    BOOL sound_enabled;
    BOOL autostart_enabled;
    int ui_language;
    HotkeyBinding selected_hotkey;
    HotkeyBinding lastword_hotkey;
} AppSettings;

typedef struct SettingsControls {
    HWND logging_checkbox;
    HWND sound_checkbox;
    HWND autostart_checkbox;
    HWND selected_value;
    HWND lastword_value;
    HWND status_value;
    HWND github_link;
    HWND language_combo;
    HWND version_value;
    HWND store_button;
    HWND brand_logo;
} SettingsControls;

typedef enum UiLanguage {
    UI_LANGUAGE_EN = 0,
    UI_LANGUAGE_UK = 1
} UiLanguage;

typedef enum TransformRequestMode {
    TRANSFORM_MODE_NONE = 0,
    TRANSFORM_MODE_SELECTED = 1,
    TRANSFORM_MODE_LASTWORD = 2,
    TRANSFORM_MODE_COMBINED = 3
} TransformRequestMode;

typedef enum CaptureTarget {
    CAPTURE_NONE = 0,
    CAPTURE_SELECTED = 1,
    CAPTURE_LASTWORD = 2
} CaptureTarget;

extern HINSTANCE g_instance;
extern HWND g_main_window;
extern ULONGLONG g_last_pause_tick;
extern volatile LONG g_transform_running;
extern HANDLE g_clipboard_update_event;
extern volatile LONG g_clipboard_waiting;
extern LastWordTracker g_last_word;
extern AppSettings g_settings;

void SetDefaultSettings(AppSettings *settings);
void NormalizeHotkeyBinding(HotkeyBinding *binding);
BOOL IsModifierVirtualKey(UINT vk);
UINT CaptureCurrentModifiers(void);
void LoadSettings(AppSettings *settings);
void SaveSettings(const AppSettings *settings);
void FormatHotkeyBinding(const HotkeyBinding *binding, wchar_t *buffer, size_t capacity);
BOOL HotkeyBindingsEqual(const HotkeyBinding *left, const HotkeyBinding *right);
BOOL HotkeyOwnsEvent(const HotkeyBinding *binding, const KBDLLHOOKSTRUCT *kbd);
void PlaySwitchSound(void);
void LogDebug(const wchar_t *format, ...);
BOOL IsAutostartEnabled(void);
BOOL SetAutostartEnabled(BOOL enabled);

BOOL RefreshInstalledLayouts(void);
const LayoutDef *GetCurrentLayoutDef(void);
BOOL TransformSelectedText(void);
BOOL TransformLastWord(void);
void ResetLastWordTracker(void);
void UpdateLastWordTrackerFromKey(const KBDLLHOOKSTRUCT *kbd);
int RunLayoutSelfTest(void);
void NotifyClipboardUpdated(void);

void CreateTrayIcons(void);
void DestroyTrayIcons(void);
BOOL AddTrayIcon(HWND hwnd);
void RemoveTrayIcon(void);
void UpdateTrayLayoutIndicator(BOOL force);
void ShowTrayMenu(void);
void OpenSettingsWindow(void);
BOOL HandleSettingsHotkeyCapture(const KBDLLHOOKSTRUCT *kbd, WPARAM wParam);
void NotifySettingsSoundToggled(void);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

BOOL InstallKeyboardHook(void);
BOOL InstallMouseHook(void);
void RemoveKeyboardHook(void);
void RemoveMouseHook(void);
DWORD WINAPI TransformWorkerThread(LPVOID param);
TransformRequestMode ResolveTransformModeForEvent(const KBDLLHOOKSTRUCT *kbd);
BOOL IsKeyDownMessage(WPARAM wParam);
BOOL IsKeyUpMessage(WPARAM wParam);
void QueueTransformRequest(TransformRequestMode mode);

#endif
