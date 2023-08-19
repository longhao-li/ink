#pragma once

#include <type_traits>

namespace ink {

template <typename T, typename = std::enable_if_t<std::is_floating_point<T>::value, T>>
inline constexpr T E = static_cast<T>(2.71828182845904523536);

template <typename T, typename = std::enable_if_t<std::is_floating_point<T>::value, T>>
inline constexpr T Sqrt2 = static_cast<T>(1.41421356237309504880);

template <typename T, typename = std::enable_if_t<std::is_floating_point<T>::value, T>>
inline constexpr T Sqrt3 = static_cast<T>(1.73205080756887729352);

template <typename T, typename = std::enable_if_t<std::is_floating_point<T>::value, T>>
inline constexpr T Log2E = static_cast<T>(1.44269504088896340736);

template <typename T, typename = std::enable_if_t<std::is_floating_point<T>::value, T>>
inline constexpr T Log10E = static_cast<T>(0.434294481903251827651);

template <typename T, typename = std::enable_if_t<std::is_floating_point<T>::value, T>>
inline constexpr T Ln2 = static_cast<T>(0.693147180559945309417);

template <typename T, typename = std::enable_if_t<std::is_floating_point<T>::value, T>>
inline constexpr T Ln10 = static_cast<T>(2.30258509299404568402);

template <typename T, typename = std::enable_if_t<std::is_floating_point<T>::value, T>>
inline constexpr T Pi = static_cast<T>(3.14159265358979323846);

template <typename T, typename = std::enable_if_t<std::is_floating_point<T>::value, T>>
inline constexpr T InvPi = static_cast<T>(0.318309886183790671538);

} // namespace ink
