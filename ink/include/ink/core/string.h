#pragma once

#include "ink/core/hash.h"

#include <fmt/format.h>
#include <fmt/xchar.h>

#include <vector>

#define INK_UTF16_(x) u##x
#define INK_UTF16(x) INK_UTF16_(x)

namespace ink {

class InkApi StringView {
public:
    using traits_type     = std::char_traits<char16_t>;
    using value_type      = char16_t;
    using pointer         = value_type *;
    using const_pointer   = const value_type *;
    using reference       = value_type &;
    using const_reference = const value_type &;
    using const_iterator  = const_pointer;
    using iterator        = const_iterator;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    /// @brief
    ///     Create an empty string view.
    constexpr StringView() noexcept = default;

    /// @brief
    ///   Create a new string view from @p std::basic_string_view.
    ///
    /// @param str
    ///   The @p std::basic_string_view to be created from.
    constexpr StringView(std::basic_string_view<value_type> str) noexcept
        : m_ptr(str.data()), m_len(str.size()) {}

    /// @brief
    ///   Create a string view from a consecutive sequence of characters.
    ///
    /// @param str
    ///   Pointer to start of the character sequence.
    /// @param count
    ///   Number of characters in the sequence.
    constexpr StringView(const_pointer str, size_type count) noexcept : m_ptr(str), m_len(count) {}

    /// @brief
    ///   Create a string view from a null-terminated string.
    ///
    /// @param str
    ///   Pointer to start of the null-terminated string.
    constexpr StringView(const_pointer str) noexcept
        : m_ptr(str), m_len(traits_type::length(str)) {}

    /// @brief
    ///   Get the iterator to the first character of this string view if this is not an empty string
    ///   view.
    ///
    /// @return
    ///   The iterator to the first character of this string view.
    [[nodiscard]]
    constexpr auto begin() const noexcept -> const_iterator {
        return m_ptr;
    }

    /// @brief
    ///   Get the iterator to the place after the last character of this string view if this is not
    ///   an empty string view.
    ///
    /// @return
    ///   The iterator to the place after the last character of this string view.
    [[nodiscard]]
    constexpr auto end() const noexcept -> const_iterator {
        return (m_ptr + m_len);
    }

    /// @brief
    ///   Random access characters in this string view.
    /// @note
    ///   Boundary checking is not performed.
    ///
    /// @param i
    ///   Index of the character to be accessed.
    ///
    /// @return
    ///   Reference to the character to be accessed.
    constexpr auto operator[](size_type i) const noexcept -> const_reference {
        return m_ptr[i];
    }

    /// @brief
    ///   Random access characters in this string view.
    /// @note
    ///   Please notice that this method is not the same as @p std::basic_string_view. This method
    ///   is exactly the same as @p operator[].
    ///
    /// @param i
    ///   Index of the character to be accessed.
    /// @return
    ///   Reference to the character to be accessed.
    [[nodiscard]]
    constexpr auto at(size_type i) const noexcept -> const_reference {
        return m_ptr[i];
    }

    /// @brief
    ///   Get the first character of this string view.
    /// @note
    ///   Empty check is not performed.
    ///
    /// @return
    ///   Reference to the first character in this string view.
    [[nodiscard]]
    constexpr auto front() const noexcept -> const_reference {
        return *m_ptr;
    }

    /// @brief
    ///   Get the last character of this string view.
    /// @note
    ///   Empty check is not performed.
    ///
    /// @return
    ///   Reference to the last character of this string view.
    [[nodiscard]]
    constexpr auto back() const noexcept -> const_reference {
        return *(m_ptr + m_len - 1);
    }

    /// @brief
    ///   Get pointer to start of the string view.
    ///
    /// @return
    ///   Return pointer to start of the string view.
    [[nodiscard]]
    constexpr auto data() const noexcept -> const_pointer {
        return m_ptr;
    }

    /// @brief
    ///   Get length of this string view.
    /// @remark
    ///   This is exactly total number of characters of this string. This value could be considered
    ///   as string length if all characters could be stored in a single UTF-16 character.
    ///
    /// @return
    ///   Number of UTF-16 characters of this string view.
    [[nodiscard]]
    constexpr auto length() const noexcept -> size_type {
        return m_len;
    }

    /// @brief
    ///   Checks if this is an empty string view.
    ///
    /// @return
    ///   A boolean value that indicates whether this is an empty string view.
    /// @retval true
    ///   This is an empty string view.
    /// @retval false
    ///   This is not an empty string view.
    [[nodiscard]]
    constexpr auto isEmpty() const noexcept -> bool {
        return (m_len == 0);
    }

    /// @brief
    ///   Checks if this string view is null-terminated.
    /// @note
    ///   It is safe to call this method on empty string view since null check is performed, but it
    ///   may still cause segment fault in certain conditions.
    ///
    /// @return
    ///   A boolean value that indicates whether content of this string view is null-terminated.
    /// @retval true
    ///   This string view is null-terminated.
    /// @retval false
    ///   This string view is not null-terminated.
    [[nodiscard]]
    constexpr auto isNullTerminated() const noexcept -> bool {
        return (m_ptr != nullptr) && (m_ptr[m_len] == value_type());
    }

    /// @brief
    ///   Remove the first @p count characters of this string view.
    ///
    /// @param count
    ///   Expected number of characters to be removed. This value will be clamped if greater than
    ///   length of this string view.
    ///
    /// @return
    ///   Reference to this string view.
    constexpr auto removePrefix(size_type count) noexcept -> StringView & {
        count = (count > m_len ? m_len : count);
        m_ptr += count;
        m_len -= count;
        return *this;
    }

    /// @brief
    ///   Remove the last @p count characters of this string view.
    ///
    /// @param count
    ///   Expected number of characters to be removed. This value will be clamped if greater than
    ///   length of this string view.
    ///
    /// @return
    ///   Reference to this string view.
    constexpr auto removeSuffix(size_type count) noexcept -> StringView & {
        count = (count > m_len ? m_len : count);
        m_len -= count;
        return *this;
    }

#if (__cplusplus >= 202002L) || (defined(_MSVC_LANG) && (_MSVC_LANG >= 202002L))
    /// @brief
    ///   Copy part of this string view to the specified memory.
    ///
    /// @param dest
    ///   Pointer to start of the copy destination.
    /// @param count
    ///   Expected number of characters to be copied. This value will be clamped if greater than
    ///   maximum available characters could be copied.
    /// @param from
    ///   Offset to start of this string view to start copying.
    ///
    /// @return
    ///   Total number of characters copied.
    constexpr auto copyTo(pointer dest, size_type count, size_type from = 0) noexcept -> size_type {
        if (from >= m_len)
            return 0;

        const const_pointer copyStart = m_ptr + from;
        const size_type     maxCount  = m_len - from;

        count = (count > maxCount ? maxCount : count);
        traits_type::copy(dest, copyStart, count);

        return count;
    }
#else
    /// @brief
    ///   Copy part of this string view to the specified memory.
    ///
    /// @param dest
    ///   Pointer to start of the copy destination.
    /// @param count
    ///   Expected number of characters to be copied. This value will be clamped if greater than
    ///   maximum available characters could be copied.
    /// @param from
    ///   Offset to start of this string view to start copying.
    ///
    /// @return
    ///   Total number of characters copied.
    auto copyTo(pointer dest, size_type count, size_type from = 0) noexcept -> size_type {
        if (from >= m_len)
            return 0;

        const const_pointer copyStart = m_ptr + from;
        const size_type     maxCount  = m_len - from;

        count = (count > maxCount ? maxCount : count);
        traits_type::copy(dest, copyStart, count);

        return count;
    }
#endif

