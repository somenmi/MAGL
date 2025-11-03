#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <commctrl.h>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <fstream>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "comctl32.lib")

// Check if another instance is already running
bool IsAlreadyRunning()
{
    HANDLE hMutex = CreateMutexW(NULL, TRUE, L"MAGL_Mutex_v2.1");

    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        if (hMutex)
            CloseHandle(hMutex);
        return true;
    }

    return false;
}

void ShowAlreadyRunningMessage()
{
    HWND hDesktop = GetDesktopWindow();
    RECT desktopRect;
    GetWindowRect(hDesktop, &desktopRect);

    const wchar_t *message =

        L"                                             MAGL v2.1\n"
        L"     ____________________________________________________________\n\n"
        L"            PROGRAM IS ALREADY RUNNING / УЖЕ ЗАПУЩЕНА\n"
        L"     ____________________________________________________________\n"
        L"     en:\n              MAGL is already running!\n"
        L"         Please check your system tray or task manager.\n"
        L"         Only one instance can run at a time.\n\n"
        L"     ru:\n              MAGL уже запущен!\n"
        L"         Проверьте системный трей или диспетчер задач.\n"
        L"         Одновременно может работать только одна копия.\n"
        L"     ____________________________________________________________\n\n"
        L"      • Look for the MAGL icon in system tray\n\n"
        L"      • Ищите иконку MAGL в системном трее\n";

    MessageBoxW(NULL,
                message,
                L"MAGL - Already Running // Уже работает",
                MB_OK | MB_TOPMOST);
}

// Global variables
IAudioEndpointVolume *pMicVolume = NULL;
bool isRunning = true;
float targetVolume = 1.0f;
float lastVolume = 1.0f;
int reactionTimeMs = 150;
bool russianLanguage = false;
bool startMinimized = false;

// UI elements
HWND hMainWindow;
HWND hLogEdit;
HWND hVolumeSlider;
HWND hDelayEdit;
HWND hApplyButton;
HWND hMinimizeButton;
HWND hVolumeLabel;
HWND hDelayLabel;
HWND hLogLabel;
NOTIFYICONDATA nid;

struct LanguageStrings
{
    const wchar_t *windowTitle;
    const wchar_t *restoreLevel;
    const wchar_t *restoreDelay;
    const wchar_t *applySettings;
    const wchar_t *minimizeToTray;
    const wchar_t *changeLog;
    const wchar_t *errorMicAccess;
    const wchar_t *trayTip;
};

LanguageStrings english = {
    L"MAGL v2.1",
    L"Restore Level:",
    L"Restore Delay (ms):",
    L"Apply Settings",
    L"Minimize to Tray",
    L"Change Log:",
    L"Cannot access microphone.\nRun as Administrator.",
    L"MAGL v2.1"};

LanguageStrings russian = {
    L"MAGL v2.1",
    L"Уровень восстановления:",
    L"Задержка восстановления (ms):",
    L"Применить настройки",
    L"Свернуть в трей",
    L"Лог изменений:",
    L"Не удалось получить доступ к микрофону.\nЗапустите от имени администратора.",
    L"MAGL v2.1"};

const wchar_t *SETTINGS_FILE = L"magl_settings.cfg";

