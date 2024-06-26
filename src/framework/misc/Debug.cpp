#include "Debug.h"
#include "Common.h"

// #include "CritSect.h"

#include "memmgr.h"

#include <cstdarg>

bool gInAssert = false;
// Seemingly unused
// extern bool gSexyDumpLeakedMem = false;

static FILE *gTraceFile = nullptr;
static int gTraceFileLen = 0;
static int gTraceFileNum = 1;

using namespace Sexy;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct SEXY_ALLOC_INFO {
    int size;
    char file[_MAX_PATH + 1];
    int line;
};

static bool gShowLeaks = false;
static bool gSexyAllocMapValid = false;

class SexyAllocMap : public std::map<void *, SEXY_ALLOC_INFO> {
public:
    SexyAllocMap() { gSexyAllocMapValid = true; }

    ~SexyAllocMap() {
        if (gShowLeaks) SexyDumpUnfreed();

        gSexyAllocMapValid = false;
    }
};

static SexyAllocMap gSexyAllocMap;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SexyTrace(const char *theStr) {
    if (gTraceFile == nullptr) {
        gTraceFileNum = (gTraceFileNum + 1) % 2;
        char aBuf[50];
        sprintf(aBuf, "trace%d.txt", gTraceFileNum + 1);
        gTraceFile = fopen(aBuf, "w");
        if (gTraceFile == nullptr) return;
    }

    fprintf(gTraceFile, "%s\n", theStr);
    fflush(gTraceFile);

    gTraceFileLen += strlen(theStr);
    if (gTraceFileLen > 100000) {
        fclose(gTraceFile);
        gTraceFile = nullptr;
        gTraceFileLen = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SexyMemAddTrack(void *addr, int asize, const char *fname, int lnum) {
    if (!gSexyAllocMapValid) return;

    // AutoCrit aCrit(gSexyAllocMap.mCrit);
    gShowLeaks = true;

    SEXY_ALLOC_INFO &info = gSexyAllocMap[addr];
    strncpy(info.file, fname, sizeof(info.file) - 1);
    info.line = lnum;
    info.size = asize;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SexyMemRemoveTrack(void *addr) {
    if (!gSexyAllocMapValid) return;

    // AutoCrit aCrit(gSexyAllocMap.mCrit);
    auto anItr = gSexyAllocMap.find(addr);
    if (anItr != gSexyAllocMap.end()) gSexyAllocMap.erase(anItr);
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SexyDumpUnfreed() {
    if (!gSexyAllocMapValid) return;

    // AutoCrit aCrit(gSexyAllocMap.mCrit);
    int totalSize = 0;
    char buf[8192];

#ifdef SEXY_DUMP_LEAKED_MEM
    char hex_dump[1024];
    char ascii_dump[1024];
    int count = 0;
    int index = 0;
#endif

    FILE *f = fopen("mem_leaks.txt", "wt");
    if (!f) return;

    time_t aTime = time(nullptr);
    sprintf(buf, "Memory Leak Report for %s\n", asctime(localtime(&aTime)));
    fprintf(f, "%s", buf);
    printf("\n");
    printf("%s", buf);
    for (auto i = gSexyAllocMap.begin(); i != gSexyAllocMap.end(); ++i) {
        sprintf(
            buf, "%s(%d) : Leak %d byte%s\n", i->second.file, i->second.line, i->second.size,
            i->second.size > 1 ? "s" : ""
        );
        printf("%s", buf);
        fprintf(f, "%s", buf);

#ifdef SEXY_DUMP_LEAKED_MEM
        unsigned char *data = (unsigned char *)i->first;

        for (index = 0; index < i->second.size; index++) {
            unsigned char _c = *data;

            if (count == 0) sprintf(hex_dump, "\t%02X ", _c);
            else sprintf(hex_dump, "%s%02X ", hex_dump, _c);

            if ((_c < 32) || (_c > 126)) _c = '.';

            if (count == 7) sprintf(ascii_dump, "%s%c ", ascii_dump, _c);
            else sprintf(ascii_dump, "%s%c", count == 0 ? "\t" : ascii_dump, _c);

            if (++count == 16) {
                count = 0;
                sprintf(buf, "%s\t%s\n", hex_dump, ascii_dump);
                fprintf(f, buf);

                memset((void *)hex_dump, 0, 1024);
                memset((void *)ascii_dump, 0, 1024);
            }

            data++;
        }

        if (count != 0) {
            fprintf(f, hex_dump);
            for (index = 0; index < 16 - count; index++)
                fprintf(f, "\t");

            fprintf(f, ascii_dump);

            for (index = 0; index < 16 - count; index++)
                fprintf(f, ".");
        }

        count = 0;
        fprintf(f, "\n\n");
        memset((void *)hex_dump, 0, 1024);
        memset((void *)ascii_dump, 0, 1024);

#endif // SEXY_DUMP_LEAKED_MEM

        totalSize += i->second.size;
    }

    sprintf(buf, "-----------------------------------------------------------\n");
    fprintf(f, "%s", buf);
    printf("%s", buf);
    sprintf(buf, "Total Unfreed: %d bytes (%dKB)\n\n", totalSize, totalSize / 1024);
    printf("%s", buf);
    fprintf(f, "%s", buf);
}
