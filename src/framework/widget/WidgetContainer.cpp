#include "WidgetContainer.h"
#include "Common.h"
#include "Widget.h"
#include "WidgetManager.h"
#include "misc/Debug.h"
#include <algorithm>

using namespace Sexy;

WidgetContainer::WidgetContainer() {
    mX = 0;
    mY = 0;
    mWidth = 0;
    mHeight = 0;
    mParent = nullptr;
    mWidgetManager = nullptr;
    mUpdateIteratorModified = false;
    mUpdateIterator = mWidgets.end();
    mLastWMUpdateCount = 0;
    mUpdateCnt = 0;
    mDirty = false;
    mHasAlpha = false;
    mClip = true;
    mPriority = 0;
    mZOrder = 0;
}

WidgetContainer::~WidgetContainer() {
    // call RemoveWidget before you delete it!
    DBG_ASSERT(mParent == NULL);
    DBG_ASSERT(mWidgets.empty());
}

void WidgetContainer::RemoveAllWidgets(bool doDelete, bool recursive) {
    while (!mWidgets.empty()) {
        Widget *aWidget = mWidgets.front();
        RemoveWidget(aWidget);
        if (recursive) aWidget->RemoveAllWidgets(doDelete, recursive);

        if (doDelete) delete aWidget;
    }
}

Rect WidgetContainer::GetRect() { return Rect(mX, mY, mWidth, mHeight); }

bool WidgetContainer::Intersects(WidgetContainer *theWidget) { return GetRect().Intersects(theWidget->GetRect()); }

void WidgetContainer::AddWidget(Widget *theWidget) {
    if (std::ranges::find(mWidgets, theWidget) == mWidgets.end()) {
        InsertWidgetHelper(mWidgets.end(), theWidget);
        theWidget->mWidgetManager = mWidgetManager;
        theWidget->mParent = this;

        if (mWidgetManager != nullptr) {
            theWidget->AddedToManager(mWidgetManager);
            theWidget->MarkDirtyFull();
            mWidgetManager->RehupMouse();
        }

        MarkDirty();
    }
}

bool WidgetContainer::HasWidget(Widget *theWidget) { return std::ranges::find(mWidgets, theWidget) != mWidgets.end(); }

void WidgetContainer::RemoveWidget(Widget *theWidget) {
    auto anItr = std::ranges::find(mWidgets, theWidget);
    if (anItr != mWidgets.end()) {
        theWidget->WidgetRemovedHelper();
        theWidget->mParent = nullptr;

        bool erasedCur = (anItr == mUpdateIterator);
        mWidgets.erase(anItr++);
        if (erasedCur) {
            mUpdateIterator = anItr;
            mUpdateIteratorModified = true;
        }
    }
}

Widget *WidgetContainer::GetWidgetAtHelper(int x, int y, int theFlags, bool *found, int *theWidgetX, int *theWidgetY) {
    bool belowModal = false;

    ModFlags(theFlags, mWidgetFlagsMod);

    auto anItr = mWidgets.rbegin();
    while (anItr != mWidgets.rend()) {
        Widget *aWidget = *anItr;

        int aCurFlags = theFlags;
        ModFlags(aCurFlags, aWidget->mWidgetFlagsMod);
        if (belowModal) ModFlags(aCurFlags, mWidgetManager->mBelowModalFlagsMod);

        if (aCurFlags & WIDGETFLAGS_ALLOW_MOUSE) {
            if (aWidget->mVisible) {
                bool childFound;
                Widget *aCheckWidget = aWidget->GetWidgetAtHelper(
                    x - aWidget->mX, y - aWidget->mY, aCurFlags, &childFound, theWidgetX, theWidgetY
                );
                if ((aCheckWidget != nullptr) || (childFound)) {
                    *found = true;
                    return aCheckWidget;
                }

                if ((aWidget->mMouseVisible) && (aWidget->GetInsetRect().Contains(x, y))) {
                    *found = true;

                    if (aWidget->IsPointVisible(x - aWidget->mX, y - aWidget->mY)) {
                        if (theWidgetX) *theWidgetX = x - aWidget->mX;
                        if (theWidgetY) *theWidgetY = y - aWidget->mY;
                        return aWidget;
                    }
                }
            }
        }

        belowModal |= aWidget == mWidgetManager->mBaseModalWidget;

        ++anItr;
    }

    *found = false;
    return nullptr;
}