    /// @brief
    ///   Create a sub-string from this string view.
    ///
    /// @param from
    ///   Offset from start of this string view to create the sub-string.
    ///
    /// @return
    ///   A string view that contains characters from @p from to the end of this string view.
    [[nodiscard]]
    constexpr auto substr(size_type from) const noexcept -> StringView {
        return (from > m_len) ? StringView() : StringView(m_ptr + from, m_len - from);
    }

    /// @brief
    ///   Create a sub-string from this string view.
    ///
    /// @param from
    ///   Offset from start of this string view to create the sub-string.
    /// @param count
    ///   Expected number of characters in the sub-string. This value will be clamped if greater
    ///   than maximum available characters.
    ///
    /// @return
    ///   A string view that contains @p count characters from @p from of this string view.
    [[nodiscard]]
    constexpr auto substr(size_type from, size_type count) const noexcept -> StringView {
        if (from > m_len)
            return StringView();

        const const_pointer strStart = m_ptr + from;
        const size_type     maxCount = m_len - from;

        count = (count > maxCount ? maxCount : count);
        return StringView(strStart, count);
    }

    /// @brief
    ///   Compare this string view with another one.
    ///
    /// @param rhs
    ///   The string view to be compared with.
    ///
    /// @return
    ///   An integer that indicates the comparision result of the two string views.
    /// @retval -1
    ///   This string view is lexically less than @p rhs.
    /// @retval 0
    ///   This string view is lexically equal to @p rhs.
    /// @retval 1
    ///   This string view is lexically greater than @p rhs.
    [[nodiscard]]
    constexpr auto compare(StringView rhs) const noexcept -> int {
        const size_type cmpLen = (m_len < rhs.m_len ? m_len : rhs.m_len);

        int result = traits_type::compare(m_ptr, rhs.m_ptr, cmpLen);
        if (result == 0)
            result = (m_len < rhs.m_len ? -1 : (m_len > rhs.m_len ? 1 : 0));

        return result;
    }

    /// @brief
    ///   Checks if this string view starts with the specified string pattern.
    ///
    /// @param str
    ///   The string to be compared with beginning part of this string view.
    ///
    /// @return
    ///   A boolean value that indicates whether this string view starts with the specified string
    ///   pattern.
    /// @retval true
    ///   This string view starts with the specified string pattern.
    /// @retval false
    ///   This string view does not start with the specified string pattern.
    [[nodiscard]]
    constexpr auto startsWith(StringView str) const noexcept -> bool {
        if (str.m_len > m_len)
            return false;
        return !traits_type::compare(m_ptr, str.m_ptr, str.m_len);
    }

    /// @brief
    ///   Checks if this string view starts with the specified character.
    ///
    /// @param ch
    ///   The character to be compared with the first character of this string view.
    ///
    /// @return
    ///   A boolean value that indicates whether this string view starts with the specified
    ///   character.
    /// @retval true
    ///   This string view starts with the specified character.
    /// @retval false
    ///   This string view does not start with the specified character.
    [[nodiscard]]
    constexpr auto startsWith(value_type ch) const noexcept -> bool {
        return (m_len != 0) && traits_type::eq(*m_ptr, ch);
    }

    /// @brief
    ///   Checks if this string view ends with the specified string pattern.
    ///
    /// @param str
    ///   The string to be compared with end part of this string view.
    ///
    /// @return
    ///   A boolean value that indicates whether this string view ends with the specified string
    ///   pattern.
    /// @retval true
    ///   This string view ends with the specified string pattern.
    /// @retval false
    ///   This string view does not end with the specified string pattern.
    [[nodiscard]]
    constexpr auto endsWith(StringView str) const noexcept -> bool {
        if (str.m_len > m_len)
            return false;
        return !traits_type::compare(m_ptr + m_len - str.m_len, str.m_ptr, str.m_len);
    }

    /// @brief
    ///   Checks if this string view ends with the specified character.
    ///
    /// @param ch
    ///   The character to be compared with the last character of this string view.
    ///
    /// @return
    ///   A boolean value that indicates whether this string view ends with the specified character.
    /// @retval true
    ///   This string view ends with the specified character.
    /// @retval false
    ///   This string view does not end with the specified character.
    [[nodiscard]]
    constexpr auto endsWith(value_type ch) const noexcept -> bool {
        return (m_len != 0) && traits_type::eq(*(m_ptr + m_len - 1), ch);
    }

    /// @brief
    ///   Checks if this string view contains a sub-string that matches the specified string
    ///   pattern.
    ///
    /// @param str
    ///   The string pattern to be found in this string view.
    ///
    /// @return
    ///   A boolean value that indicates whether this string view contains the specified sub-string.
    /// @retval true
    ///   This string view contains a sub-string that matches the specified string pattern.
    /// @retval false
    ///   This string view does not contain a sub-string that matches the specified string pattern.
    [[nodiscard]]
    constexpr auto contains(StringView str) const noexcept -> bool {
        if (str.m_len > m_len)
            return false;

        const auto cmpEnd = m_ptr + m_len - str.m_len + 1;
        for (auto i = m_ptr; i != cmpEnd; ++i) {
            if (!traits_type::compare(i, str.m_ptr, str.m_len))
                return true;
        }

        return false;
    }

    /// @brief
    ///   Checks if this string view contains the specified character.
    ///
    /// @param ch
    ///   The character to be found in this string view.
    ///
    /// @return
    ///   A boolean value that indicates whether this string view contains the specified character.
    /// @retval true
    ///   This string view contains the specified character.
    /// @retval false
    ///   This string view does not contain the specified character.
    [[nodiscard]]
    constexpr auto contains(value_type ch) const noexcept -> bool {
        const auto cmpEnd = m_ptr + m_len;
        for (auto i = m_ptr; i != cmpEnd; ++i) {
            if (traits_type::eq(*i, ch))
                return true;
        }
        return false;
    }

    /// @brief
    ///   Find index of the first occurance of the specified string pattern in this string view.
    ///
    /// @param str
    ///   The string pattern to be found in this string view.
    /// @param from
    ///   Index to start searching for the specified string pattern.
    ///
    /// @return
    ///   Index to start of the specified string pattern in this string view. -1 will be returned if
    ///   no such string pattern is found.
    [[nodiscard]]
    constexpr auto indexOf(StringView str, size_type from = 0) const noexcept -> size_type {
        if (str.m_len + from > m_len)
            return size_type(-1);

        const auto cmpEnd = m_ptr + m_len - str.m_len + 1;
        for (auto i = m_ptr + from; i != cmpEnd; ++i) {
            if (!traits_type::compare(i, str.m_ptr, str.m_len))
                return static_cast<size_type>(i - m_ptr);
        }

        return size_type(-1);
    }

    /// @brief
    ///   Find index of the first occurance of the specified character in this string view.
    ///
    /// @param ch
    ///   The character to be found in this string view.
    /// @param from
    ///   Index to start searching for the specified character in this string view.
    ///
    /// @return
    ///   Index of the specified character in this string view. -1 will be returned if no such
    ///   character is found.
    [[nodiscard]]
    constexpr auto indexOf(value_type ch, size_type from = 0) const noexcept -> size_type {
        if (from >= m_len)
            return size_type(-1);

        const auto cmpEnd = m_ptr + m_len;
        for (auto i = m_ptr; i != cmpEnd; ++i) {
            if (traits_type::eq(*i, ch))
                return static_cast<size_type>(i - m_ptr);
        }

        return size_type(-1);
    }

