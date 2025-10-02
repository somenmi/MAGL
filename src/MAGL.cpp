#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <iostream>
#include <thread>
#include <chrono>

#pragma comment(lib, "ole32.lib")

IAudioEndpointVolume *pMicVolume = NULL;
bool isRunning = true;
float targetVolume = 1.0f;        // 100% - можно менять от 0.01 (1%) до 1.0 (100%)
float lastVolume = 1.0f;          // Так же как и targetVol
const int REACTION_TIME_MS = 150; // 1000 = 1 сек

void printRussian(const wchar_t *format, ...)
{
    wchar_t buffer[256];
    va_list args;
    va_start(args, format);
    vswprintf(buffer, 256, format, args);
    va_end(args);

    DWORD written;
    WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), buffer, wcslen(buffer), &written, NULL);
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

BOOL WINAPI ConsoleHandler(DWORD signal)
{
    if (signal == CTRL_C_EVENT)
    {
        printRussian(L" \nВырубаю программу...\n");
        isRunning = false;
        return TRUE;
    }
    return FALSE;
}

int main()
{
    Sleep(2000);

    ShowWindow(GetConsoleWindow(), SW_SHOWMINNOACTIVE);
    SetForegroundWindow(GetConsoleWindow());

    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);

    system("chcp 1251 >nul");

    printRussian(L" ============================================\n\n");
    printRussian(L"         MAGL (MicAutoGainLock) v2.0\n");
    printRussian(L"       Умная защита громкости микрофона\n\n");
    printRussian(L" ============================================\n\n");
    printRussian(L"    Разработчик: Yalkee\n");
    printRussian(L"    GitHub: https://github.com/somenmi/MAGL\n\n");
    printRussian(L" ============================================\n\n");

    SetConsoleCtrlHandler(ConsoleHandler, TRUE);

    if (!InitMicrophone())
    {
        printRussian(L" ❌ Ошибка: Не удалось получить доступ к микрофону.\n");
        printRussian(L" Запусти программу от имени администратора.\n");
        system("pause");
        return 1;
    }

    printRussian(L" 👹 Микрофон инициализирован\n\n");
    printRussian(L"      Поднимает на: %.0f%%\n", targetVolume * 100);
    printRussian(L"      > Режим: Отслеживание изменений\n");

    wchar_t reactionText[100];
    swprintf(reactionText, 100, L"      > Реакция: %d сек", REACTION_TIME_MS / 1000);
    printRussian(reactionText);

    swprintf(reactionText, 100, L" (%d ms)\n\n", REACTION_TIME_MS);
    printRussian(reactionText);

    int protectionCount = 0;
    bool volumeChanged = false;
    auto lastChangeTime = std::chrono::steady_clock::now();

    SetVolume(targetVolume);
    lastVolume = targetVolume;

    printRussian(L" ⛔ Защита активирована. Ждём изменений...\n\n");

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
                printf(" [Volume dropped...] %02d:%02d:%02d - ", time.wHour, time.wMinute, time.wSecond);
                printf(" Fell from %.0f%% to %.0f%%\n", lastVolume * 100, currentVolume * 100);
            }
            lastVolume = currentVolume;
        }

        if (volumeChanged)
        {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastChangeTime);

            if (elapsed.count() >= REACTION_TIME_MS)
            {
                protectionCount++;

                float beforeVolume = GetVolume();
                SetVolume(targetVolume);
                float afterVolume = GetVolume();

                SYSTEMTIME time;
                GetLocalTime(&time);
                printf(" [Fixed #%d times] %02d:%02d:%02d - ", protectionCount, time.wHour, time.wMinute, time.wSecond);
                printf(" Restored from %.0f%% to %.0f%%\n\n", beforeVolume * 100, afterVolume * 100);

                volumeChanged = false;
                lastVolume = targetVolume;
            }
        }

        // Можно меня на своё усмотрение, 700-1000 идеально для слабого железа
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    if (pMicVolume)
    {
        pMicVolume->Release();
    }
    CoUninitialize();

    printRussian(L"\nПрограмма завершена 🫣\n");
    return 0;
}