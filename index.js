/**
 * @namespace winmem
 * @desc Windows Drive (disk) & Devices - Node.JS low level binding
 */
const winmem = require("bindings")("winmem.node");
console.time("getPerformanceInfo");
winmem.getPerformanceInfo();
console.timeEnd("getPerformanceInfo");

console.time("globalMemoryStatus");
winmem.globalMemoryStatus();
console.timeEnd("globalMemoryStatus");


console.time("getProcessMemory");
winmem.getProcessMemory();
console.timeEnd("getProcessMemory");
