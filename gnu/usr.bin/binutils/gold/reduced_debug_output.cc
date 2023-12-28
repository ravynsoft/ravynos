// reduced_debug_output.cc -- output reduced debugging information to save space

// Copyright (C) 2008-2023 Free Software Foundation, Inc.
// Written by Caleb Howe <cshowe@google.com>.

// This file is part of gold.

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
// MA 02110-1301, USA.

#include "gold.h"

#include "parameters.h"
#include "options.h"
#include "dwarf.h"
#include "dwarf_reader.h"
#include "reduced_debug_output.h"
#include "int_encoding.h"

#include <vector>

namespace gold
{

// Given a pointer to the beginning of a die and the beginning of the associated
// abbreviation fills in die_end with the end of the information entry.  If
// successful returns true.  Get_die_end also takes a pointer to the end of the
// buffer containing the die.  If die_end would be beyond the end of the
// buffer, or if an unsupported dwarf form is encountered returns false.
bool
Output_reduced_debug_info_section::get_die_end(
    unsigned char* die, unsigned char* abbrev, unsigned char** die_end,
    unsigned char* buffer_end, int address_size, bool is64)
{
  size_t LEB_size;
  uint64_t LEB_decoded;
  for(;;)
    {
      uint64_t attribute = read_unsigned_LEB_128(abbrev, &LEB_size);
      abbrev += LEB_size;
      elfcpp::DW_FORM form =
          static_cast<elfcpp::DW_FORM>(read_unsigned_LEB_128(abbrev,
                                                             &LEB_size));
      abbrev += LEB_size;
      if (!(attribute || form))
        break;
      if (die >= buffer_end)
        return false;
      switch(form)
        {
          case elfcpp::DW_FORM_flag_present:
            break;
          case elfcpp::DW_FORM_strp:
          case elfcpp::DW_FORM_sec_offset:
            die += is64 ? 8 : 4;
            break;
          case elfcpp::DW_FORM_addr:
          case elfcpp::DW_FORM_ref_addr:
            die += address_size;
            break;
          case elfcpp::DW_FORM_block1:
            die += *die;
            die += 1;
            break;
          case elfcpp::DW_FORM_block2:
            {
              uint16_t block_size;
              block_size = read_from_pointer<16>(&die);
              die += block_size;
              break;
            }
          case elfcpp::DW_FORM_block4:
            {
              uint32_t block_size;
              block_size = read_from_pointer<32>(&die);
              die += block_size;
              break;
            }
          case elfcpp::DW_FORM_block:
          case elfcpp::DW_FORM_exprloc:
            LEB_decoded = read_unsigned_LEB_128(die, &LEB_size);
            die += (LEB_decoded + LEB_size);
            break;
          case elfcpp::DW_FORM_data1:
          case elfcpp::DW_FORM_ref1:
          case elfcpp::DW_FORM_flag:
            die += 1;
            break;
          case elfcpp::DW_FORM_data2:
          case elfcpp::DW_FORM_ref2:
            die += 2;
            break;
          case elfcpp::DW_FORM_data4:
          case elfcpp::DW_FORM_ref4:
            die += 4;
            break;
          case elfcpp::DW_FORM_data8:
          case elfcpp::DW_FORM_ref8:
          case elfcpp::DW_FORM_ref_sig8:
            die += 8;
            break;
          case elfcpp::DW_FORM_ref_udata:
          case elfcpp::DW_FORM_udata:
            read_unsigned_LEB_128(die, &LEB_size);
            die += LEB_size;
            break;
          case elfcpp::DW_FORM_sdata:
            read_signed_LEB_128(die, &LEB_size);
            die += LEB_size;
            break;
          case elfcpp::DW_FORM_string:
            {
              size_t length = strlen(reinterpret_cast<char*>(die));
              die += length + 1;
              break;
            }
          case elfcpp::DW_FORM_indirect:
          case elfcpp::DW_FORM_GNU_addr_index:
          case elfcpp::DW_FORM_GNU_str_index:
          default:
            return false;
      }
    }
  *die_end = die;
  return true;
}

void
Output_reduced_debug_abbrev_section::set_final_data_size()
{
  if (this->sized_ || this->failed_)
    return;

  uint64_t abbrev_number;
  size_t LEB_size;
  unsigned char* abbrev_data = this->postprocessing_buffer();
  unsigned char* abbrev_end = this->postprocessing_buffer() +
                              this->postprocessing_buffer_size();
  this->write_to_postprocessing_buffer();
  while(abbrev_data < abbrev_end)
    {
      uint64_t abbrev_offset = abbrev_data - this->postprocessing_buffer();
      while((abbrev_number = read_unsigned_LEB_128(abbrev_data, &LEB_size)))
        {
          if (abbrev_data >= abbrev_end)
            {
              failed("Debug abbreviations extend beyond .debug_abbrev "
                     "section; failed to reduce debug abbreviations");
              return;
            }
          abbrev_data += LEB_size;

          // Together with the abbreviation number these fields make up
          // the header for each abbreviation.
          uint64_t abbrev_type = read_unsigned_LEB_128(abbrev_data, &LEB_size);
          abbrev_data += LEB_size;

          // This would ordinarily be the has_children field of the
          // abbreviation.  But it's going to be false after reducing the
          // information, so there's no point in storing it.
          abbrev_data++;

          // Read to the end of the current abbreviation.
          // This is indicated by two zero unsigned LEBs in a row.  We don't
          // need to parse the data yet, so we just scan through the data
          // looking for two consecutive 0 bytes indicating the end of the
          // abbreviation.
          unsigned char* current_abbrev;
          for (current_abbrev = abbrev_data;
               current_abbrev[0] || current_abbrev[1];
               current_abbrev++)
            {
              if (current_abbrev >= abbrev_end)
                {
                  this->failed(_("Debug abbreviations extend beyond "
				 ".debug_abbrev section; failed to reduce "
				 "debug abbreviations"));
                  return;
                }
            }
          // Account for the two nulls and advance to the start of the
          // next abbreviation.
          current_abbrev += 2;

          // We're eliminating every entry except for compile units, so we
          // only need to store abbreviations that describe them
          if (abbrev_type == elfcpp::DW_TAG_compile_unit)
            {
              write_unsigned_LEB_128(&this->data_, ++this->abbrev_count_);
              write_unsigned_LEB_128(&this->data_, abbrev_type);
              // has_children is false for all entries
              this->data_.push_back(0);
              this->abbrev_mapping_[std::make_pair(abbrev_offset,
                                                   abbrev_number)] =
                  std::make_pair(abbrev_count_, this->data_.size());
              this->data_.insert(this->data_.end(), abbrev_data,
                                 current_abbrev);
            }
          abbrev_data = current_abbrev;
        }
      gold_assert(LEB_size == 1);
      abbrev_data += LEB_size;
    }
  // Null terminate the list of abbreviations
  this->data_.push_back(0);
  this->set_data_size(data_.size());
  this->sized_ = true;
}

void
Output_reduced_debug_abbrev_section::do_write(Output_file* of)
{
  off_t offset = this->offset();
  off_t data_size = this->data_size();
  unsigned char* view = of->get_output_view(offset, data_size);
  if (this->failed_)
    memcpy(view, this->postprocessing_buffer(),
           this->postprocessing_buffer_size());
  else
    memcpy(view, &this->data_.front(), data_size);
  of->write_output_view(offset, data_size, view);
}

// Locates the abbreviation with abbreviation_number abbrev_number in the
// abbreviation table at offset abbrev_offset.  abbrev_number is updated with
// its new abbreviation number and a pointer to the beginning of the
// abbreviation is returned.
unsigned char*
Output_reduced_debug_abbrev_section::get_new_abbrev(
  uint64_t* abbrev_number, uint64_t abbrev_offset)
{
  set_final_data_size();
  std::pair<uint64_t, uint64_t> abbrev_info =
      this->abbrev_mapping_[std::make_pair(abbrev_offset, *abbrev_number)];
  *abbrev_number = abbrev_info.first;
  return &this->data_[abbrev_info.second];
}

void Output_reduced_debug_info_section::set_final_data_size()
{
  if (this->failed_)
    return;
  unsigned char* debug_info = this->postprocessing_buffer();
  unsigned char* debug_info_end = (this->postprocessing_buffer()
				   + this->postprocessing_buffer_size());
  unsigned char* next_compile_unit;
  this->write_to_postprocessing_buffer();

  while (debug_info < debug_info_end)
    {
      uint32_t compile_unit_start = read_from_pointer<32>(&debug_info);
      // The first 4 bytes of each compile unit determine whether or
      // not we're using dwarf32 or dwarf64.  This is not necessarily
      // related to whether the binary is 32 or 64 bits.
      if (compile_unit_start == 0xFFFFFFFF)
        {
          // Technically the size can be up to 96 bits.  Rather than handle
          // 96/128 bit integers we just truncate the size at 64 bits.
          if (0 != read_from_pointer<32>(&debug_info))
            {
              this->failed(_("Extremely large compile unit in debug info; "
			     "failed to reduce debug info"));
              return;
            }
          const int dwarf64_header_size = sizeof(uint64_t) + sizeof(uint16_t) +
                                          sizeof(uint64_t) + sizeof(uint8_t);
          if (debug_info + dwarf64_header_size >= debug_info_end)
            {
              this->failed(_("Debug info extends beyond .debug_info section;"
			     "failed to reduce debug info"));
              return;
            }

          uint64_t compile_unit_size = read_from_pointer<64>(&debug_info);
          next_compile_unit = debug_info + compile_unit_size;
          uint16_t version = read_from_pointer<16>(&debug_info);
          uint64_t abbrev_offset = read_from_pointer<64>(&debug_info);
          uint8_t address_size = read_from_pointer<8>(&debug_info);
          size_t LEB_size;
          uint64_t abbreviation_number = read_unsigned_LEB_128(debug_info,
                                                               &LEB_size);
          debug_info += LEB_size;
          unsigned char* die_abbrev = this->associated_abbrev_->get_new_abbrev(
              &abbreviation_number, abbrev_offset);
          unsigned char* die_end;
          if (!this->get_die_end(debug_info, die_abbrev, &die_end,
                                 debug_info_end, address_size, true))
            {
              this->failed(_("Invalid DIE in debug info; "
			     "failed to reduce debug info"));
              return;
            }

          insert_into_vector<32>(&this->data_, 0xFFFFFFFF);
          insert_into_vector<32>(&this->data_, 0);
          insert_into_vector<64>(
              &this->data_,
              (11 + get_length_as_unsigned_LEB_128(abbreviation_number)
	       + die_end - debug_info));
          insert_into_vector<16>(&this->data_, version);
          insert_into_vector<64>(&this->data_, 0);
          insert_into_vector<8>(&this->data_, address_size);
          write_unsigned_LEB_128(&this->data_, abbreviation_number);
          this->data_.insert(this->data_.end(), debug_info, die_end);
        }
      else
        {
          const int dwarf32_header_size =
              sizeof(uint16_t) + sizeof(uint32_t) + sizeof(uint8_t);
          if (debug_info + dwarf32_header_size >= debug_info_end)
            {
              this->failed(_("Debug info extends beyond .debug_info section; "
			     "failed to reduce debug info"));
              return;
            }
          uint32_t compile_unit_size = compile_unit_start;
          next_compile_unit = debug_info + compile_unit_size;
          uint16_t version = read_from_pointer<16>(&debug_info);
          uint32_t abbrev_offset = read_from_pointer<32>(&debug_info);
          uint8_t address_size = read_from_pointer<8>(&debug_info);
          size_t LEB_size;
          uint64_t abbreviation_number = read_unsigned_LEB_128(debug_info,
                                                               &LEB_size);
          debug_info += LEB_size;
          unsigned char* die_abbrev = this->associated_abbrev_->get_new_abbrev(
              &abbreviation_number, abbrev_offset);
          unsigned char* die_end;
          if (!this->get_die_end(debug_info, die_abbrev, &die_end,
                                 debug_info_end, address_size, false))
            {
              this->failed(_("Invalid DIE in debug info; "
			     "failed to reduce debug info"));
              return;
            }

          insert_into_vector<32>(
              &this->data_,
              (7 + get_length_as_unsigned_LEB_128(abbreviation_number)
	       + die_end - debug_info));
          insert_into_vector<16>(&this->data_, version);
          insert_into_vector<32>(&this->data_, 0);
          insert_into_vector<8>(&this->data_, address_size);
          write_unsigned_LEB_128(&this->data_, abbreviation_number);
          this->data_.insert(this->data_.end(), debug_info, die_end);
        }
      debug_info = next_compile_unit;
    }
  this->set_data_size(data_.size());
}

void Output_reduced_debug_info_section::do_write(Output_file* of)
{
  off_t offset = this->offset();
  off_t data_size = this->data_size();
  unsigned char* view = of->get_output_view(offset, data_size);
  if (this->failed_)
    memcpy(view, this->postprocessing_buffer(),
           this->postprocessing_buffer_size());
  else
    memcpy(view, &this->data_.front(), data_size);
  of->write_output_view(offset, data_size, view);
}

} // End namespace gold.
