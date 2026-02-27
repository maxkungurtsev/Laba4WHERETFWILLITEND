#include "Window.h"
#include "InputDevice.h"
#include "Renderer.h"
#include "Model.h"
#include <chrono>
Window g_Window;
InputDevice g_Input;
Renderer g_Renderer;
const int width = 800;
const int height = 800;
const int depth = 1000;
int texture_width = 800;
int texture_height = 800;
//light stuff
const float intensity = 0.1;
XMFLOAT3 light_coords = {10.0, 10.0, 10.0};
//camera stuff
const XMVECTOR cam_coords = {-7.5, 0.0, 5.0, 1.0};
const XMVECTOR look_at = {-3.0, -1.0, 0.0, 1.0};
const XMVECTOR up = {0.0, 1.0, 0.0, 1.0 };
// material stuff
const float ambient_k = 0.3;
const float diffuse_k = 0.5;
const float specular_k = 0.8;
const float shiny_k = 32;
int Run(Model &mesh) {
    MSG msg = {};
    bool running = true;
    using clock = std::chrono::high_resolution_clock;
    auto lastTime = clock::now();
    float time = 0;
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
       auto currentTime = clock::now();
       float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
       lastTime = currentTime;
       time += deltaTime;
       time = std::fmod(time, 4.0f);
       g_Renderer.RenderFrame(mesh, time);
    }
    return (int)msg.wParam;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow){
    //init window and check if it actually worked
    if (!g_Window.Create(hInstance, nCmdShow))
        return 0;
    //init input device
    g_Input.Initialize(g_Window.GetHWND());
    //catch messege stuff
    Model mesh("bean.obj"); // загружаем модель и материалы и текстуры и вообще пздц
    g_Renderer.Initialize(width,height,2, g_Window.GetHWND(),mesh, cam_coords, look_at, up, light_coords);

    int messege = Run(mesh);
    return static_cast<int>(messege);
}