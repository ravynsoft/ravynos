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

#ifndef _FILEDATA_H
#define _FILEDATA_H

#include "gp-defs.h"
#include "gp-time.h"

#include "vec.h"
#include "data_pckts.h"
#include "Histable.h"

#define FSTYPESZ  16

#define VIRTUAL_FD_TOTAL    0
#define VIRTUAL_FD_STDIN    1
#define VIRTUAL_FD_STDOUT   2
#define VIRTUAL_FD_STDERR   3
#define VIRTUAL_FD_OTHERIO  4
#define VIRTUAL_FD_NONE     -1

#define STDIN_FD            0
#define STDOUT_FD           1
#define STDERR_FD           2
#define OTHERIO_FD          -1

#define OTHERIO_FILENAME    "<Other IO activity>"
#define STDIN_FILENAME      "<stdin>"
#define STDOUT_FILENAME     "<stdout>"
#define STDERR_FILENAME     "<stderr>"
#define TOTAL_FILENAME      NTXT("<Total>")
#define UNKNOWNFD_FILENAME  "<pipe(), socket(), or other fds>"

#define _1KB        1024
#define _8KB        8192
#define _32KB       32768
#define _128KB      131072
#define _256KB      262144
#define _512KB      524288
#define _1000KB     1048576
#define _10MB       10485760
#define _100MB      104857600
#define _1GB        1073741824
#define _10GB       10737418240
#define _100GB      107374182400
#define _1TB        1099511627776
#define _10TB       10995116277760

class FileData : public Histable
{
  friend class IOActivity;
public:
  FileData (const char *fName);
  FileData (FileData *fData);
  ~FileData ();

  virtual char *get_name (Histable::NameFormat nfmt);
  virtual Histable *convertto (Histable_type, Histable* = NULL);

  char *get_raw_name (Histable::NameFormat nfmt);
  void setFsType (FileSystem_type fst);
  void setFsType (const char* fst);

  virtual Histable_type
  get_type ()
  {
    return histType;
  };

  virtual uint64_t
  get_addr ()
  {
    return virtualFd;
  };

  uint64_t
  get_index ()
  {
    return virtualFd;
  };

  void init ();

  char *
  getFileName ()
  {
    return fileName;
  }

  void
  addReadEvent (hrtime_t rt, int64_t nb)
  {
    readTime += rt;
    readBytes += nb;
    readCnt++;
  }

  hrtime_t
  getReadTime ()
  {
    return readTime;
  }

  int64_t
  getReadBytes ()
  {
    return readBytes;
  }

  int32_t
  getReadCnt ()
  {
    return readCnt;
  }

  void
  addWriteEvent (hrtime_t wt, int64_t nb)
  {
    writeTime += wt;
    writeBytes += nb;
    writeCnt++;
  }

  hrtime_t
  getWriteTime ()
  {
    return writeTime;
  }

  int64_t
  getWriteBytes ()
  {
    return writeBytes;
  }

  int32_t
  getWriteCnt ()
  {
    return writeCnt;
  }

  void
  addOtherEvent (hrtime_t ot)
  {
    otherTime += ot;
    otherCnt++;
  }

  hrtime_t
  getOtherTime ()
  {
    return otherTime;
  }

  int32_t
  getOtherCnt ()
  {
    return otherCnt;
  }

  void
  addErrorEvent (hrtime_t er)
  {
    errorTime += er;
    errorCnt++;
  }

  hrtime_t
  getErrorTime ()
  {
    return errorTime;
  }

  int32_t
  getErrorCnt ()
  {
    return errorCnt;
  }

  void setFileDesList (int fd);

  Vector<int> *
  getFileDesList ()
  {
    return fileDesList;
  }

  void
  setFileDes (int fd)
  {
    fileDes = fd;
  }

  int32_t
  getFileDes ()
  {
    return fileDes;
  }

  void setVirtualFds (int64_t vfd);

  Vector<int64_t> *
  getVirtualFds ()
  {
    return virtualFds;
  }

  char *
  getFsType ()
  {
    return fsType;
  }

  void
  setVirtualFd (int64_t vFd)
  {
    virtualFd = vFd;
  }

