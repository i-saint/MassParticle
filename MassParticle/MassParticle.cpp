// Example low level rendering Unity plugin


#include "UnityPluginInterface.h"

#include <math.h>
#include <stdio.h>
#include <tbb/tbb.h>
#include "mpTypes.h"
#include "MassParticle.h"

mpPerspectiveCamera g_camera;
mpWorld g_mpWorld;


// --------------------------------------------------------------------------
// Include headers for the graphics APIs we support

#if SUPPORT_D3D9
    #include <d3d9.h>
#endif
#if SUPPORT_D3D11
    #include <d3d11.h>
#endif
#if SUPPORT_OPENGL
    #if UNITY_WIN
        #include <gl/GL.h>
    #else
        #include <OpenGL/OpenGL.h>
    #endif
#endif



// --------------------------------------------------------------------------
// Helper utilities


// Prints a string
static void DebugLog (const char* str)
{
    #if UNITY_WIN
    OutputDebugStringA (str);
    #else
    printf ("%s", str);
    #endif
}

// COM-like Release macro
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(a) if (a) { a->Release(); a = NULL; }
#endif



// --------------------------------------------------------------------------
// SetTimeFromUnity, an example function we export which is called by one of the scripts.

static float g_Time;

extern "C" void EXPORT_API SetTimeFromUnity (float t) { g_Time = t; }



// --------------------------------------------------------------------------
// SetTextureFromUnity, an example function we export which is called by one of the scripts.

static void* g_TexturePointer;

extern "C" void EXPORT_API SetTextureFromUnity (void* texturePtr)
{
    // A script calls this at initialization time; just remember the texture pointer here.
    // Will update texture pixels each frame from the plugin rendering event (texture update
    // needs to happen on the rendering thread).
    g_TexturePointer = texturePtr;
}



// --------------------------------------------------------------------------
// UnitySetGraphicsDevice

static int g_DeviceType = -1;


// Actual setup/teardown functions defined below
#if SUPPORT_D3D9
static void SetGraphicsDeviceD3D9 (IDirect3DDevice9* device, GfxDeviceEventType eventType);
#endif
#if SUPPORT_D3D11
static void SetGraphicsDeviceD3D11 (ID3D11Device* device, GfxDeviceEventType eventType);
#endif


bool mpInitialize(ID3D11Device *dev);
void mpFinalize();
void mpRender();

extern "C" void EXPORT_API UnitySetGraphicsDevice (void* device, int deviceType, int eventType)
{
    // Set device type to -1, i.e. "not recognized by our plugin"
    g_DeviceType = -1;
    
    #if SUPPORT_D3D9
    // D3D9 device, remember device pointer and device type.
    // The pointer we get is IDirect3DDevice9.
    if (deviceType == kGfxRendererD3D9)
    {
        DebugLog ("Set D3D9 graphics device\n");
        g_DeviceType = deviceType;
        SetGraphicsDeviceD3D9 ((IDirect3DDevice9*)device, (GfxDeviceEventType)eventType);
    }
    #endif

    #if SUPPORT_D3D11
    // D3D11 device, remember device pointer and device type.
    // The pointer we get is ID3D11Device.
    if (deviceType == kGfxRendererD3D11)
    {
        if (eventType == kGfxDeviceEventInitialize) {
            DebugLog("Set D3D11 graphics device\n");
            g_DeviceType = deviceType;
            SetGraphicsDeviceD3D11((ID3D11Device*)device, (GfxDeviceEventType)eventType);
            mpInitialize((ID3D11Device*)device);
        }
        else if (eventType == kGfxDeviceEventShutdown) {
            mpFinalize();
        }
        else if (eventType == kGfxDeviceEventBeforeReset) {
            mpClearParticles();
        }
    }
    #endif

    #if SUPPORT_OPENGL
    // If we've got an OpenGL device, remember device type. There's no OpenGL
    // "device pointer" to remember since OpenGL always operates on a currently set
    // global context.
    if (deviceType == kGfxRendererOpenGL)
    {
        DebugLog ("Set OpenGL graphics device\n");
        g_DeviceType = deviceType;
    }
    #endif
}



// --------------------------------------------------------------------------
// UnityRenderEvent
// This will be called for GL.IssuePluginEvent script calls; eventID will
// be the integer passed to IssuePluginEvent. In this example, we just ignore
// that value.


