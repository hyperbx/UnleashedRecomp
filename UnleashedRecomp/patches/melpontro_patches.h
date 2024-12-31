#pragma once

#include <apu/embedded_player.h>
#include <res/melpontro/frames/BadApple/vector.h>
#include <user/config.h>

static float g_origMasterVolume = 1.0f;

size_t GetTotalStrlen(const std::vector<const char*> strs);
void CreateStringPool(const std::vector<const char*> strs);
void FreeStringPool();

class MultiStringSequence
{
public:
    std::vector<const char*> Strings;
    std::function<void()> Callback;

    MultiStringSequence(const std::vector<const char*> strs) : Strings(strs) {}
    MultiStringSequence(const std::vector<const char*> strs, const std::function<void()> callback) : Strings(strs), Callback(callback) {}
};

class VideoStringSequence
{
protected:
    bool m_isStarted{};
    bool m_isPlaying{};

    size_t m_currentFrame{};
    float m_frameTimer{};

public:
    std::vector<std::vector<const char*>> Frames{};
    std::function<void()> PlayCallback{ nullptr };
    std::function<void()> StopCallback{ nullptr };
    float UpdateRate{};
    bool IsFinished{};

    void Play()
    {
        g_origMasterVolume = Config::MasterVolume;

        Config::MasterVolume = 0.0f;

        m_isPlaying = true;
    }

    void Pause()
    {
        m_isPlaying = false;
    }

    void Stop()
    {
        m_isPlaying = false;

        m_isStarted = false;
        m_currentFrame = 0;
        m_frameTimer = 0.0f;

        if (StopCallback)
            StopCallback();

        Config::MasterVolume = g_origMasterVolume;
    }

    void Update(float deltaTime)
    {
        if (m_isPlaying)
        {
            FreeStringPool();

            if (!m_isStarted)
            {
                if (PlayCallback)
                    PlayCallback();

                m_isStarted = true;
            }

            m_frameTimer += deltaTime;

            if (m_frameTimer >= UpdateRate)
            {
                m_currentFrame++;
                m_frameTimer = 0.0f;
            }

            if (m_currentFrame >= Frames.size())
            {
                Stop();

                IsFinished = true;
            }
            else
            {
                CreateStringPool(Frames[m_currentFrame]);
            }
        }
    }

    bool IsPlaying()
    {
        return m_isPlaying && !IsFinished;
    }
};

class BadAppleSequence : public VideoStringSequence
{
public:
    BadAppleSequence()
    {
        Frames = g_frmBadApple;

        PlayCallback = []()
        {
            EmbeddedPlayer::Play("BadApple");
        };

        UpdateRate = 0.62f;
    }
};

std::vector<const char*> g_singleStringSequences =
{
    "10 YEARS IN THE JOINT MADE YOU A FUCKING PUSSY",
    "SONIC UNLEASHED SUCKS",
    "SONIC HEROES IS PEAK",
    "MELPONTRO IS BEAUTIFUL",
    "(NOT) SPONSORED BY TACO BELL",
    "10 YEARS IN THE PUSSY MADE YOU A FUCKING JOINT",
    "I falling",
    "IS THIS PART OF MELPONTERATIONS?",
    "why are there so many icecream images in knuckles unleashed :screaming:",
    "WHY IS MY MESSAGE THERE",
    "HELLO! I'M EMU OTORI!! EMU IS MEANING SMILE!!!!!",
    "NOW LOADING...",
    "Chat Is This Real?",
    "This is Xenia",
    "ADACHEY",
    "NO WAY! NO WAY? NO WAY! NO WAY? NO WAY! NO WAY?",
    "this is not sequel, it will be brand new experience",
    "Welcome To the leveldesign forest",
    "The programmer has a nap. Hold out! Programmer!",
    "Sliced thinly, if you please.",
    "Without the Time Stones?",
    "This is RPCS3",
    "Coo...",
    "I'm something of a scientist myself.",
    "I'M WITH THE SCIENCE TEAM",
    "imgui sega balls!"
};

