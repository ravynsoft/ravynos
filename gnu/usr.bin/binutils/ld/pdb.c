/* Support for generating PDB CodeView debugging files.
   Copyright (C) 2022-2023 Free Software Foundation, Inc.

   This file is part of the GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "pdb.h"
#include "bfdlink.h"
#include "ld.h"
#include "ldmain.h"
#include "ldmisc.h"
#include "libbfd.h"
#include "libiberty.h"
#include "coff/i386.h"
#include "coff/external.h"
#include "coff/internal.h"
#include "coff/pe.h"
#include "libcoff.h"
#include <time.h>

struct public
{
  struct public *next;
  uint32_t offset;
  uint32_t hash;
  unsigned int index;
  uint16_t section;
  uint32_t address;
};

struct string
{
  struct string *next;
  uint32_t hash;
  uint32_t offset;
  uint32_t source_file_offset;
  size_t len;
  char s[];
};

struct string_table
{
  struct string *strings_head;
  struct string *strings_tail;
  uint32_t strings_len;
  htab_t hashmap;
};

struct mod_source_files
{
  uint16_t files_count;
  struct string **files;
};

struct source_files_info
{
  uint16_t mod_count;
  struct mod_source_files *mods;
};

struct type_entry
{
  struct type_entry *next;
  uint32_t index;
  uint32_t cv_hash;
  bool has_udt_src_line;
  uint8_t data[];
};

struct types
{
  htab_t hashmap;
  uint32_t num_types;
  struct type_entry *first;
  struct type_entry *last;
};

struct global
{
  struct global *next;
  uint32_t offset;
  uint32_t hash;
  uint32_t refcount;
  unsigned int index;
  uint8_t data[];
};

struct globals
{
  uint32_t num_entries;
  struct global *first;
  struct global *last;
  htab_t hashmap;
};

struct in_sc
{
  asection *s;
  uint16_t sect_num;
  uint16_t mod_index;
};

static const uint32_t crc_table[] =
{
  0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
  0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
  0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
  0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
  0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
  0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
  0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
  0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
  0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
  0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
  0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
  0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
  0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
  0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
  0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
  0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
  0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
  0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
  0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
  0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
  0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
  0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
  0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
  0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
  0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
  0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
  0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
  0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
  0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
  0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
  0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
  0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
  0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
  0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
  0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
  0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
  0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
  0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
  0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
  0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
  0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
  0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
  0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

/* Add a new stream to the PDB archive, and return its BFD.  */
static bfd *
add_stream (bfd *pdb, const char *name, uint16_t *stream_num)
{
  bfd *stream;
  uint16_t num;

  stream = bfd_create (name ? name : "", pdb);
  if (!stream)
    return NULL;

  if (!bfd_make_writable (stream))
    {
      bfd_close (stream);
      return false;
    }

  if (!pdb->archive_head)
    {
      bfd_set_archive_head (pdb, stream);
      num = 0;
    }
  else
    {
      bfd *b = pdb->archive_head;

      num = 1;

      while (b->archive_next)
	{
	  num++;
	  b = b->archive_next;
	}

      b->archive_next = stream;
    }

  if (stream_num)
    *stream_num = num;

  return stream;
}

/* Stream 0 ought to be a copy of the MSF directory from the last
   time the PDB file was written.  Because we don't do incremental
   writes this isn't applicable to us, but we fill it with a dummy
   value so as not to confuse radare.  */
static bool
create_old_directory_stream (bfd *pdb)
{
  bfd *stream;
  char buf[sizeof (uint32_t)];

  stream = add_stream (pdb, NULL, NULL);
  if (!stream)
    return false;

  bfd_putl32 (0, buf);

  return bfd_bwrite (buf, sizeof (uint32_t), stream) == sizeof (uint32_t);
}

/* Calculate the hash of a given string.  */
static uint32_t
calc_hash (const char *data, size_t len)
{
  uint32_t hash = 0;

  while (len >= 4)
    {
      hash ^= data[0];
      hash ^= data[1] << 8;
      hash ^= data[2] << 16;
      hash ^= data[3] << 24;

      data += 4;
      len -= 4;
    }

  if (len >= 2)
    {
      hash ^= data[0];
      hash ^= data[1] << 8;

      data += 2;
      len -= 2;
    }

  if (len != 0)
    hash ^= *data;

  hash |= 0x20202020;
  hash ^= (hash >> 11);

  return hash ^ (hash >> 16);
}

/* Stream 1 is the PDB info stream - see
   https://llvm.org/docs/PDB/PdbStream.html.  */
static bool
populate_info_stream (bfd *pdb, bfd *info_stream, const unsigned char *guid)
{
  bool ret = false;
  struct pdb_stream_70 h;
  uint32_t num_entries, num_buckets;
  uint32_t names_length, stream_num;
  char int_buf[sizeof (uint32_t)];

  struct hash_entry
  {
    uint32_t offset;
    uint32_t value;
  };

  struct hash_entry **buckets = NULL;

  /* Write header.  */

  bfd_putl32 (PDB_STREAM_VERSION_VC70, &h.version);
  bfd_putl32 (time (NULL), &h.signature);
  bfd_putl32 (1, &h.age);

  bfd_putl32 (bfd_getb32 (guid), h.guid);
  bfd_putl16 (bfd_getb16 (&guid[4]), &h.guid[4]);
  bfd_putl16 (bfd_getb16 (&guid[6]), &h.guid[6]);
  memcpy (&h.guid[8], &guid[8], 8);

  if (bfd_bwrite (&h, sizeof (h), info_stream) != sizeof (h))
    return false;

  /* Write hash list of named streams.  This is a "rollover" hash, i.e.
     if a bucket is filled an entry gets placed in the next free
     slot.  */

  num_entries = 0;
  for (bfd *b = pdb->archive_head; b; b = b->archive_next)
    {
      if (strcmp (b->filename, ""))
	num_entries++;
    }

  num_buckets = num_entries * 2;

  names_length = 0;
  stream_num = 0;

  if (num_buckets > 0)
    {
      buckets = xmalloc (sizeof (struct hash_entry *) * num_buckets);
      memset (buckets, 0, sizeof (struct hash_entry *) * num_buckets);

      for (bfd *b = pdb->archive_head; b; b = b->archive_next)
	{
	  if (strcmp (b->filename, ""))
	    {
	      size_t len = strlen (b->filename);
	      uint32_t hash = (uint16_t) calc_hash (b->filename, len);
	      uint32_t bucket_num = hash % num_buckets;

	      while (buckets[bucket_num])
		{
		  bucket_num++;

		  if (bucket_num == num_buckets)
		    bucket_num = 0;
		}

	      buckets[bucket_num] = xmalloc (sizeof (struct hash_entry));

	      buckets[bucket_num]->offset = names_length;
	      buckets[bucket_num]->value = stream_num;

	      names_length += len + 1;
	    }

	  stream_num++;
	}
    }

  /* Write the strings list - the hash keys are indexes into this.  */

  bfd_putl32 (names_length, int_buf);

  if (bfd_bwrite (int_buf, sizeof (uint32_t), info_stream) !=
      sizeof (uint32_t))
    goto end;

  for (bfd *b = pdb->archive_head; b; b = b->archive_next)
    {
      if (!strcmp (b->filename, ""))
	continue;

      size_t len = strlen (b->filename) + 1;

      if (bfd_bwrite (b->filename, len, info_stream) != len)
	goto end;
    }

  /* Write the number of entries and buckets.  */

  bfd_putl32 (num_entries, int_buf);

  if (bfd_bwrite (int_buf, sizeof (uint32_t), info_stream) !=
      sizeof (uint32_t))
    goto end;

  bfd_putl32 (num_buckets, int_buf);

  if (bfd_bwrite (int_buf, sizeof (uint32_t), info_stream) !=
      sizeof (uint32_t))
    goto end;

  /* Write the present bitmap.  */

  bfd_putl32 ((num_buckets + 31) / 32, int_buf);

  if (bfd_bwrite (int_buf, sizeof (uint32_t), info_stream) !=
      sizeof (uint32_t))
    goto end;

  for (unsigned int i = 0; i < num_buckets; i += 32)
    {
      uint32_t v = 0;

      for (unsigned int j = 0; j < 32; j++)
	{
	  if (i + j >= num_buckets)
	    break;

	  if (buckets[i + j])
	    v |= 1 << j;
	}

      bfd_putl32 (v, int_buf);

      if (bfd_bwrite (int_buf, sizeof (uint32_t), info_stream) !=
	  sizeof (uint32_t))
	goto end;
    }

  /* Write the (empty) deleted bitmap.  */

  bfd_putl32 (0, int_buf);

  if (bfd_bwrite (int_buf, sizeof (uint32_t), info_stream) !=
      sizeof (uint32_t))
    goto end;

  /* Write the buckets.  */

  for (unsigned int i = 0; i < num_buckets; i++)
    {
      if (buckets[i])
	{
	  bfd_putl32 (buckets[i]->offset, int_buf);

	  if (bfd_bwrite (int_buf, sizeof (uint32_t), info_stream) !=
	      sizeof (uint32_t))
	    goto end;

	  bfd_putl32 (buckets[i]->value, int_buf);

	  if (bfd_bwrite (int_buf, sizeof (uint32_t), info_stream) !=
	      sizeof (uint32_t))
	    goto end;
	}
    }

  bfd_putl32 (0, int_buf);

  if (bfd_bwrite (int_buf, sizeof (uint32_t), info_stream) !=
      sizeof (uint32_t))
    goto end;

  bfd_putl32 (PDB_STREAM_VERSION_VC140, int_buf);

  if (bfd_bwrite (int_buf, sizeof (uint32_t), info_stream) !=
      sizeof (uint32_t))
    goto end;

  ret = true;

end:
  for (unsigned int i = 0; i < num_buckets; i++)
    {
      if (buckets[i])
	free (buckets[i]);
    }

  free (buckets);

  return ret;
}

/* Calculate the CRC32 used for type hashes.  */
static uint32_t
crc32 (const uint8_t *data, size_t len)
{
  uint32_t crc = 0;

  while (len > 0)
    {
      crc = (crc >> 8) ^ crc_table[(crc & 0xff) ^ *data];

      data++;
      len--;
    }

  return crc;
}

/* Stream 2 is the type information (TPI) stream, and stream 4 is
   the ID information (IPI) stream.  They differ only in which records
   go in which stream. */
static bool
populate_type_stream (bfd *pdb, bfd *stream, struct types *types)
{
  struct pdb_tpi_stream_header h;
  struct type_entry *e;
  uint32_t len = 0, index_offset_len, off;
  struct bfd *hash_stream = NULL;
  uint16_t hash_stream_index;

  static const uint32_t index_skip = 0x2000;

  e = types->first;

  index_offset_len = 0;

  while (e)
    {
      uint32_t old_len = len;

      len += sizeof (uint16_t) + bfd_getl16 (e->data);

      if (old_len == 0 || old_len / index_skip != len / index_skip)
	index_offset_len += sizeof (uint32_t) * 2;

      e = e->next;
    }

  /* Each type stream also has a stream which holds the hash value for each
     type, along with a skip list to speed up searching.  */

  hash_stream = add_stream (pdb, "", &hash_stream_index);

  if (!hash_stream)
    return false;

  bfd_putl32 (TPI_STREAM_VERSION_80, &h.version);
  bfd_putl32 (sizeof (h), &h.header_size);
  bfd_putl32 (TPI_FIRST_INDEX, &h.type_index_begin);
  bfd_putl32 (TPI_FIRST_INDEX + types->num_types, &h.type_index_end);
  bfd_putl32 (len, &h.type_record_bytes);
  bfd_putl16 (hash_stream_index, &h.hash_stream_index);
  bfd_putl16 (0xffff, &h.hash_aux_stream_index);
  bfd_putl32 (sizeof (uint32_t), &h.hash_key_size);
  bfd_putl32 (NUM_TPI_HASH_BUCKETS, &h.num_hash_buckets);
  bfd_putl32 (0, &h.hash_value_buffer_offset);
  bfd_putl32 (types->num_types * sizeof (uint32_t),
	      &h.hash_value_buffer_length);
  bfd_putl32 (types->num_types * sizeof (uint32_t),
	      &h.index_offset_buffer_offset);
  bfd_putl32 (index_offset_len, &h.index_offset_buffer_length);
  bfd_putl32 ((types->num_types * sizeof (uint32_t)) + index_offset_len,
	      &h.hash_adj_buffer_offset);
  bfd_putl32 (0, &h.hash_adj_buffer_length);

  if (bfd_bwrite (&h, sizeof (h), stream) != sizeof (h))
    return false;

  /* Write the type definitions into the main stream, and the hashes
     into the hash stream.  The hashes have already been calculated
     in handle_type.  */

  e = types->first;

  while (e)
    {
      uint8_t buf[sizeof (uint32_t)];
      uint16_t size;

      size = bfd_getl16 (e->data);

      if (bfd_bwrite (e->data, size + sizeof (uint16_t), stream)
	  != size + sizeof (uint16_t))
	return false;

      bfd_putl32 (e->cv_hash % NUM_TPI_HASH_BUCKETS, buf);

      if (bfd_bwrite (buf, sizeof (uint32_t), hash_stream)
	  != sizeof (uint32_t))
	return false;

      e = e->next;
    }

  /* Write the index offsets, i.e. the skip list, into the hash stream.  We
     copy MSVC here by writing a new entry for every 8192 bytes.  */

  e = types->first;
  off = 0;

  while (e)
    {
      uint32_t old_off = off;
      uint16_t size = bfd_getl16 (e->data);

      off += size + sizeof (uint16_t);

      if (old_off == 0 || old_off / index_skip != len / index_skip)
	{
	  uint8_t buf[sizeof (uint32_t)];

	  bfd_putl32 (TPI_FIRST_INDEX + e->index, buf);

	  if (bfd_bwrite (buf, sizeof (uint32_t), hash_stream)
	      != sizeof (uint32_t))
	    return false;

	  bfd_putl32 (old_off, buf);

	  if (bfd_bwrite (buf, sizeof (uint32_t), hash_stream)
	      != sizeof (uint32_t))
	    return false;
	}

      e = e->next;
    }

  return true;
}

/* Return the PE architecture number for the image.  */
static uint16_t
get_arch_number (bfd *abfd)
{
  switch (abfd->arch_info->arch)
    {
    case bfd_arch_i386:
      if (abfd->arch_info->mach & bfd_mach_x86_64)
	return IMAGE_FILE_MACHINE_AMD64;
      else
	return IMAGE_FILE_MACHINE_I386;

    case bfd_arch_aarch64:
      return IMAGE_FILE_MACHINE_ARM64;

    default:
      return 0;
    }
}

/* Validate the DEBUG_S_FILECHKSMS entry within a module's .debug$S
   section, and copy it to the module's symbol stream.  */
static bool
copy_filechksms (uint8_t *data, uint32_t size, char *string_table,
		 struct string_table *strings, uint8_t *out,
		 struct mod_source_files *mod_source)
{
  uint8_t *orig_data = data;
  uint32_t orig_size = size;
  uint16_t num_files = 0;
  struct string **strptr;

  bfd_putl32 (DEBUG_S_FILECHKSMS, out);
  out += sizeof (uint32_t);

  bfd_putl32 (size, out);
  out += sizeof (uint32_t);

  /* Calculate the number of files, and check for any overflows.  */

  while (size > 0)
    {
      struct file_checksum *fc = (struct file_checksum *) data;
      uint8_t padding;
      size_t len;

      if (size < sizeof (struct file_checksum))
	{
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}

      len = sizeof (struct file_checksum) + fc->checksum_length;

      if (size < len)
	{
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}

      data += len;
      size -= len;

      if (len % sizeof (uint32_t))
	padding = sizeof (uint32_t) - (len % sizeof (uint32_t));
      else
	padding = 0;

      if (size < padding)
	{
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}

      num_files++;

      data += padding;
      size -= padding;
    }

  /* Add the files to mod_source, so that they'll appear in the source
     info substream.  */

  strptr = NULL;
  if (num_files > 0)
    {
      uint16_t new_count = num_files + mod_source->files_count;

      mod_source->files = xrealloc (mod_source->files,
				    sizeof (struct string *) * new_count);

      strptr = mod_source->files + mod_source->files_count;

      mod_source->files_count += num_files;
    }

  /* Actually copy the data.  */

  data = orig_data;
  size = orig_size;

  while (size > 0)
    {
      struct file_checksum *fc = (struct file_checksum *) data;
      uint32_t string_off;
      uint8_t padding;
      size_t len;
      struct string *str = NULL;

      string_off = bfd_getl32 (&fc->file_id);
      len = sizeof (struct file_checksum) + fc->checksum_length;

      if (len % sizeof (uint32_t))
	padding = sizeof (uint32_t) - (len % sizeof (uint32_t));
      else
	padding = 0;

      /* Remap the "file ID", i.e. the offset in the module's string table,
	 so it points to the right place in the main string table.  */

      if (string_table)
	{
	  char *fn = string_table + string_off;
	  size_t fn_len = strlen (fn);
	  uint32_t hash = calc_hash (fn, fn_len);
	  void **slot;

	  slot = htab_find_slot_with_hash (strings->hashmap, fn, hash,
					   NO_INSERT);

	  if (slot)
	    str = (struct string *) *slot;
	}

      *strptr = str;
      strptr++;

      bfd_putl32 (str ? str->offset : 0, &fc->file_id);

      memcpy (out, data, len + padding);

      data += len + padding;
      size -= len + padding;
      out += len + padding;
    }