struct MyVertex {
    float x, y, z;
    unsigned int color;
};
static void SetDefaultGraphicsState ();
static void DoRendering (const float* worldMatrix, const float* identityMatrix, float* projectionMatrix, const MyVertex* verts);


extern "C" void EXPORT_API UnityRenderEvent (int eventID)
{
    // Unknown graphics device type? Do nothing.
    if (g_DeviceType == -1)
        return;


    // A colored triangle. Note that colors will come out differently
    // in D3D9/11 and OpenGL, for example, since they expect color bytes
    // in different ordering.
    MyVertex verts[3] = {
        { -0.5f, -0.25f,  0, 0xFFff0000 },
        {  0.5f, -0.25f,  0, 0xFF00ff00 },
        {  0,     0.5f ,  0, 0xFF0000ff },
    };


    // Some transformation matrices: rotate around Z axis for world
    // matrix, identity view matrix, and identity projection matrix.

    float phi = g_Time;
    float cosPhi = cosf(phi);
    float sinPhi = sinf(phi);

    float worldMatrix[16] = {
        cosPhi,-sinPhi,0,0,
        sinPhi,cosPhi,0,0,
        0,0,1,0,
        0,0,0.7f,1,
    };
    float identityMatrix[16] = {
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1,
    };
    float projectionMatrix[16] = {
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1,
    };

    // Actual functions defined below
    SetDefaultGraphicsState ();
    DoRendering (worldMatrix, identityMatrix, projectionMatrix, verts);
}


// -------------------------------------------------------------------
//  Direct3D 9 setup/teardown code


#if SUPPORT_D3D9

static IDirect3DDevice9* g_D3D9Device;

// A dynamic vertex buffer just to demonstrate how to handle D3D9 device resets.
static IDirect3DVertexBuffer9* g_D3D9DynamicVB;

static void SetGraphicsDeviceD3D9 (IDirect3DDevice9* device, GfxDeviceEventType eventType)
{
    g_D3D9Device = device;

    // Create or release a small dynamic vertex buffer depending on the event type.
    switch (eventType) {
    case kGfxDeviceEventInitialize:
    case kGfxDeviceEventAfterReset:
        // After device is initialized or was just reset, create the VB.
        if (!g_D3D9DynamicVB)
            g_D3D9Device->CreateVertexBuffer (1024, D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT, &g_D3D9DynamicVB, NULL);
        break;
    case kGfxDeviceEventBeforeReset:
    case kGfxDeviceEventShutdown:
        // Before device is reset or being shut down, release the VB.
        SAFE_RELEASE(g_D3D9DynamicVB);
        break;
    }
}

#endif // #if SUPPORT_D3D9



// -------------------------------------------------------------------
//  Direct3D 11 setup/teardown code


#if SUPPORT_D3D11

static ID3D11Device* g_D3D11Device;
static ID3D11Buffer* g_D3D11VB; // vertex buffer
static ID3D11Buffer* g_D3D11CB; // constant buffer
static ID3D11VertexShader* g_D3D11VertexShader;
static ID3D11PixelShader* g_D3D11PixelShader;
static ID3D11InputLayout* g_D3D11InputLayout;
static ID3D11RasterizerState* g_D3D11RasterState;
static ID3D11BlendState* g_D3D11BlendState;
static ID3D11DepthStencilState* g_D3D11DepthState;

typedef HRESULT (WINAPI *D3DCompileFunc)(
    const void* pSrcData,
    unsigned long SrcDataSize,
    const char* pFileName,
    const D3D10_SHADER_MACRO* pDefines,
    ID3D10Include* pInclude,
    const char* pEntrypoint,
    const char* pTarget,
    unsigned int Flags1,
    unsigned int Flags2,
    ID3D10Blob** ppCode,
    ID3D10Blob** ppErrorMsgs);

static const char* kD3D11ShaderText =
"cbuffer MyCB : register(b0) {\n"
"	float4x4 worldMatrix;\n"
"}\n"
"void VS (float3 pos : POSITION, float4 color : COLOR, out float4 ocolor : COLOR, out float4 opos : SV_Position) {\n"
"	opos = mul (worldMatrix, float4(pos,1));\n"
"	ocolor = color;\n"
"}\n"
"float4 PS (float4 color : COLOR) : SV_TARGET {\n"
"	return color;\n"
"}\n";


