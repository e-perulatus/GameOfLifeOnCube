#pragma once

namespace MFW
{

    class IObject
    {
    public:
        virtual ~IObject() noexcept {}
        virtual void Update() noexcept = 0;
        virtual void Draw() noexcept = 0;
        virtual uint8_t GetTypeValue() const noexcept = 0;
    };

}