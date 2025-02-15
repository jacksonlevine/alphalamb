//
// Created by jack on 12/23/2024.
//

#ifndef INTTUP_H
#define INTTUP_H



#include "PrecompHeader.h"

struct IntTup {
public:
    int x;
    int y;
    int z;
    IntTup(int x, int y, int z);
    IntTup(int x, int z);
    IntTup();
    void set(int x, int y, int z);
    void set(int x, int z);
    bool operator==(const IntTup& other) const;
    bool operator!=(const IntTup& other) const;
    IntTup& operator=(const IntTup& other);
    IntTup& operator+=(const IntTup& other);
};

IntTup operator+(IntTup first, const IntTup& second);

IntTup operator-(IntTup first, const IntTup& second);

struct IntTupHash {
    std::size_t operator()(const IntTup& tup) const;
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

#endif //INTTUP_H
