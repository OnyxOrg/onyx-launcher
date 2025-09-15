#include "../../includes/core/functional/animations.hpp"
#define PI 3.14159265358979323846f

float Animations::eOutBounce(float t)
{
    float n1 = 7.5625f;
    const float d1 = 2.75f;

    if (t < 1.0f / d1)
        return n1 * t * t;

    else if (t < 2.0f / d1)
    {
        float reduced_t = t - 1.5f / d1;
        return n1 * reduced_t * reduced_t + 0.75f;
    }

    else if (t < 2.5f / d1)
    {
        float reduced_t = t - 2.25f / d1;
        return n1 * reduced_t * reduced_t + 0.9375f;
    }

    else
    {
        float reduced_t = t - 2.625f / d1;
        return n1 * reduced_t * reduced_t + 0.984375f;
    }
}

float Animations::eInOutQuad(float t)
{
    return t < 0.5f ? 2 * t * t : -1 + (4 - 2 * t) * t;
}

float Animations::eInOutCubic(float t)
{
    return t < 0.5 ? 4 * t * t * t : 1 - std::pow(-2 * t + 2, 3) / 2;
}

float Animations::eInOutQuart(float t)
{
    return t < 0.5f ? 8 * t * t * t * t : 1 - std::pow(-2 * t + 2, 4) / 2;
}

float Animations::eInOutQuint(float t)
{
    return t < 0.5f ? 16 * t * t * t * t * t : 1 - std::pow(-2 * t + 2, 5) / 2;
}

float Animations::eInOutSine(float t)
{
    return -(std::cos(PI * t) - 1) / 2;
}

float Animations::eInOutExpo(float t)
{
    if (t == 0.0f) return 0.0f;
    if (t == 1.0f) return 1.0f;
    return t < 0.5f
        ? std::pow(2, 20 * t - 10) / 2
        : (2 - std::pow(2, -20 * t + 10)) / 2;
}

float Animations::eInOutCirc(float t)
{
    return t < 0.5f
        ? (1 - std::sqrt(1 - 4 * t * t)) / 2
        : (std::sqrt(1 - std::pow(-2 * t + 2, 2)) + 1) / 2;
}

float Animations::eInOutElastic(float t)
{
    const float c5 = (2 * PI) / 4.5;
    return t == 0 ? 0 : t == 1 ? 1 : t < 0.5 ? -(std::pow(2, 20 * t - 10) * std::sin((20 * t - 11.125) * c5)) / 2 : (std::pow(2, -20 * t + 10) * std::sin((20 * t - 11.125) * c5)) / 2 + 1;
}

float Animations::eInOutBack(float t)
{
    const float c1 = 1.70158f;
    const float c2 = c1 * 1.525f;
    return t < 0.5f
        ? (std::pow(2 * t, 2) * ((c2 + 1) * 2 * t - c2)) / 2
        : (std::pow(2 * t - 2, 2) * ((c2 + 1) * (t * 2 - 2) + c2) + 2) / 2;
}

float Animations::eInOutBounce(float t)
{
    return t < 0.5f
        ? (1.0f - eOutBounce(1.0f - 2.0f * t)) * 0.5f
        : (1.0f + eOutBounce(2.0f * t - 1.0f)) * 0.5f;
}

float Animations::GetEasing(float t, Easings type)
{
    switch (type)
    {
    case Easings::InOutQuad: return eInOutQuad(t);
    case Easings::InOutCubic: return eInOutCubic(t);
    case Easings::InOutQuart: return eInOutQuart(t);
    case Easings::InOutQuint: return eInOutQuint(t);
    case Easings::InOutSine: return eInOutSine(t);
    case Easings::InOutExpo: return eInOutExpo(t);
    case Easings::InOutCirc: return eInOutCirc(t);
    case Easings::InOutElastic: return eInOutElastic(t);
    case Easings::InOutBack: return eInOutBack(t);
    case Easings::InOutBounce: return eInOutBounce(t);
    case Easings::OutBounce: return eOutBounce(t);
    }
}

float Animations::Animate(const std::string& id, bool callback, float start, float end, float speed, Easings type)
{
    static std::map<std::string, Anim> a;
    auto& anim = a[id];

    if (callback != anim.condition)
        anim.time = 0.0f;
    else
        anim.time = std::min(anim.time + 0.1f * speed, 1.0f);

    anim.condition = callback;

    if (anim.condition)
        anim.init = true;

    float t = GetEasing(anim.time, type);
    float result = anim.init ? ImLerp(start, end, callback ? t : 1.0f - t) : start;

    return result;
}

vec2 Animations::Animate(const std::string& id, bool callback, ImVec2 start, ImVec2 end, float speed, Easings type)
{
    static std::map<std::string, Anim> a;
    auto& anim = a[id];

    if (callback != anim.condition)
        anim.time = 0.0f;
    else
        anim.time = std::min(anim.time + 0.1f * speed, 1.0f);

    anim.condition = callback;

    if (anim.condition)
        anim.init = true;

    float t = GetEasing(anim.time, type);
    ImVec2 result;

    result.x = callback ? t * (end.x - start.x) + start.x : t * (start.x - end.x) + end.x;
    result.y = callback ? t * (end.y - start.y) + start.y : t * (start.y - end.y) + end.y;

    ImVec2 rResult = anim.init ? result : start;

    return rResult;
}