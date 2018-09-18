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
/*
 * @doc : Global Documentation : https://msdn.microsoft.com/en-us/b27ca747-8fd2-4267-9979-4e2e14a5a19f
 */

/*
 * Asycnronous Worker to Retrieve Windows Performance Info
 * 
 * @doc: https://docs.microsoft.com/fr-fr/windows/desktop/api/psapi/nf-psapi-getperformanceinfo
 * @doc: https://docs.microsoft.com/fr-fr/windows/desktop/api/psapi/ns-psapi-_performance_information
 */
class PerformanceInfoWorker : public AsyncWorker {
    public:
        PerformanceInfoWorker(Function& callback) : AsyncWorker(callback) {}
        ~PerformanceInfoWorker() {}
    private:
        PERFORMANCE_INFORMATION PerformanceInformation;
        
        void Execute(){
            SecureZeroMemory(&PerformanceInformation, sizeof(PERFORMANCE_INFORMATION));

            BOOL status = GetPerformanceInfo(&PerformanceInformation, sizeof(PerformanceInformation));
            if (!status) {
                return SetError("Failed status of GetPerformanceInfo");
            }
        }

        void OnError(const Error& e) {
            stringstream error;
            error << e.what() << getLastErrorMessage();
            Error::New(Env(), error.str()).ThrowAsJavaScriptException();
        }

        void OnOK() {
            HandleScope scope(Env());

            Object ret = Object::New(Env());
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

            Callback().Call({Env().Null(), ret});
        }
};

Value getPerformanceInfo(const CallbackInfo& info) {
    Env env = info.Env();

    // Check argument length!
    if (info.Length() < 1) {
        Error::New(env, "Wrong number of argument provided!").ThrowAsJavaScriptException();
        return env.Null();
    }

    // callback should be a Napi::Function
    if (!info[0].IsFunction()) {
        Error::New(env, "argument callback should be a Function!").ThrowAsJavaScriptException();
        return env.Null();
    }

    // Execute worker
    Function cb = info[0].As<Function>();
    (new PerformanceInfoWorker(cb))->Queue();
    
    return env.Undefined();
}

/*
 * Asycnronous Worker to Retrieve Windows Global Memory Status
 * 
 * @doc: https://msdn.microsoft.com/en-us/aa366589
 * @doc: https://msdn.microsoft.com/en-us/aa366770
 */
class globalMemoryWorker : public AsyncWorker {
    public:
        globalMemoryWorker(Function& callback) : AsyncWorker(callback) {}
        ~globalMemoryWorker() {}
    private:
        MEMORYSTATUSEX statex;
        
        void Execute() {
            SecureZeroMemory(&statex, sizeof(statex));
            statex.dwLength = sizeof(statex);
            BOOL status = GlobalMemoryStatusEx(&statex);
            if (!status) {
                return SetError("Failed status of GlobalMemoryStatusEx");
            }
        }

        void OnError(const Error& e) {
            stringstream error;
            error << e.what() << getLastErrorMessage();
            Error::New(Env(), error.str()).ThrowAsJavaScriptException();
        }

        void OnOK() {
            HandleScope scope(Env());

            Object ret = Object::New(Env());
            ret.Set("dwMemoryLoad", statex.dwMemoryLoad);
            ret.Set("ullTotalPhys", statex.ullTotalPhys);
            ret.Set("ullAvailPhys", statex.ullAvailPhys);
            ret.Set("ullTotalPageFile", statex.ullTotalPageFile);
            ret.Set("ullAvailPageFile", statex.ullAvailPageFile);
            ret.Set("ullTotalVirtual", statex.ullTotalVirtual);
            ret.Set("ullAvailVirtual", statex.ullAvailVirtual);
            ret.Set("ullAvailExtendedVirtual", statex.ullAvailExtendedVirtual);

            Callback().Call({Env().Null(), ret});
        }
};

