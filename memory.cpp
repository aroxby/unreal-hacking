#include <iostream>
#include <sstream>
#include <vector>
#include <windows.h>
#include "exceptions.h"
#include "memory.h"
using namespace std;

void *increasePointer(void *base, size_t addend) {
    size_t baseAddr = (size_t)base;
    void *vp = (void*)(baseAddr + addend);
    return vp;
}

ptrdiff_t pointerOffset(const void *a, const void *b) {
    ptrdiff_t offset = (ptrdiff_t)a - (ptrdiff_t)b;
    return offset;
}

ostream &operator<<(ostream &os, const MemoryRegion &region) {
    return os << region.start << " - " << increasePointer(region.start, region.length);
};

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
vector<MemoryRegion> getProcessMemoryRegions(HANDLE hProcess) {
    MEMORY_BASIC_INFORMATION mi;
    SYSTEM_INFO si;
    vector<MemoryRegion> regions;

    GetSystemInfo(&si);

    LPVOID lpMem = si.lpMinimumApplicationAddress;
    size_t total = 0;

    while (lpMem <= si.lpMaximumApplicationAddress) {
        mi = {0};
        SIZE_T vqr = VirtualQueryEx(hProcess, lpMem, &mi, sizeof mi);
        if (vqr != sizeof mi) {
            stringstream msg;
            msg << "VirtualQueryEx returned " << vqr << " with error " << GetLastError();
            throw SimpleException(msg.str());
        } else {
            lpMem = increasePointer(mi.BaseAddress, mi.RegionSize);
            if (isWriteable(mi)) {
                MemoryRegion thisRegion(mi.BaseAddress, mi.RegionSize);
                regions.push_back(thisRegion);
                total += mi.RegionSize;
            }
        }
    }

    cout << "Total bytes: " << total << endl;
    return regions;
}

void *memmem(void *haystack, size_t haystacklen, const void *needle, size_t needlelen) {
    while(haystacklen >= needlelen) {
        if(!memcmp(haystack, needle, needlelen)) {
            return haystack;
        } else {
            haystack = increasePointer(haystack, 1);
            haystacklen--;
        }
    }
    return nullptr;
}

unique_ptr<unsigned char> CopyProcessMemory(HANDLE hProcess, const MemoryRegion &region) {
    unique_ptr<unsigned char> bufferPointer(new unsigned char[region.length]);
    SIZE_T bytesRead = 0;
    BOOL rpr = ReadProcessMemory(hProcess, region.start, bufferPointer.get(), region.length, &bytesRead);
    if(!rpr) {
        DWORD gle = GetLastError();
        if (gle == ERROR_PARTIAL_COPY && bytesRead == 0) {
            // This region is probably all gone
            cerr << "Could not read region " << region << endl;
            return unique_ptr<unsigned char>(nullptr);
        } else {
            stringstream msg;
            msg << "ReadProcessMemory returned " << rpr << " with error " << gle;
            throw SimpleException(msg.str());
        }
    }
    if(bytesRead != region.length) {
        stringstream msg;
        msg << "Expected to read " << region.length << " bytes but read " << bytesRead;
        throw SimpleException(msg.str());
    }
    return bufferPointer;
}

void ScanProcessMemory(HANDLE hProcess, const vector<MemoryRegion> &regions, const void *goal, size_t goalLength) {
    for(MemoryRegion region : regions) {
        unique_ptr<unsigned char> dataPointer = CopyProcessMemory(hProcess, region);
        void *data = dataPointer.get();
        if (!data) {
            continue;
        }
        size_t remainingBytes = region.length;
        size_t localToRemote = pointerOffset(region.start, data);
        void *result;
        do {
            result = memmem(data, remainingBytes, goal, goalLength);
            if (result) {
                cout << "Found at " << increasePointer(result, localToRemote) << endl;
                size_t searchDistance = pointerOffset(data, result);
                data = increasePointer(result, goalLength);
                remainingBytes -= goalLength + searchDistance;
            }
        } while(result);
    }
}