    /// @brief
    ///   Find index of the last occurance of the specified string pattern in this string view.
    ///
    /// @param str
    ///   The string to be found in this string view.
    ///
    /// @return
    ///   Index to start of the specified string pattern in this string view. -1 will be returned if
    ///   no such string pattern is found.
    [[nodiscard]]
    constexpr auto lastIndexOf(StringView str) const noexcept -> size_type {
        if (str.m_len > m_len)
            return size_type(-1);

        const auto searchStart = m_ptr + m_len - str.m_len + 1;
        for (auto i = searchStart; i != m_ptr; --i) {
            if (!traits_type::compare(i - 1, str.m_ptr, str.m_len))
                return static_cast<size_type>(i - m_ptr - 1);
        }

        return size_type(-1);
    }

    /// @brief
    ///   Find index of the last occurance of the specified string pattern in this string view.
    ///
    /// @param str
    ///   The string to be found in this string view.
    /// @param from
    ///   Index to start searching for the specified string pattern.
    ///
    /// @return
    ///   Index to start of the specified string pattern in this string view. -1 will be returned if
    ///   no such string pattern is found.
    [[nodiscard]]
    constexpr auto lastIndexOf(StringView str, size_type from) const noexcept -> size_type {
        if (str.m_len > m_len)
            return size_type(-1);

        const auto maxFrom     = m_len - str.m_len;
        from                   = (from > maxFrom ? maxFrom : from);
        const auto searchStart = m_ptr + from + 1;

        for (auto i = searchStart; i != m_ptr; --i) {
            if (!traits_type::compare(i - 1, str.m_ptr, str.m_len))
                return static_cast<size_type>(i - m_ptr - 1);
        }

        return size_type(-1);
    }

    /// @brief
    ///   Find index of the last occurance of the specified character in this string view.
    ///
    /// @param ch
    ///   The character to be found in this string view.
    ///
    /// @return
    ///   Index of the specified character in this string view. -1 will be returned if no such
    ///   character is found.
    [[nodiscard]]
    constexpr auto lastIndexOf(value_type ch) const noexcept -> size_type {
        if (m_len == 0)
            return size_type(-1);

        for (auto i = m_ptr + m_len; i != m_ptr; --i) {
            if (traits_type::eq(*(i - 1), ch))
                return static_cast<size_type>(i - m_ptr - 1);
        }

        return size_type(-1);
    }

    /// @brief
    ///   Find index of the last occurance of the specified character in this string view.
    ///
    /// @param ch
    ///   The character to be found in this string view.
    /// @param from
    ///   Index to start searching for the specified character in this string view.
    ///
    /// @return
    ///   Index of the specified character in this string view. -1 will be returned if no such
    ///   character is found.
    [[nodiscard]]
    constexpr auto lastIndexOf(value_type ch, size_type from) const noexcept -> size_type {
        if (m_len == 0)
            return size_type(-1);

        const auto maxFrom     = m_len - 1;
        from                   = (from > maxFrom ? maxFrom : from);
        const auto searchStart = m_ptr + from + 1;

        for (auto i = searchStart; i != m_ptr; --i) {
            if (traits_type::eq(*(i - 1), ch))
                return static_cast<size_type>(i - m_ptr - 1);
        }

        return size_type(-1);
    }

    /// @brief
    ///   Find index of the first occurancy of any character in the specified character set.
    ///
    /// @param charSet
    ///   A character set that contains characters to be found in this string view.
    /// @param from
    ///   Index to start searching for the characters.
    ///
    /// @return
    ///   Index of the character in this string view. -1 will be returned if none of the given
    ///   characters is found.
    [[nodiscard]]
    constexpr auto indexOfAny(StringView charSet, size_type from = 0) const noexcept -> size_type {
        if (from > m_len)
            return size_type(-1);

        const auto strEnd = m_ptr + m_len;
        for (auto i = m_ptr + from; i != strEnd; ++i) {
            if (charSet.contains(*i))
                return static_cast<size_type>(i - m_ptr);
        }

        return size_type(-1);
    }

    /// @brief
    ///   Find index of the last occurancy of any character in the specified character set.
    ///
    /// @param charSet
    ///   A character set that contains characters to be found in this string view.
    ///
    /// @return
    ///   Index of the character in this string view. -1 will be returned if none of the given
    ///   characters is found.
    [[nodiscard]]
    constexpr auto lastIndexOfAny(StringView charSet) const noexcept -> size_type {
        for (auto i = m_ptr + m_len; i != m_ptr; --i) {
            if (charSet.contains(*(i - 1)))
                return static_cast<size_type>(i - m_ptr - 1);
        }

        return size_type(-1);
    }

    /// @brief
    ///   Find index of the last occurancy of any character in the specified character set.
    ///
    /// @param charSet
    ///   A character set that contains characters to be found in this string view.
    /// @param from
    ///   Index to start searching for the characters.
    ///
    /// @return
    ///   Index of the character in this string view. -1 will be returned if none of the given
    ///   characters is found.
    [[nodiscard]]
    constexpr auto lastIndexOfAny(StringView charSet, size_type from) const noexcept -> size_type {
        const auto maxFrom = m_len - 1;
        from               = (from > maxFrom ? maxFrom : from);

        for (auto i = m_ptr + from + 1; i != m_ptr; --i) {
            if (charSet.contains(*(i - 1)))
                return static_cast<size_type>(i - m_ptr - 1);
        }

        return size_type(-1);
    }

    /// @brief
    ///   Remove all leading characters that are contained in the specified character set.
    ///
    /// @param charSet
    ///   The character set that contains all the characters to be removed. The default character
    ///   set is whitespace characters.
    ///
    /// @return
    ///   Reference to this string view.
    constexpr auto trimStart(StringView charSet = u" \f\n\r\t\v") noexcept -> StringView & {
        while (m_len && charSet.contains(*m_ptr)) {
            m_ptr += 1;
            m_len -= 1;
        }
        return *this;
    }

    /// @brief
    ///   Remove all trailing characters that are contained in the specified character set.
    ///
    /// @param charSet
    ///   The character set that contains all the characters to be removed. The default character
    ///   set is whitespace characters.
    ///
    /// @return
    ///   Reference to this string view.
    constexpr auto trimEnd(StringView charSet = u" \f\n\r\t\v") noexcept -> StringView & {
        while (m_len && charSet.contains(*(m_ptr + m_len - 1)))
            m_len -= 1;
        return *this;
    }

    /// @brief
    ///   Remove all leading and trailing characters that are contained in the specified character
    ///   set.
    ///
    /// @param charSet
    ///   The character set that contains all the characters to be removed. The default character
    ///   set is whitespace characters.
    ///
    /// @return
    ///   Reference to this string view.
    constexpr auto trim(StringView charSet = u" \f\n\r\t\v") noexcept -> StringView & {
        while (m_len && charSet.contains(*m_ptr)) {
            m_ptr += 1;
            m_len -= 1;
        }

        while (m_len && charSet.contains(*(m_ptr + m_len - 1)))
            m_len -= 1;

        return *this;
    }

    /// @brief
    ///   Split this string view by the specified delimiter character.
    ///
    /// @param delim
    ///   The delimiter character that is used to split this string view.
    ///
    /// @return
    ///   A string view array that contains the split result.
    [[nodiscard]]
    auto split(value_type delim) const noexcept -> std::vector<StringView>;

    /// @brief
    ///   Split this string view by any of the characters in the delimiter character set.
    ///
    /// @param delims
    ///   The delimiter character set that contains characters to be used to separate this string
    ///   view.
    ///
    /// @return
    ///   A string view array that contains the split result.
    [[nodiscard]]
    auto splitByAnyOf(StringView delims) const noexcept -> std::vector<StringView>;

    /// @brief
    ///   Convert this UTF-16 string to an UTF-8 string.
    ///
    /// @return
    ///   A std::string that contains the UTF-8 encoded string of this string view.
    [[nodiscard]]
    auto toUTF8String() const noexcept -> std::string;

