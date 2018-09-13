#include <windows.h>
#include <comdef.h>
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

// Export Addon as winmem
NODE_API_MODULE(windrive, Init)
