#include <windows.h>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <psapi.h>
#include <tlhelp32.h>
#include "napi.h"

using namespace std;
using namespace Napi;

/*
 * Retrieve last message error with code of GetLastError()
 */
string getLastErrorMessage() {
    char err[256];
    FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        err, 255, NULL
    );
    return string(err);
}

Value getPerformanceInfo(const CallbackInfo& info) {
    Env env = info.Env();

    PERFORMANCE_INFORMATION PerformanceInformation;
    SecureZeroMemory(&PerformanceInformation, sizeof(PERFORMANCE_INFORMATION));

    BOOL status = GetPerformanceInfo(&PerformanceInformation, sizeof(PerformanceInformation));
    if (!status) {
        Error::New(env, getLastErrorMessage()).ThrowAsJavaScriptException();
        return env.Null();
    }

    Object ret = Object::New(env);
    ret.Set("commitTotal", PerformanceInformation.CommitTotal);
    ret.Set("commitLimit", PerformanceInformation.CommitLimit);
    ret.Set("commitPeak", PerformanceInformation.CommitPeak);
    ret.Set("physicalTotal", PerformanceInformation.PhysicalTotal);
    ret.Set("physicalAvailable", PerformanceInformation.PhysicalAvailable);
    ret.Set("systemCache", PerformanceInformation.SystemCache);
    ret.Set("kernelTotal", PerformanceInformation.KernelTotal);
    ret.Set("kernelPaged", PerformanceInformation.KernelPaged);
    ret.Set("kernelNonpaged", PerformanceInformation.KernelNonpaged);
    ret.Set("pageSize", PerformanceInformation.PageSize);
    ret.Set("handleCount", PerformanceInformation.HandleCount);
    ret.Set("processCount", PerformanceInformation.ProcessCount);
    ret.Set("threadCount", PerformanceInformation.ThreadCount);

    return ret;
}

Value globalMemoryStatus(const CallbackInfo& info) {
    Env env = info.Env();

    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);

    BOOL status = GlobalMemoryStatusEx(&statex);
    if (!status) {
        Error::New(env, getLastErrorMessage()).ThrowAsJavaScriptException();
        return env.Null();
    }

    Object ret = Object::New(env);
    ret.Set("dwMemoryLoad", statex.dwMemoryLoad);
    ret.Set("ullTotalPhys", statex.ullTotalPhys);
    ret.Set("ullAvailPhys", statex.ullAvailPhys);
    ret.Set("ullTotalPageFile", statex.ullTotalPageFile);
    ret.Set("ullAvailPageFile", statex.ullAvailPageFile);
    ret.Set("ullTotalVirtual", statex.ullTotalVirtual);
    ret.Set("ullAvailVirtual", statex.ullAvailVirtual);
    ret.Set("ullAvailExtendedVirtual", statex.ullAvailExtendedVirtual);

    return ret;
}

/**
 * Retrieve Process Name and ID
 * 
 * @header: tlhelp32.h
 * @doc: https://docs.microsoft.com/en-us/windows/desktop/api/tlhelp32/nf-tlhelp32-createtoolhelp32snapshot
 * @doc: https://docs.microsoft.com/en-us/windows/desktop/api/tlhelp32/nf-tlhelp32-process32first
 * @doc: https://docs.microsoft.com/en-us/windows/desktop/api/tlhelp32/nf-tlhelp32-process32next
 */ 
BOOL getProcessNameAndId(vector<pair<DWORD, string>>* vPairProc) {
    PROCESSENTRY32 pe32;
    HANDLE hProcessSnap;
 
    // Take a snapshot of all processes in the system.
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        return false;
    }
 
    // Set the size of the structure before using it.
    pe32.dwSize = sizeof(PROCESSENTRY32);
 
    // Retrieve information about the first process,
    // and exit if unsuccessful
    if (!Process32First(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap); // clean the snapshot object
        return false;
    }
    
    // Insert rows in the processes map
    do {
        wstring wSzExeFile((wchar_t*) pe32.szExeFile);
        vPairProc->push_back(make_pair(pe32.th32ProcessID, string(wSzExeFile.begin(), wSzExeFile.end())));
    } while (Process32Next(hProcessSnap, &pe32));
 
    CloseHandle(hProcessSnap);
    return true;
}

Value getProcessMemory(const CallbackInfo& info){
    Env env = info.Env();

    vector<pair<DWORD, string>> processNameAndId;
    BOOL status = getProcessNameAndId(&processNameAndId);
    if (!status) {
        Error::New(env, getLastErrorMessage()).ThrowAsJavaScriptException();
        return env.Null();
    }

    BOOL statusProcessMemory;
    HANDLE processHandle;
    Object ProcessMemories = Object::New(env);
    for(size_t i = 0; i < processNameAndId.size(); i++){
        HandleScope scope(info.Env());
        PROCESS_MEMORY_COUNTERS_EX ProcessMemory;
        SecureZeroMemory(&ProcessMemory, sizeof(PROCESS_MEMORY_COUNTERS_EX));

        // Retrieve process name and id
        pair<DWORD, string> process = processNameAndId.at(i);

        // Create default Object
        Object oProcessMemory = Object::New(env);
        ProcessMemories.Set(process.second, oProcessMemory);

        // Open process handle!
        processHandle = OpenProcess(PROCESS_QUERY_INFORMATION, false, process.first);
        if(processHandle == NULL) {
            stringstream error;
            error << "Failed to OpenProcess id(" << process.first << "), error code (" << GetLastError() << ")" << endl;
            oProcessMemory.Set("error", error.str());
            continue;
        }

        statusProcessMemory = GetProcessMemoryInfo(processHandle, (PROCESS_MEMORY_COUNTERS*) &ProcessMemory, sizeof(ProcessMemory));
        if (!statusProcessMemory) {
            CloseHandle(processHandle);
            continue;
        }

        oProcessMemory.Set("error", env.Null());
        oProcessMemory.Set("processId", process.first);
        oProcessMemory.Set("pageFaultCount", ProcessMemory.PageFaultCount);
        oProcessMemory.Set("peakWorkingSetSize", ProcessMemory.PeakWorkingSetSize);
        oProcessMemory.Set("workingSetSize", ProcessMemory.WorkingSetSize);
        oProcessMemory.Set("quotaPeakPagedPoolUsage", ProcessMemory.QuotaPeakPagedPoolUsage);
        oProcessMemory.Set("quotaPagedPoolUsage", ProcessMemory.QuotaPagedPoolUsage);
        oProcessMemory.Set("quotaPeakNonPagedPoolUsage", ProcessMemory.QuotaPeakNonPagedPoolUsage);
        oProcessMemory.Set("quotaNonPagedPoolUsage", ProcessMemory.QuotaNonPagedPoolUsage);
        oProcessMemory.Set("pagefileUsage", ProcessMemory.PagefileUsage);
        oProcessMemory.Set("peakPagefileUsage", ProcessMemory.PeakPagefileUsage);
        oProcessMemory.Set("privateUsage", ProcessMemory.PrivateUsage);

        CloseHandle(processHandle);
    }

    return ProcessMemories;
}


// Initialize Native Addon
Object Init(Env env, Object exports) {
    exports.Set("getPerformanceInfo", Function::New(env, getPerformanceInfo));
    exports.Set("globalMemoryStatus", Function::New(env, globalMemoryStatus));
    exports.Set("getProcessMemory", Function::New(env, getProcessMemory));
    return exports;
}

// Export Addon as winmem
NODE_API_MODULE(winmem, Init)