    /// @brief
    ///   Calculate hash value of this string view.
    ///
    /// @return
    ///   Hash value of this string view.
    [[nodiscard]]
    auto hash() const noexcept -> std::size_t {
        return ink::hash(m_ptr, m_len * sizeof(value_type));
    }

private:
    /// @brief
    ///   Pointer to start of the string.
    const value_type *m_ptr = nullptr;

    /// @brief
    ///   Length of the string.
    size_type m_len = 0;
};

constexpr auto operator==(StringView lhs, StringView rhs) noexcept -> bool {
    return lhs.compare(rhs) == 0;
}

constexpr auto operator!=(StringView lhs, StringView rhs) noexcept -> bool {
    return lhs.compare(rhs) != 0;
}

constexpr auto operator<(StringView lhs, StringView rhs) noexcept -> bool {
    return lhs.compare(rhs) < 0;
}

constexpr auto operator<=(StringView lhs, StringView rhs) noexcept -> bool {
    return lhs.compare(rhs) <= 0;
}

constexpr auto operator>(StringView lhs, StringView rhs) noexcept -> bool {
    return lhs.compare(rhs) > 0;
}

constexpr auto operator>=(StringView lhs, StringView rhs) noexcept -> bool {
    return lhs.compare(rhs) >= 0;
}

constexpr auto operator==(StringView lhs, const char16_t *rhs) noexcept -> bool {
    return lhs.compare(rhs) == 0;
}

constexpr auto operator!=(StringView lhs, const char16_t *rhs) noexcept -> bool {
    return lhs.compare(rhs) != 0;
}

constexpr auto operator<(StringView lhs, const char16_t *rhs) noexcept -> bool {
    return lhs.compare(rhs) < 0;
}

constexpr auto operator<=(StringView lhs, const char16_t *rhs) noexcept -> bool {
    return lhs.compare(rhs) <= 0;
}

constexpr auto operator>(StringView lhs, const char16_t *rhs) noexcept -> bool {
    return lhs.compare(rhs) > 0;
}

constexpr auto operator>=(StringView lhs, const char16_t *rhs) noexcept -> bool {
    return lhs.compare(rhs) >= 0;
}

constexpr auto operator==(const char16_t *lhs, StringView rhs) noexcept -> bool {
    return StringView(lhs).compare(rhs) == 0;
}

constexpr auto operator!=(const char16_t *lhs, StringView rhs) noexcept -> bool {
    return StringView(lhs).compare(rhs) != 0;
}

constexpr auto operator<(const char16_t *lhs, StringView rhs) noexcept -> bool {
    return StringView(lhs).compare(rhs) < 0;
}

constexpr auto operator<=(const char16_t *lhs, StringView rhs) noexcept -> bool {
    return StringView(lhs).compare(rhs) <= 0;
}

constexpr auto operator>(const char16_t *lhs, StringView rhs) noexcept -> bool {
    return StringView(lhs).compare(rhs) > 0;
}

constexpr auto operator>=(const char16_t *lhs, StringView rhs) noexcept -> bool {
    return StringView(lhs).compare(rhs) >= 0;
}

} // namespace ink

template <>
struct std::hash<ink::StringView> {
    auto operator()(ink::StringView str) const noexcept -> std::size_t {
        return str.hash();
    }
};

template <>
struct fmt::formatter<ink::StringView, char16_t>
    : fmt::formatter<fmt::basic_string_view<char16_t>, char16_t> {
    template <typename FormatContext>
    auto format(ink::StringView str, FormatContext &ctx) const -> decltype(ctx.out()) {
        using Super = fmt::formatter<fmt::basic_string_view<char16_t>, char16_t>;
        return this->Super::format(fmt::basic_string_view<char16_t>(str.data(), str.length()), ctx);
    }
};

namespace ink {

class InkApi String {
public:
    using traits_type     = std::char_traits<char16_t>;
    using value_type      = char16_t;
    using pointer         = value_type *;
    using const_pointer   = const value_type *;
    using reference       = value_type &;
    using const_reference = const value_type &;
    using iterator        = pointer;
    using const_iterator  = const_pointer;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

private:
    struct LongStr {
        pointer   ptr;
        size_type capacity;
        size_type size;
        size_type padding : sizeof(size_type) * 8 - 1;
        size_type isLong : 1;
    };

    static const size_type SHORT_CAPACITY = (sizeof(LongStr) - 1) / sizeof(value_type);

    struct ShortStr {
        value_type   str[SHORT_CAPACITY];
        std::uint8_t size;
        std::uint8_t padding : 7;
        std::uint8_t isLong  : 1;
    };

public:
    /// @brief
    ///   Create an empty string.
    String() noexcept : m_impl() {}

    /// @brief
    ///   Create a string from a consecutive sequence of characters.
    ///
    /// @param ptr
    ///   Pointer to start of the character sequence.
    /// @param len
    ///   Number of characters in the sequence.
    String(const value_type *ptr, size_type len) noexcept;

    /// @brief
    ///   Create a string from a null-terminated string.
    ///
    /// @param str
    ///   Pointer to start of the null-terminated string.
    String(const value_type *ptr) noexcept;

    /// @brief
    ///   Create a string from a string view.
    ///
    /// @param str
    ///   The string to be copied from.
    String(StringView str) noexcept;

    /// @brief
    ///   Copy constructor of string.
    ///
    /// @param other
    ///   The string to be copied from.
    String(const String &other) noexcept;

    /// @brief
    ///   Move constructor of string.
    ///
    /// @param other
    ///   The string to be moved from. The moved string will be invalidated.
    String(String &&other) noexcept : m_impl(other.m_impl) {
        std::memset(&other.m_impl, 0, sizeof(other.m_impl));
    }

    /// @brief
    ///   Destroy this string object.
    ~String() noexcept;

    /// @brief
    ///   Assign another string to this one.
    ///
    /// @param other
    ///   The string to be copied from.
    ///
    /// @return
    ///   Reference to this string.
    auto assign(const String &other) noexcept -> String &;

    /// @brief
    ///   Assign another string to this one.
    ///
    /// @param other
    ///   The string to be moved from. The moved string will be invalidated.
    ///
    /// @return
    ///   Reference to this string.
    auto assign(String &&other) noexcept -> String &;

    /// @brief
    ///   Assign another string to this one.
    ///
    /// @param str
    ///   The string to be copied from.
    ///
    /// @return
    ///   Reference to this string.
    auto assign(StringView str) noexcept -> String &;

    /// @brief
    ///   Assign a sequence of characters to this string.
    ///
    /// @param str
    ///   Pointer to start of the character sequence.
    /// @param len
    ///   Number of characters in the character sequence.
    ///
    /// @return
    ///   Reference to this string.
    auto assign(const_pointer str, size_type len) noexcept -> String & {
        return this->assign(StringView(str, len));
    }

    /// @brief
    ///   Assign a null-terminated string to this string.
    ///
    /// @param ptr
    ///   The null-terminated string to be copied from.
    ///
    /// @return
    ///   Reference to this string.
    auto assign(const_pointer ptr) noexcept -> String &;

    /// @brief
    ///   Copy assignment of string.
    ///
    /// @param other
    ///   The string to be copied from.
    ///
    /// @return
    ///   Reference to this string.
    auto operator=(const String &other) noexcept -> String & {
        return this->assign(other);
    }

    /// @brief
    ///   Move assignment of string.
    ///
    /// @param other
    ///   The string to be moved from. The moved string will be invalidated.
    ///
    /// @return
    ///   Reference to this string.
    auto operator=(String &&other) noexcept -> String & {
        return this->assign(static_cast<String &&>(other));
    }

