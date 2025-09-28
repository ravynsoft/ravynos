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

#include "config.h"
#include <assert.h>
#include <string.h>

#include "util.h"
#include "HeapData.h"

void
HeapData::init ()
{
  allocBytes = 0;
  leakBytes = 0;
  allocCnt = 0;
  leakCnt = 0;
  stackId = 0;
  histType = Histable::HEAPCALLSTACK;
  peakMemUsage = 0;
  timestamp = 0;
  pid = 0;
  userExpId = 0;
  aSmallestBytes = _10TB;
  aLargestBytes = 0;
  a0KB1KBCnt = 0;
  a1KB8KBCnt = 0;
  a8KB32KBCnt = 0;
  a32KB128KBCnt = 0;
  a128KB256KBCnt = 0;
  a256KB512KBCnt = 0;
  a512KB1000KBCnt = 0;
  a1000KB10MBCnt = 0;
  a10MB100MBCnt = 0;
  a100MB1GBCnt = 0;
  a1GB10GBCnt = 0;
  a10GB100GBCnt = 0;
  a100GB1TBCnt = 0;
  a1TB10TBCnt = 0;

  lSmallestBytes = _10TB;
  lLargestBytes = 0;
  l0KB1KBCnt = 0;
  l1KB8KBCnt = 0;
  l8KB32KBCnt = 0;
  l32KB128KBCnt = 0;
  l128KB256KBCnt = 0;
  l256KB512KBCnt = 0;
  l512KB1000KBCnt = 0;
  l1000KB10MBCnt = 0;
  l10MB100MBCnt = 0;
  l100MB1GBCnt = 0;
  l1GB10GBCnt = 0;
  l10GB100GBCnt = 0;
  l100GB1TBCnt = 0;
  l1TB10TBCnt = 0;
}

HeapData::HeapData (char *sName)
{
  stackName = dbe_strdup (sName);
  peakStackIds = new Vector<uint64_t>;
  peakTimestamps = new Vector<hrtime_t>;
  init ();
}

HeapData::HeapData (HeapData *hData)
{
  stackName = dbe_strdup (hData->stackName);
  stackId = hData->stackId;
  histType = hData->histType;
  allocBytes = hData->allocBytes;
  leakBytes = hData->leakBytes;
  allocCnt = hData->allocCnt;
  leakCnt = hData->leakCnt;
  peakMemUsage = hData->peakMemUsage;
  timestamp = hData->timestamp;
  pid = hData->getPid ();
  userExpId = hData->getUserExpId ();
  peakStackIds = new Vector<uint64_t>;
  Vector<uint64_t> *sIds = hData->peakStackIds;
  uint64_t sId;
  if (sIds != NULL)
    for (int i = 0; i < sIds->size (); i++)
      {
	sId = sIds->fetch (i);
	peakStackIds->append (sId);
      }

  peakTimestamps = new Vector<hrtime_t>;
  Vector<hrtime_t> *pts = hData->peakTimestamps;
  hrtime_t ts;
  if (pts != NULL)
    for (int i = 0; i < pts->size (); i++)
      {
	ts = pts->fetch (i);
	peakTimestamps->append (ts);
      }

  aSmallestBytes = hData->aSmallestBytes;
  aLargestBytes = hData->aLargestBytes;
  a0KB1KBCnt = hData->a0KB1KBCnt;
  a1KB8KBCnt = hData->a1KB8KBCnt;
  a8KB32KBCnt = hData->a8KB32KBCnt;
  a32KB128KBCnt = hData->a32KB128KBCnt;
  a128KB256KBCnt = hData->a128KB256KBCnt;
  a256KB512KBCnt = hData->a256KB512KBCnt;
  a512KB1000KBCnt = hData->a512KB1000KBCnt;
  a1000KB10MBCnt = hData->a1000KB10MBCnt;
  a10MB100MBCnt = hData->a10MB100MBCnt;
  a100MB1GBCnt = hData->a100MB1GBCnt;
  a1GB10GBCnt = hData->a1GB10GBCnt;
  a10GB100GBCnt = hData->a10GB100GBCnt;
  a100GB1TBCnt = hData->a100GB1TBCnt;
  a1TB10TBCnt = hData->a1TB10TBCnt;

  lSmallestBytes = hData->lSmallestBytes;
  lLargestBytes = hData->lLargestBytes;
  l0KB1KBCnt = hData->l0KB1KBCnt;
  l1KB8KBCnt = hData->l1KB8KBCnt;
  l8KB32KBCnt = hData->l8KB32KBCnt;
  l32KB128KBCnt = hData->l32KB128KBCnt;
  l128KB256KBCnt = hData->l128KB256KBCnt;
  l256KB512KBCnt = hData->l256KB512KBCnt;
  l512KB1000KBCnt = hData->l512KB1000KBCnt;
  l1000KB10MBCnt = hData->l1000KB10MBCnt;
  l10MB100MBCnt = hData->l10MB100MBCnt;
  l100MB1GBCnt = hData->l100MB1GBCnt;
  l1GB10GBCnt = hData->l1GB10GBCnt;
  l10GB100GBCnt = hData->l10GB100GBCnt;
  l100GB1TBCnt = hData->l100GB1TBCnt;
  l1TB10TBCnt = hData->l1TB10TBCnt;
}