bool WidgetContainer::IsBelowHelper(Widget *theWidget1, Widget *theWidget2, bool *found) {
    auto anItr = mWidgets.begin();
    while (anItr != mWidgets.end()) {
        Widget *aWidget = *anItr;

        if (aWidget == theWidget1) {
            *found = true;
            return true;
        } else if (aWidget == theWidget2) {
            *found = true;
            return false;
        }

        bool result = aWidget->IsBelowHelper(theWidget1, theWidget2, found);
        if (*found) return result;

        ++anItr;
    }

    return false;
}

bool WidgetContainer::IsBelow(Widget *theWidget1, Widget *theWidget2) {
    bool found = false;
    return IsBelowHelper(theWidget1, theWidget2, &found);
}

void WidgetContainer::MarkAllDirty() {
    MarkDirty();

    auto anItr = mWidgets.begin();
    while (anItr != mWidgets.end()) {
        (*anItr)->mDirty = true;
        (*anItr)->MarkAllDirty();
        ++anItr;
    }
}

void WidgetContainer::InsertWidgetHelper(const WidgetList::iterator &where, Widget *theWidget) {
    // Search forwards
    auto anItr = where;
    while (anItr != mWidgets.end()) {
        Widget *aWidget = *anItr;
        if (aWidget->mZOrder >= theWidget->mZOrder) {
            if (anItr != mWidgets.begin()) {
                auto anItr2 = anItr;
                --anItr2;
                aWidget = *anItr;
                if (aWidget->mZOrder > theWidget->mZOrder) // need to search backwards
                    break;
            }

            mWidgets.insert(anItr, theWidget);
            return;
        }
        ++anItr;
    }

    // Search backwards
    while (anItr != mWidgets.begin()) {
        --anItr;
        Widget *aWidget = *anItr;
        if (aWidget->mZOrder <= theWidget->mZOrder) {
            mWidgets.insert(++anItr, theWidget);
            return;
        }
    }

    // It goes at the beginning
    mWidgets.push_front(theWidget);
}

void WidgetContainer::BringToFront(Widget *theWidget) {
    auto anItr = std::ranges::find(mWidgets, theWidget);
    if (anItr != mWidgets.end()) {
        if (anItr == mUpdateIterator) {
            ++mUpdateIterator;
            mUpdateIteratorModified = true;
        }

        mWidgets.erase(anItr);
        InsertWidgetHelper(mWidgets.end(), theWidget);

        theWidget->OrderInManagerChanged();
    }
}

void WidgetContainer::BringToBack(Widget *theWidget) {
    auto anItr = std::ranges::find(mWidgets, theWidget);
    if (anItr != mWidgets.end()) {
        if (anItr == mUpdateIterator) {
            ++mUpdateIterator;
            mUpdateIteratorModified = true;
        }

        mWidgets.erase(anItr);
        InsertWidgetHelper(mWidgets.begin(), theWidget);

        theWidget->OrderInManagerChanged();
    }
}

void WidgetContainer::PutBehind(Widget *theWidget, Widget *theRefWidget) {
    auto anItr = std::ranges::find(mWidgets, theWidget);
    if (anItr != mWidgets.end()) {
        if (anItr == mUpdateIterator) {
            ++mUpdateIterator;
            mUpdateIteratorModified = true;
        }

        mWidgets.erase(anItr);
        anItr = std::ranges::find(mWidgets, theRefWidget);
        InsertWidgetHelper(anItr, theWidget);

        theWidget->OrderInManagerChanged();
    }
}

void WidgetContainer::PutInfront(Widget *theWidget, Widget *theRefWidget) {
    auto anItr = std::ranges::find(mWidgets, theWidget);
    if (anItr != mWidgets.end()) {
        if (anItr == mUpdateIterator) {
            ++mUpdateIterator;
            mUpdateIteratorModified = true;
        }

        mWidgets.erase(anItr);
        anItr = std::ranges::find(mWidgets, theRefWidget);
        if (anItr != mWidgets.end()) ++anItr;
        InsertWidgetHelper(anItr, theWidget);

        theWidget->OrderInManagerChanged();
    }
}

