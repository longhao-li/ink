#include "ink/core/string.h"

#include <Windows.h>

using namespace ink;

auto ink::StringView::split(value_type delim) const noexcept -> std::vector<StringView> {
    std::vector<StringView> result;

    const_pointer       lastSplit = m_ptr;
    const const_pointer cmpEnd    = m_ptr + m_len;

    for (const_pointer i = m_ptr; i != cmpEnd; ++i) {
        if (traits_type::eq(*i, delim)) {
            if (i != lastSplit)
                result.emplace_back(lastSplit, static_cast<size_type>(i - lastSplit));
            lastSplit = i + 1;
        }
    }

    if (lastSplit < m_ptr + m_len)
        result.emplace_back(lastSplit, static_cast<size_type>(cmpEnd - lastSplit));

    return result;
}

auto ink::StringView::splitByAnyOf(StringView delims) const noexcept -> std::vector<StringView> {
    std::vector<StringView> result;

    const_pointer       lastSplit = m_ptr;
    const const_pointer cmpEnd    = m_ptr + m_len;

    for (const_pointer i = m_ptr; i != cmpEnd; ++i) {
        if (delims.contains(*i)) {
            if (i != lastSplit)
                result.emplace_back(lastSplit, static_cast<size_type>(i - lastSplit));
            lastSplit = i + 1;
        }
    }

    if (lastSplit < m_ptr + m_len)
        result.emplace_back(lastSplit, static_cast<size_type>(cmpEnd - lastSplit));

    return result;
}

auto ink::StringView::toUTF8String() const noexcept -> std::string {
    int count = WideCharToMultiByte(CP_UTF8, 0, reinterpret_cast<LPCWCH>(m_ptr),
                                    static_cast<int>(m_len), nullptr, 0, nullptr, nullptr);
    if (count <= 0)
        return std::string();

    std::string result(static_cast<std::size_t>(count), char());
    WideCharToMultiByte(CP_UTF8, 0, reinterpret_cast<LPCWCH>(m_ptr), static_cast<int>(m_len),
                        result.data(), count, nullptr, nullptr);

    return result;
}

namespace {

[[nodiscard]]
__forceinline auto alignUpAllocCount(std::size_t cap) noexcept -> std::size_t {
    constexpr const std::size_t alignment = sizeof(void *);
    return ((cap + alignment - 1) & ~std::size_t(alignment - 1));
}

} // namespace

#if !defined(__clang__) && defined(_MSC_VER)
#    pragma warning(push)
#    pragma warning(disable : 6011 6308 6385 6386 6387 28182 28183)
#endif

ink::String::String(const value_type *ptr, size_type len) noexcept {
    if (len < SHORT_CAPACITY) {
        traits_type::copy(m_impl.s.str, ptr, len);
        traits_type::assign(m_impl.s.str[len], value_type());
        m_impl.s.size    = static_cast<std::uint8_t>(len);
        m_impl.s.padding = 0;
        m_impl.s.isLong  = 0;
    } else {
        const size_type allocCount = alignUpAllocCount(len + 1);
        const size_type allocSize  = allocCount * sizeof(value_type);
        const pointer   buffer     = static_cast<pointer>(std::malloc(allocSize));

        traits_type::copy(buffer, ptr, len);
        traits_type::assign(buffer[len], value_type());

        m_impl.l.ptr      = buffer;
        m_impl.l.capacity = allocCount - 1;
        m_impl.l.size     = len;
        m_impl.l.padding  = 0;
        m_impl.l.isLong   = 1;
    }
}

ink::String::String(const value_type *ptr) noexcept {
    const size_type len = traits_type::length(ptr);
    if (len < SHORT_CAPACITY) {
        // Copy with trailing '\0'.
        traits_type::copy(m_impl.s.str, ptr, len + 1);
        m_impl.s.size    = static_cast<std::uint8_t>(len);
        m_impl.s.padding = 0;
        m_impl.s.isLong  = 0;
    } else {
        const size_type allocCount = alignUpAllocCount(len + 1);
        const size_type allocSize  = allocCount * sizeof(value_type);
        const pointer   buffer     = static_cast<pointer>(std::malloc(allocSize));

        // Copy with trailing '\0'.
        traits_type::copy(buffer, ptr, len + 1);

        m_impl.l.ptr      = buffer;
        m_impl.l.capacity = allocCount - 1;
        m_impl.l.size     = len;
        m_impl.l.padding  = 0;
        m_impl.l.isLong   = 1;
    }
}

