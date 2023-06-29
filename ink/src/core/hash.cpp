#include "ink/core/hash.h"

#include <cstdint>
#include <cstring>

using namespace ink;

#if !defined(_WIN64)
auto ink::hash(const void *data, std::size_t size, std::size_t seed) noexcept -> std::size_t {
    std::uint32_t h32; // No need to initialize.

    if (size >= 16) {
        const auto *const bEnd  = static_cast<const std::uint8_t *>(data) + size;
        const auto *const limit = bEnd - 15;

        std::uint32_t v[4] = {
            seed + 0x9E3779B1U + 0x85EBCA77U,
            seed + 0x85EBCA77U,
            seed + 0,
            seed - 0x9E3779B1U,
        };

        do {
            std::uint32_t in[4];
            std::memcpy(&in, data, sizeof(in)); // Avoid alignment problem.

            v[0] += in[0] * 0x85EBCA77U;
            v[1] += in[1] * 0x85EBCA77U;
            v[2] += in[2] * 0x85EBCA77U;
            v[3] += in[3] * 0x85EBCA77U;

            v[0] = ((v[0] << 13) | (v[0] >> 19));
            v[1] = ((v[1] << 13) | (v[1] >> 19));
            v[2] = ((v[2] << 13) | (v[2] >> 19));
            v[3] = ((v[3] << 13) | (v[3] >> 19));

            v[0] *= 0x9E3779B1U;
            v[1] *= 0x9E3779B1U;
            v[2] *= 0x9E3779B1U;
            v[3] *= 0x9E3779B1U;

            data = static_cast<const std::uint8_t *>(data) + 16;
        } while (data < limit);

        h32 = ((v[0] << 1) | (v[0] >> (32 - 1))) + ((v[1] << 7) | (v[1] >> (32 - 7))) +
              ((v[2] << 12) | (v[2] >> (32 - 12))) + ((v[3] << 18) | (v[3] >> (32 - 18)));
    } else {
        h32 = seed + 0x165667B1U;
    }

    h32 += static_cast<std::uint32_t>(size);
    size &= 15;

    while (size >= 4) {
        std::uint32_t temp;
        std::memcpy(&temp, data, sizeof(temp));

        h32 += temp * 0xC2B2AE3DU;
        data = static_cast<const std::uint8_t *>(data) + 4;
        h32  = ((h32 << 17) | (h32 >> (32 - 17))) * 0x27D4EB2FU;
        size -= 4;
    }

    while (size > 0) {
        h32 += *static_cast<const std::uint8_t *>(data) * 0x165667B1U;
        data = static_cast<const std::uint8_t *>(data) + 1;
        h32  = ((h32 << 11) | (h32 >> (32 - 11))) * 0x9E3779B1U;
        --size;
    }

    h32 ^= h32 >> 15;
    h32 *= 0x85EBCA77U;
    h32 ^= h32 >> 13;
    h32 *= 0xC2B2AE3DU;
    h32 ^= h32 >> 16;

    return h32;
}
#else
auto ink::hash(const void *data, std::size_t size, std::size_t seed) noexcept -> std::size_t {
    std::uint64_t h64; // No need to initialize.

    if (size >= 32) {
        const auto *const bEnd  = static_cast<const std::uint8_t *>(data) + size;
        const auto *const limit = bEnd - 31;

        std::uint64_t v[4] = {
            seed + 0x9E3779B185EBCA87ULL + 0xC2B2AE3D27D4EB4FULL,
            seed + 0xC2B2AE3D27D4EB4FULL,
            seed + 0,
            seed - 0x9E3779B185EBCA87ULL,
        };

        do {
            std::uint64_t in[4];
            std::memcpy(&in, data, sizeof(in)); // Avoid alignment problem.

            v[0] += in[0] * 0xC2B2AE3D27D4EB4FULL;
            v[1] += in[1] * 0xC2B2AE3D27D4EB4FULL;
            v[2] += in[2] * 0xC2B2AE3D27D4EB4FULL;
            v[3] += in[3] * 0xC2B2AE3D27D4EB4FULL;

            v[0] = ((v[0] << 31) | (v[0] >> 33));
            v[1] = ((v[1] << 31) | (v[1] >> 33));
            v[2] = ((v[2] << 31) | (v[2] >> 33));
            v[3] = ((v[3] << 31) | (v[3] >> 33));

            v[0] *= 0x9E3779B185EBCA87ULL;
            v[1] *= 0x9E3779B185EBCA87ULL;
            v[2] *= 0x9E3779B185EBCA87ULL;
            v[3] *= 0x9E3779B185EBCA87ULL;

            data = static_cast<const std::uint8_t *>(data) + 32;
        } while (data < limit);

        h64 = ((v[0] << 1) | (v[0] >> (64 - 1))) + ((v[1] << 7) | (v[1] >> (64 - 7))) +
              ((v[2] << 12) | (v[2] >> (64 - 12))) + ((v[3] << 18) | (v[3] >> (64 - 18)));

        v[0] *= 0xC2B2AE3D27D4EB4FULL;
        v[0] = ((v[0] << 31) | (v[0] >> 33));
        v[0] *= 0x9E3779B185EBCA87ULL;

        h64 ^= v[0];
        h64 = h64 * 0x9E3779B185EBCA87ULL + 0x85EBCA77C2B2AE63ULL;

        v[1] *= 0xC2B2AE3D27D4EB4FULL;
        v[1] = ((v[1] << 31) | (v[1] >> 33));
        v[1] *= 0x9E3779B185EBCA87ULL;

        h64 ^= v[1];
        h64 = h64 * 0x9E3779B185EBCA87ULL + 0x85EBCA77C2B2AE63ULL;

        v[2] *= 0xC2B2AE3D27D4EB4FULL;
        v[2] = ((v[2] << 31) | (v[2] >> 33));
        v[2] *= 0x9E3779B185EBCA87ULL;

        h64 ^= v[2];
        h64 = h64 * 0x9E3779B185EBCA87ULL + 0x85EBCA77C2B2AE63ULL;

        v[3] *= 0xC2B2AE3D27D4EB4FULL;
        v[3] = ((v[3] << 31) | (v[3] >> 33));
        v[3] *= 0x9E3779B185EBCA87ULL;

        h64 ^= v[3];
        h64 = h64 * 0x9E3779B185EBCA87ULL + 0x85EBCA77C2B2AE63ULL;
    } else {
        h64 = seed + 0x27D4EB2F165667C5ULL;
    }

    h64 += size;
    size &= 31;

    while (size >= 8) {
        std::uint64_t temp;
        std::memcpy(&temp, data, sizeof(temp));

        temp = temp * 0xC2B2AE3D27D4EB4FULL;
        temp = ((temp << 31) | (temp >> 33));
        temp *= 0x9E3779B185EBCA87ULL;

        data = static_cast<const std::uint8_t *>(data) + 8;

        h64 ^= temp;
        h64 = ((h64 << 27) | (h64 >> 37)) * 0x9E3779B185EBCA87ULL + 0x85EBCA77C2B2AE63ULL;

        size -= 8;
    }

    if (size >= 4) {
        std::uint32_t temp;
        std::memcpy(&temp, data, sizeof(temp));

        h64 ^= std::uint64_t(temp) * 0x9E3779B185EBCA87ULL;
        data = static_cast<const std::uint8_t *>(data) + 4;
        h64  = ((h64 << 23) | (h64 >> 41)) * 0xC2B2AE3D27D4EB4FULL + 0x165667B19E3779F9ULL;
        size -= 4;
    }

    while (size > 0) {
        auto temp = *static_cast<const std::uint8_t *>(data);
        data      = static_cast<const std::uint8_t *>(data) + 1;

        h64 ^= std::uint64_t(temp) * 0x27D4EB2F165667C5ULL;
        h64 = ((h64 << 11) | (h64 >> 53)) * 0x9E3779B185EBCA87ULL;
        size -= 1;
    }

    h64 ^= h64 >> 33;
    h64 *= 0xC2B2AE3D27D4EB4FULL;
    h64 ^= h64 >> 29;
    h64 *= 0x165667B19E3779F9ULL;
    h64 ^= h64 >> 32;

    return h64;
}
#endif
