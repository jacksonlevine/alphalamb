//
// Created by jack on 12/23/2024.
//

#ifndef INTTUP_H
#define INTTUP_H



#include "PrecompHeader.h"
#include <PxShape.h>

struct IntTup {
public:
    int x;
    int y;
    int z;
    constexpr IntTup(int x, int y, int z) : x(x), y(y), z(z) {}
    IntTup(int x, int z);
    IntTup();
    void set(int x, int y, int z);
    void set(int x, int z);
    bool operator==(const IntTup& other) const;
    bool operator!=(const IntTup& other) const;
    IntTup& operator=(const IntTup& other);
    IntTup& operator+=(const IntTup& other);

    operator physx::PxVec3() const {
        return physx::PxVec3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
    }

};

IntTup operator+(IntTup first, const IntTup& second);

IntTup operator-(IntTup first, const IntTup& second);

struct IntTupHash {
    std::size_t operator()(const IntTup& tup) const;
    std::uint16_t operator()(const IntTup& tup, bool small) const noexcept {
        std::uint16_t hash = static_cast<std::uint16_t>(tup.x);
        hash ^= static_cast<std::uint16_t>(tup.y << 5);
        hash ^= static_cast<std::uint16_t>(tup.z << 10);

        // One multiply for final mixing
        hash = static_cast<std::uint16_t>(hash * 0x9E37);
        hash ^= hash >> 8;

        return hash;
    }
};






struct TwoIntTup {
public:
    int x;
    int z;
    TwoIntTup(int x, int z);
    TwoIntTup();
    void set(int x2, int z);
    bool operator==(const TwoIntTup& other) const;
    bool operator!=(const TwoIntTup& other) const;
    TwoIntTup& operator=(const TwoIntTup& other);
    TwoIntTup& operator+=(const TwoIntTup& other);
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(x,z);
    }
};

TwoIntTup operator+(TwoIntTup first, const TwoIntTup& second);

inline TwoIntTup operator+(TwoIntTup first, const TwoIntTup& second)
{
    return first += second;
}

TwoIntTup operator-(TwoIntTup first, const TwoIntTup& second);

struct TwoIntTupHash {
    std::size_t operator()(const TwoIntTup& tup) const;
};


std::string getIntTupHashAsString(const IntTup& tup);

struct TwoIntTupHashCompare {
    static size_t hash(const TwoIntTup& tup) {
        size_t h1 = std::hash<int>()(tup.x);
        size_t h2 = std::hash<int>()(tup.z);
        return h1 ^ (h2 << 1);
    }

    static bool equal(const TwoIntTup& a, const TwoIntTup& b) {
        return a == b;
    }
};
#endif //INTTUP_H
