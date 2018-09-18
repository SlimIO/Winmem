/**
 * @namespace winmem
 * @desc Windows Drive (disk) & Devices - Node.JS low level binding
 */
const winmem = require("bindings")("winmem.node");

/**
 * @async
 * @function getPerformanceInfo
 * @memberof winmem#
 * @desc Retrieves the currently performance information. Return a PerfomanceInfo Object.
 * @return {Promise<Winmem.PerfomanceInfo>}
 *
 * @version 1.0.0
 * @example
 * const { getPerformanceInfo } = require("@slimio/winmem");
 * async function main() {
 *     const perfomanceInfo = await getPerformanceInfo();
 *     console.log(JSON.stringify(perfomanceInfo, null, 4));
 * }
 * main().catch(console.error);
 */
function getPerformanceInfo() {
    return new Promise((resolve, reject) => {
        winmem.getPerformanceInfo((error, performanceInfo) => {
            if (error) {
                return reject(error);
            }

            return resolve(performanceInfo);
        });
    });
}


/**
 * @async
 * @function globalMemoryStatus
 * @memberof winmem#
 * @desc Retrieves the currently global memory status. Return a GlobalMemory Object.
 * @return {Promise<Winmem.PerfomanceInfo>}
 *
 * @version 1.0.0
 * @example
 * const { globalMemoryStatus } = require("@slimio/winmem");
 * async function main() {
 *     const globalMemory = await globalMemoryStatus();
 *     console.log(JSON.stringify(globalMemory, null, 4));
 * }
 * main().catch(console.error);
 */
function globalMemoryStatus() {
    return new Promise((resolve, reject) => {
        winmem.globalMemoryStatus((error, globalMemory) => {
            if (error) {
                return reject(error);
            }

            return resolve(globalMemory);
        });
    });
}


/**
 * @async
 * @function getProcessMemory
 * @memberof winmem#
 * @desc Retrieves the currently process memories. An array of ProcessMemory is returned.
 * @return {Promise<Winmem.ProcessMemory[]>}
 *
 * @version 1.0.0
 * @example
 * const { getProcessMemory } = require("@slimio/winmem");
 * async function main() {
 *     const processMemories = await getProcessMemory();
 *     console.log(JSON.stringify(processMemories, null, 4));
 * }
 * main().catch(console.error);
 */
function getProcessMemory() {
    return new Promise((resolve, reject) => {
        winmem.getProcessMemory((error, processMemory) => {
            if (error) {
                return reject(error);
            }

            return resolve(processMemory);
        });
    });
}

// Export all methods
module.exports = {
    getPerformanceInfo,
    globalMemoryStatus,
    getProcessMemory
};
