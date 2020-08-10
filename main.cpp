#include <iostream>
#include <windows.h>
using namespace std;

bool isWriteable(const MEMORY_BASIC_INFORMATION &mi) {
    const auto writeableFlags = PAGE_EXECUTE_READWRITE
        | PAGE_EXECUTE_WRITECOPY
        | PAGE_READWRITE
        | PAGE_WRITECOPY;
    if ((mi.Protect & writeableFlags) == 0) {
        return false;
    } else {
        return true;
    }
}

// Source: https://www.codeproject.com/Articles/4865/Performing-a-hex-dump-of-another-process-s-memory
void walkMemory(HANDLE hProcess) {
    MEMORY_BASIC_INFORMATION mi;
    SYSTEM_INFO si;
    GetSystemInfo(&si);

    LPVOID lpMem = si.lpMinimumApplicationAddress;
    size_t total = 0;

    while (lpMem <= si.lpMaximumApplicationAddress) {
        mi = {0};
        SIZE_T vqr = VirtualQueryEx(hProcess, lpMem, &mi, sizeof mi);
        if (vqr != sizeof mi) {
            cout << "Error: " << lpMem << endl;
            break;
        } else {
            auto lpMemStart = lpMem;
            lpMem = (LPVOID)((size_t)mi.BaseAddress + (size_t)mi.RegionSize);
            if (isWriteable(mi)) {
                cout << lpMemStart << " - " << lpMem << endl;
                total += mi.RegionSize;
            }
        }
    }

    cout << "Total bytes: " << total << endl;
}

int main() {
    HANDLE victim = GetCurrentProcess();
    walkMemory(victim);
    return 0;
}
