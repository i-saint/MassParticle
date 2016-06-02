#include "TestGraphicsDevice.h"

#ifdef _WIN32
    #define WithD3D9
    #define WithD3D11
    #define WithD3D12
    #define WithOpenGL
    #define WithVulkan

    #include <wrl.h>
    using Microsoft::WRL::ComPtr;
#else
#endif




#ifdef WithD3D9
#include <d3d9.h>
#include <dxgi.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "dxgi.lib")

class TestImplD3D9 : public TestImpl
{
public:
    ~TestImplD3D9() override;
    TestType getType() const override { return TestType::D3D9; }
    void* getDevice() const override { return m_device.Get(); }
    void onInit(void *hwnd) override;

private:
    ComPtr<IDirect3D9> m_d3d9;
    ComPtr<IDirect3DDevice9> m_device;
};

TestImplD3D9::~TestImplD3D9()
{
}

void TestImplD3D9::onInit(void *hwnd)
{
    m_d3d9 = ::Direct3DCreate9(D3D_SDK_VERSION);

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = TRUE;
    d3dpp.hDeviceWindow = (HWND)hwnd;
    d3dpp.BackBufferCount = 1;
    d3dpp.BackBufferWidth = WindowWidth;
    d3dpp.BackBufferHeight = WindowHeight;
    d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    m_d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, (HWND)hwnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &m_device);
}

TestImpl* CreateTestD3D9() { return new TestImplD3D9(); }
#else // WithD3D9
TestImpl* CreateTestD3D9() { return nullptr; }
#endif // WithD3D9



#ifdef WithD3D11
#include <d3d11.h>
#include <dxgi.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

class TestImplD3D11 : public TestImpl
{
public:
    ~TestImplD3D11() override;
    TestType getType() const override { return TestType::D3D11; }
    void* getDevice() const override { return m_device.Get(); }
    void onInit(void *hwnd) override;

private:
    ComPtr<IDXGISwapChain> m_swapchain = nullptr;
    ComPtr<ID3D11Device> m_device = nullptr;
    ComPtr<ID3D11DeviceContext> m_context = nullptr;
};

TestImplD3D11::~TestImplD3D11()
{
}

void TestImplD3D11::onInit(void *hwnd)
{
    D3D_FEATURE_LEVEL feature_levels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };
    D3D_FEATURE_LEVEL valid_feature_level;

    DXGI_SWAP_CHAIN_DESC sc_desc;
    ZeroMemory(&sc_desc, sizeof(sc_desc));
    sc_desc.BufferCount = 1;
    sc_desc.BufferDesc.Width  = WindowWidth;
    sc_desc.BufferDesc.Height = WindowHeight;
    sc_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    sc_desc.BufferDesc.RefreshRate.Numerator = 60;
    sc_desc.BufferDesc.RefreshRate.Denominator = 1;
    sc_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sc_desc.OutputWindow = (HWND)hwnd;
    sc_desc.SampleDesc.Count = 1;
    sc_desc.SampleDesc.Quality = 0;
    sc_desc.Windowed = TRUE;

    IDXGIAdapter *adapter = nullptr;
    HRESULT	hr = D3D11CreateDeviceAndSwapChain(
        adapter,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        feature_levels,
        _countof(feature_levels),
        D3D11_SDK_VERSION,
        &sc_desc,
        &m_swapchain,
        &m_device,
        &valid_feature_level,
        &m_context);
}

TestImpl* CreateTestD3D11() { return new TestImplD3D11(); }
#else // WithD3D11
TestImpl* CreateTestD3D11() { return nullptr; }
#endif // WithD3D11



#ifdef WithD3D12
#include <d3d12.h>
#include <dxgi1_4.h>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

