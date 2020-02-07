#include <windows.h>
#include <string>
#include <sstream>
#include <vector>
#include "Psapi.h"
#include <tlhelp32.h>
#include "napi.h"

/*
 * Retrieve last message error with code of GetLastError()
 */
std::string getLastErrorMessage() {
    char err[256];
    FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        err, 255, NULL
    );
    return std::string(err);
}

/**
 * Retrieve Process Name and ID
 *
 * @header: tlhelp32.h
 * @doc: https://docs.microsoft.com/en-us/windows/desktop/api/tlhelp32/nf-tlhelp32-createtoolhelp32snapshot
 * @doc: https://docs.microsoft.com/en-us/windows/desktop/api/tlhelp32/nf-tlhelp32-process32first
 * @doc: https://docs.microsoft.com/en-us/windows/desktop/api/tlhelp32/nf-tlhelp32-process32next
 */
BOOL getProcessNameAndId(std::vector<std::pair<DWORD, std::string>>* vPairProc) {
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
        std::wstring wSzExeFile((wchar_t*) pe32.szExeFile);
        vPairProc->push_back(std::make_pair(pe32.th32ProcessID, std::string(wSzExeFile.begin(), wSzExeFile.end())));
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    return true;
}

/*
 * Asycnronous Worker to Retrieve Windows Performance Info
 *
 * @header: psapi.h
 * @doc: https://docs.microsoft.com/fr-fr/windows/desktop/api/psapi/nf-psapi-getperformanceinfo
 * @doc: https://docs.microsoft.com/fr-fr/windows/desktop/api/psapi/ns-psapi-_performance_information
 * @doc: https://msdn.microsoft.com/en-us/b27ca747-8fd2-4267-9979-4e2e14a5a19f
 */
class PerformanceInfoWorker : public Napi::AsyncWorker {
    public:
        PerformanceInfoWorker(Napi::Function& callback) : AsyncWorker(callback) {}
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

        void OnError(const Napi::Error& e) {
            std::stringstream error;
            error << e.what() << getLastErrorMessage();
            Napi::Error::New(Env(), error.str()).ThrowAsJavaScriptException();
        }

        void OnOK() {
            Napi::HandleScope scope(Env());
            Napi::Object ret = Napi::Object::New(Env());

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

/*
 * Asycnronous Worker to Retrieve Windows Global Memory Status
 *
 * @doc: https://msdn.microsoft.com/en-us/aa366589
 * @doc: https://msdn.microsoft.com/en-us/aa366770
 */
class globalMemoryWorker : public Napi::AsyncWorker {
    public:
        globalMemoryWorker(Napi::Function& callback) : AsyncWorker(callback) {}
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

        void OnError(const Napi::Error& e) {
            std::stringstream error;
            error << e.what() << getLastErrorMessage();
            Napi::Error::New(Env(), error.str()).ThrowAsJavaScriptException();
        }

        void OnOK() {
            Napi::HandleScope scope(Env());
            Napi::Object ret = Napi::Object::New(Env());

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

/*
 * Asycnronous Worker to Retrieve Windows Process Memory Info
 *
 * @header: psapi.h
 * @doc: https://docs.microsoft.com/fr-fr/windows/desktop/api/psapi/nf-psapi-getprocessmemoryinfo
 * @doc: https://docs.microsoft.com/fr-fr/windows/desktop/api/psapi/ns-psapi-_process_memory_counters_ex
 */
class ProcessMemoryWorker : public Napi::AsyncWorker {
    public:
        ProcessMemoryWorker(Napi::Function& callback) : AsyncWorker(callback) {}
        ~ProcessMemoryWorker() {}
    private:
        struct PROCESS_MEMORY{
            std::string error;
            DWORD processId;
            std::string processName;
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

        std::vector<PROCESS_MEMORY> vProcessMemory;

        void Execute() {
            std::vector<std::pair<DWORD, std::string>> processNameAndId;
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
                std::pair<DWORD, std::string> process = processNameAndId.at(i);

                nProcessMemory.processId = process.first;
                nProcessMemory.processName = process.second;

                // Open process handle!
                processHandle = OpenProcess(PROCESS_QUERY_INFORMATION, false, process.first);
                if(processHandle == NULL) {
                    std::stringstream error;
                    error << "Failed to OpenProcess id(" << process.first << "), error code (" << GetLastError() << ")" << std::endl;
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

        void OnError(const Napi::Error& e) {
            std::stringstream error;
            error << e.what() << getLastErrorMessage();
            Napi::Error::New(Env(), error.str()).ThrowAsJavaScriptException();
        }

        void OnOK() {
            Napi::HandleScope scope(Env());
            Napi::Object ret = Napi::Object::New(Env());

            for(size_t i = 0; i < vProcessMemory.size(); i++){
                Napi::Object oProcessMemory = Napi::Object::New(Env());
                PROCESS_MEMORY ProcessMemory = vProcessMemory.at(i);

                ret.Set(ProcessMemory.processName, oProcessMemory);
                if(ProcessMemory.error == std::string("")) {
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

Napi::Value globalMemoryStatus(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    Napi::Function cb;

    if (info.Length() < 1) {
        Napi::Error::New(env, "Wrong number of argument provided!").ThrowAsJavaScriptException();
        return env.Null();
    }
    if (!info[0].IsFunction()) {
        Napi::Error::New(env, "argument callback should be a Function!").ThrowAsJavaScriptException();
        return env.Null();
    }

    cb = info[0].As<Napi::Function>();
    (new globalMemoryWorker(cb))->Queue();

    return env.Undefined();
}

Napi::Value getPerformanceInfo(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    Napi::Function cb;

    if (info.Length() < 1) {
        Napi::Error::New(env, "Wrong number of argument provided!").ThrowAsJavaScriptException();
        return env.Null();
    }
    if (!info[0].IsFunction()) {
        Napi::Error::New(env, "argument callback should be a Function!").ThrowAsJavaScriptException();
        return env.Null();
    }

    cb = info[0].As<Napi::Function>();
    (new PerformanceInfoWorker(cb))->Queue();

    return env.Undefined();
}

Napi::Value getProcessMemory(const Napi::CallbackInfo& info){
    Napi::Env env = info.Env();
    Napi::Function cb;

    if (info.Length() < 1) {
        Napi::Error::New(env, "Wrong number of argument provided!").ThrowAsJavaScriptException();
        return env.Null();
    }
    if (!info[0].IsFunction()) {
        Napi::Error::New(env, "argument callback should be a Function!").ThrowAsJavaScriptException();
        return env.Null();
    }

    cb = info[0].As<Napi::Function>();
    (new ProcessMemoryWorker(cb))->Queue();

    return env.Undefined();
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("getPerformanceInfo", Napi::Function::New(env, getPerformanceInfo));
    exports.Set("globalMemoryStatus", Napi::Function::New(env, globalMemoryStatus));
    exports.Set("getProcessMemory", Napi::Function::New(env, getProcessMemory));

    return exports;
}

NODE_API_MODULE(winmem, Init)