  int64_t
  getVirtualFd ()
  {
    return virtualFd;
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

  void setWriteStat (hrtime_t wt, int64_t nb);

  hrtime_t
  getWSlowestBytes ()
  {
    return wSlowestBytes;
  }

  int64_t
  getWSmallestBytes ()
  {
    return wSmallestBytes;
  }

  int64_t
  getWLargestBytes ()
  {
    return wLargestBytes;
  }

  int32_t
  getW0KB1KBCnt ()
  {
    return w0KB1KBCnt;
  }

  int32_t
  getW1KB8KBCnt ()
  {
    return w1KB8KBCnt;
  }

  int32_t
  getW8KB32KBCnt ()
  {
    return w8KB32KBCnt;
  }

  int32_t
  getW32KB128KBCnt ()
  {
    return w32KB128KBCnt;
  }

  int32_t
  getW128KB256KBCnt ()
  {
    return w128KB256KBCnt;
  }

  int32_t
  getW256KB512KBCnt ()
  {
    return w256KB512KBCnt;
  }

  int32_t
  getW512KB1000KBCnt ()
  {
    return w512KB1000KBCnt;
  }

  int32_t
  getW1000KB10MBCnt ()
  {
    return w1000KB10MBCnt;
  }

  int32_t
  getW10MB100MBCnt ()
  {
    return w10MB100MBCnt;
  }

  int32_t
  getW100MB1GBCnt ()
  {
    return w100MB1GBCnt;
  }

  int32_t
  getW1GB10GBCnt ()
  {
    return w1GB10GBCnt;
  }

  int32_t
  getW10GB100GBCnt ()
  {
    return w10GB100GBCnt;
  }

  int32_t
  getW100GB1TBCnt ()
  {
    return w100GB1TBCnt;
  }

  int32_t
  getW1TB10TBCnt ()
  {
    return w1TB10TBCnt;
  }

  void setReadStat (hrtime_t rt, int64_t nb);

  hrtime_t
  getRSlowestBytes ()
  {
    return rSlowestBytes;
  }

  int64_t
  getRSmallestBytes ()
  {
    return rSmallestBytes;
  }

  int64_t
  getRLargestBytes ()
  {
    return rLargestBytes;
  }

  int32_t
  getR0KB1KBCnt ()
  {
    return r0KB1KBCnt;
  }

  int32_t
  getR1KB8KBCnt ()
  {
    return r1KB8KBCnt;
  }

  int32_t
  getR8KB32KBCnt ()
  {
    return r8KB32KBCnt;
  }

  int32_t
  getR32KB128KBCnt ()
  {
    return r32KB128KBCnt;
  }

  int32_t
  getR128KB256KBCnt ()
  {
    return r128KB256KBCnt;
  }

  int32_t
  getR256KB512KBCnt ()
  {
    return r256KB512KBCnt;
  }

  int32_t
  getR512KB1000KBCnt ()
  {
    return r512KB1000KBCnt;
  }

  int32_t
  getR1000KB10MBCnt ()
  {
    return r1000KB10MBCnt;
  }

  int32_t
  getR10MB100MBCnt ()
  {
    return r10MB100MBCnt;
  }

  int32_t
  getR100MB1GBCnt ()
  {
    return r100MB1GBCnt;
  }

  int32_t
  getR1GB10GBCnt ()
  {
    return r1GB10GBCnt;
  }

  int32_t
  getR10GB100GBCnt ()
  {
    return r10GB100GBCnt;
  }

  int32_t
  getR100GB1TBCnt ()
  {
    return r100GB1TBCnt;
  }

  int32_t
  getR1TB10TBCnt ()
  {
    return r1TB10TBCnt;
  }

private:
  char *fileName;           // File name
  hrtime_t readTime;        // The Total time for read operations;
  hrtime_t writeTime;       // The Total time for write operations;
  hrtime_t otherTime;       // The Total time for other IO operations;
  hrtime_t errorTime;       // The Total time for failed IO operations;
  int64_t readBytes;        //The total bytes read
  int64_t writeBytes;       //The total bytes written
  int32_t readCnt;          // The read count
  int32_t writeCnt;         // The write count
  int32_t otherCnt;         // The other IO count
  int32_t errorCnt;         // The failed IO count
  Vector<int> *fileDesList; // The list of file descriptors
  Vector<int64_t> *virtualFds; // The list of file virtual descriptors
  char fsType[FSTYPESZ];    // The file system type
  int64_t virtualFd;        // The virtual file descriptor
  int32_t fileDes;          // The file descriptor
  Histable::Type histType;  // The Histable type: IOACTFILE, IOACTVFD, ...

  // Write statistics
  hrtime_t wSlowestBytes;
  int64_t wSmallestBytes;
  int64_t wLargestBytes;
  int32_t w0KB1KBCnt;
  int32_t w1KB8KBCnt;
  int32_t w8KB32KBCnt;
  int32_t w32KB128KBCnt;
  int32_t w128KB256KBCnt;
  int32_t w256KB512KBCnt;
  int32_t w512KB1000KBCnt;
  int32_t w1000KB10MBCnt;
  int32_t w10MB100MBCnt;
  int32_t w100MB1GBCnt;
  int32_t w1GB10GBCnt;
  int32_t w10GB100GBCnt;
  int32_t w100GB1TBCnt;
  int32_t w1TB10TBCnt;

  // Read statistics
  hrtime_t rSlowestBytes;
  int64_t rSmallestBytes;
  int64_t rLargestBytes;
  int32_t r0KB1KBCnt;
  int32_t r1KB8KBCnt;
  int32_t r8KB32KBCnt;
  int32_t r32KB128KBCnt;
  int32_t r128KB256KBCnt;
  int32_t r256KB512KBCnt;
  int32_t r512KB1000KBCnt;
  int32_t r1000KB10MBCnt;
  int32_t r10MB100MBCnt;
  int32_t r100MB1GBCnt;
  int32_t r1GB10GBCnt;
  int32_t r10GB100GBCnt;
  int32_t r100GB1TBCnt;
  int32_t r1TB10TBCnt;
};

#endif
