#include "Framework.h"

CommonStates* CommonStates::instance = nullptr;

//float blendFactor[4] = { 0, 0, 0, 0 };
//DC->OMSetBlendState(opaque.Get(), blendFactor, 0xffffffff);
ID3D11BlendState* CommonStates::Opaque()
{
    if (opaque == nullptr)
    {
        CreateBlendState(D3D11_BLEND_ONE, D3D11_BLEND_ZERO, opaque.ReleaseAndGetAddressOf());
        Path::SetDebugName(opaque.Get(), "opaque");
    }
    return opaque.Get();
}

//float blendFactor[4] = { 0, 0, 0, 0 };
//DC->OMSetBlendState(alphaBlend.Get(), blendFactor, 0xffffffff);
ID3D11BlendState* CommonStates::AlphaBlend()
{
    if (alphaBlend == nullptr)
    {
        CreateBlendState(D3D11_BLEND_ONE, D3D11_BLEND_INV_SRC_ALPHA, alphaBlend.ReleaseAndGetAddressOf());
        Path::SetDebugName(alphaBlend.Get(), "alphaBlend");
    }
    return alphaBlend.Get();
}

//float blendFactor[4] = { 0, 0, 0, 0 };
//DC->OMSetBlendState(additive.Get(), blendFactor, 0xffffffff);
ID3D11BlendState* CommonStates::Additive()
{
    if (additive == nullptr)
    {
        CreateBlendState(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE, additive.ReleaseAndGetAddressOf());
        Path::SetDebugName(additive.Get(), "additive");
    }
    return additive.Get();
}

//float blendFactor[4] = { 0, 0, 0, 0 };
//DC->OMSetBlendState(nonPremultiplied.Get(), blendFactor, 0xffffffff);
ID3D11BlendState* CommonStates::NonPremultiplied()
{
    if (nonPremultiplied == nullptr)
    {
        CreateBlendState(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, nonPremultiplied.ReleaseAndGetAddressOf());
        Path::SetDebugName(nonPremultiplied.Get(), "nonPremultiplied");
    }
    return nonPremultiplied.Get();
}

//DC->OMSetDepthStencilState(depthNone.Get(), stencilRef);//일단 stencilRef는 1로
//don't use
ID3D11DepthStencilState* CommonStates::DepthNone(UINT stencilRef)
{
    if (depthNone == nullptr)
    {
        CreateDepthStencilState(false, false, depthNone.ReleaseAndGetAddressOf());
        Path::SetDebugName(depthNone.Get(), "depthNone");
    }
    return depthNone.Get();
}

//DC->OMSetDepthStencilState(depthDefault.Get(), stencilRef);
//read & write
ID3D11DepthStencilState* CommonStates::DepthDefault(UINT stencilRef)
{
    if (depthDefault == nullptr)
    {
        CreateDepthStencilState(true, true, depthDefault.ReleaseAndGetAddressOf());
        Path::SetDebugName(depthDefault.Get(), "depthDefault");
    }
    return depthDefault.Get();
}

//DC->OMSetDepthStencilState(depthRead.Get(), stencilRef);
//read only
ID3D11DepthStencilState* CommonStates::DepthRead(UINT stencilRef)
{
    if (depthRead == nullptr)
    {
        CreateDepthStencilState(true, false, depthRead.ReleaseAndGetAddressOf());
        Path::SetDebugName(depthRead.Get(), "depthRead");
    }
    return depthRead.Get();
}

//DC->RSSetState(cullNone.Get());
ID3D11RasterizerState* CommonStates::CullNone()
{
    if (cullNone == nullptr)
    {
        CreateRasterizerState(D3D11_CULL_NONE, D3D11_FILL_SOLID, cullNone.ReleaseAndGetAddressOf());
        Path::SetDebugName(cullNone.Get(), "cullNone");
    }
    return cullNone.Get();
}

//DC->RSSetState(cullClockwise.Get());
ID3D11RasterizerState* CommonStates::CullClockwise()
{
    if (cullClockwise == nullptr)
    {
        CreateRasterizerState(D3D11_CULL_FRONT, D3D11_FILL_SOLID, cullClockwise.ReleaseAndGetAddressOf());
        Path::SetDebugName(cullClockwise.Get(), "cullClockwise");
    }
    return cullClockwise.Get();
}

//DC->RSSetState(cullCounterClockwise.Get());
ID3D11RasterizerState* CommonStates::CullCounterClockwise()
{
    if (cullCounterClockwise == nullptr)
    {
        CreateRasterizerState(D3D11_CULL_BACK, D3D11_FILL_SOLID, cullCounterClockwise.ReleaseAndGetAddressOf());
        Path::SetDebugName(cullCounterClockwise.Get(), "cullCounterClockwise");
    }
    return cullCounterClockwise.Get();
}

//DC->RSSetState(wireframe.Get());
ID3D11RasterizerState* CommonStates::Wireframe()
{
    if (wireframe == nullptr)
    {
        CreateRasterizerState(D3D11_CULL_NONE, D3D11_FILL_WIREFRAME, wireframe.ReleaseAndGetAddressOf());
        Path::SetDebugName(wireframe.Get(), "wireframe");
    }
    return wireframe.Get();
}

