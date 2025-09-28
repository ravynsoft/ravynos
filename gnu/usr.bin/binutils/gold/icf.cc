// icf.cc -- Identical Code Folding.
//
// Copyright (C) 2009-2023 Free Software Foundation, Inc.
// Written by Sriraman Tallam <tmsriram@google.com>.

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

// Identical Code Folding Algorithm
// ----------------------------------
// Detecting identical functions is done here and the basic algorithm
// is as follows.  A checksum is computed on each foldable section using
// its contents and relocations.  If the symbol name corresponding to
// a relocation is known it is used to compute the checksum.  If the
// symbol name is not known the stringified name of the object and the
// section number pointed to by the relocation is used.  The checksums
// are stored as keys in a hash map and a section is identical to some
// other section if its checksum is already present in the hash map.
// Checksum collisions are handled by using a multimap and explicitly
// checking the contents when two sections have the same checksum.
//
// However, two functions A and B with identical text but with
// relocations pointing to different foldable sections can be identical if
// the corresponding foldable sections to which their relocations point to
// turn out to be identical.  Hence, this checksumming process must be
// done repeatedly until convergence is obtained.  Here is an example for
// the following case :
//
// int funcA ()               int funcB ()
// {                          {
//   return foo();              return goo();
// }                          }
//
// The functions funcA and funcB are identical if functions foo() and
// goo() are identical.
//
// Hence, as described above, we repeatedly do the checksumming,
// assigning identical functions to the same group, until convergence is
// obtained.  Now, we have two different ways to do this depending on how
// we initialize.
//
// Algorithm I :
// -----------
// We can start with marking all functions as different and repeatedly do
// the checksumming.  This has the advantage that we do not need to wait
// for convergence. We can stop at any point and correctness will be
// guaranteed although not all cases would have been found.  However, this
// has a problem that some cases can never be found even if it is run until
// convergence.  Here is an example with mutually recursive functions :
//
// int funcA (int a)            int funcB (int a)
// {                            {
//   if (a == 1)                  if (a == 1)
//     return 1;                    return 1;
//   return 1 + funcB(a - 1);     return 1 + funcA(a - 1);
// }                            }
//
// In this example funcA and funcB are identical and one of them could be
// folded into the other.  However, if we start with assuming that funcA
// and funcB are not identical, the algorithm, even after it is run to
// convergence, cannot detect that they are identical.  It should be noted
// that even if the functions were self-recursive, Algorithm I cannot catch
// that they are identical, at least as is.
//
// Algorithm II :
// ------------
// Here we start with marking all functions as identical and then repeat
// the checksumming until convergence.  This can detect the above case
// mentioned above.  It can detect all cases that Algorithm I can and more.
// However, the caveat is that it has to be run to convergence.  It cannot
// be stopped arbitrarily like Algorithm I as correctness cannot be
// guaranteed.  Algorithm II is not implemented.
//
// Algorithm I is used because experiments show that about three
// iterations are more than enough to achieve convergence. Algorithm I can
// handle recursive calls if it is changed to use a special common symbol
// for recursive relocs.  This seems to be the most common case that
// Algorithm I could not catch as is.  Mutually recursive calls are not
// frequent and Algorithm I wins because of its ability to be stopped
// arbitrarily.
//
// Caveat with using function pointers :
// ------------------------------------
//
// Programs using function pointer comparisons/checks should use function
// folding with caution as the result of such comparisons could be different
// when folding takes place.  This could lead to unexpected run-time
// behaviour.
//
// Safe Folding :
// ------------
//
// ICF in safe mode folds only ctors and dtors if their function pointers can
// never be taken.  Also, for X86-64, safe folding uses the relocation
// type to determine if a function's pointer is taken or not and only folds
// functions whose pointers are definitely not taken.
//
// Caveat with safe folding :
// ------------------------
//
// This applies only to x86_64.
//
// Position independent executables are created from PIC objects (compiled
// with -fPIC) and/or PIE objects (compiled with -fPIE).  For PIE objects, the
// relocation types for function pointer taken and a call are the same.
// Now, it is not always possible to tell if an object used in the link of
// a pie executable is a PIC object or a PIE object.  Hence, for pie
// executables, using relocation types to disambiguate function pointers is
// currently disabled.
//
// Further, it is not correct to use safe folding to build non-pie
// executables using PIC/PIE objects.  PIC/PIE objects have different
// relocation types for function pointers than non-PIC objects, and the
// current implementation of safe folding does not handle those relocation
// types.  Hence, if used, functions whose pointers are taken could still be
// folded causing unpredictable run-time behaviour if the pointers were used
// in comparisons.
//
// Notes regarding C++ exception handling :
// --------------------------------------
//
// It is possible for two sections to have identical text, identical
// relocations, but different exception handling metadata (unwind
// information in the .eh_frame section, and/or handler information in
// a .gcc_except_table section).  Thus, if a foldable section is
// referenced from a .eh_frame FDE, we must include in its checksum
// the contents of that FDE as well as of the CIE that the FDE refers
// to.  The CIE and FDE in turn probably contain relocations to the
// personality routine and LSDA, which are handled like any other
// relocation for ICF purposes.  This logic is helped by the fact that
// gcc with -ffunction-sections puts each function's LSDA in its own
// .gcc_except_table.<functionname> section.  Given sections for two
// functions with nontrivial exception handling logic, we will
// determine on the first iteration that their .gcc_except_table
// sections are identical and can be folded, and on the second
// iteration that their .text and .eh_frame contents (including the
// now-merged .gcc_except_table relocations for the LSDA) are
// identical and can be folded.
//
//
// How to run  : --icf=[safe|all|none]
// Optional parameters : --icf-iterations <num> --print-icf-sections
//
// Performance : Less than 20 % link-time overhead on industry strength
// applications.  Up to 6 %  text size reductions.