  return true;
}

/* Add a string to the strings table, if it's not already there.  Returns its
   offset within the string table.  */
static uint32_t
add_string (char *str, size_t len, struct string_table *strings)
{
  uint32_t hash = calc_hash (str, len);
  struct string *s;
  void **slot;

  slot = htab_find_slot_with_hash (strings->hashmap, str, hash, INSERT);

  if (!*slot)
    {
      *slot = xmalloc (offsetof (struct string, s) + len);

      s = (struct string *) *slot;

      s->next = NULL;
      s->hash = hash;
      s->offset = strings->strings_len;
      s->source_file_offset = 0xffffffff;
      s->len = len;
      memcpy (s->s, str, len);

      if (strings->strings_tail)
	strings->strings_tail->next = s;
      else
	strings->strings_head = s;

      strings->strings_tail = s;

      strings->strings_len += len + 1;
    }
  else
    {
      s = (struct string *) *slot;
    }

  return s->offset;
}

/* Return the hash of an entry in the string table.  */
static hashval_t
hash_string_table_entry (const void *p)
{
  const struct string *s = (const struct string *) p;

  return s->hash;
}

/* Compare an entry in the string table with a string.  */
static int
eq_string_table_entry (const void *a, const void *b)
{
  const struct string *s1 = (const struct string *) a;
  const char *s2 = (const char *) b;
  size_t s2_len = strlen (s2);

  if (s2_len != s1->len)
    return 0;

  return memcmp (s1->s, s2, s2_len) == 0;
}

/* Parse the string table within the .debug$S section.  */
static void
parse_string_table (bfd_byte *data, size_t size,
		    struct string_table *strings)
{
  while (true)
    {
      size_t len = strnlen ((char *) data, size);

      add_string ((char *) data, len, strings);

      data += len + 1;

      if (size <= len + 1)
	break;

      size -= len + 1;
    }
}

/* Remap a type reference within a CodeView symbol.  */
static bool
remap_symbol_type (void *data, struct type_entry **map, uint32_t num_types)
{
  uint32_t type = bfd_getl32 (data);

  /* Ignore builtin types (those with IDs below 0x1000).  */
  if (type < TPI_FIRST_INDEX)
    return true;

  if (type >= TPI_FIRST_INDEX + num_types)
    {
      einfo (_("%P: CodeView symbol references out of range type %v\n"),
	       type);
      return false;
    }

  type = TPI_FIRST_INDEX + map[type - TPI_FIRST_INDEX]->index;
  bfd_putl32 (type, data);

  return true;
}

/* Add an entry into the globals stream.  If it already exists, increase
   the refcount.  */
static bool
add_globals_ref (struct globals *glob, bfd *sym_rec_stream, const char *name,
		 size_t name_len, uint8_t *data, size_t len)
{
  void **slot;
  uint32_t hash;
  struct global *g;

  slot = htab_find_slot_with_hash (glob->hashmap, data,
				   iterative_hash (data, len, 0), INSERT);

  if (*slot)
    {
      g = *slot;
      g->refcount++;
      return true;
    }

  *slot = xmalloc (offsetof (struct global, data) + len);

  hash = crc32 ((const uint8_t *) name, name_len);
  hash %= NUM_GLOBALS_HASH_BUCKETS;

  g = *slot;
  g->next = NULL;
  g->offset = bfd_tell (sym_rec_stream);
  g->hash = hash;
  g->refcount = 1;
  memcpy (g->data, data, len);

  glob->num_entries++;

  if (glob->last)
    glob->last->next = g;
  else
    glob->first = g;

  glob->last = g;

  return bfd_bwrite (data, len, sym_rec_stream) == len;
}

/* Find the end of the current scope within symbols data.  */
static uint8_t *
find_end_of_scope (uint8_t *data, uint32_t size)
{
  unsigned int scope_level = 1;
  uint16_t len;

  len = bfd_getl16 (data) + sizeof (uint16_t);

  data += len;
  size -= len;

  while (true)
    {
      uint16_t type;

      if (size < sizeof (uint32_t))
	return NULL;

      len = bfd_getl16 (data) + sizeof (uint16_t);
      type = bfd_getl16 (data + sizeof (uint16_t));

      if (size < len)
	return NULL;

      switch (type)
	{
	case S_GPROC32:
	case S_LPROC32:
	case S_BLOCK32:
	case S_INLINESITE:
	case S_THUNK32:
	  scope_level++;
	  break;

	case S_END:
	case S_PROC_ID_END:
	case S_INLINESITE_END:
	  scope_level--;

	  if (scope_level == 0)
	    return data;

	  break;
	}

      data += len;
      size -= len;
    }
}

/* Return the size of an extended value parameter, as used in
   LF_ENUMERATE etc.  */
static unsigned int
extended_value_len (uint16_t type)
{
  switch (type)
    {
    case LF_CHAR:
      return 1;

    case LF_SHORT:
    case LF_USHORT:
      return 2;

    case LF_LONG:
    case LF_ULONG:
      return 4;

    case LF_QUADWORD:
    case LF_UQUADWORD:
      return 8;
    }

  return 0;
}

/* Parse the symbols in a .debug$S section, and copy them to the module's
   symbol stream.  */
static bool
parse_symbols (uint8_t *data, uint32_t size, uint8_t **buf,
	       struct type_entry **map, uint32_t num_types,
	       bfd *sym_rec_stream, struct globals *glob, uint16_t mod_num)
{
  uint8_t *orig_buf = *buf;
  unsigned int scope_level = 0;
  uint8_t *scope = NULL;

  while (size >= sizeof (uint16_t))
    {
      uint16_t len, type;

      len = bfd_getl16 (data) + sizeof (uint16_t);

      if (len > size)
	{
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}

      type = bfd_getl16 (data + sizeof (uint16_t));

      switch (type)
	{
	case S_LDATA32:
	case S_GDATA32:
	case S_LTHREAD32:
	case S_GTHREAD32:
	  {
	    struct datasym *d = (struct datasym *) data;
	    size_t name_len;

	    if (len < offsetof (struct datasym, name))
	      {
		einfo (_("%P: warning: truncated CodeView record"
			 " S_LDATA32/S_GDATA32/S_LTHREAD32/S_GTHREAD32\n"));
		bfd_set_error (bfd_error_bad_value);
		return false;
	      }

	    if (scope_level == 0)
	      {
		uint16_t section = bfd_getl16 (&d->section);

		if (section == 0) /* GC'd, ignore */
		  break;
	      }

	    name_len =
	      strnlen (d->name, len - offsetof (struct datasym, name));

	    if (name_len == len - offsetof (struct datasym, name))
	      {
		einfo (_("%P: warning: name for S_LDATA32/S_GDATA32/"
			 "S_LTHREAD32/S_GTHREAD32 has no terminating"
			 " zero\n"));
		bfd_set_error (bfd_error_bad_value);
		return false;
	      }

	    if (!remap_symbol_type (&d->type, map, num_types))
	      {
		bfd_set_error (bfd_error_bad_value);
		return false;
	      }

	    /* If S_LDATA32 or S_LTHREAD32, copy into module symbols.  */

	    if (type == S_LDATA32 || type == S_LTHREAD32)
	      {
		memcpy (*buf, d, len);
		*buf += len;
	      }

	    /* S_LDATA32 and S_LTHREAD32 only go in globals if
	       not in function scope.  */
	    if (type == S_GDATA32 || type == S_GTHREAD32 || scope_level == 0)
	      {
		if (!add_globals_ref (glob, sym_rec_stream, d->name,
				      name_len, data, len))
		  return false;
	      }

	    break;
	  }

	case S_GPROC32:
	case S_LPROC32:
	case S_GPROC32_ID:
	case S_LPROC32_ID:
	  {
	    struct procsym *proc = (struct procsym *) data;
	    size_t name_len;
	    uint16_t section;
	    uint32_t end;
	    uint8_t *endptr;
	    size_t ref_size, padding;
	    struct refsym *ref;

	    if (len < offsetof (struct procsym, name))
	      {
		einfo (_("%P: warning: truncated CodeView record"
			 " S_GPROC32/S_LPROC32\n"));
		bfd_set_error (bfd_error_bad_value);
		return false;
	      }

	    section = bfd_getl16 (&proc->section);

	    endptr = find_end_of_scope (data, size);

	    if (!endptr)
	      {
		einfo (_("%P: warning: could not find end of"
			 " S_GPROC32/S_LPROC32 record\n"));
		bfd_set_error (bfd_error_bad_value);
		return false;
	      }

	    if (section == 0) /* skip if GC'd */
	      {
		/* Skip to after S_END.  */

		size -= endptr - data;
		data = endptr;

		len = bfd_getl16 (data) + sizeof (uint16_t);

		data += len;
		size -= len;

		continue;
	      }

	    name_len =
	      strnlen (proc->name, len - offsetof (struct procsym, name));

	    if (name_len == len - offsetof (struct procsym, name))
	      {
		einfo (_("%P: warning: name for S_GPROC32/S_LPROC32 has no"
			 " terminating zero\n"));
		bfd_set_error (bfd_error_bad_value);
		return false;
	      }

	    if (type == S_GPROC32_ID || type == S_LPROC32_ID)
	      {
		/* Transform into S_GPROC32 / S_LPROC32.  */

		uint32_t t_idx = bfd_getl32 (&proc->type);
		struct type_entry *t;
		uint16_t t_type;

		if (t_idx < TPI_FIRST_INDEX
		    || t_idx >= TPI_FIRST_INDEX + num_types)
		  {
		    einfo (_("%P: CodeView symbol references out of range"
			     " type %v\n"), type);
		    bfd_set_error (bfd_error_bad_value);
		    return false;
		  }

		t = map[t_idx - TPI_FIRST_INDEX];

		t_type = bfd_getl16 (t->data + sizeof (uint16_t));

		switch (t_type)
		  {
		  case LF_FUNC_ID:
		    {
		      struct lf_func_id *t_data =
			(struct lf_func_id *) t->data;

		      /* Replace proc->type with function type.  */

		      memcpy (&proc->type, &t_data->function_type,
			      sizeof (uint32_t));

		      break;
		    }

		  case LF_MFUNC_ID:
		    {
		      struct lf_mfunc_id *t_data =
			(struct lf_mfunc_id *) t->data;

		      /* Replace proc->type with function type.  */

		      memcpy (&proc->type, &t_data->function_type,
			      sizeof (uint32_t));

		      break;
		    }

		  default:
		    einfo (_("%P: CodeView S_GPROC32_ID/S_LPROC32_ID symbol"
			     " referenced unknown type as ID\n"));
		    bfd_set_error (bfd_error_bad_value);
		    return false;
		  }

		/* Change record type.  */

		if (type == S_GPROC32_ID)
		  bfd_putl32 (S_GPROC32, &proc->kind);
		else
		  bfd_putl32 (S_LPROC32, &proc->kind);
	      }
	    else
	      {
		if (!remap_symbol_type (&proc->type, map, num_types))
		  {
		    bfd_set_error (bfd_error_bad_value);
		    return false;
		  }
	      }

	    end = *buf - orig_buf + sizeof (uint32_t) + endptr - data;
	    bfd_putl32 (end, &proc->end);

	    /* Add S_PROCREF / S_LPROCREF to globals stream.  */

	    ref_size = offsetof (struct refsym, name) + name_len + 1;

	    if (ref_size % sizeof (uint32_t))
	      padding = sizeof (uint32_t) - (ref_size % sizeof (uint32_t));
	    else
	      padding = 0;

	    ref = xmalloc (ref_size + padding);

	    bfd_putl16 (ref_size + padding - sizeof (uint16_t), &ref->size);
	    bfd_putl16 (type == S_GPROC32 || type == S_GPROC32_ID ?
			S_PROCREF : S_LPROCREF, &ref->kind);
	    bfd_putl32 (0, &ref->sum_name);
	    bfd_putl32 (*buf - orig_buf + sizeof (uint32_t),
			&ref->symbol_offset);
	    bfd_putl16 (mod_num + 1, &ref->mod);

	    memcpy (ref->name, proc->name, name_len + 1);

	    memset (ref->name + name_len + 1, 0, padding);

	    if (!add_globals_ref (glob, sym_rec_stream, proc->name, name_len,
				  (uint8_t *) ref, ref_size + padding))
	      {
		free (ref);
		return false;
	      }

	    free (ref);

	    scope = *buf;

	    memcpy (*buf, proc, len);
	    *buf += len;

	    scope_level++;

	    break;
	  }

	case S_UDT:
	  {
	    struct udtsym *udt = (struct udtsym *) data;
	    size_t name_len;

	    if (len < offsetof (struct udtsym, name))
	      {
		einfo (_("%P: warning: truncated CodeView record"
			 " S_UDT\n"));
		bfd_set_error (bfd_error_bad_value);
		return false;
	      }

	    name_len =
	      strnlen (udt->name, len - offsetof (struct udtsym, name));

	    if (name_len == len - offsetof (struct udtsym, name))
	      {
		einfo (_("%P: warning: name for S_UDT has no"
			 " terminating zero\n"));
		bfd_set_error (bfd_error_bad_value);
		return false;
	      }

	    if (!remap_symbol_type (&udt->type, map, num_types))
	      {
		bfd_set_error (bfd_error_bad_value);
		return false;
	      }

	    /* S_UDT goes in the symbols stream if within a procedure,
	       otherwise it goes in the globals stream.  */
	    if (scope_level == 0)
	      {
		if (!add_globals_ref (glob, sym_rec_stream, udt->name,
				      name_len, data, len))
		  return false;
	      }
	    else
	      {
		memcpy (*buf, udt, len);
		*buf += len;
	      }

	    break;
	  }

	case S_CONSTANT:
	  {
	    struct constsym *c = (struct constsym *) data;
	    size_t name_len, rec_size;
	    uint16_t val;

	    if (len < offsetof (struct constsym, name))
	      {
		einfo (_("%P: warning: truncated CodeView record"
			 " S_CONSTANT\n"));
		bfd_set_error (bfd_error_bad_value);
		return false;
	      }

	    rec_size = offsetof (struct constsym, name);

	    val = bfd_getl16 (&c->value);

	    /* If val >= 0x8000, actual value follows.  */
	    if (val >= 0x8000)
	      {
		unsigned int param_len = extended_value_len (val);

		if (param_len == 0)
		  {
		    einfo (_("%P: warning: unhandled type %v within"
			     " S_CONSTANT\n"), val);
		    bfd_set_error (bfd_error_bad_value);
		    return false;
		  }

		rec_size += param_len;
	      }

	    name_len =
	      strnlen ((const char *) data + rec_size, len - rec_size);

	    if (name_len == len - rec_size)
	      {
		einfo (_("%P: warning: name for S_CONSTANT has no"
			 " terminating zero\n"));
		bfd_set_error (bfd_error_bad_value);
		return false;
	      }

	    if (!remap_symbol_type (&c->type, map, num_types))
	      {
		bfd_set_error (bfd_error_bad_value);
		return false;
	      }

	    if (!add_globals_ref (glob, sym_rec_stream,
				  (const char *) data + rec_size, name_len,
				  data, len))
	      return false;

	    break;
	  }

	case S_END:
	case S_INLINESITE_END:
	case S_PROC_ID_END:
	  memcpy (*buf, data, len);

	  if (type == S_PROC_ID_END) /* transform to S_END */
	    bfd_putl16 (S_END, *buf + sizeof (uint16_t));

	  /* Reset scope variable back to the address of the previous
	     scope start.  */
	  if (scope)
	    {
	      uint32_t parent;
	      uint16_t scope_start_type =
		bfd_getl16 (scope + sizeof (uint16_t));

	      switch (scope_start_type)
		{
		case S_GPROC32:
		case S_LPROC32:
		  parent = bfd_getl32 (scope + offsetof (struct procsym,
							 parent));
		  break;

		case S_BLOCK32:
		  parent = bfd_getl32 (scope + offsetof (struct blocksym,
							 parent));
		  break;

		case S_INLINESITE:
		  parent = bfd_getl32 (scope + offsetof (struct inline_site,
							 parent));
		  break;

		case S_THUNK32:
		  parent = bfd_getl32 (scope + offsetof (struct thunk,
							 parent));
		  break;

		default:
		  einfo (_("%P: warning: unexpected CodeView scope start"
			   " record %v\n"), scope_start_type);
		  bfd_set_error (bfd_error_bad_value);
		  return false;
		}

	      if (parent == 0)
		scope = NULL;
	      else
		scope = orig_buf + parent - sizeof (uint32_t);
	    }

	  *buf += len;
	  scope_level--;
	  break;

	case S_BUILDINFO:
	  {
	    struct buildinfosym *bi = (struct buildinfosym *) data;

	    if (len < sizeof (struct buildinfosym))
	      {
		einfo (_("%P: warning: truncated CodeView record"
			 " S_BUILDINFO\n"));
		bfd_set_error (bfd_error_bad_value);
		return false;
	      }

	    if (!remap_symbol_type (&bi->type, map, num_types))
	      {
		bfd_set_error (bfd_error_bad_value);
		return false;
	      }

	    memcpy (*buf, data, len);
	    *buf += len;

	    break;
	  }

	case S_BLOCK32:
	  {
	    struct blocksym *bl = (struct blocksym *) data;
	    uint8_t *endptr;
	    uint32_t end;

	    if (len < offsetof (struct blocksym, name))
	      {
		einfo (_("%P: warning: truncated CodeView record"
			 " S_BLOCK32\n"));
		bfd_set_error (bfd_error_bad_value);
		return false;
	      }

	    bfd_putl32 (scope - orig_buf + sizeof (uint32_t), &bl->parent);

	    endptr = find_end_of_scope (data, size);

	    if (!endptr)
	      {
		einfo (_("%P: warning: could not find end of"
			 " S_BLOCK32 record\n"));
		bfd_set_error (bfd_error_bad_value);
		return false;
	      }

	    end = *buf - orig_buf + sizeof (uint32_t) + endptr - data;
	    bfd_putl32 (end, &bl->end);

	    scope = *buf;

	    memcpy (*buf, data, len);
	    *buf += len;

	    scope_level++;

	    break;
	  }

	case S_BPREL32:
	  {
	    struct bprelsym *bp = (struct bprelsym *) data;

	    if (len < offsetof (struct bprelsym, name))
	      {
		einfo (_("%P: warning: truncated CodeView record"
			 " S_BPREL32\n"));
		bfd_set_error (bfd_error_bad_value);
		return false;
	      }

	    if (!remap_symbol_type (&bp->type, map, num_types))
	      {
		bfd_set_error (bfd_error_bad_value);
		return false;
	      }

	    memcpy (*buf, data, len);
	    *buf += len;

	    break;
	  }

	case S_REGISTER:
	  {
	    struct regsym *reg = (struct regsym *) data;

	    if (len < offsetof (struct regsym, name))
	      {
		einfo (_("%P: warning: truncated CodeView record"
			 " S_REGISTER\n"));
		bfd_set_error (bfd_error_bad_value);
		return false;
	      }

	    if (!remap_symbol_type (&reg->type, map, num_types))
	      {
		bfd_set_error (bfd_error_bad_value);
		return false;
	      }

	    memcpy (*buf, data, len);
	    *buf += len;

	    break;
	  }

	case S_REGREL32:
	  {
	    struct regrel *rr = (struct regrel *) data;

	    if (len < offsetof (struct regrel, name))
	      {
		einfo (_("%P: warning: truncated CodeView record"
			 " S_REGREL32\n"));
		bfd_set_error (bfd_error_bad_value);
		return false;
	      }

	    if (!remap_symbol_type (&rr->type, map, num_types))
	      {
		bfd_set_error (bfd_error_bad_value);
		return false;
	      }

	    memcpy (*buf, data, len);
	    *buf += len;

	    break;
	  }

	case S_LOCAL:
	  {
	    struct localsym *l = (struct localsym *) data;

	    if (len < offsetof (struct localsym, name))
	      {
		einfo (_("%P: warning: truncated CodeView record"
			 " S_LOCAL\n"));
		bfd_set_error (bfd_error_bad_value);
		return false;
	      }

	    if (!remap_symbol_type (&l->type, map, num_types))
	      {
		bfd_set_error (bfd_error_bad_value);
		return false;
	      }

	    memcpy (*buf, data, len);
	    *buf += len;

	    break;
	  }

	case S_INLINESITE:
	  {
	    struct inline_site *is = (struct inline_site *) data;
	    uint8_t *endptr;
	    uint32_t end;

	    if (len < offsetof (struct inline_site, binary_annotations))
	      {
		einfo (_("%P: warning: truncated CodeView record"
			 " S_INLINESITE\n"));
		bfd_set_error (bfd_error_bad_value);
		return false;
	      }

	    bfd_putl32 (scope - orig_buf + sizeof (uint32_t), &is->parent);

	    endptr = find_end_of_scope (data, size);

	    if (!endptr)
	      {
		einfo (_("%P: warning: could not find end of"
			 " S_INLINESITE record\n"));
		bfd_set_error (bfd_error_bad_value);
		return false;
	      }

	    end = *buf - orig_buf + sizeof (uint32_t) + endptr - data;
	    bfd_putl32 (end, &is->end);

	    if (!remap_symbol_type (&is->inlinee, map, num_types))
	      {
		bfd_set_error (bfd_error_bad_value);
		return false;
	      }

	    scope = *buf;

	    memcpy (*buf, data, len);
	    *buf += len;

	    scope_level++;

	    break;
	  }

	case S_THUNK32:
	  {
	    struct thunk *th = (struct thunk *) data;
	    uint8_t *endptr;
	    uint32_t end;

	    if (len < offsetof (struct thunk, name))
	      {
		einfo (_("%P: warning: truncated CodeView record"
			 " S_THUNK32\n"));
		bfd_set_error (bfd_error_bad_value);
		return false;
	      }

	    bfd_putl32 (scope - orig_buf + sizeof (uint32_t), &th->parent);

	    endptr = find_end_of_scope (data, size);

	    if (!endptr)
	      {
		einfo (_("%P: warning: could not find end of"
			 " S_THUNK32 record\n"));
		bfd_set_error (bfd_error_bad_value);
		return false;
	      }

	    end = *buf - orig_buf + sizeof (uint32_t) + endptr - data;
	    bfd_putl32 (end, &th->end);

	    scope = *buf;

	    memcpy (*buf, data, len);
	    *buf += len;

	    scope_level++;

	    break;
	  }

	case S_HEAPALLOCSITE:
	  {
	    struct heap_alloc_site *has = (struct heap_alloc_site *) data;

	    if (len < sizeof (struct heap_alloc_site))
	      {
		einfo (_("%P: warning: truncated CodeView record"
			 " S_HEAPALLOCSITE\n"));
		bfd_set_error (bfd_error_bad_value);
		return false;
	      }

	    if (!remap_symbol_type (&has->type, map, num_types))
	      {
		bfd_set_error (bfd_error_bad_value);
		return false;
	      }

	    memcpy (*buf, data, len);
	    *buf += len;

	    break;
	  }

	case S_OBJNAME: /* just copy */
	case S_COMPILE3:
	case S_UNAMESPACE:
	case S_FRAMEPROC:
	case S_FRAMECOOKIE:
	case S_LABEL32:
	case S_DEFRANGE_REGISTER_REL:
	case S_DEFRANGE_FRAMEPOINTER_REL:
	case S_DEFRANGE_SUBFIELD_REGISTER:
	case S_DEFRANGE_FRAMEPOINTER_REL_FULL_SCOPE:
	case S_DEFRANGE_REGISTER:
	  memcpy (*buf, data, len);
	  *buf += len;
	  break;

	default:
	  einfo (_("%P: warning: unrecognized CodeView record %v\n"), type);
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}

      data += len;
      size -= len;
    }

  return true;
}