bool InitMicrophone();
void SetVolume(float volume);
float GetVolume();
void AddToLog(const wchar_t *text);
void UpdateSettings();
void UpdateLanguage();
void UpdateVolumeLabel();
void SaveSettings();
void LoadSettings();
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    int argc;
    LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    if (argv)
    {
        for (int i = 1; i < argc; i++)
        {
            if (wcscmp(argv[i], L"-tray") == 0 || wcscmp(argv[i], L"/tray") == 0 ||
                wcscmp(argv[i], L"-minimized") == 0 || wcscmp(argv[i], L"/minimized") == 0)
            {
                startMinimized = true;
            }
        }
        LocalFree(argv);
    }

    if (IsAlreadyRunning())
    {
        ShowAlreadyRunningMessage();
        return 0;
    }

    LoadSettings();

    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_BAR_CLASSES;
    InitCommonControlsEx(&icex);

    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
    wc.lpszClassName = L"MAGLWindow";
    wc.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(1));

    RegisterClassEx(&wc);

    hMainWindow = CreateWindowW(
        L"MAGLWindow",
        L"MAGL v2.1",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT,
        666, 350, // Size window
        NULL, NULL, hInstance, NULL);

    if (startMinimized)
    {
        ShowWindow(hMainWindow, SW_HIDE);
    }
    else
    {
        ShowWindow(hMainWindow, nCmdShow);
    }
    UpdateWindow(hMainWindow);

    if (!InitMicrophone())
    {
        MessageBoxW(hMainWindow,
                    russianLanguage ? russian.errorMicAccess : english.errorMicAccess,
                    L"Error", MB_ICONERROR);
        return 1;
    }

    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hMainWindow;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_USER + 1;
    nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(100));
    wcscpy_s(nid.szTip, L"MAGL v2.1");
    Shell_NotifyIconW(NIM_ADD, &nid);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    SaveSettings();
    Shell_NotifyIconW(NIM_DELETE, &nid);
    if (pMicVolume)
        pMicVolume->Release();

    return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        // Create menu bar
        HMENU hMenuBar = CreateMenu();
        HMENU hSettingsMenu = CreatePopupMenu();
        HMENU hLanguageMenu = CreatePopupMenu();

        // Language submenu
        AppendMenuW(hLanguageMenu, MF_STRING, 101, L"&English");
        AppendMenuW(hLanguageMenu, MF_STRING, 102, L"&Russian");

        // Settings menu - Dark Mode отключен с пометкой COMING SOON
        AppendMenuW(hSettingsMenu, MF_STRING | MF_GRAYED, 103, L"&Dark Mode (COMING SOON)");
        AppendMenuW(hSettingsMenu, MF_SEPARATOR, 0, NULL);
        AppendMenuW(hSettingsMenu, MF_POPUP, (UINT_PTR)hLanguageMenu, L"&Language");

        // Main menu bar
        AppendMenuW(hMenuBar, MF_POPUP, (UINT_PTR)hSettingsMenu, L"&Settings");

        SetMenu(hWnd, hMenuBar);

        hLogLabel = CreateWindowW(L"STATIC", L"",
                                  WS_VISIBLE | WS_CHILD,
                                  20, 14, 350, 25, hWnd, NULL, NULL, NULL);

        hLogEdit = CreateWindowW(L"EDIT", L"",
                                 WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE |
                                     ES_READONLY | WS_VSCROLL | ES_AUTOVSCROLL,
                                 20, 40, 350, 242, hWnd, NULL, NULL, NULL);

        // Volume slider
        hVolumeLabel = CreateWindowW(L"STATIC", L"",
                                     WS_VISIBLE | WS_CHILD,
                                     375, 14, 220, 25, hWnd, NULL, NULL, NULL);

        hVolumeSlider = CreateWindowW(TRACKBAR_CLASS, L"",
                                      WS_VISIBLE | WS_CHILD | TBS_AUTOTICKS | TBS_BOTH,
                                      375, 50, 270, 40, hWnd, NULL, NULL, NULL);

        SendMessage(hVolumeSlider, TBM_SETRANGE, TRUE, MAKELONG(1, 100));
        SendMessage(hVolumeSlider, TBM_SETPOS, TRUE, (int)(targetVolume * 100));
        SendMessage(hVolumeSlider, TBM_SETTICFREQ, 10, 0);

        // Delay input
        hDelayLabel = CreateWindowW(L"STATIC", L"",
                                    WS_VISIBLE | WS_CHILD,
                                    375, 110, 220, 25, hWnd, NULL, NULL, NULL);

        wchar_t delayText[32];
        swprintf(delayText, 32, L"%d", reactionTimeMs);
        hDelayEdit = CreateWindowW(L"EDIT", delayText,
                                   WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER,
                                   600, 105, 50, 30, hWnd, NULL, NULL, NULL);

        // Buttons - под задержкой
        hApplyButton = CreateWindowW(L"BUTTON", L"",
                                     WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | WS_BORDER,
                                     420, 166, 180, 40, hWnd, (HMENU)1, NULL, NULL);

        hMinimizeButton = CreateWindowW(L"BUTTON", L"",
                                        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | WS_BORDER,
                                        420, 218, 180, 40, hWnd, (HMENU)2, NULL, NULL);

        // Set font for log
        HFONT hLogFont = CreateFontW(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                     DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                     DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Consolas");
        SendMessageW(hLogEdit, WM_SETFONT, (WPARAM)hLogFont, TRUE);

        // Update UI texts
        UpdateLanguage();
        UpdateVolumeLabel();

        if (russianLanguage)
        {
            AddToLog(L"MAGL запущен - работаем!");
            AddToLog(L"Загружены настройки из файла");
        }
        else
        {
            AddToLog(L"MAGL started - start!");
            AddToLog(L"Settings loaded from file");
        }

        // Start monitoring thread
        std::thread([]()
                    {
            int protectionCount = 0;
            bool volumeChanged = false;
            auto lastChangeTime = std::chrono::steady_clock::now();

            SetVolume(targetVolume);
            lastVolume = targetVolume;

            while (isRunning)
            {
                float currentVolume = GetVolume();

                if (abs(currentVolume - lastVolume) > 0.01f)
                {
                    if (!volumeChanged)
                    {
                        volumeChanged = true;
                        lastChangeTime = std::chrono::steady_clock::now();

                        SYSTEMTIME time;
                        GetLocalTime(&time);
                        
                        wchar_t log[256];
                        if (russianLanguage) {
                            swprintf(log, 256, L"%02d:%02d:%02d - Громкость упала: %.0f%% -> %.0f%%", 
                                time.wHour, time.wMinute, time.wSecond,
                                lastVolume * 100, currentVolume * 100);
                        } else {
                            swprintf(log, 256, L"%02d:%02d:%02d - Volume dropped: %.0f%% -> %.0f%%", 
                                time.wHour, time.wMinute, time.wSecond,
                                lastVolume * 100, currentVolume * 100);
                        }
                        AddToLog(log);
                    }
                    lastVolume = currentVolume;
                }

                if (volumeChanged)
                {
                    auto now = std::chrono::steady_clock::now();
                    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastChangeTime);

                    if (elapsed.count() >= reactionTimeMs)
                    {
                        protectionCount++;

                        float beforeVolume = GetVolume();
                        SetVolume(targetVolume);
                        float afterVolume = GetVolume();

                        SYSTEMTIME time;
                        GetLocalTime(&time);
                        
                        wchar_t log[256];
                        if (russianLanguage) {
                            swprintf(log, 256, L"%02d:%02d:%02d - Починил #%d-й раз: %.0f%% -> %.0f%%", 
                                time.wHour, time.wMinute, time.wSecond, protectionCount,
                                beforeVolume * 100, afterVolume * 100);
                        } else {
                            swprintf(log, 256, L"%02d:%02d:%02d - Fixed #%d: %.0f%% -> %.0f%%", 
                                time.wHour, time.wMinute, time.wSecond, protectionCount,
                                beforeVolume * 100, afterVolume * 100);
                        }
                        AddToLog(log);

                        volumeChanged = false;
                        lastVolume = targetVolume;
                    }
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            } })
            .detach();

        break;
    }

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case 1:
            UpdateSettings();
            break;
        case 2:
            ShowWindow(hWnd, SW_HIDE);
            break;
        case 101: // English
            russianLanguage = false;
            UpdateLanguage();
            break;
        case 102: // Russian
            russianLanguage = true;
            UpdateLanguage();
            break;
        case 103: // Dark Mode (disabled)
            MessageBoxW(hWnd,
                        L"Dark Mode feature is coming soon!\n\nТемная тема будет доступна в следующем обновлении!",
                        L"Coming Soon", MB_ICONINFORMATION);
            break;
        }
        break;
    }

    case WM_HSCROLL:
    {
        if ((HWND)lParam == hVolumeSlider)
        {
            UpdateVolumeLabel();

            int pos = SendMessage(hVolumeSlider, TBM_GETPOS, 0, 0);
            wchar_t tip[128];
            if (russianLanguage)
            {
                swprintf(tip, 128, L"MAGL - Уровень: %d%%", pos);
            }
            else
            {
                swprintf(tip, 128, L"MAGL - Level: %d%%", pos);
            }
            wcscpy_s(nid.szTip, tip);
            Shell_NotifyIconW(NIM_MODIFY, &nid);
        }
        break;
    }

    case WM_USER + 1:
        switch (lParam)
        {
        case WM_RBUTTONDOWN:
        case WM_LBUTTONDOWN:
            ShowWindow(hWnd, SW_SHOW);
            SetForegroundWindow(hWnd);
            break;
        }
        break;

    case WM_CLOSE:
        isRunning = false;
        DestroyWindow(hWnd);
        break;

    case WM_DESTROY:
        isRunning = false;
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