//DC->PSSetSamplers(startSlot, 1, &pointWrap);
ID3D11SamplerState** CommonStates::PointWrap(UINT startSlot)
{
    if (pointWrap == nullptr)
    {
        CreateSamplerState(D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_WRAP, pointWrap.ReleaseAndGetAddressOf());
        Path::SetDebugName(pointWrap.Get(), "pointWrap");
    }
    return pointWrap.GetAddressOf();
}

//DC->PSSetSamplers(startSlot, 1, &pointClamp);
ID3D11SamplerState** CommonStates::PointClamp(UINT startSlot)
{
    if (pointClamp == nullptr)
    {
        CreateSamplerState(D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_CLAMP, pointClamp.ReleaseAndGetAddressOf());
        Path::SetDebugName(pointClamp.Get(), "pointClamp");
    }
    return pointClamp.GetAddressOf();
}

//DC->PSSetSamplers(startSlot, 1, &linearWrap);
ID3D11SamplerState** CommonStates::LinearWrap(UINT startSlot)
{
    if (linearWrap == nullptr)
    {
        CreateSamplerState(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP, linearWrap.ReleaseAndGetAddressOf());
        Path::SetDebugName(linearWrap.Get(), "linearWrap");
    }
    return linearWrap.GetAddressOf();
}

//DC->PSSetSamplers(startSlot, 1, &linearClamp);
ID3D11SamplerState** CommonStates::LinearClamp(UINT startSlot)
{
    if (linearClamp == nullptr)
    {
        CreateSamplerState(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP, linearClamp.ReleaseAndGetAddressOf());
        Path::SetDebugName(linearClamp.Get(), "linearClamp");
    }
    return linearClamp.GetAddressOf();
}

//DC->PSSetSamplers(startSlot, 1, &anisotropicWrap);
ID3D11SamplerState** CommonStates::AnisotropicWrap(UINT startSlot)
{
    if (anisotropicWrap == nullptr)
    {
        CreateSamplerState(D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_WRAP, anisotropicWrap.ReleaseAndGetAddressOf());
        Path::SetDebugName(anisotropicWrap.Get(), "anisotropicWrap");
    }
    return anisotropicWrap.GetAddressOf();
}

//DC->PSSetSamplers(startSlot, 1, &anisotropicClamp);
ID3D11SamplerState** CommonStates::AnisotropicClamp(UINT startSlot)
{
    if (anisotropicClamp == nullptr)
    {
        CreateSamplerState(D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_CLAMP, anisotropicClamp.ReleaseAndGetAddressOf());
        Path::SetDebugName(anisotropicClamp.Get(), "anisotropicClamp");
    }
    return anisotropicClamp.GetAddressOf();
}

CommonStates::CommonStates()
{
}

CommonStates::~CommonStates()
{
}

HRESULT CommonStates::CreateBlendState(D3D11_BLEND srcBlend, D3D11_BLEND destBlend, ID3D11BlendState** pResult)
{
    D3D11_BLEND_DESC desc = {};

    desc.RenderTarget[0].BlendEnable = (srcBlend != D3D11_BLEND_ONE) ||
        (destBlend != D3D11_BLEND_ZERO);

    desc.RenderTarget[0].SrcBlend = desc.RenderTarget[0].SrcBlendAlpha = srcBlend;
    desc.RenderTarget[0].DestBlend = desc.RenderTarget[0].DestBlendAlpha = destBlend;
    desc.RenderTarget[0].BlendOp = desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

    desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    return DEVICE->CreateBlendState(&desc, pResult);
}

HRESULT CommonStates::CreateDepthStencilState(bool enable, bool writeEnable, ID3D11DepthStencilState** pResult)
{
    D3D11_DEPTH_STENCIL_DESC desc = {};

    desc.DepthEnable = enable ? TRUE : FALSE;
    desc.DepthWriteMask = writeEnable ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
    desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

    desc.StencilEnable = FALSE;
    desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
    desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

    desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;

    desc.BackFace = desc.FrontFace;

    return DEVICE->CreateDepthStencilState(&desc, pResult);
}

HRESULT CommonStates::CreateRasterizerState(D3D11_CULL_MODE cullMode, D3D11_FILL_MODE fillMode, ID3D11RasterizerState** pResult)
{
    D3D11_RASTERIZER_DESC desc = {};

    desc.CullMode = cullMode;
    desc.FillMode = fillMode;
    desc.DepthClipEnable = TRUE;
    desc.MultisampleEnable = TRUE;

    return DEVICE->CreateRasterizerState(&desc, pResult);
}

HRESULT CommonStates::CreateSamplerState(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode, ID3D11SamplerState** pResult)
{
    D3D11_SAMPLER_DESC desc = {};

    desc.Filter = filter;

    desc.AddressU = addressMode;
    desc.AddressV = addressMode;
    desc.AddressW = addressMode;

    desc.MaxAnisotropy = (DEVICE->GetFeatureLevel() > D3D_FEATURE_LEVEL_9_1) ? D3D11_MAX_MAXANISOTROPY : 2u;

    desc.MaxLOD = FLT_MAX;
    desc.ComparisonFunc = D3D11_COMPARISON_NEVER;

    return DEVICE->CreateSamplerState(&desc, pResult);
}