/* For a given symbol subsection, work out how much space to allocate in the
   result module stream.  This is different because we don't copy certain
   symbols, such as S_CONSTANT, and we skip over any procedures or data that
   have been GC'd out.  */
static bool
calculate_symbols_size (uint8_t *data, uint32_t size, uint32_t *sym_size)
{
  unsigned int scope_level = 0;

  while (size >= sizeof (uint32_t))
    {
      uint16_t len = bfd_getl16 (data) + sizeof (uint16_t);
      uint16_t type = bfd_getl16 (data + sizeof (uint16_t));

      switch (type)
	{
	case S_LDATA32:
	case S_LTHREAD32:
	  {
	    struct datasym *d = (struct datasym *) data;
	    uint16_t section;

	    if (len < offsetof (struct datasym, name))
	      {
		einfo (_("%P: warning: truncated CodeView record"
			 " S_LDATA32/S_LTHREAD32\n"));
		return false;
	      }

	    section = bfd_getl16 (&d->section);

	    /* copy if not GC'd or within function */
	    if (scope_level != 0 || section != 0)
	      *sym_size += len;
	  }

	case S_GDATA32:
	case S_GTHREAD32:
	case S_CONSTANT:
	  /* Not copied into symbols stream.  */
	  break;

	case S_GPROC32:
	case S_LPROC32:
	case S_GPROC32_ID:
	case S_LPROC32_ID:
	  {
	    struct procsym *proc = (struct procsym *) data;
	    uint16_t section;

	    if (len < offsetof (struct procsym, name))
	      {
		einfo (_("%P: warning: truncated CodeView record"
			 " S_GPROC32/S_LPROC32\n"));
		return false;
	      }

	    section = bfd_getl16 (&proc->section);

	    if (section != 0)
	      {
		*sym_size += len;
	      }
	    else
	      {
		uint8_t *endptr = find_end_of_scope (data, size);

		if (!endptr)
		  {
		    einfo (_("%P: warning: could not find end of"
			     " S_GPROC32/S_LPROC32 record\n"));
		    return false;
		  }

		/* Skip to after S_END.  */

		size -= endptr - data;
		data = endptr;

		len = bfd_getl16 (data) + sizeof (uint16_t);

		data += len;
		size -= len;

		continue;
	      }

	    scope_level++;

	    break;
	  }

	case S_UDT:
	  if (scope_level != 0) /* only goes in symbols if local */
	    *sym_size += len;
	  break;

	case S_BLOCK32: /* always copied */
	case S_INLINESITE:
	case S_THUNK32:
	  *sym_size += len;
	  scope_level++;
	  break;

	case S_END: /* always copied */
	case S_PROC_ID_END:
	case S_INLINESITE_END:
	  *sym_size += len;
	  scope_level--;
	  break;

	case S_OBJNAME: /* always copied */
	case S_COMPILE3:
	case S_UNAMESPACE:
	case S_FRAMEPROC:
	case S_FRAMECOOKIE:
	case S_LABEL32:
	case S_BUILDINFO:
	case S_BPREL32:
	case S_REGISTER:
	case S_REGREL32:
	case S_LOCAL:
	case S_DEFRANGE_REGISTER_REL:
	case S_DEFRANGE_FRAMEPOINTER_REL:
	case S_DEFRANGE_SUBFIELD_REGISTER:
	case S_DEFRANGE_FRAMEPOINTER_REL_FULL_SCOPE:
	case S_DEFRANGE_REGISTER:
	case S_HEAPALLOCSITE:
	  *sym_size += len;
	  break;

	default:
	  einfo (_("%P: warning: unrecognized CodeView record %v\n"), type);
	  return false;
	}

      data += len;
      size -= len;
    }

  return true;
}

/* Parse the .debug$S section within an object file.  */
static bool
handle_debugs_section (asection *s, bfd *mod, struct string_table *strings,
		       uint8_t **dataptr, uint32_t *sizeptr,
		       struct mod_source_files *mod_source,
		       bfd *abfd, uint8_t **syms, uint32_t *sym_byte_size,
		       struct type_entry **map, uint32_t num_types,
		       bfd *sym_rec_stream, struct globals *glob,
		       uint16_t mod_num)
{
  bfd_byte *data = NULL;
  size_t off;
  uint32_t c13_size = 0;
  char *string_table = NULL;
  uint8_t *buf, *bufptr, *symbuf, *symbufptr;
  uint32_t sym_size = 0;

  if (!bfd_get_full_section_contents (mod, s, &data))
    return false;

  if (!data)
    return false;

  /* Resolve relocations.  Addresses are stored within the .debug$S section as
     a .secidx, .secrel32 pair.  */

  if (s->flags & SEC_RELOC)
    {
      struct internal_reloc *relocs;
      struct internal_syment *symbols;
      asection **sectlist;
      unsigned int syment_count;
      int sect_num;
      struct external_syment *ext;

      syment_count = obj_raw_syment_count (mod);

      relocs =
	_bfd_coff_read_internal_relocs (mod, s, false, NULL, true, NULL);

      symbols = xmalloc (sizeof (struct internal_syment) * syment_count);
      sectlist = xmalloc (sizeof (asection *) * syment_count);

      ext = (struct external_syment *) (coff_data (mod)->external_syms);

      for (unsigned int i = 0; i < syment_count; i++)
	{
	  bfd_coff_swap_sym_in (mod, &ext[i], &symbols[i]);
	}

      sect_num = 1;

      for (asection *sect = mod->sections; sect; sect = sect->next)
	{
	  for (unsigned int i = 0; i < syment_count; i++)
	    {
	      if (symbols[i].n_scnum == sect_num)
		sectlist[i] = sect;
	    }

	  sect_num++;
	}

      if (!bfd_coff_relocate_section (abfd, coff_data (abfd)->link_info, mod,
				      s, data, relocs, symbols, sectlist))
	{
	  free (sectlist);
	  free (symbols);
	  free (data);
	  return false;
	}

      free (sectlist);
      free (symbols);
    }

  if (bfd_getl32 (data) != CV_SIGNATURE_C13)
    {
      free (data);
      return true;
    }

  off = sizeof (uint32_t);

  /* calculate size */

  while (off + sizeof (uint32_t) <= s->size)
    {
      uint32_t type, size;

      type = bfd_getl32 (data + off);

      off += sizeof (uint32_t);

      if (off + sizeof (uint32_t) > s->size)
	{
	  free (data);
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}

      size = bfd_getl32 (data + off);

      off += sizeof (uint32_t);

      if (off + size > s->size)
	{
	  free (data);
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}

      switch (type)
	{
	case DEBUG_S_FILECHKSMS:
	  c13_size += sizeof (uint32_t) + sizeof (uint32_t) + size;

	  if (c13_size % sizeof (uint32_t))
	    c13_size += sizeof (uint32_t) - (c13_size % sizeof (uint32_t));

	  break;

	case DEBUG_S_STRINGTABLE:
	  parse_string_table (data + off, size, strings);

	  string_table = (char *) data + off;

	  break;

	case DEBUG_S_LINES:
	  {
	    uint16_t sect;

	    if (size < sizeof (uint32_t) + sizeof (uint16_t))
	      {
		free (data);
		bfd_set_error (bfd_error_bad_value);
		return false;
	      }

	    sect = bfd_getl16 (data + off + sizeof (uint32_t));

	    /* Skip GC'd symbols.  */
	    if (sect != 0)
	      {
		c13_size += sizeof (uint32_t) + sizeof (uint32_t) + size;

		if (c13_size % sizeof (uint32_t))
		  c13_size +=
		    sizeof (uint32_t) - (c13_size % sizeof (uint32_t));
	      }

	    break;
	  }

	case DEBUG_S_SYMBOLS:
	  if (!calculate_symbols_size (data + off, size, &sym_size))
	    {
	      free (data);
	      bfd_set_error (bfd_error_bad_value);
	      return false;
	    }

	  break;
	}

      off += size;

      if (off % sizeof (uint32_t))
	off += sizeof (uint32_t) - (off % sizeof (uint32_t));
    }

  if (sym_size % sizeof (uint32_t))
    sym_size += sizeof (uint32_t) - (sym_size % sizeof (uint32_t));

  if (c13_size == 0 && sym_size == 0)
    {
      free (data);
      return true;
    }

  /* copy data */

  buf = NULL;
  if (c13_size != 0)
    buf = xmalloc (c13_size);
  bufptr = buf;

  symbuf = NULL;
  if (sym_size != 0)
    symbuf = xmalloc (sym_size);
  symbufptr = symbuf;

  off = sizeof (uint32_t);

  while (off + sizeof (uint32_t) <= s->size)
    {
      uint32_t type, size;

      type = bfd_getl32 (data + off);
      off += sizeof (uint32_t);

      size = bfd_getl32 (data + off);
      off += sizeof (uint32_t);

      switch (type)
	{
	case DEBUG_S_FILECHKSMS:
	  if (!copy_filechksms (data + off, size, string_table,
				strings, bufptr, mod_source))
	    {
	      free (data);
	      free (symbuf);
	      return false;
	    }

	  bufptr += sizeof (uint32_t) + sizeof (uint32_t) + size;

	  break;

	case DEBUG_S_LINES:
	  {
	    uint16_t sect;

	    sect = bfd_getl16 (data + off + sizeof (uint32_t));

	    /* Skip if GC'd.  */
	    if (sect != 0)
	      {
		bfd_putl32 (type, bufptr);
		bufptr += sizeof (uint32_t);

		bfd_putl32 (size, bufptr);
		bufptr += sizeof (uint32_t);

		memcpy (bufptr, data + off, size);
		bufptr += size;
	      }

	    break;
	  }

	case DEBUG_S_SYMBOLS:
	  if (!parse_symbols (data + off, size, &symbufptr, map, num_types,
			      sym_rec_stream, glob, mod_num))
	    {
	      free (data);
	      free (symbuf);
	      return false;
	    }

	  break;
	}

      off += size;

      if (off % sizeof (uint32_t))
	off += sizeof (uint32_t) - (off % sizeof (uint32_t));
    }

  free (data);

  if (buf)
    {
      if (*dataptr)
	{
	  /* Append the C13 info to what's already there, if the module has
	     multiple .debug$S sections.  */

	  *dataptr = xrealloc (*dataptr, *sizeptr + c13_size);
	  memcpy (*dataptr + *sizeptr, buf, c13_size);

	  free (buf);
	}
      else
	{
	  *dataptr = buf;
	}

      *sizeptr += c13_size;
    }

  if (symbuf)
    {
      if (*syms)
	{
	  *syms = xrealloc (*syms, *sym_byte_size + sym_size);
	  memcpy (*syms + *sym_byte_size, symbuf, sym_size);

	  free (symbuf);
	}
      else
	{
	  *syms = symbuf;
	}

      *sym_byte_size += sym_size;
    }

  return true;
}

/* Remap the type number stored in data from the per-module numbering to
   that of the deduplicated output list.  */