class TestImplD3D12 : public TestImpl
{
public:
    ~TestImplD3D12() override;
    TestType getType() const override { return TestType::D3D12; }
    void* getDevice() const override { return m_device.Get(); }
    void onInit(void *hwnd) override;

private:
    ComPtr<ID3D12Device> m_device;
    ComPtr<ID3D12CommandQueue> m_queue;
    ComPtr<IDXGISwapChain1> m_swapchain;
};

TestImplD3D12::~TestImplD3D12()
{
}

void TestImplD3D12::onInit(void *hwnd)
{
    ComPtr<IDXGIFactory4> factory;
    CreateDXGIFactory1(IID_PPV_ARGS(&factory));

    IDXGIAdapter1 *adapter;
    for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex) {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
            continue;
        }

        if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)))) {
            break;
        }
    }

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_queue));

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = 60;
    swapChainDesc.Width = WindowWidth;
    swapChainDesc.Height = WindowHeight;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    factory->CreateSwapChainForHwnd(
        m_queue.Get(), // Swap chain needs the queue so that it can force a flush on it.
        (HWND)hwnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &m_swapchain
    );

    // This sample does not support fullscreen transitions.
    factory->MakeWindowAssociation((HWND)hwnd, DXGI_MWA_NO_ALT_ENTER);
}

TestImpl* CreateTestD3D12() { return new TestImplD3D12(); }
#else // WithD3D12
TestImpl* CreateTestD3D12() { return nullptr; }
#endif // WithD3D12



#ifdef WithOpenGL
#include <GL/GL.h>

class TestImplOpenGL : public TestImpl
{
public:
    ~TestImplOpenGL() override;
    TestType getType() const override { return TestType::OpenGL; }
    void* getDevice() const override { return nullptr; }
    void onInit(void *hwnd) override;

private:
    HWND m_hwnd = nullptr;
    HDC m_hdc = nullptr;
    HGLRC m_hglrc = nullptr;
};

TestImplOpenGL::~TestImplOpenGL()
{
    if (m_hglrc != nullptr) {
        ::wglMakeCurrent(nullptr, nullptr);
        ::wglDeleteContext(m_hglrc);
        m_hglrc = nullptr;
    }
    if (m_hdc != nullptr) {
        ::ReleaseDC(m_hwnd, m_hdc);
        m_hdc = nullptr;
    }
}

void TestImplOpenGL::onInit(void *hwnd)
{
    static PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),    //この構造体のサイズ
        1,                  //OpenGLバージョン
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,       //ダブルバッファ使用可能
        PFD_TYPE_RGBA,      //RGBAカラー
        32,                 //色数
        0, 0,               //RGBAのビットとシフト設定        
        0, 0,                //G
        0, 0,                //B
        0, 0,                //A
        0,                  //アキュムレーションバッファ
        0, 0, 0, 0,         //RGBAアキュムレーションバッファ
        32,                 //Zバッファ    
        0,                  //ステンシルバッファ
        0,                  //使用しない
        PFD_MAIN_PLANE,     //レイヤータイプ
        0,                  //予約
        0, 0, 0             //レイヤーマスクの設定・未使用
    };

    m_hwnd = (HWND)hwnd;
    m_hdc = ::GetDC(m_hwnd);
    int pixelformat = ::ChoosePixelFormat(m_hdc, &pfd);
    ::SetPixelFormat(m_hdc, pixelformat, &pfd);
    m_hglrc = ::wglCreateContext(m_hdc);
    ::wglMakeCurrent(m_hdc, m_hglrc);
}

TestImpl* CreateTestOpenGL() { return new TestImplOpenGL(); }
#else // WithOpenGL
TestImpl* CreateTestOpenGL() { return nullptr; }
#endif // WithOpenGL



#ifdef WithVulkan
#ifdef _WIN32
    #define VK_USE_PLATFORM_WIN32_KHR
#endif // _WIN32
#include <vulkan/vulkan.h>
#pragma comment(lib, "vulkan-1.lib")

