#include <kernel/function.h>
#include <api/SWA.h>
#include <apu/embedded_player.h>
#include <gpu/video.h>
#include <ui/achievement_menu.h>
#include <ui/button_guide.h>
#include <ui/game_window.h>
#include <ui/imgui_utils.h>
#include <ui/options_menu.h>
#include <ui/sprite.h>
#include <locale/locale.h>
#include <app.h>

#define PULSE_TIMER_THRESHOLD 0.6f

bool g_isClosed;
bool g_isAchievementsAccessed = false;
bool g_isAchievementsExitFinished = false;
bool g_isOptionsAccessed = false;

ESpriteType g_spriteType = ESpriteType::SonicDash;
float g_spriteX = 0.0f;
float g_buttonGuideSideMargin = 379.0f;
float g_pulseTimer = PULSE_TIMER_THRESHOLD;

float g_achievementMenuIntroTime = 0.0f;
constexpr float g_achievementMenuIntroThreshold = 3.0f;
float g_achievementMenuOutroTime = 0.0f;
constexpr float g_achievementMenuOutroThreshold = 0.32f;
bool g_isAchievementMenuOutro = false;

void CHudPauseAddOptionsItemMidAsmHook(PPCRegister& pThis)
{
    if (g_isOptionsAccessed)
        return;

    guest_stack_var<Hedgehog::Base::CSharedString> menu("TopMenu");
    guest_stack_var<Hedgehog::Base::CSharedString> name("option");

    GuestToHostFunction<int>(sub_824AE690, pThis.u32, menu.get(), name.get());
}

bool InjectMenuBehaviour(uint32_t pThis, uint32_t count)
{
    if (App::s_isLoading)
        return true;

    auto pHudPause = (SWA::CHudPause*)g_memory.Translate(pThis);
    auto cursorIndex = *(be<uint32_t>*)g_memory.Translate(4 * (*(be<uint32_t>*)g_memory.Translate(pThis + 0x19C) + 0x68) + pThis);

    auto actionType = SWA::eActionType_Undefined;
    auto transitionType = SWA::eTransitionType_Undefined;

    switch (pHudPause->m_Menu)
    {
        case SWA::eMenuType_WorldMap:
        case SWA::eMenuType_Stage:
        case SWA::eMenuType_Misc:
            actionType = SWA::eActionType_Return;
            transitionType = SWA::eTransitionType_Quit;
            break;

        case SWA::eMenuType_Village:
        case SWA::eMenuType_Hub:
            actionType = SWA::eActionType_Return;
            transitionType = SWA::eTransitionType_Hide;
            break;
    }

    if (auto pInputState = SWA::CInputState::GetInstance())
    {
        if (pInputState->GetPadState().IsTapped(SWA::eKeyState_Select) && !g_isAchievementsAccessed)
        {
            g_spriteX = GameWindow::s_width;
            g_isAchievementsAccessed = true;
        }
    }

    if (pHudPause->m_Status == SWA::eStatusType_Accept)
    {
        if (cursorIndex == count - 2)
        {
            EmbeddedPlayer::Play("Mystery");

            g_isOptionsAccessed = true;

            GuestToHostFunction<int>(0x824AFD28, pHudPause, 0, 0, 0, 1);

            if (pHudPause->m_rcBg1Select)
                pHudPause->m_rcBg1Select->SetHideFlag(true);

            return true;
        }
        else if (cursorIndex == count - 1)
        {
            pHudPause->m_Action = actionType;
            pHudPause->m_Transition = transitionType;

            return true;
        }
    }

    return false;
}

bool CHudPauseItemCountMidAsmHook(PPCRegister& pThis, PPCRegister& count)
{
    if (!g_isOptionsAccessed)
        count.u32 += 1;

    return InjectMenuBehaviour(pThis.u32, count.u32);
}

void CHudPauseVillageItemCountMidAsmHook(PPCRegister& pThis, PPCRegister& count)
{
    if (!g_isOptionsAccessed)
        count.u32 += 1;

    InjectMenuBehaviour(pThis.u32, count.u32);
}

bool CHudPauseMiscItemCountMidAsmHook(PPCRegister& count)
{
    if (!g_isOptionsAccessed && count.u32 < 3)
        return true;

    return false;
}