static bool
remap_type (void *data, struct type_entry **map,
	    uint32_t type_num, uint32_t num_types)
{
  uint32_t type = bfd_getl32 (data);

  /* Ignore builtin types (those with IDs below 0x1000).  */
  if (type < TPI_FIRST_INDEX)
    return true;

  if (type >= TPI_FIRST_INDEX + type_num)
    {
      einfo (_("%P: CodeView type %v references other type %v not yet "
	       "declared\n"), TPI_FIRST_INDEX + type_num, type);
      return false;
    }

  if (type >= TPI_FIRST_INDEX + num_types)
    {
      einfo (_("%P: CodeView type %v references out of range type %v\n"),
	     TPI_FIRST_INDEX + type_num, type);
      return false;
    }

  type = TPI_FIRST_INDEX + map[type - TPI_FIRST_INDEX]->index;
  bfd_putl32 (type, data);

  return true;
}

/* Determines whether the name of a struct, class, or union counts as
   "anonymous".  Non-anonymous types have a hash based on just the name,
   rather than the whole structure.  */
static bool
is_name_anonymous (char *name, size_t len)
{
  static const char tag1[] = "<unnamed-tag>";
  static const char tag2[] = "__unnamed";
  static const char tag3[] = "::<unnamed-tag>";
  static const char tag4[] = "::__unnamed";

  if (len == sizeof (tag1) - 1 && !memcmp (name, tag1, sizeof (tag1) - 1))
    return true;

  if (len == sizeof (tag2) - 1 && !memcmp (name, tag2, sizeof (tag2) - 1))
    return true;

  if (len >= sizeof (tag3) - 1
      && !memcmp (name + len - sizeof (tag3) + 1, tag3, sizeof (tag3) - 1))
    return true;

  if (len >= sizeof (tag4) - 1
      && !memcmp (name + len - sizeof (tag4) + 1, tag4, sizeof (tag4) - 1))
    return true;

  return false;
}

/* Handle LF_UDT_SRC_LINE type entries, which are a special case.  These
   give the source file and line number for each user-defined type that is
   declared.  We parse these and emit instead an LF_UDT_MOD_SRC_LINE entry,
   which also includes the module number.  */
static bool
handle_udt_src_line (uint8_t *data, uint16_t size, struct type_entry **map,
		     uint32_t type_num, uint32_t num_types,
		     struct types *ids, uint16_t mod_num,
		     struct string_table *strings)
{
  struct lf_udt_src_line *usl = (struct lf_udt_src_line *) data;
  uint32_t orig_type, source_file_type;
  void **slot;
  hashval_t hash;
  struct type_entry *e, *type_e, *str_e;
  struct lf_udt_mod_src_line *umsl;
  struct lf_string_id *str;
  uint32_t source_file_offset;

  if (size < sizeof (struct lf_udt_src_line))
    {
      einfo (_("%P: warning: truncated CodeView type record"
	       " LF_UDT_SRC_LINE\n"));
      return false;
    }

  /* Check if LF_UDT_MOD_SRC_LINE already present for type, and return.  */

  orig_type = bfd_getl32 (&usl->type);

  if (orig_type < TPI_FIRST_INDEX ||
      orig_type >= TPI_FIRST_INDEX + num_types ||
      !map[orig_type - TPI_FIRST_INDEX])
    {
      einfo (_("%P: warning: CodeView type record LF_UDT_SRC_LINE"
	       " referred to unknown type %v\n"), orig_type);
      return false;
    }

  type_e = map[orig_type - TPI_FIRST_INDEX];

  /* Skip if type already declared in other module.  */
  if (type_e->has_udt_src_line)
    return true;

  if (!remap_type (&usl->type, map, type_num, num_types))
    return false;

  /* Extract string from source_file_type.  */

  source_file_type = bfd_getl32 (&usl->source_file_type);

  if (source_file_type < TPI_FIRST_INDEX ||
      source_file_type >= TPI_FIRST_INDEX + num_types ||
      !map[source_file_type - TPI_FIRST_INDEX])
    {
      einfo (_("%P: warning: CodeView type record LF_UDT_SRC_LINE"
	       " referred to unknown string %v\n"), source_file_type);
      return false;
    }

  str_e = map[source_file_type - TPI_FIRST_INDEX];

  if (bfd_getl16 (str_e->data + sizeof (uint16_t)) != LF_STRING_ID)
    {
      einfo (_("%P: warning: CodeView type record LF_UDT_SRC_LINE"
	       " pointed to unexpected record type\n"));
      return false;
    }

  str = (struct lf_string_id *) str_e->data;

  /* Add string to string table.  */

  source_file_offset = add_string (str->string, strlen (str->string),
				   strings);

  /* Add LF_UDT_MOD_SRC_LINE entry.  */

  size = sizeof (struct lf_udt_mod_src_line);

  e = xmalloc (offsetof (struct type_entry, data) + size);

  e->next = NULL;
  e->index = ids->num_types;
  e->has_udt_src_line = false;

  /* LF_UDT_MOD_SRC_LINE use calc_hash on the type number, rather than
     the crc32 used for type hashes elsewhere.  */
  e->cv_hash = calc_hash ((char *) &usl->type, sizeof (uint32_t));

  type_e->has_udt_src_line = true;

  umsl = (struct lf_udt_mod_src_line *) e->data;

  bfd_putl16 (size - sizeof (uint16_t), &umsl->size);
  bfd_putl16 (LF_UDT_MOD_SRC_LINE, &umsl->kind);
  memcpy (&umsl->type, &usl->type, sizeof (uint32_t));
  bfd_putl32 (source_file_offset, &umsl->source_file_string);
  memcpy (&umsl->line_no, &usl->line_no, sizeof (uint32_t));
  bfd_putl16 (mod_num + 1, &umsl->module_no);

  hash = iterative_hash (e->data, size, 0);

  slot = htab_find_slot_with_hash (ids->hashmap, data, hash, INSERT);
  if (!slot)
    {
      free (e);
      return false;
    }

  if (*slot)
    {
      free (e);
      einfo (_("%P: warning: duplicate CodeView type record "
	       "LF_UDT_MOD_SRC_LINE\n"));
      return false;
    }

  *slot = e;

  if (ids->last)
    ids->last->next = e;
  else
    ids->first = e;

  ids->last = e;

  map[type_num] = e;

  ids->num_types++;

  return true;
}

/* Parse a type definition in the .debug$T section.  We remap the numbers
   of any referenced types, and if the type is not a duplicate of one
   already seen add it to types (for TPI types) or ids (for IPI types).  */
static bool
handle_type (uint8_t *data, struct type_entry **map, uint32_t type_num,
	     uint32_t num_types, struct types *types,
	     struct types *ids, uint16_t mod_num,
	     struct string_table *strings)
{
  uint16_t size, type;
  void **slot;
  hashval_t hash;
  bool other_hash = false;
  uint32_t cv_hash;
  struct types *t;
  bool ipi = false;

  size = bfd_getl16 (data) + sizeof (uint16_t);
  type = bfd_getl16 (data + sizeof (uint16_t));

