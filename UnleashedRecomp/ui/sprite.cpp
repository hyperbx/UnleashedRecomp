#include "sprite.h"
#include <gpu/video.h>
#include <res/melpontro/images/sprSonicDash.dds.h>
#include <res/melpontro/images/sprSonicPush.dds.h>
#include <ui/imgui_utils.h>
#include <decompressor.h>

float g_x;
float g_y;
float g_scale;
ESpriteType g_type;

std::unique_ptr<GuestTexture> g_upSprSonicDash;
std::unique_ptr<GuestTexture> g_upSprSonicPush;

void Sprite::Init()
{
    g_upSprSonicDash = LOAD_ZSTD_TEXTURE(g_sprSonicDash);
    g_upSprSonicPush = LOAD_ZSTD_TEXTURE(g_sprSonicPush);
}

void Sprite::Draw()
{
    auto drawList = ImGui::GetForegroundDrawList();

    GuestTexture* texture = nullptr;
    int columns, rows, frames;
    float fps, textureWidth, textureHeight, spriteSize;

    switch (g_type)
    {
        case ESpriteType::SonicDash:
            texture = g_upSprSonicDash.get();
            columns = 4;
            rows = 1;
            frames = 4;
            fps = 20;
            textureWidth = 288;
            textureHeight = 72;
            spriteSize = 72;
            break;

        case ESpriteType::SonicPush:
            texture = g_upSprSonicPush.get();
            columns = 4;
            rows = 1;
            frames = 4;
            fps = 3;
            textureWidth = 296;
            textureHeight = 74;
            spriteSize = 74;
            break;

        default:
            return;
    }

    if (!texture)
        return;

    auto frameIndex = int32_t(floor(ImGui::GetTime() * fps)) % frames;
    auto columnIndex = frameIndex % columns;
    auto rowIndex = frameIndex / columns;
    auto uv0 = ImVec2(columnIndex * spriteSize / textureWidth, rowIndex * spriteSize / textureHeight);
    auto uv1 = ImVec2((columnIndex + 1) * spriteSize / textureWidth, (rowIndex + 1) * spriteSize / textureHeight);

    drawList->AddImage(texture, { g_x, g_y }, { g_x + g_scale, g_y + g_scale }, uv0, uv1);
}

void Sprite::Show(float x, float y, float scale, ESpriteType type)
{
    g_x = x;
    g_y = y;
    g_scale = scale;
    g_type = type;
}
