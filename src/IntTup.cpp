#include "IntTup.h"

IntTup::IntTup()
{
    this->x = 0;
    this->y = 0;
    this->z = 0;
}

void IntTup::set(int nx, int ny, int nz)
{
    this->x = nx;
    this->y = ny;
    this->z = nz;
}

void IntTup::set(int nx, int nz)
{
    this->x = nx;
    this->y = 0;
    this->z = nz;
}

bool IntTup::operator==(const IntTup& other) const {
    return (x == other.x) && (y == other.y) && (z == other.z);
}

bool IntTup::operator!=(const IntTup& other) const {
    return (x != other.x) || (y != other.y) || (z != other.z);
}

IntTup& IntTup::operator+=(const IntTup& other)
{
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}


IntTup& IntTup::operator=(const IntTup& other) {
    if (this != &other) {
        x = other.x;
        y = other.y;
        z = other.z;
    }
    return *this;
}

IntTup operator+(IntTup first,
                 const IntTup& second)
{
    first.x += second.x;
    first.y += second.y;
    first.z += second.z;

    return first;
}

IntTup operator-(IntTup first,
                 const IntTup& second)
{
    first.x -= second.x;
    first.y -= second.y;
    first.z -= second.z;

    return first;
}

IntTup::IntTup(int x, int y, int z)
{
    this->x = x;
    this->y = y;
    this->z = z;
}

IntTup::IntTup(int x, int z)
{
    this->x = x;
    this->y = 0;
    this->z = z;
}

std::size_t IntTupHash::operator()(const IntTup& tup) const {
    return (std::hash<int>{}(tup.x) ^ (std::hash<int>{}(tup.y) << 1)) ^ (std::hash<int>{}(tup.z) << 2);
}



TwoIntTup::TwoIntTup(int x, int z) : x(x), z(z)
{
}

TwoIntTup::TwoIntTup() : x(0), z(0)
{
}

void TwoIntTup::set(int x2, int z2)
{
    this->x = x2;
    this->z = z2;
}

bool TwoIntTup::operator==(const TwoIntTup& other) const
{
    return (x == other.x) && (z == other.z);
}



bool TwoIntTup::operator!=(const TwoIntTup& other) const
{
    return (x != other.x) || (z != other.z);
}

TwoIntTup& TwoIntTup::operator=(const TwoIntTup& other)
{
    if (this != &other)
    {
        this->x = other.x;
        this->z = other.z;
    }
    return *this;
}

TwoIntTup& TwoIntTup::operator+=(const TwoIntTup& other)
{
    this->x += other.x;
    this->z += other.z;
    return *this;
}

std::size_t TwoIntTupHash::operator()(const TwoIntTup& tup) const
{
    return std::hash<int>{}(tup.x) ^ (std::hash<int>{}(tup.z) << 1);
}

std::string getIntTupHashAsString(const IntTup& tup) {
    IntTupHash hashFunc; // Instantiate the hash functor
    std::size_t hashValue = hashFunc(tup); // Compute the hash
    return std::to_string(hashValue); // Convert hash to string
}
