#ifndef __DATASYNC_H__
#define __DATASYNC_H__

#include "framework/Common.h"

class DataReader {
protected:
    FILE *mFile;       //+0x4
    const char *mData; //+0x8
    uint32_t mDataLen; //+0xC
    uint32_t mDataPos; //+0x10
    bool mOwnData;     //+0x14

public:
    DataReader();
    virtual ~DataReader();

    bool OpenFile(const std::string &theFileName);
    void OpenMemory(const void *theData, uint32_t theDataLen, bool takeOwnership);
    void Close();
    void ReadBytes(void *theMem, uint32_t theNumBytes);
    void Rewind(uint32_t theNumBytes);
    uint32_t ReadLong();
    unsigned short ReadShort();
    unsigned char ReadByte();
    bool ReadBool();
    float ReadFloat();
    double ReadDouble();
    void ReadString(SexyString &theStr);
};

class DataReaderException : public std::exception {};

class DataWriter {
protected:
    FILE *mFile;        //+0x4
    char *mData;        //+0x8
    uint32_t mDataLen;  //+0xC
    uint32_t mCapacity; //+0x10

protected:
    void EnsureCapacity(uint32_t theNumBytes);

public:
    DataWriter();
    virtual ~DataWriter();

    bool OpenFile(const std::string &theFileName);
    void OpenMemory(uint32_t theReserveAmount = 0x20);
    void Close();
    inline bool WriteToFile(const std::string &theFileName);
    void WriteBytes(const void *theData, uint32_t theDataLen);
    void WriteLong(uint32_t theLong);
    void WriteShort(unsigned short theShort);
    void WriteByte(unsigned char theChar);
    void WriteBool(bool theBool);
    void WriteFloat(float theFloat);
    void WriteDouble(double theDouble);
    void WriteString(const SexyString &theStr);
    inline uint32_t GetPos();

    inline void SetLong(uint32_t, uint32_t) {}
    inline void SetShort(unsigned int, uint32_t) {}
    inline void SetByte(unsigned int, uint32_t) {}

    inline void *GetDataPtr() { return mData; }
    inline int GetDataLen() { return mDataLen; }
};

using PointerToIntMap = std::map<void *, int>;
using IntToPointerMap = std::map<int, void *>;

class DataSync {
protected:
    DataReader *mReader{};
    DataWriter *mWriter{};
    int mVersion{};
    PointerToIntMap mPointerToIntMap;
    IntToPointerMap mIntToPointerMap;
    std::vector<void **> mPointerSyncList;
    int mCurPointerIndex{};

protected:
    void ResetPointerTable();
    void Reset();

public:
    DataSync(DataReader &theReader);
    DataSync(DataWriter &theWriter);
    virtual ~DataSync();

    inline void SyncPointers() {}

    inline void SetReader(DataReader *theReader) { mReader = theReader; }
    inline void SetWriter(DataWriter *theWriter) { mWriter = theWriter; }
    inline DataReader *GetReader() { return mReader; }
    inline DataWriter *GetWriter() { return mWriter; }
    void SyncBytes(void *theData, uint32_t theDataLen) const;

    void SyncLong(char &theNum);
    void SyncLong(short &theNum);
    void SyncLong(int32_t &theNum);
    void SyncLong(unsigned char &theNum);
    void SyncLong(unsigned short &theNum);
    void SyncLong(uint32_t &theNum) const;

    void SyncSLong(char &theNum);
    void SyncSLong(short &theNum);
    void SyncSLong(int32_t &theNum);
    void SyncSLong(unsigned char &theNum);
    void SyncSLong(unsigned short &theNum);
    void SyncSLong(uint32_t &theNum);

    void SyncShort(char &theNum);
    void SyncShort(short &theNum);
    void SyncShort(int32_t &theNum);
    void SyncShort(unsigned char &theNum);
    void SyncShort(unsigned short &theNum);
    void SyncShort(uint32_t &theNum);

    void SyncSShort(char &theNum);
    void SyncSShort(short &theNum);
    void SyncSShort(int32_t &theNum);
    void SyncSShort(unsigned char &theNum);
    void SyncSShort(unsigned short &theNum);
    void SyncSShort(uint32_t &theNum);

    void SyncByte(char &theChar);
    void SyncByte(short &theChar);
    void SyncByte(int32_t &theChar);
    void SyncByte(unsigned char &theChar);
    void SyncByte(unsigned short &theChar);
    void SyncByte(uint32_t &theChar);

    void SyncSByte(char &theChar);
    void SyncSByte(short &theChar);
    void SyncSByte(int32_t &theChar);
    void SyncSByte(unsigned char &theChar);
    void SyncSByte(unsigned short &theChar);
    void SyncSByte(uint32_t &theChar);

    void SyncBool(bool &theBool);
    void SyncFloat(float &theFloat);
    void SyncDouble(double &theDouble);
    void SyncString(SexyString &theStr);

    template <typename Unit, typename R, typename P> void SyncTime(std::chrono::duration<R, P> &theDuration) {
        uint32_t aTimeMsec = 0;
        if (mReader) {
            aTimeMsec = mReader->ReadLong();
            theDuration = std::chrono::duration<R, P>(Unit(aTimeMsec));
        } else {
            aTimeMsec = std::chrono::duration_cast<Unit>(theDuration).count();
            mWriter->WriteLong(aTimeMsec);
        }
    }

    template <typename Unit, typename C, typename D> void SyncTime(std::chrono::time_point<C, D> &theTime) {
        uint32_t aTimeMsec = 0;
        if (mReader) {
            aTimeMsec = mReader->ReadLong();
            theTime = std::chrono::time_point<C, D>(Unit(aTimeMsec));
        } else {
            aTimeMsec = std::chrono::duration_cast<Unit>(theTime.time_since_epoch()).count();
            mWriter->WriteLong(aTimeMsec);
        }
    }

    inline void SyncPointer(void **) {}
    inline void RegisterPointer(void *) {}

    inline void SetVersion(int theVersion) { mVersion = theVersion; }
    inline int GetVersion() const { return mVersion; }
};

#endif
