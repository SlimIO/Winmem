// Require Third-party dependencies
const test = require("ava");
const is = require("@slimio/is");

// Require package
const winmem = require("../index");

// Test method getPerformanceInfo
test("getPerformanceInfo()", async function getPerformanceInfo(assert) {
    assert.is(Reflect.has(winmem, "getPerformanceInfo"), true);

    const performanceInfo = await winmem.getPerformanceInfo();
    assert.is(is.plainObject(performanceInfo), true);

    assert.is(is.number(performanceInfo.commitTotal), true);
    assert.is(is.number(performanceInfo.commitLimit), true);
    assert.is(is.number(performanceInfo.commitPeak), true);
    assert.is(is.number(performanceInfo.physicalTotal), true);
    assert.is(is.number(performanceInfo.physicalAvailable), true);
    assert.is(is.number(performanceInfo.systemCache), true);
    assert.is(is.number(performanceInfo.kernelTotal), true);
    assert.is(is.number(performanceInfo.kernelPaged), true);
    assert.is(is.number(performanceInfo.kernelNonpaged), true);
    assert.is(is.number(performanceInfo.pageSize), true);
    assert.is(is.number(performanceInfo.handleCount), true);
    assert.is(is.number(performanceInfo.processCount), true);
    assert.is(is.number(performanceInfo.threadCount), true);
});

// Test method globalMemoryStatus
test("globalMemoryStatus()", async function globalMemoryStatus(assert) {
    assert.is(Reflect.has(winmem, "globalMemoryStatus"), true);

    const globalMemory = await winmem.globalMemoryStatus();
    assert.is(is.plainObject(globalMemory), true);

    assert.is(is.number(globalMemory.dwMemoryLoad), true);
    assert.is(is.number(globalMemory.ullTotalPhys), true);
    assert.is(is.number(globalMemory.ullAvailPhys), true);
    assert.is(is.number(globalMemory.ullTotalPageFile), true);
    assert.is(is.number(globalMemory.ullAvailPageFile), true);
    assert.is(is.number(globalMemory.ullTotalVirtual), true);
    assert.is(is.number(globalMemory.ullAvailVirtual), true);
    assert.is(is.number(globalMemory.ullAvailExtendedVirtual), true);
});

// Test method getProcessMemory
test("getProcessMemory()", async function getProcessMemory(assert) {
    assert.is(Reflect.has(winmem, "getProcessMemory"), true);

    const processMemories = await winmem.getProcessMemory();
    assert.is(is.plainObject(processMemories), true);

    for (const [processName, processMemory] of Object.entries(processMemories)) {
        assert.is(is.string(processName), true);
        assert.is(is.number(processMemory.processId), true);
        assert.is(is.plainObject(processMemory), true);

        assert.is(Reflect.has(processMemory, "error"), true);
        if (is.string(processMemory.error)) {
            assert.is(processMemory.pageFaultCount, 0);
            assert.is(processMemory.peakWorkingSetSize, 0);
            assert.is(processMemory.workingSetSize, 0);
            assert.is(processMemory.quotaPeakPagedPoolUsage, 0);
            assert.is(processMemory.quotaPagedPoolUsage, 0);
            assert.is(processMemory.quotaPeakNonPagedPoolUsage, 0);
            assert.is(processMemory.quotaNonPagedPoolUsage, 0);
            assert.is(processMemory.pagefileUsage, 0);
            assert.is(processMemory.peakPagefileUsage, 0);
            assert.is(processMemory.privateUsage, 0);
        }
        else if (is.nullOrUndefined(processMemory.error)) {
            assert.is(is.number(processMemory.pageFaultCount), true);
            assert.is(is.number(processMemory.peakWorkingSetSize), true);
            assert.is(is.number(processMemory.workingSetSize), true);
            assert.is(is.number(processMemory.quotaPeakPagedPoolUsage), true);
            assert.is(is.number(processMemory.quotaPagedPoolUsage), true);
            assert.is(is.number(processMemory.quotaPeakNonPagedPoolUsage), true);
            assert.is(is.number(processMemory.quotaNonPagedPoolUsage), true);
            assert.is(is.number(processMemory.pagefileUsage), true);
            assert.is(is.number(processMemory.peakPagefileUsage), true);
            assert.is(is.number(processMemory.privateUsage), true);
        }
        else {
            assert.fail("Invalid type for error field. It should be either string or null.");
        }
    }
});
