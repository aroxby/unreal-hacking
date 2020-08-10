#include <iostream>
#include <iomanip>
#include <windows.h>
using namespace std;

bool isWriteable(const MEMORY_BASIC_INFORMATION &mi) {
    constexpr auto writeableFlags = PAGE_EXECUTE_READWRITE
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

void *createUnrealData() {
    char *dataBlock = new char[1024];
    char *dataBlockStart = dataBlock;
    memset(dataBlock, 0, 4);
    dataBlock += 4;
    memcpy(dataBlock, "None", 4);
    dataBlock += 4;
    memset(dataBlock, 0, 8);
    dataBlock += 8;
    memcpy(dataBlock, &dataBlock, 8);
    dataBlock += 8;
    memset(dataBlock, 2, 1);
    dataBlock += 1;
    memset(dataBlock, 0, 3);
    dataBlock += 3;

    return dataBlockStart;
}

void dump(const void *vp, size_t length) {
    const auto oldFlags(cout.flags());
    const auto oldFill = cout.fill();
    cout << hex << setfill('0');

    const unsigned char *start = (const unsigned char *)vp;
    const unsigned char *end = start + length;
    for(const unsigned char *pos = start; pos < end; pos++) {
        cout << setw(2) << int(*pos) << " ";
    }
    cout << endl;

    cout << setfill(oldFill);
    cout.flags(oldFlags);
}

int main() {
    HANDLE victim = GetCurrentProcess();
    const void *unreal = createUnrealData();
    dump(unreal, 32);
    // walkMemory(victim);
    return 0;
}
