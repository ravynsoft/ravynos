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
#include "FileData.h"

void
FileData::init ()
{
  readTime = 0;
  writeTime = 0;
  otherTime = 0;
  errorTime = 0;
  readBytes = 0;
  writeBytes = 0;
  readCnt = 0;
  writeCnt = 0;
  otherCnt = 0;
  errorCnt = 0;
  wSlowestBytes = 0;
  wSmallestBytes = _10TB;
  wLargestBytes = 0;
  w0KB1KBCnt = 0;
  w1KB8KBCnt = 0;
  w8KB32KBCnt = 0;
  w32KB128KBCnt = 0;
  w128KB256KBCnt = 0;
  w256KB512KBCnt = 0;
  w512KB1000KBCnt = 0;
  w1000KB10MBCnt = 0;
  w10MB100MBCnt = 0;
  w100MB1GBCnt = 0;
  w1GB10GBCnt = 0;
  w10GB100GBCnt = 0;
  w100GB1TBCnt = 0;
  w1TB10TBCnt = 0;
  rSlowestBytes = 0;
  rSmallestBytes = _10TB;
  rLargestBytes = 0;
  r0KB1KBCnt = 0;
  r1KB8KBCnt = 0;
  r8KB32KBCnt = 0;
  r32KB128KBCnt = 0;
  r128KB256KBCnt = 0;
  r256KB512KBCnt = 0;
  r512KB1000KBCnt = 0;
  r1000KB10MBCnt = 0;
  r10MB100MBCnt = 0;
  r100MB1GBCnt = 0;
  r1GB10GBCnt = 0;
  r10GB100GBCnt = 0;
  r100GB1TBCnt = 0;
  r1TB10TBCnt = 0;
}

FileData::FileData (const char *fName)
{
  fileName = dbe_strdup (fName);
  fileDesList = new Vector<int>;
  virtualFds = new Vector<int64_t>;
  virtualFd = -1;
  fileDes = -1;
  fsType[0] = '\0';
  histType = Histable::IOACTVFD;
  init ();
}

FileData::FileData (FileData *fData)
{
  fileName = dbe_strdup (fData->fileName);
  fileDesList = new Vector<int>;
  Vector<int> *fdList = fData->fileDesList;
  int fd;
  if (fdList != NULL)
    for (int i = 0; i < fdList->size (); i++)
      if ((fd = fdList->fetch (i)) == -1)
	fileDesList->append (fd);

  virtualFds = new Vector<int64_t>;
  Vector<int64_t> *vfds = fData->virtualFds;
  int64_t vfd;
  if (vfds != NULL)
    for (int i = 0; i < vfds->size (); i++)
      if ((vfd = vfds->fetch (i)) == -1)
	virtualFds->append (vfd);
  virtualFd = fData->virtualFd;
  fileDes = fData->fileDes;
  histType = fData->histType;

  for (int i = 0; i < FSTYPESZ; i++)
    fsType[i] = fData->fsType[i];

  readTime = fData->readTime;
  writeTime = fData->writeTime;
  otherTime = fData->otherTime;
  errorTime = fData->errorTime;
  readBytes = fData->readBytes;
  writeBytes = fData->writeBytes;
  readCnt = fData->readCnt;
  writeCnt = fData->writeCnt;
  otherCnt = fData->otherCnt;
  errorCnt = fData->errorCnt;
  wSlowestBytes = fData->wSlowestBytes;
  wSmallestBytes = fData->wSmallestBytes;
  wLargestBytes = fData->wLargestBytes;
  w0KB1KBCnt = fData->w0KB1KBCnt;
  w1KB8KBCnt = fData->w1KB8KBCnt;
  w8KB32KBCnt = fData->w8KB32KBCnt;
  w32KB128KBCnt = fData->w32KB128KBCnt;
  w128KB256KBCnt = fData->w128KB256KBCnt;
  w256KB512KBCnt = fData->w256KB512KBCnt;
  w512KB1000KBCnt = fData->w512KB1000KBCnt;
  w1000KB10MBCnt = fData->w1000KB10MBCnt;
  w10MB100MBCnt = fData->w10MB100MBCnt;
  w100MB1GBCnt = fData->w100MB1GBCnt;
  w1GB10GBCnt = fData->w1GB10GBCnt;
  w10GB100GBCnt = fData->w10GB100GBCnt;
  w100GB1TBCnt = fData->w100GB1TBCnt;
  w1TB10TBCnt = fData->w1TB10TBCnt;
  rSlowestBytes = fData->rSlowestBytes;
  rSmallestBytes = fData->rSmallestBytes;
  rLargestBytes = fData->rLargestBytes;
  r0KB1KBCnt = fData->r0KB1KBCnt;
  r1KB8KBCnt = fData->r1KB8KBCnt;
  r8KB32KBCnt = fData->r8KB32KBCnt;
  r32KB128KBCnt = fData->r32KB128KBCnt;
  r128KB256KBCnt = fData->r128KB256KBCnt;
  r256KB512KBCnt = fData->r256KB512KBCnt;
  r512KB1000KBCnt = fData->r512KB1000KBCnt;
  r1000KB10MBCnt = fData->r1000KB10MBCnt;
  r10MB100MBCnt = fData->r10MB100MBCnt;
  r100MB1GBCnt = fData->r100MB1GBCnt;
  r1GB10GBCnt = fData->r1GB10GBCnt;
  r10GB100GBCnt = fData->r10GB100GBCnt;
  r100GB1TBCnt = fData->r100GB1TBCnt;
  r1TB10TBCnt = fData->r1TB10TBCnt;
}

