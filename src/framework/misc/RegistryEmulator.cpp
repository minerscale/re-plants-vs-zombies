#include "RegistryEmulator.h"

#include <cstdint>
#include <fstream>
#include <filesystem>

RegistryEmulator::RegistryEmulator(const std::string& theFileName) : mREG_FILENAME{ theFileName } {
    if (!std::filesystem::exists(mREG_FILENAME)) return; // We're done here

    std::ifstream aRegFile(mREG_FILENAME, std::ios::binary);
    if (!aRegFile) throw std::runtime_error("Cannot open registry file!");

    aRegFile.seekg(0, std::ios::end);
    aRegFile.seekg(0, std::ios::beg);

    auto aHeader = RegistryHeader{};
    aRegFile.read(reinterpret_cast<char*>(&aHeader), sizeof(aHeader));
    if (aHeader.mMagic != RegistryMagic()) return;

    for (size_t idx = 0; idx < aHeader.mNumEntries; ++idx) {
        auto aItemEntry = RegistryEntry{};
        aRegFile.read(reinterpret_cast<char*>(&aItemEntry), sizeof(aItemEntry));

        // Read value name (assuming it's a null-terminated string)
        std::string aName(aItemEntry.mValueNameLength, '\0');
        aRegFile.read(aName.data(), aItemEntry.mValueNameLength);

        // Read value content
        std::vector<uint8_t> aContent(aItemEntry.mValueLength);
        aRegFile.read(reinterpret_cast<char*>(aContent.data()), aItemEntry.mValueLength);

        // Store in mRegMap
        mRegMap[aName] = std::make_tuple(static_cast<SexyReg>(aItemEntry.mValueType), aContent);
    }
}

RegistryEmulator::~RegistryEmulator() { Flush(); }


size_t RegistryEmulator::Flush() const {
    std::ofstream outFile(mREG_FILENAME, std::ios::binary);
    if (!outFile) return 0;

    size_t aOffset = 0;

    const RegistryHeader aHeader{
        .mMagic = RegistryMagic(),
        .mNumEntries = mRegMap.size()
    };

    outFile.write(reinterpret_cast<const char*>(&aHeader), sizeof(aHeader));

    // Write each entry
    for (const auto &[aName, aTuple] : mRegMap) {
        const auto [aValueType, aContent] = aTuple;

        RegistryEntry aItemEntry{
            .mValueNameLength = aName.size(),
            .mValueType = static_cast<uint64_t>(aValueType),
            .mValueLength = aContent.size(),
        };

        outFile.write(reinterpret_cast<const char*>(&aItemEntry), sizeof(aItemEntry));
        outFile.write(aName.c_str(), aName.size());
        outFile.write(reinterpret_cast<const char*>(aContent.data()), aContent.size());
    }

    // ！TODO Calculate size
    return aOffset;
}


bool RegistryEmulator::Read(
    const std::string &theValueName, SexyReg &theType, std::vector<uint8_t> &theValue, uint32_t &theLength
) {
    const auto aKeyIter = mRegMap.find(theValueName);
    if (aKeyIter == mRegMap.end()) return false;

    theType = std::get<0>(aKeyIter->second);
    theValue = std::get<1>(aKeyIter->second);
    theLength = theValue.size();
    return true;
}


bool RegistryEmulator::Erase(const std::string &theValueName) {
    const auto aKeyIter = mRegMap.find(theValueName);
    if (aKeyIter == mRegMap.end()) return false;
    mRegMap.erase(aKeyIter);
    return true;
}

void RegistryEmulator::Write(
    const std::string &theValueName, SexyReg theType, const uint8_t *theValue, uint32_t theLength
) {
    mRegMap[theValueName] = std::make_tuple(theType, std::vector<uint8_t>(theValue, theValue + theLength));
}