Value globalMemoryStatus(const CallbackInfo& info) {
    Env env = info.Env();

    // Check argument length!
    if (info.Length() < 1) {
        Error::New(env, "Wrong number of argument provided!").ThrowAsJavaScriptException();
        return env.Null();
    }

    // callback should be a Napi::Function
    if (!info[0].IsFunction()) {
        Error::New(env, "argument callback should be a Function!").ThrowAsJavaScriptException();
        return env.Null();
    }

    Function cb = info[0].As<Function>();
    (new globalMemoryWorker(cb))->Queue();
    
    return env.Undefined();
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

/*
 * Asycnronous Worker to Retrieve Windows Process Memory Info
 * 
 * @doc: https://docs.microsoft.com/fr-fr/windows/desktop/api/psapi/nf-psapi-getprocessmemoryinfo
 * @doc: https://docs.microsoft.com/fr-fr/windows/desktop/api/psapi/ns-psapi-_process_memory_counters_ex
 */
class ProcessMemoryWorker : public AsyncWorker {
    public:
        ProcessMemoryWorker(Function& callback) : AsyncWorker(callback) {}
        ~ProcessMemoryWorker() {}
    private:
        struct PROCESS_MEMORY{
            string error;
            DWORD processId;
            string processName;
            DWORD  cb;
            DWORD  PageFaultCount;
            SIZE_T PeakWorkingSetSize;
            SIZE_T WorkingSetSize;
            SIZE_T QuotaPeakPagedPoolUsage;
            SIZE_T QuotaPagedPoolUsage;
            SIZE_T QuotaPeakNonPagedPoolUsage;
            SIZE_T QuotaNonPagedPoolUsage;
            SIZE_T PagefileUsage;
            SIZE_T PeakPagefileUsage;
            SIZE_T PrivateUsage;
        };

        vector<PROCESS_MEMORY> vProcessMemory;

        void Execute() {
            vector<pair<DWORD, string>> processNameAndId;
            BOOL status = getProcessNameAndId(&processNameAndId);
            if (!status) {
                SetError("Failed status of getProcessNameAndId");
            }

            BOOL statusProcessMemory;
            HANDLE processHandle;
            PROCESS_MEMORY_COUNTERS_EX ProcessMemory;
            PROCESS_MEMORY nProcessMemory;
            for(size_t i = 0; i < processNameAndId.size(); i++){
                SecureZeroMemory(&ProcessMemory, sizeof(ProcessMemory));
                SecureZeroMemory(&nProcessMemory, sizeof(nProcessMemory));

                // Retrieve process name and id
                pair<DWORD, string> process = processNameAndId.at(i);

                nProcessMemory.processId = process.first;
                nProcessMemory.processName = process.second;
                
                // Open process handle!
                processHandle = OpenProcess(PROCESS_QUERY_INFORMATION, false, process.first);
                if(processHandle == NULL) {
                    stringstream error;
                    error << "Failed to OpenProcess id(" << process.first << "), error code (" << GetLastError() << ")" << endl;
                    nProcessMemory.error = error.str();
                    vProcessMemory.push_back(nProcessMemory);
                    continue;
                }

                statusProcessMemory = GetProcessMemoryInfo(processHandle, (PROCESS_MEMORY_COUNTERS*) &ProcessMemory, sizeof(ProcessMemory));
                if (!statusProcessMemory) {
                    CloseHandle(processHandle);
                    vProcessMemory.push_back(nProcessMemory);
                    continue;
                }
                nProcessMemory.PageFaultCount = ProcessMemory.PageFaultCount;
                nProcessMemory.PeakWorkingSetSize = ProcessMemory.PeakWorkingSetSize;
                nProcessMemory.WorkingSetSize = ProcessMemory.WorkingSetSize;
                nProcessMemory.QuotaPeakPagedPoolUsage = ProcessMemory.QuotaPeakPagedPoolUsage;
                nProcessMemory.QuotaPagedPoolUsage = ProcessMemory.QuotaPagedPoolUsage;
                nProcessMemory.QuotaPeakNonPagedPoolUsage = ProcessMemory.QuotaPeakNonPagedPoolUsage;
                nProcessMemory.QuotaNonPagedPoolUsage = ProcessMemory.QuotaNonPagedPoolUsage;
                nProcessMemory.PagefileUsage = ProcessMemory.PagefileUsage;
                nProcessMemory.PeakPagefileUsage = ProcessMemory.PeakPagefileUsage;
                nProcessMemory.PrivateUsage = ProcessMemory.PrivateUsage;
                vProcessMemory.push_back(nProcessMemory);

                CloseHandle(processHandle);
            }
        }

        void OnError(const Error& e) {
            stringstream error;
            error << e.what() << getLastErrorMessage();
            Error::New(Env(), error.str()).ThrowAsJavaScriptException();
        }

        void OnOK() {
            HandleScope scope(Env());

            Object ret = Object::New(Env());
            for(size_t i = 0; i < vProcessMemory.size(); i++){
                Object oProcessMemory = Object::New(Env());

                PROCESS_MEMORY ProcessMemory = vProcessMemory.at(i);

                ret.Set(ProcessMemory.processName, oProcessMemory);
                if(ProcessMemory.error == string("")) {
                    oProcessMemory.Set("error", Env().Null());
                }
                else {
                    oProcessMemory.Set("error", ProcessMemory.error);
                }
                oProcessMemory.Set("processId",                     ProcessMemory.processId);
                oProcessMemory.Set("pageFaultCount",                ProcessMemory.PageFaultCount);
                oProcessMemory.Set("peakWorkingSetSize",            ProcessMemory.PeakWorkingSetSize);
                oProcessMemory.Set("workingSetSize",                ProcessMemory.WorkingSetSize);
                oProcessMemory.Set("quotaPeakPagedPoolUsage",       ProcessMemory.QuotaPeakPagedPoolUsage);
                oProcessMemory.Set("quotaPagedPoolUsage",           ProcessMemory.QuotaPagedPoolUsage);
                oProcessMemory.Set("quotaPeakNonPagedPoolUsage",    ProcessMemory.QuotaPeakNonPagedPoolUsage);
                oProcessMemory.Set("quotaNonPagedPoolUsage",        ProcessMemory.QuotaNonPagedPoolUsage);
                oProcessMemory.Set("pagefileUsage",                 ProcessMemory.PagefileUsage);
                oProcessMemory.Set("peakPagefileUsage",             ProcessMemory.PeakPagefileUsage);
                oProcessMemory.Set("privateUsage",                  ProcessMemory.PrivateUsage);
            }
            Callback().Call({Env().Null(), ret});
        }
};

Value getProcessMemory(const CallbackInfo& info){
    Env env = info.Env();

    // Check argument length!
    if (info.Length() < 1) {
        Error::New(env, "Wrong number of argument provided!").ThrowAsJavaScriptException();
        return env.Null();
    }

    // callback should be a Napi::Function
    if (!info[0].IsFunction()) {
        Error::New(env, "argument callback should be a Function!").ThrowAsJavaScriptException();
        return env.Null();
    }

    Function cb = info[0].As<Function>();
    (new ProcessMemoryWorker(cb))->Queue();
    
    return env.Undefined();
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
