#include "model.hpp"
#include "ink/core/exception.hpp"
#include "ink/render/device.hpp"

#include <tiny_gltf.h>

#include <cassert>

using namespace ink;

namespace {

[[nodiscard]] auto startsWith(std::string_view str, std::string_view match) noexcept -> bool {
    if (match.size() > str.size())
        return false;
    return !std::string_view::traits_type::compare(str.data(), match.data(), match.size());
}

} // namespace

ink::Model::Model(RenderDevice &renderDevice, std::string_view path, bool isBinary)
    : m_buffers(), m_textures(), m_materials(), m_nodes() {
    tinygltf::TinyGLTF gltfLoader;
    tinygltf::Model    gltfModel;

    { // Load GLTF file.
        std::string error;
        bool        succeeded = false;

        if (isBinary)
            succeeded =
                gltfLoader.LoadBinaryFromFile(&gltfModel, &error, nullptr, std::string(path));
        else
            succeeded =
                gltfLoader.LoadASCIIFromFile(&gltfModel, &error, nullptr, std::string(path));

        if (!succeeded)
            throw Exception("Failed to load GLTF model: " + error);
    }

    CommandBuffer cmdBuffer{renderDevice.newCommandBuffer()};

    // Upload buffers.
    for (const auto &buffer : gltfModel.buffers) {
        auto &newBuffer = m_buffers.emplace_back(renderDevice.newGpuBuffer(buffer.data.size()));
        cmdBuffer.copyBuffer(buffer.data.data(), newBuffer, 0, buffer.data.size());
        cmdBuffer.transition(newBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
    }

    // Upload textures.
    for (const auto &image : gltfModel.images) {
        const std::uint32_t width  = image.width;
        const std::uint32_t height = image.height;
        const std::byte    *data   = nullptr;

        std::vector<std::byte> imageData;
        if (image.component == 4) {
            data = reinterpret_cast<const std::byte *>(image.image.data());
        } else {
            assert(image.component == 3);
            imageData.resize(width * height * 4);
            std::byte  *dst    = imageData.data();
            const auto *src    = image.image.data();
            const auto *srcEnd = src + image.image.size();
            while (src != srcEnd) {
                dst[0] = static_cast<std::byte>(src[0]);
                dst[1] = static_cast<std::byte>(src[1]);
                dst[2] = static_cast<std::byte>(src[2]);
                dst[3] = std::byte{0xFF};

                dst += 4;
                src += 3;
            }

            data = imageData.data();
        }

        auto &texture = m_textures.emplace_back(
            renderDevice.new2DTexture(width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
        cmdBuffer.copyTexture(data, DXGI_FORMAT_R8G8B8A8_UNORM, width * 4, width, height, texture,
                              0);
        cmdBuffer.transition(texture, D3D12_RESOURCE_STATE_GENERIC_READ);
    }

    cmdBuffer.submit();

    // Upload materials.
    for (const auto &material : gltfModel.materials) {
        const auto &pbr = material.pbrMetallicRoughness;

        Material mat{};
        if (material.alphaMode == "OPAQUE")
            mat.alphaMode = AlphaMode::Opaque;
        else if (material.alphaMode == "MASK")
            mat.alphaMode = AlphaMode::Mask;
        else if (material.alphaMode == "BLEND")
            mat.alphaMode = AlphaMode::Blend;

        mat.alphaCutoff      = static_cast<float>(material.alphaCutoff);
        mat.emissiveFactor.x = static_cast<float>(material.emissiveFactor[0]);
        mat.emissiveFactor.y = static_cast<float>(material.emissiveFactor[1]);
        mat.emissiveFactor.z = static_cast<float>(material.emissiveFactor[2]);
        mat.metallic         = static_cast<float>(pbr.metallicFactor);
        mat.roughness        = static_cast<float>(pbr.roughnessFactor);
        mat.baseColor.x      = static_cast<float>(pbr.baseColorFactor[0]);
        mat.baseColor.y      = static_cast<float>(pbr.baseColorFactor[1]);
        mat.baseColor.z      = static_cast<float>(pbr.baseColorFactor[2]);
        mat.baseColor.w      = static_cast<float>(pbr.baseColorFactor[3]);

        if (pbr.baseColorTexture.index != -1)
            mat.baseColorTexture = &m_textures[pbr.baseColorTexture.index];
        if (pbr.metallicRoughnessTexture.index != -1)
            mat.metallicRoughnessTexture = &m_textures[pbr.metallicRoughnessTexture.index];
        if (material.normalTexture.index != -1)
            mat.normalTexture = &m_textures[material.normalTexture.index];
        if (material.occlusionTexture.index != -1)
            mat.occlusionTexture = &m_textures[material.occlusionTexture.index];
        if (material.emissiveTexture.index != -1)
            mat.emissiveTexture = &m_textures[material.emissiveTexture.index];

        m_materials.push_back(mat);
    }

    { // Load nodes via BFS.
        const auto &scene =
            gltfModel.scenes[gltfModel.defaultScene == -1 ? 0 : gltfModel.defaultScene];
        std::queue<std::pair<const tinygltf::Node *, Node *>> nodeQueue;

        auto convertNode = [this, &gltfModel](const tinygltf::Node &gltfNode) -> Node {
            Node node{
                /* parent      = */ nullptr,
                /* children    = */ {},
                /* mesh        = */ {},
                /* transform   = */ Matrix4{1.0f},
                /* translation = */ {},
                /* scale       = */ Vector3{1.0f},
                /* rotation    = */ Quaternion{1.0f},
            };

            if (gltfNode.rotation.size() == 4) {
                node.rotation.w = static_cast<float>(gltfNode.rotation[0]);
                node.rotation.x = static_cast<float>(gltfNode.rotation[1]);
                node.rotation.y = static_cast<float>(gltfNode.rotation[2]);
                node.rotation.z = static_cast<float>(gltfNode.rotation[3]);
            }

            if (gltfNode.scale.size() == 3) {
                node.scale.x = static_cast<float>(gltfNode.scale[0]);
                node.scale.y = static_cast<float>(gltfNode.scale[1]);
                node.scale.z = static_cast<float>(gltfNode.scale[2]);
            }

            if (gltfNode.translation.size() == 3) {
                node.translation.x = static_cast<float>(gltfNode.translation[0]);
                node.translation.y = static_cast<float>(gltfNode.translation[1]);
                node.translation.z = static_cast<float>(gltfNode.translation[2]);
            }

            if (gltfNode.matrix.size() == 16) {
                node.transform[0][0] = static_cast<float>(gltfNode.matrix[0]);
                node.transform[0][1] = static_cast<float>(gltfNode.matrix[1]);
                node.transform[0][2] = static_cast<float>(gltfNode.matrix[2]);
                node.transform[0][3] = static_cast<float>(gltfNode.matrix[3]);
                node.transform[1][0] = static_cast<float>(gltfNode.matrix[4]);
                node.transform[1][1] = static_cast<float>(gltfNode.matrix[5]);
                node.transform[1][2] = static_cast<float>(gltfNode.matrix[6]);
                node.transform[1][3] = static_cast<float>(gltfNode.matrix[7]);
                node.transform[2][0] = static_cast<float>(gltfNode.matrix[8]);
                node.transform[2][1] = static_cast<float>(gltfNode.matrix[9]);
                node.transform[2][2] = static_cast<float>(gltfNode.matrix[10]);
                node.transform[2][3] = static_cast<float>(gltfNode.matrix[11]);
                node.transform[3][0] = static_cast<float>(gltfNode.matrix[12]);
                node.transform[3][1] = static_cast<float>(gltfNode.matrix[13]);
                node.transform[3][2] = static_cast<float>(gltfNode.matrix[14]);
                node.transform[3][3] = static_cast<float>(gltfNode.matrix[15]);
            }

            if (gltfNode.mesh == -1)
                return node;

            const auto &gltfMesh = gltfModel.meshes[gltfNode.mesh];
            for (const auto &primitive : gltfMesh.primitives) {
                Mesh mesh{};

                { // Get indices.
                    const auto &accessor    = gltfModel.accessors[primitive.indices];
                    const auto &bufferView  = gltfModel.bufferViews[accessor.bufferView];
                    const auto  bufferID    = static_cast<std::size_t>(bufferView.buffer);
                    const auto  indexCount  = static_cast<std::uint32_t>(accessor.count);
                    const auto  indexOffset = accessor.byteOffset + bufferView.byteOffset;

                    mesh.index.buffer = m_buffers[bufferID].gpuAddress() + indexOffset;
                    mesh.index.count  = indexCount;

                    switch (accessor.componentType) {
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
                        mesh.index.stride = 4;
                        break;
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
                        mesh.index.stride = 2;
                        break;
                    default:
                        throw Exception("Unsupported index type.");
                    }
                }

                const auto &attributes = primitive.attributes;
                for (const auto &[key, index] : attributes) {
                    const auto &accessor   = gltfModel.accessors[index];
                    const auto &bufferView = gltfModel.bufferViews[accessor.bufferView];

                    const auto bufferID     = static_cast<std::size_t>(bufferView.buffer);
                    const auto bufferOffset = accessor.byteOffset + bufferView.byteOffset;
                    const auto count        = static_cast<std::uint32_t>(accessor.count);
                    const auto stride = static_cast<std::uint32_t>(accessor.ByteStride(bufferView));

                    if (key == "POSITION") {
                        mesh.position.buffer = m_buffers[bufferID].gpuAddress() + bufferOffset;
                        mesh.position.count  = count;
                        mesh.position.stride = stride;
                    } else if (key == "NORMAL") {
                        mesh.normal.buffer = m_buffers[bufferID].gpuAddress() + bufferOffset;
                        mesh.normal.count  = count;
                        mesh.normal.stride = stride;
                    } else if (key == "TANGENT") {
                        mesh.tangent.buffer = m_buffers[bufferID].gpuAddress() + bufferOffset;
                        mesh.tangent.count  = count;
                        mesh.tangent.stride = stride;
                    } else if (startsWith(key, "TEXCOORD_")) {
                        const auto i = std::stoi(key.substr(9));
                        assert(i < 16);
                        mesh.texCoord[i].buffer = m_buffers[bufferID].gpuAddress() + bufferOffset;
                        mesh.texCoord[i].count  = count;
                        mesh.texCoord[i].stride = stride;
                    } else if (startsWith(key, "COLOR_")) {
                        const auto i = std::stoi(key.substr(6));
                        assert(i < 16);
                        mesh.color[i].buffer = m_buffers[bufferID].gpuAddress() + bufferOffset;
                        mesh.color[i].count  = count;
                        mesh.color[i].stride = stride;
                    } else if (startsWith(key, "JOINTS_")) {
                        const auto i = std::stoi(key.substr(7));
                        assert(i < 16);
                        mesh.joint[i].buffer = m_buffers[bufferID].gpuAddress() + bufferOffset;
                        mesh.joint[i].count  = count;
                        mesh.joint[i].stride = stride;
                    } else if (startsWith(key, "WEIGHTS_")) {
                        const auto i = std::stoi(key.substr(8));
                        assert(i < 16);
                        mesh.weight[i].buffer = m_buffers[bufferID].gpuAddress() + bufferOffset;
                        mesh.weight[i].count  = count;
                        mesh.weight[i].stride = stride;
                    }
                }

                if (primitive.material != -1)
                    mesh.material = &m_materials[primitive.material];

                node.meshes.push_back(mesh);
            }

            return node;
        };

        for (auto i : scene.nodes) {
            const auto &gltfNode = gltfModel.nodes[i];
            nodeQueue.emplace(&gltfNode, &m_nodes.emplace_back(convertNode(gltfNode)));
        }

        while (!nodeQueue.empty()) {
            auto [gltfNode, node] = nodeQueue.front();
            nodeQueue.pop();

            for (auto i : gltfNode->children) {
                const auto &gltfChild = gltfModel.nodes[i];
                auto        child     = &node->children.emplace_back(convertNode(gltfChild));
                child->parent         = node;
                nodeQueue.emplace(&gltfChild, child);
            }
        }
    }
}

ink::Model::~Model() noexcept = default;
