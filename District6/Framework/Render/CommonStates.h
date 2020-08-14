#pragma once

class CommonStates
{
public:
    static CommonStates* Get() { return instance; }
    static void Create() { instance = NEW CommonStates(); }
    static void Delete() { SAFE_DELETE(instance); }

    // Blend states.
    ID3D11BlendState* Opaque();
    ID3D11BlendState* AlphaBlend();
    ID3D11BlendState* Additive();
    ID3D11BlendState* NonPremultiplied();

    // Depth stencil states.
    ID3D11DepthStencilState* DepthNone(UINT stencilRef = 1);
    ID3D11DepthStencilState* DepthDefault(UINT stencilRef = 1);
    ID3D11DepthStencilState* DepthRead(UINT stencilRef = 1);

    // Rasterizer states.
    ID3D11RasterizerState* CullNone();
    ID3D11RasterizerState* CullClockwise();
    ID3D11RasterizerState* CullCounterClockwise();
    ID3D11RasterizerState* Wireframe();

    // Sampler states.
    ID3D11SamplerState** PointWrap(UINT startSlot = 0);
    ID3D11SamplerState** PointClamp(UINT startSlot = 0);
    ID3D11SamplerState** LinearWrap(UINT startSlot = 0);
    ID3D11SamplerState** LinearClamp(UINT startSlot = 0);
    ID3D11SamplerState** AnisotropicWrap(UINT startSlot = 0);
    ID3D11SamplerState** AnisotropicClamp(UINT startSlot = 0);
private:
    CommonStates();
    ~CommonStates();

    HRESULT CreateBlendState(D3D11_BLEND, D3D11_BLEND, _Out_ ID3D11BlendState**);
    HRESULT CreateDepthStencilState(bool, bool, _Out_ ID3D11DepthStencilState**);
    HRESULT CreateRasterizerState(D3D11_CULL_MODE, D3D11_FILL_MODE, _Out_ ID3D11RasterizerState**);
    HRESULT CreateSamplerState(D3D11_FILTER, D3D11_TEXTURE_ADDRESS_MODE, _Out_ ID3D11SamplerState**);

    static CommonStates* instance;

    ComPtr<ID3D11BlendState> opaque;
    ComPtr<ID3D11BlendState> alphaBlend;
    ComPtr<ID3D11BlendState> additive;
    ComPtr<ID3D11BlendState> nonPremultiplied;

    ComPtr<ID3D11DepthStencilState> depthNone;
    ComPtr<ID3D11DepthStencilState> depthDefault;
    ComPtr<ID3D11DepthStencilState> depthRead;

    ComPtr<ID3D11RasterizerState> cullNone;
    ComPtr<ID3D11RasterizerState> cullClockwise;
    ComPtr<ID3D11RasterizerState> cullCounterClockwise;
    ComPtr<ID3D11RasterizerState> wireframe;

    ComPtr<ID3D11SamplerState> pointWrap;
    ComPtr<ID3D11SamplerState> pointClamp;
    ComPtr<ID3D11SamplerState> linearWrap;
    ComPtr<ID3D11SamplerState> linearClamp;
    ComPtr<ID3D11SamplerState> anisotropicWrap;
    ComPtr<ID3D11SamplerState> anisotropicClamp;
};