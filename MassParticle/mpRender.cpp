#pragma warning( disable : 4996 ) // _s じゃない CRT 関数使うと出るやつ
#pragma warning( disable : 4005 ) // DirectX のマクロの redefinition

#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
#include <cstdio>
#include "resource.h"

#include "mpTypes.h"
#include "mpCore_ispc.h"
#include "MassParticle.h"

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------
struct SimpleVertex
{
    XMFLOAT3 Pos;
    XMFLOAT3 Normal;
};

struct CBChangesEveryFrame
{
    XMMATRIX ViewProjection;
    XMFLOAT4 CameraPos;

    XMFLOAT4 LightPos;
    XMFLOAT4 LightColor;

    XMFLOAT4 MeshColor;
    FLOAT    MeshShininess;
    FLOAT    padding[3];
};




//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
const int NUM_CUBE_INSTANCES = 512;

ID3D11Device*                       g_pd3dDevice = NULL;
ID3D11DeviceContext*                g_pImmediateContext = NULL;
ID3D11DepthStencilState             *g_pDepthStencilState = NULL;

ID3D11VertexShader*                 g_pCubeVertexShader = NULL;
ID3D11InputLayout*                  g_pCubeVertexLayout = NULL;
ID3D11PixelShader*                  g_pCubePixelShader = NULL;
ID3D11Buffer*                       g_pCubeVertexBuffer = NULL;
ID3D11Buffer*                       g_pCubeInstanceBuffer = NULL;
ID3D11Buffer*                       g_pCubeIndexBuffer = NULL;
ID3D11Buffer*                       g_pCBChangesEveryFrame = NULL;
XMFLOAT4                            g_vMeshColor( 0.7f, 0.7f, 0.7f, 1.0f );

extern mpPerspectiveCamera g_camera;
extern mpWorld g_mpWorld;





//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DX11
//--------------------------------------------------------------------------------------
HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DX11CompileFromFile( szFileName, NULL, NULL, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL );
    if( FAILED(hr) )
    {
        if( pErrorBlob != NULL )
            OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
        if( pErrorBlob ) pErrorBlob->Release();
        return hr;
    }
    if( pErrorBlob ) pErrorBlob->Release();

    return S_OK;
}


ID3D11Buffer* CreateVertexBuffer(const void *data, UINT size)
{
    D3D11_BUFFER_DESC bd;
    ZeroMemory( &bd, sizeof(bd) );
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = size;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory( &InitData, sizeof(InitData) );
    InitData.pSysMem = data;

    ID3D11Buffer *buffer;
    HRESULT hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &buffer );
    if( FAILED( hr ) ) {
        return NULL;
    }
    return buffer;
}



