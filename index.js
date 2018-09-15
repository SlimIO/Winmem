/**
 * @namespace winmem
 * @desc Windows Drive (disk) & Devices - Node.JS low level binding
 */
const winmem = require("bindings")("winmem.node");


// console.time("getPerformanceInfo");
// winmem.getPerformanceInfo((error, data) => {
//     if (error) {
//         console.log(`Error : ${error}`);
//     }
//     // console.log("Data :");
//     // console.log(data);
// });
// console.timeEnd("getPerformanceInfo");

// console.time("globalMemoryStatus");
// winmem.globalMemoryStatus((error, data) => {
//     if (error) {
//         console.log(`Error : ${error}`);
//     }
//     console.log("Data :");
//     console.log(data);
// });
// console.timeEnd("globalMemoryStatus");


// console.time("getProcessMemory");
console.log(winmem.getProcessMemory());
// console.timeEnd("getProcessMemory");
