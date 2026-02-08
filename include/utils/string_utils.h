#pragma once

#include <string>

namespace pathtracer
{
namespace utils
{

/// <summary>
/// Convert a narrow string (char*) to a wide string (wchar_t*)
/// <para></para>
/// Simple ASCII-compatible conversion.
/// For international characters, consider using MultiByteToWideChar.
/// </summary>
/// <param name="str">The narrow string to convert</param>
/// <returns>Wide string equivalent</returns>
inline std::wstring StringToWide(const std::string& str)
{
    if (str.empty())
        return std::wstring();

    return std::wstring(str.begin(), str.end());
}

/// <summary>
/// Convert a narrow C-string (const char*) to a wide string
/// </summary>
/// <param name="str">The narrow C-string to convert</param>
/// <returns>Wide string equivalent</returns>
inline std::wstring StringToWide(const char* str)
{
    if (!str)
        return std::wstring();

    return StringToWide(std::string(str));
}

} // namespace utils
} // namespace pathtracer
