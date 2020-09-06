#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <windows.h>
#include <psapi.h>
using namespace std;

typedef map<string, DWORD> ProcessIndex;

string getProcessName(DWORD pid) {
    const static size_t nameBufferSize = MAX_PATH;
    char imageName[nameBufferSize] = {0};

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if(hProcess) {
        DWORD status = GetProcessImageFileName(hProcess, imageName, nameBufferSize);
        if(status) {
            const char *nameStart = strrchr(imageName, '\\');
            if(nameStart) {
                strcpy(imageName, nameStart + 1);
            }
        }
        CloseHandle(hProcess);
    }
    return imageName;
}

ProcessIndex getAllProcessIds() {
    DWORD processes[1024] = {0};
    DWORD numProcesses;
    ProcessIndex index;

    if(!EnumProcesses(processes, sizeof(processes), &numProcesses)) {
        cerr << "Failed to enumerate processes!\n";
    } else {
        numProcesses /= sizeof(DWORD);
        for(int i = 0; i < numProcesses; i++ ) {
            string name = getProcessName(processes[i]);
            if(name.length()) { // skip processes we can't access
                // Do everything in lower case so we are not case sensitive
                transform(name.begin(), name.end(), name.begin(), ::tolower);
                // NB: On collision only the first process is saved
                index.emplace(name, processes[i]);
            }
        }
    }

    return index;
}

DWORD getProcessIdByName(string name) {
    // Do everything in lower case so we are not case sensitive
    transform(name.begin(), name.end(), name.begin(), ::tolower);
    ProcessIndex index = getAllProcessIds();
    return index[name];
}

HANDLE openProcessByName(string name) {
    DWORD pid = getProcessIdByName(name);
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    return hProcess;
}
