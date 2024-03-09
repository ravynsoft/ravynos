#include <internal/utf8.h>

namespace tvision
{

size_t utf16To8(TSpan<const uint16_t> input, char *output) noexcept
{
    size_t j = 0;
    for (size_t i = 0; i < input.size(); ++i)
    {
        uint16_t lead = input[i];
        uint16_t trail = 0;
        if (i + 1 < input.size())
        {
            uint16_t next = input[i + 1];
            if ( 0xD800 <= lead && lead <= 0xDBFF &&
                 0xDC00 <= next && next <= 0xDFFF )
            {
                // Surrogate pairs.
                trail = next;
                ++i;
            }
        }

        uint16_t u16[2] {lead, trail};
        j += utf16To8(u16, &output[j]);
    }
    return j;
}

} // namespace tvision
