/* Copyright (c) 2017-2022 Hans-Kristian Arntzen
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <mutex>
#include <unordered_map>
#include <vector>

namespace Granite
{

struct ASTCQuantizationMode
{
        uint8_t bits, trits, quints;
};

// In order to decode color endpoints, we need to convert available bits and number of values
// into a format of (bits, trits, quints). A simple LUT texture is a reasonable approach for this.
// Decoders are expected to have some form of LUT to deal with this ...
static const ASTCQuantizationMode astc_quantization_modes[] = {
        { 8, 0, 0 },
        { 6, 1, 0 },
        { 5, 0, 1 },
        { 7, 0, 0 },
        { 5, 1, 0 },
        { 4, 0, 1 },
        { 6, 0, 0 },
        { 4, 1, 0 },
        { 3, 0, 1 },
        { 5, 0, 0 },
        { 3, 1, 0 },
        { 2, 0, 1 },
        { 4, 0, 0 },
        { 2, 1, 0 },
        { 1, 0, 1 },
        { 3, 0, 0 },
        { 1, 1, 0 },
};

constexpr size_t astc_num_quantization_modes = sizeof(astc_quantization_modes) / sizeof(astc_quantization_modes[0]);

static const ASTCQuantizationMode astc_weight_modes[] = {
        { 0, 0, 0 }, // Invalid
        { 0, 0, 0 }, // Invalid
        { 1, 0, 0 },
        { 0, 1, 0 },
        { 2, 0, 0 },
        { 0, 0, 1 },
        { 1, 1, 0 },
        { 3, 0, 0 },
        { 0, 0, 0 }, // Invalid
        { 0, 0, 0 }, // Invalid
        { 1, 0, 1 },
        { 2, 1, 0 },
        { 4, 0, 0 },
        { 2, 0, 1 },
        { 3, 1, 0 },
        { 5, 0, 0 },
};

constexpr size_t astc_num_weight_modes = sizeof(astc_weight_modes) / sizeof(astc_weight_modes[0]);

struct ASTCLutHolder
{
        ASTCLutHolder();

        void init_color_endpoint();
        void init_weight_luts();
        void init_trits_quints();

        struct
        {
                size_t unquant_offset = 0;
                uint8_t unquant_lut[2048];
                uint16_t lut[9][128][4];
                size_t unquant_lut_offsets[astc_num_quantization_modes];
        } color_endpoint;

        struct
        {
                size_t unquant_offset = 0;
                uint8_t unquant_lut[2048];
                uint8_t lut[astc_num_weight_modes][4];
        } weights;

        struct
        {
                uint16_t trits_quints[256 + 128];
        } integer;

        struct PartitionTable
        {
                PartitionTable() = default;
                PartitionTable(unsigned width, unsigned height);
                std::vector<uint8_t> lut_buffer;
                unsigned lut_width = 0;
                unsigned lut_height = 0;
        };

        std::mutex table_lock;
        std::unordered_map<unsigned, PartitionTable> tables;

        PartitionTable &get_partition_table(unsigned width, unsigned height);
};

ASTCLutHolder &get_astc_luts();
}
