#include "../../includes/core/utils/updater.hpp"
#include "../../deps/json.hpp"
#include <Shlwapi.h>
#include <bcrypt.h>
#pragma comment(lib, "Bcrypt.lib")
#include <fstream>
#include <filesystem>
#include <thread>
#include <cstring>
#include <sstream>
#include <wininet.h>
#pragma comment(lib, "Wininet.lib")

namespace Updater
{
    static bool g_envLoaded = false;

    void LoadEnvFile()
    {
        if (g_envLoaded) return;
        g_envLoaded = true;
        wchar_t exePath[MAX_PATH];
        GetModuleFileNameW(nullptr, exePath, _countof(exePath));
        std::wstring dir(exePath);
        size_t pos = dir.find_last_of(L"\\/");
        if (pos != std::wstring::npos) dir = dir.substr(0, pos);
        std::wstring envPath = dir + L"\\.env";
        std::ifstream ifs(std::filesystem::path(envPath), std::ios::binary);
        if (!ifs.is_open()) return;
        std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        std::istringstream ss(content);
        std::string line;
        while (std::getline(ss, line))
        {
            if (line.empty() || line[0] == '#') continue;
            size_t eq = line.find('=');
            if (eq == std::string::npos) continue;
            std::string key = line.substr(0, eq);
            std::string val = line.substr(eq + 1);
            // trim
            auto trim = [](std::string& s){
                size_t a = s.find_first_not_of(" \t\r\n");
                size_t b = s.find_last_not_of(" \t\r\n");
                if (a == std::string::npos) { s.clear(); return; }
                s = s.substr(a, b - a + 1);
            };
            trim(key); trim(val);
            if (!key.empty()) _putenv_s(key.c_str(), val.c_str());
        }
    }
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
            // ENV-first: if provided via .env, don't do any network
            LoadEnvFile();
            char* ev = nullptr; size_t el = 0;
            _dupenv_s(&ev, &el, "LAUNCHER_VERSION");
            if (ev && el)
            {
                out.version.assign(ev, el);
                free(ev); ev=nullptr; el=0;
                char* eu=nullptr; size_t eul=0; _dupenv_s(&eu, &eul, "LAUNCHER_URL");
                if (eu) { out.url.assign(eu, eul); free(eu); }
                char* es=nullptr; size_t esl=0; _dupenv_s(&es, &esl, "LAUNCHER_SHA256");
                if (es) { out.sha256.assign(es, esl); free(es); }
                // normalize sha256 like network path below
                if (!out.sha256.empty())
                {
                    const std::string prefix = "sha256:";
                    if (out.sha256.rfind(prefix, 0) == 0)
                        out.sha256 = out.sha256.substr(prefix.size());
                    std::transform(out.sha256.begin(), out.sha256.end(), out.sha256.begin(), [](unsigned char c){ return (char)std::tolower(c); });
                }
                return !out.version.empty() && !out.url.empty();
            }

            std::wstring wurl = Utf8ToWide(manifestUrl);
            wchar_t tempDir[MAX_PATH];
            GetTempPathW(_countof(tempDir), tempDir);
            wchar_t tempFile[MAX_PATH];
            GetTempFileNameW(tempDir, L"onyxmf", 0, tempFile);

            HRESULT hr = URLDownloadToFileW(nullptr, wurl.c_str(), tempFile, 0, nullptr);
            if (FAILED(hr)) { DeleteFileW(tempFile); return false; }

