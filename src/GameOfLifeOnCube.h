#pragma once

#include "MFrameWork.h"
#include "GameOfLifeOnCube/Type.h"

namespace Gol3d {

    constexpr uint16_t RgbTo16bitColor(uint8_t r, uint8_t g, uint8_t b)
    {
        return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    }

    constexpr uint16_t SwappedColor(uint16_t color)
    {
        return color << 8 | color >> 8;
    }

    constexpr uint16_t ColorBlack       = SwappedColor(RgbTo16bitColor(  0,   0,   0));
    constexpr uint16_t ColorRed         = SwappedColor(RgbTo16bitColor(255,   0,   0));
    constexpr uint16_t ColorGreen       = SwappedColor(RgbTo16bitColor(  0, 255,   0));
    constexpr uint16_t ColorBlue        = SwappedColor(RgbTo16bitColor(  0,   0, 255));
    constexpr uint16_t ColorTransparent = SwappedColor(0x0120);

    class ISprite
    {
    public:
        virtual ~ISprite() noexcept {}
        virtual void PushImageWithAlphaBlend(uint16_t *pImage, Rect rect, Position pos, uint8_t alpha, uint8_t bgAlpha) noexcept = 0;
        virtual void PushImageAffineWithAlphaBlend(uint16_t* pImage, Rect rect, float affine[6], uint8_t alpha, uint8_t bgAlpha) noexcept = 0;
    };

    typedef ISprite* (*GetSprite)();

    class IInput
    {        
    public:
        virtual ~IInput() noexcept {}
        virtual InputFlag Get() const noexcept = 0;
        virtual Position GetTouchMove() const noexcept = 0;
    };

namespace detail {

    struct IconContext
    {
        int kind;
        const uint16_t* pImageForOn;
        const uint16_t* pImageForOff;
        Rect rect;
    };

    constexpr static const int IconWidth = 36;
    constexpr static const int IconHeight = 36;

    template <const IconContext& Context>
    class IconBase : public MFW::IObject
    {
    public:
        IconBase(GetSprite getSprite, Position position) noexcept;

    public:
        virtual void Update() noexcept final;
        virtual void Draw() noexcept final;
        virtual uint8_t GetTypeValue() const noexcept final
        {
            return 1;
        }

    private:        
        GetSprite m_GetSprite;
        const IInput* m_pInput;
        const Position m_Position;
        const int m_Kind;
        bool m_Selected = false;

        alignas(32) uint16_t m_Buffer[IconWidth * IconHeight];
    };

    extern const IconContext StartIconContext;
    extern const IconContext PauseIconContext;
    extern const IconContext RandomizeIconContext;

}

    template class detail::IconBase<detail::StartIconContext>;
    typedef detail::IconBase<detail::StartIconContext> StartIcon;
    template class detail::IconBase<detail::PauseIconContext>;
    typedef detail::IconBase<detail::PauseIconContext> PauseIcon;
    template class detail::IconBase<detail::RandomizeIconContext>;
    typedef detail::IconBase<detail::RandomizeIconContext> RandomizeIcon;

    class Cube : public MFW::IObject
    {
    public:
        Cube(GetSprite pSprite, Position position, int length, long seed) noexcept;

    public:
        virtual void Update() noexcept final;
        virtual void Draw() noexcept final;
        virtual uint8_t GetTypeValue() const noexcept final
        {
            return 2;
        }

    private:
        Position NormalizePosition(float x, float) const noexcept;
        void DrawSurface(int surfaceIndex, const Position vertexPosition[8], const bool* pGameOfLifeBitmap) noexcept;

    private:
        GetSprite m_GetSprite;
        const Position m_Position;
        const int m_Length;

        Vector3d m_Vertex[8];
        Axes3d m_Attitude;
        Axes3d m_AttitudeDelta;

        const Vector3d m_WorldPosition = { 0.0, 0.0, 0.0 };
        const float m_Acceleration = 0.99;

        static const int BufferWidth = 64;
        static const int BufferHeight = 64;
        alignas(32) uint16_t m_Buffer[BufferWidth * BufferHeight];
    };

    class InputEvent : public MFW::IObject
    {
    public:
        InputEvent(IInput* pInput) noexcept;

    public:
        virtual void Update() noexcept final;
        virtual void Draw() noexcept final {}
        virtual uint8_t GetTypeValue() const noexcept final
        {
            return 0;
        }

    private:
        const IInput* m_pInput;
    };

}