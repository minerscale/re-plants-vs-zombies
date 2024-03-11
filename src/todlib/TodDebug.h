#ifndef __TODDEBUG_H__
#define __TODDEBUG_H__
#include <fmt/core.h>
#include <initializer_list>
#include <mutex>
#include <fstream>
#include <fmt/chrono.h>

struct TodLogger {
    TodLogger();
    ~TodLogger();

    bool Init();
    void LogString(const std::string &theMsg);

    constexpr static size_t kBufferSize = 256 * 1024;
    constexpr static size_t kMaxDelayCycle = 1000;

    std::ofstream mLogFile;
    std::atomic<size_t> mBufferOffset{0};
    std::mutex mFileMutex;
    std::vector<char> mBuffer;
};

void TodLogString(const std::string &theMsg);

template <class... Types> void TodLog(const fmt::format_string<Types...> theFormat, Types... theArgs) {
    const std::string aButter = fmt::format(theFormat, std::forward<Types>(theArgs)...);
    TodLogString(aButter.c_str());
}

void TodTraceMemory();

template <class... Types> void TodTraceAndLog(const fmt::format_string<Types...> theFormat, Types... theArgs) {
    auto aButter = fmt::format(theFormat, std::forward<Types>(theArgs)...);
    aButter += '\n';
    fmt::print("{}", aButter);
    TodLogString(aButter.c_str());
}

template <class... Types> void TodTraceWithoutSpamming(const fmt::format_string<Types...> theFormat, Types... theArgs) {
    static time_t gLastTraceTime = 0;
    const time_t aTime = time(nullptr);
    if (aTime < gLastTraceTime) {
        return;
    }
    gLastTraceTime = aTime;
    fmt::print(theFormat, std::forward<Types>(theArgs)...);
}

void TodHesitationTrace(...);

template <class... Types>
void TodAssertFailed(
    const char *theCondition, const char *theFile, int theLine, fmt::format_string<Types...> theFmt, Types &&...Args
) {
    const std::string aFormattedMsg = fmt::format(theFmt, std::forward<Types>(Args)...);
    if (*theCondition != '\0') {
        fmt::println("\n{}({})\nassertion failed: '{}'\n{}", theFile, theLine, theCondition, aFormattedMsg);
    } else {
        fmt::println("\n{}({})\nassertion failed: {}", theFile, theLine, aFormattedMsg);
    }
    exit(0);
}

template <class... Types>
void TodAssertFailed(const char *theCondition, const char *theFile, int theLine, Types &&...Args) {
    if (*theCondition != '\0') {
        fmt::println("\n{}({})\nassertion failed: '{}'", theFile, theLine, theCondition);
    } else {
        fmt::println("\n{}({})\nassertion failed.", theFile, theLine);
    }
    exit(0);
}

void TodErrorMessageBox(const char *theMessage, const char *theTitle);
void *TodMalloc(int theSize);
void TodFree(void *theBlock);
void TodAssertInitForApp();

template <class T> inline bool TodAssertContains(std::initializer_list<T> List, T Val) {
    bool result = false;
    for (auto &item : List) {
        if (item == Val) {
            result = true;
            break;
        }
    }
    return result;
}

template <int DurationLimit = 100> class TodHesitationBracket {
public:
    std::string mMsg;
    std::chrono::time_point<std::chrono::high_resolution_clock> mStartTime;

    template <class... Types>
    explicit TodHesitationBracket(const fmt::format_string<Types...> theFormat, Types... theArgs) {
        mMsg = fmt::format(theFormat, std::forward<Types>(theArgs)...);
        mStartTime = std::chrono::high_resolution_clock::now();
    }

    ~TodHesitationBracket() { EndBracket(); }

    void EndBracket() const {
        const auto aEndTime = std::chrono::high_resolution_clock::now();
        const auto aDuration = std::chrono::duration_cast<std::chrono::milliseconds>(aEndTime - mStartTime).count();
        if (aDuration > DurationLimit) {
            TodTraceAndLog("{} took {}ms", mMsg, aDuration);
        }
    }
};

#if __has_builtin(__builtin_debugtrap)
#define DBG_BREAK() __builtin_debugtrap()
#else
#define DBG_BREAK() __debugbreak()
#endif

#ifdef _DEBUG
#define TOD_ASSERT(condition, ...)                                                                                     \
    do {                                                                                                               \
        if (!bool(condition)) {                                                                                        \
            TodAssertFailed("" #condition, __FILE__, __LINE__, ##__VA_ARGS__);                                         \
            DBG_BREAK();                                                                                               \
            TodTraceMemory();                                                                                          \
        }                                                                                                              \
    } while (0)
#else
#define TOD_ASSERT(condition, ...)
#endif

#endif
