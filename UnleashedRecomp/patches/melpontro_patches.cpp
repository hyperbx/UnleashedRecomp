#include "melpontro_patches.h"
#include <api/SWA.h>
#include <os/logger.h>
#include <app.h>

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
    // TODO (Hyper): fix this being inconsistent.
    if (scrollingTextDist(rng) && *pType == 3)
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

    // TODO (Hyper): nullify chances of this occurring until after a specific stage?
    std::bernoulli_distribution badAppleDist(g_videoStringSequence.IsFinished ? 0 : 0.25);

    if (badAppleDist(rng))
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
