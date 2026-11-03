#pragma once
#include <safetyhook.hpp>
#include <intrin.h>
#include <cstring>
#include <vector>
#include <algorithm>

// DXVK creates its graphics objects before our first EndScene. Route only its
// creation requests through OpenXR enable2, preserving the original create infos.
// Other Vulkan clients and the runtime itself keep the native loader functions.
namespace VRRuntimeVulkan {
struct Creation {
    XrInstance xr=XR_NULL_HANDLE;
    XrSystemId system=XR_NULL_SYSTEM_ID;
    PFN_xrCreateVulkanInstanceKHR createInstance=nullptr;
    PFN_xrCreateVulkanDeviceKHR createDevice=nullptr;
    PFN_xrGetVulkanGraphicsDevice2KHR getPhysical=nullptr;
    PFN_vkGetInstanceProcAddr nativeProc=nullptr;
    PFN_vkCreateDevice nativeDevice=nullptr;
    VkInstance graphicsInstance=VK_NULL_HANDLE;
    VkPhysicalDevice physical=VK_NULL_HANDLE;
    VkDevice graphicsDevice=VK_NULL_HANDLE;
    struct GraphicsObjects {VkInstance instance;VkPhysicalDevice physical;VkDevice device;};
    std::vector<GraphicsObjects> ownedDevices;
    PFN_vkDestroyDevice nativeDestroyDevice=nullptr;
    PFN_vkDestroyInstance nativeDestroyInstance=nullptr;
    XrResult result=XR_SUCCESS;
    VkResult graphicsResult=VK_SUCCESS;
    const char* problem="";
    SRWLOCK lock=SRWLOCK_INIT;
    safetyhook::InlineHook resolver{};
    HMODULE library=nullptr; // Kept while DXVK may hold returned callbacks.
    void (*trace)(const char*,void*,bool)=nullptr;
    static inline Creation* owner=nullptr;

