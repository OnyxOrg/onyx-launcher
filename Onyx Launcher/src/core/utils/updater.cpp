#include "../../includes/core/utils/updater.hpp"
#include "../../deps/json.hpp"
#include <Shlwapi.h>
#include <bcrypt.h>
#pragma comment(lib, "Bcrypt.lib")

namespace Updater
{
    static std::wstring GetCurrentExePath()
    {
        wchar_t buf[MAX_PATH];
        DWORD got = GetModuleFileNameW(nullptr, buf, _countof(buf));
        return std::wstring(buf, got);
    }

    static std::wstring Utf8ToWide(const std::string& s)
    {
        if (s.empty()) return L"";
        int needed = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
        std::wstring w; w.resize(needed);
        MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &w[0], needed);
        return w;
    }

    int CompareVersions(const std::string& a, const std::string& b)
    {
        auto split = [](const std::string& v){ std::vector<int> out; int n=0; char c; std::stringstream ss(v); while (ss >> n) { out.push_back(n); if (!(ss >> c)) break; } return out; };
        auto pa = split(a), pb = split(b);
        size_t m = std::max(pa.size(), pb.size());
        pa.resize(m, 0); pb.resize(m, 0);
        for (size_t i=0;i<m;i++) { if (pa[i] < pb[i]) return -1; if (pa[i] > pb[i]) return 1; }
        return 0;
    }

    bool FetchManifest(const std::string& manifestUrl, Manifest& out)
    {
        try
        {
            std::wstring wurl = Utf8ToWide(manifestUrl);
            wchar_t tempDir[MAX_PATH];
            GetTempPathW(_countof(tempDir), tempDir);
            wchar_t tempFile[MAX_PATH];
            GetTempFileNameW(tempDir, L"onyxmf", 0, tempFile);

            HRESULT hr = URLDownloadToFileW(nullptr, wurl.c_str(), tempFile, 0, nullptr);
            if (FAILED(hr)) { DeleteFileW(tempFile); return false; }

            std::ifstream ifs(tempFile, std::ios::binary);
            std::string body((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
            ifs.close();
            DeleteFileW(tempFile);

            nlohmann::json j = nlohmann::json::parse(body, nullptr, false);
            if (j.is_discarded()) return false;
            out.version = j.value<std::string>("version", "");
            out.url = j.value<std::string>("url", "");
            out.sha256 = j.value<std::string>("sha256", "");
            return !out.version.empty() && !out.url.empty();
        }
        catch (...) { return false; }
    }

    std::string DownloadToTemp(const std::string& url, std::string& outError)
    {
        outError.clear();
        try
        {
            std::wstring wurl = Utf8ToWide(url);
            wchar_t tempDir[MAX_PATH];
            GetTempPathW(_countof(tempDir), tempDir);
            wchar_t tempFile[MAX_PATH];
            GetTempFileNameW(tempDir, L"onyxupd", 0, tempFile);

            HRESULT hr = URLDownloadToFileW(nullptr, wurl.c_str(), tempFile, 0, nullptr);
            if (FAILED(hr)) { DeleteFileW(tempFile); outError = "download failed"; return {}; }

            int len = WideCharToMultiByte(CP_UTF8, 0, tempFile, -1, nullptr, 0, nullptr, nullptr);
            std::string out; out.resize(len - 1);
            WideCharToMultiByte(CP_UTF8, 0, tempFile, -1, &out[0], len, nullptr, nullptr);
            return out;
        }
        catch (...) { outError = "unexpected"; return {}; }
    }

    std::string ComputeFileSha256(const std::wstring& path)
    {
        BCRYPT_ALG_HANDLE hAlg = nullptr; BCRYPT_HASH_HANDLE hHash = nullptr; PBYTE hashObj = nullptr; DWORD objLen=0, hashLen=0;
        if (BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, nullptr, 0) < 0) return {};
        if (BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PUCHAR)&objLen, sizeof(objLen), &hashLen, 0) < 0) { BCryptCloseAlgorithmProvider(hAlg,0); return {}; }
        if (BCryptGetProperty(hAlg, BCRYPT_HASH_LENGTH, (PUCHAR)&hashLen, sizeof(hashLen), &hashLen, 0) < 0) { BCryptCloseAlgorithmProvider(hAlg,0); return {}; }
        hashObj = (PBYTE)HeapAlloc(GetProcessHeap(), 0, objLen);
        if (!hashObj) { BCryptCloseAlgorithmProvider(hAlg,0); return {}; }
        if (BCryptCreateHash(hAlg, &hHash, hashObj, objLen, nullptr, 0, 0) < 0) { HeapFree(GetProcessHeap(),0,hashObj); BCryptCloseAlgorithmProvider(hAlg,0); return {}; }

        HANDLE f = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (f == INVALID_HANDLE_VALUE) { BCryptDestroyHash(hHash); HeapFree(GetProcessHeap(),0,hashObj); BCryptCloseAlgorithmProvider(hAlg,0); return {}; }
        BYTE buf[64*1024]; DWORD rd=0;
        while (ReadFile(f, buf, sizeof(buf), &rd, nullptr) && rd) {
            if (BCryptHashData(hHash, buf, rd, 0) < 0) { CloseHandle(f); BCryptDestroyHash(hHash); HeapFree(GetProcessHeap(),0,hashObj); BCryptCloseAlgorithmProvider(hAlg,0); return {}; }
        }
        CloseHandle(f);
        std::string hex; hex.resize(hashLen*2);
        std::vector<BYTE> hash(hashLen);
        if (BCryptFinishHash(hHash, hash.data(), hashLen, 0) < 0) { BCryptDestroyHash(hHash); HeapFree(GetProcessHeap(),0,hashObj); BCryptCloseAlgorithmProvider(hAlg,0); return {}; }
        static const char* kHex = "0123456789abcdef";
        for (DWORD i=0;i<hashLen;i++){ hex[i*2]=kHex[(hash[i]>>4)&0xF]; hex[i*2+1]=kHex[hash[i]&0xF]; }
        BCryptDestroyHash(hHash); HeapFree(GetProcessHeap(),0,hashObj); BCryptCloseAlgorithmProvider(hAlg,0);
        return hex;
    }

    bool LaunchSelfReplaceAndExit(const std::wstring& newExePath)
    {
        std::wstring current = GetCurrentExePath();
        wchar_t tempBat[MAX_PATH];
        GetTempFileNameW(L".", L"up", 0, tempBat);
        std::wstring ps;
        ps += L"$old='" + current + L"';";
        ps += L"$new='" + newExePath + L"';";
        ps += L"while(Get-Process | where {$_.Path -eq $old}){ Start-Sleep -Milliseconds 300 };";
        ps += L"Copy-Item -LiteralPath $new -Destination $old -Force;";
        ps += L"Start-Process -FilePath $old;";
        ps += L"Remove-Item -LiteralPath $new -Force -ErrorAction SilentlyContinue;";

        std::wstring cmd = L"powershell -ExecutionPolicy Bypass -NoProfile -Command \"" + ps + L"\"";

        STARTUPINFOW si{}; si.cb = sizeof(si); PROCESS_INFORMATION pi{};
        BOOL ok = CreateProcessW(nullptr, (LPWSTR)cmd.c_str(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);
        if (ok) {
            CloseHandle(pi.hThread); CloseHandle(pi.hProcess);
            ExitProcess(0);
            return true;
        }
        return false;
    }
}


