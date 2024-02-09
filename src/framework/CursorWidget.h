#ifndef __CURSORWIDGET_H__
#define __CURSORWIDGET_H__

#include "Point.h"
#include "Widget.h"

namespace Sexy {

class Image;

class CursorWidget : public Widget {
public:
    Image *mImage;

public:
    CursorWidget();

    virtual void Draw(Graphics *g);
    void SetImage(Image *theImage);
    Point GetHotspot();
};

} // namespace Sexy

#endif //__CURSORWIDGET_H__
