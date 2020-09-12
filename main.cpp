#include <iostream>
#include <windows.h>
#include "memory.h"
#include "processes.h"
#include "unreal.h"
using namespace std;

int main() {
    HANDLE victim = openProcessByName("unreal-hacking.exe");
    const void *unreal = createUnrealData();
    cout << "Unreal data is at " << unreal << endl;
    auto regions = getProcessMemoryRegions(victim);
    ScanProcessMemory(victim, regions, "\0\0\0\0None\0\0\0\0\0\0\0", 16);
    return 0;
}