static void CreateD3D11Resources()
{
    D3D11_BUFFER_DESC desc;
    memset (&desc, 0, sizeof(desc));

    // vertex buffer
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.ByteWidth = 1024;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    g_D3D11Device->CreateBuffer (&desc, NULL, &g_D3D11VB);

    // constant buffer
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.ByteWidth = 64; // hold 1 matrix
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = 0;
    g_D3D11Device->CreateBuffer (&desc, NULL, &g_D3D11CB);

    // shaders
    HMODULE compiler = LoadLibraryA("D3DCompiler_43.dll");

    if (compiler == NULL)
    {
        // Try compiler from Windows 8 SDK
        compiler = LoadLibraryA("D3DCompiler_46.dll");
    }
    if (compiler)
    {
        ID3D10Blob* vsBlob = NULL;
        ID3D10Blob* psBlob = NULL;

        D3DCompileFunc compileFunc = (D3DCompileFunc)GetProcAddress (compiler, "D3DCompile");
        if (compileFunc)
        {
            HRESULT hr;
            hr = compileFunc(kD3D11ShaderText, strlen(kD3D11ShaderText), NULL, NULL, NULL, "VS", "vs_4_0", 0, 0, &vsBlob, NULL);
            if (SUCCEEDED(hr))
            {
                g_D3D11Device->CreateVertexShader (vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), NULL, &g_D3D11VertexShader);
            }

            hr = compileFunc(kD3D11ShaderText, strlen(kD3D11ShaderText), NULL, NULL, NULL, "PS", "ps_4_0", 0, 0, &psBlob, NULL);
            if (SUCCEEDED(hr))
            {
                g_D3D11Device->CreatePixelShader (psBlob->GetBufferPointer(), psBlob->GetBufferSize(), NULL, &g_D3D11PixelShader);
            }
        }

        // input layout
        if (g_D3D11VertexShader && vsBlob)
        {
            D3D11_INPUT_ELEMENT_DESC layout[] = {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            };

            g_D3D11Device->CreateInputLayout (layout, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &g_D3D11InputLayout);
        }

        SAFE_RELEASE(vsBlob);
        SAFE_RELEASE(psBlob);

        FreeLibrary (compiler);
    }
    else
    {
        DebugLog ("D3D11: HLSL shader compiler not found, will not render anything\n");
    }

    // render states
    D3D11_RASTERIZER_DESC rsdesc;
    memset (&rsdesc, 0, sizeof(rsdesc));
    rsdesc.FillMode = D3D11_FILL_SOLID;
    rsdesc.CullMode = D3D11_CULL_NONE;
    rsdesc.DepthClipEnable = TRUE;
    g_D3D11Device->CreateRasterizerState (&rsdesc, &g_D3D11RasterState);

    D3D11_DEPTH_STENCIL_DESC dsdesc;
    memset (&dsdesc, 0, sizeof(dsdesc));
    dsdesc.DepthEnable = TRUE;
    dsdesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    dsdesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    g_D3D11Device->CreateDepthStencilState (&dsdesc, &g_D3D11DepthState);

    D3D11_BLEND_DESC bdesc;
    memset (&bdesc, 0, sizeof(bdesc));
    bdesc.RenderTarget[0].BlendEnable = FALSE;
    bdesc.RenderTarget[0].RenderTargetWriteMask = 0xF;
    g_D3D11Device->CreateBlendState (&bdesc, &g_D3D11BlendState);
}

static void ReleaseD3D11Resources()
{
    SAFE_RELEASE(g_D3D11VB);
    SAFE_RELEASE(g_D3D11CB);
    SAFE_RELEASE(g_D3D11VertexShader);
    SAFE_RELEASE(g_D3D11PixelShader);
    SAFE_RELEASE(g_D3D11InputLayout);
    SAFE_RELEASE(g_D3D11RasterState);
    SAFE_RELEASE(g_D3D11BlendState);
    SAFE_RELEASE(g_D3D11DepthState);
}

static void SetGraphicsDeviceD3D11 (ID3D11Device* device, GfxDeviceEventType eventType)
{
    g_D3D11Device = device;

    if (eventType == kGfxDeviceEventInitialize)
        CreateD3D11Resources();
    if (eventType == kGfxDeviceEventShutdown)
        ReleaseD3D11Resources();
}

#endif // #if SUPPORT_D3D11