            std::ifstream ifs(std::filesystem::path(tempFile), std::ios::binary);
            std::string body((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
            ifs.close();
            DeleteFileW(tempFile);

            nlohmann::json j = nlohmann::json::parse(body, nullptr, false);
            if (j.is_discarded()) return false;
            out.version = j.value<std::string>("version", "");
            out.url = j.value<std::string>("url", "");
            out.sha256 = j.value<std::string>("sha256", "");
            if (!out.sha256.empty())
            {
                const std::string prefix = "sha256:";
                if (out.sha256.rfind(prefix, 0) == 0)
                    out.sha256 = out.sha256.substr(prefix.size());
                // lowercase normalize
                std::transform(out.sha256.begin(), out.sha256.end(), out.sha256.begin(), [](unsigned char c){ return (char)std::tolower(c); });
            }
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

            // If we have a GitHub token (env or embedded), use WinINet to attach the header.
            LoadEnvFile();
            char* envTok = nullptr; size_t envTokLen = 0; _dupenv_s(&envTok, &envTokLen, "LAUNCHER_GITHUB_TOKEN");
            std::string token = (envTok && envTokLen) ? std::string(envTok, envTokLen) : std::string();
            if (envTok) free(envTok);

            HRESULT hr = E_FAIL;
            if (!token.empty())
            {
                HINTERNET hInet = InternetOpenW(L"OnyxLauncher/1.0", INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
                if (!hInet) { DeleteFileW(tempFile); outError = "net init"; return {}; }
                // Parse URL
                URL_COMPONENTSW uc{}; uc.dwStructSize = sizeof(uc);
                wchar_t host[256]; wchar_t path[1024];
                uc.lpszHostName = host; uc.dwHostNameLength = _countof(host);
                uc.lpszUrlPath = path; uc.dwUrlPathLength = _countof(path);
                if (!InternetCrackUrlW(wurl.c_str(), 0, 0, &uc)) { InternetCloseHandle(hInet); DeleteFileW(tempFile); outError = "bad url"; return {}; }
                bool https = (uc.nScheme == INTERNET_SCHEME_HTTPS);
                HINTERNET hConn = InternetConnectW(hInet, host, uc.nPort, nullptr, nullptr, INTERNET_SERVICE_HTTP, 0, 0);
                if (!hConn) { InternetCloseHandle(hInet); DeleteFileW(tempFile); outError = "conn"; return {}; }
                const wchar_t* types = L"*/*";
                HINTERNET hReq = HttpOpenRequestW(hConn, L"GET", path, nullptr, nullptr, &types, https ? INTERNET_FLAG_SECURE : 0, 0);
                if (!hReq) { InternetCloseHandle(hConn); InternetCloseHandle(hInet); DeleteFileW(tempFile); outError = "req"; return {}; }
                std::wstring hdr = L"Authorization: Bearer "; hdr += std::wstring(token.begin(), token.end()); hdr += L"\r\nUser-Agent: OnyxLauncher\r\nAccept: application/octet-stream\r\n";
                if (!HttpSendRequestW(hReq, hdr.c_str(), (DWORD)hdr.size(), nullptr, 0)) { InternetCloseHandle(hReq); InternetCloseHandle(hConn); InternetCloseHandle(hInet); DeleteFileW(tempFile); outError = "send"; return {}; }
                BYTE buf[64*1024]; DWORD rd=0; HANDLE hOut = CreateFileW(tempFile, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, nullptr);
                if (hOut == INVALID_HANDLE_VALUE) { InternetCloseHandle(hReq); InternetCloseHandle(hConn); InternetCloseHandle(hInet); DeleteFileW(tempFile); outError = "fs"; return {}; }
                while (InternetReadFile(hReq, buf, sizeof(buf), &rd) && rd) {
                    DWORD wr=0; WriteFile(hOut, buf, rd, &wr, nullptr);
                }
                CloseHandle(hOut);
                InternetCloseHandle(hReq); InternetCloseHandle(hConn); InternetCloseHandle(hInet);
                hr = S_OK;
            }
            else
            {
                // Anonymous download (for public URLs)
                hr = URLDownloadToFileW(nullptr, wurl.c_str(), tempFile, 0, nullptr);
            }
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

    // Helper: prompt user for update using native MessageBox and orchestrate download+replace
    void PromptAndMaybeUpdate(AppState& state)
    {
        static bool s_prompted = false;
        if (!state.updateAvailable || s_prompted) return;
        s_prompted = true;

        std::wstring msg = L"A new Onyx Launcher update is available.\nCurrent: ";
        msg += std::wstring(ONYX_LAUNCHER_VERSION, ONYX_LAUNCHER_VERSION + strlen(ONYX_LAUNCHER_VERSION));
        msg += L"\nAvailable: ";
        if (!state.availableVersion.empty())
        {
            std::wstring wver(state.availableVersion.begin(), state.availableVersion.end());
            msg += wver;
        }
        msg += L"\n\nUpdate and restart now?";

        HWND hwnd = ::window->hWindow;
        ShowWindow(hwnd, SW_HIDE);
        int rc = MessageBoxW(nullptr, msg.c_str(), L"Onyx Launcher Update", MB_ICONINFORMATION | MB_YESNO | MB_SETFOREGROUND);
        if (rc != IDYES) { exit(0); }

        state.downloadingUpdate = true;
        state.updateError.clear();
        auto ps = &state;
        // Resolve download URL preference:
        // 1) If .env contains LAUNCHER_URL, prefer it
        // 2) Otherwise use manifest url
        LoadEnvFile();
        char* envUrl = nullptr; size_t envLen = 0;
        _dupenv_s(&envUrl, &envLen, "LAUNCHER_URL");
        std::string url = (envUrl && envLen) ? std::string(envUrl, envLen) : state.availableUrl;
        if (envUrl) free(envUrl);
        std::string sha = state.availableSha256;
        std::thread([ps, url, sha, hwnd]() {
            std::string err;
            std::string temp = DownloadToTemp(url, err);
            if (temp.empty()) { ps->updateError = err.empty()? std::string("Download failed") : err; ps->downloadingUpdate = false; MessageBoxW(nullptr, L"Update download failed.", L"Onyx Launcher", MB_ICONERROR | MB_OK); ExitProcess(0); return; }

            std::wstring wtemp(temp.begin(), temp.end());
            if (!sha.empty())
            {
                // Allow override from .env too (LAUNCHER_SHA256)
                LoadEnvFile();
                char* envSha = nullptr; size_t envShaLen = 0; _dupenv_s(&envSha, &envShaLen, "LAUNCHER_SHA256");
                std::string expected = (envSha && envShaLen) ? std::string(envSha, envShaLen) : sha;
                if (envSha) free(envSha);

                std::string got = ComputeFileSha256(wtemp);
                if (_stricmp(got.c_str(), expected.c_str()) != 0)
                {
                    ps->updateError = "Checksum mismatch";
                    ps->downloadingUpdate = false;
                    MessageBoxW(nullptr, L"Update checksum mismatch.", L"Onyx Launcher", MB_ICONERROR | MB_OK);
                    ExitProcess(0);
                    return;
                }
            }

            LaunchSelfReplaceAndExit(wtemp);
            ps->downloadingUpdate = false;
        }).detach();
    }
}


