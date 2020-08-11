#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <windows.h>
using namespace std;

class SimpleException : public exception {
public:
    SimpleException(const string &what): msg(what) {}
    virtual const char *what() const noexcept override { return msg.c_str(); }
private:
    const string msg;
};

class MemoryRegion {
public:
    MemoryRegion(void *start, size_t length) : start(start), length(length) {}
    void *start;
    size_t length;
};

void *increasePointer(void *base, size_t addend) {
    size_t baseAddr = (size_t)base;
    void *vp = (void*)(baseAddr + addend);
    return vp;
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
        cout << region << endl;
        unique_ptr<unsigned char> dataPointer = CopyProcessMemory(hProcess, region);
        void *data = dataPointer.get();
        if (!data) {
            continue;
        }
        void *result;
        size_t remainingBytes = region.length;
        do {
            result = memmem(data, remainingBytes, goal, goalLength);
            if (result) {
                cout << "Found at " << result << endl;  // FIXME: this is local address
                size_t distance = (size_t)result - (size_t)data;
                data = increasePointer(result, goalLength);
                remainingBytes -= goalLength + distance;
            }
        } while(result);
    }
}

int main() {
    HANDLE victim = GetCurrentProcess();
    const void *unreal = createUnrealData();
    cout << "Unreal data is at " << unreal << endl;
    auto regions = getProcessMemoryRegions(victim);
    ScanProcessMemory(victim, regions, "\0\0\0\0None\0\0\0\0\0\0\0", 16);
    return 0;
}
