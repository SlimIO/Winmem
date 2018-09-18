declare namespace Winmem {

    export interface PerfomanceInfo {
        commitTotal: number;
        commitLimit: number;
        commitPeak: number;
        physicalTotal: number;
        physicalAvailable: number;
        systemCache: number;
        kernelTotal: number;
        kernelPaged: number;
        kernelNonpaged: number;
        pageSize: number;
        handleCount: number;
        processCount: number;
        threadCount: number;
    }

    export interface GlobalMemory {
        dwMemoryLoad: number;
        ullTotalPhys: number;
        ullAvailPhys: number;
        ullTotalPageFile: number;
        ullAvailPageFile: number;
        ullTotalVirtual: number;
        ullAvailVirtual: number;
        ullAvailExtendedVirtual: number;
    }

    export interface ProcessMemory {
        error: string;
        processId: number;
        pageFaultCount: number;
        peakWorkingSetSize: number;
        workingSetSize: number;
        quotaPeakPagedPoolUsage: number;
        quotaPagedPoolUsage: number;
        quotaPeakNonPagedPoolUsage: number;
        quotaNonPagedPoolUsage: number;
        pagefileUsage: number;
        peakPagefileUsage: number;
        privateUsage: number;
    }

    export interface ProcessMemories{
        [processName: string]: ProcessMemory;
    }

    export function getPerformanceInfo(): Promise<PerfomanceInfo[]>;
    export function globalMemoryStatus(): Promise<GlobalMemory>;
    export function getProcessMemory(): Promise<ProcessMemories>;
}

export as namespace Winmem;
export = Winmem;
