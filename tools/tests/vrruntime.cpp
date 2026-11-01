#define WIN32_LEAN_AND_MEAN
#define VK_NO_PROTOTYPES
#define XR_USE_GRAPHICS_API_VULKAN
#include <windows.h>
#include <vulkan/vulkan_core.h>
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include "../../source/includes/vrruntimevulkan.hpp"
#include "../../source/includes/vrcontrollerbindings.hpp"
#include <cassert>
#include <cstdio>

namespace {
int nativeInstances=0,nativeDevices=0,xrInstances=0,xrDevices=0;
bool fail=false;
const VkInstanceCreateInfo* seenInstance=nullptr;
const VkDeviceCreateInfo* seenDevice=nullptr;
VkResult VKAPI_CALL NativeInstance(const VkInstanceCreateInfo*,const VkAllocationCallbacks*,VkInstance* out) {
    ++nativeInstances;*out=reinterpret_cast<VkInstance>(1);return VK_SUCCESS;
}
VkResult VKAPI_CALL NativeDevice(VkPhysicalDevice,const VkDeviceCreateInfo*,const VkAllocationCallbacks*,VkDevice* out) {
    ++nativeDevices;*out=reinterpret_cast<VkDevice>(3);return VK_SUCCESS;
}
PFN_vkVoidFunction VKAPI_CALL Proc(VkInstance,const char* name) {
    if(!std::strcmp(name,"vkCreateInstance"))return reinterpret_cast<PFN_vkVoidFunction>(NativeInstance);
    return nullptr;
}
XrResult XRAPI_CALL Instance(XrInstance,const XrVulkanInstanceCreateInfoKHR* info,VkInstance* out,VkResult* result) {
    ++xrInstances;seenInstance=info->vulkanCreateInfo;
    if(fail){*result=VK_ERROR_INITIALIZATION_FAILED;return XR_ERROR_RUNTIME_FAILURE;}
    assert(info->pfnGetInstanceProcAddr(VK_NULL_HANDLE,"vkCreateInstance")==reinterpret_cast<PFN_vkVoidFunction>(NativeInstance));
    *out=reinterpret_cast<VkInstance>(1);*result=VK_SUCCESS;return XR_SUCCESS;
}
XrResult XRAPI_CALL Physical(XrInstance,const XrVulkanGraphicsDeviceGetInfoKHR*,VkPhysicalDevice* out) {
    *out=reinterpret_cast<VkPhysicalDevice>(2);return XR_SUCCESS;
}
XrResult XRAPI_CALL Device(XrInstance,const XrVulkanDeviceCreateInfoKHR* info,VkDevice* out,VkResult* result) {
    ++xrDevices;seenDevice=info->vulkanCreateInfo;
    *out=reinterpret_cast<VkDevice>(3);*result=VK_SUCCESS;return XR_SUCCESS;
}
}
int main() {
    VRRuntimeVulkan::Creation creation;
    VRRuntimeVulkan::Creation::owner=&creation;
    creation.nativeProc=Proc;creation.nativeDevice=NativeDevice;
    creation.xr=static_cast<XrInstance>(1);creation.system=1;
    creation.createInstance=Instance;creation.createDevice=Device;creation.getPhysical=Physical;
    VkInstanceCreateInfo instance{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    VkDeviceCreateInfo device{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    VkInstance vkInstance{};VkDevice vkDevice{};
    assert(creation.CreateDxvkInstance(&instance,nullptr,&vkInstance)==VK_SUCCESS);
    assert(seenInstance==&instance && nativeInstances==0 && xrInstances==1);
    assert(creation.CreateDxvkDevice(creation.physical,&device,nullptr,&vkDevice)==VK_SUCCESS);
    assert(seenDevice==&device && nativeDevices==0 && xrDevices==1);
    assert(creation.Matches(vkInstance,creation.physical,vkDevice));
    // A different adapter must keep flat-screen creation and reject VR use.
    assert(creation.CreateDxvkDevice(reinterpret_cast<VkPhysicalDevice>(4),&device,nullptr,&vkDevice)==VK_SUCCESS);
    assert(nativeDevices==1 && xrDevices==1 && !creation.Matches(vkInstance,creation.physical,vkDevice));
    creation.problem="";fail=true;
    assert(creation.CreateDxvkInstance(&instance,nullptr,&vkInstance)==VK_SUCCESS);
    assert(nativeInstances==1 && *creation.problem);
    creation.Disable();fail=false;creation.problem="";
    assert(creation.CreateDxvkInstance(&instance,nullptr,&vkInstance)==VK_SUCCESS && nativeInstances==2);
    using namespace VRControllerBindings;
    assert(Component(Profile::Vive,"jump",1,"/input/trackpad/click","on_foot")=="/input/trackpad/click");
    assert(Component(Profile::Touch,"move",0,"/input/trackpad","on_foot")=="/input/thumbstick");
    assert(Component(Profile::Touch,"move_enable",0,"/input/trackpad/touch","on_foot").empty());
    assert(Component(Profile::Touch,"select",1,"/input/trigger/click","menus")=="/input/trigger/value");
    assert(Component(Profile::Touch,"interact",1,"/input/menu/click","on_foot")=="/input/b/click");
    assert(Component(Profile::Touch,"confirm",1,"","driving")=="/input/a/click");
    assert(Component(Profile::Index,"pause",0,"/input/menu/click","system")=="/input/b/click");
    assert(Component(Profile::Index,"jump",1,"/input/trackpad/click","on_foot")=="/input/thumbstick/click");
    assert(Component(Profile::Motion,"select",1,"/input/trigger/click","menus")=="/input/trigger/value");
    puts("OpenXR Vulkan creation, fallback, adapter ownership and controller defaults passed");
}
