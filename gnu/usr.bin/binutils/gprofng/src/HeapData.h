/* Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Oracle.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#ifndef _HEAPDATA_H
#define _HEAPDATA_H

#include "gp-defs.h"
#include "gp-time.h"

#include "vec.h"
#include "data_pckts.h"
#include "Histable.h"

#define TOTAL_HEAPNAME      NTXT("<Total>")
#define TOTAL_STACK_ID      0
#define _1KB                1024
#define _8KB                8192
#define _32KB               32768
#define _128KB              131072
#define _256KB              262144
#define _512KB              524288
#define _1000KB             1048576
#define _10MB               10485760
#define _100MB              104857600
#define _1GB                1073741824
#define _10GB               10737418240
#define _100GB              107374182400
#define _1TB                1099511627776
#define _10TB               10995116277760

class HeapData : public Histable
{
  friend class HeapActivity;
public:
  HeapData (char *sName);
  HeapData (HeapData *hData);
  ~HeapData ();
  char *get_raw_name (Histable::NameFormat nfmt);
  void init ();
  void setStackName (char* sName);
  void setPeakMemUsage (int64_t pmu, uint64_t sId, hrtime_t ts, int procId, int uei);

  virtual char *get_name (Histable::NameFormat nfmt);
  virtual void set_name (char * _name);
  virtual Histable *convertto (Histable_type, Histable* = NULL);

  virtual Histable_type
  get_type ()
  {
    return histType;
  }

  virtual uint64_t
  get_addr ()
  {
    return stackId;
  }

  uint64_t
  get_index ()
  {
    return stackId;
  }

  char *
  getStackName ()
  {
    return stackName;
  }

  void
  addAllocEvent (uint64_t nb)
  {
    allocBytes += nb;
    allocCnt++;
  }

  uint64_t
  getAllocBytes ()
  {
    return allocBytes;
  }

  int32_t
  getAllocCnt ()
  {
    return allocCnt;
  }

  void
  addLeakEvent (uint64_t nb)
  {
    leakBytes += nb;
    leakCnt++;
  }

  uint64_t
  getLeakBytes ()
  {
    return leakBytes;
  }

  int32_t
  getLeakCnt ()
  {
    return leakCnt;
  }

  void
  setStackId (uint64_t sId)
  {
    stackId = sId;
  }

  uint64_t
  getStackId ()
  {
    return stackId;
  }

  void
  setTimestamp (hrtime_t ts)
  {
    timestamp = ts;
  }

  hrtime_t
  getTimestamp ()
  {
    return timestamp;
  }

  void
  setHistType (Histable::Type hType)
  {
    histType = hType;
  }

  Histable::Type
  getHistType ()
  {
    return histType;
  }

  int64_t
  getPeakMemUsage ()
  {
    return peakMemUsage;
  }

  Vector<uint64_t> *
  getPeakStackIds ()
  {
    return peakStackIds;
  }

  Vector<hrtime_t> *
  getPeakTimestamps ()
  {
    return peakTimestamps;
  }

  void
  setPid (int procId)
  {
    pid = procId;
  }

  int
  getPid ()
  {
    return pid;
  }

  void
  setUserExpId (int uei)
  {
    userExpId = uei;
  }

  int
  getUserExpId ()
  {
    return userExpId;
  }

  void setAllocStat (int64_t nb);

  int64_t
  getASmallestBytes ()
  {
    return aSmallestBytes;
  }

  int64_t
  getALargestBytes ()
  {
    return aLargestBytes;
  }

  int32_t
  getA0KB1KBCnt ()
  {
    return a0KB1KBCnt;
  }

  int32_t
  getA1KB8KBCnt ()
  {
    return a1KB8KBCnt;
  }

  int32_t
  getA8KB32KBCnt ()
  {
    return a8KB32KBCnt;
  }

  int32_t
  getA32KB128KBCnt ()
  {
    return a32KB128KBCnt;
  }

  int32_t
  getA128KB256KBCnt ()
  {
    return a128KB256KBCnt;
  }

  int32_t
  getA256KB512KBCnt ()
  {
    return a256KB512KBCnt;
  }

  int32_t
  getA512KB1000KBCnt ()
  {
    return a512KB1000KBCnt;
  }

  int32_t
  getA1000KB10MBCnt ()
  {
    return a1000KB10MBCnt;
  }

  int32_t
  getA10MB100MBCnt ()
  {
    return a10MB100MBCnt;
  }

  int32_t
  getA100MB1GBCnt ()
  {
    return a100MB1GBCnt;
  }

  int32_t
  getA1GB10GBCnt ()
  {
    return a1GB10GBCnt;
  }

  int32_t
  getA10GB100GBCnt ()
  {
    return a10GB100GBCnt;
  }

  int32_t
  getA100GB1TBCnt ()
  {
    return a100GB1TBCnt;
  }

  int32_t
  getA1TB10TBCnt ()
  {
    return a1TB10TBCnt;
  }

  void setLeakStat (int64_t nb);

  int64_t
  getLSmallestBytes ()
  {
    return lSmallestBytes;
  }

  int64_t
  getLLargestBytes ()
  {
    return lLargestBytes;
  }

  int32_t
  getL0KB1KBCnt ()
  {
    return l0KB1KBCnt;
  }

  int32_t
  getL1KB8KBCnt ()
  {
    return l1KB8KBCnt;
  }

  int32_t
  getL8KB32KBCnt ()
  {
    return l8KB32KBCnt;
  }

  int32_t
  getL32KB128KBCnt ()
  {
    return l32KB128KBCnt;
  }

  int32_t
  getL128KB256KBCnt ()
  {
    return l128KB256KBCnt;
  }

  int32_t
  getL256KB512KBCnt ()
  {
    return l256KB512KBCnt;
  }

  int32_t
  getL512KB1000KBCnt ()
  {
    return l512KB1000KBCnt;
  }

  int32_t
  getL1000KB10MBCnt ()
  {
    return l1000KB10MBCnt;
  }

  int32_t
  getL10MB100MBCnt ()
  {
    return l10MB100MBCnt;
  }

  int32_t
  getL100MB1GBCnt ()
  {
    return l100MB1GBCnt;
  }

  int32_t
  getL1GB10GBCnt ()
  {
    return l1GB10GBCnt;
  }

  int32_t
  getL10GB100GBCnt ()
  {
    return l10GB100GBCnt;
  }

  int32_t
  getL100GB1TBCnt ()
  {
    return l100GB1TBCnt;
  }

  int32_t
  getL1TB10TBCnt ()
  {
    return l1TB10TBCnt;
  }

private:
  char *stackName;                  // stack name
  uint64_t allocBytes;              // The total bytes allocated
  uint64_t leakBytes;               // The total bytes leaked
  int32_t allocCnt;                 // The alloc count
  int32_t leakCnt;                  // The leak count
  Histable::Type histType;          // The Histable type: HEAPCALLSTACK
  int64_t peakMemUsage;             // Keep track of peak memory usage
  uint64_t stackId;
  Vector<uint64_t> *peakStackIds;   // The peak memory usage stack ids
  hrtime_t timestamp;
  Vector<hrtime_t> *peakTimestamps; // The peak data
  int pid;                          // The process id
  int userExpId;                    // The experiment id

  int64_t aSmallestBytes;
  int64_t aLargestBytes;
  int32_t a0KB1KBCnt;
  int32_t a1KB8KBCnt;
  int32_t a8KB32KBCnt;
  int32_t a32KB128KBCnt;
  int32_t a128KB256KBCnt;
  int32_t a256KB512KBCnt;
  int32_t a512KB1000KBCnt;
  int32_t a1000KB10MBCnt;
  int32_t a10MB100MBCnt;
  int32_t a100MB1GBCnt;
  int32_t a1GB10GBCnt;
  int32_t a10GB100GBCnt;
  int32_t a100GB1TBCnt;
  int32_t a1TB10TBCnt;

  int64_t lSmallestBytes;
  int64_t lLargestBytes;
  int32_t l0KB1KBCnt;
  int32_t l1KB8KBCnt;
  int32_t l8KB32KBCnt;
  int32_t l32KB128KBCnt;
  int32_t l128KB256KBCnt;
  int32_t l256KB512KBCnt;
  int32_t l512KB1000KBCnt;
  int32_t l1000KB10MBCnt;
  int32_t l10MB100MBCnt;
  int32_t l100MB1GBCnt;
  int32_t l1GB10GBCnt;
  int32_t l10GB100GBCnt;
  int32_t l100GB1TBCnt;
  int32_t l1TB10TBCnt;
};

#endif
