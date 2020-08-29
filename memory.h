#ifndef _INC_MEMORY_H_
#define _INC_MEMORY_H_

#include <iostream>
#include <memory>
#include <vector>
#include <windows.h>

class MemoryRegion {
public:
    void *start;
    size_t length;

    MemoryRegion(void *start, size_t length) : start(start), length(length) {}
    bool includes(const void *address);
};

std::ostream &operator<<(std::ostream &os, const MemoryRegion &region);

void *increasePointer(void *base, size_t addend);
ptrdiff_t pointerOffset(const void *a, const void *b);

bool isWriteable(const MEMORY_BASIC_INFORMATION &mi);
std::vector<MemoryRegion> getProcessMemoryRegions(HANDLE hProcess);

void *memmem(void *haystack, size_t haystacklen, const void *needle, size_t needlelen);
std::unique_ptr<unsigned char> CopyProcessMemory(HANDLE hProcess, const MemoryRegion &region);
void ScanProcessMemory(
    HANDLE hProcess, const std::vector<MemoryRegion> &regions, const void *goal, size_t goalLength
);

#endif//_INC_MEMORY_H_