Point WidgetContainer::GetAbsPos() // relative to top level
{
    if (mParent == nullptr) return {mX, mY};
    else return Point(mX, mY) + mParent->GetAbsPos();
}

void WidgetContainer::AddedToManager(WidgetManager *theWidgetManager) {
    auto anItr = mWidgets.begin();
    while (anItr != mWidgets.end()) {
        Widget *theWidget = *anItr;

        theWidget->mWidgetManager = theWidgetManager;
        theWidget->AddedToManager(theWidgetManager);
        ++anItr;

        MarkDirty();
    }
}

void WidgetContainer::RemovedFromManager(WidgetManager *theWidgetManager) {
    for (auto anItr = mWidgets.begin(); anItr != mWidgets.end(); ++anItr) {
        Widget *aWidget = *anItr;

        // theWidgetManager->DisableWidget(aWidget);
        aWidget->RemovedFromManager(theWidgetManager);
        aWidget->mWidgetManager = nullptr;
    }

    if (theWidgetManager->mPopupCommandWidget == this) theWidgetManager->mPopupCommandWidget = nullptr;
}

void WidgetContainer::MarkDirty() {
    if (mParent != nullptr) mParent->MarkDirty(this);
    else mDirty = true;
}

void WidgetContainer::MarkDirtyFull() {
    if (mParent != nullptr) mParent->MarkDirtyFull(this);
    else mDirty = true;
}

void WidgetContainer::MarkDirtyFull(WidgetContainer *theWidget) {
    // Mark all things dirty that are under or over this widget

    // Mark ourselves dirty
    MarkDirtyFull();

    theWidget->mDirty = true;

    // Top-level windows are treated differently, as marking a child dirty always
    //  causes a parent redraw which always causes all children to redraw
    if (mParent != nullptr) return;

    auto aFoundWidgetItr = std::ranges::find(mWidgets, theWidget);
    if (aFoundWidgetItr == mWidgets.end()) return;

    auto anItr = aFoundWidgetItr;
    if (anItr != mWidgets.begin()) {
        --anItr;

        for (;;) {
            Widget *aWidget = *anItr;

            if (aWidget->mVisible) {
                if ((!aWidget->mHasTransparencies) && (!aWidget->mHasAlpha)) {
                    // Clip the widget's bounds to the screen and check if it fully overlapped by this non-transparent
                    // widget underneath it If it is fully overlapped then we can stop marking dirty underneath it since
                    // it's not transparent.
                    Rect aRect = Rect(theWidget->mX, theWidget->mY, theWidget->mWidth, theWidget->mHeight)
                                     .Intersection(Rect(0, 0, mWidth, mHeight));
                    if ((aWidget->Contains(aRect.mX, aRect.mY) &&
                         (aWidget->Contains(aRect.mX + aRect.mWidth - 1, aRect.mY + aRect.mHeight - 1)))) {
                        // If this widget is fully contained within a lower widget, there is no need to dig down
                        // any deeper.
                        aWidget->MarkDirty();
                        break;
                    }
                }

                if (aWidget->Intersects(theWidget)) MarkDirty(aWidget);
            }

            if (anItr == mWidgets.begin()) break;

            --anItr;
        }
    }

    anItr = aFoundWidgetItr;
    while (anItr != mWidgets.end()) {
        Widget *aWidget = *anItr;
        if ((aWidget->mVisible) && (aWidget->Intersects(theWidget))) MarkDirty(aWidget);

        ++anItr;
    }
}

void WidgetContainer::MarkDirty(WidgetContainer *theWidget) {
    if (theWidget->mDirty) return;

    // Only mark things dirty that are on top of this widget
    // Mark ourselves dirty
    MarkDirty();

    theWidget->mDirty = true;

    // Top-level windows are treated differently, as marking a child dirty always
    //  causes a parent redraw which always causes all children to redraw
    if (mParent != nullptr) return;

    if (theWidget->mHasAlpha) MarkDirtyFull(theWidget);
    else {
        bool found = false;
        auto anItr = mWidgets.begin();
        while (anItr != mWidgets.end()) {
            Widget *aWidget = *anItr;
            if (aWidget == theWidget) found = true;
            else if (found) {
                if ((aWidget->mVisible) && (aWidget->Intersects(theWidget))) MarkDirty(aWidget);
            }

            ++anItr;
        }
    }
}

