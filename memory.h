#ifndef _INC_MEMORY_H_
#define _INC_MEMORY_H_

#include <iostream>
#include <memory>
#include <vector>
#include <windows.h>
#include "exceptions.h"

EXCEPTIONTYPE(MemoryAPIError);

class MemoryRegion {
public:
    void *start;
    size_t length;

    MemoryRegion(void *start, size_t length);
    bool includes(const void *address);
};

class RemoteDataReference {
public:
    RemoteDataReference(
        const void *local, const void *remote, const std::shared_ptr<const void> localCopy
    );
    const void * const local;
    const void * const remote;
    const std::shared_ptr<const void> localCopy;
};

std::ostream &operator<<(std::ostream &os, const MemoryRegion &region);

void *increasePointer(void *base, size_t addend);
ptrdiff_t pointerOffset(const void *a, const void *b);

bool isWriteable(const MEMORY_BASIC_INFORMATION &mi);
std::vector<MemoryRegion> getProcessMemoryRegions(HANDLE hProcess);

void *memmem(void *haystack, size_t haystacklen, const void *needle, size_t needlelen);
std::unique_ptr<unsigned char> CopyProcessMemory(HANDLE hProcess, const MemoryRegion &region);
std::vector<RemoteDataReference> ScanProcessMemory(
    HANDLE hProcess, const std::vector<MemoryRegion> &regions, const void *goal, size_t goalLength
);

#endif//_INC_MEMORY_H_