//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
bool mpInitializeDevice(void *_dev)
{
    ID3D11Device *dev = (ID3D11Device*)_dev;
    HRESULT hr = S_OK;

    g_pd3dDevice = dev;
    g_pd3dDevice->GetImmediateContext(&g_pImmediateContext);

    {
        // Compile the vertex shader
        ID3DBlob* pVSBlob = NULL;
        hr = CompileShaderFromFile( L"MassParticle.fx", "VS", "vs_4_0", &pVSBlob );
        if( FAILED( hr ) )
        {
            MessageBox( NULL,
                        L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
            return false;
        }

        // Create the vertex shader
        hr = g_pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_pCubeVertexShader );
        if( FAILED( hr ) )
        {    
            pVSBlob->Release();
            return false;
        }

        // Create the input layout
        D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "INSTANCE_POSITION",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1,  0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
            { "INSTANCE_SCALE",     0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
            { "INSTANCE_COLOR",     0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        };
        UINT numElements = ARRAYSIZE( layout );

        hr = g_pd3dDevice->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
            pVSBlob->GetBufferSize(), &g_pCubeVertexLayout );
        pVSBlob->Release();
        if (FAILED(hr)) {
            return false;
        }
    }

    {
        // Compile the pixel shader
        ID3DBlob* pPSBlob = NULL;
        hr = CompileShaderFromFile( L"MassParticle.fx", "PS", "ps_4_0", &pPSBlob );
        if( FAILED( hr ) )
        {
            MessageBox( NULL,
                L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
            return false;
        }

        // Create the pixel shader
        hr = g_pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pCubePixelShader );
        pPSBlob->Release();
        if (FAILED(hr)) {
            return false;
        }
    }

    // Create vertex buffer
    {
        SimpleVertex vertices[] =
        {
            { XMFLOAT3(-1.0f, 1.0f,-1.0f),  XMFLOAT3( 0.0f, 1.0f, 0.0f) },
            { XMFLOAT3( 1.0f, 1.0f,-1.0f),  XMFLOAT3( 0.0f, 1.0f, 0.0f) },
            { XMFLOAT3( 1.0f, 1.0f, 1.0f),  XMFLOAT3( 0.0f, 1.0f, 0.0f) },
            { XMFLOAT3(-1.0f, 1.0f, 1.0f),  XMFLOAT3( 0.0f, 1.0f, 0.0f) },

            { XMFLOAT3(-1.0f,-1.0f,-1.0f),  XMFLOAT3( 0.0f,-1.0f, 0.0f) },
            { XMFLOAT3( 1.0f,-1.0f,-1.0f),  XMFLOAT3( 0.0f,-1.0f, 0.0f) },
            { XMFLOAT3( 1.0f,-1.0f, 1.0f),  XMFLOAT3( 0.0f,-1.0f, 0.0f) },
            { XMFLOAT3(-1.0f,-1.0f, 1.0f),  XMFLOAT3( 0.0f,-1.0f, 0.0f) },

            { XMFLOAT3(-1.0f,-1.0f, 1.0f),  XMFLOAT3(-1.0f, 0.0f, 0.0f) },
            { XMFLOAT3(-1.0f,-1.0f,-1.0f),  XMFLOAT3(-1.0f, 0.0f, 0.0f) },
            { XMFLOAT3(-1.0f, 1.0f,-1.0f),  XMFLOAT3(-1.0f, 0.0f, 0.0f) },
            { XMFLOAT3(-1.0f, 1.0f, 1.0f),  XMFLOAT3(-1.0f, 0.0f, 0.0f) },

            { XMFLOAT3( 1.0f,-1.0f, 1.0f),  XMFLOAT3( 1.0f, 0.0f, 0.0f) },
            { XMFLOAT3( 1.0f,-1.0f,-1.0f),  XMFLOAT3( 1.0f, 0.0f, 0.0f) },
            { XMFLOAT3( 1.0f, 1.0f,-1.0f),  XMFLOAT3( 1.0f, 0.0f, 0.0f) },
            { XMFLOAT3( 1.0f, 1.0f, 1.0f),  XMFLOAT3( 1.0f, 0.0f, 0.0f) },

            { XMFLOAT3(-1.0f,-1.0f,-1.0f),  XMFLOAT3( 0.0f, 0.0f,-1.0f ) },
            { XMFLOAT3( 1.0f,-1.0f,-1.0f),  XMFLOAT3( 0.0f, 0.0f,-1.0f ) },
            { XMFLOAT3( 1.0f, 1.0f,-1.0f),  XMFLOAT3( 0.0f, 0.0f,-1.0f ) },
            { XMFLOAT3(-1.0f, 1.0f,-1.0f),  XMFLOAT3( 0.0f, 0.0f,-1.0f ) },

            { XMFLOAT3(-1.0f,-1.0f, 1.0f),  XMFLOAT3( 0.0f, 0.0f, 1.0f ) },
            { XMFLOAT3( 1.0f,-1.0f, 1.0f),  XMFLOAT3( 0.0f, 0.0f, 1.0f ) },
            { XMFLOAT3( 1.0f, 1.0f, 1.0f),  XMFLOAT3( 0.0f, 0.0f, 1.0f ) },
            { XMFLOAT3(-1.0f, 1.0f, 1.0f),  XMFLOAT3( 0.0f, 0.0f, 1.0f ) },
        };
        g_pCubeVertexBuffer = CreateVertexBuffer(vertices, sizeof(SimpleVertex)*ARRAYSIZE(vertices));
    }
    {
        g_pCubeInstanceBuffer = CreateVertexBuffer(g_mpWorld.particles, sizeof(mpParticle) * SPH_MAX_PARTICLE_NUM);
    }

    // Create index buffer
    {
        WORD indices[] =
        {
            3,1,0, 2,1,3,
            6,4,5, 7,4,6,
            11,9,8, 10,9,11,
            14,12,13, 15,12,14,
            19,17,16, 18,17,19,
            22,20,21, 23,20,22,
        };

        D3D11_BUFFER_DESC bd;
        ZeroMemory( &bd, sizeof(bd) );
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof( WORD ) * 36;
        bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bd.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA InitData;
        ZeroMemory( &InitData, sizeof(InitData) );
        InitData.pSysMem = indices;
        hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pCubeIndexBuffer );
        if (FAILED(hr)) {
            return false;
        }
    }

    // Create the constant buffers
    {
        D3D11_BUFFER_DESC bd;
        ZeroMemory( &bd, sizeof(bd) );
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(CBChangesEveryFrame);
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags = 0;
        hr = g_pd3dDevice->CreateBuffer( &bd, NULL, &g_pCBChangesEveryFrame );
        if (FAILED(hr)) {
            return false;
        }
    }

    g_camera.setProjection(XMConvertToRadians(45.0f), 1.0f, 0.1f, 100.0f);
    g_camera.setView(XMVectorSet(0.0f, 10.0f, -12.5f, 0.0f), XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));

    return true;
}


