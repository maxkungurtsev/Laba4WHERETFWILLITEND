#include "NEWRenderer.h"

NewRenderer::NewRenderer(UINT width, UINT height, int frame_count, Window* hwnd, std::string mesh_path, XMVECTOR cam_pos, XMVECTOR look_at, XMVECTOR up, int time) {
    frame_count_ = frame_count;
	//device, cmd, fence, heaps, viewport, scissor
	device_ = std::make_shared<Gdevice>(width, height, 2048+frame_count);
    swap_chain_=hwnd->CreateSwapChain(device_);
    back_buffer_ = std::make_shared<BackBuffer>(frame_count, swap_chain_, device_);
    render_system_ = std::make_shared<RenderingSystem>(device_,mesh_path, cam_pos, look_at, up, time);
    
}
void NewRenderer::RenderFrame(float time, XMVECTOR look_at, XMVECTOR cam_pos, XMVECTOR up) {
    device_->cmd_->ResetAllocator();
    D3D12_RESOURCE_BARRIER barriersBegin[2];
    UINT barrierCount = 0;
    //back buffer: PRESENT -> RESOLVE_DEST
    barriersBegin[barrierCount++] = {
        D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
        D3D12_RESOURCE_BARRIER_FLAG_NONE,
        {
            back_buffer_->GetCurrentBackBuffer().Get(),
            D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
            D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_RENDER_TARGET
        }
    };
    device_->cmd_->command_list_->ResourceBarrier(barrierCount, barriersBegin);

    //Viewport / Scissor
    device_->cmd_->command_list_->RSSetViewports(1, &device_->viewport_);
    device_->cmd_->command_list_->RSSetScissorRects(1, &device_->scissor_rect_);



    //rtv
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = back_buffer_->GetCurrentHandle().cpu_;
    //fill cbv
    //renderframe
    render_system_->RenderFrame(time, look_at, cam_pos, up, rtvHandle);

    D3D12_RESOURCE_BARRIER toPresent{};
    toPresent.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    toPresent.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    toPresent.Transition.pResource = back_buffer_->GetCurrentBackBuffer().Get();
    toPresent.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    toPresent.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    toPresent.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    device_->cmd_->command_list_->ResourceBarrier(1, &toPresent);

    //close cmd
    device_->cmd_->command_list_->Close();
    ID3D12CommandList* lists[] = { device_->cmd_->command_list_.Get() };
    device_->cmd_->command_queue_->ExecuteCommandLists(1, lists);
    
    //Present
    swap_chain_->Present(1, 0);
    back_buffer_->SetCurrentBackBuffer();

    //Fence
    device_->fence_->IncrementFenceValue();
    device_->cmd_->command_queue_->Signal(device_->fence_->GetFence().Get(), device_->fence_->GetFenceValue());

    if (device_->fence_->GetFence()->GetCompletedValue() < device_->fence_->GetFenceValue()) {
        HANDLE eventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        device_->fence_->GetFence()->SetEventOnCompletion(device_->fence_->GetFenceValue(), eventHandle);
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }
}