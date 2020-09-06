#include <iostream>
#include <windows.h>
#include "memory.h"
#include "processes.h"
using namespace std;

void *createUnrealData() {
    char *dataBlock = new char[1024];
    char *dataBlockStart = dataBlock;
    memset(dataBlock, 0, 4);    //4-byte index
    dataBlock += 4;
    memcpy(dataBlock, "None", 4);   //Object name (None is always first)
    dataBlock += 4;
    memset(dataBlock, 0, 8);    //8-byte null terminator
    dataBlock += 8;
    memcpy(dataBlock, &dataBlock, 8);   //Object address
    dataBlock += 8;
    memset(dataBlock, 2, 1);    //index of next object (counts by 2's)
    dataBlock += 1;
    memset(dataBlock, 0, 3);
    dataBlock += 3;

    return dataBlockStart;
}

int main() {
    HANDLE victim = openProcessByName("unreal-hacking.exe");
    const void *unreal = createUnrealData();
    cout << "Unreal data is at " << unreal << endl;
    auto regions = getProcessMemoryRegions(victim);
    ScanProcessMemory(victim, regions, "\0\0\0\0None\0\0\0\0\0\0\0", 16);
    return 0;
}
