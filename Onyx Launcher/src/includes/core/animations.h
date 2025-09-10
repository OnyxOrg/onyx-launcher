#pragma once

#include <iostream>
#include <map>
#include "imgui.h"
#include "imgui_internal.h"

typedef ImVec2 vec2;

class Animations // Mainly IN-OUT animations, rest is kinda pointless
{
public:
	enum Easings
	{
		InOutQuad, InOutCubic, InOutQuart,
		InOutQuint, InOutSine, InOutExpo,
		InOutCirc, InOutElastic, InOutBack,
		InOutBounce, OutBounce
	};

	float Animate(const std::string& id, bool callback, float start, float end, float speed, Easings type);
	vec2 Animate(const std::string& id, bool callback, ImVec2 start, ImVec2 end, float speed, Easings type);
private:
	float eInOutQuad(float t);    float eInOutCubic(float t);
	float eInOutQuart(float t);   float eInOutQuint(float t);
	float eInOutSine(float t);    float eInOutExpo(float t);
	float eInOutCirc(float t);    float eInOutElastic(float t);
	float eInOutBack(float t);    float eInOutBounce(float t);
	float eOutBounce(float t);

	float GetEasing(float t, Easings type);

	struct Anim
	{
		float time;
		bool condition, init;
	};
}; inline std::unique_ptr<Animations> animations = std::make_unique<Animations>();

