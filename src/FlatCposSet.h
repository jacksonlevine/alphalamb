//
// Created by jack on 5/26/2025.
//

#ifndef FLATCPOSSET_H
#define FLATCPOSSET_H

#include "PrecompHeader.h"

template<typename T>
class FlatCposSetPoolAllocator {
    std::pmr::memory_resource* resource_;
public:
    using value_type = T;

    FlatCposSetPoolAllocator(std::pmr::memory_resource* r) : resource_(r) {}

    T* allocate(size_t n) {
        return static_cast<T*>(resource_->allocate(n * sizeof(T), alignof(T)));
    }

    void deallocate(T* p, size_t n) {
        resource_->deallocate(p, n * sizeof(T), alignof(T));
    }
};

using flatCposSet = tbb::concurrent_unordered_set<TwoIntTup, TwoIntTupHash, std::equal_to<TwoIntTup>, FlatCposSetPoolAllocator<TwoIntTup>>;


#endif //FLATCPOSSET_H