    /// @brief
    ///   Assign another string to this one.
    ///
    /// @param other
    ///   The string to be copied from.
    ///
    /// @return
    ///   Reference to this string.
    auto operator=(StringView other) noexcept -> String & {
        return this->assign(other);
    }

    /// @brief
    ///   Assign a null-terminated string to this string.
    ///
    /// @param other
    ///   The null-terminated string to be copied from.
    ///
    /// @return
    ///   Reference to this string.
    auto operator=(const_pointer other) noexcept -> String & {
        return this->assign(other);
    }

    /// @brief
    ///   Random access characters in this string.
    /// @note
    ///   Boundary checking is not performed.
    ///
    /// @param i
    ///   Index of the character to be accessed.
    ///
    /// @return
    ///   Reference to the character to be accessed.
    auto operator[](size_type i) noexcept -> reference {
        return data()[i];
    }

    /// @brief
    ///   Random access characters in this string.
    /// @note
    ///   Boundary checking is not performed.
    ///
    /// @param i
    ///   Index of the character to be accessed.
    ///
    /// @return
    ///   Reference to the character to be accessed.
    auto operator[](size_type i) const noexcept -> const_reference {
        return data()[i];
    }

    /// @brief
    ///   Get the first character of this string.
    /// @note
    ///   Empty check is not performed.
    ///
    /// @return
    ///   Reference to the first character in this string.
    [[nodiscard]]
    auto front() noexcept -> reference {
        return *data();
    }

    /// @brief
    ///   Get the first character of this string.
    /// @note
    ///   Empty check is not performed.
    ///
    /// @return
    ///   Reference to the first character in this string.
    [[nodiscard]]
    auto front() const noexcept -> const_reference {
        return *data();
    }

    /// @brief
    ///   Get the last character of this string.
    /// @note
    ///   Empty check is not performed.
    ///
    /// @return
    ///   Reference to the last character of this string.
    [[nodiscard]]
    auto back() noexcept -> reference {
        return *(m_impl.l.isLong ? (m_impl.l.ptr + m_impl.l.size - 1)
                                 : (m_impl.s.str + m_impl.s.size - 1));
    }

    /// @brief
    ///   Get the last character of this string.
    /// @note
    ///   Empty check is not performed.
    ///
    /// @return
    ///   Reference to the last character of this string.
    [[nodiscard]]
    auto back() const noexcept -> const_reference {
        return *(m_impl.l.isLong ? (m_impl.l.ptr + m_impl.l.size - 1)
                                 : (m_impl.s.str + m_impl.s.size - 1));
    }

    /// @brief
    ///   Get pointer to start of this string.
    ///
    /// @return
    ///   Pointer to start of this string.
    [[nodiscard]]
    auto data() noexcept -> pointer {
        return (m_impl.l.isLong ? m_impl.l.ptr : m_impl.s.str);
    }

    /// @brief
    ///   Get pointer to start of this string.
    ///
    /// @return
    ///   Pointer to start of this string.
    [[nodiscard]]
    auto data() const noexcept -> const_pointer {
        return (m_impl.l.isLong ? m_impl.l.ptr : m_impl.s.str);
    }

    /// @brief
    ///   Allow implicit conversion to string view.
    operator StringView() const noexcept {
        return m_impl.l.isLong ? StringView(m_impl.l.ptr, m_impl.l.size)
                               : StringView(m_impl.s.str, m_impl.s.size);
    }

    /// @brief
    ///   Get the iterator to the first character of this string if this is not an empty string.
    ///
    /// @return
    ///   The iterator to the first character of this string.
    [[nodiscard]]
    auto begin() noexcept -> iterator {
        return data();
    }

    /// @brief
    ///   Get the iterator to the first character of this string if this is not an empty string.
    ///
    /// @return
    ///   The iterator to the first character of this string.
    [[nodiscard]]
    auto begin() const noexcept -> const_iterator {
        return data();
    }

    /// @brief
    ///   Get the iterator to the place after the last character of this string if this is not an
    ///   empty string.
    ///
    /// @return
    ///   The iterator to the place after the last character of this string.
    [[nodiscard]]
    auto end() noexcept -> iterator {
        return (m_impl.l.isLong ? (m_impl.l.ptr + m_impl.l.size) : (m_impl.s.str + m_impl.s.size));
    }

    /// @brief
    ///   Get the iterator to the place after the last character of this string if this is not an
    ///   empty string.
    ///
    /// @return
    ///   The iterator to the place after the last character of this string.
    [[nodiscard]]
    auto end() const noexcept -> const_iterator {
        return (m_impl.l.isLong ? (m_impl.l.ptr + m_impl.l.size) : (m_impl.s.str + m_impl.s.size));
    }

    /// @brief
    ///   Get length of this string.
    /// @remark
    ///   This is exactly total number of characters of this string. This value could be considered
    ///   as string length if all characters could be stored in a single UTF-16 character.
    ///
    /// @return
    ///   Number of UTF-16 characters of this string.
    [[nodiscard]]
    auto length() const noexcept -> size_type {
        return (m_impl.l.isLong ? m_impl.l.size : m_impl.s.size);
    }

    /// @brief
    ///   Checks if this is an empty string.
    ///
    /// @return
    ///   A boolean value that indicates whether this is an empty string.
    /// @retval true
    ///   This is an empty string.
    /// @retval false
    ///   This is not an empty string.
    [[nodiscard]]
    auto isEmpty() const noexcept -> bool {
        return length() == 0;
    }

    /// @brief
    ///   Reserve at least @p newCap characters of memory for this string.
    ///
    /// @param newCap
    ///   Expected number of characters to be stored in this string.
    auto reserve(size_type newCap) noexcept -> void;

    /// @brief
    ///   Get capacity of this string.
    ///
    /// @return
    ///   Total number of characters that this string could store currently.
    [[nodiscard]]
    auto capacity() const noexcept -> size_type {
        return (m_impl.l.isLong ? m_impl.l.capacity : (SHORT_CAPACITY - 1));
    }

    /// @brief
    ///   Remove all characters in this string.
    auto clear() noexcept -> void {
        if (m_impl.l.isLong) {
            traits_type::assign(m_impl.l.ptr[0], value_type());
            m_impl.l.size = 0;
        } else {
            traits_type::assign(m_impl.s.str[0], value_type());
            m_impl.s.size = 0;
        }
    }

    /// @brief
    ///   Insert a sequence of characters at the specified position of this string.
    /// @note
    ///   This method is equivalent to @p append() if @p position is greater than length of this
    ///   string.
    ///
    /// @param position
    ///   Index of the character to insert the specified character sequence.
    /// @param str
    ///   Pointer to start of the character to be inserted.
    /// @param count
    ///   Number of characters in the character sequence to be inserted.
    ///
    /// @return
    ///   Reference to this string.
    auto insert(size_type position, const_pointer str, size_type count) noexcept -> String &;

    /// @brief
    ///   Insert a string to the specified position of this string.
    /// @note
    ///   This method is equivalent to @p append() if @p position is greater than length of this
    ///   string.
    ///
    /// @param position
    ///   Index of the character to insert the specified string.
    /// @param str
    ///   The string to be inserted.
    ///
    /// @return
    ///   Reference to this string.
    auto insert(size_type position, StringView str) noexcept -> String & {
        return this->insert(position, str.data(), str.length());
    }

    /// @brief
    ///   Insert @p count characters at the specified position of this string.
    /// @note
    ///   This method is equivalent to @p append() if @p position is greater than length of this
    ///   string.
    ///
    /// @param position
    ///   Index of the character to insert the new characters.
    /// @param count
    ///   Number of characters to be copied.
    /// @param ch
    ///   The character to be copied.
    ///
    /// @return
    ///   Reference to this string.
    auto insert(size_type position, size_type count, value_type ch) noexcept -> String &;

