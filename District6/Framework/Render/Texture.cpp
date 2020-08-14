#include "Framework.h"

map<wstring, Texture*> Texture::totalTexture;

Texture::Texture(wstring file)
{
	auto ptr = std::make_shared<ComPtr<ID3D11ShaderResourceView>>();

	TexMetadata image_meta;


	const auto file_path = file;
	const auto extension = Path::GetExtension(file_path);

	HRESULT hr;

	if (extension == L"tga")
	{
		hr = GetMetadataFromTGAFile(file_path.c_str(), image_meta);
		V(hr);
	}
	else if (extension == L"dds")
	{
		hr = GetMetadataFromDDSFile(file_path.c_str(), DDS_FLAGS_NONE, image_meta);
		V(hr);
	}
	else
	{
		hr = GetMetadataFromWICFile(file_path.c_str(), WIC_CODEC_PNG, image_meta);
		V(hr);
	}
	width = image_meta.width;
	height= image_meta.height;
	
	ScratchImage temp_image;
	
	temp_image.Initialize1D(DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1, 0);
	

	
	srv = std::make_shared<ID3D11ShaderResourceView*>();
	
	V(CreateShaderResourceView(DEVICE, temp_image.GetImages(), temp_image.GetImageCount(), temp_image.GetMetadata(), &(*srv)));

	task = concurrency::create_task([file, this]
	{
		std::unique_lock<std::mutex> lock1(lock,std::defer_lock);

		lock1.lock();

		const auto file_path = file;
		const auto extension = Path::GetExtension(file_path);


		HRESULT hr;
		image = std::make_shared<ScratchImage>();
		if (extension == L"tga")
		{
			hr = LoadFromTGAFile(file_path.c_str(), nullptr, *image);
			V(hr);
		}
		else if (extension == L"dds")
		{
			hr = LoadFromDDSFile(file_path.c_str(), DDS_FLAGS_NONE, nullptr, *image);
			V(hr);
		}
		else
		{
			hr = LoadFromWICFile(file_path.c_str(), WIC_CODEC_PNG, nullptr, *image);
			V(hr);
		}

		if (FAILED(hr))
			return;


		(*srv)->Release();
		V(CreateShaderResourceView(DEVICE, image->GetImages(), image->GetImageCount(), image->GetMetadata(), &(*srv)));

		lock1.unlock();
	});
}

Texture::~Texture()
{
	(*srv)->Release();
}

Texture* Texture::Add(wstring file)
{
	if (file.length() == 0)
		return nullptr;

	if (totalTexture.count(file) > 0)
		return totalTexture[file];


	Texture* texture = new Texture(file);
	texture->file = file;

	totalTexture.insert({ file,  texture });

	return totalTexture[file];
}

void Texture::Delete()
{
	for (auto texture : totalTexture)
	{
		delete texture.second;
	}
}

void Texture::Set(UINT slot)
{
	DC->PSSetShaderResources(slot, 1, &(*srv));
}

vector<Float4> Texture::ReadPixels()
{
	std::unique_lock<std::mutex> lock1(lock, std::defer_lock);

	lock1.lock();
	
	vector<Float4> result;
	if(image == nullptr)
	{
		result.resize(width * height,{0,0,0,0});
		
		lock1.unlock();
		return result;
	}
	

	uint8_t* colors = image->GetPixels();
	UINT size = image->GetPixelsSize();

	for (int i = 0; i < size / 4; i++)
	{
		Float4 color;

		float scale = 1.0f / 255.0f;
		color.x = colors[i * 4 + 0] * scale;
		color.y = colors[i * 4 + 1] * scale;
		color.z = colors[i * 4 + 2] * scale;
		color.w = colors[i * 4 + 3] * scale;

		result.push_back(color);
	}

	lock1.unlock();
	
	return result;
}