bool InitMicrophone()
{
    CoInitialize(NULL);

    IMMDeviceEnumerator *pEnumerator = NULL;
    IMMDevice *pMicrophone = NULL;

    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL,
                                  CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void **)&pEnumerator);

    if (FAILED(hr))
        return false;

    hr = pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &pMicrophone);
    if (FAILED(hr))
    {
        pEnumerator->Release();
        return false;
    }

    hr = pMicrophone->Activate(__uuidof(IAudioEndpointVolume),
                               CLSCTX_ALL, NULL, (void **)&pMicVolume);

    pMicrophone->Release();
    pEnumerator->Release();

    if (SUCCEEDED(hr))
    {
        pMicVolume->GetMasterVolumeLevelScalar(&lastVolume);
    }

    return SUCCEEDED(hr);
}

void SetVolume(float volume)
{
    if (pMicVolume)
    {
        pMicVolume->SetMasterVolumeLevelScalar(volume, NULL);
    }
}

float GetVolume()
{
    float volume = 0.0f;
    if (pMicVolume)
    {
        pMicVolume->GetMasterVolumeLevelScalar(&volume);
    }
    return volume;
}

void AddToLog(const wchar_t *text)
{
    int len = GetWindowTextLengthW(hLogEdit);
    SendMessageW(hLogEdit, EM_SETSEL, len, len);
    SendMessageW(hLogEdit, EM_REPLACESEL, FALSE, (LPARAM)text);
    SendMessageW(hLogEdit, EM_REPLACESEL, FALSE, (LPARAM)L"\r\n");
    SendMessageW(hLogEdit, WM_VSCROLL, SB_BOTTOM, 0);
}

