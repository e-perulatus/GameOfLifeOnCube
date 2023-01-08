#include <Arduino.h>
#include <M5Core2.h>

#define LGFX_M5STACK_CORE2
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <LGFX_AUTODETECT.hpp>

#include "MFrameWork.h"
#include "GameOfLifeOnCube.h"

namespace {
    LGFX g_Lcd;
    LGFX_Sprite g_BaseSprite[2] = {
        LGFX_Sprite(&g_Lcd),
        LGFX_Sprite(&g_Lcd)
    };

    QueueHandle_t g_Queue = NULL;
    uint32_t g_FrameCount = 0;
}

namespace {

    constexpr static const uint16_t BackGroundImageL[36 * 200] = 
    {
#include "GameOfLifeOnCube\Data\BackGround\L.h"
    };

    constexpr static const uint16_t BackGroundImageM[200 * 200] = 
    {
#include "GameOfLifeOnCube\Data\BackGround\M.h"
    };

    constexpr static const uint16_t BackGroundImageR[36 * 200] = 
    {
#include "GameOfLifeOnCube\Data\BackGround\R.h"
    };

}

namespace {

    class InputImpl : public Gol3d::IInput
    {
    public:
        virtual Gol3d::InputFlag Get() const noexcept final
        {
            auto flag = static_cast<uint8_t>(Gol3d::InputFlag::None);
            if (M5.BtnA.wasPressed())
            {
                flag |= static_cast<uint8_t>(Gol3d::InputFlag::Select);
            }
            if (M5.BtnB.wasPressed())
            {
                flag |= static_cast<uint8_t>(Gol3d::InputFlag::Enter);
            }
            if (M5.BtnC.wasPressed())
            {
                flag |= static_cast<uint8_t>(Gol3d::InputFlag::Cancel);
            }
            return static_cast<Gol3d::InputFlag>(flag);
        }
        virtual Gol3d::Position GetTouchMove() const noexcept final
        {
            Gol3d::Position ret = {};
            auto pressPoint = M5.Touch.getPressPoint();
            if ((pressPoint.x != -1 && pressPoint.y != -1) &&
                (m_PrevPressPoint.x != -1 && m_PrevPressPoint.y != -1))
            {
                ret = { pressPoint.x - m_PrevPressPoint.x, m_PrevPressPoint.y - pressPoint.y };
            }
            m_PrevPressPoint = pressPoint;
            return ret;
        }
    private:
        mutable Point m_PrevPressPoint;
    } g_Input;

    class SpriteImpl : public Gol3d::ISprite
    {
    public:
        SpriteImpl(LGFX_Sprite* pImpl) : m_pImpl(pImpl) {}
        virtual void PushImageWithAlphaBlend(uint16_t *pImage, Gol3d::Rect rect, Gol3d::Position pos, uint8_t alpha, uint8_t bgAlpha) noexcept final
        {
            if (alpha != 255)
            {
                for (int y = 0; y < rect.height; y++)
                {
                    for (int x = 0; x < rect.width; x++)
                    {
                        const int i = y * rect.width + x;

                        const uint16_t color = pImage[i];
                        const uint16_t bgColor = Gol3d::SwappedColor(static_cast<uint16_t>(m_pImpl->readPixelValue(x + pos.x, y + pos.y)));

                        if (color == Gol3d::ColorTransparent)
                        {
                            pImage[i] = Gol3d::SwappedColor(bgColor);
                        }
                        else
                        {
                            auto color565 = Gol3d::SwappedColor(color);
                            color565 = AlphaBlend(color != Gol3d::ColorBlack ? alpha : bgAlpha, color565, bgColor);
                            pImage[i] = Gol3d::SwappedColor(color565);
                        }
                    }
                }
            }
            
            m_pImpl->pushImage(pos.x, pos.y, rect.width, rect.height, pImage);
        }

        virtual void PushImageAffineWithAlphaBlend(uint16_t* pImage, Gol3d::Rect rect, float affine[6], uint8_t alpha, uint8_t bgAlpha) noexcept final
        {
            if (alpha != 255)
            {
                for (int y = 0; y < rect.height; y++)
                {
                    for (int x = 0; x < rect.width; x++)
                    {
                        const int dx = (int)(affine[0] * x + affine[1] * y + affine[2]);
                        const int dy = (int)(affine[3] * x + affine[4] * y + affine[5]);
                        const uint16_t color = pImage[y * rect.width + x];
                        const uint16_t bgColor = Gol3d::SwappedColor(static_cast<uint16_t>(m_pImpl->readPixelValue(dx, dy)));
                        if (color == Gol3d::ColorTransparent)
                        {
                            pImage[y * rect.width + x] = Gol3d::SwappedColor(bgColor);
                        }
                        else
                        {
                            auto color565 = Gol3d::SwappedColor(color);
                            color565 = AlphaBlend(color != Gol3d::ColorBlack ? alpha : bgAlpha, color565, bgColor);
                            pImage[y * rect.width + x] = Gol3d::SwappedColor(color565);
                        }
                    }
                }
            }

            m_pImpl->pushImageAffine(affine, rect.width, rect.height, pImage);
        }

