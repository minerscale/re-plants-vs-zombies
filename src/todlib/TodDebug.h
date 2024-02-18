#ifndef __TODDEBUG_H__
#define __TODDEBUG_H__
#include <fmt/core.h>
#include <initializer_list>

// #define NOMINMAX 1
// #include <windows.h>

class TodHesitationBracket {
public:
    char mMessage[256];
    int mBracketStartTime;

public:
    explicit TodHesitationBracket(const char * /*theFormat*/, ...) : mMessage{}, mBracketStartTime(0) {}

    ~TodHesitationBracket() {}

    static inline void EndBracket() {}
};

void TodLogString(const char *theMsg);

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
void TodTraceWithoutSpamming(const char *theFormat, ...);
void TodHesitationTrace(...);
// void				TodReportError(LPEXCEPTION_POINTERS exceptioninfo, const char* theMessage);

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
// long __stdcall	TodUnhandledExceptionFilter(LPEXCEPTION_POINTERS exceptioninfo);

/*inline*/ void *TodMalloc(int theSize);
/*inline*/ void TodFree(void *theBlock);
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
