/**************************************************************************
 *
 * Copyright 2012-2021 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 **************************************************************************/


#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include <initguid.h>
#include <windows.h>

#include <d3d11.h>

#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

#include "tri_vs_4_0.h"
#include "tri_ps_4_0.h"


int
main(int argc, char *argv[])
{
    HRESULT hr;

    HINSTANCE hInstance = GetModuleHandle(nullptr);

    WNDCLASSEX wc = {
        sizeof(WNDCLASSEX),
        CS_CLASSDC,
        DefWindowProc,
        0,
        0,
        hInstance,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        "tri",
        nullptr
    };
    RegisterClassEx(&wc);

    const int WindowWidth = 250;
    const int WindowHeight = 250;

    DWORD dwStyle = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW | WS_VISIBLE;

    RECT rect = {0, 0, WindowWidth, WindowHeight};
    AdjustWindowRect(&rect, dwStyle, false);

    HWND hWnd = CreateWindow(wc.lpszClassName,
                             "Simple example using DirectX10",
                             dwStyle,
                             CW_USEDEFAULT, CW_USEDEFAULT,
                             rect.right - rect.left,
                             rect.bottom - rect.top,
                             nullptr,
                             nullptr,
                             hInstance,
                             nullptr);
    if (!hWnd) {
        return EXIT_FAILURE;
    }

    UINT Flags = 0;
    hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_NULL, 0, D3D11_CREATE_DEVICE_DEBUG, nullptr, 0, D3D11_SDK_VERSION, nullptr, nullptr, nullptr);
    if (SUCCEEDED(hr)) {
        Flags |= D3D11_CREATE_DEVICE_DEBUG;
    }

    static const D3D_FEATURE_LEVEL FeatureLevels[] = {
        D3D_FEATURE_LEVEL_10_0
    };

    HMODULE hSoftware = LoadLibraryA("d3d10sw.dll");
    if (!hSoftware) {
       return EXIT_FAILURE;
    }

    DXGI_SWAP_CHAIN_DESC SwapChainDesc;
    ZeroMemory(&SwapChainDesc, sizeof SwapChainDesc);
    SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;;
    SwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    SwapChainDesc.SampleDesc.Quality = 0;
    SwapChainDesc.SampleDesc.Count = 1;
    SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDesc.BufferCount = 2;
    SwapChainDesc.OutputWindow = hWnd;
    SwapChainDesc.Windowed = true;
    SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    ComPtr<ID3D11Device> pDevice;
    ComPtr<ID3D11DeviceContext> pDeviceContext;
    ComPtr<IDXGISwapChain> pSwapChain;
    hr = D3D11CreateDeviceAndSwapChain(nullptr,
                                       D3D_DRIVER_TYPE_SOFTWARE,
                                       hSoftware,
                                       Flags,
                                       FeatureLevels,
                                       _countof(FeatureLevels),
                                       D3D11_SDK_VERSION,
                                       &SwapChainDesc,
                                       &pSwapChain,
                                       &pDevice,
                                       nullptr, /* pFeatureLevel */
                                       &pDeviceContext);
    if (FAILED(hr)) {
        return EXIT_FAILURE;
    }

    ComPtr<IDXGIDevice> pDXGIDevice;
    hr = pDevice->QueryInterface(IID_IDXGIDevice, (void **)&pDXGIDevice);
    if (FAILED(hr)) {
        return EXIT_FAILURE;
    }

    ComPtr<IDXGIAdapter> pAdapter;
    hr = pDXGIDevice->GetAdapter(&pAdapter);
    if (FAILED(hr)) {
        return EXIT_FAILURE;
    }

    DXGI_ADAPTER_DESC Desc;
    hr = pAdapter->GetDesc(&Desc);
    if (FAILED(hr)) {
        return EXIT_FAILURE;
    }

    printf("using %S\n", Desc.Description);

    ComPtr<ID3D11Texture2D> pBackBuffer;
    hr = pSwapChain->GetBuffer(0, IID_ID3D11Texture2D, (void **)&pBackBuffer);
    if (FAILED(hr)) {
        return EXIT_FAILURE;
    }

    D3D11_RENDER_TARGET_VIEW_DESC RenderTargetViewDesc;
    ZeroMemory(&RenderTargetViewDesc, sizeof RenderTargetViewDesc);
    RenderTargetViewDesc.Format = SwapChainDesc.BufferDesc.Format;
    RenderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    RenderTargetViewDesc.Texture2D.MipSlice = 0;

    ComPtr<ID3D11RenderTargetView> pRenderTargetView;
    hr = pDevice->CreateRenderTargetView(pBackBuffer.Get(), &RenderTargetViewDesc, &pRenderTargetView);
    if (FAILED(hr)) {
        return EXIT_FAILURE;
    }

    pDeviceContext->OMSetRenderTargets(1, pRenderTargetView.GetAddressOf(), nullptr);


    const float clearColor[4] = { 0.3f, 0.1f, 0.3f, 1.0f };
    pDeviceContext->ClearRenderTargetView(pRenderTargetView.Get(), clearColor);

    ComPtr<ID3D11VertexShader> pVertexShader;
    hr = pDevice->CreateVertexShader(g_VS, sizeof g_VS, nullptr, &pVertexShader);
    if (FAILED(hr)) {
        return EXIT_FAILURE;
    }

    struct Vertex {
        float position[4];
        float color[4];
    };

    static const D3D11_INPUT_ELEMENT_DESC InputElementDescs[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex, color),    D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    ComPtr<ID3D11InputLayout> pVertexLayout;
    hr = pDevice->CreateInputLayout(InputElementDescs,
                                    _countof(InputElementDescs),
                                    g_VS, sizeof g_VS,
                                    &pVertexLayout);
    if (FAILED(hr)) {
        return EXIT_FAILURE;
    }

    pDeviceContext->IASetInputLayout(pVertexLayout.Get());

    ComPtr<ID3D11PixelShader> pPixelShader;
    hr = pDevice->CreatePixelShader(g_PS, sizeof g_PS, nullptr, &pPixelShader);
    if (FAILED(hr)) {
        return EXIT_FAILURE;
    }

    pDeviceContext->VSSetShader(pVertexShader.Get(), nullptr, 0);
    pDeviceContext->PSSetShader(pPixelShader.Get(), nullptr, 0);

    static const Vertex vertices[] = {
        { { -0.9f, -0.9f, 0.5f, 1.0f}, { 0.8f, 0.0f, 0.0f, 0.1f } },
        { {  0.9f, -0.9f, 0.5f, 1.0f}, { 0.0f, 0.9f, 0.0f, 0.1f } },
        { {  0.0f,  0.9f, 0.5f, 1.0f}, { 0.0f, 0.0f, 0.7f, 0.1f } },
    };

    D3D11_BUFFER_DESC BufferDesc;
    ZeroMemory(&BufferDesc, sizeof BufferDesc);
    BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    BufferDesc.ByteWidth = sizeof vertices;
    BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    BufferDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA BufferData;
    BufferData.pSysMem = vertices;
    BufferData.SysMemPitch = 0;
    BufferData.SysMemSlicePitch = 0;

    ComPtr<ID3D11Buffer> pVertexBuffer;
    hr = pDevice->CreateBuffer(&BufferDesc, &BufferData, &pVertexBuffer);
    if (FAILED(hr)) {
        return EXIT_FAILURE;
    }

    UINT Stride = sizeof(Vertex);
    UINT Offset = 0;
    pDeviceContext->IASetVertexBuffers(0, 1, pVertexBuffer.GetAddressOf(), &Stride, &Offset);

    D3D11_VIEWPORT ViewPort;
    ViewPort.TopLeftX = 0;
    ViewPort.TopLeftY = 0;
    ViewPort.Width = WindowWidth;
    ViewPort.Height = WindowHeight;
    ViewPort.MinDepth = 0.0f;
    ViewPort.MaxDepth = 1.0f;
    pDeviceContext->RSSetViewports(1, &ViewPort);

    D3D11_RASTERIZER_DESC RasterizerDesc;
    ZeroMemory(&RasterizerDesc, sizeof RasterizerDesc);
    RasterizerDesc.CullMode = D3D11_CULL_NONE;
    RasterizerDesc.FillMode = D3D11_FILL_SOLID;
    RasterizerDesc.FrontCounterClockwise = true;
    RasterizerDesc.DepthClipEnable = true;
    ComPtr<ID3D11RasterizerState> pRasterizerState;
    hr = pDevice->CreateRasterizerState(&RasterizerDesc, &pRasterizerState);
    if (FAILED(hr)) {
        return EXIT_FAILURE;
    }
    pDeviceContext->RSSetState(pRasterizerState.Get());

    pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    pDeviceContext->Draw(_countof(vertices), 0);

    pSwapChain->Present(0, 0);

    Sleep(1000);

    ID3D11Buffer *pNullBuffer = nullptr;
    UINT NullStride = 0;
    UINT NullOffset = 0;
    pDeviceContext->IASetVertexBuffers(0, 1, &pNullBuffer, &NullStride, &NullOffset);

    pDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

    pDeviceContext->IASetInputLayout(nullptr);

    pDeviceContext->VSSetShader(nullptr, nullptr, 0);

    pDeviceContext->PSSetShader(nullptr, nullptr, 0);

    pDeviceContext->RSSetState(nullptr);

    DestroyWindow(hWnd);

    return EXIT_SUCCESS;
}