    static bool IsDxvkCaller(void* caller) {
        HMODULE module=nullptr;
        return GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCWSTR>(caller),&module) && GetProcAddress(module,"Direct3DCreate9") && GetProcAddress(module,"Direct3DCreate9Ex");
    }
    static PFN_vkVoidFunction VKAPI_CALL NativeProc(VkInstance instance,const char* name) {
        auto* self=owner;
        return self->resolver ? self->resolver.original<PFN_vkGetInstanceProcAddr>()(instance,name):self->nativeProc(instance,name);
    }
    static PFN_vkVoidFunction VKAPI_CALL Resolve(VkInstance instance,const char* name) {
        const auto original=NativeProc(instance,name);
        if(original && name) {
            if(!std::strcmp(name,"vkCreateInstance")) {
                if(owner->trace)owner->trace("resolve instance",_ReturnAddress(),false);
                return reinterpret_cast<PFN_vkVoidFunction>(CreateInstance);
            }
            if(!std::strcmp(name,"vkCreateDevice")) {
                if(owner->trace)owner->trace("resolve device",_ReturnAddress(),false);
                return reinterpret_cast<PFN_vkVoidFunction>(CreateDevice);
            }
            if(!std::strcmp(name,"vkDestroyDevice"))return reinterpret_cast<PFN_vkVoidFunction>(DestroyDevice);
            if(!std::strcmp(name,"vkDestroyInstance"))return reinterpret_cast<PFN_vkVoidFunction>(DestroyInstance);
        }
        return original;
    }
    static VkResult VKAPI_CALL CreateInstance(const VkInstanceCreateInfo* info,const VkAllocationCallbacks* allocator,VkInstance* output) {
        auto* self=owner;
        auto native=reinterpret_cast<PFN_vkCreateInstance>(NativeProc(VK_NULL_HANDLE,"vkCreateInstance"));
        const bool dxvk=IsDxvkCaller(_ReturnAddress());
        if(self->trace)self->trace("create instance",_ReturnAddress(),dxvk);
        if(!dxvk)return native(info,allocator,output);
        return self->CreateDxvkInstance(info,allocator,output);
    }
    VkResult CreateDxvkInstance(const VkInstanceCreateInfo* info,const VkAllocationCallbacks* allocator,VkInstance* output) {
        auto* self=this;
        auto native=reinterpret_cast<PFN_vkCreateInstance>(NativeProc(VK_NULL_HANDLE,"vkCreateInstance"));
        AcquireSRWLockExclusive(&self->lock);
        if(self->xr && self->createInstance && !*self->problem) {
            XrVulkanInstanceCreateInfoKHR create{XR_TYPE_VULKAN_INSTANCE_CREATE_INFO_KHR};
            create.systemId=self->system;create.pfnGetInstanceProcAddr=NativeProc;
            create.vulkanCreateInfo=info;create.vulkanAllocator=allocator;
            self->result=self->createInstance(self->xr,&create,output,&self->graphicsResult);
            if(XR_SUCCEEDED(self->result) && self->graphicsResult==VK_SUCCESS) {
                self->graphicsInstance=*output;
                XrVulkanGraphicsDeviceGetInfoKHR get{XR_TYPE_VULKAN_GRAPHICS_DEVICE_GET_INFO_KHR};
                get.systemId=self->system;get.vulkanInstance=*output;
                self->result=self->getPhysical(self->xr,&get,&self->physical);
                if(XR_FAILED(self->result))self->problem="OpenXR could not select the DXVK graphics adapter";
                ReleaseSRWLockExclusive(&self->lock);return VK_SUCCESS;
            }
            self->problem="OpenXR Vulkan instance creation failed; native flat-screen creation retained";
        }
        ReleaseSRWLockExclusive(&self->lock);
        return native(info,allocator,output);
    }
    static VkResult VKAPI_CALL CreateDevice(VkPhysicalDevice gpu,const VkDeviceCreateInfo* info,
        const VkAllocationCallbacks* allocator,VkDevice* output) {
        auto* self=owner;
        auto native=self->nativeDevice;
        const bool dxvk=IsDxvkCaller(_ReturnAddress());
        if(self->trace)self->trace("create device",_ReturnAddress(),dxvk);
        if(!dxvk)return native(gpu,info,allocator,output);
        return self->CreateDxvkDevice(gpu,info,allocator,output);
    }
    VkResult CreateDxvkDevice(VkPhysicalDevice gpu,const VkDeviceCreateInfo* info,const VkAllocationCallbacks* allocator,VkDevice* output) {
        auto* self=this;auto native=self->nativeDevice;
        AcquireSRWLockExclusive(&self->lock);
        if(self->xr && self->graphicsInstance && !*self->problem) {
            if(gpu==self->physical) {
                XrVulkanDeviceCreateInfoKHR create{XR_TYPE_VULKAN_DEVICE_CREATE_INFO_KHR};
                create.systemId=self->system;create.pfnGetInstanceProcAddr=NativeProc;
                create.vulkanPhysicalDevice=gpu;create.vulkanCreateInfo=info;create.vulkanAllocator=allocator;
                self->result=self->createDevice(self->xr,&create,output,&self->graphicsResult);
                if(XR_SUCCEEDED(self->result) && self->graphicsResult==VK_SUCCESS) {
                    self->graphicsDevice=*output;
                    self->ownedDevices.push_back({self->graphicsInstance,gpu,*output});
                    ReleaseSRWLockExclusive(&self->lock);return VK_SUCCESS;
                }
                self->problem="OpenXR Vulkan device creation failed; native flat-screen creation retained";
            } else self->problem="DXVK selected a different GPU from the OpenXR runtime";
        }
        ReleaseSRWLockExclusive(&self->lock);
        return native(gpu,info,allocator,output);
    }
    void ForgetDevice(VkDevice device) {
        AcquireSRWLockExclusive(&lock);
        std::erase_if(ownedDevices,[&](const auto& objects){return objects.device==device;});
        ReleaseSRWLockExclusive(&lock);
    }
    void ForgetInstance(VkInstance instance) {
        AcquireSRWLockExclusive(&lock);
        std::erase_if(ownedDevices,[&](const auto& objects){return objects.instance==instance;});
        if(graphicsInstance==instance) {graphicsInstance=VK_NULL_HANDLE;physical=VK_NULL_HANDLE;graphicsDevice=VK_NULL_HANDLE;}
        ReleaseSRWLockExclusive(&lock);
    }
    static void VKAPI_CALL DestroyDevice(VkDevice device,const VkAllocationCallbacks* allocator) {
        auto* self=owner;
        // Runtime-internal creation/cleanup can occur inside an enable2 call
        // while our lock is held. Only DXVK owns records in this collection.
        if(IsDxvkCaller(_ReturnAddress()))self->ForgetDevice(device);
        self->nativeDestroyDevice(device,allocator);
    }
    static void VKAPI_CALL DestroyInstance(VkInstance instance,const VkAllocationCallbacks* allocator) {
        auto* self=owner;
        if(IsDxvkCaller(_ReturnAddress()))self->ForgetInstance(instance);
        self->nativeDestroyInstance(instance,allocator);
    }
    bool Install(XrInstance instance,XrSystemId id,PFN_xrGetInstanceProcAddr get) {
        xr=instance;system=id;
        if(XR_FAILED(get(xr,"xrCreateVulkanInstanceKHR",reinterpret_cast<PFN_xrVoidFunction*>(&createInstance))) ||
            XR_FAILED(get(xr,"xrCreateVulkanDeviceKHR",reinterpret_cast<PFN_xrVoidFunction*>(&createDevice))) ||
            XR_FAILED(get(xr,"xrGetVulkanGraphicsDevice2KHR",reinterpret_cast<PFN_xrVoidFunction*>(&getPhysical))) ||
            !createInstance || !createDevice || !getPhysical)return false;
        library=LoadLibraryW(L"vulkan-1.dll");if(!library)return false;
        nativeProc=reinterpret_cast<PFN_vkGetInstanceProcAddr>(GetProcAddress(library,"vkGetInstanceProcAddr"));
        nativeDevice=reinterpret_cast<PFN_vkCreateDevice>(GetProcAddress(library,"vkCreateDevice"));
        nativeDestroyDevice=reinterpret_cast<PFN_vkDestroyDevice>(GetProcAddress(library,"vkDestroyDevice"));
        nativeDestroyInstance=reinterpret_cast<PFN_vkDestroyInstance>(GetProcAddress(library,"vkDestroyInstance"));
        if(!nativeProc || !nativeDevice || !nativeDestroyDevice || !nativeDestroyInstance)return false;
        owner=this;resolver=safetyhook::create_inline(nativeProc,Resolve);
        return bool(resolver);
    }
    void Disable() {
        AcquireSRWLockExclusive(&lock);xr=XR_NULL_HANDLE;ReleaseSRWLockExclusive(&lock);
        // Do not retire trampolines that another graphics client may retain.
    }
    bool Matches(VkInstance instance,VkPhysicalDevice gpu,VkDevice device) {
        AcquireSRWLockShared(&lock);
        // A launcher/overlay may create another D3D9 device through DXVK before
        // the game's first EndScene. It must not replace the game's ownership.
        const bool matches=std::any_of(ownedDevices.begin(),ownedDevices.end(),[&](const auto& objects){
            return objects.instance==instance && objects.physical==gpu && objects.device==device;
        });
        if(trace) {
            trace("interop device",reinterpret_cast<void*>(device),matches);
            trace("last created device",reinterpret_cast<void*>(graphicsDevice),matches);
        }
        ReleaseSRWLockShared(&lock);return matches;
    }
};
}