std::vector<MultiStringSequence> g_multiStringSequences =
{
    { { "nullptr" } },
    { { "Graphics device lost (probably due to an internal error)" } },
    { { "Fatal Crash Intercepted!" }, []() { EmbeddedPlayer::Play("XboxNotify"); }},
    {
        {
            " ",
            "                                  * HACK DETECTION *                                       ",
            "                  ONE OR MORE GAME RESOURCES WERE MANIPULATED BY AN                        ",
            "                  OUTSIDE SOURCE. THIS IS NOT ALLOWED AS SPECIFIED IN                      ",
            "                  THE GAME LICENSE.                                                        ",
            "                  YOU MUST REINSTALL THE GAME AND ACCEPT THE GAME                          ",
            "                  LICENSE AGAIN, TO CONTINUE TO PLAY THE GAME.                             ",
            "                                                                                           ",
            "                  GAME HALTED.                                                             ",
            " "
        }
    },
    {
        {
            "Pele The Beloved Dog was taking a long walk on the beach enjoying the brilliant scenery",
            "of the Soleanna coast. He was lost deep in thought, trying to unravel the secrets of the",
            "universe. However, this deep thought was soon broken by a shriek of terror.",
            " ",
            "Pele The Beloved Dog raced towards the noise. He saw a helpless person being tormented by",
            "robots from the nefarious Dr.Eggman. Pele The Beloved Dog could not stand by and do",
            "nothing.",
            " ",
            "He raced toward the robot and quickly defeated it and saved the poor soul. The person",
            "graciously thank Pele The Beloved Dog but told him some disturbing news. The entire beach",
            "had been taking over by evil Eggman robots.",
            " ",
            "Pele The Beloved Dog springs into action to save the world from Dr.Eggman's Tyranny.",
            "Only time will tell what adventures Pele The Beloved Dog will go on.",
            " "
        }
    },
    {
        {
            " ",
            " ",
            "                                  ******************                                       ",
            "                              ****                  ****                                   ",
            "                            **                          **                                 ",
            "                            **                          **                                 ",
            "                          **                              **                               ",
            "                          **                    ******    **                               ",
            "                          **                    ******    **                               ",
            "                          **    ******    **    ******    **                               ",
            "                            **          ******          **        H E Y                    ",
            "                          ****  **                  **  ****                               ",
            "                          **    **********************    **                               ",
            "                          **      **  **  **  **  **      **                               ",
            "                            ****    **************    ****                                 ",
            "                          **********              **********                               ",
            "                        **  ******************************  **                             ",
            "                      ****  **    **      **      **    **  ****                           ",
            "                      **      **    ******  ******    **      **                           ",
            "                    **    ****  ******    **    ******  ****    **                         ",
            "                    **        **    **          **    **        **                         ",
            "                    **          **  **          **  **          **                         ",
            "                      **      **    ****      ****    **      **                           ",
            "                        ****  **    **          **    **  ****                             ",
            "                          ******    **************    ******                               ",
            "                            ****    **************    ****                                 ",
            "                          **********************************                               ",
            "                          ****************  ****************                               ",
            "                            ************      ************                                 ",
            "                        ******        **      **        ******                             ",
            "                        **          ****      ****          **                             ",
            "                          **********              **********                               "
        },
        []() { EmbeddedPlayer::Play("Sans"); }
    },
    {
        {
            " ",
            "Uh-oh!",
            " ",
            "The guest has crashed.",
            " ",
            "Xenia has now paused itself.",
            "A crash dump has been written into the log.",
            " ",
        }
    },
    {
        {
            " ",
            " ",
            "                                        ######                                             ",
            "                                   ##################                                      ",
            "                                 #####++++++++++#######                                    ",
            "                                ####+++++++++++++++++####                                  ",
            "                               ####+++++++++++++++++++####                                 ",
            "                               ###++++++++++###############                                ",
            "                              ####+++++++####################                              ",
            "                              ###+++++++###+----........--####                             ",
            "                              ###+++++++###+-----.........-+###                            ",
            "                        #########+++++++###++---------------####     S U S                 ",
            "                      #####+++###+++++++###+++++++++++++++++####                           ",
            "                     ####+++++###+++++++####++++++++++++++++###                            ",
            "                    ####++++++###++++++++####+++++++++++++#####                            ",
            "                    ####++++++###+++++++++####################                             ",
            "                    ###++++++####++++++++++++###########+++###                             ",
            "                    ###++++++####++++++++++++++++++++++++++###                             ",
            "                    ###++++++####++++++++++++++++++++++++++###                             ",
            "                    ###++++++####++++++++++++++++++++++++++###                             ",
            "                    ###++++++####++++++++++++++++++++++++++###                             ",
            "                    ###++++++####++++++++++++++++++++++++++###                             ",
            "                    ###++++++###+++++++++++++++++++++++++++###                             ",
            "                    ####+++++###+++++++++++++++++++++++++++###                             ",
            "                     ####++++###+++++++++++++++++++++++++++###                             ",
            "                      ##########++++++++++############++++####                             ",
            "                          ######+++++++++#########++++++++####                             ",
            "                             ###+++++++++####  ###++++++++###                              ",
            "                             ###+++++++++####  ###+++++++####                              ",
            "                             ####++++++++####   ####+++#####                               ",
            "                              ####++++++####     #########                                 ",
            "                               ############                                                ",
            "                                   #####                                                   ",
        },
        []() { EmbeddedPlayer::Play("AmongUs"); }
    },
    {
        {
            "Google Sheets is a spreadsheet application and part of the free, web-based Google Docs",
            "Editors suite offered by Google. Google Sheets is available as a web application; a mobile",
            "app for: Android, iOS, and as a desktop application on Google's ChromeOS. The app is",
            "compatible with Microsoft Excel file formats. The app allows users to create and edit",
            "files online while collaborating with other users in real-time. Edits are tracked by which",
            "user made them, along with a revision history. Where an editor is making changes is",
            "highlighted with an editor-specific color and cursor. A permissions system regulates what",
            "users can do. Updates have introduced features that use machine learning, including",
            "\"Explore\", which offers answers based on natural language questions in the spreadsheet.",
            "Sheets is one of the services provided by Google that also includes Google Docs, Google",
            "Slides, Google Drawings, Google Forms, Google Sites and Google Keep.",
            " "
        }
    },
    {
        {
            "Well, Peter, you see the McRib is a barbecue-flavored pork sandwich periodically sold by",
            "the international fast food restaurant chain McDonald's. It was first introduced to the",
            "McDonald's menu in in 1981, following test marketing the year before. After poor sales,",
            "it was removed from the menu in 1985. Since then, it appears annually or at various times,",
            "though Germany and Luxemburg have them as permanent menu items.",
            " "
        }
    },
    {
        {
            " ",
            "Critical Error!",
            " ",
            "The detected configuration does not match your current hardware.",
            "Please re-run the configuration tool.",
            " ",
        }
    },
    {
        {
            " ",
            " ",
            "                                        ..:.:::::....                                       ",   
            "                                                                                            ",   
            "                                   :....irLsu12uJ7r:....:                                   ",   
            "                                  ::sgBBBBQBBBBBBBBBBBEY::                                  ",   
            "                                 B5YBQBBBBBE1YJ1MBBBBBBBvQB                                 ",   
            "                                :Bi ...:ivR      Qvi::.. LQ.                                ",   
            "                                SB: ...   Zi....:D   ... iBY                                ",   
            "                                QB..::::. Sv ...iP .:::: :BQ                                ",   
            "                                BB..:::::.r. .. .7..:::...BB                                ",   
            "                               :BB..::::..7EBBQBPr.:::::..BB.                               ",   
            "                               rBB..::::.:BBBQQBBQ:.:::..:BBi                               ",   
            "                               vBB..:::....::iii::...::: iBBi                               ",   
            "                                BB7.:::.vuSISIS2522r..::.2BB                                ",   
            "                                 :..::.vBBBBBBQBBBBBr.::...                                 ",   
            "                                   .:.:...............:.:                                   ",   
            "                                 :Q.   .                .B.                                 ",   
            "                                 BBBSvr::.........::irYIBQB                                 ",   
            "                                 :jbRBQBBBBBBBBBQBBBBBBQKu.                                 ",
            " ",
            " "
        },
        []() { EmbeddedPlayer::Play("VineBoom"); }
    }
};