    /// @brief
    ///   Remove @p count characters from the specified index.
    ///
    /// @param from
    ///   Index of the first character to be removed.
    /// @param count
    ///   Expected number of characters to be removed. This value will be clamped if greater than
    ///   maximum available characters that could be removed.
    ///
    /// @return
    ///   Reference to this string.
    auto remove(size_type from, size_type count) noexcept -> String &;

    /// @brief
    ///   Remove all characters in the given range.
    ///
    /// @param first
    ///   Iterator to the first character to be removed. This iterator may be invalidated after
    ///   remove operation.
    /// @param last
    ///   Iterator to the position after the last character to be removed. This iterator will be
    ///   invalidated after remove operation.
    ///
    /// @return
    ///   Reference to this string.
    auto remove(const_iterator first, const_iterator last) noexcept -> String &;

    /// @brief
    ///   Remove all characters after the specified index.
    ///
    /// @param index
    ///   Index of the first character to be removed.
    ///
    /// @return
    ///   Reference to this string.
    auto removeAfter(size_type index) noexcept -> String &;

    /// @brief
    ///   Append the specified character to the end of this string.
    ///
    /// @param ch
    ///   The character to be appended.
    auto push_back(value_type ch) noexcept -> void;

    /// @brief
    ///   Remove the last character in this string. Empty check is performed.
    auto pop_back() noexcept -> void;

    /// @brief
    ///   Append a sequence of characters to the end of this string.
    ///
    /// @param str
    ///   Pointer to start of the character sequence to be appended.
    /// @param count
    ///   Number of characters in the character sequence to be appended.
    ///
    /// @return
    ///   Reference to this string.
    auto append(const value_type *str, size_type count) noexcept -> String &;

    /// @brief
    ///   Append @p count copies of the specified character at the end of this string.
    ///
    /// @param count
    ///   Number of the specified characters to be copied.
    /// @param ch
    ///   The character to be copied.
    ///
    /// @return
    ///   Reference to this string.
    auto append(size_type count, value_type ch) noexcept -> String &;

    /// @brief
    ///   Append the specified string to the end of this string.
    ///
    /// @param str
    ///   The string to be appended.
    ///
    /// @return
    ///   Reference to this string.
    auto append(StringView str) noexcept -> String & {
        return this->append(str.data(), str.length());
    }

    /// @brief
    ///   Append the specified character to the end of this string.
    /// @remark
    ///   This method is the same as @p push_back().
    ///
    /// @param ch
    ///   The character to be appended.
    ///
    /// @return
    ///   Reference to this string.
    auto append(value_type ch) noexcept -> String & {
        this->push_back(ch);
        return *this;
    }

    /// @brief
    ///   Append the specified character to the end of this string.
    ///
    /// @param ch
    ///   The character to be appended.
    ///
    /// @return
    ///   Reference to this string.
    auto operator+=(value_type ch) noexcept -> String & {
        this->push_back(ch);
        return *this;
    }

    /// @brief
    ///   Append the specified null-terminated string to the end of this string.
    ///
    /// @param str
    ///   The null-terminated string to be appended.
    ///
    /// @return
    ///   Reference to this string.
    auto operator+=(const value_type *str) noexcept -> String & {
        return this->append(str, traits_type::length(str));
    }

    /// @brief
    ///   Append the specified string to the end of this string.
    ///
    /// @param str
    ///   The string to be appended.
    ///
    /// @return
    ///   Reference to this string.
    auto operator+=(StringView str) noexcept -> String & {
        return this->append(str);
    }

    /// @brief
    ///   Append the specified string to the end of this string.
    ///
    /// @param str
    ///   The string to be appended.
    ///
    /// @return
    ///   Reference to this string.
    auto operator+=(const String &str) noexcept -> String & {
        return (str.m_impl.l.isLong ? append(str.m_impl.l.ptr, str.m_impl.l.size)
                                    : append(str.m_impl.s.str, str.m_impl.s.size));
    }

    /// @brief
    ///   Compare this string with another one.
    ///
    /// @param rhs
    ///   The string to be compared with.
    ///
    /// @return
    ///   An integer that represents the comparision result of the two strings.
    /// @retval -1
    ///   This string is lexically less than @p rhs.
    /// @retval 0
    ///   This string is lexically equal to @p rhs.
    /// @retval 1
    ///   This string is lexically greater than @p rhs.
    [[nodiscard]]
    auto compare(StringView rhs) const noexcept -> int {
        return StringView(*this).compare(rhs);
    }

    /// @brief
    ///   Checks if this string starts with the specified string pattern.
    ///
    /// @param str
    ///   The string to be compared with beginning part of this string.
    ///
    /// @return
    ///   A boolean value that indicates whether this string starts with the specified string
    ///   pattern.
    /// @retval true
    ///   This string starts with the specified string pattern.
    /// @retval false
    ///   This string does not start with the specified string pattern.
    [[nodiscard]]
    auto startsWith(StringView str) const noexcept -> bool {
        return StringView(*this).startsWith(str);
    }

    /// @brief
    ///   Checks if this string starts with the specified character.
    ///
    /// @param ch
    ///   The character to be compared with the first character of this string.
    ///
    /// @return
    ///   A boolean value that indicates whether this string starts with the specified character.
    /// @retval true
    ///   This string starts with the specified character.
    /// @retval false
    ///   This string does not start with the specified character.
    [[nodiscard]]
    auto startsWith(value_type ch) const noexcept -> bool {
        return StringView(*this).startsWith(ch);
    }

    /// @brief
    ///   Checks if this string ends with the specified string pattern.
    ///
    /// @param str
    ///   The string to be compared with end part of this string.
    ///
    /// @return
    ///   A boolean value that indicates whether this string ends with the specified string pattern.
    /// @retval true
    ///   This string ends with the specified string pattern.
    /// @retval false
    ///   This string does not end with the specified string pattern.
    [[nodiscard]]
    auto endsWith(StringView str) const noexcept -> bool {
        return StringView(*this).endsWith(str);
    }

    /// @brief
    ///   Checks if this string ends with the specified character.
    ///
    /// @param ch
    ///   The character to be compared with the last character of this string.
    ///
    /// @return
    ///   A boolean value that indicates whether this string ends with the specified character.
    /// @retval true
    ///   This string ends with the specified character.
    /// @retval false
    ///   This string does not end with the specified character.
    [[nodiscard]]
    auto endsWith(value_type ch) const noexcept -> bool {
        return StringView(*this).endsWith(ch);
    }

    /// @brief
    ///   Checks if this string contains a sub-string that matches the specified string pattern.
    ///
    /// @param str
    ///   The string pattern to be found in this string.
    ///
    /// @return
    ///   A boolean value that indicates whether this string contains the specified sub-string.
    /// @retval true
    ///   This string contains a sub-string that matches the specified string pattern.
    /// @retval false
    ///   This string does not contain a sub-string that matches the specified string pattern.
    [[nodiscard]]
    auto contains(StringView str) const noexcept -> bool {
        return StringView(*this).contains(str);
    }

    /// @brief
    ///   Checks if this string contains the specified character.
    ///
    /// @param ch
    ///   The character to be found in this string.
    ///
    /// @return
    ///   A boolean value that indicates whether this string contains the specified character.
    /// @retval true
    ///   This string contains the specified character.
    /// @retval false
    ///   This string does not contain the specified character.
    [[nodiscard]]
    auto contains(value_type ch) const noexcept -> bool {
        return StringView(*this).contains(ch);
    }

