#include <cstring>
#include <random>
#include "Type.h"
#include "../GameOfLifeOnCube.h"

namespace Gol3d{

namespace {

    static const int GameOfLifeDimension = 6;

    template <int Col, int Row>
    class GameOfLifeImpl
    {
    public:
        static const int Dimension = GameOfLifeDimension;

        GameOfLifeImpl(long seed) noexcept
        : m_Rnd(seed)
        {        
            Randomize();
        }

        void Next() noexcept
        {
            if ((m_State & 0x0010) == 0x0010)
            {
                Randomize();
                m_State = (m_State & 0x0001);
                return;
            }
            else if ((m_State & 0x0001) == 0x0001)
            {
                return;
            }

            for (int d = Dimension - 1; d >= 0; d--)
            {
                for (int y = 0; y < Row; y++)
                {
                    for (int x = 0; x < Col; x++)
                    {
                        int neighborAliveCount = 0;
                        {
                            const int ul = WrapAround(y - 1, Row) * Col + WrapAround(x - 1, Col);
                            const int um = WrapAround(y - 1, Row) * Col + WrapAround(x    , Col);
                            const int ur = WrapAround(y - 1, Row) * Col + WrapAround(x + 1, Col);
                            const int ml = WrapAround(y    , Row) * Col + WrapAround(x - 1, Col);
                            const int mr = WrapAround(y    , Row) * Col + WrapAround(x + 1, Col);
                            const int dl = WrapAround(y + 1, Row) * Col + WrapAround(x - 1, Col);
                            const int dm = WrapAround(y + 1, Row) * Col + WrapAround(x    , Col);
                            const int dr = WrapAround(y + 1, Row) * Col + WrapAround(x + 1, Col);

                            if (m_Current[d][ul]) neighborAliveCount++;
                            if (m_Current[d][um]) neighborAliveCount++;
                            if (m_Current[d][ur]) neighborAliveCount++;
                            if (m_Current[d][ml]) neighborAliveCount++;
                            if (m_Current[d][mr]) neighborAliveCount++;
                            if (m_Current[d][dl]) neighborAliveCount++;
                            if (m_Current[d][dm]) neighborAliveCount++;
                            if (m_Current[d][dr]) neighborAliveCount++;
                        }

                        if (m_Current[d][y * Col + x] &&
                            (neighborAliveCount == 2 || neighborAliveCount == 3))
                        {
                            m_Next[d][y * Col + x] = true;
                        }
                        else if (!m_Current[d][y * Col + x] &&
                                neighborAliveCount == 3)
                        {
                            m_Next[d][y * Col + x] = true;
                        }
                        else if (m_Current[d][y * Col + x] &&
                                neighborAliveCount > 3)
                        {
                            m_Next[d][y * Col + x] = false;
                        }
                        else if (m_Current[d][y * Col + x] &&
                                neighborAliveCount > 3)
                        {
                            m_Next[d][y * Col + x] = false;
                        }
                        else
                        {
                            m_Next[d][y * Col + x] = false;
                        }
                    }
                }
            }

            std::memcpy(m_Current, m_Next, sizeof(m_Current));
        }

        void GetCurrent(bool* outValue, int d) const noexcept
        {
            for (int y = 0; y < Row; y++)
            {
                for (int x = 0; x < Col; x++)
                {
                    outValue[y * Col + x] = m_Current[d][y * Col + x];
                }
            }
        }

        void SetState(int state) noexcept
        {
            switch (state)
            {
                case 0:
                    m_State = m_State & 0x1110;
                    return;
                case 1:
                    m_State = m_State | 0x0001;
                    return;
                case 2:
                    m_State = m_State | 0x0010;
                    return;
                default:
                    return;
            }
        }

    private:
        void Randomize() noexcept
        {
            for (int y = 0; y < Row; y++)
            {
                for (int x = 0; x < Col; x++)
                {
                    for (int d = 0; d < Dimension; d++)
                    {
                        m_Current[d][y * Col + x] = (m_Rnd() % 100) < 10;
                    }
                }
            }
        }