void UpdateSettings()
{
    int volumePercent = SendMessage(hVolumeSlider, TBM_GETPOS, 0, 0);
    targetVolume = volumePercent / 100.0f;

    wchar_t delayText[32];
    GetWindowTextW(hDelayEdit, delayText, 32);
    reactionTimeMs = _wtoi(delayText);

    if (reactionTimeMs < 20)
        reactionTimeMs = 20;
    if (reactionTimeMs > 5000)
        reactionTimeMs = 5000;

    wchar_t tip[128];
    if (russianLanguage)
    {
        swprintf(tip, 128, L"MAGL - Уровень: %d%%", volumePercent);
    }
    else
    {
        swprintf(tip, 128, L"MAGL - Level: %d%%", volumePercent);
    }
    wcscpy_s(nid.szTip, tip);
    Shell_NotifyIconW(NIM_MODIFY, &nid);

    wchar_t log[256];
    if (russianLanguage)
    {
        swprintf(log, 256, L"Настройки: Уровень %d%% Задержка %dms", volumePercent, reactionTimeMs);
    }
    else
    {
        swprintf(log, 256, L"Settings: Level %d%% Delay %dms", volumePercent, reactionTimeMs);
    }
    AddToLog(log);
}

void UpdateLanguage()
{
    LanguageStrings *lang = russianLanguage ? &russian : &english;

    SetWindowTextW(hVolumeLabel, lang->restoreLevel);
    SetWindowTextW(hDelayLabel, lang->restoreDelay);
    SetWindowTextW(hLogLabel, lang->changeLog);
    SetWindowTextW(hApplyButton, lang->applySettings);
    SetWindowTextW(hMinimizeButton, lang->minimizeToTray);

    UpdateVolumeLabel();

    if (russianLanguage)
    {
        AddToLog(L"Язык изменен на Русский");
    }
    else
    {
        AddToLog(L"Language changed to English");
    }
}

void UpdateVolumeLabel()
{
    int volumePercent = SendMessage(hVolumeSlider, TBM_GETPOS, 0, 0);
    wchar_t label[256];
    if (russianLanguage)
    {
        swprintf(label, 256, L"Уровень восстановления (%d%%):", volumePercent);
    }
    else
    {
        swprintf(label, 256, L"Restore Level (%d%%):", volumePercent);
    }
    SetWindowTextW(hVolumeLabel, label);
}

void SaveSettings()
{
    std::wofstream file(SETTINGS_FILE);
    if (file.is_open())
    {
        file << L"language=" << (russianLanguage ? 1 : 0) << std::endl;
        file << L"volume=" << (int)(targetVolume * 100) << std::endl;
        file << L"delay=" << reactionTimeMs << std::endl;
        file.close();
    }
}

void LoadSettings()
{
    std::wifstream file(SETTINGS_FILE);
    if (file.is_open())
    {
        std::wstring line;
        while (std::getline(file, line))
        {
            size_t pos = line.find(L'=');
            if (pos != std::wstring::npos)
            {
                std::wstring key = line.substr(0, pos);
                std::wstring value = line.substr(pos + 1);

                if (key == L"language")
                {
                    russianLanguage = (std::stoi(value) == 1);
                }
                else if (key == L"volume")
                {
                    targetVolume = std::stoi(value) / 100.0f;
                }
                else if (key == L"delay")
                {
                    reactionTimeMs = std::stoi(value);
                }
            }
        }
        file.close();
    }
}