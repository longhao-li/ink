#pragma once

#include "ink/render/descriptor.h"

#include <vector>

namespace ink {

class RenderDevice;
class RootSignature;

class DynamicDescriptorHeap {
private:
    enum class CacheType {
        None = 0,
        CpuDescriptor,
        ConstantBufferView,
        ShaderResourceView,
        UnorderedAccessView,
    };

    struct DescriptorCache {
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
        CacheType cacheType;
        union {
            D3D12_CPU_DESCRIPTOR_HANDLE     handle;
            D3D12_CONSTANT_BUFFER_VIEW_DESC constantBuffer;
        };
#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#    pragma warning(pop)
#endif
    };

    struct DescriptorTableRange {
        std::uint16_t start;
        std::uint16_t count;
    };

public:
    /// @brief
    ///   Create an empty dynamic descriptor heap.
    InkApi DynamicDescriptorHeap() noexcept;

    /// @brief
    ///   Create a dynamic descriptor heap for the specified type of descriptors.
    ///
    /// @param descriptorType
    ///   Type of this dynamic descriptor type.
    InkApi explicit DynamicDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE descriptorType) noexcept;

    /// @brief
    ///   Copy constructor of dynamic descriptor heap is disabled.
    DynamicDescriptorHeap(const DynamicDescriptorHeap &) = delete;

    /// @brief
    ///   Copy assignment of dynamic descriptor heap is disabled.
    auto operator=(const DynamicDescriptorHeap &) = delete;

    /// @brief
    ///   Move constructor of dynamic descriptor heap.
    ///
    /// @param other
    ///   The dynamic descriptor heap to be moved from. The moved dynamic descriptor heap will be
    ///   invalidated.
    InkApi DynamicDescriptorHeap(DynamicDescriptorHeap &&other) noexcept;

    /// @brief
    ///   Copy assignment of dynamic descriptor heap is disabled.
    auto operator=(DynamicDescriptorHeap &&) = delete;

    /// @brief
    ///   Destroy this dynamic descriptor heap.
    InkApi ~DynamicDescriptorHeap() noexcept;

    /// @brief
    ///   Clean up all descriptors and root signatures.
    ///
    /// @param fenceValue
    ///   The fence value that indicates when the allocated descriptor heaps could be reused.
    InkApi auto reset(std::uint64_t fenceValue) noexcept -> void;

    /// @brief
    ///   Parse the specified graphics root signature and prepare shader-visible descriptors for it.
    ///
    /// @param rootSig
    ///   The root signature to be parsed.
    InkApi auto parseGraphicsRootSignature(const RootSignature &rootSig) noexcept -> void;

    /// @brief
    ///   Parse the specified compute root signature and prepare shader-visible descriptors for it.
    ///
    /// @param rootSig
    ///   The root signature to be parsed.
    InkApi auto parseComputeRootSignature(const RootSignature &rootSig) noexcept -> void;

    /// @brief
    ///   Bind the specified CPU descriptor handle to the specified root descriptor table.
    ///
    /// @param paramIndex
    ///   Root parameter index of the descriptor table. This method does not check whether the
    ///   specified root parameter is a valid descriptor table.
    /// @param offset
    ///   Offset from start of the descriptor table to set the descriptor handle. This method does
    ///   not check whether the offset is out of range.
    /// @param descriptor
    ///   The CPU descriptor to be bind.
    InkApi auto bindGraphicsDescriptor(std::uint32_t       paramIndex,
                                       std::uint32_t       offset,
                                       CpuDescriptorHandle descriptor) noexcept -> void;

    /// @brief
    ///   Bind a constant buffer view to the specified root descriptor table.
    ///
    /// @param paramIndex
    ///   Root parameter index of the descriptor table. This method does not check whether the
    ///   specified root parameter is a valid descriptor table.
    /// @param offset
    ///   Offset from start of the descriptor table to set the descriptor handle. This method does
    ///   not check whether the offset is out of range.
    /// @param desc
    ///   A description structure that describes how to create the constant buffer view.
    InkApi auto bindGraphicsDescriptor(std::uint32_t                          paramIndex,
                                       std::uint32_t                          offset,
                                       const D3D12_CONSTANT_BUFFER_VIEW_DESC &desc) noexcept
        -> void;

