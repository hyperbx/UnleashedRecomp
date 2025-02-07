#pragma once

enum class ESpriteType
{
    SonicDash,
    SonicPush
};

class Sprite
{
public:
    static inline bool s_isVisible = false;

    static void Init();
    static void Draw();
    static void Show(float x, float y, float scale, ESpriteType type);
    static void Hide();
};
