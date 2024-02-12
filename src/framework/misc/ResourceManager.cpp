#include "ResourceManager.h"
#include "Common.h"
#include "XMLParser.h"
#include "sound/SoundManager.h"
#include <memory>
#include <stdexcept>
// #include "graphics/DDImage.h"
// #include "graphics/D3DInterface.h"
#include "graphics/ImageFont.h"
#include "imagelib/ImageLib.h"

// #define SEXY_PERF_ENABLED
#include "PerfTimer.h"

using namespace Sexy;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::ImageRes::DeleteResource() {
    // delete mImage;
    // mImage.Release();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::SoundRes::DeleteResource() {
    if (mSoundId >= 0) gSexyAppBase->mSoundManager->ReleaseSound(mSoundId);

    mSoundId = -1;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void ResourceManager::FontRes::DeleteResource() {
    delete mFont;
    mFont = nullptr;

    delete mImage;
    mImage = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ResourceManager::ResourceManager(SexyAppBase *theApp) {
    mApp = theApp;
    mHasFailed = false;
    mXMLParser = nullptr;

    mAllowMissingProgramResources = false;
    mAllowAlreadyDefinedResources = false;
    mCurResGroupList = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ResourceManager::~ResourceManager() {
    DeleteMap(mImageMap);
    DeleteMap(mSoundMap);
    DeleteMap(mFontMap);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::IsGroupLoaded(const std::string &theGroup) { return mLoadedGroups.contains(theGroup); }

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::DeleteMap(ResMap &theMap) {
    for (auto anItr = theMap.begin(); anItr != theMap.end(); ++anItr) {
        anItr->second->DeleteResource();
        delete anItr->second;
    }

    theMap.clear();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::DeleteResources(ResMap &theMap, const std::string &theGroup) {
    for (auto anItr = theMap.begin(); anItr != theMap.end(); ++anItr) {
        if (theGroup.empty() || anItr->second->mResGroup == theGroup) anItr->second->DeleteResource();
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::DeleteResources(const std::string &theGroup) {
    DeleteResources(mImageMap, theGroup);
    DeleteResources(mSoundMap, theGroup);
    DeleteResources(mFontMap, theGroup);
    mLoadedGroups.erase(theGroup);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::DeleteExtraImageBuffers(const std::string & /*theGroup*/) {
    unreachable();
    /* TODO
    for (ResMap::iterator anItr = mImageMap.begin(); anItr != mImageMap.end(); ++anItr)
    {
        if (theGroup.empty() || anItr->second->mResGroup==theGroup)
        {
            ImageRes *aRes = (ImageRes*)anItr->second;

            MemoryImage *anImage = (MemoryImage*)aRes->mImage;
            if (anImage != NULL)
                anImage->DeleteExtraBuffers();
        }
    }*/
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
std::string ResourceManager::GetErrorText() { return mError; }

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::HadError() const { return mHasFailed; }

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::Fail(const std::string &theErrorText) {
    if (!mHasFailed) {
        mHasFailed = true;
        if (mXMLParser == nullptr) {
            mError = theErrorText;
            return false;
        }

        int aLineNum = mXMLParser->GetCurrentLineNum();

        char aLineNumStr[16];
        sprintf(aLineNumStr, "%d", aLineNum);

        mError = theErrorText;

        if (aLineNum > 0) mError += std::string(" on Line ") + aLineNumStr;

        if (mXMLParser->GetFileName().length() > 0) mError += " in File '" + mXMLParser->GetFileName() + "'";
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::ParseCommonResource(XMLElement &theElement, BaseRes *theRes, ResMap &theMap) {
    mHadAlreadyDefinedError = false;

    const SexyString &aPath = theElement.mAttributes[_S("path")];
    if (aPath.empty()) return Fail("No path specified.");

    theRes->mXMLAttributes = theElement.mAttributes;
    theRes->mFromProgram = false;
    if (aPath[0] == _S('!')) {
        theRes->mPath = SexyStringToStringFast(aPath);
        if (aPath == _S("!program")) theRes->mFromProgram = true;
    } else theRes->mPath = mDefaultPath + SexyStringToStringFast(aPath);

    std::string anId;
    auto anItr = theElement.mAttributes.find(_S("id"));
    if (anItr == theElement.mAttributes.end()) anId = mDefaultIdPrefix + GetFileName(theRes->mPath, true);
    else anId = mDefaultIdPrefix + SexyStringToStringFast(anItr->second);

    theRes->mResGroup = mCurResGroup;
    theRes->mId = anId;

    std::pair<ResMap::iterator, bool> aRet = theMap.insert(ResMap::value_type(anId, theRes));
    if (!aRet.second) {
        mHadAlreadyDefinedError = true;
        return Fail("Resource already defined.");
    }

    mCurResGroupList->push_back(theRes);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::ParseSoundResource(XMLElement &theElement) {
    auto aRes = new SoundRes;
    aRes->mSoundId = -1;
    aRes->mVolume = -1;
    aRes->mPanning = 0;

    if (!ParseCommonResource(theElement, aRes, mSoundMap)) {
        if (mHadAlreadyDefinedError && mAllowAlreadyDefinedResources) {
            mError = "";
            mHasFailed = false;
            SoundRes *oldRes = aRes;
            aRes = static_cast<SoundRes *>(mSoundMap[oldRes->mId]);
            aRes->mPath = oldRes->mPath;
            aRes->mXMLAttributes = oldRes->mXMLAttributes;
            delete oldRes;
        } else {
            delete aRes;
            return false;
        }
    }

    XMLParamMap::iterator anItr = theElement.mAttributes.find(_S("volume"));
    if (anItr != theElement.mAttributes.end()) sexysscanf(anItr->second.c_str(), _S("%lf"), &aRes->mVolume);

    anItr = theElement.mAttributes.find(_S("pan"));
    if (anItr != theElement.mAttributes.end()) sexysscanf(anItr->second.c_str(), _S("%d"), &aRes->mPanning);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static void ReadIntVector(const SexyString &theVal, std::vector<int> &theVector) {
    theVector.clear();

    std::string::size_type aPos = 0;
    while (true) {
        theVector.push_back(sexyatoi(theVal.c_str() + aPos));
        aPos = theVal.find_first_of(_S(','), aPos);
        if (aPos == std::string::npos) break;

        aPos++;
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::ParseImageResource(XMLElement &theElement) {
    auto aRes = new ImageRes;
    if (!ParseCommonResource(theElement, aRes, mImageMap)) {
        if (mHadAlreadyDefinedError && mAllowAlreadyDefinedResources) {
            mError = "";
            mHasFailed = false;
            ImageRes *oldRes = aRes;
            aRes = static_cast<ImageRes *>(mImageMap[oldRes->mId]);
            aRes->mPath = oldRes->mPath;
            aRes->mXMLAttributes = oldRes->mXMLAttributes;
            delete oldRes;
        } else {
            delete aRes;
            return false;
        }
    }

    aRes->mPalletize = !theElement.mAttributes.contains(_S("nopal"));
    aRes->mA4R4G4B4 = theElement.mAttributes.contains(_S("a4r4g4b4"));
    aRes->mDDSurface = theElement.mAttributes.contains(_S("ddsurface"));
    aRes->mPurgeBits =
        (theElement.mAttributes.contains(_S("nobits"))) || (theElement.mAttributes.contains(_S("nobits3d")));
    aRes->mA8R8G8B8 = theElement.mAttributes.contains(_S("a8r8g8b8"));
    aRes->mMinimizeSubdivisions = theElement.mAttributes.contains(_S("minsubdivide"));
    aRes->mAutoFindAlpha = !theElement.mAttributes.contains(_S("noalpha"));

    XMLParamMap::iterator anItr = theElement.mAttributes.find(_S("alphaimage"));
    if (anItr != theElement.mAttributes.end()) aRes->mAlphaImage = mDefaultPath + SexyStringToStringFast(anItr->second);

    aRes->mAlphaColor = 0xFFFFFF;
    anItr = theElement.mAttributes.find(_S("alphacolor"));
    if (anItr != theElement.mAttributes.end()) sexysscanf(anItr->second.c_str(), _S("%x"), &aRes->mAlphaColor);

    anItr = theElement.mAttributes.find(_S("variant"));
    if (anItr != theElement.mAttributes.end()) aRes->mVariant = SexyStringToStringFast(anItr->second);

    anItr = theElement.mAttributes.find(_S("alphagrid"));
    if (anItr != theElement.mAttributes.end())
        aRes->mAlphaGridImage = mDefaultPath + SexyStringToStringFast(anItr->second);

    anItr = theElement.mAttributes.find(_S("rows"));
    if (anItr != theElement.mAttributes.end()) aRes->mRows = sexyatoi(anItr->second.c_str());
    else aRes->mRows = 1;

    anItr = theElement.mAttributes.find(_S("cols"));
    if (anItr != theElement.mAttributes.end()) aRes->mCols = sexyatoi(anItr->second.c_str());
    else aRes->mCols = 1;

    anItr = theElement.mAttributes.find(_S("anim"));
    AnimType anAnimType = AnimType_None;
    if (anItr != theElement.mAttributes.end()) {
        const SexyChar *aType = anItr->second.c_str();

        if (sexystricmp(aType, _S("none")) == 0) anAnimType = AnimType_None;
        else if (sexystricmp(aType, _S("once")) == 0) anAnimType = AnimType_Once;
        else if (sexystricmp(aType, _S("loop")) == 0) anAnimType = AnimType_Loop;
        else if (sexystricmp(aType, _S("pingpong")) == 0) anAnimType = AnimType_PingPong;
        else {
            Fail("Invalid animation type.");
            return false;
        }
    }
    aRes->mAnimInfo.mAnimType = anAnimType;
    if (anAnimType != AnimType_None) {
        int aNumCels = std::max(aRes->mRows, aRes->mCols);
        int aBeginDelay = 0, anEndDelay = 0;

        anItr = theElement.mAttributes.find(_S("framedelay"));
        if (anItr != theElement.mAttributes.end()) aRes->mAnimInfo.mFrameDelay = sexyatoi(anItr->second.c_str());

        anItr = theElement.mAttributes.find(_S("begindelay"));
        if (anItr != theElement.mAttributes.end()) aBeginDelay = sexyatoi(anItr->second.c_str());

        anItr = theElement.mAttributes.find(_S("enddelay"));
        if (anItr != theElement.mAttributes.end()) anEndDelay = sexyatoi(anItr->second.c_str());

        anItr = theElement.mAttributes.find(_S("perframedelay"));
        if (anItr != theElement.mAttributes.end()) ReadIntVector(anItr->second, aRes->mAnimInfo.mPerFrameDelay);

        anItr = theElement.mAttributes.find(_S("framemap"));
        if (anItr != theElement.mAttributes.end()) ReadIntVector(anItr->second, aRes->mAnimInfo.mFrameMap);

        aRes->mAnimInfo.Compute(aNumCels, aBeginDelay, anEndDelay);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::ParseFontResource(XMLElement &theElement) {
    auto aRes = new FontRes;
    aRes->mFont = nullptr;
    aRes->mImage = nullptr;

    if (!ParseCommonResource(theElement, aRes, mFontMap)) {
        if (mHadAlreadyDefinedError && mAllowAlreadyDefinedResources) {
            mError = "";
            mHasFailed = false;
            FontRes *oldRes = aRes;
            aRes = static_cast<FontRes *>(mFontMap[oldRes->mId]);
            aRes->mPath = oldRes->mPath;
            aRes->mXMLAttributes = oldRes->mXMLAttributes;
            delete oldRes;
        } else {
            delete aRes;
            return false;
        }
    }

    XMLParamMap::iterator anItr = theElement.mAttributes.find(_S("image"));
    if (anItr != theElement.mAttributes.end()) aRes->mImagePath = SexyStringToStringFast(anItr->second);

    anItr = theElement.mAttributes.find(_S("tags"));
    if (anItr != theElement.mAttributes.end()) aRes->mTags = SexyStringToStringFast(anItr->second);

    if (strncmp(aRes->mPath.c_str(), "!sys:", 5) == 0) {
        aRes->mSysFont = true;
        aRes->mPath = aRes->mPath.substr(5);

        anItr = theElement.mAttributes.find(_S("size"));
        if (anItr == theElement.mAttributes.end()) return Fail("SysFont needs point size");

        aRes->mSize = sexyatoi(anItr->second.c_str());
        if (aRes->mSize <= 0) return Fail("SysFont needs point size");

        aRes->mBold = theElement.mAttributes.contains(_S("bold"));
        aRes->mItalic = theElement.mAttributes.contains(_S("italic"));
        aRes->mShadow = theElement.mAttributes.contains(_S("shadow"));
        aRes->mUnderline = theElement.mAttributes.contains(_S("underline"));
    } else aRes->mSysFont = false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::ParseSetDefaults(XMLElement &theElement) {
    XMLParamMap::iterator anItr = theElement.mAttributes.find(_S("path"));
    if (anItr != theElement.mAttributes.end())
        mDefaultPath = RemoveTrailingSlash(SexyStringToStringFast(anItr->second)) + '/';

    anItr = theElement.mAttributes.find(_S("idprefix"));
    if (anItr != theElement.mAttributes.end())
        mDefaultIdPrefix = RemoveTrailingSlash(SexyStringToStringFast(anItr->second));

    return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::ParseResources() {
    for (;;) {
        XMLElement aXMLElement;
        if (!mXMLParser->NextElement(&aXMLElement)) return false;

        if (aXMLElement.mType == XMLElement::TYPE_START) {
            if (aXMLElement.mValue == _S("Image")) {
                if (!ParseImageResource(aXMLElement)) return false;

                if (!mXMLParser->NextElement(&aXMLElement)) return false;

                if (aXMLElement.mType != XMLElement::TYPE_END) return Fail("Unexpected element found.");
            } else if (aXMLElement.mValue == _S("Sound")) {
                if (!ParseSoundResource(aXMLElement)) return false;

                if (!mXMLParser->NextElement(&aXMLElement)) return false;

                if (aXMLElement.mType != XMLElement::TYPE_END) return Fail("Unexpected element found.");
            } else if (aXMLElement.mValue == _S("Font")) {
                if (!ParseFontResource(aXMLElement)) return false;

                if (!mXMLParser->NextElement(&aXMLElement)) return false;

                if (aXMLElement.mType != XMLElement::TYPE_END) return Fail("Unexpected element found.");
            } else if (aXMLElement.mValue == _S("SetDefaults")) {
                if (!ParseSetDefaults(aXMLElement)) return false;

                if (!mXMLParser->NextElement(&aXMLElement)) return false;

                if (aXMLElement.mType != XMLElement::TYPE_END) return Fail("Unexpected element found.");
            } else {
                Fail("Invalid Section '" + SexyStringToStringFast(aXMLElement.mValue) + "'");
                return false;
            }
        } else if (aXMLElement.mType == XMLElement::TYPE_ELEMENT) {
            Fail("Element Not Expected '" + SexyStringToStringFast(aXMLElement.mValue) + "'");
            return false;
        } else if (aXMLElement.mType == XMLElement::TYPE_END) {
            return true;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::DoParseResources() {
    if (!mXMLParser->HasFailed()) {
        for (;;) {
            XMLElement aXMLElement;
            if (!mXMLParser->NextElement(&aXMLElement)) break;

            if (aXMLElement.mType == XMLElement::TYPE_START) {
                if (aXMLElement.mValue == _S("Resources")) {
                    mCurResGroup = SexyStringToStringFast(aXMLElement.mAttributes[_S("id")]);
                    mCurResGroupList = &mResGroupMap[mCurResGroup];

                    if (mCurResGroup.empty()) {
                        Fail("No id specified.");
                        break;
                    }

                    if (!ParseResources()) break;
                } else {
                    Fail("Invalid Section '" + SexyStringToStringFast(aXMLElement.mValue) + "'");
                    break;
                }
            } else if (aXMLElement.mType == XMLElement::TYPE_ELEMENT) {
                Fail("Element Not Expected '" + SexyStringToStringFast(aXMLElement.mValue) + "'");
                break;
            }
        }
    }

    if (mXMLParser->HasFailed()) Fail(SexyStringToStringFast(mXMLParser->GetErrorText()));

    delete mXMLParser;
    mXMLParser = nullptr;

    return !mHasFailed;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::ParseResourcesFile(const std::string &theFilename) {
    mXMLParser = new XMLParser();
    if (!mXMLParser->OpenFile(theFilename)) Fail("Resource file not found: " + theFilename);

    XMLElement aXMLElement;
    while (!mXMLParser->HasFailed()) {
        if (!mXMLParser->NextElement(&aXMLElement)) Fail(SexyStringToStringFast(mXMLParser->GetErrorText()));

        if (aXMLElement.mType == XMLElement::TYPE_START) {
            if (aXMLElement.mValue != _S("ResourceManifest")) break;
            else return DoParseResources();
        }
    }

    Fail("Expecting ResourceManifest tag");

    return DoParseResources();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::ReparseResourcesFile(const std::string &theFilename) {
    bool oldDefined = mAllowAlreadyDefinedResources;
    mAllowAlreadyDefinedResources = true;

    bool aResult = ParseResourcesFile(theFilename);

    mAllowAlreadyDefinedResources = oldDefined;

    return aResult;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::DoLoadImage(ImageRes *theRes) {
    // bool lookForAlpha = theRes->mAlphaImage.empty() && theRes->mAlphaGridImage.empty() && theRes->mAutoFindAlpha; //
    // unused

    SEXY_PERF_BEGIN("ResourceManager:GetImage");

    ImageLib::gAlphaComposeColor = theRes->mAlphaColor;
    if (theRes->mVariant != "")
        throw std::runtime_error(
            "I mistakenly depricated image variants to simplify the rendering code. Apparently it was used."
        );
    Image *anImage = gSexyAppBase->GetSharedImage(*theRes);
    ImageLib::gAlphaComposeColor = 0xFFFFFF;

    if (anImage == nullptr) return Fail(StrFormat("Failed to load image: %s", theRes->mPath.c_str()));

    theRes->mImage = anImage;

    if (theRes->mAnimInfo.mAnimType != AnimType_None) anImage->mAnimInfo = new AnimInfo(theRes->mAnimInfo);

    anImage->mNumRows = theRes->mRows;
    anImage->mNumCols = theRes->mCols;

    ResourceLoadedHook(theRes);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::DeleteImage(const std::string &theName) { ReplaceImage(theName, nullptr); }

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
Image *ResourceManager::LoadImage(const std::string &theName) {
    auto anItr = mImageMap.find(theName);
    if (anItr == mImageMap.end()) return nullptr;

    auto aRes = static_cast<ImageRes *>(anItr->second);

    if (aRes->mImage != nullptr) return aRes->mImage;

    if (aRes->mFromProgram) return nullptr;

    if (!DoLoadImage(aRes)) return nullptr;

    return aRes->mImage;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::DoLoadSound(SoundRes *theRes) {
    SoundRes *aRes = theRes;

    SEXY_PERF_BEGIN("ResourceManager:LoadSound");
    int aSoundId = mApp->mSoundManager->GetFreeSoundId();
    if (aSoundId < 0) return Fail("Out of free sound ids");

    if (!mApp->mSoundManager->LoadSound(aSoundId, aRes->mPath))
        return Fail(StrFormat("Failed to load sound: %s", aRes->mPath.c_str()));
    SEXY_PERF_END("ResourceManager:LoadSound");

    if (aRes->mVolume >= 0) mApp->mSoundManager->SetBaseVolume(aSoundId, aRes->mVolume);

    if (aRes->mPanning != 0) mApp->mSoundManager->SetBasePan(aSoundId, aRes->mPanning);

    aRes->mSoundId = aSoundId;

    ResourceLoadedHook(theRes);
    return true;
}

#include <todlib/TodCommon.h>
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::DoLoadFont(FontRes *theRes) {
    _Font *aFont = nullptr;

    SEXY_PERF_BEGIN("ResourceManager:DoLoadFont");

    if (theRes->mSysFont) {
        unreachable();
        /* TODO
        bool bold = theRes->mBold, simulateBold = false;
        if (Sexy::CheckFor98Mill())
        {
            simulateBold = bold;
            bold = false;
        }
        aFont = new SysFont(theRes->mPath,theRes->mSize,bold,theRes->mItalic,theRes->mUnderline);
        SysFont* aSysFont = (SysFont*)aFont;
        aSysFont->mDrawShadow = theRes->mShadow;
        aSysFont->mSimulateBold = simulateBold;*/
    } else if (theRes->mImagePath.empty()) {
        if (strncmp(theRes->mPath.c_str(), "!ref:", 5) == 0) {
            std::string aRefName = theRes->mPath.substr(5);
            _Font *aRefFont = GetFont(aRefName);
            if (aRefFont == nullptr) return Fail("Ref font not found: " + aRefName);

            aFont = aRefFont->Duplicate();
        } else aFont = new ImageFont(mApp, theRes->mPath);
    } else {
        unreachable();
        /* TODO
        auto anImage = mApp->GetImage(theRes->mImagePath);
        if (anImage==NULL)
            return Fail(StrFormat("Failed to load image: %s",theRes->mImagePath.c_str()));

        theRes->mImage = anImage;
        aFont = new ImageFont(anImage, theRes->mPath);*/
    }

    auto anImageFont = dynamic_cast<ImageFont *>(aFont);
    if (anImageFont != nullptr) {
        if (anImageFont->mFontData == nullptr || !anImageFont->mFontData->mInitialized) {
            delete aFont;
            return Fail(StrFormat("Failed to load font: %s", theRes->mPath.c_str()));
        }

        if (!theRes->mTags.empty()) {
            char aBuf[1024];
            strcpy(aBuf, theRes->mTags.c_str());
            const char *aPtr = strtok(aBuf, ", \r\n\t");
            while (aPtr != nullptr) {
                anImageFont->AddTag(aPtr);
                aPtr = strtok(nullptr, ", \r\n\t");
            }
            anImageFont->Prepare();
        }
    }

    theRes->mFont = aFont;

    SEXY_PERF_END("ResourceManager:DoLoadFont");

    ResourceLoadedHook(theRes);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
_Font *ResourceManager::LoadFont(const std::string &theName) {
    auto anItr = mFontMap.find(theName);
    if (anItr == mFontMap.end()) return nullptr;

    auto aRes = static_cast<FontRes *>(anItr->second);
    if (aRes->mFont != nullptr) return aRes->mFont;

    if (aRes->mFromProgram) return nullptr;

    if (!DoLoadFont(aRes)) return nullptr;

    return aRes->mFont;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::DeleteFont(const std::string &theName) { ReplaceFont(theName, nullptr); }

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::LoadNextResource() {
    if (HadError()) return false;

    if (mCurResGroupList == nullptr) return false;

    while (mCurResGroupListItr != mCurResGroupList->end()) {
        BaseRes *aRes = *mCurResGroupListItr++;
        if (aRes->mFromProgram) continue;

        switch (aRes->mType) {
        case ResType_Image: {
            auto anImageRes = static_cast<ImageRes *>(aRes);
            if (anImageRes->mImage != nullptr) continue;

            return DoLoadImage(anImageRes);
        }

        case ResType_Sound: {
            auto aSoundRes = static_cast<SoundRes *>(aRes);
            if (aSoundRes->mSoundId != -1) continue;

            return DoLoadSound(aSoundRes);
        }

        case ResType_Font: {
            auto aFontRes = static_cast<FontRes *>(aRes);
            if (aFontRes->mFont != nullptr) continue;

            return DoLoadFont(aFontRes);
        }
        }
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void ResourceManager::ResourceLoadedHook(BaseRes *) {}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::StartLoadResources(const std::string &theGroup) {
    mError = "";
    mHasFailed = false;

    mCurResGroup = theGroup;
    mCurResGroupList = &mResGroupMap[theGroup];
    mCurResGroupListItr = mCurResGroupList->begin();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void ResourceManager::DumpCurResGroup(std::string &theDestStr) {
    const ResList *rl = &mResGroupMap.find(mCurResGroup)->second;
    auto it = rl->begin();
    theDestStr =
        StrFormat("About to dump %d elements from current res group name %s\r\n", rl->size(), mCurResGroup.c_str());

    auto rl_end = rl->end();
    while (it != rl_end) {
        BaseRes *br = *it++;
        std::string prefix = StrFormat("%s: %s\r\n", br->mId.c_str(), br->mPath.c_str());
        theDestStr += prefix;
        if (br->mFromProgram) theDestStr += std::string("     res is from program\r\n");
        else if (br->mType == ResType_Image) theDestStr += std::string("     res is an image\r\n");
        else if (br->mType == ResType_Sound) theDestStr += std::string("     res is a sound\r\n");
        else if (br->mType == ResType_Font) theDestStr += std::string("     res is a font\r\n");

        if (it == mCurResGroupListItr) theDestStr += std::string("iterator has reached mCurResGroupItr\r\n");
    }

    theDestStr += std::string("Done dumping resources\r\n");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::LoadResources(const std::string &theGroup) {
    mError = "";
    mHasFailed = false;
    StartLoadResources(theGroup);
    while (LoadNextResource()) {}

    if (!HadError()) {
        mLoadedGroups.insert(theGroup);
        return true;
    } else return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int ResourceManager::GetNumResources(const std::string &theGroup, ResMap &theMap) {
    if (theGroup.empty()) return theMap.size();

    int aCount = 0;
    for (auto anItr = theMap.begin(); anItr != theMap.end(); ++anItr) {
        BaseRes *aRes = anItr->second;
        if (aRes->mResGroup == theGroup && !aRes->mFromProgram) ++aCount;
    }

    return aCount;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int ResourceManager::GetNumImages(const std::string &theGroup) { return GetNumResources(theGroup, mImageMap); }

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int ResourceManager::GetNumSounds(const std::string &theGroup) { return GetNumResources(theGroup, mSoundMap); }

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int ResourceManager::GetNumFonts(const std::string &theGroup) { return GetNumResources(theGroup, mFontMap); }

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int ResourceManager::GetNumResources(const std::string &theGroup) {
    return GetNumImages(theGroup) + GetNumSounds(theGroup) + GetNumFonts(theGroup);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
Image *ResourceManager::GetImage(const std::string &theId) {
    auto anItr = mImageMap.find(theId);
    if (anItr != mImageMap.end()) return static_cast<ImageRes *>(anItr->second)->mImage;
    else return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int ResourceManager::GetSound(const std::string &theId) {
    auto anItr = mSoundMap.find(theId);
    if (anItr != mSoundMap.end()) return static_cast<SoundRes *>(anItr->second)->mSoundId;
    else return -1;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
_Font *ResourceManager::GetFont(const std::string &theId) {
    auto anItr = mFontMap.find(theId);
    if (anItr != mFontMap.end()) return static_cast<FontRes *>(anItr->second)->mFont;
    else return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
Image *ResourceManager::GetImageThrow(const std::string &theId) {
    auto anItr = mImageMap.find(theId);
    if (anItr != mImageMap.end()) {
        auto aRes = static_cast<ImageRes *>(anItr->second);

        if (aRes->mImage != nullptr) return aRes->mImage;

        if (mAllowMissingProgramResources && aRes->mFromProgram) return nullptr;
    }

    Fail(StrFormat("Image resource not found: %s", theId.c_str()));
    throw ResourceManagerException(GetErrorText());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int ResourceManager::GetSoundThrow(const std::string &theId) {
    auto anItr = mSoundMap.find(theId);
    if (anItr != mSoundMap.end()) {
        auto aRes = static_cast<SoundRes *>(anItr->second);
        if (aRes->mSoundId != -1) return aRes->mSoundId;

        if (mAllowMissingProgramResources && aRes->mFromProgram) return -1;
    }

    Fail(StrFormat("Sound resource not found: %s", theId.c_str()));
    throw ResourceManagerException(GetErrorText());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
_Font *ResourceManager::GetFontThrow(const std::string &theId) {
    auto anItr = mFontMap.find(theId);
    if (anItr != mFontMap.end()) {
        auto aRes = static_cast<FontRes *>(anItr->second);
        if (aRes->mFont != nullptr) return aRes->mFont;

        if (mAllowMissingProgramResources && aRes->mFromProgram) return nullptr;
    }

    Fail(StrFormat("Font resource not found: %s", theId.c_str()));
    throw ResourceManagerException(GetErrorText());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ResourceManager::SetAllowMissingProgramImages(bool allow) { mAllowMissingProgramResources = allow; }

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::ReplaceImage(const std::string &theId, Image *theImage) {
    auto anItr = mImageMap.find(theId);
    if (anItr != mImageMap.end()) {
        anItr->second->DeleteResource();

        static_cast<ImageRes *>(anItr->second)->mImage = theImage;
        //((ImageRes*)anItr->second)->mImage.mOwnsUnshared = true;
        return true;
    } else return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::ReplaceSound(const std::string &theId, int theSound) {
    auto anItr = mSoundMap.find(theId);
    if (anItr != mSoundMap.end()) {
        anItr->second->DeleteResource();
        static_cast<SoundRes *>(anItr->second)->mSoundId = theSound;
        return true;
    } else return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool ResourceManager::ReplaceFont(const std::string &theId, _Font *theFont) {
    auto anItr = mFontMap.find(theId);
    if (anItr != mFontMap.end()) {
        anItr->second->DeleteResource();
        static_cast<FontRes *>(anItr->second)->mFont = theFont;
        return true;
    } else return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
const XMLParamMap &ResourceManager::GetImageAttributes(const std::string &theId) {
    static XMLParamMap aStrMap;

    auto anItr = mImageMap.find(theId);
    if (anItr != mImageMap.end()) return anItr->second->mXMLAttributes;
    else return aStrMap;
}
