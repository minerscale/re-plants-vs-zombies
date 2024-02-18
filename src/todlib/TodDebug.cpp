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

void TodTraceWithoutSpamming(const char *theFormat, ...) {
    static time_t gLastTraceTime = 0;
    const time_t aTime = time(nullptr);
    if (aTime < gLastTraceTime) {
        return;
    }

    gLastTraceTime = aTime;
    char aButter[1024];
    va_list argList;
    va_start(argList, theFormat);
    const int aCount = TodVsnprintf(aButter, sizeof(aButter), theFormat, argList);
    va_end(argList);

    if (aButter[aCount - 1] != '\n') {
        if (aCount + 1 < 1024) {
            aButter[aCount] = '\n';
            aButter[aCount + 1] = '\0';
        } else {
            aButter[aCount - 1] = '\n';
        }
    }

    printf("%s", aButter);
}

/*
void TodReportError(LPEXCEPTION_POINTERS exceptioninfo, const char* theMessage)
{
    (void)theMessage;
    Sexy::SEHCatcher::UnhandledExceptionFilter(exceptioninfo);
}*/

/*
long __stdcall TodUnhandledExceptionFilter(LPEXCEPTION_POINTERS exceptioninfo)
{
    if (gInAssert)
    {
        TodLog("Exception during exception processing");
    }
    else
    {
        gInAssert = true;
        TodLog("\nUnhandled Exception");
        TodReportError(exceptioninfo, "Unhandled Exception");
        gInAssert = false;
    }

    return EXCEPTION_EXECUTE_HANDLER;
}*/

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
