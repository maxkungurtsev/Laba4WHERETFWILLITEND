#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Window.h"
#include "InputDevice.h"
#include "Renderer.h"

Renderer g_Renderer;
Window g_Window;
InputDevice g_Input;
int Run() {
    MSG msg = {};
    bool running = true;
    while (running) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                running = false;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        g_Input.Update();
        if (g_Input.IsKeyDown(VK_ESCAPE)) {
            PostQuitMessage(0);
        }
        else if (g_Input.IsKeyDown(VK_SPACE)) {
            MessageBox(nullptr, L"Space detected!", L"Input Test", MB_OK);
        }
        g_Renderer.RenderFrame();
    }
    return (int)msg.wParam;
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow){
    //init window and check if it actually worked
    if (!g_Window.Create(hInstance, nCmdShow))
        return 0;
    //init input device
    g_Input.Initialize(g_Window.GetHWND());
    //init renderer
    g_Renderer.Initialize(1280, 720, 2, g_Window.GetHWND());
    //catch messege stuff
    int messege = Run();
    return static_cast<int>(messege);
}