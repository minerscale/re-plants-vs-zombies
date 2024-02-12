#ifndef __SAVEGAMECONTEXT_H__
#define __SAVEGAMECONTEXT_H__

#include "ConstEnums.h"
#include "framework/misc/Buffer.h"
#include "todlib/TodList.h"
#include <string>

class Board;
class Trail;
enum GameMode;
class Reanimation;
class TodParticleSystem;
class TodParticleEmitter;
class ReanimatorDefinition;
class TodParticleDefinition;
class TrailDefinition;

namespace Sexy {
class Image;
}

using namespace Sexy;

struct SaveFileHeader {
    unsigned int mMagicNumber;
    unsigned int mBuildVersion;
    unsigned int mBuildDate;
};

class SaveGameContext {
public:
    Buffer mBuffer; //+0x0
    bool mFailed;   //+0x20
    bool mReading;  //+0x21

public:
    inline int ByteLeftToRead() { return (mBuffer.mDataBitSize - mBuffer.mReadBitPos + 7) / 8; }
    void SyncBytes(void *theDest, int theReadSize);
    void SyncInt(int &theInt);
    inline void SyncSizeT(size_t &theInt) { SyncBytes(&theInt, sizeof(size_t)); }
    ReanimationType SyncReanimationDef(ReanimatorDefinition *&theDefinition);
    void SyncParticleDef(TodParticleDefinition *&theDefinition);
    void SyncTrailDef(TrailDefinition *&theDefinition);
    void SyncImage(Image *&theImage);
};

void SyncDataIDList(TodList<unsigned int> *theDataIDList, SaveGameContext &theContext, TodAllocator *theAllocator);
void SyncParticleEmitter(
    TodParticleSystem *theParticleSystem, TodParticleEmitter *theParticleEmitter, SaveGameContext &theContext
);
void SyncParticleSystem(const Board *theBoard, TodParticleSystem *theParticleSystem, SaveGameContext &theContext);
void SyncReanimation(const Board *theBoard, Reanimation *theReanimation, SaveGameContext &theContext);
void SyncTrail(const Board *theBoard, Trail *theTrail, SaveGameContext &theContext);
void SyncBoard(SaveGameContext &theContext, Board *theBoard);
void FixBoardAfterLoad(Board *theBoard);
bool LawnLoadGame(Board *theBoard, const std::string &theFilePath);
bool LawnSaveGame(Board *theBoard, const std::string &theFilePath);

#endif
