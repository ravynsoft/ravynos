#ifndef TVISION_BASE64_H
#define TVISION_BASE64_H

#include <tvision/tv.h>
#include <string>

namespace tvision
{

// Returns the region of 'output' that contains the encoded data.
// Pre: the capacity of 'output' is no less than 4/3 of 'input.size()'.
TStringView encodeBase64(TStringView input, char *output) noexcept;

// Returns the region of 'output' that contains the decoded data.
// Pre: the capacity of 'output' is no less than 3/4 of 'input.size()'.
TStringView decodeBase64(TStringView input, char *output) noexcept;

inline std::string encodeBase64(TStringView input)
{
    std::string result((input.size() * 4)/3 + 4, '\0');
    auto encoded = encodeBase64(input, &result[0]);
    result.resize(encoded.size());
    return result;
}

inline std::string decodeBase64(TStringView input)
{
    std::string result((input.size() * 3)/4 + 3, '\0');
    auto encoded = decodeBase64(input, &result[0]);
    result.resize(encoded.size());
    return result;
}

} // namespace tvision

#endif // TVISION_BASE64_H