  switch (type)
    {
    case LF_MODIFIER:
      {
	struct lf_modifier *mod = (struct lf_modifier *) data;

	if (size < offsetof (struct lf_modifier, modifier))
	  {
	    einfo (_("%P: warning: truncated CodeView type record "
		     "LF_MODIFIER\n"));
	    return false;
	  }

	if (!remap_type (&mod->base_type, map, type_num, num_types))
	  return false;

	break;
      }

    case LF_POINTER:
      {
	struct lf_pointer *ptr = (struct lf_pointer *) data;

	if (size < offsetof (struct lf_pointer, attributes))
	  {
	    einfo (_("%P: warning: truncated CodeView type record"
		     " LF_POINTER\n"));
	    return false;
	  }

	if (!remap_type (&ptr->base_type, map, type_num, num_types))
	  return false;

	break;
      }

    case LF_PROCEDURE:
      {
	struct lf_procedure *proc = (struct lf_procedure *) data;

	if (size < sizeof (struct lf_procedure))
	  {
	    einfo (_("%P: warning: truncated CodeView type record"
		     " LF_PROCEDURE\n"));
	    return false;
	  }

	if (!remap_type (&proc->return_type, map, type_num, num_types))
	  return false;

	if (!remap_type (&proc->arglist, map, type_num, num_types))
	  return false;

	break;
      }

    case LF_MFUNCTION:
      {
	struct lf_mfunction *func = (struct lf_mfunction *) data;

	if (size < sizeof (struct lf_procedure))
	  {
	    einfo (_("%P: warning: truncated CodeView type record"
		     " LF_MFUNCTION\n"));
	    return false;
	  }

	if (!remap_type (&func->return_type, map, type_num, num_types))
	  return false;

	if (!remap_type (&func->containing_class_type, map, type_num,
			 num_types))
	  return false;

	if (!remap_type (&func->this_type, map, type_num, num_types))
	  return false;

	if (!remap_type (&func->arglist, map, type_num, num_types))
	  return false;

	break;
      }

    case LF_ARGLIST:
      {
	uint32_t num_entries;
	struct lf_arglist *al = (struct lf_arglist *) data;

	if (size < offsetof (struct lf_arglist, args))
	  {
	    einfo (_("%P: warning: truncated CodeView type record"
		     " LF_ARGLIST\n"));
	    return false;
	  }

	num_entries = bfd_getl32 (&al->num_entries);

	if (size < offsetof (struct lf_arglist, args)
		   + (num_entries * sizeof (uint32_t)))
	  {
	    einfo (_("%P: warning: truncated CodeView type record"
		     " LF_ARGLIST\n"));
	    return false;
	  }

	for (uint32_t i = 0; i < num_entries; i++)
	  {
	    if (!remap_type (&al->args[i], map, type_num, num_types))
	      return false;
	  }

	break;
      }

    case LF_FIELDLIST:
      {
	uint16_t left = size - sizeof (uint16_t) - sizeof (uint16_t);
	uint8_t *ptr = data + sizeof (uint16_t) + sizeof (uint16_t);

	while (left > 0)
	  {
	    uint16_t subtype;

	    if (left < sizeof (uint16_t))
	      {
		einfo (_("%P: warning: truncated CodeView type record"
			 " LF_FIELDLIST\n"));
		return false;
	      }

	    subtype = bfd_getl16 (ptr);

	    switch (subtype)
	      {
	      case LF_MEMBER:
		{
		  struct lf_member *mem = (struct lf_member *) ptr;
		  uint16_t offset;
		  size_t name_len, subtype_len;

		  if (left < offsetof (struct lf_member, name))
		    {
		      einfo (_("%P: warning: truncated CodeView type record"
			       " LF_MEMBER\n"));
		      return false;
		    }

		  if (!remap_type (&mem->type, map, type_num, num_types))
		    return false;

		  subtype_len = offsetof (struct lf_member, name);

		  offset = bfd_getl16 (&mem->offset);

		  /* If offset >= 0x8000, actual value follows.  */
		  if (offset >= 0x8000)
		    {
		      unsigned int param_len = extended_value_len (offset);

		      if (param_len == 0)
			{
			  einfo (_("%P: warning: unhandled type %v within"
				   " LF_MEMBER\n"), offset);
			  return false;
			}

		      subtype_len += param_len;

		      if (left < subtype_len)
			{
			  einfo (_("%P: warning: truncated CodeView type record"
				  " LF_MEMBER\n"));
			  return false;
			}
		    }

		  name_len =
		    strnlen ((char *) mem + subtype_len, left - subtype_len);

		  if (name_len == left - offsetof (struct lf_member, name))
		    {
		      einfo (_("%P: warning: name for LF_MEMBER has no"
			       " terminating zero\n"));
		      return false;
		    }

		  name_len++;

		  subtype_len += name_len;

		  if (subtype_len % 4 != 0)
		    subtype_len += 4 - (subtype_len % 4);

		  if (left < subtype_len)
		    {
		      einfo (_("%P: warning: truncated CodeView type record"
			       " LF_FIELDLIST\n"));
		      return false;
		    }

		  ptr += subtype_len;
		  left -= subtype_len;

		  break;
		}

	      case LF_ENUMERATE:
		{
		  struct lf_enumerate *en = (struct lf_enumerate *) ptr;
		  size_t name_len, subtype_len;
		  uint16_t val;

		  if (left < offsetof (struct lf_enumerate, name))
		    {
		      einfo (_("%P: warning: truncated CodeView type record"
			       " LF_ENUMERATE\n"));
		      return false;
		    }

		  subtype_len = offsetof (struct lf_enumerate, name);

		  val = bfd_getl16 (&en->value);

		  /* If val >= 0x8000, the actual value immediately follows.  */
		  if (val >= 0x8000)
		    {
		      unsigned int param_len = extended_value_len (val);

		      if (param_len == 0)
			{
			  einfo (_("%P: warning: unhandled type %v within"
				   " LF_ENUMERATE\n"), val);
			  return false;
			}

		      if (left < subtype_len + param_len)
			{
			  einfo (_("%P: warning: truncated CodeView type"
				   " record LF_ENUMERATE\n"));
			  return false;
			}

		      subtype_len += param_len;
		    }

		  name_len = strnlen ((char *) ptr + subtype_len,
				      left - subtype_len);

		  if (name_len == left - offsetof (struct lf_enumerate, name))
		    {
		      einfo (_("%P: warning: name for LF_ENUMERATE has no"
			       " terminating zero\n"));
		      return false;
		    }

		  name_len++;

		  subtype_len += name_len;

		  if (subtype_len % 4 != 0)
		    subtype_len += 4 - (subtype_len % 4);

		  if (left < subtype_len)
		    {
		      einfo (_("%P: warning: truncated CodeView type record"
			       " LF_ENUMERATE\n"));
		      return false;
		    }

		  ptr += subtype_len;
		  left -= subtype_len;

		  break;
		}

	      case LF_INDEX:
		{
		  struct lf_index *ind = (struct lf_index *) ptr;

		  if (left < sizeof (struct lf_index))
		    {
		      einfo (_("%P: warning: truncated CodeView type record"
			       " LF_INDEX\n"));
		      return false;
		    }

		  if (!remap_type (&ind->index, map, type_num, num_types))
		    return false;

		  ptr += sizeof (struct lf_index);
		  left -= sizeof (struct lf_index);

		  break;
		}

	      case LF_ONEMETHOD:
		{
		  struct lf_onemethod *meth = (struct lf_onemethod *) ptr;
		  size_t name_len, subtype_len;

		  if (left < offsetof (struct lf_onemethod, name))
		    {
		      einfo (_("%P: warning: truncated CodeView type record"
			       " LF_ONEMETHOD\n"));
		      return false;
		    }

		  if (!remap_type (&meth->method_type, map, type_num,
				   num_types))
		    return false;

		  name_len =
		    strnlen (meth->name,
			     left - offsetof (struct lf_onemethod, name));

		  if (name_len == left - offsetof (struct lf_onemethod, name))
		    {
		      einfo (_("%P: warning: name for LF_ONEMETHOD has no"
			       " terminating zero\n"));
		      return false;
		    }

		  name_len++;

		  subtype_len = offsetof (struct lf_onemethod, name)
				+ name_len;

		  if (subtype_len % 4 != 0)
		    subtype_len += 4 - (subtype_len % 4);

		  if (left < subtype_len)
		    {
		      einfo (_("%P: warning: truncated CodeView type record"
			       " LF_FIELDLIST\n"));
		      return false;
		    }

		  ptr += subtype_len;
		  left -= subtype_len;

		  break;
		}

	      case LF_METHOD:
		{
		  struct lf_method *meth = (struct lf_method *) ptr;
		  size_t name_len, subtype_len;

		  if (left < offsetof (struct lf_method, name))
		    {
		      einfo (_("%P: warning: truncated CodeView type record"
			       " LF_METHOD\n"));
		      return false;
		    }

		  if (!remap_type (&meth->method_list, map, type_num,
				   num_types))
		    return false;

		  name_len =
		    strnlen (meth->name,
			     left - offsetof (struct lf_method, name));

		  if (name_len == left - offsetof (struct lf_method, name))
		    {
		      einfo (_("%P: warning: name for LF_METHOD has no"
			       " terminating zero\n"));
		      return false;
		    }

		  name_len++;

		  subtype_len = offsetof (struct lf_method, name) + name_len;

		  if (subtype_len % 4 != 0)
		    subtype_len += 4 - (subtype_len % 4);

		  if (left < subtype_len)
		    {
		      einfo (_("%P: warning: truncated CodeView type record"
			       " LF_FIELDLIST\n"));
		      return false;
		    }

		  ptr += subtype_len;
		  left -= subtype_len;

		  break;
		}

	      case LF_BCLASS:
		{
		  struct lf_bclass *bc = (struct lf_bclass *) ptr;
		  size_t subtype_len;
		  uint16_t offset;

		  if (left < sizeof (struct lf_bclass))
		    {
		      einfo (_("%P: warning: truncated CodeView type record"
			       " LF_BCLASS\n"));
		      return false;
		    }

		  if (!remap_type (&bc->base_class_type, map, type_num,
				   num_types))
		    return false;

		  subtype_len = sizeof (struct lf_bclass);

		  offset = bfd_getl16 (&bc->offset);

		  /* If offset >= 0x8000, actual value follows.  */
		  if (offset >= 0x8000)
		    {
		      unsigned int param_len = extended_value_len (offset);

		      if (param_len == 0)
			{
			  einfo (_("%P: warning: unhandled type %v within"
				   " LF_BCLASS\n"), offset);
			  return false;
			}

		      subtype_len += param_len;

		      if (left < subtype_len)
			{
			  einfo (_("%P: warning: truncated CodeView type record"
				   " LF_BCLASS\n"));
			  return false;
			}
		    }

		  if (subtype_len % 4 != 0)
		    subtype_len += 4 - (subtype_len % 4);

		  if (left < subtype_len)
		    {
		      einfo (_("%P: warning: truncated CodeView type record"
			       " LF_BCLASS\n"));
		      return false;
		    }

		  ptr += subtype_len;
		  left -= subtype_len;

		  break;
		}

	      case LF_VFUNCTAB:
		{
		  struct lf_vfunctab *vft = (struct lf_vfunctab *) ptr;

		  if (left < sizeof (struct lf_vfunctab))
		    {
		      einfo (_("%P: warning: truncated CodeView type record"
			       " LF_VFUNCTAB\n"));
		      return false;
		    }

		  if (!remap_type (&vft->type, map, type_num, num_types))
		    return false;

		  ptr += sizeof (struct lf_vfunctab);
		  left -= sizeof (struct lf_vfunctab);

		  break;
		}

	      case LF_VBCLASS:
	      case LF_IVBCLASS:
		{
		  struct lf_vbclass *vbc = (struct lf_vbclass *) ptr;
		  size_t subtype_len;
		  uint16_t offset;

		  if (left < sizeof (struct lf_vbclass))
		    {
		      einfo (_("%P: warning: truncated CodeView type record"
			       " LF_VBCLASS/LF_IVBCLASS\n"));
		      return false;
		    }

		  if (!remap_type (&vbc->base_class_type, map, type_num,
				   num_types))
		    return false;

		  if (!remap_type (&vbc->virtual_base_pointer_type, map,
				   type_num, num_types))
		    return false;

		  subtype_len = offsetof (struct lf_vbclass,
					  virtual_base_vbtable_offset);

		  offset = bfd_getl16 (&vbc->virtual_base_pointer_offset);

		  /* If offset >= 0x8000, actual value follows.  */
		  if (offset >= 0x8000)
		    {
		      unsigned int param_len = extended_value_len (offset);

		      if (param_len == 0)
			{
			  einfo (_("%P: warning: unhandled type %v within"
				   " LF_VBCLASS/LF_IVBCLASS\n"), offset);
			  return false;
			}

		      subtype_len += param_len;

		      if (left < subtype_len)
			{
			  einfo (_("%P: warning: truncated CodeView type record"
				   " LF_VBCLASS/LF_IVBCLASS\n"));
			  return false;
			}
		    }

		  offset = bfd_getl16 ((char *)vbc + subtype_len);
		  subtype_len += sizeof (uint16_t);

		  /* If offset >= 0x8000, actual value follows.  */
		  if (offset >= 0x8000)
		    {
		      unsigned int param_len = extended_value_len (offset);

		      if (param_len == 0)
			{
			  einfo (_("%P: warning: unhandled type %v within"
				   " LF_VBCLASS/LF_IVBCLASS\n"), offset);
			  return false;
			}

		      subtype_len += param_len;

		      if (left < subtype_len)
			{
			  einfo (_("%P: warning: truncated CodeView type record"
				   " LF_VBCLASS/LF_IVBCLASS\n"));
			  return false;
			}
		    }

		  if (subtype_len % 4 != 0)
		    subtype_len += 4 - (subtype_len % 4);

		  if (left < subtype_len)
		    {
		      einfo (_("%P: warning: truncated CodeView type record"
			       " LF_VBCLASS/LF_IVBCLASS\n"));
		      return false;
		    }

		  ptr += subtype_len;
		  left -= subtype_len;

		  break;
		}

	      case LF_STMEMBER:
		{
		  struct lf_static_member *st =
		    (struct lf_static_member *) ptr;
		  size_t name_len, subtype_len;

		  if (left < offsetof (struct lf_static_member, name))
		    {
		      einfo (_("%P: warning: truncated CodeView type record"
			       " LF_STMEMBER\n"));
		      return false;
		    }

		  if (!remap_type (&st->type, map, type_num, num_types))
		    return false;

		  name_len =
		    strnlen (st->name,
			     left - offsetof (struct lf_static_member, name));

		  if (name_len == left
				  - offsetof (struct lf_static_member, name))
		    {
		      einfo (_("%P: warning: name for LF_STMEMBER has no"
			       " terminating zero\n"));
		      return false;
		    }

		  name_len++;

		  subtype_len = offsetof (struct lf_static_member, name)
				+ name_len;

		  if (subtype_len % 4 != 0)
		    subtype_len += 4 - (subtype_len % 4);

		  if (left < subtype_len)
		    {
		      einfo (_("%P: warning: truncated CodeView type record"
			       " LF_FIELDLIST\n"));
		      return false;
		    }

		  ptr += subtype_len;
		  left -= subtype_len;

		  break;
		}

	      case LF_NESTTYPE:
		{
		  struct lf_nest_type *nest = (struct lf_nest_type *) ptr;
		  size_t name_len, subtype_len;

		  if (left < offsetof (struct lf_nest_type, name))
		    {
		      einfo (_("%P: warning: truncated CodeView type record"
			       " LF_NESTTYPE\n"));
		      return false;
		    }

		  if (!remap_type (&nest->type, map, type_num, num_types))
		    return false;

		  name_len =
		    strnlen (nest->name,
			     left - offsetof (struct lf_nest_type, name));

		  if (name_len == left - offsetof (struct lf_nest_type, name))
		    {
		      einfo (_("%P: warning: name for LF_NESTTYPE has no"
			       " terminating zero\n"));
		      return false;
		    }

		  name_len++;

		  subtype_len = offsetof (struct lf_nest_type, name)
				+ name_len;

		  if (subtype_len % 4 != 0)
		    subtype_len += 4 - (subtype_len % 4);

		  if (left < subtype_len)
		    {
		      einfo (_("%P: warning: truncated CodeView type record"
			       " LF_FIELDLIST\n"));
		      return false;
		    }

		  ptr += subtype_len;
		  left -= subtype_len;

		  break;
		}

	      default:
		einfo (_("%P: warning: unrecognized CodeView subtype %v\n"),
		       subtype);
		return false;
	      }
	  }

	break;
      }

    case LF_BITFIELD:
      {
	struct lf_bitfield *bf = (struct lf_bitfield *) data;

	if (size < offsetof (struct lf_bitfield, length))
	  {
	    einfo (_("%P: warning: truncated CodeView type record"
		     " LF_BITFIELD\n"));
	    return false;
	  }

	if (!remap_type (&bf->base_type, map, type_num, num_types))
	  return false;

	break;
      }

    case LF_METHODLIST:
      {
	struct lf_methodlist *ml = (struct lf_methodlist *) data;
	unsigned int num_entries;

	if (size < offsetof (struct lf_methodlist, entries))
	  {
	    einfo (_("%P: warning: truncated CodeView type record"
		     " LF_METHODLIST\n"));
	    return false;
	  }

	if ((size - offsetof (struct lf_methodlist, entries))
	    % sizeof (struct lf_methodlist_entry))
	  {
	    einfo (_("%P: warning: malformed CodeView type record"
		     " LF_METHODLIST\n"));
	    return false;
	  }

	num_entries = (size - offsetof (struct lf_methodlist, entries))
		      / sizeof (struct lf_methodlist_entry);

	for (unsigned int i = 0; i < num_entries; i++)
	  {
	    if (!remap_type (&ml->entries[i].method_type, map,
			     type_num, num_types))
	      return false;
	  }

	break;
      }

    case LF_ARRAY:
      {
	struct lf_array *arr = (struct lf_array *) data;

	if (size < offsetof (struct lf_array, length_in_bytes))
	  {
	    einfo (_("%P: warning: truncated CodeView type record"
		     " LF_ARRAY\n"));
	    return false;
	  }

	if (!remap_type (&arr->element_type, map, type_num, num_types))
	  return false;

	if (!remap_type (&arr->index_type, map, type_num, num_types))
	  return false;

	break;
      }

    case LF_CLASS:
    case LF_STRUCTURE:
      {
	struct lf_class *cl = (struct lf_class *) data;
	uint16_t prop, num_bytes;
	size_t name_len, name_off;

	if (size < offsetof (struct lf_class, name))
	  {
	    einfo (_("%P: warning: truncated CodeView type record"
		     " LF_CLASS/LF_STRUCTURE\n"));
	    return false;
	  }

	if (!remap_type (&cl->field_list, map, type_num, num_types))
	  return false;

	if (!remap_type (&cl->derived_from, map, type_num, num_types))
	  return false;

	if (!remap_type (&cl->vshape, map, type_num, num_types))
	  return false;

	name_off = offsetof (struct lf_class, name);

	num_bytes = bfd_getl16 (&cl->length);

	/* If num_bytes >= 0x8000, actual value follows.  */
	if (num_bytes >= 0x8000)
	  {
	    unsigned int param_len = extended_value_len (num_bytes);

	    if (param_len == 0)
	      {
		einfo (_("%P: warning: unhandled type %v within"
			 " LF_CLASS/LF_STRUCTURE\n"), num_bytes);
		return false;
	      }

	    name_off += param_len;

	    if (size < name_off)
	      {
		einfo (_("%P: warning: truncated CodeView type record"
			 " LF_CLASS/LF_STRUCTURE\n"));
		return false;
	      }
	  }

	name_len = strnlen ((char *) cl + name_off, size - name_off);

	if (name_len == size - name_off)
	  {
	    einfo (_("%P: warning: name for LF_CLASS/LF_STRUCTURE has no"
		     " terminating zero\n"));
	    return false;
	  }

	prop = bfd_getl16 (&cl->properties);

	if (prop & CV_PROP_HAS_UNIQUE_NAME)
	  {
	    /* Structure has another name following first one.  */

	    size_t len = name_off + name_len + 1;
	    size_t unique_name_len;

	    unique_name_len = strnlen ((char *) cl + name_off + name_len + 1,
				       size - len);

	    if (unique_name_len == size - len)
	      {
		einfo (_("%P: warning: unique name for LF_CLASS/LF_STRUCTURE"
			 " has no terminating zero\n"));
		return false;
	      }
	  }

	if (!(prop & (CV_PROP_FORWARD_REF | CV_PROP_SCOPED))
	    && !is_name_anonymous ((char *) cl + name_off, name_len))
	  {
	    other_hash = true;
	    cv_hash = crc32 ((uint8_t *) cl + name_off, name_len);
	  }

	break;
      }

    case LF_UNION:
      {
	struct lf_union *un = (struct lf_union *) data;
	uint16_t prop, num_bytes;
	size_t name_len, name_off;

	if (size < offsetof (struct lf_union, name))
	  {
	    einfo (_("%P: warning: truncated CodeView type record"
		     " LF_UNION\n"));
	    return false;
	  }

	if (!remap_type (&un->field_list, map, type_num, num_types))
	  return false;

	name_off = offsetof (struct lf_union, name);

	num_bytes = bfd_getl16 (&un->length);

	/* If num_bytes >= 0x8000, actual value follows.  */
	if (num_bytes >= 0x8000)
	  {
	    unsigned int param_len = extended_value_len (num_bytes);

	    if (param_len == 0)
	      {
		einfo (_("%P: warning: unhandled type %v within"
			 " LF_UNION\n"), num_bytes);
		return false;
	      }

	    name_off += param_len;

	    if (size < name_off)
	      {
		einfo (_("%P: warning: truncated CodeView type record"
			 " LF_UNION\n"));
		return false;
	      }
	  }

	name_len = strnlen ((char *) un + name_off, size - name_off);

	if (name_len == size - name_off)
	  {
	    einfo (_("%P: warning: name for LF_UNION has no"
		     " terminating zero\n"));
	    return false;
	  }

	prop = bfd_getl16 (&un->properties);

	if (prop & CV_PROP_HAS_UNIQUE_NAME)
	  {
	    /* Structure has another name following first one.  */

	    size_t len = name_off + name_len + 1;
	    size_t unique_name_len;

	    unique_name_len = strnlen ((char *) un + name_off + name_len + 1,
				       size - len);

	    if (unique_name_len == size - len)
	      {
		einfo (_("%P: warning: unique name for LF_UNION has"
			 " no terminating zero\n"));
		return false;
	      }
	  }

	if (!(prop & (CV_PROP_FORWARD_REF | CV_PROP_SCOPED))
	    && !is_name_anonymous ((char *) un + name_off, name_len))
	  {
	    other_hash = true;
	    cv_hash = crc32 ((uint8_t *) un + name_off, name_len);
	  }

	break;
      }

    case LF_ENUM:
      {
	struct lf_enum *en = (struct lf_enum *) data;
	uint16_t prop;
	size_t name_len;

	if (size < offsetof (struct lf_enum, name))
	  {
	    einfo (_("%P: warning: truncated CodeView type record"
		     " LF_ENUM\n"));
	    return false;
	  }

	if (!remap_type (&en->underlying_type, map, type_num, num_types))
	  return false;

	if (!remap_type (&en->field_list, map, type_num, num_types))
	  return false;

	name_len = strnlen (en->name, size - offsetof (struct lf_enum, name));

	if (name_len == size - offsetof (struct lf_enum, name))
	  {
	    einfo (_("%P: warning: name for LF_ENUM has no"
		     " terminating zero\n"));
	    return false;
	  }

	prop = bfd_getl16 (&en->properties);

	if (prop & CV_PROP_HAS_UNIQUE_NAME)
	  {
	    /* Structure has another name following first one.  */

	    size_t len = offsetof (struct lf_enum, name) + name_len + 1;
	    size_t unique_name_len;

	    unique_name_len = strnlen (en->name + name_len + 1, size - len);

	    if (unique_name_len == size - len)
	      {
		einfo (_("%P: warning: unique name for LF_ENUM has"
			 " no terminating zero\n"));
		return false;
	      }
	  }

	break;
      }

    case LF_VTSHAPE:
      /* Does not reference any types, nothing to be done.  */
      break;

    case LF_VFTABLE:
      {
	struct lf_vftable *vft = (struct lf_vftable *) data;

	if (size < offsetof (struct lf_vftable, names))
	  {
	    einfo (_("%P: warning: truncated CodeView type record"
		     " LF_VFTABLE\n"));
	    return false;
	  }

	if (!remap_type (&vft->type, map, type_num, num_types))
	  return false;

	if (!remap_type (&vft->base_vftable, map, type_num, num_types))
	  return false;

	break;
      }

    case LF_STRING_ID:
      {
	struct lf_string_id *str = (struct lf_string_id *) data;
	size_t string_len;

	if (size < offsetof (struct lf_string_id, string))
	  {
	    einfo (_("%P: warning: truncated CodeView type record"
		     " LF_STRING_ID\n"));
	    return false;
	  }

	if (!remap_type (&str->substring, map, type_num, num_types))
	  return false;

	string_len = strnlen (str->string,
			      size - offsetof (struct lf_string_id, string));

	if (string_len == size - offsetof (struct lf_string_id, string))
	  {
	    einfo (_("%P: warning: string for LF_STRING_ID has no"
		     " terminating zero\n"));
	    return false;
	  }

	ipi = true;

	break;
      }

    case LF_SUBSTR_LIST:
      {
	uint32_t num_entries;
	struct lf_arglist *ssl = (struct lf_arglist *) data;

	if (size < offsetof (struct lf_arglist, args))
	  {
	    einfo (_("%P: warning: truncated CodeView type record"
		     " LF_SUBSTR_LIST\n"));
	    return false;
	  }

	num_entries = bfd_getl32 (&ssl->num_entries);

	if (size < offsetof (struct lf_arglist, args)
		   + (num_entries * sizeof (uint32_t)))
	  {
	    einfo (_("%P: warning: truncated CodeView type record"
		     " LF_SUBSTR_LIST\n"));
	    return false;
	  }

	for (uint32_t i = 0; i < num_entries; i++)
	  {
	    if (!remap_type (&ssl->args[i], map, type_num, num_types))
	      return false;
	  }

	ipi = true;

	break;
      }

    case LF_BUILDINFO:
      {
	uint16_t num_entries;
	struct lf_build_info *bi = (struct lf_build_info *) data;

	if (size < offsetof (struct lf_build_info, strings))
	  {
	    einfo (_("%P: warning: truncated CodeView type record"
		     " LF_BUILDINFO\n"));
	    return false;
	  }

	num_entries = bfd_getl16 (&bi->count);

	if (size < offsetof (struct lf_build_info, strings)
		   + (num_entries * sizeof (uint32_t)))
	  {
	    einfo (_("%P: warning: truncated CodeView type record"
		     " LF_BUILDINFO\n"));
	    return false;
	  }

	for (uint32_t i = 0; i < num_entries; i++)
	  {
	    if (!remap_type (&bi->strings[i], map, type_num, num_types))
	      return false;
	  }

	ipi = true;

	break;
      }

    case LF_FUNC_ID:
      {
	struct lf_func_id *func = (struct lf_func_id *) data;
	size_t name_len;

	if (size < offsetof (struct lf_func_id, name))
	  {
	    einfo (_("%P: warning: truncated CodeView type record"
		     " LF_FUNC_ID\n"));
	    return false;
	  }

	if (!remap_type (&func->parent_scope, map, type_num, num_types))
	  return false;

	if (!remap_type (&func->function_type, map, type_num, num_types))
	  return false;

	name_len = strnlen (func->name,
			    size - offsetof (struct lf_func_id, name));

	if (name_len == size - offsetof (struct lf_func_id, name))
	  {
	    einfo (_("%P: warning: string for LF_FUNC_ID has no"
		     " terminating zero\n"));
	    return false;
	  }

	ipi = true;

	break;
      }

    case LF_MFUNC_ID:
      {
	struct lf_mfunc_id *mfunc = (struct lf_mfunc_id *) data;
	size_t name_len;

	if (size < offsetof (struct lf_mfunc_id, name))
	  {
	    einfo (_("%P: warning: truncated CodeView type record"
		     " LF_MFUNC_ID\n"));
	    return false;
	  }

	if (!remap_type (&mfunc->parent_type, map, type_num, num_types))
	  return false;

	if (!remap_type (&mfunc->function_type, map, type_num, num_types))
	  return false;

	name_len = strnlen (mfunc->name,
			    size - offsetof (struct lf_mfunc_id, name));

	if (name_len == size - offsetof (struct lf_mfunc_id, name))
	  {
	    einfo (_("%P: warning: string for LF_MFUNC_ID has no"
		     " terminating zero\n"));
	    return false;
	  }

	ipi = true;

	break;
      }

    case LF_UDT_SRC_LINE:
      return handle_udt_src_line (data, size, map, type_num, num_types,
				  ids, mod_num, strings);

    default:
      einfo (_("%P: warning: unrecognized CodeView type %v\n"), type);
      return false;
    }