        int WrapAround(int value, int max)
        {
            return value < 0 ? max - 1 :
                   value >= max ? 0 : value;
        }

    private:
        bool m_Current[Dimension][Row * Col];
        bool m_Next[Dimension][Row * Col] = {};
        std::mt19937 m_Rnd;
        int m_State; // bit1: 0 = Start, 1 = Pause, bit2: Randomize
    };

    constexpr const int GameOfLifeCol = 32;
    constexpr const int GameOfLifeRow = 32;
    typedef GameOfLifeImpl<GameOfLifeCol, GameOfLifeRow> GameOfLife;

    GameOfLife& GetGameOfLife(long seed)
    {
        static GameOfLife s_GameOfLife(seed);
        return s_GameOfLife;
    }

    GameOfLife& GetGameOfLife()
    {
        return GetGameOfLife(0);
    }

    inline float InverseSquareRoot(float value) noexcept
    {
        // cf. https://en.wikipedia.org/wiki/Fast_inverse_square_root
        const float x2 = value * 0.5F;
        const float threehalfs = 1.5F;
        union {
            float f;
            unsigned long i;
        } conv = { .f = value };
        conv.i = 0x5f3759df - (conv.i >> 1);
        conv.f *= (threehalfs - (x2 * conv.f * conv.f));
        return conv.f;
    }

    inline float SquareLength(const Vector3d& vec) noexcept
    {
        return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
    }

    inline float Dot(const Vector3d& vec1, const Vector3d& vec2) noexcept
    {
        return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z;
    }

    inline Vector3d Cross(const Vector3d& vec1, const Vector3d& vec2) noexcept
    {
        Vector3d ret;
        ret.x = vec1.y * vec2.z - vec1.z * vec2.y;
        ret.y = vec1.z * vec2.x - vec1.x * vec2.z;
        ret.z = vec1.x * vec2.y - vec1.y * vec2.x;
        return ret;
    }

    inline Vector3d Normalize(const Vector3d& vec) noexcept
    {
        auto l = InverseSquareRoot(SquareLength(vec));
        return { vec.x * l, vec.y * l, vec.z * l };
    }

    inline Axes3d Normalize(const Axes3d& axes) noexcept
    {
        auto cx = Normalize(axes.x);
        auto cy = Normalize(axes.y - cx * Dot(axes.y, cx));
        auto cz = Normalize(axes.z - (cx * Dot(axes.z, cx)) - (cy * Dot(axes.z, cy)));
        return { cx, cy, cz };
    }

    inline Vector3d Transform(const Axes3d& dstAxes, const Vector3d& dstVec, const Vector3d& srcVec) noexcept
    {
        Vector3d ret;
        ret.x = Dot({ dstAxes.x.x, dstAxes.y.x, dstAxes.z.x }, srcVec) - dstVec.x;
        ret.y = Dot({ dstAxes.x.y, dstAxes.y.y, dstAxes.z.y }, srcVec) - dstVec.y;
        ret.z = Dot({ dstAxes.x.z, dstAxes.y.z, dstAxes.z.z }, srcVec) - dstVec.z;
        return ret;
    }

    inline Axes3d Transform(const Axes3d& dstAxes, const Vector3d& dstVec, const Axes3d& srcAxes) noexcept
    {
        Axes3d ret;
        ret.x = Transform(dstAxes, dstVec, srcAxes.x);
        ret.y = Transform(dstAxes, dstVec, srcAxes.y);
        ret.z = Transform(dstAxes, dstVec, srcAxes.z);
        return ret;
    }

    inline void MakeInverse3x3(float matrix[3][3])
    {
        // elementary row operations
        for (int k = 0; k < 3; k++)
        {
            float t = matrix[k][k];
            
            for (int i = 0; i < 3; i++)
            {
                matrix[k][i] /= t;
            }

            matrix[k][k] = 1 / t;

            for (int j = 0; j < 3; j++)
            {
                if (j != k)
                {
                    float u = matrix[j][k];
                    
                    for (int i = 0; i < 3; i++)
                    {
                        if (i != k)
                        {
                            matrix[j][i] -= matrix[k][i] * u;
                        }
                        else
                        {
                            matrix[j][i] = -u / t;
                        }
                    }
                }
            }
        }
    }

