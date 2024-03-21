/* Test of <inttypes.h> substitute.
   Copyright (C) 2006-2007, 2009-2023 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Written by Bruno Haible <bruno@clisp.org>, 2007.  */

#include <config.h>

#include <inttypes.h>

/* Tests for macros supposed to be defined in inttypes.h.  */

const char *k = /* implicit string concatenation */
#ifdef INT8_MAX
  PRId8 PRIi8
#endif
#ifdef UINT8_MAX
  PRIo8 PRIu8 PRIx8 PRIX8
#endif
#ifdef INT16_MAX
  PRId16 PRIi16
#endif
#ifdef UINT16_MAX
  PRIo16 PRIu16 PRIx16 PRIX16
#endif
#ifdef INT32_MAX
  PRId32 PRIi32
#endif
#ifdef UINT32_MAX
  PRIo32 PRIu32 PRIx32 PRIX32
#endif
#ifdef INT64_MAX
  PRId64 PRIi64
#endif
#ifdef UINT64_MAX
  PRIo64 PRIu64 PRIx64 PRIX64
#endif
  PRIdLEAST8 PRIiLEAST8 PRIoLEAST8 PRIuLEAST8 PRIxLEAST8 PRIXLEAST8
  PRIdLEAST16 PRIiLEAST16 PRIoLEAST16 PRIuLEAST16 PRIxLEAST16 PRIXLEAST16
  PRIdLEAST32 PRIiLEAST32 PRIoLEAST32 PRIuLEAST32 PRIxLEAST32 PRIXLEAST32
  PRIdLEAST64 PRIiLEAST64
  PRIoLEAST64 PRIuLEAST64 PRIxLEAST64 PRIXLEAST64
  PRIdFAST8 PRIiFAST8 PRIoFAST8 PRIuFAST8 PRIxFAST8 PRIXFAST8
  PRIdFAST16 PRIiFAST16 PRIoFAST16 PRIuFAST16 PRIxFAST16 PRIXFAST16
  PRIdFAST32 PRIiFAST32 PRIoFAST32 PRIuFAST32 PRIxFAST32 PRIXFAST32
  PRIdFAST64 PRIiFAST64
  PRIoFAST64 PRIuFAST64 PRIxFAST64 PRIXFAST64
  PRIdMAX PRIiMAX PRIoMAX PRIuMAX PRIxMAX PRIXMAX
#ifdef INTPTR_MAX
  PRIdPTR PRIiPTR
#endif
#ifdef UINTPTR_MAX
  PRIoPTR PRIuPTR PRIxPTR PRIXPTR
#endif
  ;
const char *l = /* implicit string concatenation */
#ifdef INT8_MAX
  SCNd8 SCNi8
#endif
#ifdef UINT8_MAX
  SCNo8 SCNu8 SCNx8
#endif
#ifdef INT16_MAX
  SCNd16 SCNi16
#endif
#ifdef UINT16_MAX
  SCNo16 SCNu16 SCNx16
#endif
#ifdef INT32_MAX
  SCNd32 SCNi32
#endif
#ifdef UINT32_MAX
  SCNo32 SCNu32 SCNx32
#endif
#ifdef INT64_MAX
  SCNd64 SCNi64
#endif
#ifdef UINT64_MAX
  SCNo64 SCNu64 SCNx64
#endif
  SCNdLEAST8 SCNiLEAST8 SCNoLEAST8 SCNuLEAST8 SCNxLEAST8
  SCNdLEAST16 SCNiLEAST16 SCNoLEAST16 SCNuLEAST16 SCNxLEAST16
  SCNdLEAST32 SCNiLEAST32 SCNoLEAST32 SCNuLEAST32 SCNxLEAST32
  SCNdLEAST64 SCNiLEAST64
  SCNoLEAST64 SCNuLEAST64 SCNxLEAST64
  SCNdFAST8 SCNiFAST8 SCNoFAST8 SCNuFAST8 SCNxFAST8
  SCNdFAST16 SCNiFAST16 SCNoFAST16 SCNuFAST16 SCNxFAST16
  SCNdFAST32 SCNiFAST32 SCNoFAST32 SCNuFAST32 SCNxFAST32
  SCNdFAST64 SCNiFAST64
  SCNoFAST64 SCNuFAST64 SCNxFAST64
  SCNdMAX SCNiMAX SCNoMAX SCNuMAX SCNxMAX
#ifdef INTPTR_MAX
  SCNdPTR SCNiPTR
#endif
#ifdef UINTPTR_MAX
  SCNoPTR SCNuPTR SCNxPTR
#endif
  ;

int
main (void)
{
  return 0;
}
