#ifndef REGISTRY_EMULATOR_H
#define REGISTRY_EMULATOR_H

#include "framework/Common.h"

class RegistryEmulator {
public:
    explicit RegistryEmulator(const std::string &theFileName);

    RegistryEmulator() : RegistryEmulator("registry.dat") {}

    ~RegistryEmulator();
    bool Read(const std::string &theValueName, uint32_t &theType, std::vector<uint8_t> &theValue, uint32_t &theLength);
    void Write(const std::string &theValueName, uint32_t theType, const uint8_t *theValue, uint32_t theLength);
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
