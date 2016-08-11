#pragma once
//-- Copyright 2016 Intrig
//-- See https://github.com/intrig/xenon for license.

namespace ict {
template <typename T>
struct cloned_ptr {
    cloned_ptr() : p(0) {}
    cloned_ptr(T * p) : p(p) { };

    cloned_ptr(const cloned_ptr<T> & x) {
        p = x->clone();
    }

    template <typename E>
    inline cloned_ptr(cloned_ptr<E> && x) {
        p = x.p;
        x.p = 0;
    }

    template <typename F>
    inline cloned_ptr(cloned_ptr<F> & x) {
        p = x->clone();
    }

    ~cloned_ptr() { if (p) delete p; }

    cloned_ptr<T> & operator=(const cloned_ptr & x) {
        if (p) {
            delete p;
            p = 0;
        }
        if (x.p) p = x->clone();
        return *this;
    }

    T& operator*() { return *p; }
    const T& operator*() const { return *p; }
    T* operator->() { return p; }
    const T* operator->() const { return p; }

    T * p = 0;
};

template <typename T>
cloned_ptr<T> make_cloned() {
    return cloned_ptr<T>(new T);
}

}