ink::String::String(StringView str) noexcept {
    const const_pointer ptr = str.data();
    const size_type     len = str.length();

    if (len < SHORT_CAPACITY) {
        traits_type::copy(m_impl.s.str, ptr, len);
        traits_type::assign(m_impl.s.str[len], value_type());
        m_impl.s.size    = static_cast<std::uint8_t>(len);
        m_impl.s.padding = 0;
        m_impl.s.isLong  = 0;
    } else {
        const size_type allocCount = alignUpAllocCount(len + 1);
        const size_type allocSize  = allocCount * sizeof(value_type);
        const pointer   buffer     = static_cast<pointer>(std::malloc(allocSize));

        traits_type::copy(buffer, ptr, len);
        traits_type::assign(buffer[len], value_type());

        m_impl.l.ptr      = buffer;
        m_impl.l.capacity = allocCount - 1;
        m_impl.l.size     = len;
        m_impl.l.padding  = 0;
        m_impl.l.isLong   = 1;
    }
}

ink::String::String(const String &other) noexcept {
    if (other.m_impl.l.isLong == 0) {
        // Copy if is short string.
        m_impl = other.m_impl;
    } else {
        const const_pointer ptr = other.m_impl.l.ptr;
        const size_type     len = other.m_impl.l.size;

        const size_type allocCount = alignUpAllocCount(len + 1);
        const size_type allocSize  = allocCount * sizeof(value_type);
        const pointer   buffer     = static_cast<pointer>(std::malloc(allocSize));

        // Copy with trailing '\0'.
        traits_type::copy(buffer, ptr, len + 1);

        m_impl.l.ptr      = buffer;
        m_impl.l.capacity = allocCount - 1;
        m_impl.l.size     = len;
        m_impl.l.padding  = 0;
        m_impl.l.isLong   = 1;
    }
}

ink::String::~String() noexcept {
    if (m_impl.l.isLong)
        std::free(m_impl.l.ptr);
}

auto ink::String::assign(const String &other) noexcept -> String & {
    // Self assignment.
    if (this == &other)
        return *this;

    // If other is short string.
    if (other.m_impl.l.isLong == 0) {
        if (m_impl.l.isLong) {
            // Copy with trailing '\0'.
            traits_type::copy(m_impl.l.ptr, other.m_impl.s.str, other.m_impl.s.size + size_type(1));
            m_impl.l.size = other.m_impl.s.size;
            return *this;
        }

        // Copy if is short string.
        m_impl = other.m_impl;
        return *this;
    }

    const const_pointer ptr = other.m_impl.l.ptr;
    const size_type     len = other.m_impl.l.size;

    const size_type allocCount = alignUpAllocCount(len + 1);
    const size_type allocSize  = allocCount * sizeof(value_type);

    // Allocate new memory if was short string.
    if (m_impl.l.isLong == 0) {
        const pointer buffer = static_cast<pointer>(std::malloc(allocSize));

        m_impl.l.ptr      = buffer;
        m_impl.l.capacity = allocCount - 1;
        m_impl.l.padding  = 0;
        m_impl.l.isLong   = 1;
    } else if (m_impl.l.capacity < len) { // Long string without enough memory.
        m_impl.l.ptr      = static_cast<pointer>(std::realloc(m_impl.l.ptr, allocSize));
        m_impl.l.capacity = allocCount - 1;
    }

    // Copy with trailing '\0'.
    traits_type::copy(m_impl.l.ptr, ptr, len + 1);
    m_impl.l.size = len;

    return *this;
}

auto ink::String::assign(String &&other) noexcept -> String & {
    if (m_impl.l.isLong)
        std::free(m_impl.l.ptr);

    m_impl = other.m_impl;
    std::memset(&other.m_impl, 0, sizeof(other.m_impl));

    return *this;
}

