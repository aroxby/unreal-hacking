#ifndef _INC_MEMORY_H_
#define _INC_MEMORY_H_

#include <iostream>
#include <memory>
#include <set>
#include <vector>
#include <windows.h>
#include "exceptions.h"

EXCEPTIONTYPE(MemoryAPIError);

class MemoryRegion {
public:
    void *start;
    size_t length;

    MemoryRegion(void *start, size_t length) : start(start), length(length) {}
    bool includes(const void *address);
};

class RemoteMemoryAddress {
public:
    RemoteMemoryAddress(const void *local, const void *remote) : local(local), remote(remote) {}
    const void * const local;
    const void * const remote;
};

class ScanResults {
public:
    ScanResults(
        const std::set<std::shared_ptr<const void>> &regions,
        const std::vector<RemoteMemoryAddress> &addresses
    ) : regions(regions), addresses(addresses) {}
    const std::set<std::shared_ptr<const void>> regions;
    const std::vector<RemoteMemoryAddress> addresses;
};

std::ostream &operator<<(std::ostream &os, const MemoryRegion &region);

void *increasePointer(void *base, size_t addend);
ptrdiff_t pointerOffset(const void *a, const void *b);

bool isWriteable(const MEMORY_BASIC_INFORMATION &mi);
std::vector<MemoryRegion> getProcessMemoryRegions(HANDLE hProcess);

void *memmem(void *haystack, size_t haystacklen, const void *needle, size_t needlelen);
std::unique_ptr<unsigned char> CopyProcessMemory(HANDLE hProcess, const MemoryRegion &region);
ScanResults ScanProcessMemory(
    HANDLE hProcess, const std::vector<MemoryRegion> &regions, const void *goal, size_t goalLength
);

#endif//_INC_MEMORY_H_
