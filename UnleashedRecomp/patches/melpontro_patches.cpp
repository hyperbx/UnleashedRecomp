#include "melpontro_patches.h"
#include <api/SWA.h>
#include <hid/hid.h>
#include <os/logger.h>
#include <ui/message_window.h>
#include <app.h>

SWA::CGameModeStage* g_pGameModeStage;

bool g_showMessage;
bool g_showAdabatMessage;
size_t g_messageIndex;
float g_timeSinceMessageShown;
std::string g_lastMessageStage;

float g_badAppleQueueTime;
bool g_isBadAppleQueued;

uint32_t g_pStringPool;
uint32_t g_stringPoolSize;

int g_loadCount = 0;

VideoStringSequence g_videoStringSequence{};

size_t GetTotalStrlen(const std::vector<const char*> strs)
{
    size_t result = 0;

    for (auto str : strs)
        result += strlen(str) + 1;

    return result;
}

void CreateStringPool(const std::vector<const char*> strs)
{
    void* pStrMemory = __HH_ALLOC(GetTotalStrlen(strs));
    void* pTblMemory = __HH_ALLOC(strs.size() * 4);

    size_t poolLength = 0;
    std::vector<uint32_t> table{};

    for (const char* str : strs)
    {
        auto pStr = (char*)((size_t)pStrMemory + poolLength);
        table.push_back(ByteSwap(g_memory.MapVirtual(pStr)));
        strcpy(pStr, str);
        poolLength += strlen(str) + 1;
    }

    size_t tableLength = 0;

    for (uint32_t ptr : table)
    {
        memcpy((void*)((size_t)pTblMemory + tableLength), &ptr, 4);
        tableLength += 4;
    }

    g_pStringPool = g_memory.MapVirtual(pTblMemory);
    g_stringPoolSize = table.size();
}

void FreeStringPool()
{
    if (!g_pStringPool)
        return;

    // Free string pool and offsets.
    __HH_FREE(((be<uint32_t>*)g_memory.Translate(g_pStringPool))->get());
    __HH_FREE(g_pStringPool);

    g_pStringPool = 0;
    g_stringPoolSize = 0;
}

std::mt19937 GetRandom()
{
    std::tuple<uint32_t, size_t> tpl{};
    std::random_device rd;
    std::mt19937 rd_gen(rd());

    return rd_gen;
}

void Mel_MsgRequestStartLoadingMidAsmHook(PPCRegister& pThis, PPCRegister& r4)
{
    auto pType = (be<uint32_t>*)g_memory.Translate(r4.u32 + 0x18);
    auto rng = GetRandom();

    LOGFN("Loading type: {}", pType->get());

    std::bernoulli_distribution scrollingTextDist(0.5);

    // Randomly change loading screen to use scrolling text variant.
    if (g_isBadAppleQueued || (scrollingTextDist(rng) && *pType == 3))
        *pType = 0;

    if (*pType == 0)
    {
        g_loadCount++;
    }
    else
    {
        return;
    }

    // Prevent additional calls with type 0 running this code.
    if (g_loadCount > 1)
        return;

    FreeStringPool();

    if (g_isBadAppleQueued)
    {
        g_videoStringSequence = BadAppleSequence();
        g_videoStringSequence.Play();
    }
    else
    {
        std::bernoulli_distribution singleOrMultiDist(0.5);

        if (singleOrMultiDist(rng))
        {
            std::shuffle(g_multiStringSequences.begin(), g_multiStringSequences.end(), rng);

            CreateStringPool(g_multiStringSequences[0].Strings);

            if (g_multiStringSequences[0].Callback)
                g_multiStringSequences[0].Callback();
        }
        else
        {
            std::shuffle(g_singleStringSequences.begin(), g_singleStringSequences.end(), rng);
        
            CreateStringPool(g_singleStringSequences);
        }
    }

    if (!g_stringPoolSize)
        return;

    auto pScrollIndex = (be<uint32_t>*)g_memory.Translate(pThis.u32 + 0x124);

    *pScrollIndex = g_stringPoolSize - (g_stringPoolSize / 4);
}

bool Mel_VideoStringSequenceWaitMidAsmHook(PPCRegister& pThis)
{
    auto type = *(be<uint32_t>*)g_memory.Translate(pThis.u32 + 0x138);

    if (type == 0)
        return g_videoStringSequence.IsPlaying();

    return false;
}

