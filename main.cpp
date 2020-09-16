#include <iostream>
#include <windows.h>
#include "memory.h"
#include "processes.h"
#include "unreal.h"
using namespace std;

int main() {
    auto unreal_ptr = createUnrealData();
    cout << "Unreal data is at " << unreal_ptr.get()->getBaseAddress() << endl;

    HANDLE victim = openProcessByName("unreal-hacking.exe");
    auto regions = getProcessMemoryRegions(victim);

    std::vector<RemoteDataReference> remoteMatches(
        ScanProcessMemory(victim, regions, "\0\0\0\0None\0\0\0\0\0\0\0", 16)
    );

    for(auto match : remoteMatches) {
        cout << "Found at " << match.remote << endl;
    }

    return 0;
}
