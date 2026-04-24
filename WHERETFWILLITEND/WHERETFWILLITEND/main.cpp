#include "Window.h"
#include "InputDevice.h"
#include "NEWRenderer.h"
#include "Model.h"
#include <chrono>
Window g_Window;
InputDevice g_Input;
std::shared_ptr<NewRenderer> g_Renderer;
const int width = 800;
const int height = 800;
const int depth = 1000;
int texture_width = 800;
int texture_height = 800;
//light stuff
const float intensity = 0.1;
XMFLOAT3 light_coords = {0.0, 10.0, 10.0};
//camera stuff
XMVECTOR cam_coords = {0.0, 10.0, 0.0, 1.0};
XMVECTOR look_at = {0.0, 0.0, 0.0, 1.0};
XMVECTOR up = {0.0, 0.0, 1.0, 1.0 };
// material stuff
const float ambient_k = 0.3;
const float diffuse_k = 0.5;
const float specular_k = 0.8;
const float shiny_k = 32;
int Run() {
    MSG msg = {};
    bool running = true;
    using clock = std::chrono::high_resolution_clock;
    auto lastTime = clock::now();
    float time = 0;
    bool shootLight = false;
    bool culling_enabled = true;
    while (running) {
        shootLight = false;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                running = false;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        g_Input.Update();
        XMVECTOR forward = look_at - cam_coords;
        XMVECTOR right = XMVector3Normalize((XMVector3Cross(forward, up)));
        up = XMVector3Normalize((XMVector3Cross(right, forward)));
        if (g_Input.IsKeyDown(VK_ESCAPE)) {
            PostQuitMessage(0);
        }
        if (g_Input.IsKeyDown(VK_SPACE)) {
            shootLight = true;
        }
        if (g_Input.IsKeyDown(VK_UP)) {
            XMMATRIX rotPitch = XMMatrixRotationAxis(right, 0.05);
            forward = XMVector3TransformCoord(forward, rotPitch);
            up = XMVector3TransformCoord(up, rotPitch);
        }
        if (g_Input.IsKeyDown(VK_DOWN)) {
            XMMATRIX rotPitch = XMMatrixRotationAxis(right, -0.05);
            forward = XMVector3TransformCoord(forward, rotPitch);
            up = XMVector3TransformCoord(up, rotPitch);
        }
        if (g_Input.IsKeyDown(VK_LEFT)) {
            XMMATRIX rotYaw = XMMatrixRotationAxis(up, -0.05);
            forward = XMVector3TransformCoord(forward, rotYaw);
            right = XMVector3TransformCoord(right, rotYaw);
        }
        if (g_Input.IsKeyDown(VK_RIGHT)) {
            XMMATRIX rotYaw = XMMatrixRotationAxis(up, 0.05);
            forward = XMVector3TransformCoord(forward, rotYaw);
            right = XMVector3TransformCoord(right, rotYaw);
        }
        if (g_Input.IsKeyDown('E')) {
            culling_enabled = false;
        }
        if (g_Input.IsKeyDown('Q')) {
            culling_enabled = true;
        }
        if (g_Input.IsKeyDown('W')) {
            cam_coords += forward * 1;
        }
        if (g_Input.IsKeyDown('S')) {
            cam_coords -= forward * 1;
        }
        if (g_Input.IsKeyDown('A')) {
            cam_coords += right * 1;
        }
        if (g_Input.IsKeyDown('D')) {
            cam_coords -= right * 1;
        }
       auto currentTime = clock::now();
       float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
       lastTime = currentTime;
       time += deltaTime;
       look_at = cam_coords + forward;
       g_Renderer->RenderFrame(time, look_at, cam_coords, up, shootLight, culling_enabled);
    }
    return (int)msg.wParam;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow){
    //init window and check if it actually worked
    if (!g_Window.Create(hInstance, nCmdShow)){
            return 0;
    }
    //init input device
    g_Input.Initialize(g_Window.GetHWND());
    //catch messege stuff
    std::vector<std::string> pathes = {};
    g_Renderer = std::make_shared<NewRenderer>(width,height,2, &(g_Window), pathes, cam_coords, look_at, up, 0);

    int messege = Run();
    return static_cast<int>(messege);
}