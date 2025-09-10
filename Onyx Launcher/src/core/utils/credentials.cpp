#include "includes/core/utils/credentials.hpp"
#include <windows.h>
#include <shlobj.h>
#include <fstream>
#include <vector>

namespace {
	static const wchar_t* kCredsDir = L"%APPDATA%\\onyx-launcher"; // per user request
	static const wchar_t* kCredsFile = L"%APPDATA%\\onyx-launcher\\creds.bin";

	bool EnsureDir()
	{
		// Resolve roaming path dynamically for any user
		PWSTR roam = nullptr;
		std::wstring dir;
		if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &roam)))
		{
			dir.assign(roam);
			CoTaskMemFree(roam);
		}
		else
		{
			wchar_t buf[MAX_PATH]{}; DWORD n = GetEnvironmentVariableW(L"APPDATA", buf, MAX_PATH);
			dir.assign(n ? buf : L".");
		}
		if (!dir.empty() && dir.back() != L'\\') dir += L'\\';
		dir += L"onyx-launcher";
		DWORD attrs = GetFileAttributesW(dir.c_str());
		if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY)) return true;
		return CreateDirectoryW(dir.c_str(), nullptr) || GetLastError() == ERROR_ALREADY_EXISTS;
	}

	bool Protect(const std::string& plain, std::vector<BYTE>& out)
	{
		DATA_BLOB in{}; in.pbData = (BYTE*)plain.data(); in.cbData = (DWORD)plain.size();
		DATA_BLOB outBlob{};
		if (!CryptProtectData(&in, L"onyx", nullptr, nullptr, nullptr, 0, &outBlob)) return false;
		out.assign(outBlob.pbData, outBlob.pbData + outBlob.cbData);
		LocalFree(outBlob.pbData);
		return true;
	}

	bool Unprotect(const std::vector<BYTE>& enc, std::string& out)
	{
		DATA_BLOB in{}; in.pbData = const_cast<BYTE*>(enc.data()); in.cbData = (DWORD)enc.size();
		DATA_BLOB outBlob{}; LPWSTR desc = nullptr;
		if (!CryptUnprotectData(&in, &desc, nullptr, nullptr, nullptr, 0, &outBlob)) return false;
		out.assign((char*)outBlob.pbData, (char*)outBlob.pbData + outBlob.cbData);
		LocalFree(outBlob.pbData);
		if (desc) LocalFree(desc);
		return true;
	}
}

namespace Credentials
{
	bool Load(std::string& outUsername, std::string& outPassword)
	{
		// Build file path from roaming app data
		PWSTR roam = nullptr; std::wstring path;
		if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &roam)))
		{
			path.assign(roam); CoTaskMemFree(roam);
		}
		else
		{
			wchar_t buf[MAX_PATH]{}; DWORD n = GetEnvironmentVariableW(L"APPDATA", buf, MAX_PATH); path.assign(n ? buf : L".");
		}
		if (!path.empty() && path.back() != L'\\') path += L'\\';
		path += L"onyx-launcher\\creds.bin";
		std::ifstream f(path, std::ios::binary);
		if (!f.good()) return false;
		uint32_t ulen = 0, plen = 0; f.read((char*)&ulen, sizeof(ulen));
		std::string user(ulen, '\0'); f.read(user.data(), ulen);
		f.read((char*)&plen, sizeof(plen));
		std::vector<BYTE> enc(plen); f.read((char*)enc.data(), plen);
		f.close();
		std::string pass;
		if (!Unprotect(enc, pass)) return false;
		outUsername = user; outPassword = pass; return true;
	}

	bool Save(const std::string& username, const std::string& password)
	{
		if (!EnsureDir()) return false;
		std::vector<BYTE> enc; if (!Protect(password, enc)) return false;
		PWSTR roam = nullptr; std::wstring path;
		if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &roam)))
		{
			path.assign(roam); CoTaskMemFree(roam);
		}
		else { wchar_t buf[MAX_PATH]{}; DWORD n = GetEnvironmentVariableW(L"APPDATA", buf, MAX_PATH); path.assign(n ? buf : L"."); }
		if (!path.empty() && path.back() != L'\\') path += L'\\';
		path += L"onyx-launcher\\creds.bin";
		std::ofstream f(path, std::ios::binary | std::ios::trunc);
		if (!f.good()) return false;
		uint32_t ulen = (uint32_t)username.size();
		uint32_t plen = (uint32_t)enc.size();
		f.write((const char*)&ulen, sizeof(ulen));
		f.write(username.data(), ulen);
		f.write((const char*)&plen, sizeof(plen));
		f.write((const char*)enc.data(), plen);
		return true;
	}

	bool Clear()
	{
		PWSTR roam = nullptr; std::wstring path;
		if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &roam))) { path.assign(roam); CoTaskMemFree(roam); }
		else { wchar_t buf[MAX_PATH]{}; DWORD n = GetEnvironmentVariableW(L"APPDATA", buf, MAX_PATH); path.assign(n ? buf : L"."); }
		if (!path.empty() && path.back() != L'\\') path += L'\\'; path += L"onyx-launcher\\creds.bin";
		DeleteFileW(path.c_str());
		return true;
	}
}


