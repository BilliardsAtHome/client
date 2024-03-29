#ifndef NW4R_UT_RTTI_H
#define NW4R_UT_RTTI_H
#include "types_nw4r.h"

namespace nw4r {
namespace ut {
namespace detail {
struct RuntimeTypeInfo {
    const RuntimeTypeInfo* mBase;

    inline RuntimeTypeInfo(const RuntimeTypeInfo* pBase) : mBase(pBase) {}

    inline bool IsDerivedFrom(const RuntimeTypeInfo* pBase) const {
        for (const RuntimeTypeInfo* pCur = this; pCur; pCur = pCur->mBase) {
            if (pCur == pBase)
                return true;
        }

        return false;
    }
};
} // namespace detail

struct Polymorphic {
    virtual const detail::RuntimeTypeInfo* GetRuntimeTypeInfo() const = 0;
};

template <typename TRet, typename TParam> inline TRet DynamicCast(TParam* ptr) {
    TRet ret = static_cast<TRet>(ptr);

    const detail::RuntimeTypeInfo* pBase = &ret->typeInfo;

    return ptr->GetRuntimeTypeInfo()->IsDerivedFrom(pBase) ? ret : TRet();
}
} // namespace ut
} // namespace nw4r

#endif