    /// @brief
    ///   Create a sub-string from this string.
    ///
    /// @param from
    ///   Offset from start of this string to create the sub-string.
    ///
    /// @return
    ///   A string view that contains characters from @p from to the end of this string.
    [[nodiscard]]
    auto substr(size_type from) const noexcept -> StringView {
        return StringView(*this).substr(from);
    }

    /// @brief
    ///   Create a sub-string from this string.
    ///
    /// @param from
    ///   Offset from start of this string to create the sub-string.
    /// @param count
    ///   Expected number of characters in the sub-string. This value will be clamped if greater
    ///   than maximum available characters.
    ///
    /// @return
    ///   A string view that contains @p count characters from @p from of this string.
    [[nodiscard]]
    auto substr(size_type from, size_type count) const noexcept -> StringView {
        return StringView(*this).substr(from, count);
    }

    /// @brief
    ///   Copy part of this string to the specified memory.
    ///
    /// @param dest
    ///   Pointer to start of the copy destination.
    /// @param count
    ///   Expected number of characters to be copied. This value will be clamped if greater than
    ///   maximum available characters could be copied.
    /// @param from
    ///   Offset to start of this string to start copying.
    ///
    /// @return
    ///   Total number of characters copied.
    auto copyTo(pointer dest, size_type count, size_type from = 0) const noexcept -> size_type {
        return StringView(*this).copyTo(dest, count, from);
    }

    /// @brief
    ///   Resize this string to hold @p count characters.
    ///
    /// @param count
    ///   New size of this string.
    /// @param ch
    ///   Character to initialize the new memory with.
    auto resize(size_type count, value_type ch = value_type()) noexcept -> void;

    /// @brief
    ///   Find index of the first occurance of the specified string pattern in this string.
    ///
    /// @param str
    ///   The string pattern to be found in this string.
    /// @param from
    ///   Index to start searching for the specified string pattern.
    ///
    /// @return
    ///   Index to start of the specified string pattern in this string. -1 will be returned if no
    ///   such string pattern is found.
    [[nodiscard]]
    auto indexOf(StringView str, size_type from = 0) const noexcept -> size_type {
        return StringView(*this).indexOf(str, from);
    }

    /// @brief
    ///   Find index of the first occurance of the specified character in this string.
    ///
    /// @param ch
    ///   The character to be found in this string.
    /// @param from
    ///   Index to start searching for the specified character in this string.
    ///
    /// @return
    ///   Index of the specified character in this string. -1 will be returned if no such character
    ///   is found.
    [[nodiscard]]
    auto indexOf(value_type ch, size_type from = 0) const noexcept -> size_type {
        return StringView(*this).indexOf(ch, from);
    }

    /// @brief
    ///   Find index of the last occurance of the specified string pattern in this string.
    ///
    /// @param str
    ///   The string pattern to be found in this string.
    ///
    /// @return
    ///   Index to start of the specified string pattern in this string. -1 will be returned if no
    ///   such string pattern is found.
    [[nodiscard]]
    auto lastIndexOf(StringView str) const noexcept -> size_type {
        return StringView(*this).lastIndexOf(str);
    }

    /// @brief
    ///   Find index of the last occurance of the specified string pattern in this string.
    ///
    /// @param str
    ///   The string pattern to be found in this string.
    /// @param from
    ///   Index to start searching for the specified string pattern.
    ///
    /// @return
    ///   Index to start of the specified string pattern in this string. -1 will be returned if no
    ///   such string pattern is found.
    [[nodiscard]]
    auto lastIndexOf(StringView str, size_type from) const noexcept -> size_type {
        return StringView(*this).lastIndexOf(str, from);
    }

    /// @brief
    ///   Find index of the last occurance of the specified character in this string.
    ///
    /// @param ch
    ///   The character to be found in this string.
    ///
    /// @return
    ///   Index of the specified character in this string. -1 will be returned if no such character
    ///   is found.
    [[nodiscard]]
    auto lastIndexOf(value_type ch) const noexcept -> size_type {
        return StringView(*this).lastIndexOf(ch);
    }

    /// @brief
    ///   Find index of the last occurance of the specified character in this string.
    ///
    /// @param ch
    ///   The character to be found in this string.
    /// @param from
    ///   Index to start searching for the specified character in this string.
    ///
    /// @return
    ///   Index of the specified character in this string. -1 will be returned if no such character
    ///   is found.
    [[nodiscard]]
    auto lastIndexOf(value_type ch, size_type from) const noexcept -> size_type {
        return StringView(*this).lastIndexOf(ch, from);
    }

    /// @brief
    ///   Find index of the first occurancy of any character in the specified character set.
    ///
    /// @param charSet
    ///   A character set that contains characters to be found in this string.
    /// @param from
    ///   Index to start searching for the characters.
    ///
    /// @return
    ///   Index of the character in this string. -1 will be returned if none of the given characters
    ///   is found.
    [[nodiscard]]
    auto indexOfAny(StringView charSet, size_type from = 0) const noexcept -> size_type {
        return StringView(*this).indexOfAny(charSet, from);
    }

    /// @brief
    ///   Find index of the last occurancy of any character in the specified character set.
    ///
    /// @param charSet
    ///   A character set that contains characters to be found in this string.
    ///
    /// @return
    ///   Index of the character in this string. -1 will be returned if none of the given characters
    ///   is found.
    [[nodiscard]]
    auto lastIndexOfAny(StringView charSet) const noexcept -> size_type {
        return StringView(*this).lastIndexOfAny(charSet);
    }

    /// @brief
    ///   Find index of the last occurancy of any character in the specified character set.
    ///
    /// @param charSet
    ///   A character set that contains characters to be found in this string.
    /// @param from
    ///   Index to start searching for the characters.
    ///
    /// @return
    ///   Index of the character in this string. -1 will be returned if none of the given characters
    ///   is found.
    [[nodiscard]]
    auto lastIndexOfAny(StringView charSet, size_type from) const noexcept -> size_type {
        return StringView(*this).lastIndexOfAny(charSet, from);
    }

    /// @brief
    ///   Remove all leading characters that are contained in the specified character set.
    ///
    /// @param charSet
    ///   The character set that contains all the characters to be removed. The default character
    ///   set is whitespace characters.
    ///
    /// @return
    ///   Reference to this string.
    auto trimStart(StringView charSet = u" \f\n\r\t\v") noexcept -> String &;

    /// @brief
    ///   Remove all trailing characters that are contained in the specified character set.
    ///
    /// @param charSet
    ///   The character set that contains all the characters to be removed. The default character
    ///   set is whitespace characters.
    ///
    /// @return
    ///   Reference to this string.
    auto trimEnd(StringView charSet = u" \f\n\r\t\v") noexcept -> String &;

    /// @brief
    ///   Remove all leading and trailing characters that are contained in the specified character
    ///   set.
    ///
    /// @param charSet
    ///   The character set that contains all the characters to be removed. The default character
    ///   set is whitespace characters.
    ///
    /// @return
    ///   Reference to this string.
    auto trim(StringView charSet = u" \f\n\r\t\v") noexcept -> String & {
        return this->trimEnd(charSet).trimStart(charSet);
    }

    /// @brief
    ///   Pad beginning of this string with the specified character.
    ///
    /// @param count
    ///   Number of characters to be padded.
    /// @param ch
    ///   The character to be padded.
    ///
    /// @return
    ///   Reference to this string.
    auto padLeft(size_type count, value_type ch = u' ') noexcept -> String & {
        return this->insert(0, count, ch);
    }

    /// @brief
    ///   Pad end of this string with the specified character.
    ///
    /// @param count
    ///   Number of characters to be padded.
    /// @param ch
    ///   The character to be padded.
    ///
    /// @return
    ///   Reference to this string.
    auto padRight(size_type count, value_type ch = u' ') noexcept -> String & {
        return this->append(count, ch);
    }

