#pragma once

#include <cstdint>

namespace ink {

struct Color {
#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#elif defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wpedantic"
#elif defined(_MSC_VER)
#    pragma warning(push)
#    pragma warning(disable : 4201)
#endif
    union {
        float m_arr[4];
        struct {
            float red;
            float green;
            float blue;
            float alpha;
        };
    };
#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#    pragma warning(pop)
#endif

    /// @brief
    ///   Create a transparent black color. All elements are initialized to 0.
    constexpr Color() noexcept : m_arr{} {}

    /// @brief
    ///   Create a color with the given color values.
    ///
    /// @param r
    ///   Red intensity of this color. For non-HDR images, this value should be less than or equal
    ///   to 1.
    /// @param g
    ///   Green intensity of this color. For non-HDR images, this value should be less than or equal
    ///   to 1.
    /// @param b
    ///   Blue intensity of this color. For non-HDR images, this value should be less than or equal
    ///   to 1.
    /// @param a
    ///   Opacity of this color.
    constexpr Color(float r, float g, float b, float a) noexcept : m_arr{r, g, b, a} {}
};

namespace colors {

inline constexpr Color Transparent{0.0f, 0.0f, 0.0f, 0.0f};
inline constexpr Color Black{0.0f, 0.0f, 0.0f, 1.0f};
inline constexpr Color White{1.0f, 1.0f, 1.0f, 1.0f};
inline constexpr Color Red{1.0f, 0.0f, 0.0f, 1.0f};
inline constexpr Color Green{0.0f, 1.0f, 0.0f, 1.0f};
inline constexpr Color Blue{0.0f, 0.0f, 1.0f, 1.0f};

} // namespace colors

} // namespace ink
