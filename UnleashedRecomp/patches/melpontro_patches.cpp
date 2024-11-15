#include <cpu/guest_code.h>
#include "api/SWA.h"
#include "melpontro_patches.h"

int m_loadCount = 0;

VideoStringSequence m_videoStringSequence{};

size_t GetTotalStrlen(const std::vector<const char*> strs)
{
    size_t result = 0;

    for (auto str : strs)
        result += strlen(str) + 1;

    return result;
}

void CreateStringPool(const std::vector<const char*> strs)
{
    printf("[*] ALLOCATING STRING POOL\n");

    void* pStrMemory = __HH_ALLOC(GetTotalStrlen(strs));
    void* pTblMemory = __HH_ALLOC(strs.size() * 4);

    size_t poolLength = 0;
    std::vector<uint32_t> table{};

    for (const char* str : strs)
    {
        auto pStr = (char*)((size_t)pStrMemory + poolLength);
        table.push_back(std::byteswap(g_memory.MapVirtual(pStr)));
        strcpy(pStr, str);
        poolLength += strlen(str) + 1;
    }

    size_t tableLength = 0;

    for (uint32_t ptr : table)
    {
        memcpy((void*)((size_t)pTblMemory + tableLength), &ptr, 4);
        tableLength += 4;
    }

    m_pStringPool = g_memory.MapVirtual(pTblMemory);
    m_stringPoolSize = table.size();
}

void FreeStringPool()
{
    if (!m_pStringPool)
        return;

    printf("[*] FREEING STRING POOL\n");

    // Free string pool and offsets.
    __HH_FREE((void*)((be<uint32_t>*)g_memory.Translate(m_pStringPool))->get());
    __HH_FREE((void*)m_pStringPool);

    m_pStringPool = 0;
    m_stringPoolSize = 0;
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

    printf("[*] Loading type: %d\n", pType->get());

    std::bernoulli_distribution scrollingTextDist(0.5);

    // Randomly change loading screen to use scrolling text variant.
    // TODO (Hyper): fix this being inconsistent.
    if (scrollingTextDist(rng) && *pType == 3)
        *pType = 0;

    if (*pType == 0)
    {
        m_loadCount++;
    }
    else
    {
        return;
    }

    // Prevent additional calls with type 0 running this code.
    if (m_loadCount > 1)
        return;

    FreeStringPool();

#if BAD_APPLE
    // TODO (Hyper): nullify chances of this occurring until after a specific stage?
    std::bernoulli_distribution badAppleDist(m_videoStringSequence.IsFinished ? 0 : 0.25);

    if (badAppleDist(rng))
    {
        m_videoStringSequence = BadAppleSequence();
        m_videoStringSequence.Play();
    }
    else
#endif
    {
        std::bernoulli_distribution singleOrMultiDist(0.5);

        if (singleOrMultiDist(rng))
        {
            std::shuffle(m_multiStringSequences.begin(), m_multiStringSequences.end(), rng);

            CreateStringPool(m_multiStringSequences[0].Strings);

            if (m_multiStringSequences[0].Callback)
                m_multiStringSequences[0].Callback();
        }
        else
        {
            std::shuffle(m_singleStringSequences.begin(), m_singleStringSequences.end(), rng);
        
            CreateStringPool(m_singleStringSequences);
        }
    }

    if (!m_stringPoolSize)
        return;

    auto pScrollIndex = (be<uint32_t>*)g_memory.Translate(pThis.u32 + 0x124);

    *pScrollIndex = m_stringPoolSize - (m_stringPoolSize / 4);
}

bool Mel_VideoStringSequenceWaitMidAsmHook(PPCRegister& pThis)
{
    auto type = *(be<uint32_t>*)g_memory.Translate(pThis.u32 + 0x138);

    // TODO (Hyper): mute game audio whilst playing.
    if (type == 0)
        return m_videoStringSequence.IsPlaying();

    return false;
}

void Mel_SetLoadingStringsMidAsmHook(PPCRegister& pThis, PPCRegister& pCSDText, PPCRegister& pUpdateInfo, PPCRegister& r30)
{
    auto deltaTime = *(be<float>*)g_memory.Translate(pUpdateInfo.u32);
    auto outroTime = *(be<float>*)g_memory.Translate(pThis.u32 + 0x140);
    auto pScrollIndex = (be<uint32_t>*)g_memory.Translate(pThis.u32 + 0x124);

    if (outroTime > 0.0f && outroTime < 0.01f)
        m_loadCount = 0;

    if (m_videoStringSequence.IsPlaying())
    {
        m_videoStringSequence.Update(deltaTime);

        *pScrollIndex = 0;
    }

    if (!m_stringPoolSize)
        return;

    auto scrollIndex = (*pScrollIndex + r30.u32) % m_stringPoolSize;

    pCSDText.u32 = *(be<uint32_t>*)g_memory.Translate((size_t)m_pStringPool + scrollIndex * 4);
}
