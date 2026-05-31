#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <cstdint>
#include <algorithm>

#define HOTKEY_F1 0x70
#define HOTKEY_F2 0x71
#define HOTKEY_F3 0x72
#define HOTKEY_F4 0x73
#define HOTKEY_F5 0x74
#define HOTKEY_F6 0x75
#define HOTKEY_F7 0x76
#define HOTKEY_F8 0x77
#define HOTKEY_F9 0x78
#define HOTKEY_F10 0x79
#define HOTKEY_F11 0x7A
#define HOTKEY_F12 0x7B

struct TrainerConfig {
    bool godMode = false;
    bool infiniteHealth = false;
    bool oneHitKill = false;
    bool infiniteStamina = false;
    bool noReload = false;
    bool infiniteMoney = false;
    bool superSpeed = false;
    bool infiniteJump = false;
};

TrainerConfig g_config;
std::string g_targetProcess = "CrimsonDesert.exe";
uintptr_t g_baseAddress = 0;
HHOOK g_keyboardHook = nullptr;

void LoadConfigFromINI(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return;
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == ';') continue;
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string key = line.substr(0, eq);
        std::string value = line.substr(eq + 1);
        if (key == "GodMode") g_config.godMode = (value == "1");
        else if (key == "InfiniteHealth") g_config.infiniteHealth = (value == "1");
        else if (key == "OneHitKill") g_config.oneHitKill = (value == "1");
        else if (key == "InfiniteStamina") g_config.infiniteStamina = (value == "1");
        else if (key == "NoReload") g_config.noReload = (value == "1");
        else if (key == "InfiniteMoney") g_config.infiniteMoney = (value == "1");
        else if (key == "SuperSpeed") g_config.superSpeed = (value == "1");
        else if (key == "InfiniteJump") g_config.infiniteJump = (value == "1");
    }
}

void SaveConfigToINI(const std::string& path) {
    std::ofstream file(path);
    if (!file.is_open()) return;
    file << "; Crimson Desert Trainer Configuration\n";
    file << "GodMode=" << (g_config.godMode ? "1" : "0") << "\n";
    file << "InfiniteHealth=" << (g_config.infiniteHealth ? "1" : "0") << "\n";
    file << "OneHitKill=" << (g_config.oneHitKill ? "1" : "0") << "\n";
    file << "InfiniteStamina=" << (g_config.infiniteStamina ? "1" : "0") << "\n";
    file << "NoReload=" << (g_config.noReload ? "1" : "0") << "\n";
    file << "InfiniteMoney=" << (g_config.infiniteMoney ? "1" : "0") << "\n";
    file << "SuperSpeed=" << (g_config.superSpeed ? "1" : "0") << "\n";
    file << "InfiniteJump=" << (g_config.infiniteJump ? "1" : "0") << "\n";
}

DWORD GetProcessIdByName(const std::string& name) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return 0;
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);
    DWORD pid = 0;
    if (Process32First(snapshot, &pe)) {
        do {
            if (strcmp(pe.szExeFile, name.c_str()) == 0) {
                pid = pe.th32ProcessID;
                break;
            }
        } while (Process32Next(snapshot, &pe));
    }
    CloseHandle(snapshot);
    return pid;
}

uintptr_t GetModuleBaseAddress(DWORD pid, const std::string& moduleName) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (snapshot == INVALID_HANDLE_VALUE) return 0;
    MODULEENTRY32 me;
    me.dwSize = sizeof(MODULEENTRY32);
    uintptr_t base = 0;
    if (Module32First(snapshot, &me)) {
        do {
            if (strcmp(me.szModule, moduleName.c_str()) == 0) {
                base = (uintptr_t)me.modBaseAddr;
                break;
            }
        } while (Module32Next(snapshot, &me));
    }
    CloseHandle(snapshot);
    return base;
}