    inline void SolveAffineCoefficient(FPosition src[3], FPosition dst[3], float affine[6])
    {
        // least squares method

        float rhsx[3] = {};
        float rhsy[3] = {};

        float matrix[3][3] = {};

        for (int i = 0; i < 3; i++)
        {
            auto u = src[i].x;
            auto v = src[i].y;
            auto uv = u * v;
            auto x = dst[i].x;
            auto y = dst[i].y;

            matrix[0][0] += u * u;
            matrix[0][1] += uv;
            matrix[0][2] += u;
            matrix[1][0] += uv;
            matrix[1][1] += v * v;
            matrix[1][2] += v;
            matrix[2][0] += u;
            matrix[2][1] += v;
            matrix[2][2] += 1;
    
            rhsx[0] += x * u;
            rhsx[1] += x * v;
            rhsx[2] += x;

            rhsy[0] += y * u;
            rhsy[1] += y * v;
            rhsy[2] += y;
        }

        MakeInverse3x3(matrix);

        affine[0] = matrix[0][0] * rhsx[0] + matrix[0][1] * rhsx[1] + matrix[0][2] * rhsx[2];
        affine[1] = matrix[1][0] * rhsx[0] + matrix[1][1] * rhsx[1] + matrix[1][2] * rhsx[2];
        affine[2] = matrix[2][0] * rhsx[0] + matrix[2][1] * rhsx[1] + matrix[2][2] * rhsx[2];

        affine[3] = matrix[0][0] * rhsy[0] + matrix[0][1] * rhsy[1] + matrix[0][2] * rhsy[2];
        affine[4] = matrix[1][0] * rhsy[0] + matrix[1][1] * rhsy[1] + matrix[1][2] * rhsy[2];
        affine[5] = matrix[2][0] * rhsy[0] + matrix[2][1] * rhsy[1] + matrix[2][2] * rhsy[2];
    }
}

namespace {

    int g_SelectedIconKind = -1;

    const uint8_t g_Alpha = 150;
    const uint8_t g_BgAlpha = 100;

    const Axes3d CameraAttitude = {
        { 1.0, 0.0, 0.0 },
        { 0.0, 1.0, 0.0 },
        { 0.0, 0.0, 1.0 }
    };

    const Vector3d CameraWorldPosition = { 0.0, 0.0, -3.0 };

    Position g_LastMove = {};
}

namespace detail {

    template <const IconContext& Context>
    IconBase<Context>::IconBase(GetSprite getSprite, Position position) noexcept
        : m_GetSprite(getSprite)
        , m_Position(position)
        , m_Kind(Context.kind)
    {
    }

    template <const IconContext& Context>
    void IconBase<Context>::Update() noexcept
    {
        if (!m_Selected && g_SelectedIconKind == m_Kind)
        {
            m_Selected = true;
        }
        else if (m_Selected && g_SelectedIconKind != m_Kind)
        {
            m_Selected = false;
        }
    }

    template <const IconContext& Context>
    void IconBase<Context>::Draw() noexcept
    {
        std::memcpy(m_Buffer, m_Selected ? Context.pImageForOn : Context.pImageForOff, sizeof(uint16_t) * Context.rect.width * Context.rect.height);
        m_GetSprite()->PushImageWithAlphaBlend(m_Buffer, Context.rect, m_Position, g_Alpha, g_BgAlpha);
    }

    constexpr static const uint16_t StartIconOnImage[IconWidth * IconHeight] = {
#include "GameOfLifeOnCube/Data/Icon/StartOn.h"
    };
    constexpr static const uint16_t StartIconOffImage[IconWidth * IconHeight] = {
#include "GameOfLifeOnCube/Data/Icon/StartOff.h"
    };

    const IconContext StartIconContext = { 0, StartIconOnImage, StartIconOffImage, { IconWidth, IconHeight } };

