# Winmem
SlimIO Windmem is a NodeJS binding which expose low-level Microsoft APIs on Memory.

This binding expose the following methods/struct:

- [GetPerfomanceInfo](https://docs.microsoft.com/fr-fr/windows/desktop/api/psapi/nf-psapi-getperformanceinfo)
- [GlobalMemoryStatusEx](https://msdn.microsoft.com/en-us/aa366589)
- [GetProcessMemoryInfo](https://docs.microsoft.com/fr-fr/windows/desktop/api/psapi/nf-psapi-getprocessmemoryinfo)
- [PERFORMANCE_INFORMATION](https://docs.microsoft.com/fr-fr/windows/desktop/api/psapi/ns-psapi-_performance_information)
- [MEMORYSTATUSEX](https://msdn.microsoft.com/en-us/aa366770)
- [PROCESS_MEMORY_COUNTERS_EX](https://docs.microsoft.com/fr-fr/windows/desktop/api/psapi/ns-psapi-_process_memory_counters_ex)

> !!! All method are called asynchronously without blocking the libuv event-loop !!!

## Getting Started

This package is available in the Node Package Repository and can be easily installed with [npm](https://docs.npmjs.com/getting-started/what-is-npm) or [yarn](https://yarnpkg.com).

```bash
$ npm i @slimio/winmem
# or
$ yarn add @slimio/winmem
```

## Usage example

Get, global or by process, memory informations !

```js
const winmem = require("@slimio/winmem");

async function main() {
    const perfomanceInfo = await winmem.getPerformanceInfo();
    console.log(perfomanceInfo);

    const globalMemory = await winmem.globalMemoryStatus();
    console.log(globalMemory);

    const processMemories = await windrive.getProcessMemory();
    for (const [processName, processMemory] of Object.entries(processMemories)) {
        console.log(`${processName} : ${JSON.stringify(processMemory, null, 4)}`);
    }
}
main().catch(console.error);
```

## API

### getPerformanceInfo: Promise< PerfomanceInfo >
Retrieves the currently performance information. Return a PerfomanceInfo Object.

```ts
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
```

### globalMemoryStatus: Promise< GlobalMemory >
Retrieves the currently gloval memory status. Return a GlobalMemory Object.

```ts
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
```

### getProcessMemory: Promise< ProcessMemory[] >
Retrieves all currently process memories. Return a ProcessMemories Object.

```ts
export interface ProcessMemories{
    [processName: string]: ProcessMemory;
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
```
> If error is null, all value is equal to 0

## How to build the project

Before building the project, be sure to get the following npm package installed:

- Install (or upgrade to) NodeJS v10+ and npm v6+
- [Windows build tools](https://www.npmjs.com/package/windows-build-tools)

Then, execute these commands in order:

```bash
$ npm install
$ npx node-gyp configure
$ npx node-gyp build
```

## Available commands

All projects commands are described here:

| command | description |
| --- | --- |
| npm run prebuild | Generate addon prebuild |
| npm run doc | Generate JSDoc .HTML documentation (in the /docs root directory) |
| npm run coverage | Generate coverage of tests |
| npm run report | Generate .HTML report of tests coverage |

> the report command have to be triggered after the coverage command.
