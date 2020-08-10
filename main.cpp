#include <iostream>
#include <windows.h>
using namespace std;

// Source: https://www.codeproject.com/Articles/4865/Performing-a-hex-dump-of-another-process-s-memory
void walkMemory(HANDLE hProcess) {
    MEMORY_BASIC_INFORMATION mi;
    SYSTEM_INFO si;
    GetSystemInfo(&si);

    LPVOID lpMem = si.lpMinimumApplicationAddress;
    size_t total = 0;

    while (lpMem < si.lpMaximumApplicationAddress) {
        mi = {0};
        SIZE_T vqr = VirtualQueryEx(hProcess, lpMem, &mi, sizeof mi);
        if (vqr != sizeof mi) {
            cout << "Error: " << lpMem << endl;
            break;
        } else {
            auto lpMemStart = lpMem;
            lpMem = (LPVOID)((size_t)mi.BaseAddress + (size_t)mi.RegionSize);
            cout << lpMemStart << " - " << lpMem << endl;
        }
    }
}


int main() {
    HANDLE victim = GetCurrentProcess();
    walkMemory(victim);
    return 0;
}