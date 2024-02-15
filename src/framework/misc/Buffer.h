#ifndef __BUFFER_H__
#define __BUFFER_H__

#include "framework/Common.h"
#include <string>

namespace Sexy {
using ByteVector = std::vector<uint8_t>;

class Buffer {
public:
    ByteVector mData;
    int mDataBitSize;
    mutable int mReadBitPos;
    mutable int mWriteBitPos;

public:
    Buffer();
    virtual ~Buffer();

    void SeekFront() const;
    void Clear();

    void FromWebString(const std::string &theString);
    void WriteByte(uint8_t theByte);
    void WriteNumBits(int theNum, int theBits);
    static int GetBitsRequired(int theNum, bool isSigned);
    void WriteBoolean(bool theBool);
    void WriteShort(short theShort);
    void WriteLong(int32_t theLong);
    void WriteString(const std::string &theString);
    void WriteUTF8String(const std::wstring &theString);
    void WriteLine(const std::string &theString);
    void WriteBuffer(const ByteVector &theBuffer);
    void WriteBytes(const uint8_t *theByte, int theCount);
    void SetData(const ByteVector &theBuffer);
    void SetData(uint8_t *thePtr, int theCount);

    std::string ToWebString() const;
    std::wstring UTF8ToWideString() const;
    uint8_t ReadByte() const;
    int ReadNumBits(int theBits, bool isSigned) const;
    bool ReadBoolean() const;
    short ReadShort() const;
    int32_t ReadLong() const;
    std::string ReadString() const;
    std::wstring ReadUTF8String() const;
    std::string ReadLine() const;
    void ReadBytes(uint8_t *theData, int theLen) const;
    void ReadBuffer(ByteVector *theByteVector) const;

    const uint8_t *GetDataPtr() const;
    int GetDataLen() const;
    int GetDataLenBits() const;
    uint32_t GetCRC32(uint32_t theSeed = 0) const;

    bool AtEnd() const;
    bool PastEnd() const;
};
} // namespace Sexy

#endif //__BUFFER_H__
