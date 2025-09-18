# Onyx Launcher
> **Onyx Launcher** - Where modern design meets powerful functionality.

A modern, feature-rich Windows desktop launcher application built with C++ and DirectX 11, featuring a sleek ImGui-based interface with Discord integration and product library management.

<br>

## 🚀 Features

### Authentication & User Management
- **Multiple Login Methods**: Traditional username/password, Discord OAuth, and Google authentication
- **Account Linking**: Link Discord accounts to sync roles and profile information
- **Secure Credential Storage**: Optional "Remember Me" functionality with encrypted local storage
- **Role-Based Access**: Dynamic role synchronization with Discord server roles

### Discord Integration

- **OAuth Login**: Sign in directly with your Discord account
- **Role Sync**: Automatically sync roles from your Discord server
- **Profile Integration**: Display Discord username and avatar
- **Support Server**: Quick access to Discord support server

### User Interface
- **Modern Design**: Clean, dark-themed interface with smooth animations
- **Customizable Window**: Blur effects and modern window styling
- **Responsive Layout**: Adaptive UI that scales with window size
- **Single Instance**: Prevents multiple launcher instances from running simultaneously

### Product Library
- **Digital Product Management**: View and manage your purchased products
- **Real-Time Status**: Track product availability (Online/Offline/Updating)
- **Expiration Tracking**: Visual indicators for product expiration dates
- **Lifetime Products**: Support for both time-limited and lifetime products

### Dashboard & News
- **Welcome Dashboard**: Personalized welcome screen with user information
- **Announcement System**: Stay updated with latest news and updates
- **Update History**: Track version changes and improvements

<br>

## 🛠️ Technical Details

### Built With
- **C++20** - Modern C++ with latest language features
- **DirectX 11** - Hardware-accelerated graphics rendering
- **ImGui** - Immediate mode GUI framework
- **FreeType** - Advanced font rendering with custom typography
- **HTTPLib** - Lightweight HTTP client for API communication
- **nlohmann/json** - Modern JSON parsing and serialization

### Dependencies
- DirectX SDK
- Windows SDK
- Visual Studio 2019/2022 (v145 toolset)
- vcpkg package manager

### Architecture
- **Modular Design**: Separated concerns with dedicated modules for API, UI, and core functionality
- **Async Operations**: Non-blocking authentication and API calls
- **Memory Management**: Smart pointers and RAII principles
- **Error Handling**: Comprehensive error handling with user-friendly messages

<br>

## 📋 Requirements

- **Operating System**: Windows 10 or later
- **Architecture**: x64 (64-bit)
- **Graphics**: DirectX 11 compatible graphics card
- **Memory**: 4GB RAM minimum
- **Storage**: 50MB available space

<br>

## How To Use

1. Clone the repository:
   ```bash
   git clone https://github.com/OnyxOrg/onyx-launcher.git
   cd onyx-launcher
   ```

2. Install dependencies via vcpkg:
   ```bash
   vcpkg install cpr:x64-windows
   vcpkg install libcurl:x64-windows
   ```

3. Open `Onyx Launcher.sln` in Visual Studio
4. Select `Release x64` configuration
5. Build the solution

<br>

> This project is intended for internal/private use. If you're part of the team, please refer to internal documentation or ask a maintainer for access/setup instructions. If you need access or support, contact the Onyx development team privately.


© 2025 Onyx Team. All Rights Reserved.