#pragma once

#include <memory>
#include "MFrameWork/IObject.h"

namespace MFW {

    void AddObject(std::unique_ptr<IObject>&& pObject) noexcept;
    void Update() noexcept;
    void Draw() noexcept;

}