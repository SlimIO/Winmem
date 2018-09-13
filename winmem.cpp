#include <windows.h>
#include <sstream>
#include <string>
#include "napi.h"

using namespace std;
using namespace Napi;

/*
 * Buffer length for logical drives names
 * TODO: Best value ? (120 is almost good for 30 disks)
 */
#define DRIVER_LENGTH 120


// Initialize Native Addon
Object Init(Env env, Object exports) {
    return exports;
}

// Export Addon as winmem
NODE_API_MODULE(winmem, Init)
