#include "app.h"

HINSTANCE g_instance = NULL;
HWND g_main_window = NULL;
ULONGLONG g_last_pause_tick = 0;
volatile LONG g_transform_running = 0;
HANDLE g_clipboard_update_event = NULL;
volatile LONG g_clipboard_waiting = 0;
LastWordTracker g_last_word = { 0 };
AppSettings g_settings = { 0 };

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE prev, PWSTR cmd, int show) {
    WNDCLASSEXW wc;
    HWND hwnd;
    MSG msg;

    (void)prev;
    (void)cmd;
    (void)show;

    g_instance = instance;
    LoadSettings(&g_settings);
    LogDebug(L"=== App started ===");

    if (cmd != NULL && wcsstr(cmd, L"--self-test") != NULL) {
        return RunLayoutSelfTest();
    }

    g_clipboard_update_event = CreateEventW(NULL, TRUE, FALSE, NULL);

    RefreshInstalledLayouts();
    CreateTrayIcons();

    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = instance;
    wc.lpszClassName = APP_CLASS_NAME;

    if (!RegisterClassExW(&wc)) {
        LogDebug(L"RegisterClassEx failed. Win32=%lu.", GetLastError());
        DestroyTrayIcons();
        return 1;
    }

    hwnd = CreateWindowExW(
        0,
        APP_CLASS_NAME,
        APP_NAME,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,
        NULL,
        instance,
        NULL);

    if (hwnd == NULL) {
        LogDebug(L"CreateWindowEx failed. Win32=%lu.", GetLastError());
        DestroyTrayIcons();
        return 1;
    }

    g_main_window = hwnd;

    if (!AddTrayIcon(hwnd)) {
        LogDebug(L"Failed to add tray icon.");
        DestroyWindow(hwnd);
        return 1;
    }

    if (!InstallKeyboardHook()) {
        DestroyWindow(hwnd);
        return 1;
    }

    if (!InstallMouseHook()) {
        DestroyWindow(hwnd);
        return 1;
    }

    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    if (g_clipboard_update_event != NULL) {
        CloseHandle(g_clipboard_update_event);
        g_clipboard_update_event = NULL;
    }

    return (int)msg.wParam;
}