auto ink::String::assign(StringView str) noexcept -> String & {
    const const_pointer ptr = str.data();
    const size_type     len = str.length();

    // Is short string.
    if (len < SHORT_CAPACITY) {
        if (m_impl.l.isLong) {
            traits_type::move(m_impl.l.ptr, ptr, len);
            traits_type::assign(m_impl.l.ptr[len], value_type());
            m_impl.l.size = len;
            return *this;
        }

        traits_type::move(m_impl.s.str, ptr, len);
        traits_type::assign(m_impl.s.str[len], value_type());
        m_impl.s.size = static_cast<std::uint8_t>(len);
        return *this;
    }

    const size_type allocCount = alignUpAllocCount(len + 1);
    const size_type allocSize  = allocCount * sizeof(value_type);

    // Allocate new memory if was short string.
    if (m_impl.l.isLong == 0) {
        const pointer buffer = static_cast<pointer>(std::malloc(allocSize));

        m_impl.l.ptr      = buffer;
        m_impl.l.capacity = allocCount - 1;
        m_impl.l.padding  = 0;
        m_impl.l.isLong   = 1;
    } else if (m_impl.l.capacity < len) { // Long string without enough memory.
        m_impl.l.ptr      = static_cast<pointer>(std::realloc(m_impl.l.ptr, allocSize));
        m_impl.l.capacity = allocCount - 1;
    }

    traits_type::move(m_impl.l.ptr, ptr, len);
    traits_type::assign(m_impl.l.ptr[len], value_type());
    m_impl.l.size = len;

    return *this;
}

auto ink::String::assign(const_pointer ptr) noexcept -> String & {
    const size_type len = traits_type::length(ptr);

    // Is short string.
    if (len < SHORT_CAPACITY) {
        if (m_impl.l.isLong) {
            // Copy with trailing '\0'.
            traits_type::move(m_impl.l.ptr, ptr, len + 1);
            m_impl.l.size = len;
            return *this;
        }

        traits_type::move(m_impl.s.str, ptr, len + 1);
        m_impl.s.size = static_cast<std::uint8_t>(len);
        return *this;
    }

    const size_type allocCount = alignUpAllocCount(len + 1);
    const size_type allocSize  = allocCount * sizeof(value_type);

    // Allocate new memory if was short string.
    if (m_impl.l.isLong == 0) {
        const pointer buffer = static_cast<pointer>(std::malloc(allocSize));

        m_impl.l.ptr      = buffer;
        m_impl.l.capacity = allocCount - 1;
        m_impl.l.padding  = 0;
        m_impl.l.isLong   = 1;
    } else if (m_impl.l.capacity < len) { // Long string without enough memory.
        m_impl.l.ptr      = static_cast<pointer>(std::realloc(m_impl.l.ptr, allocSize));
        m_impl.l.capacity = allocCount - 1;
    }

    // Copy with trailing '\0'.
    traits_type::move(m_impl.l.ptr, ptr, len + 1);
    m_impl.l.size = len;

    return *this;
}

auto ink::String::reserve(size_type newCap) noexcept -> void {
    if (m_impl.l.isLong) {
        if (newCap <= m_impl.l.capacity)
            return;

        const size_type allocCount = alignUpAllocCount(newCap + 1);
        const size_type allocSize  = allocCount * sizeof(value_type);

        m_impl.l.ptr      = static_cast<pointer>(std::realloc(m_impl.l.ptr, allocSize));
        m_impl.l.capacity = allocCount - 1;
        return;
    }

    if (newCap < SHORT_CAPACITY)
        return;

    const size_type originalSize = m_impl.s.size;

    const size_type allocCount = alignUpAllocCount(newCap + 1);
    const size_type allocSize  = allocCount * sizeof(value_type);
    const pointer   buffer     = static_cast<pointer>(std::malloc(allocSize));

    // Copy with trailing '\0'.
    traits_type::copy(buffer, m_impl.s.str, originalSize + 1);

    m_impl.l.ptr      = buffer;
    m_impl.l.capacity = allocCount - 1;
    m_impl.l.size     = originalSize;
    m_impl.l.padding  = 0;
    m_impl.l.isLong   = 1;
}

#if !defined(__clang__) && defined(_MSC_VER)
#    pragma warning(pop)
#endif