bool CHudPauseMiscInjectOptionsMidAsmHook(PPCRegister& pThis)
{
    return InjectMenuBehaviour(pThis.u32, g_isOptionsAccessed ? 2 : 3);
}

// SWA::CHudPause::Update
PPC_FUNC_IMPL(__imp__sub_824B0930);
PPC_FUNC(sub_824B0930)
{
    if (App::s_isLoading)
    {
        __imp__sub_824B0930(ctx, base);
        return;
    }

    auto pHudPause = (SWA::CHudPause*)g_memory.Translate(ctx.r3.u32);
    auto pInputState = SWA::CInputState::GetInstance();

    g_achievementMenuIntroTime += App::s_deltaTime;

    if (g_isAchievementMenuOutro)
    {
        g_achievementMenuOutroTime += App::s_deltaTime;

        // Re-open pause menu after achievement menu closes with delay.
        if (g_achievementMenuOutroTime >= g_achievementMenuOutroThreshold)
        {
            GuestToHostFunction<int>(sub_824AFD28, pHudPause, 0, 1, 0, 0);

            g_achievementMenuOutroTime = 0;
            g_isAchievementMenuOutro = false;
        }
    }

    if (AchievementMenu::s_isVisible)
    {
        // HACK: wait for transition to finish before restoring control.
        if (g_achievementMenuIntroThreshold >= g_achievementMenuIntroTime)
            __imp__sub_824B0930(ctx, base);

        if (pInputState->GetPadState().IsTapped(SWA::eKeyState_B) && !g_isAchievementMenuOutro)
        {
            AchievementMenu::Close();

            g_isAchievementMenuOutro = true;
        }
    }
    else if (OptionsMenu::s_isVisible && OptionsMenu::s_isPause)
    {
        if (OptionsMenu::CanClose() && pInputState->GetPadState().IsTapped(SWA::eKeyState_B))
        {
            OptionsMenu::Close();
            GuestToHostFunction<int>(sub_824AFD28, pHudPause, 0, 0, 0, 1);
            __imp__sub_824B0930(ctx, base);
        }
    }
    else
    {
        g_achievementMenuIntroTime = 0;

        if (*SWA::SGlobals::ms_IsRenderHud && pHudPause->m_IsShown && !pHudPause->m_Submenu && pHudPause->m_Transition == SWA::eTransitionType_Undefined)
        {
            if (g_isAchievementsAccessed && !g_isAchievementsExitFinished)
            {
                auto drawList = ImGui::GetForegroundDrawList();
                auto& res = ImGui::GetIO().DisplaySize;

                auto y = res.y - Scale(105);
                auto scale = Scale(40);

                if (g_spriteType == ESpriteType::SonicDash)
                {
                    g_spriteX = Lerp(g_spriteX, 0.0f, App::s_deltaTime / 2);

                    if (g_spriteX <= g_buttonGuideSideMargin + Scale(310))
                        g_spriteType = ESpriteType::SonicPush;
                }
                else if (g_spriteType == ESpriteType::SonicPush)
                {
                    g_spriteX = Lerp(g_spriteX, Scale(-350), App::s_deltaTime / 4);
                    g_buttonGuideSideMargin = Lerp(g_buttonGuideSideMargin, Scale(-350), App::s_deltaTime / 4);
                    g_pulseTimer += App::s_deltaTime;

                    if (g_pulseTimer >= PULSE_TIMER_THRESHOLD)
                    {
                        EmbeddedPlayer::Play("Push");
                        g_pulseTimer = 0.0f;
                    }

                    ButtonGuide::SetSideMargins(g_buttonGuideSideMargin);

                    if (g_spriteX + scale <= 0)
                        g_isAchievementsExitFinished = true;
                }

                Sprite::Show(g_spriteX, y, scale, g_spriteType);
            }

            if (!g_isAchievementsExitFinished)
                ButtonGuide::Open(Button("Achievements_Name", FLT_MAX, EButtonIcon::Back, EButtonAlignment::Left, EFontQuality::Low), !g_isAchievementsAccessed);

            g_isClosed = false;
        }
        else if (!g_isClosed)
        {
            ButtonGuide::Close();
            g_isClosed = true;
        }

        __imp__sub_824B0930(ctx, base);
    }
}
