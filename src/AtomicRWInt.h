//
// Created by jack on 2/11/2025.
//

#ifndef ATOMICRWINT_H
#define ATOMICRWINT_H

#include "PrecompHeader.h"

class AtomicRWInt {
private:
    mutable std::shared_mutex rwLock; // Read-write lock
    int value; // Internal integer storage

public:
    // Constructors
    AtomicRWInt(int initial = 0) : value(initial) {}

    // Store (equivalent to store())
    void store(int newValue) {
        std::unique_lock lock(rwLock);
        value = newValue;
    }

    // Load (equivalent to load())
    int load() const {
        std::shared_lock lock(rwLock);
        return value;
    }

    // Assignment operator
    AtomicRWInt& operator=(int newValue) {
        store(newValue);
        return *this;
    }

    // Implicit conversion to int
    operator int() const {
        return load();
    }

    // Fetch-add (atomic++)
    int fetch_add(int arg) {
        std::unique_lock lock(rwLock);
        int old = value;
        value += arg;
        return old;
    }

    // Fetch-sub (atomic--)
    int fetch_sub(int arg) {
        std::unique_lock lock(rwLock);
        int old = value;
        value -= arg;
        return old;
    }

    // Prefix increment
    int operator++() {
        return fetch_add(1) + 1;
    }

    // Postfix increment
    int operator++(int) {
        return fetch_add(1);
    }

    // Prefix decrement
    int operator--() {
        return fetch_sub(1) - 1;
    }

    // Postfix decrement
    int operator--(int) {
        return fetch_sub(1);
    }

    // Exchange (equivalent to exchange())
    int exchange(int newValue) {
        std::unique_lock lock(rwLock);
        int old = value;
        value = newValue;
        return old;
    }

    // Compare-and-swap (equivalent to compare_exchange_strong())
    bool compare_exchange_strong(int& expected, int desired) {
        std::unique_lock lock(rwLock);
        if (value == expected) {
            value = desired;
            return true;
        } else {
            expected = value;
            return false;
        }
    }
};

#endif //ATOMICRWINT_H