//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void mpCleanupDevice()
{
    if( g_pCBChangesEveryFrame ) g_pCBChangesEveryFrame->Release();
    if( g_pCubeInstanceBuffer ) g_pCubeInstanceBuffer->Release();
    if( g_pCubeVertexBuffer ) g_pCubeVertexBuffer->Release();
    if( g_pCubeIndexBuffer ) g_pCubeIndexBuffer->Release();
    if( g_pCubeVertexLayout ) g_pCubeVertexLayout->Release();
    if( g_pCubeVertexShader ) g_pCubeVertexShader->Release();
    if( g_pCubePixelShader ) g_pCubePixelShader->Release();
    if (g_pDepthStencilState) g_pDepthStencilState->Release();

    if( g_pImmediateContext ) g_pImmediateContext->Release();
}




extern "C" EXPORT_API void mpUpdate(float dt)
{

    //{
    //    ispc::SphereCollider sphere = { 0, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, 0.0f, 0.0f, 0.0f, 0.0f };
    //    g_peworld.collision_spheres.push_back(sphere);
    //    ispc::BoxCollider box = {
    //        0, {0.0f,0.0f,0.0f, 0.0f,0.0f,0.0f},
    //        0.0f, 0.0f, 0.0f, 
    //        {
    //            { 0.0f, 0.0f, 0.0f, 0.0f },
    //            { 0.0f, 0.0f, 0.0f, 0.0f },
    //            { 0.0f, 0.0f, 0.0f, 0.0f },
    //            { 0.0f, 0.0f, 0.0f, 0.0f },
    //            { 0.0f, 0.0f, 0.0f, 0.0f },
    //            { 0.0f, 0.0f, 0.0f, 0.0f },
    //        }
    //    };
    //    g_peworld.collision_boxes.push_back(box);

    //    ispc::PointForce force;
    //    set_xyz(force, 0.0f, 0.0f, 0.0f); force.strength = -0.001f;
    //    g_peworld.force_point.push_back(force);
    //}
    {
        static PerformanceCounter s_timer;
        static float s_prev = 0.0f;
        PerformanceCounter timer;

        g_mpWorld.update(1.0f);
        g_mpWorld.clearCollidersAndForces();

        if (s_timer.getElapsedMillisecond() - s_prev > 1000.0f) {
            char buf[128];
            _snprintf(buf, _countof(buf), "  SPH update: %d particles %.3fms\n", g_mpWorld.num_active_particles, timer.getElapsedMillisecond());
            OutputDebugStringA(buf);
            s_prev = s_timer.getElapsedMillisecond();
        }
    }
}