    constexpr static const uint16_t PauseIconOnImage[IconWidth * IconHeight] = {
#include "GameOfLifeOnCube/Data/Icon/PauseOn.h"
    };
    constexpr static const uint16_t PauseIconOffImage[IconWidth * IconHeight] = {
#include "GameOfLifeOnCube/Data/Icon/PauseOff.h"
    };
    const IconContext PauseIconContext = { 1, PauseIconOnImage, PauseIconOffImage, { IconWidth, IconHeight } };

    constexpr static const uint16_t RandomizeIconOnImage[IconWidth * IconHeight] = {
#include "GameOfLifeOnCube/Data/Icon/RandomizeOn.h"
    };
    constexpr static const uint16_t RandomizeIconOffImage[IconWidth * IconHeight] = {
#include "GameOfLifeOnCube/Data/Icon/RandomizeOff.h"
    };
    const IconContext RandomizeIconContext = { 2, RandomizeIconOnImage, RandomizeIconOffImage, { IconWidth, IconHeight } };

}

    Cube::Cube(GetSprite getSprite, Position position, int length, long seed) noexcept
    : m_GetSprite(getSprite)
    , m_Position(position)
    , m_Length(length)
    , m_Vertex{
        { -1.0,  1.0, -1.0 },
        {  1.0,  1.0, -1.0 },
        {  1.0,  1.0,  1.0 },
        { -1.0,  1.0,  1.0 },
        { -1.0, -1.0, -1.0 },
        {  1.0, -1.0, -1.0 },
        {  1.0, -1.0,  1.0 },
        { -1.0, -1.0,  1.0 }
    }
    , m_Attitude{
        { 1.0, 0.0, 0.0 },
        { 0.0, 1.0, 0.0 },
        { 0.0, 0.0, 1.0 }
    }
    , m_AttitudeDelta{
        { 0.0, 0.0, 0.0 },
        { 0.0, 0.0, 0.0 },
        { 0.0, 0.0, 0.0 }
    }
    {
        GetGameOfLife(seed);
    }

    void Cube::Update() noexcept
    {
        GetGameOfLife().Next();

        const float Rate = 0.005;

        auto cameraAttitude = CameraAttitude;
        cameraAttitude += m_AttitudeDelta * m_Acceleration;

        {
            auto axes = cameraAttitude;
            auto x = axes.z + Vector3d { Rate * g_LastMove.x * -1, Rate * g_LastMove.y, 0.0 };
            auto y = axes.y;
            auto z = axes.x;
            axes = Normalize({ x, y, z });
            cameraAttitude = Axes3d { axes.z, axes.y, axes.x };
        }

        m_AttitudeDelta = cameraAttitude - CameraAttitude;
        m_Attitude = Normalize(Transform(cameraAttitude, { 0, 0, 0 }, m_Attitude));
    }

