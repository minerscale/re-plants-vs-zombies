#ifndef __NEWOPTIONSDIALOG_H__
#define __NEWOPTIONSDIALOG_H__

#include "framework/widget/CheckboxListener.h"
#include "framework/widget/Dialog.h"
#include "framework/widget/SliderListener.h"

class LawnApp;
class LawnStoneButton;
class NewLawnButton;

namespace Sexy {
class Slider;
class Checkbox;
}; // namespace Sexy

class NewOptionsDialog : public Sexy::Dialog, public Sexy::SliderListener, public Sexy::CheckboxListener {
protected:
    enum {
        NewOptionsDialog_Almanac,
        NewOptionsDialog_MainMenu,
        NewOptionsDialog_Restart,
        NewOptionsDialog_Update,
        NewOptionsDialog_MusicVolume,
        NewOptionsDialog_SoundVolume,
        NewOptionsDialog_Fullscreen,
        NewOptionsDialog_HardwareAcceleration,
    };

public:
    LawnApp *mApp;                                 //+0x158
    Sexy::Slider *mMusicVolumeSlider;              //+0x15C
    Sexy::Slider *mSfxVolumeSlider;                //+0x160
    Sexy::Checkbox *mFullscreenCheckbox;           //+0x164
    Sexy::Checkbox *mHardwareAccelerationCheckbox; //+0x168
    LawnStoneButton *mAlmanacButton;               //+0x16C
    LawnStoneButton *mBackToMainButton;            //+0x170
    LawnStoneButton *mRestartButton;               //+0x174
    NewLawnButton *mBackToGameButton;              //+0x178
    bool mFromGameSelector;                        //+0x17C

public:
    NewOptionsDialog(LawnApp *theApp, bool theFromGameSelector);
    ~NewOptionsDialog() override;

    int GetPreferredHeight(int theWidth) override;
    void AddedToManager(Sexy::WidgetManager *theWidgetManager) override;
    void RemovedFromManager(Sexy::WidgetManager *theWidgetManager) override;
    void Resize(int theX, int theY, int theWidth, int theHeight) override;
    void Draw(Sexy::Graphics *g) override;
    void SliderVal(int theId, double theVal) override;
    void CheckboxChecked(int theId, bool checked) override;
    void ButtonPress(int theId) override;
    void ButtonDepress(int theId) override;
    void KeyDown(Sexy::KeyCode theKey) override;
};

#endif
