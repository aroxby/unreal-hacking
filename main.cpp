#include <iostream>
#include <windows.h>
using namespace std;

// Source: https://www.codeproject.com/Articles/4865/Performing-a-hex-dump-of-another-process-s-memory
void walkMemory(HANDLE hProcess) {
    MEMORY_BASIC_INFORMATION mbi;
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    
    LPCVOID lpMem = si.lpMinimumApplicationAddress;
    cout << "System Range: "
        << si.lpMinimumApplicationAddress
        << " - " <<
        si.lpMaximumApplicationAddress
        << endl;
    
    while (lpMem < si.lpMaximumApplicationAddress) {
        mbi = {0};
        size_t vqr = VirtualQueryEx(hProcess, lpMem, &mbi, sizeof mbi);
        if (vqr != sizeof mbi) {
            cout << "Error: " << lpMem << endl;
            break;
        } else {
            auto lpMemStart = lpMem;
            lpMem = (LPVOID)((size_t)mbi.BaseAddress + (size_t)mbi.RegionSize);
            cout << lpMemStart << " - " << lpMem << endl;
        }
    }
}


int main() {
    HANDLE victim = GetCurrentProcess();
    walkMemory(victim);
    return 0;
}