    void Cube::Draw() noexcept
    {
        Position vertexPosition[8];
        float zmax = 0;
        int zIndex = 0;
        for (int i = 0; i < 8; i++)
        {
            auto vertex = Transform(m_Attitude, m_WorldPosition, m_Vertex[i]);
            vertex = Transform(CameraAttitude, CameraWorldPosition, vertex);
            vertexPosition[i] = NormalizePosition(vertex.x, vertex.y);
            if (zmax < vertex.z)
            {
                zmax = vertex.z;
                zIndex = i;
            }
        }

        static bool gameOfLifeBitmap[GameOfLifeDimension][GameOfLifeRow * GameOfLifeCol];
        GetGameOfLife().GetCurrent(gameOfLifeBitmap[0], 0);
        GetGameOfLife().GetCurrent(gameOfLifeBitmap[1], 1);
        GetGameOfLife().GetCurrent(gameOfLifeBitmap[2], 2);
        GetGameOfLife().GetCurrent(gameOfLifeBitmap[3], 3);
        GetGameOfLife().GetCurrent(gameOfLifeBitmap[4], 4);
        GetGameOfLife().GetCurrent(gameOfLifeBitmap[5], 5);

        switch (zIndex)
        {
        case 0:
            DrawSurface(0, vertexPosition, gameOfLifeBitmap[0]);
            DrawSurface(3, vertexPosition, gameOfLifeBitmap[3]);
            DrawSurface(4, vertexPosition, gameOfLifeBitmap[4]);
            DrawSurface(1, vertexPosition, gameOfLifeBitmap[1]);
            DrawSurface(2, vertexPosition, gameOfLifeBitmap[2]);
            DrawSurface(5, vertexPosition, gameOfLifeBitmap[5]);
            break;
        case 1:
            DrawSurface(0, vertexPosition, gameOfLifeBitmap[0]);
            DrawSurface(1, vertexPosition, gameOfLifeBitmap[1]);
            DrawSurface(4, vertexPosition, gameOfLifeBitmap[4]);
            DrawSurface(2, vertexPosition, gameOfLifeBitmap[2]);
            DrawSurface(3, vertexPosition, gameOfLifeBitmap[3]);
            DrawSurface(5, vertexPosition, gameOfLifeBitmap[5]);
            break;
        case 2:
            DrawSurface(1, vertexPosition, gameOfLifeBitmap[1]);
            DrawSurface(2, vertexPosition, gameOfLifeBitmap[2]);
            DrawSurface(4, vertexPosition, gameOfLifeBitmap[4]);
            DrawSurface(0, vertexPosition, gameOfLifeBitmap[0]);
            DrawSurface(3, vertexPosition, gameOfLifeBitmap[3]);
            DrawSurface(5, vertexPosition, gameOfLifeBitmap[5]);
            break;
        case 3:
            DrawSurface(2, vertexPosition, gameOfLifeBitmap[2]);
            DrawSurface(3, vertexPosition, gameOfLifeBitmap[3]);
            DrawSurface(4, vertexPosition, gameOfLifeBitmap[4]);
            DrawSurface(0, vertexPosition, gameOfLifeBitmap[0]);
            DrawSurface(1, vertexPosition, gameOfLifeBitmap[1]);
            DrawSurface(5, vertexPosition, gameOfLifeBitmap[5]);
            break;
        case 4:
            DrawSurface(0, vertexPosition, gameOfLifeBitmap[0]);
            DrawSurface(3, vertexPosition, gameOfLifeBitmap[3]);
            DrawSurface(5, vertexPosition, gameOfLifeBitmap[5]);
            DrawSurface(1, vertexPosition, gameOfLifeBitmap[1]);
            DrawSurface(2, vertexPosition, gameOfLifeBitmap[2]);
            DrawSurface(4, vertexPosition, gameOfLifeBitmap[4]);
            break;
        case 5:
            DrawSurface(0, vertexPosition, gameOfLifeBitmap[0]);
            DrawSurface(1, vertexPosition, gameOfLifeBitmap[1]);
            DrawSurface(5, vertexPosition, gameOfLifeBitmap[5]);
            DrawSurface(2, vertexPosition, gameOfLifeBitmap[2]);
            DrawSurface(3, vertexPosition, gameOfLifeBitmap[3]);
            DrawSurface(4, vertexPosition, gameOfLifeBitmap[4]);
            break;
        case 6:
            DrawSurface(1, vertexPosition, gameOfLifeBitmap[1]);
            DrawSurface(2, vertexPosition, gameOfLifeBitmap[2]);
            DrawSurface(5, vertexPosition, gameOfLifeBitmap[5]);
            DrawSurface(0, vertexPosition, gameOfLifeBitmap[0]);
            DrawSurface(3, vertexPosition, gameOfLifeBitmap[3]);
            DrawSurface(4, vertexPosition, gameOfLifeBitmap[4]);
            break;
        case 7:
            DrawSurface(2, vertexPosition, gameOfLifeBitmap[2]);
            DrawSurface(3, vertexPosition, gameOfLifeBitmap[3]);
            DrawSurface(5, vertexPosition, gameOfLifeBitmap[5]);
            DrawSurface(0, vertexPosition, gameOfLifeBitmap[0]);
            DrawSurface(1, vertexPosition, gameOfLifeBitmap[1]);
            DrawSurface(4, vertexPosition, gameOfLifeBitmap[4]);
            break;
        default:
            break;
        }
    }

