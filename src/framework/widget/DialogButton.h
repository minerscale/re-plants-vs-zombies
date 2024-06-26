#ifndef __DIALOGBUTTON_H__
#define __DIALOGBUTTON_H__

#include "ButtonWidget.h"

namespace Sexy {
class DialogButton : public ButtonWidget {
public:
    Image *mComponentImage;
    int mTranslateX, mTranslateY;
    int mTextOffsetX, mTextOffsetY;

public:
    DialogButton(Image *theComponentImage, int theId, ButtonListener *theListener);

    void Draw(Graphics *g) override;
};
} // namespace Sexy

#endif //__DIALOGBUTTON_H__
