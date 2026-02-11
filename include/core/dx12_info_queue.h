#pragma once

#include <d3d12.h>

#include <string>
#include <vector>
#include <wrl.h>

namespace pathtracer
{
class DX12InfoQueue
{
  public:
    /// <summary>
    /// Initializes the DX12InfoQueue with the given device. If the device
    /// does not support the ID3D12InfoQueue interface, the info queue will be
    /// unavailable.
    /// </summary>
    explicit DX12InfoQueue(ID3D12Device* device);

    /// <summary>
    /// Marks the current position in the info queue. Subsequent calls to
    /// GetMessages() will retrieve messages added after this point.
    /// </summary>
    auto Mark() -> void;

    /// <summary>
    /// Retrieves messages from the info queue that were added after the last
    /// call to Mark().
    /// </summary>
    auto GetMessages() -> std::vector<std::string>;

    /// <summary>
    /// Returns whether the ID3D12InfoQueue interface is available on the
    /// current device.
    /// </summary>
    auto IsAvailable() -> bool;

  private:
    Microsoft::WRL::ComPtr<ID3D12InfoQueue> m_infoQueue;
    UINT64 m_msgIdx = 0;
};

} // namespace pathtracer