FileData::~FileData ()
{
  free (fileName);
  delete fileDesList;
  delete virtualFds;
}

void
FileData::setVirtualFds (int64_t vfd)
{
  for (int i = 0; i < virtualFds->size (); i++)
    if (vfd == virtualFds->fetch (i))
      return;
  virtualFds->append (vfd);
}

void
FileData::setFileDesList (int fd)
{
  for (int i = 0; i < fileDesList->size (); i++)
    if (fd == fileDesList->fetch (i))
      return;
  fileDesList->append (fd);
}

void
FileData::setFsType (const char* fst)
{
  size_t len = strlen (fst);
  if (len > 0 && len < FSTYPESZ)
    snprintf (fsType, sizeof (fsType), NTXT ("%s"), fst);
  else
    snprintf (fsType, sizeof (fsType), GTXT ("error"));
}

Histable*
FileData::convertto (Histable_type type, Histable*)
{
  return (type == histType ? this : NULL);
}

char*
FileData::get_name (Histable::NameFormat /*_nfmt*/)
{
  if (histType == Histable::IOACTVFD)
    {
      if (!streq (fileName, NTXT ("<Total>")))
	{
	  if (fileDes >= 0)
	    return dbe_sprintf (GTXT ("%s (IOVFD=%lld, FD=%d)"), fileName,
				(long long) virtualFd, (int) fileDes);
	  return dbe_sprintf (GTXT ("%s (IOVFD=%lld)"), fileName,
			      (long long) virtualFd);
	}
      else
	return fileName;
    }
  else if (histType == Histable::IOACTFILE)
    {
      if (!streq (fileName, NTXT ("<Total>")))
	{
	  if (!streq (fsType, NTXT ("N/A")))
	    return dbe_sprintf (GTXT ("%s (FS=%s)"), fileName, fsType);
	  return fileName;
	}
      return fileName;
    }
  return fileName;
}

char*
FileData::get_raw_name (Histable::NameFormat /*_nfmt*/)
{
  return fileName;
}