    inline void Cube::DrawSurface(int surfaceIndex, const Position* pVertexPosition, const bool* pGameOfLifeBitmap) noexcept
    {
        int _vertex[3] = {};
        uint16_t _color = 0;

        switch (surfaceIndex)
        {
        case 0:
            _vertex[0] = 0;
            _vertex[1] = 1;
            _vertex[2] = 4;
            _color = SwappedColor(RgbTo16bitColor(255,   0,   0));
            break;
        case 1:
            _vertex[0] = 1;
            _vertex[1] = 2;
            _vertex[2] = 5;
            _color = SwappedColor(RgbTo16bitColor(  0, 255,   0));
            break;
        case 2:
            _vertex[0] = 2;
            _vertex[1] = 3;
            _vertex[2] = 6;
            _color = SwappedColor(RgbTo16bitColor(  0,   0, 255));
            break;
        case 3:
            _vertex[0] = 3;
            _vertex[1] = 0;
            _vertex[2] = 7;
            _color = SwappedColor(RgbTo16bitColor(255, 255,   0));
            break;
        case 4:
            _vertex[0] = 3;
            _vertex[1] = 2;
            _vertex[2] = 0;
            _color = SwappedColor(RgbTo16bitColor(  0, 255, 255));
            break;
        case 5:
            _vertex[0] = 4;
            _vertex[1] = 5;
            _vertex[2] = 7;
            _color = SwappedColor(RgbTo16bitColor(255,   0, 255));
            break;
        default:
            break;
        }

        FPosition _src[3] = { { 0.0, 0.0 }, { static_cast<float>(BufferWidth), 0.0 }, { 0.0, static_cast<float>(BufferHeight) } };
        FPosition _dst[3] = { 
            { static_cast<float>(pVertexPosition[_vertex[0]].x), static_cast<float>(pVertexPosition[_vertex[0]].y) },
            { static_cast<float>(pVertexPosition[_vertex[1]].x), static_cast<float>(pVertexPosition[_vertex[1]].y) },
            { static_cast<float>(pVertexPosition[_vertex[2]].x), static_cast<float>(pVertexPosition[_vertex[2]].y) },
        };

        float _affine[6] = {};
        SolveAffineCoefficient(_src, _dst, _affine);

        for (int y = 0; y < BufferHeight; y++)
        {
            for (int x = 0; x < BufferWidth; x++)
            {
                int sy = y >> 1; // / (BufferHeight / GameOfLifeCol)
                int sx = x >> 1; // / (BufferWidth / GameOfLifeRow)
                m_Buffer[y * BufferWidth + x] = pGameOfLifeBitmap[sy * GameOfLifeRow + sx] ? _color : ColorBlack;
            }
        }

        m_GetSprite()->PushImageAffineWithAlphaBlend(m_Buffer, { BufferWidth, BufferHeight }, _affine, g_Alpha, g_BgAlpha);
    }

    inline Position Cube::NormalizePosition(float x, float y) const noexcept
    {
        auto halfLength = m_Length >> 1;
        return { static_cast<int>(x * halfLength + m_Position.x), static_cast<int>(y * halfLength + m_Position.y) };
    }

    InputEvent::InputEvent(IInput* pInput) noexcept
        : m_pInput(pInput)
    {
    }

    void InputEvent::Update() noexcept
    {
        switch (m_pInput->Get())
        {
            case InputFlag::Select:
                g_SelectedIconKind = g_SelectedIconKind == 2 ? -1 : g_SelectedIconKind + 1;
                break;
            case InputFlag::Enter:
                GetGameOfLife().SetState(g_SelectedIconKind);
                break;
            case InputFlag::Cancel:
                g_SelectedIconKind = -1;
                break;
            default:
                break;
        }

        g_LastMove = m_pInput->GetTouchMove();
    }

}