//--------------------------------------------------------------------------------------
// Render a frame
//--------------------------------------------------------------------------------------
void mpRender()
{
    {
        std::unique_lock<std::mutex> lock(g_mpWorld.m_mutex);

        CBChangesEveryFrame cb;
        XMVECTOR eye = g_camera.getEye();
        XMMATRIX vp = g_camera.getViewProjectionMatrix();

        cb.ViewProjection   = XMMatrixTranspose( vp );
        cb.CameraPos        = (FLOAT*)&eye;
        cb.LightPos         = XMFLOAT4(10.0f, 10.0f, -10.0f, 1.0f);
        cb.LightColor       = XMFLOAT4(0.9f, 0.9f, 0.9f, 1.0f);
        cb.MeshShininess    = 200.0f;
        g_pImmediateContext->UpdateSubresource( g_pCBChangesEveryFrame, 0, NULL, &cb, 0, 0 );
        g_pImmediateContext->UpdateSubresource(g_pCubeInstanceBuffer, 0, NULL, &g_mpWorld.particles, 0, 0);
    }


    {
        ID3D11Buffer *buffers[] = {g_pCubeVertexBuffer, g_pCubeInstanceBuffer};
        UINT strides[] = {sizeof(SimpleVertex), sizeof(mpParticle), };
        UINT offsets[] = {0, 0};
        g_pImmediateContext->IASetVertexBuffers( 0, ARRAYSIZE(buffers), buffers, strides, offsets );
    }
    g_pImmediateContext->IASetInputLayout( g_pCubeVertexLayout );
    g_pImmediateContext->IASetIndexBuffer( g_pCubeIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );
    g_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    g_pImmediateContext->VSSetShader( g_pCubeVertexShader, NULL, 0 );
    g_pImmediateContext->VSSetConstantBuffers( 0, 1, &g_pCBChangesEveryFrame );
    g_pImmediateContext->PSSetShader( g_pCubePixelShader, NULL, 0 );
    g_pImmediateContext->PSSetConstantBuffers( 0, 1, &g_pCBChangesEveryFrame );


    ID3D11RasterizerState *rs_old;
    ID3D11DepthStencilState *ds_old;
    UINT ds_stencil_ref;
    g_pImmediateContext->RSGetState(&rs_old);
    g_pImmediateContext->OMGetDepthStencilState(&ds_old, &ds_stencil_ref);

    {
        CD3D11_DEFAULT d;
        if (g_pDepthStencilState==nullptr) {
            CD3D11_DEPTH_STENCIL_DESC dsdesc(d);
            HRESULT hr = g_pd3dDevice->CreateDepthStencilState((D3D11_DEPTH_STENCIL_DESC*)&dsdesc, &g_pDepthStencilState);
            printf("%x\n", hr);
        }
        if (g_pDepthStencilState) {
            g_pImmediateContext->OMSetDepthStencilState(g_pDepthStencilState, ds_stencil_ref);
        }
    }

    // Render cubes
    g_pImmediateContext->DrawIndexedInstanced( 36, (UINT)g_mpWorld.num_active_particles, 0, 0, 0 );

    g_pImmediateContext->OMSetDepthStencilState(ds_old, ds_stencil_ref);
    g_pImmediateContext->RSSetState(rs_old);
    rs_old->Release();
    ds_old->Release();
}
