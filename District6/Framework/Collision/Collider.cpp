#include "Framework.h"

bool Collider::is_dev_mode = false;

Collider::Collider()
{
	shader = Shader::Add(L"Collider");
	colorBuffer = NEW ColorBuffer();
	colorBuffer->data.color = Float4(0, 1, 0, 1);
	target = nullptr;
}

Collider::~Collider()
{
	delete colorBuffer;
	//delete vertexBuffer;
	//delete indexBuffer;
}


void Collider::ChangeDevMode()
{
	is_dev_mode = !is_dev_mode;
}

void Collider::Update()
{
	if(target!= nullptr)
	{
		position = target->position;
		position += offset;
		rotation = target->rotation;
	}
	UpdateWorld();
}

void Collider::Render()
{
	if (!is_dev_mode)
		return;
	
	vertexBuffer->Set();
	indexBuffer->Set();
	IASetPT(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

	colorBuffer->SetPSBuffer(10);
	worldBuffer->SetVSBuffer(1);

	shader->Set();
	DC->DrawIndexed(indices.size(), 0, 0);
}

