#pragma once

class Render2D : public Transform
{
protected:
	Shader* shader;
	ID3D11ShaderResourceView* srv;
	Texture* texture;

	VertexBuffer* vertexBuffer;

	Matrix view;
	Matrix orthographic;

	DepthStencilState* depthState;
	BlendState* blendState;
	BlendState* oldState;
public:
	Render2D(wstring shaderFile = L"Render2D");
	virtual ~Render2D();

	virtual void Update();
	virtual void Render();

	void SetTexture(Texture* value) { texture = value; }
	void SetSRV(ID3D11ShaderResourceView* value) { srv = value; }
};