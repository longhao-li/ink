#pragma once

#include "ink/core/common.h"

#include <cstddef>

namespace ink {

/// @brief
///   Calculate hash value for the specified data.
///
/// @param data
///   Pointer to start of the data to be hashed.
/// @param size
///   Size in byte of data to be hashed.
/// @param seed
///   Seed to initialize the hasher. Default seed is 0.
///
/// @return
///   Hash value of the specified data.
[[nodiscard]]
InkApi auto hash(const void *data, std::size_t size, std::size_t seed = 0) noexcept -> std::size_t;

} // namespace ink