auto ink::String::insert(size_type position, const_pointer str, size_type count) noexcept
    -> String & {
    const size_type originalSize = this->length();
    this->reserve(originalSize + count);

    // Clamp position.
    position = (position > originalSize ? originalSize : position);

    const pointer buffer    = this->data();
    const pointer bufferEnd = buffer + originalSize;

    // Find insert position.
    const pointer insertTarget = buffer + position;

    // Move original data with trailing '\0'.
    const size_type moveCount = static_cast<size_type>(bufferEnd - insertTarget) + 1;
    traits_type::move(insertTarget + count, insertTarget, moveCount);

    // Insert data.
    traits_type::copy(insertTarget, str, count);

    // Update size.
    if (m_impl.l.isLong)
        m_impl.l.size = originalSize + count;
    else
        m_impl.s.size = static_cast<std::uint8_t>(originalSize + count);

    return *this;
}

auto ink::String::insert(size_type position, size_type count, value_type ch) noexcept -> String & {
    const size_type originalSize = this->length();
    this->reserve(originalSize + count);

    // Clamp position.
    position = (position > originalSize ? originalSize : position);

    const pointer buffer    = this->data();
    const pointer bufferEnd = buffer + originalSize;

    // Find insert position.
    const pointer insertTarget = buffer + position;

    // Move original data with trailing '\0'.
    const size_type moveCount = static_cast<size_type>(bufferEnd - insertTarget) + 1;
    traits_type::move(insertTarget + count, insertTarget, moveCount);

    // Insert data.
    traits_type::assign(insertTarget, count, ch);

    // Update size.
    if (m_impl.l.isLong)
        m_impl.l.size = originalSize + count;
    else
        m_impl.s.size = static_cast<std::uint8_t>(originalSize + count);

    return *this;
}

auto ink::String::remove(size_type from, size_type count) noexcept -> String & {
    const size_type originLength = this->length();
    if (from >= originLength)
        return *this;

    const size_type maxRemoveCount = originLength - from;
    const size_type removeCount    = (count > maxRemoveCount ? maxRemoveCount : count);

    const pointer buffer      = this->data();
    const pointer removeStart = buffer + from;
    const pointer removeEnd   = removeStart + removeCount;

    // Move characters with trailing '\0'.
    const size_type moveCount = originLength - removeCount - from + 1;
    traits_type::move(removeStart, removeEnd, moveCount);

    // Update size.
    if (m_impl.l.isLong)
        m_impl.l.size = originLength - removeCount;
    else
        m_impl.s.size = static_cast<std::uint8_t>(originLength - removeCount);

    return *this;
}

auto ink::String::remove(const_iterator first, const_iterator last) noexcept -> String & {
    if (first > last)
        std::swap(first, last);

    const pointer   buffer       = data();
    const size_type originLength = length();
    const pointer   bufferEnd    = buffer + originLength;

    // Move characters with trailing '\0'.
    const size_type moveCount = static_cast<size_type>(bufferEnd - last) + 1;
    traits_type::move(const_cast<pointer>(first), last, moveCount);

    // Update size.
    if (m_impl.l.isLong)
        m_impl.l.size = originLength - static_cast<size_type>(last - first);
    else
        m_impl.s.size = static_cast<std::uint8_t>(originLength - size_type(last - first));

    return *this;
}

auto ink::String::removeAfter(size_type index) noexcept -> String & {
    const size_type originLength = this->length();
    if (index >= originLength)
        return *this;

    const pointer buffer = data();
    traits_type::assign(*(buffer + index), value_type());

    // Update size.
    if (m_impl.l.isLong)
        m_impl.l.size = index;
    else
        m_impl.s.size = static_cast<std::uint8_t>(index);

    return *this;
}

auto ink::String::push_back(value_type ch) noexcept -> void {
    const size_type len = this->length();
    const size_type cap = this->capacity();

    if (len == cap)
        this->reserve(cap * 2);

    // Append data.
    const pointer buffer = this->data();
    traits_type::assign(*(buffer + len), ch);
    traits_type::assign(*(buffer + len + 1), value_type());

    // Update size.
    if (m_impl.l.isLong)
        m_impl.l.size = len + 1;
    else
        m_impl.s.size = static_cast<std::uint8_t>(len + 1);
}

