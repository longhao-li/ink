#pragma once

#include "ink/math/quaternion.hpp"
#include "ink/render/resource.hpp"

#include <list>
#include <memory>
#include <optional>
#include <queue>
#include <string_view>
#include <vector>

namespace ink {

enum class AlphaMode {
    Opaque,
    Mask,
    Blend,
};

struct Material {
    AlphaMode  alphaMode;
    float      alphaCutoff;
    Vector3    emissiveFactor;
    float      metallic;
    float      roughness;
    Vector4    baseColor;
    Texture2D *baseColorTexture;
    Texture2D *metallicRoughnessTexture;
    Texture2D *normalTexture;
    Texture2D *occlusionTexture;
    Texture2D *emissiveTexture;
};

struct Mesh {
    // Index data.
    std::uint64_t indexBuffer; // Index buffer GPU address.
    std::uint32_t indexCount;  // Number of indices.
    std::uint32_t indexStride; // Size of each index. Must be 2 or 4.

    // Vertex data.
    std::uint64_t positionBuffer; // Position buffer GPU address.
    std::uint32_t positionCount;  // Number of positions.
    std::uint32_t positionStride; // Position buffer stride.
    std::uint64_t normalBuffer;   // Normal buffer GPU address.
    std::uint32_t normalCount;    // Number of normals. Must be 0 or equal to positionCount.
    std::uint32_t normalStride;   // Normal buffer stride.
    std::uint64_t texCoordBuffer; // Texture coordinate buffer GPU address.
    std::uint32_t texCoordCount;  // Number of texture coordinates.
    std::uint32_t texCoordStride; // Texture coordinate buffer stride.
    std::uint64_t tangentBuffer;  // Tangent buffer GPU address.
    std::uint32_t tangentCount;   // Number of tangents.
    std::uint32_t tangentStride;  // Tangent buffer stride.
    std::uint64_t colorBuffer;    // Color buffer GPU address.
    std::uint32_t colorCount;     // Number of colors.
    std::uint32_t colorStride;    // Color buffer stride.
    std::uint64_t jointBuffer;    // Joint buffer GPU address.
    std::uint32_t jointCount;     // Number of joints.
    std::uint32_t jointStride;    // Joint buffer stride.
    std::uint64_t weightBuffer;   // Weight buffer GPU address.
    std::uint32_t weightCount;    // Number of weights.
    std::uint32_t weightStride;   // Weight buffer stride.
    Material     *material;       // Material of this mesh.
};

class Model {
private:
    struct Node {
        Node             *parent;
        std::list<Node>   children;
        std::vector<Mesh> meshes;
        Matrix4           transform;
        Vector3           translation;
        Vector3           scale;
        Quaternion        rotation;
    };

public:
    /// @brief
    ///   Load a GLTF model from a file.
    ///
    /// @param renderDevice
    ///   The render device that is used to create GPU buffers.
    /// @param path
    ///   Path to the GLTF file.
    /// @param isBinary
    ///   Whether the GLTF file is binary.
    ///
    /// @throw Exception
    ///   Thrown if failed to load GLTF model.
    /// @throw RenderAPIException
    ///   Thrown if failed to create GPU buffers.
    Model(RenderDevice &renderDevice, std::string_view path, bool isBinary);

    /// @brief
    ///   Destroy this model and release all resources.
    ~Model() noexcept;

    /// @brief
    ///   Render this model with the specified functor. The functor will be called for multi-times.
    ///
    /// @tparam Func
    ///   The type of the functor. Should accept two parameters: const Mesh &mesh and const Matrix4
    ///   &parent.
    ///
    /// @param func
    ///   The functor that is used to render the model.
    template <typename Func,
              typename = std::enable_if_t<
                  std::is_invocable_r_v<void, Func, const Mesh &, const Matrix4 &>>>
    auto render(Func &&func) const -> void {
        std::queue<std::pair<const Node *, Matrix4>> bfsQueue;
        for (const auto &node : m_nodes)
            bfsQueue.emplace(&node, node.transform.scaled(node.scale)
                                        .rotated(node.rotation)
                                        .translated(node.translation));

        while (!bfsQueue.empty()) {
            auto [node, transform] = bfsQueue.front();
            bfsQueue.pop();

            transform *= node->transform.scaled(node->scale)
                             .rotated(node->rotation)
                             .translated(node->translation);

            for (const auto &submesh : node->meshes)
                func(submesh, transform);

            for (const auto &child : node->children)
                bfsQueue.emplace(&child, transform);
        }
    }

private:
    std::vector<GpuBuffer> m_buffers;
    std::vector<Texture2D> m_textures;
    std::vector<Material>  m_materials;
    std::list<Node>        m_nodes;
};

} // namespace ink
