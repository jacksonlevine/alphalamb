//
// Created by jack on 5/26/2025.
//

#ifndef FLATCPOSSET_H
#define FLATCPOSSET_H

#include "PrecompHeader.h"

template<typename T>
class FlatCposSetPoolAllocator {
public:
    using value_type = T;

    FlatCposSetPoolAllocator() noexcept : resource_(nullptr) {}

    FlatCposSetPoolAllocator(boost::container::pmr::memory_resource* r) noexcept : resource_(r) {}

    template <class U>
    FlatCposSetPoolAllocator(const FlatCposSetPoolAllocator<U>& other) noexcept
        : resource_(other.resource_) {}

    T* allocate(std::size_t n) {
        return static_cast<T*>(resource_->allocate(n * sizeof(T), alignof(T)));
    }

    void deallocate(T* p, std::size_t n) noexcept {
        resource_->deallocate(p, n * sizeof(T), alignof(T));
    }

    template <class U>
    struct rebind {
        using other = FlatCposSetPoolAllocator<U>;
    };

    // Required for TBB's allocator use
    using is_always_equal = std::false_type;

    boost::container::pmr::memory_resource* resource_ = nullptr;
};

// Comparison operators
template <typename T, typename U>
bool operator==(const FlatCposSetPoolAllocator<T>& a, const FlatCposSetPoolAllocator<U>& b) noexcept {
    return a.resource_ == b.resource_;
}
template <typename T, typename U>
bool operator!=(const FlatCposSetPoolAllocator<T>& a, const FlatCposSetPoolAllocator<U>& b) noexcept {
    return !(a == b);
}

using flatCposSet = tbb::concurrent_unordered_set<TwoIntTup, TwoIntTupHash>;

#endif //FLATCPOSSET_H
