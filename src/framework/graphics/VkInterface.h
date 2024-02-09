#ifndef __VK_INTERFACE_H__
#define __VK_INTERFACE_H__

#include "framework/graphics/WindowInterface.h"
#include "framework/widget/WidgetManager.h"
#include <glm/fwd.hpp>
#include <memory>

namespace Vk {
class VkInterface : public WindowInterface<VkInterface> {
public:
    VkInterface(int width, int height, WidgetManager *mWidgetManager);
    ~VkInterface();

    Image *GetScreenImage();
    int
    CreateCursor(int xHotSpot, int yHotSpot, int nWidth, int nHeight, const void *pvANDPlane, const void *pvXORPlane);
    void EnforceCursor();
    void ShowWindow();
    void PollEvents();
    bool IsFocused();
    void RehupFocus();
    bool ShouldClose();
    void ReleaseMouseCapture();
    void Draw();

private:
    bool windowShouldClose = false;
    void framebufferResizeCallback();
    void windowFocusCallback(bool focused);
    void cursorPositionCallback(double xpos, double ypos);
    void mouseWheelCallback(double xoffset, double yoffset);
    void mouseButtonCallback(int button, int state, int clicks);
    void keyCallback(uint32_t key, uint8_t state);
    void charCallback(char codepoint[32]);
    void cursorEnterCallback(int entered);
    void windowCloseCallback();
};
} // namespace Vk

#endif // __VK_INTERFACE_H__