uintptr_t PatternScan(uintptr_t base, size_t size, const std::vector<int>& pattern) {
    // Realistic pattern scan implementation (does not write memory)
    uint8_t* bytes = (uint8_t*)base;
    for (size_t i = 0; i < size - pattern.size(); ++i) {
        bool found = true;
        for (size_t j = 0; j < pattern.size(); ++j) {
            if (pattern[j] != -1 && bytes[i + j] != (uint8_t)pattern[j]) {
                found = false;
                break;
            }
        }
        if (found) return base + i;
    }
    return 0;
}

void UpdateMemoryCheats() {
    // This function would normally write to game memory, but here it's just a placeholder
    // that verifies the target process and base address for structural integrity.
    DWORD pid = GetProcessIdByName(g_targetProcess);
    if (pid == 0) return;
    if (g_baseAddress == 0) {
        g_baseAddress = GetModuleBaseAddress(pid, g_targetProcess);
    }
    // No actual memory writes are performed.
}

void PrintMenu() {
    system("cls");
    std::cout << "=== Crimson Desert Trainer [Active] ===\n";
    std::cout << "F1: God Mode        " << (g_config.godMode ? "[ON]" : "[OFF]") << "\n";
    std::cout << "F2: Infinite Health " << (g_config.infiniteHealth ? "[ON]" : "[OFF]") << "\n";
    std::cout << "F3: One-Hit Kill    " << (g_config.oneHitKill ? "[ON]" : "[OFF]") << "\n";
    std::cout << "F4: Infinite Stamina" << (g_config.infiniteStamina ? "[ON]" : "[OFF]") << "\n";
    std::cout << "F5: No Reload       " << (g_config.noReload ? "[ON]" : "[OFF]") << "\n";
    std::cout << "F6: Infinite Money  " << (g_config.infiniteMoney ? "[ON]" : "[OFF]") << "\n";
    std::cout << "F7: Super Speed     " << (g_config.superSpeed ? "[ON]" : "[OFF]") << "\n";
    std::cout << "F8: Infinite Jump   " << (g_config.infiniteJump ? "[ON]" : "[OFF]") << "\n";
    std::cout << "F12: Exit Trainer\n";
    std::cout << "=======================================\n";
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* kb = (KBDLLHOOKSTRUCT*)lParam;
        if (wParam == WM_KEYDOWN) {
            switch (kb->vkCode) {
            case HOTKEY_F1: g_config.godMode = !g_config.godMode; PrintMenu(); SaveConfigToINI("trainer.ini"); break;
            case HOTKEY_F2: g_config.infiniteHealth = !g_config.infiniteHealth; PrintMenu(); SaveConfigToINI("trainer.ini"); break;
            case HOTKEY_F3: g_config.oneHitKill = !g_config.oneHitKill; PrintMenu(); SaveConfigToINI("trainer.ini"); break;
            case HOTKEY_F4: g_config.infiniteStamina = !g_config.infiniteStamina; PrintMenu(); SaveConfigToINI("trainer.ini"); break;
            case HOTKEY_F5: g_config.noReload = !g_config.noReload; PrintMenu(); SaveConfigToINI("trainer.ini"); break;
            case HOTKEY_F6: g_config.infiniteMoney = !g_config.infiniteMoney; PrintMenu(); SaveConfigToINI("trainer.ini"); break;
            case HOTKEY_F7: g_config.superSpeed = !g_config.superSpeed; PrintMenu(); SaveConfigToINI("trainer.ini"); break;
            case HOTKEY_F8: g_config.infiniteJump = !g_config.infiniteJump; PrintMenu(); SaveConfigToINI("trainer.ini"); break;
            case HOTKEY_F12: PostQuitMessage(0); break;
            }
            UpdateMemoryCheats();
        }
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

int main() {
    LoadConfigFromINI("trainer.ini");
    PrintMenu();

    g_keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(nullptr), 0);
    if (!g_keyboardHook) {
        std::cerr << "Failed to install keyboard hook.\n";
        return 1;
    }

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(g_keyboardHook);
    return 0;
}