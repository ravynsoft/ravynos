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

#include <assert.h>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "texcompress_astc_luts.h"

namespace Granite
{
static void build_astc_unquant_weight_lut(uint8_t *lut, size_t range, const ASTCQuantizationMode &mode)
{
	for (size_t i = 0; i < range; i++)
	{
		auto &v = lut[i];

		if (!mode.quints && !mode.trits)
		{
			switch (mode.bits)
			{
			case 1:
				v = i * 63;
				break;

			case 2:
				v = i * 0x15;
				break;

			case 3:
				v = i * 9;
				break;

			case 4:
				v = (i << 2) | (i >> 2);
				break;

			case 5:
				v = (i << 1) | (i >> 4);
				break;

			default:
				v = 0;
				break;
			}
		}
		else if (mode.bits == 0)
		{
			if (mode.trits)
				v = 32 * i;
			else
				v = 16 * i;
		}
		else
		{
			unsigned b = (i >> 1) & 1;
			unsigned c = (i >> 2) & 1;
			unsigned A, B, C, D;

			A = 0x7f * (i & 1);
			D = i >> mode.bits;
			B = 0;

			if (mode.trits)
			{
				static const unsigned Cs[3] = { 50, 23, 11 };
				C = Cs[mode.bits - 1];
				if (mode.bits == 2)
					B = 0x45 * b;
				else if (mode.bits == 3)
					B = 0x21 * b + 0x42 * c;
			}
			else
			{
				static const unsigned Cs[2] = { 28, 13 };
				C = Cs[mode.bits - 1];
				if (mode.bits == 2)
					B = 0x42 * b;
			}

			unsigned unq = D * C + B;
			unq ^= A;
			unq = (A & 0x20) | (unq >> 2);
			v = unq;
		}

		// Expand [0, 63] to [0, 64].
		if (mode.bits != 0 && v > 32)
			v++;
	}
}

static void build_astc_unquant_endpoint_lut(uint8_t *lut, size_t range, const ASTCQuantizationMode &mode)
{
	for (size_t i = 0; i < range; i++)
	{
		auto &v = lut[i];

		if (!mode.quints && !mode.trits)
		{
			// Bit-replication.
			switch (mode.bits)
			{
			case 1:
				v = i * 0xff;
				break;

			case 2:
				v = i * 0x55;
				break;

			case 3:
				v = (i << 5) | (i << 2) | (i >> 1);
				break;

			case 4:
				v = i * 0x11;
				break;

			case 5:
				v = (i << 3) | (i >> 2);
				break;

			case 6:
				v = (i << 2) | (i >> 4);
				break;

			case 7:
				v = (i << 1) | (i >> 6);
				break;

			default:
				v = i;
				break;
			}
		}
		else
		{
			unsigned A, B, C, D;
			unsigned b = (i >> 1) & 1;
			unsigned c = (i >> 2) & 1;
			unsigned d = (i >> 3) & 1;
			unsigned e = (i >> 4) & 1;
			unsigned f = (i >> 5) & 1;

			B = 0;
			D = i >> mode.bits;
			A = (i & 1) * 0x1ff;

			if (mode.trits)
			{
				static const unsigned Cs[6] = { 204, 93, 44, 22, 11, 5 };
				C = Cs[mode.bits - 1];

				switch (mode.bits)
				{
				case 2:
					B = b * 0x116;
					break;

				case 3:
					B = b * 0x85 + c * 0x10a;
					break;

				case 4:
					B = b * 0x41 + c * 0x82 + d * 0x104;
					break;

				case 5:
					B = b * 0x20 + c * 0x40 + d * 0x81 + e * 0x102;
					break;

				case 6:
					B = b * 0x10 + c * 0x20 + d * 0x40 + e * 0x80 + f * 0x101;
					break;
				}
			}
			else
			{
				static const unsigned Cs[5] = { 113, 54, 26, 13, 6 };
				C = Cs[mode.bits - 1];

				switch (mode.bits)
				{
				case 2:
					B = b * 0x10c;
					break;

				case 3:
					B = b * 0x82 + c * 0x105;
					break;

				case 4:
					B = b * 0x40 + c * 0x81 + d * 0x102;
					break;

				case 5:
					B = b * 0x20 + c * 0x40 + d * 0x80 + e * 0x101;
					break;
				}
			}

			unsigned unq = D * C + B;
			unq ^= A;
			unq = (A & 0x80) | (unq >> 2);
			v = uint8_t(unq);
		}
	}
}

static unsigned astc_value_range(const ASTCQuantizationMode &mode)
{
	unsigned value_range = 1u << mode.bits;
	if (mode.trits)
		value_range *= 3;
	if (mode.quints)
		value_range *= 5;

	if (value_range == 1)
		value_range = 0;
	return value_range;
}

static uint32_t astc_hash52(uint32_t p)
{
	p ^= p >> 15; p -= p << 17; p += p << 7; p += p << 4;
	p ^= p >>  5; p += p << 16; p ^= p >> 7; p ^= p >> 3;
	p ^= p <<  6; p ^= p >> 17;
	return p;
}

// Copy-paste from spec.
static int astc_select_partition(int seed, int x, int y, int z, int partitioncount, bool small_block)
{
	if (small_block)
	{
		x <<= 1;
		y <<= 1;
		z <<= 1;
	}

	seed += (partitioncount - 1) * 1024;
	uint32_t rnum = astc_hash52(seed);
	uint8_t seed1 = rnum & 0xF;
	uint8_t seed2 = (rnum >> 4) & 0xF;
	uint8_t seed3 = (rnum >> 8) & 0xF;
	uint8_t seed4 = (rnum >> 12) & 0xF;
	uint8_t seed5 = (rnum >> 16) & 0xF;
	uint8_t seed6 = (rnum >> 20) & 0xF;
	uint8_t seed7 = (rnum >> 24) & 0xF;
	uint8_t seed8 = (rnum >> 28) & 0xF;
	uint8_t seed9 = (rnum >> 18) & 0xF;
	uint8_t seed10 = (rnum >> 22) & 0xF;
	uint8_t seed11 = (rnum >> 26) & 0xF;
	uint8_t seed12 = ((rnum >> 30) | (rnum << 2)) & 0xF;

	seed1 *= seed1; seed2 *= seed2; seed3 *= seed3; seed4 *= seed4;
	seed5 *= seed5; seed6 *= seed6; seed7 *= seed7; seed8 *= seed8;
	seed9 *= seed9; seed10 *= seed10; seed11 *= seed11; seed12 *= seed12;

	int sh1, sh2, sh3;
	if (seed & 1)
	{
		sh1 = seed & 2 ? 4 : 5;
		sh2 = partitioncount == 3 ? 6 : 5;
	}
	else
	{
		sh1 = partitioncount == 3 ? 6 : 5;
		sh2 = seed & 2 ? 4 : 5;
	}
	sh3 = (seed & 0x10) ? sh1 : sh2;

	seed1 >>= sh1; seed2 >>= sh2; seed3 >>= sh1; seed4 >>= sh2;
	seed5 >>= sh1; seed6 >>= sh2; seed7 >>= sh1; seed8 >>= sh2;
	seed9 >>= sh3; seed10 >>= sh3; seed11 >>= sh3; seed12 >>= sh3;

	int a = seed1 * x + seed2 * y + seed11 * z + (rnum >> 14);
	int b = seed3 * x + seed4 * y + seed12 * z + (rnum >> 10);
	int c = seed5 * x + seed6 * y + seed9 * z + (rnum >> 6);
	int d = seed7 * x + seed8 * y + seed10 * z + (rnum >> 2);

	a &= 0x3f; b &= 0x3f; c &= 0x3f; d &= 0x3f;

	if (partitioncount < 4)
		d = 0;
	if (partitioncount < 3)
		c = 0;

	if (a >= b && a >= c && a >= d)
		return 0;
	else if (b >= c && b >= d)
		return 1;
	else if (c >= d)
		return 2;
	else
		return 3;
}

ASTCLutHolder::PartitionTable::PartitionTable(unsigned block_width, unsigned block_height)
{
	bool small_block = (block_width * block_height) < 31;

	lut_width = block_width * 32;
	lut_height = block_height * 32;
	lut_buffer.resize(lut_width * lut_height);

	for (unsigned seed_y = 0; seed_y < 32; seed_y++)
	{
		for (unsigned seed_x = 0; seed_x < 32; seed_x++)
		{
			unsigned seed = seed_y * 32 + seed_x;
			for (unsigned block_y = 0; block_y < block_height; block_y++)
			{
				for (unsigned block_x = 0; block_x < block_width; block_x++)
				{
					int part2 = astc_select_partition(seed, block_x, block_y, 0, 2, small_block);
					int part3 = astc_select_partition(seed, block_x, block_y, 0, 3, small_block);
					int part4 = astc_select_partition(seed, block_x, block_y, 0, 4, small_block);
					lut_buffer[(seed_y * block_height + block_y) * lut_width + (seed_x * block_width + block_x)] =
							(part2 << 0) | (part3 << 2) | (part4 << 4);
				}
			}
		}
	}
}

ASTCLutHolder::PartitionTable &ASTCLutHolder::get_partition_table(unsigned width, unsigned height)
{
	std::lock_guard<std::mutex> holder{table_lock};
	auto itr = tables.find(width * 16 + height);
	if (itr != tables.end())
	{
		return itr->second;
	}
	else
	{
		auto &t = tables[width * 16 + height];
		t = { width, height };
		return t;
	}
}

ASTCLutHolder &get_astc_luts()
{
	static ASTCLutHolder holder;
	return holder;
}

ASTCLutHolder::ASTCLutHolder()
{
	init_color_endpoint();
	init_weight_luts();
	init_trits_quints();
}

void ASTCLutHolder::init_color_endpoint()
{
	auto &unquant_lut = color_endpoint.unquant_lut;

	for (size_t i = 0; i < astc_num_quantization_modes; i++)
	{
		auto value_range = astc_value_range(astc_quantization_modes[i]);
		color_endpoint.unquant_lut_offsets[i] = color_endpoint.unquant_offset;
		build_astc_unquant_endpoint_lut(unquant_lut + color_endpoint.unquant_offset, value_range, astc_quantization_modes[i]);
		color_endpoint.unquant_offset += value_range;
	}

	auto &lut = color_endpoint.lut;

	// We can have a maximum of 9 endpoint pairs, i.e. 18 endpoint values in total.
	for (unsigned pairs_minus_1 = 0; pairs_minus_1 < 9; pairs_minus_1++)
	{
		for (unsigned remaining = 0; remaining < 128; remaining++)
		{
			bool found_mode = false;
			for (auto &mode : astc_quantization_modes)
			{
				unsigned num_values = (pairs_minus_1 + 1) * 2;
				unsigned total_bits = mode.bits * num_values +
				                      (mode.quints * 7 * num_values + 2) / 3 +
				                      (mode.trits * 8 * num_values + 4) / 5;

				if (total_bits <= remaining)
				{
					found_mode = true;
					lut[pairs_minus_1][remaining][0] = mode.bits;
					lut[pairs_minus_1][remaining][1] = mode.trits;
					lut[pairs_minus_1][remaining][2] = mode.quints;
					lut[pairs_minus_1][remaining][3] = color_endpoint.unquant_lut_offsets[&mode - astc_quantization_modes];
					break;
				}
			}

			if (!found_mode)
				memset(lut[pairs_minus_1][remaining], 0, sizeof(lut[pairs_minus_1][remaining]));
		}
	}
}

void ASTCLutHolder::init_weight_luts()
{
	auto &lut = weights.lut;
	auto &unquant_lut = weights.unquant_lut;
	auto &unquant_offset = weights.unquant_offset;

	for (size_t i = 0; i < astc_num_weight_modes; i++)
	{
		auto value_range = astc_value_range(astc_weight_modes[i]);
		lut[i][0] = astc_weight_modes[i].bits;
		lut[i][1] = astc_weight_modes[i].trits;
		lut[i][2] = astc_weight_modes[i].quints;
		lut[i][3] = unquant_offset;
		build_astc_unquant_weight_lut(unquant_lut + unquant_offset, value_range, astc_weight_modes[i]);
		unquant_offset += value_range;
	}

	assert(unquant_offset <= 256);
}

void ASTCLutHolder::init_trits_quints()
{
	// From specification.
	auto &trits_quints = integer.trits_quints;

	for (unsigned T = 0; T < 256; T++)
	{
		unsigned C;
		uint8_t t0, t1, t2, t3, t4;

		if (((T >> 2) & 7) == 7)
		{
			C = (((T >> 5) & 7) << 2) | (T & 3);
			t4 = t3 = 2;
		}
		else
		{
			C = T & 0x1f;
			if (((T >> 5) & 3) == 3)
			{
				t4 = 2;
				t3 = (T >> 7) & 1;
			}
			else
			{
				t4 = (T >> 7) & 1;
				t3 = (T >> 5) & 3;
			}
		}

		if ((C & 3) == 3)
		{
			t2 = 2;
			t1 = (C >> 4) & 1;
			t0 = (((C >> 3) & 1) << 1) | (((C >> 2) & 1) & ~(((C >> 3) & 1)));
		}
		else if (((C >> 2) & 3) == 3)
		{
			t2 = 2;
			t1 = 2;
			t0 = C & 3;
		}
		else
		{
			t2 = (C >> 4) & 1;
			t1 = (C >> 2) & 3;
			t0 = (((C >> 1) & 1) << 1) | ((C & 1) & ~(((C >> 1) & 1)));
		}

		trits_quints[T] = t0 | (t1 << 3) | (t2 << 6) | (t3 << 9) | (t4 << 12);
	}

	for (unsigned Q = 0; Q < 128; Q++)
	{
		unsigned C;
		uint8_t q0, q1, q2;
		if (((Q >> 1) & 3) == 3 && ((Q >> 5) & 3) == 0)
		{
			q2 = ((Q & 1) << 2) | ((((Q >> 4) & 1) & ~(Q & 1)) << 1) | (((Q >> 3) & 1) & ~(Q & 1));
			q1 = q0 = 4;
		}
		else
		{
			if (((Q >> 1) & 3) == 3)
			{
				q2 = 4;
				C = (((Q >> 3) & 3) << 3) | ((~(Q >> 5) & 3) << 1) | (Q & 1);
			}
			else
			{
				q2 = (Q >> 5) & 3;
				C = Q & 0x1f;
			}

			if ((C & 7) == 5)
			{
				q1 = 4;
				q0 = (C >> 3) & 3;
			}
			else
			{
				q1 = (C >> 3) & 3;
				q0 = C & 7;
			}
		}

		trits_quints[256 + Q] = q0 | (q1 << 3) | (q2 << 6);
	}
}
}
