#pragma once
// Public DXVK v3.1.1 D3D9 interop ABI, tested with the installed x86 build.
// GPU resources have explicit render-thread cleanup; no loader-lock destructor.
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan_core.h>
#include <d3d9.h>
#include <wrl/client.h>

namespace DxvkVulkan {
template<class F> F Method(IUnknown* object,unsigned index) {
    return reinterpret_cast<F>((*reinterpret_cast<void***>(object))[index]);
}
inline constexpr GUID deviceId{0x2eaa4b89,0x0107,0x4bdb,{0x87,0xf7,0x0f,0x54,0x1c,0x49,0x3c,0xe0}};
inline constexpr GUID textureId{0xd56344f5,0x8d35,0x46fd,{0x80,0x6d,0x94,0xc3,0x51,0xb4,0x72,0xc1}};
struct Bridge {
    Microsoft::WRL::ComPtr<IUnknown> interop;
    Microsoft::WRL::ComPtr<IDirect3DTexture9> textures[2];
    Microsoft::WRL::ComPtr<IUnknown> textureInterop[2];
    VkImage sources[2]{};
    VkImageLayout layouts[2]{};
    VkInstance instance{};
    VkPhysicalDevice physical{};
    VkDevice device{};
    VkQueue queue{};
    uint32_t family=0,index=0,width=0,height=0;
    VkCommandPool pools[2]{};
    VkCommandBuffer commands[2]{};
    VkFence fences[2]{};
    bool pending[2]{};
    HMODULE loader=nullptr;
    PFN_vkGetInstanceProcAddr instanceProc=nullptr;
    PFN_vkGetDeviceProcAddr deviceProc=nullptr;
    // OpenXR begin/end frame and acquire/release image may also use this queue.
    // Synchronize them with DXVK's asynchronous submission thread, not just our
    // vkQueueSubmit. Never call D3D9 or nest this guard inside the callback.
    template<class F> auto WithQueue(F&& action) {
        using Action=void(STDMETHODCALLTYPE*)(IUnknown*);
        Method<Action>(interop.Get(),7)(interop.Get());
        struct Unlock {
            IUnknown* owner;
            ~Unlock() {Method<Action>(owner,8)(owner);}
        } unlock{interop.Get()};
        return action();
    }
#define BRIDGE_FUNCTIONS(X) X(CreateCommandPool) X(DestroyCommandPool) X(AllocateCommandBuffers) \
    X(ResetCommandPool) X(BeginCommandBuffer) X(EndCommandBuffer) X(CreateFence) \
    X(DestroyFence) X(ResetFences) X(WaitForFences) X(CmdPipelineBarrier) X(CmdCopyImage) X(QueueSubmit)
#define DECLARE_BRIDGE(name) PFN_vk##name name=nullptr;
    BRIDGE_FUNCTIONS(DECLARE_BRIDGE)
#undef DECLARE_BRIDGE
    bool Attach(IDirect3DDevice9* d3d) {
        if(!d3d || FAILED(d3d->QueryInterface(deviceId,reinterpret_cast<void**>(interop.GetAddressOf())))) return false;
        using Handles=void(STDMETHODCALLTYPE*)(IUnknown*,VkInstance*,VkPhysicalDevice*,VkDevice*);
        using Queue=void(STDMETHODCALLTYPE*)(IUnknown*,VkQueue*,uint32_t*,uint32_t*);
        Method<Handles>(interop.Get(),3)(interop.Get(),&instance,&physical,&device);
        Method<Queue>(interop.Get(),4)(interop.Get(),&queue,&index,&family);
        if(!instance || !physical || !device || !queue) return false;
        loader=LoadLibraryW(L"vulkan-1.dll");
        if(!loader) return false;
        instanceProc=reinterpret_cast<PFN_vkGetInstanceProcAddr>(GetProcAddress(loader,"vkGetInstanceProcAddr"));
        if(!instanceProc) return false;
        deviceProc=reinterpret_cast<PFN_vkGetDeviceProcAddr>(instanceProc(instance,"vkGetDeviceProcAddr"));
        if(!deviceProc) return false;
#define LOAD_BRIDGE(name) name=reinterpret_cast<PFN_vk##name>(deviceProc(device,"vk" #name)); if(!name) return false;
        BRIDGE_FUNCTIONS(LOAD_BRIDGE)
#undef LOAD_BRIDGE
        for(unsigned eye=0;eye<2;++eye) {
            VkCommandPoolCreateInfo create{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO}; create.queueFamilyIndex=family;
            if(CreateCommandPool(device,&create,nullptr,&pools[eye])!=VK_SUCCESS) return false;
            VkCommandBufferAllocateInfo allocation{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
            allocation.commandPool=pools[eye]; allocation.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY; allocation.commandBufferCount=1;
            if(AllocateCommandBuffers(device,&allocation,&commands[eye])!=VK_SUCCESS) return false;
            VkFenceCreateInfo fenceInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
            if(CreateFence(device,&fenceInfo,nullptr,&fences[eye])!=VK_SUCCESS) return false;
        }
        return true;
    }
    bool SetTextures(IDirect3DTexture9* const (&images)[2],unsigned w,unsigned h) {
        using Info=HRESULT(STDMETHODCALLTYPE*)(IUnknown*,VkImage*,VkImageLayout*,VkImageCreateInfo*);
        for(unsigned eye=0;eye<2;++eye) {
            if(!images[eye]) return false;
            textures[eye]=images[eye];
            textureInterop[eye].Reset();
            if(FAILED(images[eye]->QueryInterface(textureId,reinterpret_cast<void**>(textureInterop[eye].GetAddressOf())))) return false;
            VkImageCreateInfo description{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
            if(FAILED(Method<Info>(textureInterop[eye].Get(),3)(textureInterop[eye].Get(),&sources[eye],&layouts[eye],&description))) return false;
            if(!sources[eye] || description.format!=VK_FORMAT_B8G8R8A8_UNORM ||
                description.extent.width!=w || description.extent.height!=h || description.extent.depth!=1 ||
                description.samples!=VK_SAMPLE_COUNT_1_BIT || !(description.usage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)) return false;
        }
        width=w; height=h; return true;
    }
    void Transition(unsigned eye,VkImageLayout from,VkImageLayout to) {
        using F=void(STDMETHODCALLTYPE*)(IUnknown*,IUnknown*,const VkImageSubresourceRange*,VkImageLayout,VkImageLayout);
        const VkImageSubresourceRange range{VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1};
        Method<F>(interop.Get(),5)(interop.Get(),textureInterop[eye].Get(),&range,from,to);
    }
    void Flush() {
        using F=void(STDMETHODCALLTYPE*)(IUnknown*);
        Method<F>(interop.Get(),6)(interop.Get());
    }
    bool Wait(unsigned eye) {
        if(pending[eye] && WaitForFences(device,1,&fences[eye],VK_TRUE,1'000'000'000)!=VK_SUCCESS) return false;
        pending[eye]=false; return true;
    }
    // Submit this frame's rendering, then wait for the previous frame's copies.
    // Call once per frame before Copy; running further ahead varies the pose age.
    bool WaitPrevious() {Flush(); return Wait(0) && Wait(1);}
    // Not waited on here: the barriers order the copy after this frame's rendering
    // and before later work, and OpenXR only needs it submitted to this queue.
    bool Copy(unsigned eye,VkImage target) {
        if(eye>1 || !target || !sources[eye] || !Wait(eye)) return false;
        // DXVK can relocate a resource's Vulkan backing storage. Keeping the
        // D3D9 texture alive does not pin the VkImage obtained on the first
        // frame. Resolve it after pending producer commands, for every copy.
        Flush();
        using Info=HRESULT(STDMETHODCALLTYPE*)(IUnknown*,VkImage*,VkImageLayout*,VkImageCreateInfo*);
        if(FAILED(Method<Info>(textureInterop[eye].Get(),3)(textureInterop[eye].Get(),
            &sources[eye],&layouts[eye],nullptr)) || !sources[eye]) return false;
        Transition(eye,layouts[eye],VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        Flush();
        const auto restore=[&](){Transition(eye,VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,layouts[eye]);Flush();};
        const auto command=commands[eye];
        if(ResetCommandPool(device,pools[eye],0)!=VK_SUCCESS || ResetFences(device,1,&fences[eye])!=VK_SUCCESS) {restore();return false;}
        VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO}; begin.flags=VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        if(BeginCommandBuffer(command,&begin)!=VK_SUCCESS) {restore();return false;}
        VkMemoryBarrier frameWrites{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
        frameWrites.srcAccessMask=VK_ACCESS_MEMORY_WRITE_BIT;
        frameWrites.dstAccessMask=VK_ACCESS_TRANSFER_READ_BIT|VK_ACCESS_TRANSFER_WRITE_BIT;
        VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        barrier.srcQueueFamilyIndex=barrier.dstQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED;
        barrier.image=target; barrier.subresourceRange={VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1};
        barrier.oldLayout=VK_IMAGE_LAYOUT_UNDEFINED; barrier.newLayout=VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.dstAccessMask=VK_ACCESS_TRANSFER_WRITE_BIT;
        CmdPipelineBarrier(command,VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,VK_PIPELINE_STAGE_TRANSFER_BIT,0,1,&frameWrites,0,nullptr,1,&barrier);
        VkImageCopy region{}; region.srcSubresource={VK_IMAGE_ASPECT_COLOR_BIT,0,0,1};
        region.dstSubresource=region.srcSubresource; region.extent={width,height,1};
        CmdCopyImage(command,sources[eye],VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,target,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,1,&region);
        barrier.oldLayout=VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL; barrier.newLayout=VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        barrier.srcAccessMask=VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask=VK_ACCESS_COLOR_ATTACHMENT_READ_BIT|VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        CmdPipelineBarrier(command,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,0,0,nullptr,0,nullptr,1,&barrier);
        if(EndCommandBuffer(command)!=VK_SUCCESS) {restore();return false;}
        VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO}; submit.commandBufferCount=1; submit.pCommandBuffers=&command;
        const auto submitted=WithQueue([&](){return QueueSubmit(queue,1,&submit,fences[eye]);});
        if(submitted!=VK_SUCCESS) {restore();return false;}
        pending[eye]=true;
        restore(); return true;
    }
    bool Destroy() {
        for(unsigned eye=0;eye<2;++eye) {
            if(!pending[eye]) continue;
            const auto done=WaitForFences(device,1,&fences[eye],VK_TRUE,1'000'000'000);
            if(done!=VK_SUCCESS && done!=VK_ERROR_DEVICE_LOST) return false;
            pending[eye]=false;
        }
        for(unsigned eye=0;eye<2;++eye) {
            if(fences[eye]) DestroyFence(device,fences[eye],nullptr);
            if(pools[eye]) DestroyCommandPool(device,pools[eye],nullptr);
            fences[eye]=VK_NULL_HANDLE; pools[eye]=VK_NULL_HANDLE; commands[eye]=VK_NULL_HANDLE;
        }
        for(unsigned i=0;i<2;++i) {textureInterop[i].Reset();textures[i].Reset();sources[i]=0;}
        interop.Reset(); instance=nullptr; physical=nullptr; device=nullptr; queue=nullptr;
        if(loader) FreeLibrary(loader); loader=nullptr;
        return true;
    }
};
#undef BRIDGE_FUNCTIONS
} // namespace DxvkVulkan