void
FileData::setFsType (FileSystem_type fst)
{
  const char *fsName;
  switch (fst)
    {
    case ZFS_TYPE:
      fsName = "zfs";
      break;
    case NFS_TYPE:
      fsName = "nfs";
      break;
    case UFS_TYPE:
      fsName = "ufs";
      break;
    case UDFS_TYPE:
      fsName = "udfs";
      break;
    case LOFS_TYPE:
      fsName = "lofs";
      break;
    case VXFS_TYPE:
      fsName = "vxfs";
      break;
    case TMPFS_TYPE:
      fsName = "tmpfs";
      break;
    case PCFS_TYPE:
      fsName = "pcfs";
      break;
    case HSFS_TYPE:
      fsName = "hsfs";
      break;
    case PROCFS_TYPE:
      fsName = "procfs";
      break;
    case FIFOFS_TYPE:
      fsName = "fifofs";
      break;
    case SWAPFS_TYPE:
      fsName = "swapfs";
      break;
    case CACHEFS_TYPE:
      fsName = "cachefs";
      break;
    case AUTOFS_TYPE:
      fsName = "autofs";
      break;
    case SPECFS_TYPE:
      fsName = "specfs";
      break;
    case SOCKFS_TYPE:
      fsName = "sockfs";
      break;
    case FDFS_TYPE:
      fsName = "fdfs";
      break;
    case MNTFS_TYPE:
      fsName = "mntfs";
      break;
    case NAMEFS_TYPE:
      fsName = "namefs";
      break;
    case OBJFS_TYPE:
      fsName = "objfs";
      break;
    case SHAREFS_TYPE:
      fsName = "sharefs";
      break;
    case EXT2FS_TYPE:
      fsName = "ext2";
      break;
    case EXT3FS_TYPE:
      fsName = "ext3";
      break;
    case EXT4FS_TYPE:
      fsName = "ext4";
      break;
    case UNKNOWNFS_TYPE:
      fsName = "N/A";
      break;
    default:
      fsName = "N/A";
      break;
    }
  setFsType (fsName);
}

void
FileData::setWriteStat (hrtime_t wt, int64_t nb)
{
  if (wSlowestBytes < wt)
    wSlowestBytes = wt;
  if (nb != 0 && wSmallestBytes > nb)
    wSmallestBytes = nb;
  if (wLargestBytes < nb)
    wLargestBytes = nb;
  if (nb >= 0 && nb <= _1KB)
    w0KB1KBCnt++;
  else if (nb <= _8KB)
    w1KB8KBCnt++;
  else if (nb <= _32KB)
    w8KB32KBCnt++;
  else if (nb <= _128KB)
    w32KB128KBCnt++;
  else if (nb <= _256KB)
    w128KB256KBCnt++;
  else if (nb <= _512KB)
    w256KB512KBCnt++;
  else if (nb <= _1000KB)
    w512KB1000KBCnt++;
  else if (nb <= _10MB)
    w1000KB10MBCnt++;
  else if (nb <= _100MB)
    w10MB100MBCnt++;
  else if (nb <= _1GB)
    w100MB1GBCnt++;
  else if (nb <= _10GB)
    w1GB10GBCnt++;
  else if (nb <= _100GB)
    w10GB100GBCnt++;
  else if (nb <= _1TB)
    w100GB1TBCnt++;
  else if (nb <= _10TB)
    w1TB10TBCnt++;
}

void
FileData::setReadStat (hrtime_t rt, int64_t nb)
{
  if (rSlowestBytes < rt)
    rSlowestBytes = rt;
  if (nb != 0 && rSmallestBytes > nb)
    rSmallestBytes = nb;
  if (rLargestBytes < nb)
    rLargestBytes = nb;
  if (nb >= 0 && nb <= _1KB)
    r0KB1KBCnt++;
  else if (nb <= _8KB)
    r1KB8KBCnt++;
  else if (nb <= _32KB)
    r8KB32KBCnt++;
  else if (nb <= _128KB)
    r32KB128KBCnt++;
  else if (nb <= _256KB)
    r128KB256KBCnt++;
  else if (nb <= _512KB)
    r256KB512KBCnt++;
  else if (nb <= _1000KB)
    r512KB1000KBCnt++;
  else if (nb <= _10MB)
    r1000KB10MBCnt++;
  else if (nb <= _100MB)
    r10MB100MBCnt++;
  else if (nb <= _1GB)
    r100MB1GBCnt++;
  else if (nb <= _10GB)
    r1GB10GBCnt++;
  else if (nb <= _100GB)
    r10GB100GBCnt++;
  else if (nb <= _1TB)
    r100GB1TBCnt++;
  else if (nb <= _10TB)
    r1TB10TBCnt++;
}
