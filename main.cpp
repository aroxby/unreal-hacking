#include <iostream>
#include <windows.h>
#include "memory.h"
using namespace std;

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

int main() {
    HANDLE victim = GetCurrentProcess();
    const void *unreal = createUnrealData();
    cout << "Unreal data is at " << unreal << endl;
    auto regions = getProcessMemoryRegions(victim);
    ScanProcessMemory(victim, regions, "\0\0\0\0None\0\0\0\0\0\0\0", 16);
    return 0;
}
