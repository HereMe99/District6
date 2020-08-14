#pragma once
#include "ppltasks.h"
#include "mutex"

class Texture
{
private:
	std::mutex lock;
	wstring file;
	std::shared_ptr<ScratchImage> image;

	std::shared_ptr<ID3D11ShaderResourceView*> srv;
	Concurrency::task<void> task;
	
	static map<wstring, Texture*> totalTexture;
	size_t width;
	size_t height;
	Texture(wstring file);
	~Texture();
	
public:
	static Texture* Add(wstring file);
	static void Delete();

	void Set(UINT slot);

	vector<Float4> ReadPixels();

	UINT GetWidth() { return width; }
	UINT GetHeight() { return height; }

	wstring GetFile() { return file; }

	ID3D11ShaderResourceView* &GetSRV()
	{
		return *srv;
	}
};
