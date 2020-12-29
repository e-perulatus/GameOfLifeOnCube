#include <set>
#include "../MFrameWork.h"

namespace MFW {

namespace {
    typedef std::unique_ptr<IObject> ObjectPointer;
    struct ObjectPointerComparer
    {
        bool operator()(const ObjectPointer& a, const ObjectPointer& b) const
        {
            return a->GetTypeValue() < b->GetTypeValue();
        }
    };
    typedef std::multiset<ObjectPointer, ObjectPointerComparer> ObjectContainer;

    ObjectContainer g_Objects;
}

    void AddObject(std::unique_ptr<IObject>&& pObject) noexcept
    {
        g_Objects.emplace(std::move(pObject));
    }

    void Update() noexcept
    {
        for (auto& obj : g_Objects)
        {
            obj->Update();
        }
    }

    void Draw() noexcept
    {
        for (auto& obj : g_Objects)
        {
            obj->Draw();
        }
    }

}