  hash = iterative_hash (data, size, 0);

  t = ipi ? ids : types;

  slot = htab_find_slot_with_hash (t->hashmap, data, hash, INSERT);
  if (!slot)
    return false;

  if (!*slot) /* new entry */
    {
      struct type_entry *e;

      *slot = xmalloc (offsetof (struct type_entry, data) + size);

      e = (struct type_entry *) *slot;

      e->next = NULL;
      e->index = t->num_types;

      if (other_hash)
	e->cv_hash = cv_hash;
      else
	e->cv_hash = crc32 (data, size);

      e->has_udt_src_line = false;

      memcpy (e->data, data, size);

      if (t->last)
	t->last->next = e;
      else
	t->first = e;

      t->last = e;

      map[type_num] = e;

      t->num_types++;
    }
  else /* duplicate */
    {
      map[type_num] = (struct type_entry *) *slot;
    }

  return true;
}

/* Parse the .debug$T section of a module, and pass any type definitions
   found to handle_type.  */
static bool
handle_debugt_section (asection *s, bfd *mod, struct types *types,
		       struct types *ids, uint16_t mod_num,
		       struct string_table *strings,
		       struct type_entry ***map, uint32_t *num_types)
{
  bfd_byte *data = NULL;
  size_t off;
  uint32_t type_num;

  if (!bfd_get_full_section_contents (mod, s, &data))
    return false;

  if (!data)
    return false;

  if (bfd_getl32 (data) != CV_SIGNATURE_C13)
    {
      free (data);
      return true;
    }

  off = sizeof (uint32_t);

  while (off + sizeof (uint16_t) <= s->size)
    {
      uint16_t size;

      size = bfd_getl16 (data + off);
      off += sizeof (uint16_t);

      if (size + off > s->size || size <= sizeof (uint16_t))
	{
	  free (data);
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}

      (*num_types)++;
      off += size;
    }

  if (*num_types == 0)
    {
      free (data);
      return true;
    }

  *map = xcalloc (*num_types, sizeof (struct type_entry *));

  off = sizeof (uint32_t);
  type_num = 0;

  while (off + sizeof (uint16_t) <= s->size)
    {
      uint16_t size;

      size = bfd_getl16 (data + off);

      if (!handle_type (data + off, *map, type_num, *num_types, types, ids,
			mod_num, strings))
	{
	  free (data);
	  free (*map);
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}

      off += sizeof (uint16_t) + size;
      type_num++;
    }

  free (data);

  return true;
}

/* Return the CodeView constant for the selected architecture.  */
static uint16_t
target_processor (bfd *abfd)
{
  switch (abfd->arch_info->arch)
    {
    case bfd_arch_i386:
      if (abfd->arch_info->mach & bfd_mach_x86_64)
	return CV_CFL_X64;
      else
	return CV_CFL_80386;

    case bfd_arch_aarch64:
      return CV_CFL_ARM64;

    default:
      return 0;
    }
}

/* Create the symbols that go in "* Linker *", the dummy module created
   for the linker itself.  */
static bool
create_linker_symbols (bfd *abfd, uint8_t **syms, uint32_t *sym_byte_size,
		       const char *pdb_name)
{
  uint8_t *ptr;
  struct objname *name;
  struct compile3 *comp;
  struct envblock *env;
  size_t padding1, padding2, env_size;
  char *cwdval, *exeval, *pdbval;

  /* extra NUL for padding */
  static const char linker_fn[] = "* Linker *\0";
  static const char linker_name[] = "GNU LD " VERSION;

  static const char cwd[] = "cwd";
  static const char exe[] = "exe";
  static const char pdb[] = "pdb";

  cwdval = getcwd (NULL, 0);
  if (!cwdval)
    {
      einfo (_("%P: warning: unable to get working directory\n"));
      return false;
    }

  exeval = lrealpath (program_name);

  if (!exeval)
    {
      einfo (_("%P: warning: unable to get program name\n"));
      free (cwdval);
      return false;
    }

  pdbval = lrealpath (pdb_name);

  if (!pdbval)
    {
      einfo (_("%P: warning: unable to get full path to PDB\n"));
      free (exeval);
      free (cwdval);
      return false;
    }

  *sym_byte_size += offsetof (struct objname, name) + sizeof (linker_fn);
  *sym_byte_size += offsetof (struct compile3, compiler) + sizeof (linker_name);

  if (*sym_byte_size % 4)
    padding1 = 4 - (*sym_byte_size % 4);
  else
    padding1 = 0;

  *sym_byte_size += padding1;

  env_size = offsetof (struct envblock, strings);
  env_size += sizeof (cwd);
  env_size += strlen (cwdval) + 1;
  env_size += sizeof (exe);
  env_size += strlen (exeval) + 1;
  env_size += sizeof (pdb);
  env_size += strlen (pdbval) + 1;

  if (env_size % 4)
    padding2 = 4 - (env_size % 4);
  else
    padding2 = 0;

  env_size += padding2;

  *sym_byte_size += env_size;

  *syms = xmalloc (*sym_byte_size);
  ptr = *syms;

  /* Write S_OBJNAME */

  name = (struct objname *) ptr;
  bfd_putl16 (offsetof (struct objname, name)
	      + sizeof (linker_fn) - sizeof (uint16_t), &name->size);
  bfd_putl16 (S_OBJNAME, &name->kind);
  bfd_putl32 (0, &name->signature);
  memcpy (name->name, linker_fn, sizeof (linker_fn));

  ptr += offsetof (struct objname, name) + sizeof (linker_fn);

  /* Write S_COMPILE3 */

  comp = (struct compile3 *) ptr;

  bfd_putl16 (offsetof (struct compile3, compiler) + sizeof (linker_name)
	      + padding1 - sizeof (uint16_t), &comp->size);
  bfd_putl16 (S_COMPILE3, &comp->kind);
  bfd_putl32 (CV_CFL_LINK, &comp->flags);
  bfd_putl16 (target_processor (abfd), &comp->machine);
  bfd_putl16 (0, &comp->frontend_major);
  bfd_putl16 (0, &comp->frontend_minor);
  bfd_putl16 (0, &comp->frontend_build);
  bfd_putl16 (0, &comp->frontend_qfe);
  bfd_putl16 (0, &comp->backend_major);
  bfd_putl16 (0, &comp->backend_minor);
  bfd_putl16 (0, &comp->backend_build);
  bfd_putl16 (0, &comp->backend_qfe);
  memcpy (comp->compiler, linker_name, sizeof (linker_name));

  memset (comp->compiler + sizeof (linker_name), 0, padding1);

  ptr += offsetof (struct compile3, compiler) + sizeof (linker_name) + padding1;

  /* Write S_ENVBLOCK */

  env = (struct envblock *) ptr;

  bfd_putl16 (env_size - sizeof (uint16_t), &env->size);
  bfd_putl16 (S_ENVBLOCK, &env->kind);
  env->flags = 0;

  ptr += offsetof (struct envblock, strings);

  memcpy (ptr, cwd, sizeof (cwd));
  ptr += sizeof (cwd);
  memcpy (ptr, cwdval, strlen (cwdval) + 1);
  ptr += strlen (cwdval) + 1;

  memcpy (ptr, exe, sizeof (exe));
  ptr += sizeof (exe);
  memcpy (ptr, exeval, strlen (exeval) + 1);
  ptr += strlen (exeval) + 1;

  memcpy (ptr, pdb, sizeof (pdb));
  ptr += sizeof (pdb);
  memcpy (ptr, pdbval, strlen (pdbval) + 1);
  ptr += strlen (pdbval) + 1;

  /* Microsoft's LINK also includes "cmd", the command-line options passed
     to the linker, but unfortunately we don't have access to argc and argv
     at this stage.  */

  memset (ptr, 0, padding2);

  free (pdbval);
  free (exeval);
  free (cwdval);

  return true;
}

/* Populate the module stream, which consists of the transformed .debug$S
   data for each object file.  */
static bool
populate_module_stream (bfd *stream, bfd *mod, uint32_t *sym_byte_size,
			struct string_table *strings,
			uint32_t *c13_info_size,
			struct mod_source_files *mod_source,
			bfd *abfd, struct types *types,
			struct types *ids, uint16_t mod_num,
			bfd *sym_rec_stream, struct globals *glob,
			const char *pdb_name)
{
  uint8_t int_buf[sizeof (uint32_t)];
  uint8_t *c13_info = NULL;
  uint8_t *syms = NULL;

  *sym_byte_size = 0;
  *c13_info_size = 0;

  if (!strcmp (bfd_get_filename (mod), "dll stuff"))
    {
      if (!create_linker_symbols (mod, &syms, sym_byte_size, pdb_name))
	return false;
    }
  else
    {
      struct type_entry **map = NULL;
      uint32_t num_types = 0;

      /* Process .debug$T section.  */

      for (asection *s = mod->sections; s; s = s->next)
	{
	  if (!strcmp (s->name, ".debug$T") && s->size >= sizeof (uint32_t))
	    {
	      if (!handle_debugt_section (s, mod, types, ids, mod_num, strings,
					  &map, &num_types))
		{
		  free (mod_source->files);
		  return false;
		}

	      break;
	    }
	}

      /* Process .debug$S section(s).  */

      for (asection *s = mod->sections; s; s = s->next)
	{
	  if (!strcmp (s->name, ".debug$S") && s->size >= sizeof (uint32_t))
	    {
	      if (!handle_debugs_section (s, mod, strings, &c13_info,
					  c13_info_size, mod_source, abfd,
					  &syms, sym_byte_size, map, num_types,
					  sym_rec_stream, glob, mod_num))
		{
		  free (c13_info);
		  free (syms);
		  free (mod_source->files);
		  free (map);
		  return false;
		}
	    }
	}

      free (map);
    }

  /* Write the signature.  */

  bfd_putl32 (CV_SIGNATURE_C13, int_buf);

  if (bfd_bwrite (int_buf, sizeof (uint32_t), stream) != sizeof (uint32_t))
    {
      free (c13_info);
      free (syms);
      return false;
    }

  if (syms)
    {
      if (bfd_bwrite (syms, *sym_byte_size, stream) != *sym_byte_size)
	{
	  free (c13_info);
	  free (syms);
	  return false;
	}

      free (syms);
    }

  if (c13_info)
    {
      if (bfd_bwrite (c13_info, *c13_info_size, stream) != *c13_info_size)
	{
	  free (c13_info);
	  return false;
	}

      free (c13_info);
    }

  /* Write the global refs size.  */

  bfd_putl32 (0, int_buf);

  if (bfd_bwrite (int_buf, sizeof (uint32_t), stream) != sizeof (uint32_t))
    return false;

  return true;
}

/* Create the module info substream within the DBI.  */
static bool
create_module_info_substream (bfd *abfd, bfd *pdb, void **data,
			      uint32_t *size, struct string_table *strings,
			      struct source_files_info *source,
			      struct types *types, struct types *ids,
			      bfd *sym_rec_stream, struct globals *glob,
			      const char *pdb_name)
{
  uint8_t *ptr;
  unsigned int mod_num;

  static const char linker_fn[] = "* Linker *";

  *size = 0;

  for (bfd *in = coff_data (abfd)->link_info->input_bfds; in;
       in = in->link.next)
    {
      size_t len = sizeof (struct module_info);

      if (!strcmp (bfd_get_filename (in), "dll stuff"))
	{
	  len += sizeof (linker_fn); /* Object name.  */
	  len++; /* Empty module name.  */
	}
      else if (in->my_archive)
	{
	  char *name = lrealpath (bfd_get_filename (in));

	  len += strlen (name) + 1; /* Object name.  */

	  free (name);

	  name = lrealpath (bfd_get_filename (in->my_archive));

	  len += strlen (name) + 1; /* Archive name.  */

	  free (name);
	}
      else
	{
	  char *name = lrealpath (bfd_get_filename (in));
	  size_t name_len = strlen (name) + 1;

	  len += name_len; /* Object name.  */
	  len += name_len; /* And again as the archive name.  */

	  free (name);
	}

      if (len % 4)
	len += 4 - (len % 4);

      *size += len;

      source->mod_count++;
    }

  *data = xmalloc (*size);

  ptr = *data;

  source->mods = xmalloc (source->mod_count
			  * sizeof (struct mod_source_files));
  memset (source->mods, 0,
	  source->mod_count * sizeof (struct mod_source_files));

  mod_num = 0;

  for (bfd *in = coff_data (abfd)->link_info->input_bfds; in;
       in = in->link.next)
    {
      struct module_info *mod = (struct module_info *) ptr;
      uint16_t stream_num;
      bfd *stream;
      uint32_t sym_byte_size, c13_info_size;
      uint8_t *start = ptr;

      stream = add_stream (pdb, NULL, &stream_num);

      if (!stream)
	{
	  for (unsigned int i = 0; i < source->mod_count; i++)
	    {
	      free (source->mods[i].files);
	    }

	  free (source->mods);
	  free (*data);
	  return false;
	}

      if (!populate_module_stream (stream, in, &sym_byte_size,
				   strings, &c13_info_size,
				   &source->mods[mod_num], abfd,
				   types, ids, mod_num,
				   sym_rec_stream, glob, pdb_name))
	{
	  for (unsigned int i = 0; i < source->mod_count; i++)
	    {
	      free (source->mods[i].files);
	    }

	  free (source->mods);
	  free (*data);
	  return false;
	}

      bfd_putl32 (0, &mod->unused1);

      /* These are dummy values - MSVC copies the first section contribution
	 entry here, but doesn't seem to use it for anything.  */
      bfd_putl16 (0xffff, &mod->sc.section);
      bfd_putl16 (0, &mod->sc.padding1);
      bfd_putl32 (0, &mod->sc.offset);
      bfd_putl32 (0xffffffff, &mod->sc.size);
      bfd_putl32 (0, &mod->sc.characteristics);
      bfd_putl16 (0xffff, &mod->sc.module_index);
      bfd_putl16 (0, &mod->sc.padding2);
      bfd_putl32 (0, &mod->sc.data_crc);
      bfd_putl32 (0, &mod->sc.reloc_crc);

      bfd_putl16 (0, &mod->flags);
      bfd_putl16 (stream_num, &mod->module_sym_stream);
      bfd_putl32 (sizeof (uint32_t) + sym_byte_size, &mod->sym_byte_size);
      bfd_putl32 (0, &mod->c11_byte_size);
      bfd_putl32 (c13_info_size, &mod->c13_byte_size);
      bfd_putl16 (0, &mod->source_file_count);
      bfd_putl16 (0, &mod->padding);
      bfd_putl32 (0, &mod->unused2);
      bfd_putl32 (0, &mod->source_file_name_index);
      bfd_putl32 (0, &mod->pdb_file_path_name_index);

      ptr += sizeof (struct module_info);

      if (!strcmp (bfd_get_filename (in), "dll stuff"))
	{
	  /* Object name.  */
	  memcpy (ptr, linker_fn, sizeof (linker_fn));
	  ptr += sizeof (linker_fn);

	  /* Empty module name.  */
	  *ptr = 0;
	  ptr++;
	}
      else if (in->my_archive)
	{
	  char *name = lrealpath (bfd_get_filename (in));
	  size_t name_len = strlen (name) + 1;

	  /* Object name.  */
	  memcpy (ptr, name, name_len);
	  ptr += name_len;

	  free (name);

	  name = lrealpath (bfd_get_filename (in->my_archive));
	  name_len = strlen (name) + 1;

	  /* Archive name.  */
	  memcpy (ptr, name, name_len);
	  ptr += name_len;

	  free (name);
	}
      else
	{
	  char *name = lrealpath (bfd_get_filename (in));
	  size_t name_len = strlen (name) + 1;

	  /* Object name.  */
	  memcpy (ptr, name, name_len);
	  ptr += name_len;

	  /* Object name again as archive name.  */
	  memcpy (ptr, name, name_len);
	  ptr += name_len;

	  free (name);
	}

      /* Pad to next four-byte boundary.  */

      if ((ptr - start) % 4)
	{
	  memset (ptr, 0, 4 - ((ptr - start) % 4));
	  ptr += 4 - ((ptr - start) % 4);
	}

      mod_num++;
    }

  return true;
}