HeapData::~HeapData ()
{
  free (stackName);
  delete peakStackIds;
  delete peakTimestamps;
}

Histable*
HeapData::convertto (Histable_type type, Histable*)
{
  return type == histType ? this : NULL;
}

char*
HeapData::get_name (Histable::NameFormat /*_nfmt*/)
{
  return stackName;
}

char*
HeapData::get_raw_name (Histable::NameFormat /*_nfmt*/)
{
  return stackName;
}

void
HeapData::set_name (char* _name)
{
  free (stackName);
  stackName = dbe_strdup (_name);
}

void
HeapData::setPeakMemUsage (int64_t pmu, uint64_t sId, hrtime_t ts, int procId, int uei)
{
  if (peakMemUsage < pmu)
    {
      peakMemUsage = pmu;
      peakStackIds->reset ();
      peakStackIds->append (sId);
      peakTimestamps->reset ();
      peakTimestamps->append (ts);
      pid = procId;
      userExpId = uei;
    }
  else if (peakMemUsage == pmu)
    {
      for (int i = 0; i < peakStackIds->size (); i++)
	{
	  uint64_t curSId = peakStackIds->fetch (i);
	  if (curSId == sId)
	    return;
	}
      peakStackIds->append (sId);
      peakTimestamps->append (ts);
      pid = procId;
      userExpId = uei;
    }
}

void
HeapData::setAllocStat (int64_t nb)
{
  if (aSmallestBytes > nb)
    aSmallestBytes = nb;
  if (aLargestBytes < nb)
    aLargestBytes = nb;
  if (nb >= 0 && nb <= _1KB)
    a0KB1KBCnt++;
  else if (nb <= _8KB)
    a1KB8KBCnt++;
  else if (nb <= _32KB)
    a8KB32KBCnt++;
  else if (nb <= _128KB)
    a32KB128KBCnt++;
  else if (nb <= _256KB)
    a128KB256KBCnt++;
  else if (nb <= _512KB)
    a256KB512KBCnt++;
  else if (nb <= _1000KB)
    a512KB1000KBCnt++;
  else if (nb <= _10MB)
    a1000KB10MBCnt++;
  else if (nb <= _100MB)
    a10MB100MBCnt++;
  else if (nb <= _1GB)
    a100MB1GBCnt++;
  else if (nb <= _10GB)
    a1GB10GBCnt++;
  else if (nb <= _100GB)
    a10GB100GBCnt++;
  else if (nb <= _1TB)
    a100GB1TBCnt++;
  else if (nb <= _10TB)
    a1TB10TBCnt++;
}

void
HeapData::setLeakStat (int64_t nb)
{
  if (lSmallestBytes > nb)
    lSmallestBytes = nb;
  if (lLargestBytes < nb)
    lLargestBytes = nb;
  if (nb >= 0 && nb <= _1KB)
    l0KB1KBCnt++;
  else if (nb <= _8KB)
    l1KB8KBCnt++;
  else if (nb <= _32KB)
    l8KB32KBCnt++;
  else if (nb <= _128KB)
    l32KB128KBCnt++;
  else if (nb <= _256KB)
    l128KB256KBCnt++;
  else if (nb <= _512KB)
    l256KB512KBCnt++;
  else if (nb <= _1000KB)
    l512KB1000KBCnt++;
  else if (nb <= _10MB)
    l1000KB10MBCnt++;
  else if (nb <= _100MB)
    l10MB100MBCnt++;
  else if (nb <= _1GB)
    l100MB1GBCnt++;
  else if (nb <= _10GB)
    l1GB10GBCnt++;
  else if (nb <= _100GB)
    l10GB100GBCnt++;
  else if (nb <= _1TB)
    l100GB1TBCnt++;
  else if (nb <= _10TB)
    l1TB10TBCnt++;
}
