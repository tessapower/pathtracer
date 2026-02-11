#include "stdafx.h"

#include "core/dx12_info_queue.h"

#include <d3d12.h>
#include <d3d12sdklayers.h>

namespace pathtracer
{
DX12InfoQueue::DX12InfoQueue(ID3D12Device* device) {
    // Try to get the ID3D12InfoQueue interface from the device. This interface
    // is only available if the debug layer is enabled, so it may fail in
    // release builds.
    device->QueryInterface(IID_PPV_ARGS(&m_infoQueue));

    if (m_infoQueue)
    {
        m_msgIdx = m_infoQueue->GetNumStoredMessages();
    }
}

auto DX12InfoQueue::Mark() -> void {
    if (m_infoQueue)
    {
        m_msgIdx = m_infoQueue->GetNumStoredMessages();
    }
}

auto DX12InfoQueue::GetMessages() -> std::vector<std::string>
{
    std::vector<std::string> Msgs;

    if (!m_infoQueue) // Not available in Release build
    {
        return Msgs;
    }

    UINT64 nMsgs = m_infoQueue->GetNumStoredMessages();

    for (auto i = m_msgIdx; i < nMsgs; ++i)
    {
        // Get message length
        SIZE_T msgLenBytes = 0;
        m_infoQueue->GetMessage(i, nullptr, &msgLenBytes);

        if (msgLenBytes > 0)
        {
            // Allocate buffer for message
            std::vector<BYTE> msgData(msgLenBytes);
            auto msg = reinterpret_cast<D3D12_MESSAGE*>(msgData.data());

            // Get the message
            m_infoQueue->GetMessage(i, msg, &msgLenBytes);
            // Convert to string and add to list
            Msgs.emplace_back(msg->pDescription);
        }
    }

    // Update index to current end of queue
    m_msgIdx = nMsgs;

    return Msgs;
}

auto DX12InfoQueue::IsAvailable() -> bool
{
    return m_infoQueue != nullptr;
}

} // namespace pathtracer