class TestImplVulkan : public TestImpl
{
public:
    ~TestImplVulkan() override;
    TestType getType() const override { return TestType::Vulkan; }
    void* getDevice() const override { return m_device; }
    void onInit(void *hwnd) override;

private:
    VkInstance m_instance = nullptr;
    VkPhysicalDevice m_physical_device = nullptr;
    VkDevice m_device = nullptr;
    VkSurfaceKHR m_surface = 0;
};

TestImplVulkan::~TestImplVulkan()
{
    if (m_surface) {
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        m_surface = 0;
    }
    if (m_device) {
        vkDestroyDevice(m_device, nullptr);
        m_device = nullptr;
    }
    if (m_instance) {
        vkDestroyInstance(m_instance, nullptr);
        m_instance = nullptr;
    }
}

void TestImplVulkan::onInit(void *hwnd)
{
    VkResult err;
    
    // initialize instance
    {
        VkApplicationInfo app_info = {};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "";
        app_info.pEngineName = "";
        app_info.apiVersion = VK_API_VERSION_1_0;

        std::vector<const char*> extensions = { VK_KHR_SURFACE_EXTENSION_NAME };
        // Enable surface extensions depending on os
#if defined(_WIN32)
        extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(__ANDROID__)
        extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(__linux__)
        extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif

        VkInstanceCreateInfo instance_info = {};
        instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_info.pNext = nullptr;
        instance_info.pApplicationInfo = &app_info;
        if (extensions.size() > 0) {
            instance_info.enabledExtensionCount = (uint32_t)extensions.size();
            instance_info.ppEnabledExtensionNames = extensions.data();
        }
        vkCreateInstance(&instance_info, nullptr, &m_instance);
    }

    // get physical device
    {
        uint32_t num_gpus = 0;
        vkEnumeratePhysicalDevices(m_instance, &num_gpus, nullptr);

        std::vector<VkPhysicalDevice> physical_devices(num_gpus);
        err = vkEnumeratePhysicalDevices(m_instance, &num_gpus, physical_devices.data());

        m_physical_device = physical_devices[0];
    }

    // initialize device
    {
        // Find a queue that supports graphics operations
        std::vector<VkQueueFamilyProperties> queue_props;
        uint32_t graphics_queue = 0;
        uint32_t num_queues = 0;

        vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &num_queues, nullptr);
        queue_props.resize(num_queues);
        vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &num_queues, queue_props.data());

        for (graphics_queue = 0; graphics_queue < num_queues; graphics_queue++) {
            if (queue_props[graphics_queue].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                break;
            }
        }

        // Vulkan device
        std::array<float, 1> queue_priorities = { 0.0f };
        VkDeviceQueueCreateInfo queue_info = {};
        queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info.queueFamilyIndex = graphics_queue;
        queue_info.queueCount = 1;
        queue_info.pQueuePriorities = queue_priorities.data();

        VkDeviceCreateInfo device_info = {};
        device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_info.pNext = nullptr;
        device_info.queueCreateInfoCount = 1;
        device_info.pQueueCreateInfos = &queue_info;
        device_info.pEnabledFeatures = nullptr;

        std::vector<const char*> extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
        device_info.enabledExtensionCount = (uint32_t)extensions.size();
        device_info.ppEnabledExtensionNames = extensions.data();

        vkCreateDevice(m_physical_device, &device_info, nullptr, &m_device);
    }


    // initialize surface
    {
#if defined(_WIN32)
        VkWin32SurfaceCreateInfoKHR surface_info = {};
        surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        surface_info.hinstance = (HINSTANCE)GetModuleHandleA(nullptr);
        surface_info.hwnd = (HWND)hwnd;
        vkCreateWin32SurfaceKHR(m_instance, &surface_info, nullptr, &m_surface);
#else
#endif
    }
}

TestImpl* CreateTestVulkan() { return new TestImplVulkan(); }
#else // WithVulkan
TestImpl* CreateTestVulkan() { return nullptr; }
#endif // WithVulkan