void WidgetContainer::Update() { mUpdateCnt++; }

void WidgetContainer::UpdateAll(ModalFlags *theFlags) {
    AutoModalFlags anAutoModalFlags(theFlags, mWidgetFlagsMod);

    if (theFlags->GetFlags() & WIDGETFLAGS_MARK_DIRTY) MarkDirty();

    // Can update?
    WidgetManager *aWidgetManager = mWidgetManager;
    if (aWidgetManager == nullptr) return;

    if (theFlags->GetFlags() & WIDGETFLAGS_UPDATE) {
        if (mLastWMUpdateCount != static_cast<uint32_t>(mWidgetManager->mUpdateCnt)) {
            mLastWMUpdateCount = mWidgetManager->mUpdateCnt;
            Update();
        }
    }

    mUpdateIterator = mWidgets.begin();

    while (mUpdateIterator != mWidgets.end()) {
        mUpdateIteratorModified = false;

        Widget *aWidget = *mUpdateIterator;
        if (aWidget == aWidgetManager->mBaseModalWidget) theFlags->mIsOver = true;

        aWidget->UpdateAll(theFlags);

        if (!mUpdateIteratorModified) ++mUpdateIterator;
    }

    mUpdateIteratorModified = true; // prevent incrementing iterator off the end of the list
}

/*
void WidgetContainer::UpdateF(float theFrac)
{
}
*/

void WidgetContainer::UpdateFAll(ModalFlags *theFlags, float theFrac) {
    AutoModalFlags anAutoModalFlags(theFlags, mWidgetFlagsMod);

    // Can update?
    /*
    if (theFlags->GetFlags() & WIDGETFLAGS_UPDATE)
    {
        UpdateF(theFrac);
    }
    */

    mUpdateIterator = mWidgets.begin();
    while (mUpdateIterator != mWidgets.end()) {
        mUpdateIteratorModified = false;

        Widget *aWidget = *mUpdateIterator;
        if (aWidget == mWidgetManager->mBaseModalWidget) theFlags->mIsOver = true;

        aWidget->UpdateFAll(theFlags, theFrac);

        if (!mUpdateIteratorModified) ++mUpdateIterator;
    }

    mUpdateIteratorModified = true; // prevent incrementing iterator off the end of the list
}

void WidgetContainer::Draw(Graphics *) {}

void WidgetContainer::DrawAll(ModalFlags *theFlags, Graphics *g) {
    if (mPriority > mWidgetManager->mMinDeferredOverlayPriority) mWidgetManager->FlushDeferredOverlayWidgets(mPriority);

    AutoModalFlags anAutoModalFlags(theFlags, mWidgetFlagsMod);

    if ((mClip) && (theFlags->GetFlags() & WIDGETFLAGS_CLIP)) g->ClipRect(0, 0, mWidth, mHeight);

    if (mWidgets.size() == 0) {
        if (theFlags->GetFlags() & WIDGETFLAGS_DRAW) Draw(g);
        return;
    }

    if (theFlags->GetFlags() & WIDGETFLAGS_DRAW) {
        g->PushState();
        Draw(g);
        g->PopState();
    }

    auto anItr = mWidgets.begin();
    while (anItr != mWidgets.end()) {
        Widget *aWidget = *anItr;

        if (aWidget->mVisible) {
            if (aWidget == mWidgetManager->mBaseModalWidget) theFlags->mIsOver = true;

            Graphics aClipG(*g);
            aClipG.Translate(aWidget->mX, aWidget->mY);
            aWidget->DrawAll(theFlags, &aClipG);
            aWidget->mDirty = false;
        }

        ++anItr;
    }
}

void WidgetContainer::SysColorChanged() {}

void WidgetContainer::SysColorChangedAll() {
    SysColorChanged();

    /* unused
    static int aDepthCount = 0;
    if (mWidgets.size() > 0)
        aDepthCount++;
    */

    auto anItr = mWidgets.begin();
    while (anItr != mWidgets.end()) {
        Widget *aWidget = *anItr;

        aWidget->SysColorChangedAll();
        ++anItr;
    }
}

void WidgetContainer::DisableWidget(Widget *) {}

void WidgetContainer::SetFocus(Widget *) {}
