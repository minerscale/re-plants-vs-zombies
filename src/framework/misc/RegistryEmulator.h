#ifndef REGISTRY_EMULATOR_H
#define REGISTRY_EMULATOR_H

/**********************************************************************************************/
/* Adapted from github.com/Alexpux/mingw-w64/blob/master/mingw-w64-tools/widl/include/winnt.h */
/**********************************************************************************************/

enum class SexyReg : uint32_t {
    NONE = 0,      /* no type */
    SZ = 1,        /* string type (ASCII) */
    EXPAND_SZ = 2, /* string, includes %ENVVAR% (expanded by caller) (ASCII) */
    BINARY = 3,    /* binary format, callerspecific */

    /* YES, REG_DWORD == REG_DWORD_LITTLE_ENDIAN */
    DWORD = 4,                    /* DWORD in little endian format */
    DWORD_LITTLE_ENDIAN = 4,      /* DWORD in little endian format */
    DWORD_BIG_ENDIAN = 5,         /* DWORD in big endian format  */
    LINK = 6,                     /* symbolic link (UNICODE) */
    MULTI_SZ = 7,                 /* multiple strings, delimited by \0, terminated by \0\0 (ASCII) */
    RESOURCE_LIST = 8,            /* resource list? huh? */
    FULL_RESOURCE_DESCRIPTOR = 9, /* full resource descriptor? huh? */
    RESOURCE_REQUIREMENTS_LIST = 10,
    QWORD = 11,              /* QWORD in little endian format */
    QWORD_LITTLE_ENDIAN = 11 /* QWORD in little endian format */
};

/***********************************************************************************************/

class RegistryEmulator {
public:
    explicit RegistryEmulator(const std::string &theFileName);

    RegistryEmulator() : RegistryEmulator("registry.dat") {}

    ~RegistryEmulator();
    bool Read(const std::string &theValueName, SexyReg &theType, std::vector<uint8_t> &theValue, uint32_t &theLength);
    void Write(const std::string &theValueName, SexyReg theType, const uint8_t *theValue, uint32_t theLength);
    bool Erase(const std::string &theValueName);
    size_t Flush() const;

private:
    const std::string mREG_FILENAME;
    std::vector<uint8_t> mRegVec;

    size_t FindKey(const std::string &theValueName);
    size_t GetNextKey(size_t theKeyIdx);
    void DeleteKey(size_t theKeyIdx);
};

#endif // REGISTRY_EMULATOR_H
