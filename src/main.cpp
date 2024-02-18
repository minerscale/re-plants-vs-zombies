#if defined(_WIN32) && !defined(_DEBUG)
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

#ifdef __ANDROID__
#include "SDL.h"
#endif

#include "LawnApp.h"
#include "Resources.h"
#include "todlib/TodStringFile.h"
using namespace Sexy;

bool (*gAppCloseRequest)();     //[0x69E6A0]
bool (*gAppHasUsedCheatKeys)(); //[0x69E6A4]
SexyString (*gGetCurrentLevelName)();

// 0x44E8F0
int main(const int argc, char *argv[]) {
    TodStringListSetColors(gLawnStringFormats, gLawnStringFormatCount);
    gGetCurrentLevelName = LawnGetCurrentLevelName;
    gAppCloseRequest = LawnGetCloseRequest;
    gAppHasUsedCheatKeys = LawnHasUsedCheatKeys;
    gExtractResourcesByName = Sexy::ExtractResourcesByName;

#ifdef __ANDROID__
    SetAppDataFolder(SDL_AndroidGetExternalStoragePath());
#endif

    gLawnApp = new LawnApp();
    auto shouldChangeDir =
        (!Sexy::FileExists("properties/resources.xml") && Sexy::FileExists("../properties/resources.xml")) ||
        (!Sexy::FileExists("main.pak") && Sexy::FileExists("../main.pak"));

    gLawnApp->mChangeDirTo = shouldChangeDir ? ".." : ".";
    gLawnApp->DoParseCmdLine(argc, argv);
    gLawnApp->Init();
    gLawnApp->Start();
    gLawnApp->Shutdown();

    delete gLawnApp;

    return 0;
};