// --------------------------------------------------------------------------
// SetDefaultGraphicsState
//
// Helper function to setup some "sane" graphics state. Rendering state
// upon call into our plugin can be almost completely arbitrary depending
// on what was rendered in Unity before.
// Before calling into the plugin, Unity will set shaders to null,
// and will unbind most of "current" objects (e.g. VBOs in OpenGL case).
//
// Here, we set culling off, lighting off, alpha blend & test off, Z
// comparison to less equal, and Z writes off.

static void SetDefaultGraphicsState ()
{
    #if SUPPORT_D3D9
    // D3D9 case
    if (g_DeviceType == kGfxRendererD3D9)
    {
        g_D3D9Device->SetRenderState (D3DRS_CULLMODE, D3DCULL_NONE);
        g_D3D9Device->SetRenderState (D3DRS_LIGHTING, FALSE);
        g_D3D9Device->SetRenderState (D3DRS_ALPHABLENDENABLE, FALSE);
        g_D3D9Device->SetRenderState (D3DRS_ALPHATESTENABLE, FALSE);
        g_D3D9Device->SetRenderState (D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
        g_D3D9Device->SetRenderState (D3DRS_ZWRITEENABLE, FALSE);
    }
    #endif


    #if SUPPORT_D3D11
    // D3D11 case
    if (g_DeviceType == kGfxRendererD3D11)
    {
        ID3D11DeviceContext* ctx = NULL;
        g_D3D11Device->GetImmediateContext (&ctx);
        ctx->OMSetDepthStencilState (g_D3D11DepthState, 0);
        ctx->RSSetState (g_D3D11RasterState);
        ctx->OMSetBlendState (g_D3D11BlendState, NULL, 0xFFFFFFFF);
        ctx->Release();
    }
    #endif


    #if SUPPORT_OPENGL
    // OpenGL case
    if (g_DeviceType == kGfxRendererOpenGL)
    {
        glDisable (GL_CULL_FACE);
        glDisable (GL_LIGHTING);
        glDisable (GL_BLEND);
        glDisable (GL_ALPHA_TEST);
        glDepthFunc (GL_LEQUAL);
        glEnable (GL_DEPTH_TEST);
        glDepthMask (GL_FALSE);
    }
    #endif
}


static void FillTextureFromCode (int width, int height, int stride, unsigned char* dst)
{
    const float t = g_Time * 4.0f;

    for (int y = 0; y < height; ++y)
    {
        unsigned char* ptr = dst;
        for (int x = 0; x < width; ++x)
        {
            // Simple oldskool "plasma effect", a bunch of combined sine waves
            int vv = int(
                (127.0f + (127.0f * sinf(x/7.0f+t))) +
                (127.0f + (127.0f * sinf(y/5.0f-t))) +
                (127.0f + (127.0f * sinf((x+y)/6.0f-t))) +
                (127.0f + (127.0f * sinf(sqrtf(float(x*x + y*y))/4.0f-t)))
                ) / 4;

            // Write the texture pixel
            ptr[0] = vv;
            ptr[1] = vv;
            ptr[2] = vv;
            ptr[3] = vv;

            // To next pixel (our pixels are 4 bpp)
            ptr += 4;
        }

        // To next image row
        dst += stride;
    }
}


static void DoRendering (const float* worldMatrix, const float* identityMatrix, float* projectionMatrix, const MyVertex* verts)
{
    // Does actual rendering of a simple triangle

    #if SUPPORT_D3D9
    // D3D9 case
    if (g_DeviceType == kGfxRendererD3D9)
    {
        // Transformation matrices
        g_D3D9Device->SetTransform (D3DTS_WORLD, (const D3DMATRIX*)worldMatrix);
        g_D3D9Device->SetTransform (D3DTS_VIEW, (const D3DMATRIX*)identityMatrix);
        g_D3D9Device->SetTransform (D3DTS_PROJECTION, (const D3DMATRIX*)projectionMatrix);

        // Vertex layout
        g_D3D9Device->SetFVF (D3DFVF_XYZ|D3DFVF_DIFFUSE);

        // Texture stage states to output vertex color
        g_D3D9Device->SetTextureStageState (0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        g_D3D9Device->SetTextureStageState (0, D3DTSS_COLORARG1, D3DTA_CURRENT);
        g_D3D9Device->SetTextureStageState (0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        g_D3D9Device->SetTextureStageState (0, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
        g_D3D9Device->SetTextureStageState (1, D3DTSS_COLOROP, D3DTOP_DISABLE);
        g_D3D9Device->SetTextureStageState (1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

        // Copy vertex data into our small dynamic vertex buffer. We could have used
        // DrawPrimitiveUP just fine as well.
        void* vbPtr;
        g_D3D9DynamicVB->Lock (0, 0, &vbPtr, D3DLOCK_DISCARD);
        memcpy (vbPtr, verts, sizeof(verts[0])*3);
        g_D3D9DynamicVB->Unlock ();
        g_D3D9Device->SetStreamSource (0, g_D3D9DynamicVB, 0, sizeof(MyVertex));

        // Draw!
        g_D3D9Device->DrawPrimitive (D3DPT_TRIANGLELIST, 0, 1);

        // Update native texture from code
        if (g_TexturePointer)
        {
            IDirect3DTexture9* d3dtex = (IDirect3DTexture9*)g_TexturePointer;
            D3DSURFACE_DESC desc;
            d3dtex->GetLevelDesc (0, &desc);
            D3DLOCKED_RECT lr;
            d3dtex->LockRect (0, &lr, NULL, 0);
            FillTextureFromCode (desc.Width, desc.Height, lr.Pitch, (unsigned char*)lr.pBits);
            d3dtex->UnlockRect (0);
        }
    }
    #endif


    #if SUPPORT_D3D11
    // D3D11 case
    if (g_DeviceType == kGfxRendererD3D11)
    {
        mpRender();
    }
    #endif


    #if SUPPORT_OPENGL
    // OpenGL case
    if (g_DeviceType == kGfxRendererOpenGL)
    {
        // Transformation matrices
        glMatrixMode (GL_MODELVIEW);
        glLoadMatrixf (worldMatrix);
        glMatrixMode (GL_PROJECTION);
        // Tweak the projection matrix a bit to make it match what identity
        // projection would do in D3D case.
        projectionMatrix[10] = 2.0f;
        projectionMatrix[14] = -1.0f;
        glLoadMatrixf (projectionMatrix);

        // Vertex layout
        glVertexPointer (3, GL_FLOAT, sizeof(verts[0]), &verts[0].x);
        glEnableClientState (GL_VERTEX_ARRAY);
        glColorPointer (4, GL_UNSIGNED_BYTE, sizeof(verts[0]), &verts[0].color);
        glEnableClientState (GL_COLOR_ARRAY);

        // Draw!
        glDrawArrays (GL_TRIANGLES, 0, 3);

        // update native texture from code
        if (g_TexturePointer)
        {
            GLuint gltex = (GLuint)(size_t)(g_TexturePointer);
            glBindTexture (GL_TEXTURE_2D, gltex);
            int texWidth, texHeight;
            glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texWidth);
            glGetTexLevelParameteriv (GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texHeight);

            unsigned char* data = new unsigned char[texWidth*texHeight*4];
            FillTextureFromCode (texWidth, texHeight, texHeight*4, data);
            glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, texWidth, texHeight, GL_RGBA, GL_UNSIGNED_BYTE, data);
            delete[] data;
        }
    }
    #endif
}





bool mpInitialize(ID3D11Device *dev)
{
    tbb::task_scheduler_init tbb_init;
    //tbb::task_scheduler_init tbb_init(1); // for debug


    if (FAILED(mpInitializeDevice(dev)))
    {
        mpCleanupDevice();
        return false;
    }
    mpClearParticles();

    return true;
}

void mpFinalize()
{
    mpCleanupDevice();
}

void mpClearParticles()
{
    g_mpWorld.clearParticles();
}

extern "C" EXPORT_API void mpSetViewProjectionMatrix(XMFLOAT4X4 view, XMFLOAT4X4 proj)
{
    g_camera.forceSetMatrix(view, proj);
}

extern "C" EXPORT_API uint32_t mpGetNumParticles(mpWorld *ctx)
{
    return g_mpWorld.num_active_particles;
}

extern "C" EXPORT_API uint32_t mpScatterParticlesSphererical(mpWorld *ctx, XMFLOAT3 center, float radius, uint32 num)
{
    std::vector<mpParticle> particles(num);
    for (size_t i = 0; i < particles.size(); ++i) {
        particles[i].position = ist::simdvec4_set(
            center.x + mpGenRand()*radius, center.y + mpGenRand()*radius, center.z + mpGenRand()*radius, 1.0f);
        particles[i].velocity = _mm_set1_ps(0.0f);
    }
    g_mpWorld.addParticles(&particles[0], particles.size());
    return num;
}

extern "C" EXPORT_API uint32_t mpAddBoxCollider(mpWorld *ctx, XMFLOAT4X4 transform, XMFLOAT3 size)
{
    size.x *= 0.5f;
    size.y *= 0.5f;
    size.z *= 0.5f;

    XMMATRIX st = XMMATRIX((float*)&transform);
    XMVECTOR vertices[] = {
        { size.x, size.y, size.z, 0.0f },
        { -size.x, size.y, size.z, 0.0f },
        { -size.x, -size.y, size.z, 0.0f },
        { size.x, -size.y, size.z, 0.0f },
        { size.x, size.y, -size.z, 0.0f },
        { -size.x, size.y, -size.z, 0.0f },
        { -size.x, -size.y, -size.z, 0.0f },
        { size.x, -size.y, -size.z, 0.0f },
    };
    for (int i = 0; i < _countof(vertices); ++i) {
        vertices[i] = XMVector4Transform(vertices[i], st);
    }

    XMVECTOR normals[6] = {
        XMVector3Normalize(XMVector3Cross(XMVectorSubtract(vertices[3], vertices[0]), XMVectorSubtract(vertices[4], vertices[0]))),
        XMVector3Normalize(XMVector3Cross(XMVectorSubtract(vertices[5], vertices[1]), XMVectorSubtract(vertices[2], vertices[1]))),
        XMVector3Normalize(XMVector3Cross(XMVectorSubtract(vertices[7], vertices[3]), XMVectorSubtract(vertices[2], vertices[3]))),
        XMVector3Normalize(XMVector3Cross(XMVectorSubtract(vertices[1], vertices[0]), XMVectorSubtract(vertices[4], vertices[0]))),
        XMVector3Normalize(XMVector3Cross(XMVectorSubtract(vertices[1], vertices[0]), XMVectorSubtract(vertices[3], vertices[0]))),
        XMVector3Normalize(XMVector3Cross(XMVectorSubtract(vertices[7], vertices[4]), XMVectorSubtract(vertices[5], vertices[4]))),
    };
    float32 distances[6] = {
        -XMVector3Dot(vertices[0], normals[0]).m128_f32[0],
        -XMVector3Dot(vertices[1], normals[1]).m128_f32[0],
        -XMVector3Dot(vertices[0], normals[2]).m128_f32[0],
        -XMVector3Dot(vertices[3], normals[3]).m128_f32[0],
        -XMVector3Dot(vertices[0], normals[4]).m128_f32[0],
        -XMVector3Dot(vertices[4], normals[5]).m128_f32[0],
    };

    ispc::BoxCollider box = {
        0,
        { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
        transform.m[3][0], transform.m[3][1], transform.m[3][2],
        {
            { normals[0].m128_f32[0], normals[0].m128_f32[1], normals[0].m128_f32[2], distances[0] },
            { normals[1].m128_f32[0], normals[1].m128_f32[1], normals[1].m128_f32[2], distances[1] },
            { normals[2].m128_f32[0], normals[2].m128_f32[1], normals[2].m128_f32[2], distances[2] },
            { normals[3].m128_f32[0], normals[3].m128_f32[1], normals[3].m128_f32[2], distances[3] },
            { normals[4].m128_f32[0], normals[4].m128_f32[1], normals[4].m128_f32[2], distances[4] },
            { normals[5].m128_f32[0], normals[5].m128_f32[1], normals[5].m128_f32[2], distances[5] },
        }
    };
    g_mpWorld.collision_boxes.push_back(box);
    return 0;
}

extern "C" EXPORT_API uint32_t mpAddSphereCollider(mpWorld *ctx, XMFLOAT3 center, float radius)
{
    ispc::SphereCollider sphere = {
        0,
        { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
        center.x, center.y, center.z, radius
    };
    g_mpWorld.collision_spheres.push_back(sphere);
    return 0;
}

extern "C" EXPORT_API uint32_t mpAddDirectionalForce(mpWorld *ctx, XMFLOAT3 direction, float strength)
{
    ispc::DirectionalForce force;
    set_nxyz(force, direction.x, direction.y, direction.z);
    force.strength = strength;
    g_mpWorld.force_directional.push_back(force);
    return 0;
}