#include "gold.h"
#include "object.h"
#include "gc.h"
#include "icf.h"
#include "symtab.h"
#include "libiberty.h"
#include "demangle.h"
#include "elfcpp.h"
#include "int_encoding.h"

#include <limits>

namespace gold
{

// This function determines if a section or a group of identical
// sections has unique contents.  Such unique sections or groups can be
// declared final and need not be processed any further.
// Parameters :
// ID_SECTION : Vector mapping a section index to a Section_id pair.
// IS_SECN_OR_GROUP_UNIQUE : To check if a section or a group of identical
//                            sections is already known to be unique.
// SECTION_CONTENTS : Contains the section's text and relocs to sections
//                    that cannot be folded.   SECTION_CONTENTS are NULL
//                    implies that this function is being called for the
//                    first time before the first iteration of icf.

static void
preprocess_for_unique_sections(const std::vector<Section_id>& id_section,
                               std::vector<bool>* is_secn_or_group_unique,
                               std::vector<std::string>* section_contents)
{
  Unordered_map<uint32_t, unsigned int> uniq_map;
  std::pair<Unordered_map<uint32_t, unsigned int>::iterator, bool>
    uniq_map_insert;

  for (unsigned int i = 0; i < id_section.size(); i++)
    {
      if ((*is_secn_or_group_unique)[i])
        continue;

      uint32_t cksum;
      Section_id secn = id_section[i];
      section_size_type plen;
      if (section_contents == NULL)
        {
          // Lock the object so we can read from it.  This is only called
          // single-threaded from queue_middle_tasks, so it is OK to lock.
          // Unfortunately we have no way to pass in a Task token.
          const Task* dummy_task = reinterpret_cast<const Task*>(-1);
          Task_lock_obj<Object> tl(dummy_task, secn.first);
          const unsigned char* contents;
          contents = secn.first->section_contents(secn.second,
                                                  &plen,
                                                  false);
          cksum = xcrc32(contents, plen, 0xffffffff);
        }
      else
        {
          const unsigned char* contents_array = reinterpret_cast
            <const unsigned char*>((*section_contents)[i].c_str());
          cksum = xcrc32(contents_array, (*section_contents)[i].length(),
                         0xffffffff);
        }
      uniq_map_insert = uniq_map.insert(std::make_pair(cksum, i));
      if (uniq_map_insert.second)
        {
          (*is_secn_or_group_unique)[i] = true;
        }
      else
        {
          (*is_secn_or_group_unique)[i] = false;
          (*is_secn_or_group_unique)[uniq_map_insert.first->second] = false;
        }
    }
}

// For SHF_MERGE sections that use REL relocations, the addend is stored in
// the text section at the relocation offset.  Read  the addend value given
// the pointer to the addend in the text section and the addend size.
// Update the addend value if a valid addend is found.
// Parameters:
// RELOC_ADDEND_PTR   : Pointer to the addend in the text section.
// ADDEND_SIZE        : The size of the addend.
// RELOC_ADDEND_VALUE : Pointer to the addend that is updated.

inline void
get_rel_addend(const unsigned char* reloc_addend_ptr,
	       const unsigned int addend_size,
	       uint64_t* reloc_addend_value)
{
  switch (addend_size)
    {
    case 0:
      break;
    case 1:
      *reloc_addend_value =
        read_from_pointer<8>(reloc_addend_ptr);
      break;
    case 2:
      *reloc_addend_value =
          read_from_pointer<16>(reloc_addend_ptr);
      break;
    case 4:
      *reloc_addend_value =
        read_from_pointer<32>(reloc_addend_ptr);
      break;
    case 8:
      *reloc_addend_value =
        read_from_pointer<64>(reloc_addend_ptr);
      break;
    default:
      gold_unreachable();
    }
}

// This returns the buffer containing the section's contents, both
// text and relocs.  Relocs are differentiated as those pointing to
// sections that could be folded and those that cannot.  Only relocs
// pointing to sections that could be folded are recomputed on
// subsequent invocations of this function.
// Parameters  :
// FIRST_ITERATION    : true if it is the first invocation.
// FIXED_CACHE        : String that stores the portion of the result that
//                      does not change from iteration to iteration;
//                      written if first_iteration is true, read if it's false.
// SECN               : Section for which contents are desired.
// SELF_SECN          : Relocations that target this section will be
//                      considered "relocations to self" so that recursive
//                      functions can be folded. Should normally be the
//                      same as `secn` except when processing extra identity
//                      regions.
// NUM_TRACKED_RELOCS : Vector reference to store the number of relocs
//                      to ICF sections.
// KEPT_SECTION_ID    : Vector which maps folded sections to kept sections.
// START_OFFSET       : Only consider the part of the section at and after
//                      this offset.
// END_OFFSET         : Only consider the part of the section before this
//                      offset.

static std::string
get_section_contents(bool first_iteration,
		     std::string* fixed_cache,
                     const Section_id& secn,
		     const Section_id& self_secn,
                     unsigned int* num_tracked_relocs,
                     Symbol_table* symtab,
                     const std::vector<unsigned int>& kept_section_id,
		     section_offset_type start_offset = 0,
		     section_offset_type end_offset =
		       std::numeric_limits<section_offset_type>::max())
{
  section_size_type plen;
  const unsigned char* contents = NULL;
  if (first_iteration)
    contents = secn.first->section_contents(secn.second, &plen, false);

  // The buffer to hold all the contents including relocs.  A checksum
  // is then computed on this buffer.
  std::string buffer;
  std::string icf_reloc_buffer;

  Icf::Reloc_info_list& reloc_info_list = 
    symtab->icf()->reloc_info_list();

  Icf::Reloc_info_list::iterator it_reloc_info_list =
    reloc_info_list.find(secn);

  buffer.clear();
  icf_reloc_buffer.clear();

  // Process relocs and put them into the buffer.

  if (it_reloc_info_list != reloc_info_list.end())
    {
      Icf::Sections_reachable_info &v =
        (it_reloc_info_list->second).section_info;
      // Stores the information of the symbol pointed to by the reloc.
      const Icf::Symbol_info &s = (it_reloc_info_list->second).symbol_info;
      // Stores the addend and the symbol value.
      Icf::Addend_info &a = (it_reloc_info_list->second).addend_info;
      // Stores the offset of the reloc.
      const Icf::Offset_info &o = (it_reloc_info_list->second).offset_info;
      const Icf::Reloc_addend_size_info &reloc_addend_size_info =
        (it_reloc_info_list->second).reloc_addend_size_info;
      Icf::Sections_reachable_info::iterator it_v = v.begin();
      Icf::Symbol_info::const_iterator it_s = s.begin();
      Icf::Addend_info::iterator it_a = a.begin();
      Icf::Offset_info::const_iterator it_o = o.begin();
      Icf::Reloc_addend_size_info::const_iterator it_addend_size =
        reloc_addend_size_info.begin();

      for (; it_v != v.end(); ++it_v, ++it_s, ++it_a, ++it_o, ++it_addend_size)
        {
	  Symbol* gsym = *it_s;
	  bool is_section_symbol = false;

	  // Ignore relocations outside the region we were told to look at
	  if (static_cast<section_offset_type>(*it_o) < start_offset
	      || static_cast<section_offset_type>(*it_o) >= end_offset)
	    continue;

	  // A -1 value in the symbol vector indicates a local section symbol.
	  if (gsym == reinterpret_cast<Symbol*>(-1))
	    {
	      is_section_symbol = true;
	      gsym = NULL;
	    }

	  if (first_iteration
	      && it_v->first != NULL)
	    {
	      Symbol_location loc;
	      loc.object = it_v->first;
	      loc.shndx = it_v->second;
	      loc.offset = convert_types<off_t, long long>(it_a->first
							   + it_a->second);
	      // Look through function descriptors
	      parameters->target().function_location(&loc);
	      if (loc.shndx != it_v->second)
		{
		  it_v->second = loc.shndx;
		  // Modify symvalue/addend to the code entry.
		  it_a->first = loc.offset;
		  it_a->second = 0;
		}
	    }

          // ADDEND_STR stores the symbol value and addend and offset,
          // each at most 16 hex digits long.  it_a points to a pair
          // where first is the symbol value and second is the
          // addend.
          char addend_str[50];

	  // It would be nice if we could use format macros in inttypes.h
	  // here but there are not in ISO/IEC C++ 1998.
          snprintf(addend_str, sizeof(addend_str), "%llx %llx %llx",
                   static_cast<long long>((*it_a).first),
		   static_cast<long long>((*it_a).second),
		   static_cast<unsigned long long>(*it_o - start_offset));

	  // If the symbol pointed to by the reloc is not in an ordinary
	  // section or if the symbol type is not FROM_OBJECT, then the
	  // object is NULL.
	  if (it_v->first == NULL)
            {
	      if (first_iteration)
                {
		  // If the symbol name is available, use it.
                  if (gsym != NULL)
                      buffer.append(gsym->name());
                  // Append the addend.
                  buffer.append(addend_str);
                  buffer.append("@");
		}
	      continue;
	    }

          Section_id reloc_secn(it_v->first, it_v->second);

          // If this reloc turns back and points to the same section,
          // like a recursive call, use a special symbol to mark this.
          if (reloc_secn.first == self_secn.first
              && reloc_secn.second == self_secn.second)
            {
              if (first_iteration)
                {
                  buffer.append("R");
                  buffer.append(addend_str);
                  buffer.append("@");
                }
              continue;
            }
          Icf::Uniq_secn_id_map& section_id_map =
            symtab->icf()->section_to_int_map();
          Icf::Uniq_secn_id_map::iterator section_id_map_it =
            section_id_map.find(reloc_secn);
          bool is_sym_preemptible = (gsym != NULL
				     && !gsym->is_from_dynobj()
				     && !gsym->is_undefined()
				     && gsym->is_preemptible());
          if (!is_sym_preemptible
              && section_id_map_it != section_id_map.end())
            {
              // This is a reloc to a section that might be folded.
              if (num_tracked_relocs)
                (*num_tracked_relocs)++;

              char kept_section_str[10];
              unsigned int secn_id = section_id_map_it->second;
              snprintf(kept_section_str, sizeof(kept_section_str), "%u",
                       kept_section_id[secn_id]);
              if (first_iteration)
                {
                  buffer.append("ICF_R");
                  buffer.append(addend_str);
                }
              icf_reloc_buffer.append(kept_section_str);
              // Append the addend.
              icf_reloc_buffer.append(addend_str);
              icf_reloc_buffer.append("@");
            }
          else
            {
              // This is a reloc to a section that cannot be folded.
              // Process it only in the first iteration.
              if (!first_iteration)
                continue;

              uint64_t secn_flags = (it_v->first)->section_flags(it_v->second);
              // This reloc points to a merge section.  Hash the
              // contents of this section.
              if ((secn_flags & elfcpp::SHF_MERGE) != 0
		  && parameters->target().can_icf_inline_merge_sections())
                {
                  uint64_t entsize =
                    (it_v->first)->section_entsize(it_v->second);
		  long long offset = it_a->first;

		  // Handle SHT_RELA and SHT_REL addends. Only one of these
		  // addends exists. When pointing to a merge section, the
		  // addend only matters if it's relative to a section
		  // symbol. In order to unambiguously identify the target
		  // of the relocation, the compiler (and assembler) must use
		  // a local non-section symbol unless Symbol+Addend does in
		  // fact point directly to the target. (In other words,
		  // a bias for a pc-relative reference or a non-zero based
		  // access forces the use of a local symbol, and the addend
		  // is used only to provide that bias.)
		  uint64_t reloc_addend_value = 0;
		  if (is_section_symbol)
		    {
		      // Get the SHT_RELA addend.  For RELA relocations,
		      // we have the addend from the relocation.
		      reloc_addend_value = it_a->second;

		      // Handle SHT_REL addends.
		      // For REL relocations, we need to fetch the addend
		      // from the section contents.
		      const unsigned char* reloc_addend_ptr =
			contents + static_cast<unsigned long long>(*it_o);

		      // Update the addend value with the SHT_REL addend if
		      // available.
		      get_rel_addend(reloc_addend_ptr, *it_addend_size,
				     &reloc_addend_value);

		      // Ignore the addend when it is a negative value.
		      // See the comments in Merged_symbol_value::value
		      // in object.h.
		      if (reloc_addend_value < 0xffffff00)
			offset = offset + reloc_addend_value;
		    }

                  section_size_type secn_len;

                  const unsigned char* str_contents =
                  (it_v->first)->section_contents(it_v->second,
                                                  &secn_len,
                                                  false) + offset;
		  gold_assert (offset < (long long) secn_len);

                  if ((secn_flags & elfcpp::SHF_STRINGS) != 0)
                    {
                      // String merge section.
                      const char* str_char =
                        reinterpret_cast<const char*>(str_contents);
                      switch(entsize)
                        {
                        case 1:
                          {
                            buffer.append(str_char);
                            break;
                          }
                        case 2:
                          {
                            const uint16_t* ptr_16 =
                              reinterpret_cast<const uint16_t*>(str_char);
                            unsigned int strlen_16 = 0;
                            // Find the NULL character.
                            while(*(ptr_16 + strlen_16) != 0)
                                strlen_16++;
                            buffer.append(str_char, strlen_16 * 2);
                          }
                          break;
                        case 4:
                          {
                            const uint32_t* ptr_32 =
                              reinterpret_cast<const uint32_t*>(str_char);
                            unsigned int strlen_32 = 0;
                            // Find the NULL character.
                            while(*(ptr_32 + strlen_32) != 0)
                                strlen_32++;
                            buffer.append(str_char, strlen_32 * 4);
                          }
                          break;
                        default:
                          gold_unreachable();
                        }
                    }
                  else
                    {
                      // Use the entsize to determine the length to copy.
		      uint64_t bufsize = entsize;
		      // If entsize is too big, copy all the remaining bytes.
		      if ((offset + entsize) > secn_len)
			bufsize = secn_len - offset;
                      buffer.append(reinterpret_cast<const
                                                     char*>(str_contents),
                                    bufsize);
                    }
		  buffer.append("@");
                }
              else if (gsym != NULL)
                {
                  // If symbol name is available use that.
                  buffer.append(gsym->name());
                  // Append the addend.
                  buffer.append(addend_str);
                  buffer.append("@");
                }
              else
                {
                  // Symbol name is not available, like for a local symbol,
                  // use object and section id.
                  buffer.append(it_v->first->name());
                  char secn_id[10];
                  snprintf(secn_id, sizeof(secn_id), "%u",it_v->second);
                  buffer.append(secn_id);
                  // Append the addend.
                  buffer.append(addend_str);
                  buffer.append("@");
                }
            }
        }
    }

  if (first_iteration)
    {
      buffer.append("Contents = ");

      const unsigned char* slice_end =
	contents + std::min<section_offset_type>(plen, end_offset);

      if (contents + start_offset < slice_end)
	{
	  buffer.append(reinterpret_cast<const char*>(contents + start_offset),
			slice_end - (contents + start_offset));
	}
    }

  // Add any extra identity regions.
  std::pair<Icf::Extra_identity_list::const_iterator,
	    Icf::Extra_identity_list::const_iterator>
    extra_range = symtab->icf()->extra_identity_list().equal_range(secn);
  for (Icf::Extra_identity_list::const_iterator it_ext = extra_range.first;
       it_ext != extra_range.second; ++it_ext)
    {
      std::string external_fixed;
      std::string external_all =
	get_section_contents(first_iteration, &external_fixed,
			     it_ext->second.section, self_secn,
			     num_tracked_relocs, symtab,
			     kept_section_id, it_ext->second.offset,
			     it_ext->second.offset + it_ext->second.length);
      buffer.append(external_fixed);
      icf_reloc_buffer.append(external_all, external_fixed.length(),
			      std::string::npos);
    }

  if (first_iteration)
    {
      // Store the section contents that don't change to avoid recomputing
      // during the next call to this function.
      *fixed_cache = buffer;
    }
  else
    {
      gold_assert(buffer.empty());

      // Reuse the contents computed in the previous iteration.
      buffer.append(*fixed_cache);
    }

  buffer.append(icf_reloc_buffer);
  return buffer;
}

// This function computes a checksum on each section to detect and form
// groups of identical sections.  The first iteration does this for all 
// sections.
// Further iterations do this only for the kept sections from each group to
// determine if larger groups of identical sections could be formed.  The
// first section in each group is the kept section for that group.
//
// CRC32 is the checksumming algorithm and can have collisions.  That is,
// two sections with different contents can have the same checksum. Hence,
// a multimap is used to maintain more than one group of checksum
// identical sections.  A section is added to a group only after its
// contents are explicitly compared with the kept section of the group.
//
// Parameters  :
// ITERATION_NUM           : Invocation instance of this function.
// NUM_TRACKED_RELOCS : Vector reference to store the number of relocs
//                      to ICF sections.
// KEPT_SECTION_ID    : Vector which maps folded sections to kept sections.
// ID_SECTION         : Vector mapping a section to an unique integer.
// IS_SECN_OR_GROUP_UNIQUE : To check if a section or a group of identical
//                            sections is already known to be unique.
// SECTION_CONTENTS   : Store the section's text and relocs to non-ICF
//                      sections.

static bool
match_sections(unsigned int iteration_num,
               Symbol_table* symtab,
               std::vector<unsigned int>* num_tracked_relocs,
               std::vector<unsigned int>* kept_section_id,
               const std::vector<Section_id>& id_section,
	       const std::vector<uint64_t>& section_addraligns,
               std::vector<bool>* is_secn_or_group_unique,
               std::vector<std::string>* section_contents)
{
  Unordered_multimap<uint32_t, unsigned int> section_cksum;
  std::pair<Unordered_multimap<uint32_t, unsigned int>::iterator,
            Unordered_multimap<uint32_t, unsigned int>::iterator> key_range;
  bool converged = true;

  if (iteration_num == 1)
    preprocess_for_unique_sections(id_section,
                                   is_secn_or_group_unique,
                                   NULL);
  else
    preprocess_for_unique_sections(id_section,
                                   is_secn_or_group_unique,
                                   section_contents);

  std::vector<std::string> full_section_contents;

  for (unsigned int i = 0; i < id_section.size(); i++)
    {
      full_section_contents.push_back("");
      if ((*is_secn_or_group_unique)[i])
        continue;

      Section_id secn = id_section[i];

      // Lock the object so we can read from it.  This is only called
      // single-threaded from queue_middle_tasks, so it is OK to lock.
      // Unfortunately we have no way to pass in a Task token.
      const Task* dummy_task = reinterpret_cast<const Task*>(-1);
      Task_lock_obj<Object> tl(dummy_task, secn.first);

      std::string this_secn_contents;
      uint32_t cksum;
      std::string* this_secn_cache = &((*section_contents)[i]);
      if (iteration_num == 1)
        {
          unsigned int num_relocs = 0;
          this_secn_contents = get_section_contents(true, this_secn_cache,
						    secn, secn, &num_relocs,
						    symtab, (*kept_section_id));
          (*num_tracked_relocs)[i] = num_relocs;
        }
      else
        {
          if ((*kept_section_id)[i] != i)
            {
              // This section is already folded into something.
              continue;
            }
          this_secn_contents = get_section_contents(false, this_secn_cache,
						    secn, secn, NULL,
						    symtab, (*kept_section_id));
        }

      const unsigned char* this_secn_contents_array =
            reinterpret_cast<const unsigned char*>(this_secn_contents.c_str());
      cksum = xcrc32(this_secn_contents_array, this_secn_contents.length(),
                     0xffffffff);
      size_t count = section_cksum.count(cksum);

      if (count == 0)
        {
          // Start a group with this cksum.
          section_cksum.insert(std::make_pair(cksum, i));
          full_section_contents[i] = this_secn_contents;
        }
      else
        {
          key_range = section_cksum.equal_range(cksum);
          Unordered_multimap<uint32_t, unsigned int>::iterator it;
          // Search all the groups with this cksum for a match.
          for (it = key_range.first; it != key_range.second; ++it)
            {
              unsigned int kept_section = it->second;
              if (full_section_contents[kept_section].length()
                  != this_secn_contents.length())
                  continue;
              if (memcmp(full_section_contents[kept_section].c_str(),
                         this_secn_contents.c_str(),
                         this_secn_contents.length()) != 0)
                  continue;

	      // Check section alignment here.
	      // The section with the larger alignment requirement
	      // should be kept.  We assume alignment can only be 
	      // zero or positive integral powers of two.
	      uint64_t align_i = section_addraligns[i];
	      uint64_t align_kept = section_addraligns[kept_section];
	      if (align_i <= align_kept)
		{
		  (*kept_section_id)[i] = kept_section;
		}
	      else
		{
		  (*kept_section_id)[kept_section] = i;
		  it->second = i;
		  full_section_contents[kept_section].swap(
		      full_section_contents[i]);
		}

              converged = false;
              break;
            }
          if (it == key_range.second)
            {
              // Create a new group for this cksum.
              section_cksum.insert(std::make_pair(cksum, i));
              full_section_contents[i] = this_secn_contents;
            }
        }
      // If there are no relocs to foldable sections do not process
      // this section any further.
      if (iteration_num == 1 && (*num_tracked_relocs)[i] == 0)
        (*is_secn_or_group_unique)[i] = true;
    }

  // If a section was folded into another section that was later folded
  // again then the former has to be updated.
  for (unsigned int i = 0; i < id_section.size(); i++)
    {
      // Find the end of the folding chain
      unsigned int kept = i;
      while ((*kept_section_id)[kept] != kept)
        {
          kept = (*kept_section_id)[kept];
        }
      // Update every element of the chain
      unsigned int current = i;
      while ((*kept_section_id)[current] != kept)
        {
          unsigned int next = (*kept_section_id)[current];
          (*kept_section_id)[current] = kept;
          current = next;
        }
    }

  return converged;
}

// During safe icf (--icf=safe), only fold functions that are ctors or dtors.
// This function returns true if the section name is that of a ctor or a dtor.

static bool
is_function_ctor_or_dtor(const std::string& section_name)
{
  const char* mangled_func_name = strrchr(section_name.c_str(), '.');
  gold_assert(mangled_func_name != NULL);
  if ((is_prefix_of("._ZN", mangled_func_name)
       || is_prefix_of("._ZZ", mangled_func_name))
      && (is_gnu_v3_mangled_ctor(mangled_func_name + 1)
          || is_gnu_v3_mangled_dtor(mangled_func_name + 1)))
    {
      return true;
    }
  return false;
}

// Iterate through the .eh_frame section that has index
// `ehframe_shndx` in `object`, adding entries to extra_identity_list_
// that will cause the contents of each FDE and its CIE to be included
// in the logical ICF identity of the function that the FDE refers to.

bool
Icf::add_ehframe_links(Relobj* object, unsigned int ehframe_shndx,
		       Reloc_info& relocs)
{
  section_size_type contents_len;
  const unsigned char* pcontents = object->section_contents(ehframe_shndx,
							    &contents_len,
							    false);
  const unsigned char* p = pcontents;
  const unsigned char* pend = pcontents + contents_len;

  Sections_reachable_info::iterator it_target = relocs.section_info.begin();
  Sections_reachable_info::iterator it_target_end = relocs.section_info.end();
  Offset_info::iterator it_offset = relocs.offset_info.begin();
  Offset_info::iterator it_offset_end = relocs.offset_info.end();

  // Maps section offset to the length of the CIE defined at that offset.
  typedef Unordered_map<section_offset_type, section_size_type> Cie_map;
  Cie_map cies;

  uint32_t (*read_swap_32)(const unsigned char*);
  if (object->is_big_endian())
    read_swap_32 = &elfcpp::Swap<32, true>::readval;
  else
    read_swap_32 = &elfcpp::Swap<32, false>::readval;

  // TODO: The logic for parsing the CIE/FDE framing is copied from
  // Eh_frame::do_add_ehframe_input_section() and might want to be
  // factored into a shared helper function.
  while (p < pend)
    {
      if (pend - p < 4)
	return false;

      unsigned int len = read_swap_32(p);
      p += 4;
      if (len == 0)
	{
	  // We should only find a zero-length entry at the end of the
	  // section.
	  if (p < pend)
	    return false;
	  break;
	}
      // We don't support a 64-bit .eh_frame.
      if (len == 0xffffffff)
	return false;
      if (static_cast<unsigned int>(pend - p) < len)
	return false;

      const unsigned char* const pentend = p + len;

      if (pend - p < 4)
	return false;

      unsigned int id = read_swap_32(p);
      p += 4;

      if (id == 0)
	{
	  // CIE.
	  cies.insert(std::make_pair(p - pcontents, len - 4));
	}
      else
	{
	  // FDE.
	  Cie_map::const_iterator it;
	  it = cies.find((p - pcontents) - (id - 4));
	  if (it == cies.end())
	    return false;

	  // Figure out which section this FDE refers into. The word at `p`
	  // is an address, and we expect to see a relocation there. If not,
	  // this FDE isn't ICF-relevant.
	  while (it_offset != it_offset_end
		 && it_target != it_target_end
		 && static_cast<ptrdiff_t>(*it_offset) < (p - pcontents))
	    {
	      ++it_offset;
	      ++it_target;
	    }
	  if (it_offset != it_offset_end
	      && it_target != it_target_end
	      && static_cast<ptrdiff_t>(*it_offset) == (p - pcontents))
	    {
	      // Found a reloc. Add this FDE and its CIE as extra identity
	      // info for the section it refers to.
	      Extra_identity_info rec_fde = {Section_id(object, ehframe_shndx),
					     p - pcontents, len - 4};
	      Extra_identity_info rec_cie = {Section_id(object, ehframe_shndx),
					     it->first, it->second};
	      extra_identity_list_.insert(std::make_pair(*it_target, rec_fde));
	      extra_identity_list_.insert(std::make_pair(*it_target, rec_cie));
	    }
	}

      p = pentend;
    }

  return true;
}

// This is the main ICF function called in gold.cc.  This does the
// initialization and calls match_sections repeatedly (thrice by default)
// which computes the crc checksums and detects identical functions.

void
Icf::find_identical_sections(const Input_objects* input_objects,
                             Symbol_table* symtab)
{
  unsigned int section_num = 0;
  std::vector<unsigned int> num_tracked_relocs;
  std::vector<uint64_t> section_addraligns;
  std::vector<bool> is_secn_or_group_unique;
  std::vector<std::string> section_contents;
  const Target& target = parameters->target();

  // Decide which sections are possible candidates first.

  for (Input_objects::Relobj_iterator p = input_objects->relobj_begin();
       p != input_objects->relobj_end();
       ++p)
    {
      // Lock the object so we can read from it.  This is only called
      // single-threaded from queue_middle_tasks, so it is OK to lock.
      // Unfortunately we have no way to pass in a Task token.
      const Task* dummy_task = reinterpret_cast<const Task*>(-1);
      Task_lock_obj<Object> tl(dummy_task, *p);
      std::vector<unsigned int> eh_frame_ind;

      for (unsigned int i = 0; i < (*p)->shnum(); ++i)
        {
          if ((*p)->section_size(i) == 0)
            continue;
	  const std::string section_name = (*p)->section_name(i);
          if (!is_section_foldable_candidate(section_name))
	    {
	      if (is_prefix_of(".eh_frame", section_name.c_str()))
		eh_frame_ind.push_back(i);
	      continue;
	    }

          if (!(*p)->is_section_included(i))
            continue;
          if (parameters->options().gc_sections()
              && symtab->gc()->is_section_garbage(*p, i))
              continue;
	  // With --icf=safe, check if the mangled function name is a ctor
	  // or a dtor.  The mangled function name can be obtained from the
	  // section name by stripping the section prefix.
	  if (parameters->options().icf_safe_folding()
              && !is_function_ctor_or_dtor(section_name)
	      && (!target.can_check_for_function_pointers()
                  || section_has_function_pointers(*p, i)))
            {
	      continue;
            }
          this->id_section_.push_back(Section_id(*p, i));
          this->section_id_[Section_id(*p, i)] = section_num;
          this->kept_section_id_.push_back(section_num);
          num_tracked_relocs.push_back(0);
	  section_addraligns.push_back((*p)->section_addralign(i));
          is_secn_or_group_unique.push_back(false);
          section_contents.push_back("");
          section_num++;
        }

      for (std::vector<unsigned int>::iterator it_eh_ind = eh_frame_ind.begin();
	   it_eh_ind != eh_frame_ind.end(); ++it_eh_ind)
	{
	  // gc_process_relocs() recorded relocations for this
	  // section even though we can't fold it. We need to
	  // use those relocations to associate other foldable
	  // sections with the FDEs and CIEs that are relevant
	  // to them, so we can avoid merging sections that
	  // don't have identical exception-handling behavior.

	  Section_id sect(*p, *it_eh_ind);
	  Reloc_info_list::iterator it_rel = this->reloc_info_list().find(sect);
	  if (it_rel != this->reloc_info_list().end())
	    {
	      if (!add_ehframe_links(*p, *it_eh_ind, it_rel->second))
		{
		  gold_warning(_("could not parse eh_frame section %s(%s); ICF "
				 "might not preserve exception handling "
				 "behavior"),
			       (*p)->name().c_str(),
			       (*p)->section_name(*it_eh_ind).c_str());
		}
	    }
	}
    }

  unsigned int num_iterations = 0;

  // Default number of iterations to run ICF is 3.
  unsigned int max_iterations = (parameters->options().icf_iterations() > 0)
                            ? parameters->options().icf_iterations()
                            : 3;

  bool converged = false;

  while (!converged && (num_iterations < max_iterations))
    {
      num_iterations++;
      converged = match_sections(num_iterations, symtab,
                                 &num_tracked_relocs, &this->kept_section_id_,
                                 this->id_section_, section_addraligns,
                                 &is_secn_or_group_unique, &section_contents);
    }

  if (parameters->options().print_icf_sections())
    {
      if (converged)
        gold_info(_("%s: ICF Converged after %u iteration(s)"),
                  program_name, num_iterations);
      else
        gold_info(_("%s: ICF stopped after %u iteration(s)"),
                  program_name, num_iterations);
    }

  // Unfold --keep-unique symbols.
  for (options::String_set::const_iterator p =
	 parameters->options().keep_unique_begin();
       p != parameters->options().keep_unique_end();
       ++p)
    {
      const char* name = p->c_str();
      Symbol* sym = symtab->lookup(name);
      if (sym == NULL)
	{
	  gold_warning(_("Could not find symbol %s to unfold\n"), name);
	}
      else if (sym->source() == Symbol::FROM_OBJECT 
               && !sym->object()->is_dynamic())
        {
          Relobj* obj = static_cast<Relobj*>(sym->object());
          bool is_ordinary;
          unsigned int shndx = sym->shndx(&is_ordinary);
          if (is_ordinary)
            {
	      this->unfold_section(obj, shndx);
            }
        }

    }

  this->icf_ready();
}

// Unfolds the section denoted by OBJ and SHNDX if folded.

void
Icf::unfold_section(Relobj* obj, unsigned int shndx)
{
  Section_id secn(obj, shndx);
  Uniq_secn_id_map::iterator it = this->section_id_.find(secn);
  if (it == this->section_id_.end())
    return;
  unsigned int section_num = it->second;
  unsigned int kept_section_id = this->kept_section_id_[section_num];
  if (kept_section_id != section_num)
    this->kept_section_id_[section_num] = section_num;
}

// This function determines if the section corresponding to the
// given object and index is folded based on if the kept section
// is different from this section.

bool
Icf::is_section_folded(Relobj* obj, unsigned int shndx)
{
  Section_id secn(obj, shndx);
  Uniq_secn_id_map::iterator it = this->section_id_.find(secn);
  if (it == this->section_id_.end())
    return false;
  unsigned int section_num = it->second;
  unsigned int kept_section_id = this->kept_section_id_[section_num];
  return kept_section_id != section_num;
}

// This function returns the folded section for the given section.

Section_id
Icf::get_folded_section(Relobj* dup_obj, unsigned int dup_shndx)
{
  Section_id dup_secn(dup_obj, dup_shndx);
  Uniq_secn_id_map::iterator it = this->section_id_.find(dup_secn);
  gold_assert(it != this->section_id_.end());
  unsigned int section_num = it->second;
  unsigned int kept_section_id = this->kept_section_id_[section_num];
  Section_id folded_section = this->id_section_[kept_section_id];
  return folded_section;
}

} // End of namespace gold.