auto ink::String::pop_back() noexcept -> void {
    const size_type len = this->length();
    if (len == 0)
        return;

    const pointer buffer = this->data();
    traits_type::assign(*(buffer + len - 1), value_type());

    // Update size.
    if (m_impl.l.isLong)
        m_impl.l.size = len - 1;
    else
        m_impl.s.size = static_cast<std::uint8_t>(len - 1);
}

auto ink::String::append(const value_type *str, size_type count) noexcept -> String & {
    const size_type len = this->length();
    const size_type cap = this->capacity();
    if (cap < count + len) {
        const size_type newCap = (len > count ? cap * 2 : len + count);
        this->reserve(newCap);
    }

    // Append data.
    const pointer buffer = this->data();
    traits_type::copy(buffer + len, str, count);

    const size_type newSize = len + count;
    traits_type::assign(*(buffer + newSize), value_type());

    // Update size.
    if (m_impl.l.isLong)
        m_impl.l.size = newSize;
    else
        m_impl.s.size = static_cast<std::uint8_t>(newSize);

    return *this;
}

auto ink::String::append(size_type count, value_type ch) noexcept -> String & {
    const size_type len = this->length();
    const size_type cap = this->capacity();
    if (cap < count + len) {
        const size_type newCap = (len > count ? cap * 2 : len + count);
        this->reserve(newCap);
    }

    // Append data.
    const pointer buffer = this->data();
    traits_type::assign(buffer + len, count, ch);

    const size_type newSize = len + count;
    traits_type::assign(*(buffer + newSize), value_type());

    // Update size.
    if (m_impl.l.isLong)
        m_impl.l.size = newSize;
    else
        m_impl.s.size = static_cast<std::uint8_t>(newSize);

    return *this;
}

auto ink::String::resize(size_type count, value_type ch) noexcept -> void {
    this->reserve(count);

    const size_type len    = this->length();
    const pointer   buffer = this->data();

    if (count > len)
        traits_type::assign(buffer + len, count - len, ch);
    traits_type::assign(*(buffer + count), value_type());

    // Update size.
    if (m_impl.l.isLong)
        m_impl.l.size = count;
    else
        m_impl.s.size = static_cast<std::uint8_t>(count);
}

auto ink::String::trimStart(StringView charSet) noexcept -> String & {
    const pointer   buffer    = this->data();
    const size_type len       = this->length();
    const pointer   bufferEnd = buffer + len;

    pointer trimLast = buffer;
    while (trimLast != bufferEnd && charSet.contains(*trimLast))
        trimLast += 1;

    // Move data with trailing '\0'.
    const size_type newSize = static_cast<size_type>(bufferEnd - trimLast);
    traits_type::move(buffer, trimLast, newSize + 1);

    // Update size.
    if (m_impl.l.isLong)
        m_impl.l.size = newSize;
    else
        m_impl.s.size = static_cast<std::uint8_t>(newSize);

    return *this;
}

auto ink::String::trimEnd(StringView charSet) noexcept -> String & {
    const pointer   buffer    = this->data();
    const size_type len       = this->length();
    const pointer   bufferEnd = buffer + len;

    pointer trimHead = bufferEnd;
    while (trimHead != buffer && charSet.contains(*(trimHead - 1)))
        trimHead -= 1;

    traits_type::assign(*trimHead, value_type());

    // Update size.
    const size_type newSize = static_cast<size_type>(trimHead - buffer);
    if (m_impl.l.isLong)
        m_impl.l.size = newSize;
    else
        m_impl.s.size = static_cast<std::uint8_t>(newSize);

    return *this;
}

auto ink::String::toLower() noexcept -> String & {
    const pointer   buffer    = this->data();
    const size_type len       = this->length();
    const pointer   bufferEnd = buffer + len;

    for (pointer i = buffer; i != bufferEnd; ++i) {
        if (*i >= value_type('A') && *i <= value_type('Z'))
            traits_type::assign(*i, *i - value_type('A') + value_type('a'));
    }

    return *this;
}

auto ink::String::toUpper() noexcept -> String & {
    const pointer   buffer    = this->data();
    const size_type len       = this->length();
    const pointer   bufferEnd = buffer + len;

    for (pointer i = buffer; i != bufferEnd; ++i) {
        if (*i >= value_type('a') && *i <= value_type('z'))
            traits_type::assign(*i, *i - value_type('a') + value_type('A'));
    }

    return *this;
}
