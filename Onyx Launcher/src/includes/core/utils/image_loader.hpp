#pragma once

#include "../../core/main.hpp"
#include <vector>

namespace ImageLoader
{
	// Downloads an image from a URL into memory using Urlmon and creates an SRV using D3DX11.
	// Returns nullptr on failure.
	inline ID3D11ShaderResourceView* LoadTextureFromUrl(const char* url)
	{
		if (!url || !*url) return nullptr;
		IStream* stream = nullptr;
		HRESULT hr = URLOpenBlockingStreamA(nullptr, url, &stream, 0, nullptr);
		if (FAILED(hr) || !stream) return nullptr;
		// Read all bytes
		std::vector<unsigned char> data;
		unsigned char buf[4096];
		for (;;)
		{
			ULONG read = 0;
			HRESULT r = stream->Read(buf, sizeof(buf), &read);
			if (FAILED(r) || read == 0) break;
			data.insert(data.end(), buf, buf + read);
		}
		stream->Release();
		if (data.empty()) return nullptr;
		ID3D11ShaderResourceView* srv = nullptr;
		HRESULT hr2 = D3DX11CreateShaderResourceViewFromMemory(window->Dvc, data.data(), (UINT)data.size(), nullptr, nullptr, &srv, nullptr);
		if (FAILED(hr2)) return nullptr;
		return srv;
	}
}


