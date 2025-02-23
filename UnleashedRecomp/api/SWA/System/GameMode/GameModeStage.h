#pragma once

#include <SWA.inl>

namespace SWA
{
    class CGameModeStage : public CGameMode
    {
    public:
        xpointer<void> m_pVftable;
        SWA_INSERT_PADDING(0x114);
        be<float> m_GameSpeed;
        SWA_INSERT_PADDING(0x9C);
    };

    SWA_ASSERT_OFFSETOF(CGameModeStage, m_GameSpeed, 0x180);
    SWA_ASSERT_SIZEOF(CGameModeStage, 0x220);
}
