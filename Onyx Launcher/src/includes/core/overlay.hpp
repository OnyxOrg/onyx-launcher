#pragma once

#include "main.hpp"
#include "colors.hpp"

// Simple fullscreen loading overlay with spinner
void RenderLoadingOverlay();
void RenderLoadingOverlay(const char* label);
// Extended: allows fading out by multiplying final alpha in [0..1]
void RenderLoadingOverlayEx(const char* label, float alphaMultiplier);

// Returns true during frames when the loading overlay is being rendered
bool IsOverlayActive();

// Query whether overlay is currently dragging the native window
bool IsOverlayDragging();

// Explicitly set overlay active state for the current frame
void SetOverlayActive(bool active);


