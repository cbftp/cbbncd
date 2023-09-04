#pragma once

// simple smart pointer template class.
// suggested usage:
// Pointer<T> p(makePointer<T>(args));

#include <unistd.h>

namespace {
  template <typename D> void delfunc(D * d) {
    delete d;
  }
}

template <class T> class Pointer {
  template<class F> friend class Pointer;
public:
  Pointer() :
    object(NULL),
    counter(NULL),
    deleter(NULL)
  {
  }
  explicit Pointer(T * t) :
    object(t),
    counter(new int(1)),
    deleter(delfunc<T>)
  {
  }
  Pointer(const Pointer & other) :
    object(other.object),
    counter(other.counter),
    deleter(other.deleter)
  {
    counter && ++*counter;
  }
  template<typename X> Pointer(const Pointer<X> & other) :
    object(static_cast<T *>(other.object)),
    counter(other.counter),
    deleter(delfunc<T>)
  {
    counter && ++*counter;
  }
  ~Pointer() {
    decrease();
  }
  Pointer & operator=(const Pointer & other) {
    decrease();
    object = other.object;
    counter = other.counter;
    deleter = other.deleter;
    counter && ++*counter;
    return *this;
  }
  template<typename X> Pointer & operator=(const Pointer<X> & other) {
    decrease();
    object = static_cast<T *>(other.object);
    counter = other.counter;
    deleter = delfunc<T>;
    counter && ++*counter;
    return *this;
  }
  T * operator->() const {
    return object;
  }
  T & operator*() const {
    return *object;
  }
  T * get() const {
    return object;
  }
  bool operator!() const {
    return counter == NULL;
  }
  bool operator==(const Pointer & other) const {
    return object == other.object;
  }
  bool operator!=(const Pointer & other) const {
    return object != other.object;
  }
  template<typename X> bool operator==(const Pointer<X> & other) const {
    return object == static_cast<T *>(other.object);
  }
  template<typename X> bool operator!=(const Pointer<X> & other) const {
    return object != static_cast<T *>(other.object);
  }
  bool operator<(const Pointer & other) const {
    return object < other.object;
  }
  template<typename X> bool operator<(const Pointer<X> & other) const {
    return object < static_cast<T *>(other.object);
  }
  void reset() {
    decrease();
    object = NULL;
    counter = NULL;
    deleter = NULL;
  }
  template<typename X> X * get() const {
    return static_cast<X *>(object);
  }
private:
  void decrease() {
    if (counter && !--*counter) {
      delete counter;
      deleter(object);
    }
  }
  T * object;
  int * counter;
  void (*deleter)(T *);
};

template <typename T>
Pointer<T> makePointer() {
  return Pointer<T>(new T());
}

template <typename T, typename A1>
Pointer<T> makePointer(const A1 & a1) {
  return Pointer<T>(new T(a1));
}

template <typename T, typename A1, typename A2>
Pointer<T> makePointer(const A1 & a1, const A2 & a2) {
  return Pointer<T>(new T(a1, a2));
}

template <typename T, typename A1, typename A2, typename A3>
Pointer<T> makePointer(const A1 & a1, const A2 & a2, const A3 & a3) {
  return Pointer<T>(new T(a1, a2, a3));
}

template <typename T, typename A1, typename A2, typename A3, typename A4>
Pointer<T> makePointer(const A1 & a1, const A2 & a2, const A3 & a3,
    const A4 & a4) {
  return Pointer<T>(new T(a1, a2, a3, a4));
}

template <typename T, typename A1, typename A2, typename A3, typename A4,
typename A5>
Pointer<T> makePointer(const A1 & a1, const A2 & a2, const A3 & a3,
    const A4 & a4, const A5 & a5) {
  return Pointer<T>(new T(a1, a2, a3, a4, a5));
}

template <typename T, typename A1, typename A2, typename A3, typename A4,
typename A5, typename A6>
Pointer<T> makePointer(const A1 & a1, const A2 & a2, const A3 & a3,
    const A4 & a4, const A5 & a5, const A6 & a6) {
  return Pointer<T>(new T(a1, a2, a3, a4, a5, a6));
}

template <typename T, typename A1, typename A2, typename A3, typename A4,
typename A5, typename A6, typename A7>
Pointer<T> makePointer(const A1 & a1, const A2 & a2, const A3 & a3,
    const A4 & a4, const A5 & a5, const A6 & a6, const A7 & a7) {
  return Pointer<T>(new T(a1, a2, a3, a4, a5, a6, a7));
}

template <typename T, typename A1, typename A2, typename A3, typename A4,
typename A5, typename A6, typename A7, typename A8>
Pointer<T> makePointer(const A1 & a1, const A2 & a2, const A3 & a3,
    const A4 & a4, const A5 & a5, const A6 & a6, const A7 & a7, const A8 & a8) {
  return Pointer<T>(new T(a1, a2, a3, a4, a5, a6, a7, a8));
}

template <typename T, typename A1, typename A2, typename A3, typename A4,
typename A5, typename A6, typename A7, typename A8, typename A9>
Pointer<T> makePointer(const A1 & a1, const A2 & a2, const A3 & a3,
    const A4 & a4, const A5 & a5, const A6 & a6, const A7 & a7, const A8 & a8,
    const A9 & a9) {
  return Pointer<T>(new T(a1, a2, a3, a4, a5, a6, a7, a8, a9));
}

template <typename T, typename A1, typename A2, typename A3, typename A4,
typename A5, typename A6, typename A7, typename A8, typename A9, typename A10>
Pointer<T> makePointer(const A1 & a1, const A2 & a2, const A3 & a3,
    const A4 & a4, const A5 & a5, const A6 & a6, const A7 & a7, const A8 & a8,
    const A9 & a9, const A10 & a10) {
  return Pointer<T>(new T(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10));
}

template <typename T, typename A1, typename A2, typename A3, typename A4,
typename A5, typename A6, typename A7, typename A8, typename A9, typename A10,
typename A11>
Pointer<T> makePointer(const A1 & a1, const A2 & a2, const A3 & a3,
    const A4 & a4, const A5 & a5, const A6 & a6, const A7 & a7, const A8 & a8,
    const A9 & a9, const A10 & a10, const A11 & a11) {
  return Pointer<T>(new T(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11));
}

template <typename T, typename A1, typename A2, typename A3, typename A4,
typename A5, typename A6, typename A7, typename A8, typename A9, typename A10,
typename A11, typename A12, typename A13, typename A14, typename A15>
Pointer<T> makePointer(const A1 & a1, const A2 & a2, const A3 & a3,
    const A4 & a4, const A5 & a5, const A6 & a6, const A7 & a7, const A8 & a8,
    const A9 & a9, const A10 & a10, const A11 & a11, const A12 & a12,
    const A13 & a13, const A14 & a14, const A15 & a15) {
  return Pointer<T>(new T(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12,
      a13, a14, a15));
}
