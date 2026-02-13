#pragma once

#include "core/dx12_info_queue.h"

#include <sstream>
#include <stdexcept>

// TODO: introduce custom pathtracer error type to replace std::runtime_error

#define DX12_CHECK(hr_call)                                                    \
    do                                                                         \
    {                                                                          \
        HRESULT hr = (hr_call);                                                \
        if (FAILED(hr))                                                        \
        {                                                                      \
            std::ostringstream oss;                                            \
            oss << "DX12 Error in " << __FILE__ << ":" << __LINE__             \
                << "\nFunction: " << #hr_call << "\nHRESULT: 0x" << std::hex   \
                << hr;                                                         \
            throw std::runtime_error(oss.str());                               \
        }                                                                      \
    } while (0)

#define DX12_CHECK_MSG(hr_call, info_queue)                                    \
    do                                                                         \
    {                                                                          \
        (info_queue).Mark();                                                   \
        HRESULT hr = (hr_call);                                                \
        if (FAILED(hr))                                                        \
        {                                                                      \
            std::ostringstream oss;                                            \
            oss << "DX12 Error in " << __FILE__ << ":" << __LINE__             \
                << "\nFunction: " << #hr_call << "\nHRESULT: 0x" << std::hex   \
                << hr << std::dec;                                             \
            auto msgs = (info_queue).GetMessages();                            \
            if (!msgs.empty())                                                 \
            {                                                                  \
                oss << "\n\nDebug Layer Messages:\n";                          \
                for (const auto& msg : msgs)                                   \
                {                                                              \
                    oss << "  - " << msg << "\n";                              \
                }                                                              \
            }                                                                  \
            throw std::runtime_error(oss.str());                               \
        }                                                                      \
    } while (0)
