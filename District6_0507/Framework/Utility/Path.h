#pragma once

namespace Path
{
	string ToString(wstring value);
	wstring ToWString(string value);

	bool ExistFile(string path);
	bool ExistDirectory(string path);

	wstring GetExtension(wstring path);
	string GetFileName(string path);
	string GetDirectoryName(string path);
	string GetFileNameWithoutExtension(string path);

	void CreateFolders(string path);

	vector<string> SplitString(string origin, string tok);

	void Replace(string* str, string comp, string rep);

	inline void SetDebugName(ID3D11DeviceChild* child, string name)
	{
#if defined(DEBUG) || defined(_DEBUG)  
		if (child != nullptr && name != "")
			V(child->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)name.size(), name.c_str()));
#endif
	}
}