void Mel_SetLoadingStringsMidAsmHook(PPCRegister& pThis, PPCRegister& pCSDText, PPCRegister& pUpdateInfo, PPCRegister& r30)
{
    auto deltaTime = *(be<float>*)g_memory.Translate(pUpdateInfo.u32);
    auto outroTime = *(be<float>*)g_memory.Translate(pThis.u32 + 0x140);
    auto pScrollIndex = (be<uint32_t>*)g_memory.Translate(pThis.u32 + 0x124);

    if (outroTime > 0.0f && outroTime < 0.01f)
        g_loadCount = 0;

    if (g_videoStringSequence.IsPlaying())
    {
        g_videoStringSequence.Update(deltaTime);

        *pScrollIndex = 0;
    }

    if (!g_stringPoolSize)
        return;

    auto scrollIndex = (*pScrollIndex + r30.u32) % g_stringPoolSize;

    pCSDText.u32 = *(be<uint32_t>*)g_memory.Translate((size_t)g_pStringPool + scrollIndex * 4);
}

// SWA::CGameModeStage::CGameModeStage
PPC_FUNC_IMPL(__imp__sub_82541138);
PPC_FUNC(sub_82541138)
{
    g_pGameModeStage = (SWA::CGameModeStage*)g_memory.Translate(ctx.r3.u32);

    __imp__sub_82541138(ctx, base);
}

// SWA::CObjEntranceDoor::MsgHitEventCollision::Impl
PPC_FUNC_IMPL(__imp__sub_8273EAE8);
PPC_FUNC(sub_8273EAE8)
{
    if (auto pGameDocument = SWA::CGameDocument::GetInstance())
    {
        auto& rStageName = pGameDocument->m_pMember->m_StageName;

        // Only display this message once per entrance stage.
        if (strcmp(g_lastMessageStage.c_str(), rStageName.c_str()) != 0)
        {
            static bool shownAdabatMessage;

            if (!shownAdabatMessage && std::strstr(rStageName.c_str(), "SouthEastAsia"))
            {
                g_showAdabatMessage = true;
                shownAdabatMessage = true;
            }

            g_showMessage = true;
            g_timeSinceMessageShown = App::s_time;
        }

        g_lastMessageStage = rStageName.c_str();
    }

    __imp__sub_8273EAE8(ctx, base);
}

PPC_FUNC_IMPL(__imp__sub_8250F2B8);
PPC_FUNC(sub_8250F2B8)
{
    PPC_STORE_U8(0x82032835, (strcmp(reinterpret_cast<char*>(base + PPC_LOAD_U32(ctx.r4.u32)), "bgm_ClockTown") == 0) ? '2' : '\0');

    __imp__sub_8250F2B8(ctx, base);
}

void MelpontroPatches::Update()
{
    auto keyboardState = SDL_GetKeyboardState(NULL);

    if (keyboardState[SDL_SCANCODE_F5] && !g_isBadAppleQueued)
    {
        EmbeddedPlayer::Play("Jarate");
        g_badAppleQueueTime = App::s_time;
        g_isBadAppleQueued = true;
    }

    if (g_isBadAppleQueued)
    {
        if (App::s_time - g_badAppleQueueTime > 1.1745f)
            Config::XboxColorCorrection = true;
    }

    if (g_showMessage)
    {
        static int result = -1;

        g_pGameModeStage->m_GameSpeed = 0.001f;

        auto msg = g_gateMessages[g_messageIndex];
        auto msgText = std::get<0>(msg);
        auto msgButtons = std::get<1>(msg);

        if (g_showAdabatMessage)
        {
            msgText = "amazing looking water in this game";
            msgButtons = { "Yeah!" };
        }

        if (msgButtons.empty())
            hid::SetProhibitedInputs(App::s_time - g_timeSinceMessageShown > 1.5f ? 0 : 0xFFFF);

        if (MessageWindow::Open(msgText, &result, msgButtons) == MSG_CLOSED)
        {
            result = -1;

            g_pGameModeStage->m_GameSpeed = 1.0f;

            g_showMessage = false;

            if (!g_showAdabatMessage)
                g_messageIndex = (g_messageIndex + 1) % g_gateMessages.size();

            g_showAdabatMessage = false;
        }
    }

    if (auto pGameDocument = SWA::CGameDocument::GetInstance())
    {
        if (pGameDocument->m_pMember->m_StageName.empty())
            g_lastMessageStage = "";
    }
}
