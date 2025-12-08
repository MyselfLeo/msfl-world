//
// Created by leo on 12/8/25.
//

#pragma once

/// This
template<typename T>
class Singleton {
protected:
    Singleton() = default;

    virtual ~Singleton() = default;

public:
    Singleton(const Singleton &) = delete;

    Singleton &operator=(const Singleton &) = delete;

    static T &get() {
        static T instance;
        return instance;
    }
};
