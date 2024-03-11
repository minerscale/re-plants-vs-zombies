#include "TodDebug.h"
#include "TodCommon.h"
#include "framework/SexyAppBase.h"
#include "misc/Debug.h"
#include <cstdarg>
#include <fstream>

using namespace Sexy;

TodLogger* gTodLogger = nullptr;

TodLogger::TodLogger() {
    mBuffer.resize(kBufferSize);
    gTodLogger = this;
}

TodLogger::~TodLogger() {
    TodLog("Ended at {:%Y-%m-%d %H:%M:%S}\n", std::chrono::system_clock::now());
    gTodLogger = nullptr;

    mLogFile.write(mBuffer.data(), mBufferOffset);
    mLogFile.flush();
    mLogFile.close();
}

bool TodLogger::Init() {
    MkDir(GetAppDataFolder() + "userdata");
    const auto aStart = std::chrono::system_clock::now();
    const std::string aRelativeUserPath = GetAppDataFolder() + "userdata/";
    const auto aLogFileName = fmt::format("{}log_{}.txt", GetFullPath(aRelativeUserPath), std::chrono::system_clock::to_time_t(aStart));

    mLogFile.open(aLogFileName, std::ios::app);
    TodLog("Started at {}\n", aStart);

    return true;
}

void TodLogger::LogString(const std::string &theMsg) {
    if (theMsg.empty()) {
        return;
    }

    if (theMsg.length() > kBufferSize / 2) {
        mFileMutex.lock();
        mLogFile.write(mBuffer.data(), mBufferOffset);
        mLogFile.write(theMsg.data(), theMsg.size());
        mBufferOffset = 0;
        mFileMutex.unlock();
    }

    for (size_t aRetry = 0; aRetry < kMaxDelayCycle; aRetry++) {
        size_t aOffset = mBufferOffset.fetch_add(theMsg.size());
        if (aOffset >= kBufferSize) continue;
        if (aOffset + theMsg.size() > kBufferSize) {
            mFileMutex.lock();
            mLogFile.write(mBuffer.data(), aOffset);
            mBufferOffset = 0;
            mFileMutex.unlock();
        } else {
            std::ranges::copy(theMsg, mBuffer.begin() + aOffset);
            break;
        }
    }
}

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

void TodLogString(const std::string &theMsg) {
    if (gTodLogger != nullptr) {
        gTodLogger->LogString(theMsg);
    }
}

void TodHesitationTrace(...) {}

void TodAssertInitForApp()
{
    if (gTodLogger != nullptr) {
        gTodLogger->Init();
    }
}