/* Return the index of a given output section.  */
static uint16_t
find_section_number (bfd *abfd, asection *sect)
{
  uint16_t i = 1;

  for (asection *s = abfd->sections; s; s = s->next)
    {
      if (s == sect)
	return i;

      /* Empty sections aren't output.  */
      if (s->size != 0)
	i++;
    }

  return 0;
}

/* Used as parameter to qsort, to sort section contributions by section and
   offset.  */
static int
section_contribs_compare (const void *p1, const void *p2)
{
  const struct in_sc *sc1 = p1;
  const struct in_sc *sc2 = p2;

  if (sc1->sect_num < sc2->sect_num)
    return -1;
  if (sc1->sect_num > sc2->sect_num)
    return 1;

  if (sc1->s->output_offset < sc2->s->output_offset)
    return -1;
  if (sc1->s->output_offset > sc2->s->output_offset)
    return 1;

  return 0;
}

/* Create the substream which maps addresses in the image file to locations
   in the original object files.  */
static bool
create_section_contrib_substream (bfd *abfd, void **data, uint32_t *size)
{
  unsigned int num_sc = 0;
  struct section_contribution *sc;
  uint16_t mod_index;
  char *sect_flags;
  file_ptr offset;
  struct in_sc *sc_in, *sc2;
  uint32_t *ptr;

  for (bfd *in = coff_data (abfd)->link_info->input_bfds; in;
       in = in->link.next)
    {
      for (asection *s = in->sections; s; s = s->next)
	{
	  if (s->size == 0 || discarded_section (s))
	    continue;

	  num_sc++;
	}
    }

  *size = sizeof (uint32_t) + (num_sc * sizeof (struct section_contribution));
  *data = xmalloc (*size);

  bfd_putl32 (SECTION_CONTRIB_VERSION_60, *data);

  /* Read characteristics of outputted sections.  */

  sect_flags = xmalloc (sizeof (uint32_t) * abfd->section_count);

  offset = bfd_coff_filhsz (abfd) + bfd_coff_aoutsz (abfd);
  offset += offsetof (struct external_scnhdr, s_flags);

  for (unsigned int i = 0; i < abfd->section_count; i++)
    {
      bfd_seek (abfd, offset, SEEK_SET);

      if (bfd_bread (sect_flags + (i * sizeof (uint32_t)), sizeof (uint32_t),
		     abfd) != sizeof (uint32_t))
	{
	  free (*data);
	  free (sect_flags);
	  return false;
	}

      offset += sizeof (struct external_scnhdr);
    }

  /* Microsoft's DIA expects section contributions to be sorted by section
     number and offset, otherwise it will be unable to resolve line numbers.  */

  sc_in = xmalloc (num_sc * sizeof (* sc_in));
  sc2 = sc_in;

  mod_index = 0;
  for (bfd *in = coff_data (abfd)->link_info->input_bfds; in;
       in = in->link.next)
    {
      for (asection *s = in->sections; s; s = s->next)
	{
	  if (s->size == 0 || discarded_section (s))
	    continue;

	  sc2->s = s;
	  sc2->sect_num = find_section_number (abfd, s->output_section);
	  sc2->mod_index = mod_index;

	  sc2++;
	}

      mod_index++;
    }

  qsort (sc_in, num_sc, sizeof (* sc_in), section_contribs_compare);

  ptr = *data;
  sc = (struct section_contribution *) (ptr + 1); /* Skip the version word.  */

  for (unsigned int i = 0; i < num_sc; i++)
    {
      memcpy (&sc->characteristics,
	      sect_flags + ((sc_in[i].sect_num - 1) * sizeof (uint32_t)),
	      sizeof (uint32_t));

      bfd_putl16 (sc_in[i].sect_num, &sc->section);
      bfd_putl16 (0, &sc->padding1);
      bfd_putl32 (sc_in[i].s->output_offset, &sc->offset);
      bfd_putl32 (sc_in[i].s->size, &sc->size);
      bfd_putl16 (sc_in[i].mod_index, &sc->module_index);
      bfd_putl16 (0, &sc->padding2);
      bfd_putl32 (0, &sc->data_crc);
      bfd_putl32 (0, &sc->reloc_crc);

      sc++;
    }

  free (sc_in);
  free (sect_flags);

  return true;
}

/* The source info substream lives within the DBI stream, and lists the
   source files for each object file (i.e. it's derived from the
   DEBUG_S_FILECHKSMS parts of the .debug$S sections).  This is a bit
   superfluous, as the filenames are also available in the C13 parts of
   the module streams, but MSVC relies on it to work properly.  */
static void
create_source_info_substream (void **data, uint32_t *size,
			      struct source_files_info *source)
{
  uint16_t dedupe_source_files_count = 0;
  uint16_t source_files_count = 0;
  uint32_t strings_len = 0;
  uint8_t *ptr;

  /* Loop through the source files, marking unique filenames.  The pointers
     here are for entries in the main string table, and so have already
     been deduplicated.  */

  for (uint16_t i = 0; i < source->mod_count; i++)
    {
      for (uint16_t j = 0; j < source->mods[i].files_count; j++)
	{
	  if (source->mods[i].files[j])
	    {
	      if (source->mods[i].files[j]->source_file_offset == 0xffffffff)
		{
		  source->mods[i].files[j]->source_file_offset = strings_len;
		  strings_len += source->mods[i].files[j]->len + 1;
		  dedupe_source_files_count++;
		}

	      source_files_count++;
	    }
	}
    }

  *size = sizeof (uint16_t) + sizeof (uint16_t);
  *size += (sizeof (uint16_t) + sizeof (uint16_t)) * source->mod_count;
  *size += sizeof (uint32_t) * source_files_count;
  *size += strings_len;

  *data = xmalloc (*size);

  ptr = (uint8_t *) *data;

  /* Write header (module count and source file count).  */

  bfd_putl16 (source->mod_count, ptr);
  ptr += sizeof (uint16_t);

  bfd_putl16 (dedupe_source_files_count, ptr);
  ptr += sizeof (uint16_t);

  /* Write "ModIndices".  As the LLVM documentation puts it, "this array is
     present, but does not appear to be useful".  */

  for (uint16_t i = 0; i < source->mod_count; i++)
    {
      bfd_putl16 (i, ptr);
      ptr += sizeof (uint16_t);
    }

  /* Write source file count for each module.  */

  for (uint16_t i = 0; i < source->mod_count; i++)
    {
      bfd_putl16 (source->mods[i].files_count, ptr);
      ptr += sizeof (uint16_t);
    }

  /* For each module, write the offsets within the string table
     for each source file.  */

  for (uint16_t i = 0; i < source->mod_count; i++)
    {
      for (uint16_t j = 0; j < source->mods[i].files_count; j++)
	{
	  if (source->mods[i].files[j])
	    {
	      bfd_putl32 (source->mods[i].files[j]->source_file_offset, ptr);
	      ptr += sizeof (uint32_t);
	    }
	}
    }

  /* Write the string table.  We set source_file_offset to a dummy value for
     each entry we write, so we don't write duplicate filenames.  */

  for (uint16_t i = 0; i < source->mod_count; i++)
    {
      for (uint16_t j = 0; j < source->mods[i].files_count; j++)
	{
	  if (source->mods[i].files[j]
	      && source->mods[i].files[j]->source_file_offset != 0xffffffff)
	    {
	      memcpy (ptr, source->mods[i].files[j]->s,
		      source->mods[i].files[j]->len);
	      ptr += source->mods[i].files[j]->len;

	      *ptr = 0;
	      ptr++;

	      source->mods[i].files[j]->source_file_offset = 0xffffffff;
	    }
	}
    }
}

/* Used as parameter to qsort, to sort globals by hash.  */
static int
global_compare_hash (const void *s1, const void *s2)
{
  const struct global *g1 = *(const struct global **) s1;
  const struct global *g2 = *(const struct global **) s2;

  if (g1->hash < g2->hash)
    return -1;
  if (g1->hash > g2->hash)
    return 1;

  return 0;
}

/* Create the globals stream, which contains the unmangled symbol names.  */
static bool
create_globals_stream (bfd *pdb, struct globals *glob, uint16_t *stream_num)
{
  bfd *stream;
  struct globals_hash_header h;
  uint32_t buckets_size, filled_buckets = 0;
  struct global **sorted = NULL;
  bool ret = false;
  struct global *buckets[NUM_GLOBALS_HASH_BUCKETS];
  char int_buf[sizeof (uint32_t)];

  stream = add_stream (pdb, NULL, stream_num);
  if (!stream)
    return false;

  memset (buckets, 0, sizeof (buckets));

  if (glob->num_entries > 0)
    {
      struct global *g;

      /* Create an array of pointers, sorted by hash value.  */

      sorted = xmalloc (sizeof (struct global *) * glob->num_entries);

      g = glob->first;
      for (unsigned int i = 0; i < glob->num_entries; i++)
	{
	  sorted[i] = g;
	  g = g->next;
	}

      qsort (sorted, glob->num_entries, sizeof (struct global *),
	     global_compare_hash);

      /* Populate the buckets.  */

      for (unsigned int i = 0; i < glob->num_entries; i++)
	{
	  if (!buckets[sorted[i]->hash])
	    {
	      buckets[sorted[i]->hash] = sorted[i];
	      filled_buckets++;
	    }

	  sorted[i]->index = i;
	}
    }

  buckets_size = NUM_GLOBALS_HASH_BUCKETS / 8;
  buckets_size += sizeof (uint32_t);
  buckets_size += filled_buckets * sizeof (uint32_t);

  bfd_putl32 (GLOBALS_HASH_SIGNATURE, &h.signature);
  bfd_putl32 (GLOBALS_HASH_VERSION_70, &h.version);
  bfd_putl32 (glob->num_entries * sizeof (struct hash_record),
	      &h.entries_size);
  bfd_putl32 (buckets_size, &h.buckets_size);

  if (bfd_bwrite (&h, sizeof (h), stream) != sizeof (h))
    return false;

  /* Write hash entries, sorted by hash.  */

  for (unsigned int i = 0; i < glob->num_entries; i++)
    {
      struct hash_record hr;

      bfd_putl32 (sorted[i]->offset + 1, &hr.offset);
      bfd_putl32 (sorted[i]->refcount, &hr.reference);

      if (bfd_bwrite (&hr, sizeof (hr), stream) != sizeof (hr))
	goto end;
    }

  /* Write the bitmap for filled and unfilled buckets.  */

  for (unsigned int i = 0; i < NUM_GLOBALS_HASH_BUCKETS; i += 8)
    {
      uint8_t v = 0;

      for (unsigned int j = 0; j < 8; j++)
	{
	  if (buckets[i + j])
	    v |= 1 << j;
	}

      if (bfd_bwrite (&v, sizeof (v), stream) != sizeof (v))
	goto end;
    }

  /* Add a 4-byte gap.  */

  bfd_putl32 (0, int_buf);

  if (bfd_bwrite (int_buf, sizeof (uint32_t), stream) != sizeof (uint32_t))
    goto end;

  /* Write the bucket offsets.  */

  for (unsigned int i = 0; i < NUM_GLOBALS_HASH_BUCKETS; i++)
    {
      if (buckets[i])
	{
	  /* 0xc is size of internal hash_record structure in
	     Microsoft's parser.  */
	  bfd_putl32 (buckets[i]->index * 0xc, int_buf);

	  if (bfd_bwrite (int_buf, sizeof (uint32_t), stream) !=
	      sizeof (uint32_t))
	    goto end;
	}
    }

  ret = true;

end:
  free (sorted);

  return ret;
}

/* Hash an entry in the globals list.  */
static hashval_t
hash_global_entry (const void *p)
{
  const struct global *g = (const struct global *) p;
  uint16_t len = bfd_getl16 (g->data);

  return iterative_hash (g->data, len, 0);
}

/* Compare an entry in the globals list with a symbol.  */
static int
eq_global_entry (const void *a, const void *b)
{
  const struct global *g = (const struct global *) a;
  uint16_t len1, len2;

  len1 = bfd_getl16 (g->data) + sizeof (uint16_t);
  len2 = bfd_getl16 (b) + sizeof (uint16_t);

  if (len1 != len2)
    return 0;

  return !memcmp (g->data, b, len1);
}

/* Stream 4 is the debug information (DBI) stream.  */
static bool
populate_dbi_stream (bfd *stream, bfd *abfd, bfd *pdb,
		     uint16_t section_header_stream_num,
		     uint16_t sym_rec_stream_num,
		     uint16_t publics_stream_num,
		     struct string_table *strings,
		     struct types *types,
		     struct types *ids,
		     bfd *sym_rec_stream, const char *pdb_name)
{
  struct pdb_dbi_stream_header h;
  struct optional_dbg_header opt;
  void *mod_info, *sc, *source_info;
  uint32_t mod_info_size, sc_size, source_info_size;
  struct source_files_info source;
  struct globals glob;
  uint16_t globals_stream_num;

  source.mod_count = 0;
  source.mods = NULL;

  glob.num_entries = 0;
  glob.first = NULL;
  glob.last = NULL;

  glob.hashmap = htab_create_alloc (0, hash_global_entry,
				    eq_global_entry, free, xcalloc, free);

  if (!create_module_info_substream (abfd, pdb, &mod_info, &mod_info_size,
				     strings, &source, types, ids,
				     sym_rec_stream, &glob, pdb_name))
    {
      htab_delete (glob.hashmap);
      return false;
    }

  if (!create_globals_stream (pdb, &glob, &globals_stream_num))
    {
      htab_delete (glob.hashmap);

      for (unsigned int i = 0; i < source.mod_count; i++)
	{
	  free (source.mods[i].files);
	}
      free (source.mods);

      free (mod_info);
      return false;
    }

  htab_delete (glob.hashmap);

  if (!create_section_contrib_substream (abfd, &sc, &sc_size))
    {
      for (unsigned int i = 0; i < source.mod_count; i++)
	{
	  free (source.mods[i].files);
	}
      free (source.mods);

      free (mod_info);
      return false;
    }

  create_source_info_substream (&source_info, &source_info_size, &source);

  for (unsigned int i = 0; i < source.mod_count; i++)
    {
      free (source.mods[i].files);
    }
  free (source.mods);

  bfd_putl32 (0xffffffff, &h.version_signature);
  bfd_putl32 (DBI_STREAM_VERSION_70, &h.version_header);
  bfd_putl32 (1, &h.age);
  bfd_putl16 (globals_stream_num, &h.global_stream_index);
  bfd_putl16 (0x8e1d, &h.build_number); // MSVC 14.29
  bfd_putl16 (publics_stream_num, &h.public_stream_index);
  bfd_putl16 (0, &h.pdb_dll_version);
  bfd_putl16 (sym_rec_stream_num, &h.sym_record_stream);
  bfd_putl16 (0, &h.pdb_dll_rbld);
  bfd_putl32 (mod_info_size, &h.mod_info_size);
  bfd_putl32 (sc_size, &h.section_contribution_size);
  bfd_putl32 (0, &h.section_map_size);
  bfd_putl32 (source_info_size, &h.source_info_size);
  bfd_putl32 (0, &h.type_server_map_size);
  bfd_putl32 (0, &h.mfc_type_server_index);
  bfd_putl32 (sizeof (opt), &h.optional_dbg_header_size);
  bfd_putl32 (0, &h.ec_substream_size);
  bfd_putl16 (0, &h.flags);
  bfd_putl16 (get_arch_number (abfd), &h.machine);
  bfd_putl32 (0, &h.padding);

  if (bfd_bwrite (&h, sizeof (h), stream) != sizeof (h))
    {
      free (source_info);
      free (sc);
      free (mod_info);
      return false;
    }

  if (bfd_bwrite (mod_info, mod_info_size, stream) != mod_info_size)
    {
      free (source_info);
      free (sc);
      free (mod_info);
      return false;
    }

  free (mod_info);

  if (bfd_bwrite (sc, sc_size, stream) != sc_size)
    {
      free (source_info);
      free (sc);
      return false;
    }

  free (sc);

  if (bfd_bwrite (source_info, source_info_size, stream) != source_info_size)
    {
      free (source_info);
      return false;
    }

  free (source_info);

  bfd_putl16 (0xffff, &opt.fpo_stream);
  bfd_putl16 (0xffff, &opt.exception_stream);
  bfd_putl16 (0xffff, &opt.fixup_stream);
  bfd_putl16 (0xffff, &opt.omap_to_src_stream);
  bfd_putl16 (0xffff, &opt.omap_from_src_stream);
  bfd_putl16 (section_header_stream_num, &opt.section_header_stream);
  bfd_putl16 (0xffff, &opt.token_map_stream);
  bfd_putl16 (0xffff, &opt.xdata_stream);
  bfd_putl16 (0xffff, &opt.pdata_stream);
  bfd_putl16 (0xffff, &opt.new_fpo_stream);
  bfd_putl16 (0xffff, &opt.orig_section_header_stream);

  if (bfd_bwrite (&opt, sizeof (opt), stream) != sizeof (opt))
    return false;

  return true;
}

/* Used as parameter to qsort, to sort publics by hash.  */
static int
public_compare_hash (const void *s1, const void *s2)
{
  const struct public *p1 = *(const struct public **) s1;
  const struct public *p2 = *(const struct public **) s2;

  if (p1->hash < p2->hash)
    return -1;
  if (p1->hash > p2->hash)
    return 1;

  return 0;
}

