#include "TodDebug.h"
#include "TodCommon.h"
#include "framework/SexyAppBase.h"
#include "misc/Debug.h"
#include <cstdarg>

using namespace Sexy;

#define MAX_PATH 260
static char gLogFileName[MAX_PATH];
static char gDebugDataFolder[MAX_PATH];

// 0x514EA0
void TodErrorMessageBox(const char *theMessage, const char *theTitle) {
    throw std::runtime_error("Error Box\n--" + std::string(theTitle) + "--\n" + theMessage);
}

void TodTraceMemory() {}

void *TodMalloc(int theSize) {
    TOD_ASSERT(theSize > 0);
    return malloc(theSize);
}

void TodFree(void *theBlock) {
    if (theBlock != nullptr) {
        free(theBlock);
    }
}

void TodLogString(const char *theMsg) {
    FILE *f = fopen(gLogFileName, "a");
    if (f == nullptr) {
        fmt::println(_S("Failed to open log file"));
    }

    if (fwrite(theMsg, strlen(theMsg), 1, f) != 1) {
        fmt::println(_S("Failed to write to log file"));
    }

    fclose(f);
}

void TodHesitationTrace(...) {}

void TodAssertInitForApp() {
    MkDir(GetAppDataFolder() + "userdata");
    const std::string aRelativeUserPath = GetAppDataFolder() + "userdata/";
    strcpy(gDebugDataFolder, GetFullPath(aRelativeUserPath).c_str());
    strcpy(gLogFileName, gDebugDataFolder);
    strcpy(gLogFileName + strlen(gLogFileName), "log.txt");
    TOD_ASSERT(strlen(gLogFileName) < MAX_PATH);

    const time_t aclock = time(nullptr);
    TodLog("Started {}\n", asctime(localtime(&aclock)));
}
