#ifndef __SLIDER_H__
#define __SLIDER_H__

#include "Widget.h"

namespace Sexy {
class SliderListener;

class Slider : public Widget {
public:
    SliderListener *mListener;
    double mVal;
    int mId;
    Image *mTrackImage;
    Image *mThumbImage;

    bool mDragging;
    int mRelX;
    int mRelY;

    bool mHorizontal;

public:
    Slider(Image *theTrackImage, Image *theThumbImage, int theId, SliderListener *theListener);

    virtual void SetValue(double theValue);

    virtual bool HasTransparencies();
    void Draw(Graphics *g) override;

    void MouseMove(int x, int y) override;
    void MouseDown(int x, int y, int theClickCount) override;
    void MouseDrag(int x, int y) override;
    void MouseUp(int x, int y) override;
    void MouseLeave() override;
};
} // namespace Sexy

#endif //__SLIDER_H__
