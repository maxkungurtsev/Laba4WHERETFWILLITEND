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
XMFLOAT3 light_coords = {0.0, 10.0, 10.0};
//camera stuff
XMVECTOR cam_coords = {0.0, 2.0, -5.0, 1.0};
XMVECTOR look_at = {5.0, 5.0, 0.0, 1.0};
XMVECTOR up = {0.0, 1.0, 0.0, 1.0 };
int Run(Model &mesh) {
    MSG msg = {};
    bool running = true;
    using clock = std::chrono::high_resolution_clock;
    auto lastTime = clock::now();
    float time = 0;
    float pitch = 0;
    float yaw = 0;
    while (running) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                running = false;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        pitch = 0;
        yaw = 0;
        g_Input.Update();
        if (g_Input.IsKeyDown(VK_ESCAPE)) {
            PostQuitMessage(0);
        }
        // CAMERA ROTATION
        if (g_Input.IsKeyDown(VK_UP)) {
            pitch--;
            //OutputDebugStringA("up");
        }
        if (g_Input.IsKeyDown(VK_DOWN)) {
            pitch++;
            //OutputDebugStringA("down");
        }
        if (g_Input.IsKeyDown(VK_RIGHT)) {
            yaw++;
            //OutputDebugStringA("right");
        }
        if (g_Input.IsKeyDown(VK_LEFT)) {
            yaw--;
            //OutputDebugStringA("left");s
        }// CAMERA MOVEMENT

        XMVECTOR forward= XMVector3Normalize(look_at - cam_coords);
        XMVECTOR right = XMVector3Normalize(XMVector3Cross(up, forward));
        //pitch rotation
        if (pitch!=0){
            XMMATRIX rot = XMMatrixRotationAxis(right, pitch/18);
            forward = XMVector3TransformNormal(forward, rot);
            up = XMVector3TransformNormal(up, rot);
        }
        //yaw rotation
        if (yaw != 0) {
            XMMATRIX rot = XMMatrixRotationAxis(up, yaw / 18);
            forward = XMVector3TransformNormal(forward, rot);
            right = XMVector3TransformNormal(right, rot);
        }
        if (g_Input.IsKeyDown('W')) {
            cam_coords += forward*4;
        }
        if (g_Input.IsKeyDown('S')) {
            cam_coords -= forward * 4;
        }
        if (g_Input.IsKeyDown('D')) {
            cam_coords += right * 4;
        }
        if (g_Input.IsKeyDown('A')) {
            cam_coords -= right * 4;
        }
        look_at = cam_coords + forward;
       auto currentTime = clock::now();
       float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
       lastTime = currentTime;
       time += deltaTime;
       time = std::fmod(time, 4.0f);
       g_Renderer.RenderFrame(mesh, time, cam_coords, look_at, up);
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
    Model mesh("sponza.obj"); // загружаем модель и материалы и текстуры и вообще пздц
    g_Renderer.Initialize(width,height,2, g_Window.GetHWND(),mesh, cam_coords, look_at, up, light_coords);

    int messege = Run(mesh);
    return static_cast<int>(messege);
}