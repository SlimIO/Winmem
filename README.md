# Winmem
![version](https://img.shields.io/badge/dynamic/json.svg?url=https://raw.githubusercontent.com/SlimIO/Winmem/master/package.json&query=$.version&label=Version)
![N-API](https://img.shields.io/badge/N--API-v3-green.svg)
[![Maintenance](https://img.shields.io/badge/Maintained%3F-yes-green.svg)](https://github.com/SlimIO/Winmem/commit-activity)
[![GitHub license](https://img.shields.io/github/license/Naereen/StrapDown.js.svg)](https://github.com/SlimIO/Winmem/blob/master/LICENSE)
![2DEP](https://img.shields.io/badge/Dependencies-2-yellow.svg)
[![Known Vulnerabilities](https://snyk.io/test/github/SlimIO/Windrive/badge.svg?targetFile=package.json)](https://snyk.io/test/github/SlimIO/Windrive?targetFile=package.json)
[![Build Status](https://travis-ci.com/SlimIO/Winmem.svg?branch=master)](https://travis-ci.com/SlimIO/Winmem) [![Greenkeeper badge](https://badges.greenkeeper.io/SlimIO/Winmem.svg)](https://greenkeeper.io/)

SlimIO Winmem is a Node.js binding which expose low-level Microsoft APIs on Memory.

This binding expose the following methods/struct:

- [GetPerfomanceInfo](https://docs.microsoft.com/fr-fr/windows/desktop/api/psapi/nf-psapi-getperformanceinfo)
- [GlobalMemoryStatusEx](https://msdn.microsoft.com/en-us/aa366589)
- [GetProcessMemoryInfo](https://docs.microsoft.com/fr-fr/windows/desktop/api/psapi/nf-psapi-getprocessmemoryinfo)
- [PERFORMANCE_INFORMATION](https://docs.microsoft.com/fr-fr/windows/desktop/api/psapi/ns-psapi-_performance_information)
- [MEMORYSTATUSEX](https://msdn.microsoft.com/en-us/aa366770)
- [PROCESS_MEMORY_COUNTERS_EX](https://docs.microsoft.com/fr-fr/windows/desktop/api/psapi/ns-psapi-_process_memory_counters_ex)

> ⚠️ All methods are handled asynchronously in a Worker to not starve the event-loop.

## Requirements
- Node.js v10 or higher.

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

<details><summary>getPerformanceInfo: Promise< PerfomanceInfo ></summary>
<br />

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
</details>

<details><summary>globalMemoryStatus: Promise< GlobalMemory ></summary>
<br />

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
</details>

<details><summary>getProcessMemory: Promise< ProcessMemory[] ></summary>
<br />

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
> If the result of `error` property is `null`, all properties (except `processId`) are equal to `0`
</details>

## Contribution Guidelines
To contribute to the project, please read the [code of conduct](https://github.com/SlimIO/Governance/blob/master/COC_POLICY.md) and the guide for [N-API compilation](https://github.com/SlimIO/Governance/blob/master/docs/native_addons.md).

## License
MIT

