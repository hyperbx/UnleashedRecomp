#pragma once

enum class ESpriteType
{
    SonicDash,
    SonicPush
};

class Sprite
{
public:
    static void Init();
    static void Draw();
    static void Show(float x, float y, float scale, ESpriteType type);
};
