#pragma once
struct VertexShaderSrc
{
	ComPtr<ID3D11VertexShader> shader = nullptr;
	ComPtr<ID3D11InputLayout> layout = nullptr;
};

class SelfShader
{
public:
	static SelfShader* Get() { return instance; }
	static void Create() { instance = NEW SelfShader(); }
	static void Delete() { SAFE_DELETE(instance); }

	VertexShaderSrc* VertexShader(wstring file, string entryPoint);
	ID3D11HullShader* HullShader(wstring file, string entryPoint);
	ID3D11DomainShader* DomainShader(wstring file, string entryPoint);
	ID3D11GeometryShader* GeometryShader(wstring file, string entryPoint);
	ID3D11PixelShader* PixelShader(wstring file, string entryPoint);
	ID3D11ComputeShader* ComputeShader(wstring file, string entryPoint);
private:
	~SelfShader();

	static SelfShader* instance;

	map<wstring, VertexShaderSrc> vertexShaders;
	map<wstring, ComPtr<ID3D11HullShader>> hullShaders;
	map<wstring, ComPtr<ID3D11DomainShader>> domainShaders;
	map<wstring, ComPtr<ID3D11GeometryShader>> geometryShaders;
	map<wstring, ComPtr<ID3D11PixelShader>> pixelShaders;
	map<wstring, ComPtr<ID3D11ComputeShader>> computeShaders;
};

