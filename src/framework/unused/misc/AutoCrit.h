#ifndef AUTOCRIT_INCLUDED_
#define __AUTOCRIT_INCLUDED__

#include "Common.h"
#include "CritSect.h"

namespace Sexy {

class AutoCrit {
    CritSect *mCritSec;

public:
    AutoCrit(CritSect &theCritSect) : mCritSec(&theCritSect) { mCritSec->Lock(); }

    ~AutoCrit() { mCritSec->Unlock(); }
};
} // namespace Sexy

#endif //__AUTOCRIT_INCLUDED__