    private:
        static uint16_t AlphaBlend(uint8_t alpha, uint16_t fgc, uint16_t bgc)
        {
            uint16_t fgR = ((fgc >> 10) & 0x3E) + 1;
            uint16_t fgG = ((fgc >>  4) & 0x7E) + 1;
            uint16_t fgB = ((fgc <<  1) & 0x3E) + 1;

            uint16_t bgR = ((bgc >> 10) & 0x3E) + 1;
            uint16_t bgG = ((bgc >>  4) & 0x7E) + 1;
            uint16_t bgB = ((bgc <<  1) & 0x3E) + 1;

            uint16_t r = (((fgR * alpha) + (bgR * (255 - alpha))) >> 9);
            uint16_t g = (((fgG * alpha) + (bgG * (255 - alpha))) >> 9);
            uint16_t b = (((fgB * alpha) + (bgB * (255 - alpha))) >> 9);

            return (r << 11) | (g << 5) | (b << 0);
        }

        LGFX_Sprite* m_pImpl;
    };
    
    SpriteImpl g_MySprite[2] = {
        SpriteImpl(&g_BaseSprite[0]),
        SpriteImpl(&g_BaseSprite[1])
    };

    Gol3d::ISprite* GetSprite() noexcept
    {
        return &g_MySprite[g_FrameCount % 2];
    }

}

void DrawTaskFunction(void*)
{
    uint32_t drawFrameCountPerSecond = 0;
    uint32_t drawFrameCount = 0;
    auto startTick = micros();

    while (true)
    {
        while (uxQueueMessagesWaiting(::g_Queue) == 0)
        {
            delay(1);
        }

        ::g_BaseSprite[(g_FrameCount + 1) % 2].pushSprite(24, 20);
        g_Lcd.setCursor(10, 10);
        g_Lcd.printf("fps: %2d\n", drawFrameCountPerSecond);
        drawFrameCount++;

        auto currentTick = micros();
        auto elapsedTick = currentTick - startTick;
        if (elapsedTick < 0)
        {
            startTick = currentTick;
            drawFrameCount = 0;
        }
        else if (elapsedTick >= 1000000ULL)
        {
            startTick = currentTick;
            drawFrameCountPerSecond = drawFrameCount;
            drawFrameCount = 0;
        }

        int command = 0;
        xQueueReceive(g_Queue, &command, 0);
    }
}

void setup()
{
    setCpuFrequencyMhz(240);

    M5.begin();
    ::g_Lcd.init();

    ::g_Queue = xQueueCreate(1, sizeof(int));
    xTaskCreatePinnedToCore(DrawTaskFunction, "DrawTask", 4096, nullptr, 1, nullptr, 0);

    {
        ::g_BaseSprite[0].createSprite(272, 200);
        ::g_BaseSprite[0].pushImage(0, 0, 36, 200, ::BackGroundImageL);
        ::g_BaseSprite[0].pushImage(272 - 36, 0, 36, 200, ::BackGroundImageR);
        ::g_BaseSprite[0].pushImage(36, 0, 200, 200, ::BackGroundImageM);
        ::g_BaseSprite[0].pushSprite(24, 20);
        ::g_BaseSprite[0].deleteSprite();

        ::g_BaseSprite[0].createSprite(236, 200);
        ::g_BaseSprite[1].createSprite(236, 200);

        MFW::AddObject(std::unique_ptr<MFW::IObject>(new Gol3d::StartIcon(::GetSprite, {0, 18})));
        MFW::AddObject(std::unique_ptr<MFW::IObject>(new Gol3d::PauseIcon(::GetSprite, {0, 54})));
        MFW::AddObject(std::unique_ptr<MFW::IObject>(new Gol3d::RandomizeIcon(::GetSprite, {0, 90})));
        MFW::AddObject(std::unique_ptr<MFW::IObject>(new Gol3d::InputEvent(&::g_Input)));
        MFW::AddObject(std::unique_ptr<MFW::IObject>(new Gol3d::Cube(::GetSprite, {136, 100}, 120, analogRead(26))));
    }
}

void loop()
{
    M5.update();
    MFW::Update();

    ::g_BaseSprite[g_FrameCount % 2].pushImage(0, 0, 36, 200, BackGroundImageL);
    ::g_BaseSprite[g_FrameCount % 2].pushImage(36, 0, 200, 200, BackGroundImageM);

    MFW::Draw();

    static int drawCmd = 0;
    xQueueSend(::g_Queue, &drawCmd, 0xffffffff);

    ::g_FrameCount++;
}