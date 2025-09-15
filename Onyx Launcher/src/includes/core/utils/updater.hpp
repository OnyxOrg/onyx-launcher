#pragma once

#include "../main.hpp"
#include "../version.hpp"
#include <string>
#include <functional>

namespace Updater
{
    struct Manifest
    {
        std::string version;   // e.g. "1.2.3"
        std::string url;       // download URL of the new exe
        std::string sha256;    // optional checksum (hex)
    };

    // Compare semantic versions. Returns: -1 if a<b, 0 if equal, +1 if a>b
    int CompareVersions(const std::string& a, const std::string& b);

    // Fetch manifest JSON from URL. Returns true on success.
    bool FetchManifest(const std::string& manifestUrl, Manifest& out);

    // Download file to a temp path. Returns full path on success, empty on failure.
    std::string DownloadToTemp(const std::string& url, std::string& outError);

    // Compute SHA256 of a file, hex lowercase. Returns empty on failure.
    std::string ComputeFileSha256(const std::wstring& path);

    // Launch a PowerShell helper to replace current exe with new exe, relaunch, and exit current process.
    // Returns true if helper launched.
    bool LaunchSelfReplaceAndExit(const std::wstring& newExePath);
}


