#pragma once

#include "main.hpp"
#include "colors.hpp"

// Simple fullscreen loading overlay with spinner
void RenderLoadingOverlay();

// Returns true during frames when the loading overlay is being rendered
bool IsOverlayActive();

// Query whether overlay is currently dragging the native window
bool IsOverlayDragging();

// Explicitly set overlay active state for the current frame
void SetOverlayActive(bool active);


