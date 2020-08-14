#include "Framework.h"

SelfShader* SelfShader::instance = nullptr;

VertexShaderSrc* SelfShader::VertexShader(wstring file, string entryPoint)
{
	if (vertexShaders.count(file) > 0)
		return &vertexShaders[file];

	ID3D11VertexShader* shader;
	ID3D11InputLayout* layout;

	ComPtr<ID3DBlob> blob;

	D3DCompileFromFile(file.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str(), "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, blob.ReleaseAndGetAddressOf(), nullptr);
	V(DEVICE->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(),nullptr, &shader));

	ComPtr<ID3D11ShaderReflection> reflection;

	V(D3DReflect(blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(reflection.ReleaseAndGetAddressOf())));

	D3D11_SHADER_DESC shaderDesc;
	reflection->GetDesc(&shaderDesc);

	vector<D3D11_INPUT_ELEMENT_DESC> inputElements;

	for (UINT i = 0; i < shaderDesc.InputParameters; i++)
	{
		D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
		reflection->GetInputParameterDesc(i, &paramDesc);

		D3D11_INPUT_ELEMENT_DESC elementDesc;
		elementDesc.SemanticName = paramDesc.SemanticName;
		elementDesc.SemanticIndex = paramDesc.SemanticIndex;
		elementDesc.InputSlot = 0;
		elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		elementDesc.InstanceDataStepRate = 0;

		if (paramDesc.Mask == 1)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
				elementDesc.Format = DXGI_FORMAT_R32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
				elementDesc.Format = DXGI_FORMAT_R32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
				elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
		}
		else if (paramDesc.Mask < 4)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
				elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
				elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
				elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
		}
		else if (paramDesc.Mask < 8)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
				elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
				elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
				elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		}
		else if (paramDesc.Mask < 16)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
				elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
				elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
				elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		}

		string temp = paramDesc.SemanticName;
		if (temp == "POSITION")
			elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;

		if (temp == "INSTANCE")
		{
			elementDesc.InputSlot = 1;
			elementDesc.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
			elementDesc.InstanceDataStepRate = 1;
		}

		inputElements.emplace_back(elementDesc);
	}

	DEVICE->CreateInputLayout(inputElements.data(), (UINT)inputElements.size(), blob->GetBufferPointer(), blob->GetBufferSize(), &layout);

	vertexShaders[file].shader.Attach(shader);
	vertexShaders[file].layout.Attach(layout);
	//SetDebugName(shader, "_vertexShader " + ToString(file));
	//SetDebugName(layout, "_InputLayout " + ToString(file));
	Path::SetDebugName(vertexShaders[file].shader.Get(), "vertexShader " + Path::ToString(file));
	Path::SetDebugName(vertexShaders[file].layout.Get(), "InputLayout " + Path::ToString(file));

	return &vertexShaders[file];
}


ID3D11HullShader* SelfShader::HullShader(wstring file, string entryPoint)
{
	if (hullShaders.count(file) > 0)
		return hullShaders[file].Get();

	ID3D11HullShader* shader;

	ComPtr<ID3DBlob> blob;

	V(D3DCompileFromFile(file.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str(), "hs_5_0", D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, blob.ReleaseAndGetAddressOf(), nullptr));
	V(DEVICE->CreateHullShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &shader));
	Path::SetDebugName(shader, "_hullShaders" + Path::ToString(file));
	hullShaders[file].Attach(shader);
	Path::SetDebugName(hullShaders[file].Get(), "hullShaders " + Path::ToString(file));

	return hullShaders[file].Get();
}

ID3D11DomainShader* SelfShader::DomainShader(wstring file, string entryPoint)
{
	if (domainShaders.count(file) > 0)
		return domainShaders[file].Get();

	ID3D11DomainShader* shader;

	ComPtr<ID3DBlob> blob;

	V(D3DCompileFromFile(file.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str(), "ds_5_0", D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, blob.ReleaseAndGetAddressOf(), nullptr));
	V(DEVICE->CreateDomainShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &shader));
	Path::SetDebugName(shader, "_domainShaders" + Path::ToString(file));
	domainShaders[file].Attach(shader);
	Path::SetDebugName(domainShaders[file].Get(), "domainShaders " + Path::ToString(file));

	return domainShaders[file].Get();
}

ID3D11GeometryShader* SelfShader::GeometryShader(wstring file, string entryPoint)
{
	if (geometryShaders.count(file) > 0)
		return geometryShaders[file].Get();

	ID3D11GeometryShader* shader;

	ComPtr<ID3DBlob> blob;

	V(D3DCompileFromFile(file.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str(), "gs_5_0", D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, blob.ReleaseAndGetAddressOf(), nullptr));
	V(DEVICE->CreateGeometryShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &shader));
	Path::SetDebugName(shader, "_geometryShaders" + Path::ToString(file));
	geometryShaders[file].Attach(shader);
	Path::SetDebugName(geometryShaders[file].Get(), "geometryShaders " + Path::ToString(file));

	return geometryShaders[file].Get();
}

ID3D11PixelShader* SelfShader::PixelShader(wstring file, string entryPoint)
{
	if (pixelShaders.count(file) > 0)
		return pixelShaders[file].Get();

	ID3D11PixelShader* shader;

	ComPtr<ID3DBlob> blob;

	V(D3DCompileFromFile(file.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str(), "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, blob.ReleaseAndGetAddressOf(), nullptr));
	V(DEVICE->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &shader));
	Path::SetDebugName(shader, "_pixelShaders" + Path::ToString(file));
	pixelShaders[file].Attach(shader);
	Path::SetDebugName(pixelShaders[file].Get(), "pixelShaders " + Path::ToString(file));

	return pixelShaders[file].Get();
}

ID3D11ComputeShader* SelfShader::ComputeShader(wstring file, string entryPoint)
{
	if (computeShaders.count(file) > 0)
		return computeShaders[file].Get();

	ID3D11ComputeShader* shader;

	ComPtr<ID3DBlob> blob;

	V(D3DCompileFromFile(file.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str(), "cs_5_0", D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0, blob.ReleaseAndGetAddressOf(), nullptr));
	V(DEVICE->CreateComputeShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &shader));
	Path::SetDebugName(shader, "_computeShaders" + Path::ToString(file));
	computeShaders[file].Attach(shader);
	Path::SetDebugName(computeShaders[file].Get(), "computeShaders " + Path::ToString(file));

	return computeShaders[file].Get();
}

SelfShader::~SelfShader()
{
	//if (!vertexShaders.empty())
	//	map<wstring,VertexShaderSrc>().swap(vertexShaders);
	//if (!hullShaders.empty())
	//	map<wstring, ComPtr<ID3D11HullShader>>().swap(hullShaders);
	//if (!domainShaders.empty())
	//	map<wstring, ComPtr<ID3D11DomainShader>>().swap(domainShaders);
	//if (!geometryShaders.empty())
	//	map<wstring, ComPtr<ID3D11GeometryShader>>().swap(geometryShaders);
	//if (!pixelShaders.empty())
	//	map<wstring, ComPtr<ID3D11PixelShader>>().swap(pixelShaders);
	//if (!computeShaders.empty())
	//	map<wstring, ComPtr<ID3D11ComputeShader>>().swap(computeShaders);
	vertexShaders.clear();
	hullShaders.clear();
	domainShaders.clear();
	geometryShaders.clear();
	pixelShaders.clear();
	computeShaders.clear();

}

