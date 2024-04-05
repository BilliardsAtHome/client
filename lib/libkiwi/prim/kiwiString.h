#ifndef LIBKIWI_PRIM_STRING_H
#define LIBKIWI_PRIM_STRING_H
#include <types.h>

namespace kiwi {

/**
 * String wrapper
 */
template <typename T> class StringImpl {
public:
    /**
     * Constructor
     */
    StringImpl() : mpBuffer(NULL), mCapacity(0), mLength(0) {
        Clear();
    }

    /**
     * Constructor
     * @details Copy constructor
     *
     * @param str String to copy
     */
    StringImpl(const StringImpl& str)
        : mpBuffer(NULL), mCapacity(0), mLength(0) {
        Assign(str);
    }

    /**
     * Constructor
     * @details Substring constructor
     *
     * @param str String to copy
     * @param pos Substring start index
     * @param len Substring length
     */
    StringImpl(const StringImpl& str, u32 pos, u32 len = npos)
        : mpBuffer(NULL), mCapacity(0), mLength(0) {
        Assign(str.Substr(pos, len));
    }

    /**
     * Constructor
     * @details C-style string constructor
     *
     * @param s C-style string
     */
    StringImpl(const T* s) : mpBuffer(NULL), mCapacity(0), mLength(0) {
        Assign(s);
    }

    /**
     * Constructor
     * @details Buffer/sequence constructor
     *
     * @param s Buffer/sequence
     * @param n Number of characters to copy
     */
    StringImpl(const T* s, u32 n) : mpBuffer(NULL), mCapacity(0), mLength(0) {
        Assign(s, n);
    }

    /**
     * Constructor
     * @details Reserve space
     *
     * @param n Number of characters to reserve
     */
    explicit StringImpl(u32 n) : mpBuffer(NULL), mCapacity(0), mLength(0) {
        Reserve(n);
    }

    /**
     * Destructor
     */
    ~StringImpl() {
        // Don't delete static memory
        if (mpBuffer == scEmptyCStr) {
            return;
        }

        delete mpBuffer;
        mpBuffer = NULL;
    }

    /**
     * Implicit conversion operator to C-style string
     */
    operator const T*() const {
        return CStr();
    }

    /**
     * Gets the length of the underlying string (not including term)
     */
    u32 Length() const {
        return mLength;
    }

    /**
     * Tests whether the string is empty
     */
    bool Empty() const {
        return Length() == 0;
    }

    /**
     * Gets the underlying C-style string
     */
    const T* CStr() const {
        return mpBuffer;
    }

    /**
     * Accesses a character in the string
     *
     * @param i Character index
     * @return Reference to character
     */
    T& operator[](u32 i) {
        K_ASSERT(i < mLength);
        return mpBuffer[i];
    }

    /**
     * Accesses a character in the string
     *
     * @param i Character index
     * @return Reference to character
     */
    const T& operator[](u32 i) const {
        K_ASSERT(i < mLength);
        return mpBuffer[i];
    }

    void Clear();
    StringImpl Substr(u32 pos = 0, u32 len = npos) const;

    u32 Find(const StringImpl& str, u32 pos = 0) const;
    u32 Find(const T* s, u32 pos = 0) const;
    u32 Find(T c, u32 pos = 0) const;

    StringImpl& operator+=(const StringImpl& str) {
        Append(str);
        return *this;
    }
    StringImpl& operator+=(const T* s) {
        K_ASSERT(s != -NULL);
        Append(s);
        return *this;
    }
    StringImpl& operator+=(T c) {
        Append(c);
        return *this;
    }

    StringImpl& operator=(const StringImpl& str) {
        Assign(str);
        return *this;
    }
    StringImpl& operator=(const T* s) {
        Assign(s);
        return *this;
    }
    StringImpl& operator=(T c) {
        Assign(c);
        return *this;
    }

    bool operator==(const StringImpl& str) const;
    bool operator==(const T* s) const;
    bool operator==(const T c) const {
        return mLength == 1 && mpBuffer[0] == c;
    }

    bool operator!=(const StringImpl& str) const {
        return (*this == str) == false;
    }
    bool operator!=(const T* s) const {
        return (*this == s) == false;
    }
    bool operator!=(T c) const {
        return (*this == c) == false;
    }

private:
    void Reserve(u32 n);
    void Shrink();

    void Assign(const StringImpl& str);
    void Assign(const T* s, u32 n = npos);
    void Assign(T c);

    void Append(const StringImpl& str);
    void Append(const T* s);
    void Append(T c);

private:
    // String buffer
    T* mpBuffer;
    // Allocated buffer size
    u32 mCapacity;
    // String length
    u32 mLength;

    // Static string for empty StringImpls
    static const T* scEmptyCStr;

public:
    static const u32 npos = -1;
};

typedef StringImpl<char> String;
typedef StringImpl<wchar_t> WString;

template <typename T>
inline StringImpl<T> operator+(const StringImpl<T>& lhs,
                               const StringImpl<T>& rhs) {
    StringImpl<T> str = lhs;
    str += rhs;
    return str;
}

template <typename T>
inline StringImpl<T> operator+(const StringImpl<T>& lhs, const T* rhs) {
    StringImpl<T> str = lhs;
    str += rhs;
    return str;
}

template <typename T>
inline StringImpl<T> operator+(const StringImpl<T>& lhs, T rhs) {
    StringImpl<T> str = lhs;
    str += rhs;
    return str;
}

/**
 * Creates a new string from format arguments
 *
 * @param fmt Format string
 * @param args Format arguments
 */
template <typename T>
inline StringImpl<T> VFormat(const StringImpl<T>& fmt, std::va_list args) {
    char buffer[1024];
    std::vsnprintf(buffer, sizeof(buffer), fmt, args);
    return StringImpl<T>(buffer);
}

/**
 * Creates a new string from format arguments
 *
 * @param fmt Format C-style string
 * @param args Format arguments
 */
template <typename T>
inline StringImpl<T> VFormat(const T* fmt, std::va_list args) {
    char buffer[1024];
    std::vsnprintf(buffer, sizeof(buffer), fmt, args);
    return StringImpl<T>(buffer);
}

/**
 * Creates a new string from format arguments
 *
 * @param fmt Format string
 * @param ... Format arguments
 */
template <typename T>
inline StringImpl<T> Format(const StringImpl<T>& fmt, ...) {
    std::va_list list;
    va_start(list, fmt);
    StringImpl<T> str = VFormat(fmt, list);
    va_end(list);

    return str;
}

/**
 * Creates a new string from format arguments
 *
 * @param fmt Format C-style string
 * @param ... Format arguments
 */
template <typename T> inline StringImpl<T> Format(const T* fmt, ...) {
    std::va_list list;
    va_start(list, fmt);
    StringImpl<T> str = VFormat(fmt, list);
    va_end(list);

    return str;
}

template <typename T> inline String ToString(const T& t);

/**
 * @brief Convert integer to string
 */
template <> inline String ToString<int>(const int& t) {
    return Format("%d", t);
}
/**
 * @brief Convert float to string
 */
template <> inline String ToString<f32>(const f32& t) {
    return Format("%f", t);
}
/**
 * @brief Convert double to string
 */
template <> inline String ToString<f64>(const f64& t) {
    return Format("%f", t);
}

} // namespace kiwi

#endif