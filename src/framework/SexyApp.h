#ifndef __SEXYAPP_H__
#define __SEXYAPP_H__

#include "SexyAppBase.h"

namespace Sexy {
class InternetManager;
class BetaSupport;

class SexyApp : public SexyAppBase {
public:
    InternetManager *mInternetManager;
    BetaSupport *mBetaSupport;

    std::string mBetaSupportSiteOverride;
    std::string mBetaSupportProdNameOverride;
    std::string mReferId;
    std::string mVariation;
    uint32_t mDownloadId;
    std::string mRegSource;
    uint32_t mLastVerCheckQueryTime;
    bool mSkipAd;
    bool mDontUpdate;

    int mBuildNum;
    std::string mBuildDate;

    std::string mUserName;
    std::string mRegUserName;
    std::string mRegCode;
    bool mIsRegistered;
    bool mBuildUnlocked;

    int mTimesPlayed;
    int mTimesExecuted;
    bool mTimedOut;

#ifdef ZYLOM
    uint mZylomGameId;
#endif

public:
    void UpdateFrames() override;

    void WriteToRegistry() override;
    void ReadFromRegistry() override;

    //	virtual bool			CheckSignature(const Buffer& theBuffer, const std::string& theFileName);

    virtual bool ShouldCheckForUpdate();
    virtual void UpdateCheckQueried();

    void URLOpenSucceeded(const std::string &theURL) override;

public:
    SexyApp();
    ~SexyApp() override;

    //	bool					Validate(const std::string& theUserName, const std::string& theRegCode);

    virtual bool OpenRegisterPage(DefinesMap theDefinesMap);
    virtual bool OpenRegisterPage();

    void PreDisplayHook() override;
    void InitPropertiesHook() override;
    void Init() override;
    void PreTerminate() override;

    virtual bool OpenHTMLTemplate(const std::string &theTemplateFile, const DefinesMap &theDefinesMap);
    virtual void OpenUpdateURL();

    void HandleCmdLineParam(const std::string &theParamName, const std::string &theParamValue) override;
    virtual std::string GetGameSEHInfo();
    void GetSEHWebParams(DefinesMap *theDefinesMap) override;

#ifdef ZYLOM
    bool ZylomUpdateCheckNeeded();
    void ZylomShowAd();
#endif
};

extern SexyApp *gSexyApp;
}; // namespace Sexy

#endif //__SEXYAPP_H__
