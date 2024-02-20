#ifndef __VK_INTERFACE_H__
#define __VK_INTERFACE_H__

#include "framework/graphics/WindowInterface.h"
#include "framework/widget/WidgetManager.h"

namespace Vk {
class VkInterface : public WindowInterface<VkInterface> {
public:
    VkInterface(int width, int height, WidgetManager *mWidgetManager, bool fullscreen);
    ~VkInterface();

    int GetRefreshRate();
    static void UpdateWindowOptions(const int width, const int height, const bool fullscreen);
    static Image *GetScreenImage();
    static int
    CreateCursor(int xHotSpot, int yHotSpot, int nWidth, int nHeight, const void *pvANDPlane, const void *pvXORPlane);
    static void EnforceCursor();
    static void ShowWindow();
    void PollEvents();
    static bool IsFocused();
    static void RehupFocus();
    bool ShouldClose() const;
    static void ReleaseMouseCapture();
    static void Draw();

private:
    bool windowShouldClose = false;
    static void framebufferResizeCallback();
    static void windowFocusCallback(bool focused);
    static void cursorPositionCallback(double xpos, double ypos);
    static void mouseWheelCallback(double xoffset, double yoffset);
    void mouseButtonCallback(int button, int state, int clicks) const;
    static void keyCallback(uint32_t key, uint8_t state);
    static void charCallback(char codepoint[32]);
    static void cursorEnterCallback(int entered);
    void windowCloseCallback();
};
} // namespace Vk

#endif // __VK_INTERFACE_H__