    /// @brief
    ///   Bind the specified CPU descriptor handle to the specified root descriptor table.
    ///
    /// @param paramIndex
    ///   Root parameter index of the descriptor table. This method does not check whether the
    ///   specified root parameter is a valid descriptor table.
    /// @param offset
    ///   Offset from start of the descriptor table to set the descriptor handle. This method does
    ///   not check whether the offset is out of range.
    /// @param descriptor
    ///   The CPU descriptor to be bind.
    InkApi auto bindComputeDescriptor(std::uint32_t       paramIndex,
                                      std::uint32_t       offset,
                                      CpuDescriptorHandle descriptor) noexcept -> void;

    /// @brief
    ///   Bind a constant buffer view to the specified root descriptor table.
    ///
    /// @param paramIndex
    ///   Root parameter index of the descriptor table. This method does not check whether the
    ///   specified root parameter is a valid descriptor table.
    /// @param offset
    ///   Offset from start of the descriptor table to set the descriptor handle. This method does
    ///   not check whether the offset is out of range.
    /// @param desc
    ///   A description structure that describes how to create the constant buffer view.
    InkApi auto bindComputeDescriptor(std::uint32_t                          paramIndex,
                                      std::uint32_t                          offset,
                                      const D3D12_CONSTANT_BUFFER_VIEW_DESC &desc) noexcept -> void;

    /// @brief
    ///   Upload all descriptors in the cached descriptor tables to shader-visible descriptor heaps
    ///   and bind the specified descriptor heap to current command list.
    ///
    /// @param cmdList
    ///   The command list that is used to bind the specified descriptor heap.
    InkApi auto submitGraphicsDescriptors(ID3D12GraphicsCommandList *cmdList) noexcept -> void;

    /// @brief
    ///   Upload all descriptors in the cached descriptor tables to shader-visible descriptor heaps
    ///   and bind the specified descriptor heap to current command list.
    ///
    /// @param cmdList
    ///   The command list that is used to bind the specified descriptor heap.
    InkApi auto submitComputeDescriptors(ID3D12GraphicsCommandList *cmdList) noexcept -> void;

private:
    /// @brief
    ///   The render device that is used to allocate and free dynamic descriptor heaps.
    RenderDevice *m_renderDevice;

    /// @brief
    ///   The D3D12 device that is used to create and copy descriptors.
    ID3D12Device5 *m_device;

    /// @brief
    ///   Descriptor type of this dynamic descriptor heap.
    D3D12_DESCRIPTOR_HEAP_TYPE m_heapType;

    /// @brief
    ///   Descriptor handle increment size.
    std::uint32_t m_descriptorSize;

    /// @brief
    ///   Current graphics root signature of this dynamic descriptor heap.
    const RootSignature *m_graphicsRootSignature;

    /// @brief
    ///   Current compute root signature of this dynamic descriptor heap.
    const RootSignature *m_computeRootSignature;

    /// @brief
    ///   Current shader-visible descriptor heap.
    ID3D12DescriptorHeap *m_currentHeap;

    /// @brief
    ///   Current shader-visible descriptor handle.
    DescriptorHandle m_currentHandle;

    /// @brief
    ///   Number of free descriptors in current shader-visible descriptor heap.
    std::uint32_t m_freeDescriptorCount;

    /// @brief
    ///   Retired descriptor heaps.
    std::vector<ID3D12DescriptorHeap *> m_retiredHeaps;

    /// @brief
    ///   Cached graphics descriptors.
    std::vector<DescriptorCache> m_graphicsDescriptors;

    /// @brief
    ///   Cached compute descriptors.
    std::vector<DescriptorCache> m_computeDescriptors;

    /// @brief
    ///   Cached descriptor table ranges for graphics root signature.
    DescriptorTableRange m_graphicsTableRange[64];

    /// @brief
    ///   Cached descriptor table ranges for compute root signature.
    DescriptorTableRange m_computeTableRange[64];
};

} // namespace ink