    /// @brief
    ///   Convert all English alphabets to lowercase.
    ///
    /// @return
    ///   Reference to this string.
    auto toLower() noexcept -> String &;

    /// @brief
    ///   Convert all English alphabets to uppercase.
    ///
    /// @return
    ///   Reference to this string.
    auto toUpper() noexcept -> String &;

    /// @brief
    ///   Calculate hash value of this string.
    ///
    /// @return
    ///   Hash value of this string.
    [[nodiscard]]
    auto hash() const noexcept -> std::size_t {
        return (m_impl.l.isLong ? ink::hash(m_impl.l.ptr, m_impl.l.size * sizeof(value_type))
                                : ink::hash(m_impl.s.str, m_impl.s.size * sizeof(value_type)));
    }

private:
    union {
        LongStr  l;
        ShortStr s;
    } m_impl;
};

inline auto operator==(const String &lhs, const String &rhs) noexcept -> bool {
    return StringView(lhs) == StringView(rhs);
}

inline auto operator!=(const String &lhs, const String &rhs) noexcept -> bool {
    return StringView(lhs) != StringView(rhs);
}

inline auto operator<(const String &lhs, const String &rhs) noexcept -> bool {
    return StringView(lhs) < StringView(rhs);
}

inline auto operator<=(const String &lhs, const String &rhs) noexcept -> bool {
    return StringView(lhs) <= StringView(rhs);
}

inline auto operator>(const String &lhs, const String &rhs) noexcept -> bool {
    return StringView(lhs) > StringView(rhs);
}

inline auto operator>=(const String &lhs, const String &rhs) noexcept -> bool {
    return StringView(lhs) >= StringView(rhs);
}

inline auto operator==(const String &lhs, StringView rhs) noexcept -> bool {
    return StringView(lhs) == rhs;
}

inline auto operator!=(const String &lhs, StringView rhs) noexcept -> bool {
    return StringView(lhs) != rhs;
}

inline auto operator<(const String &lhs, StringView rhs) noexcept -> bool {
    return StringView(lhs) < rhs;
}

inline auto operator<=(const String &lhs, StringView rhs) noexcept -> bool {
    return StringView(lhs) <= rhs;
}

inline auto operator>(const String &lhs, StringView rhs) noexcept -> bool {
    return StringView(lhs) > rhs;
}

inline auto operator>=(const String &lhs, StringView rhs) noexcept -> bool {
    return StringView(lhs) >= rhs;
}

inline auto operator==(StringView lhs, const String &rhs) noexcept -> bool {
    return lhs == StringView(rhs);
}

inline auto operator!=(StringView lhs, const String &rhs) noexcept -> bool {
    return lhs != StringView(rhs);
}

inline auto operator<(StringView lhs, const String &rhs) noexcept -> bool {
    return lhs < StringView(rhs);
}

inline auto operator<=(StringView lhs, const String &rhs) noexcept -> bool {
    return lhs <= StringView(rhs);
}

inline auto operator>(StringView lhs, const String &rhs) noexcept -> bool {
    return lhs > StringView(rhs);
}

inline auto operator>=(StringView lhs, const String &rhs) noexcept -> bool {
    return lhs >= StringView(rhs);
}

inline auto operator==(const String &lhs, const char16_t *rhs) noexcept -> bool {
    return StringView(lhs) == StringView(rhs);
}

inline auto operator!=(const String &lhs, const char16_t *rhs) noexcept -> bool {
    return StringView(lhs) != StringView(rhs);
}

inline auto operator<(const String &lhs, const char16_t *rhs) noexcept -> bool {
    return StringView(lhs) < StringView(rhs);
}

inline auto operator<=(const String &lhs, const char16_t *rhs) noexcept -> bool {
    return StringView(lhs) <= StringView(rhs);
}

inline auto operator>(const String &lhs, const char16_t *rhs) noexcept -> bool {
    return StringView(lhs) > StringView(rhs);
}

inline auto operator>=(const String &lhs, const char16_t *rhs) noexcept -> bool {
    return StringView(lhs) >= StringView(rhs);
}

inline auto operator==(const char16_t *lhs, const String &rhs) noexcept -> bool {
    return StringView(lhs) == StringView(rhs);
}

inline auto operator!=(const char16_t *lhs, const String &rhs) noexcept -> bool {
    return StringView(lhs) != StringView(rhs);
}

inline auto operator<(const char16_t *lhs, const String &rhs) noexcept -> bool {
    return StringView(lhs) < StringView(rhs);
}

inline auto operator<=(const char16_t *lhs, const String &rhs) noexcept -> bool {
    return StringView(lhs) <= StringView(rhs);
}

inline auto operator>(const char16_t *lhs, const String &rhs) noexcept -> bool {
    return StringView(lhs) > StringView(rhs);
}

inline auto operator>=(const char16_t *lhs, const String &rhs) noexcept -> bool {
    return StringView(lhs) >= StringView(rhs);
}

InkApi auto operator+(const String &lhs, const String &rhs) noexcept -> String;
InkApi auto operator+(const String &lhs, StringView rhs) noexcept -> String;
InkApi auto operator+(StringView lhs, const String &rhs) noexcept -> String;
InkApi auto operator+(const String &lhs, const char16_t *rhs) noexcept -> String;
InkApi auto operator+(const char16_t *lhs, const String &rhs) noexcept -> String;

/// @brief
///   Format pattern string with compile-time argument check.
///
/// @tparam ...Args
///   Types of arguments to be formatted.
template <typename... Args>
using FormatString = fmt::basic_format_string<char16_t, fmt::type_identity_t<Args>...>;

/// @brief
///   Memory buffer for formatting string.
using FormatMemoryBuffer = fmt::basic_memory_buffer<char16_t>;

/// @brief
///   Format buffer context.
using FormatContext = fmt::buffer_context<char16_t>;

/// @brief
///   Format a string and write to the specified output iterator.
///
/// @tparam OutputIt
///   Type of the output iterator to be written to.
/// @tparam ...T
///   Types of arguments to be formatted.
///
/// @param out
///   The output iterator to be written to.
/// @param format
///   The format pattern string.
/// @param ...args
///   Arguments to be formatted.
///
/// @return
///   The new output iterator after formatting.
template <typename OutputIt, typename... T>
auto formatTo(OutputIt out, FormatString<T...> format, T &&...args) -> OutputIt {
    return fmt::format_to(out, fmt::basic_string_view<char16_t>(format), std::forward<T>(args)...);
}

/// @brief
///   Format a string with arguments.
///
/// @tparam ...Args
///   Types of arguments to be formatted.
///
/// @param pattern
///   The format pattern string.
/// @param ...args
///   Arguments to be formatted.
///
/// @return
///   The formatted string.
template <typename... Args>
auto format(FormatString<Args...> pattern, Args &&...args) -> String {
    FormatMemoryBuffer buffer;
    fmt::format_to(std::back_inserter(buffer), fmt::basic_string_view<char16_t>(pattern),
                   std::forward<Args>(args)...);
    return String(buffer.data(), buffer.size());
}

} // namespace ink

template <>
struct std::hash<ink::String> {
    auto operator()(const ink::String &str) const noexcept -> std::size_t {
        return str.hash();
    }
};

template <>
struct fmt::formatter<ink::String, char16_t> : fmt::formatter<ink::StringView, char16_t> {
    template <typename FormatContext>
    auto format(const ink::String &str, FormatContext &ctx) const -> decltype(ctx.out()) {
        using Super = fmt::formatter<ink::StringView, char16_t>;
        return this->Super::format(ink::StringView(str), ctx);
    }
};
