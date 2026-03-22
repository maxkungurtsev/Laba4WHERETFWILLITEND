#pragma once
#include <windows.h>
#include<iostream>
#include "Graphics/Gdevice.h"
class Window
{
private:
    ComPtr<IDXGISwapChain3> swap_chain_;
    HWND m_hWnd = nullptr;
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
public:
    bool Create(HINSTANCE hInstance, int nCmdShow);
    HWND GetHWND() const { return m_hWnd; }
    void CreateSwapChain(std::shared_ptr<Gdevice> device);
};