/* Used as parameter to qsort, to sort publics by address.  */
static int
public_compare_addr (const void *s1, const void *s2)
{
  const struct public *p1 = *(const struct public **) s1;
  const struct public *p2 = *(const struct public **) s2;

  if (p1->section < p2->section)
    return -1;
  if (p1->section > p2->section)
    return 1;

  if (p1->address < p2->address)
    return -1;
  if (p1->address > p2->address)
    return 1;

  return 0;
}

/* The publics stream is a hash map of S_PUB32 records, which are stored
   in the symbol record stream.  Each S_PUB32 entry represents a symbol
   from the point of view of the linker: a section index, an offset within
   the section, and a mangled name.  Compare with S_GDATA32 and S_GPROC32,
   which are the same thing but generated by the compiler.  */
static bool
populate_publics_stream (bfd *stream, bfd *abfd, bfd *sym_rec_stream)
{
  struct publics_header header;
  struct globals_hash_header hash_header;
  const unsigned int num_buckets = 4096;
  unsigned int num_entries = 0, filled_buckets = 0;
  unsigned int buckets_size, sym_hash_size;
  char int_buf[sizeof (uint32_t)];
  struct public *publics_head = NULL, *publics_tail = NULL;
  struct public **buckets;
  struct public **sorted = NULL;
  bool ret = false;

  buckets = xmalloc (sizeof (struct public *) * num_buckets);
  memset (buckets, 0, sizeof (struct public *) * num_buckets);

  /* Loop through the global symbols in our input files, and write S_PUB32
     records in the symbol record stream for those that make it into the
     final image.  */
  for (bfd *in = coff_data (abfd)->link_info->input_bfds; in;
       in = in->link.next)
    {
      if (!in->outsymbols)
	continue;

      for (unsigned int i = 0; i < in->symcount; i++)
	{
	  struct bfd_symbol *sym = in->outsymbols[i];

	  if (sym->flags & BSF_GLOBAL)
	    {
	      struct pubsym ps;
	      uint16_t record_length;
	      const char *name = sym->name;
	      size_t name_len = strlen (name);
	      struct public *p = xmalloc (sizeof (struct public));
	      unsigned int padding = 0;
	      uint16_t section;
	      uint32_t flags = 0;

	      section =
		find_section_number (abfd, sym->section->output_section);

	      if (section == 0)
		continue;

	      p->next = NULL;
	      p->offset = bfd_tell (sym_rec_stream);
	      p->hash = calc_hash (name, name_len) % num_buckets;
	      p->section = section;
	      p->address = sym->section->output_offset + sym->value;

	      record_length = sizeof (struct pubsym) + name_len + 1;

	      if (record_length % 4)
		padding = 4 - (record_length % 4);

	      /* Assume that all global symbols in executable sections
		 are functions.  */
	      if (sym->section->flags & SEC_CODE)
		flags = PUBSYM_FUNCTION;

	      bfd_putl16 (record_length + padding - sizeof (uint16_t),
			  &ps.record_length);
	      bfd_putl16 (S_PUB32, &ps.record_type);
	      bfd_putl32 (flags, &ps.flags);
	      bfd_putl32 (p->address, &ps.offset);
	      bfd_putl16 (p->section, &ps.section);

	      if (bfd_bwrite (&ps, sizeof (struct pubsym), sym_rec_stream) !=
		  sizeof (struct pubsym))
		goto end;

	      if (bfd_bwrite (name, name_len + 1, sym_rec_stream) !=
		  name_len + 1)
		goto end;

	      for (unsigned int j = 0; j < padding; j++)
		{
		  uint8_t b = 0;

		  if (bfd_bwrite (&b, sizeof (uint8_t), sym_rec_stream) !=
		      sizeof (uint8_t))
		    goto end;
		}

	      if (!publics_head)
		publics_head = p;
	      else
		publics_tail->next = p;

	      publics_tail = p;
	      num_entries++;
	    }
	}
    }


  if (num_entries > 0)
    {
      /* Create an array of pointers, sorted by hash value.  */

      sorted = xmalloc (sizeof (struct public *) * num_entries);

      struct public *p = publics_head;
      for (unsigned int i = 0; i < num_entries; i++)
	{
	  sorted[i] = p;
	  p = p->next;
	}

      qsort (sorted, num_entries, sizeof (struct public *),
	     public_compare_hash);

      /* Populate the buckets.  */

      for (unsigned int i = 0; i < num_entries; i++)
	{
	  if (!buckets[sorted[i]->hash])
	    {
	      buckets[sorted[i]->hash] = sorted[i];
	      filled_buckets++;
	    }

	  sorted[i]->index = i;
	}
    }

  buckets_size = num_buckets / 8;
  buckets_size += sizeof (uint32_t);
  buckets_size += filled_buckets * sizeof (uint32_t);

  sym_hash_size = sizeof (hash_header);
  sym_hash_size += num_entries * sizeof (struct hash_record);
  sym_hash_size += buckets_size;

  /* Output the publics header.  */

  bfd_putl32 (sym_hash_size, &header.sym_hash_size);
  bfd_putl32 (num_entries * sizeof (uint32_t), &header.addr_map_size);
  bfd_putl32 (0, &header.num_thunks);
  bfd_putl32 (0, &header.thunks_size);
  bfd_putl32 (0, &header.thunk_table);
  bfd_putl32 (0, &header.thunk_table_offset);
  bfd_putl32 (0, &header.num_sects);

  if (bfd_bwrite (&header, sizeof (header), stream) != sizeof (header))
    goto end;

  /* Output the global hash header.  */

  bfd_putl32 (GLOBALS_HASH_SIGNATURE, &hash_header.signature);
  bfd_putl32 (GLOBALS_HASH_VERSION_70, &hash_header.version);
  bfd_putl32 (num_entries * sizeof (struct hash_record),
	      &hash_header.entries_size);
  bfd_putl32 (buckets_size, &hash_header.buckets_size);

  if (bfd_bwrite (&hash_header, sizeof (hash_header), stream) !=
      sizeof (hash_header))
    goto end;

  /* Write the entries in hash order.  */

  for (unsigned int i = 0; i < num_entries; i++)
    {
      struct hash_record hr;

      bfd_putl32 (sorted[i]->offset + 1, &hr.offset);
      bfd_putl32 (1, &hr.reference);

      if (bfd_bwrite (&hr, sizeof (hr), stream) != sizeof (hr))
	goto end;
    }

  /* Write the bitmap for filled and unfilled buckets.  */

  for (unsigned int i = 0; i < num_buckets; i += 8)
    {
      uint8_t v = 0;

      for (unsigned int j = 0; j < 8; j++)
	{
	  if (buckets[i + j])
	    v |= 1 << j;
	}

      if (bfd_bwrite (&v, sizeof (v), stream) != sizeof (v))
	goto end;
    }

  /* Add a 4-byte gap.  */

  bfd_putl32 (0, int_buf);

  if (bfd_bwrite (int_buf, sizeof (uint32_t), stream) != sizeof (uint32_t))
    goto end;

  /* Write the bucket offsets.  */

  for (unsigned int i = 0; i < num_buckets; i++)
    {
      if (buckets[i])
	{
	  /* 0xc is size of internal hash_record structure in
	     Microsoft's parser.  */
	  bfd_putl32 (buckets[i]->index * 0xc, int_buf);

	  if (bfd_bwrite (int_buf, sizeof (uint32_t), stream) !=
	      sizeof (uint32_t))
	    goto end;
	}
    }

  /* Write the address map: offsets into the symbol record stream of
     S_PUB32 records, ordered by address.  */

  if (num_entries > 0)
    {
      qsort (sorted, num_entries, sizeof (struct public *),
	     public_compare_addr);

      for (unsigned int i = 0; i < num_entries; i++)
	{
	  bfd_putl32 (sorted[i]->offset, int_buf);

	  if (bfd_bwrite (int_buf, sizeof (uint32_t), stream) !=
	      sizeof (uint32_t))
	    goto end;
	}
    }

  ret = true;

end:
  free (buckets);

  while (publics_head)
    {
      struct public *p = publics_head->next;

      free (publics_head);
      publics_head = p;
    }

  free (sorted);

  return ret;
}

/* The section header stream contains a copy of the section headers
   from the PE file, in the same format.  */
static bool
create_section_header_stream (bfd *pdb, bfd *abfd, uint16_t *num)
{
  bfd *stream;
  unsigned int section_count;
  file_ptr scn_base;
  size_t len;
  char *buf;

  stream = add_stream (pdb, NULL, num);
  if (!stream)
    return false;

  section_count = abfd->section_count;

  /* Empty sections aren't output.  */
  for (asection *sect = abfd->sections; sect; sect = sect->next)
    {
      if (sect->size == 0)
	section_count--;
    }

  if (section_count == 0)
    return true;

  /* Copy section table from output - it's already been written at this
     point.  */

  scn_base = bfd_coff_filhsz (abfd) + bfd_coff_aoutsz (abfd);

  bfd_seek (abfd, scn_base, SEEK_SET);

  len = section_count * sizeof (struct external_scnhdr);
  buf = xmalloc (len);

  if (bfd_bread (buf, len, abfd) != len)
    {
      free (buf);
      return false;
    }

  if (bfd_bwrite (buf, len, stream) != len)
    {
      free (buf);
      return false;
    }

  free (buf);

  return true;
}

/* Populate the "/names" named stream, which contains the string table.  */
static bool
populate_names_stream (bfd *stream, struct string_table *strings)
{
  char int_buf[sizeof (uint32_t)];
  struct string_table_header h;
  uint32_t num_strings = 0, num_buckets;
  struct string **buckets;

  bfd_putl32 (STRING_TABLE_SIGNATURE, &h.signature);
  bfd_putl32 (STRING_TABLE_VERSION, &h.version);

  if (bfd_bwrite (&h, sizeof (h), stream) != sizeof (h))
    return false;

  bfd_putl32 (strings->strings_len, int_buf);

  if (bfd_bwrite (int_buf, sizeof (uint32_t), stream) != sizeof (uint32_t))
    return false;

  int_buf[0] = 0;

  if (bfd_bwrite (int_buf, 1, stream) != 1)
    return false;

  for (struct string *s = strings->strings_head; s; s = s->next)
    {
      if (bfd_bwrite (s->s, s->len, stream) != s->len)
	return false;

      if (bfd_bwrite (int_buf, 1, stream) != 1)
	return false;

      num_strings++;
    }

  num_buckets = num_strings * 2;

  buckets = xmalloc (sizeof (struct string *) * num_buckets);
  memset (buckets, 0, sizeof (struct string *) * num_buckets);

  for (struct string *s = strings->strings_head; s; s = s->next)
    {
      uint32_t bucket_num = s->hash % num_buckets;

      while (buckets[bucket_num])
	{
	  bucket_num++;

	  if (bucket_num == num_buckets)
	    bucket_num = 0;
	}

      buckets[bucket_num] = s;
    }

  bfd_putl32 (num_buckets, int_buf);

  if (bfd_bwrite (int_buf, sizeof (uint32_t), stream) != sizeof (uint32_t))
    {
      free (buckets);
      return false;
    }

  for (unsigned int i = 0; i < num_buckets; i++)
    {
      if (buckets[i])
	bfd_putl32 (buckets[i]->offset, int_buf);
      else
	bfd_putl32 (0, int_buf);

      if (bfd_bwrite (int_buf, sizeof (uint32_t), stream) !=
	  sizeof (uint32_t))
	{
	  free (buckets);
	  return false;
	}
    }

  free (buckets);

  bfd_putl32 (num_strings, int_buf);

  if (bfd_bwrite (int_buf, sizeof (uint32_t), stream) != sizeof (uint32_t))
    return false;

  return true;
}

/* Calculate the hash of a type_entry.  */
static hashval_t
hash_type_entry (const void *p)
{
  const struct type_entry *e = (const struct type_entry *) p;
  uint16_t size = bfd_getl16 (e->data) + sizeof (uint16_t);

  return iterative_hash (e->data, size, 0);
}

/* Compare a type_entry with a type.  */
static int
eq_type_entry (const void *a, const void *b)
{
  const struct type_entry *e = (const struct type_entry *) a;
  uint16_t size_a = bfd_getl16 (e->data);
  uint16_t size_b = bfd_getl16 (b);

  if (size_a != size_b)
    return 0;

  return memcmp (e->data + sizeof (uint16_t),
		 (const uint8_t *) b + sizeof (uint16_t), size_a) == 0;
}

/* Create a PDB debugging file for the PE image file abfd with the build ID
   guid, stored at pdb_name.  */
bool
create_pdb_file (bfd *abfd, const char *pdb_name, const unsigned char *guid)
{
  bfd *pdb;
  bool ret = false;
  bfd *info_stream, *dbi_stream, *names_stream, *sym_rec_stream,
    *publics_stream, *tpi_stream, *ipi_stream;
  uint16_t section_header_stream_num, sym_rec_stream_num, publics_stream_num;
  struct string_table strings;
  struct types types, ids;

  pdb = bfd_openw (pdb_name, "pdb");
  if (!pdb)
    {
      einfo (_("%P: warning: cannot create PDB file: %E\n"));
      return false;
    }

  strings.strings_head = NULL;
  strings.strings_tail = NULL;
  strings.strings_len = 1;
  strings.hashmap = htab_create_alloc (0, hash_string_table_entry,
				       eq_string_table_entry, free,
				       xcalloc, free);

  bfd_set_format (pdb, bfd_archive);

  if (!create_old_directory_stream (pdb))
    {
      einfo (_("%P: warning: cannot create old directory stream "
	       "in PDB file: %E\n"));
      goto end;
    }

  info_stream = add_stream (pdb, NULL, NULL);

  if (!info_stream)
    {
      einfo (_("%P: warning: cannot create info stream "
	       "in PDB file: %E\n"));
      goto end;
    }

  tpi_stream = add_stream (pdb, NULL, NULL);

  if (!tpi_stream)
    {
      einfo (_("%P: warning: cannot create TPI stream "
	       "in PDB file: %E\n"));
      goto end;
    }

  dbi_stream = add_stream (pdb, NULL, NULL);

  if (!dbi_stream)
    {
      einfo (_("%P: warning: cannot create DBI stream "
	       "in PDB file: %E\n"));
      goto end;
    }

  ipi_stream = add_stream (pdb, NULL, NULL);

  if (!ipi_stream)
    {
      einfo (_("%P: warning: cannot create IPI stream "
	       "in PDB file: %E\n"));
      goto end;
    }

  names_stream = add_stream (pdb, "/names", NULL);

  if (!names_stream)
    {
      einfo (_("%P: warning: cannot create /names stream "
	       "in PDB file: %E\n"));
      goto end;
    }

  sym_rec_stream = add_stream (pdb, NULL, &sym_rec_stream_num);

  if (!sym_rec_stream)
    {
      einfo (_("%P: warning: cannot create symbol record stream "
	       "in PDB file: %E\n"));
      goto end;
    }

  publics_stream = add_stream (pdb, NULL, &publics_stream_num);

  if (!publics_stream)
    {
      einfo (_("%P: warning: cannot create publics stream "
	       "in PDB file: %E\n"));
      goto end;
    }

  if (!create_section_header_stream (pdb, abfd, &section_header_stream_num))
    {
      einfo (_("%P: warning: cannot create section header stream "
	       "in PDB file: %E\n"));
      goto end;
    }

  types.num_types = 0;
  types.hashmap = htab_create_alloc (0, hash_type_entry, eq_type_entry,
				     free, xcalloc, free);
  types.first = types.last = NULL;

  ids.num_types = 0;
  ids.hashmap = htab_create_alloc (0, hash_type_entry, eq_type_entry,
				   free, xcalloc, free);
  ids.first = ids.last = NULL;

  if (!populate_dbi_stream (dbi_stream, abfd, pdb, section_header_stream_num,
			    sym_rec_stream_num, publics_stream_num,
			    &strings, &types, &ids, sym_rec_stream, pdb_name))
    {
      einfo (_("%P: warning: cannot populate DBI stream "
	       "in PDB file: %E\n"));
      htab_delete (types.hashmap);
      htab_delete (ids.hashmap);
      goto end;
    }

  if (!populate_type_stream (pdb, tpi_stream, &types))
    {
      einfo (_("%P: warning: cannot populate TPI stream "
	       "in PDB file: %E\n"));
      htab_delete (types.hashmap);
      htab_delete (ids.hashmap);
      goto end;
    }

  htab_delete (types.hashmap);

  if (!populate_type_stream (pdb, ipi_stream, &ids))
    {
      einfo (_("%P: warning: cannot populate IPI stream "
	       "in PDB file: %E\n"));
      htab_delete (ids.hashmap);
      goto end;
    }

  htab_delete (ids.hashmap);

  add_string ("", 0, &strings);

  if (!populate_names_stream (names_stream, &strings))
    {
      einfo (_("%P: warning: cannot populate names stream "
	       "in PDB file: %E\n"));
      goto end;
    }

  if (!populate_publics_stream (publics_stream, abfd, sym_rec_stream))
    {
      einfo (_("%P: warning: cannot populate publics stream "
	       "in PDB file: %E\n"));
      goto end;
    }

  if (!populate_info_stream (pdb, info_stream, guid))
    {
      einfo (_("%P: warning: cannot populate info stream "
	       "in PDB file: %E\n"));
      goto end;
    }

  ret = true;

end:
  bfd_close (pdb);

  htab_delete (strings.hashmap);

  return ret;
}
