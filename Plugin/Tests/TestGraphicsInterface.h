#pragma once
#include <cstdio>
#include <vector>
#include <array>

#define WindowWidth 320
#define WindowHeight 240

enum class TestType
{
    D3D9,
    D3D11,
    D3D12,
    OpenGL,
    Vulkan,
};

class TestImpl
{
public:
    virtual ~TestImpl() {}
    virtual TestType getType() const = 0;
    virtual void* getDevice() const = 0;

    virtual void onInit(void *hwnd) = 0;
    virtual void testMain();
};

TestImpl* CreateTestD3D9();
TestImpl* CreateTestD3D11();
TestImpl* CreateTestD3D12();
TestImpl* CreateTestOpenGL();
TestImpl* CreateTestVulkan();
