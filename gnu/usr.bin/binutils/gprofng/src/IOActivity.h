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

#ifndef _IOACTIVITY_H
#define _IOACTIVITY_H


#include <stdio.h>

#include "vec.h"
#include "Histable.h"
#include "Hist_data.h"
#include "Metric.h"
#include "FileData.h"
#include "DefaultMap.h"
#include "dbe_types.h"

class Experiment;
class Expression;
class DataView;
class DbeView;

class IOActivity
{
public:

  IOActivity (DbeView *_dbev);
  void reset (void);
  Hist_data *compute_metrics (MetricList *, Histable::Type, Hist_data::Mode,
			      Histable*);

  ~IOActivity ()
  {
    this->reset ();
  }

private:
  void computeData (Histable::Type);
  void computeCallStack (Histable::Type, VMode viewMode);
  void createHistItemTotals (Hist_data *, MetricList *, Histable::Type, bool);
  void computeHistTotals (Hist_data *, MetricList *);
  void computeHistData (Hist_data *, MetricList *, Hist_data::Mode, Histable *);

  Vector<FileData*> *fDataObjs;
  Vector<FileData*> *fDataObjsFile;
  Vector<FileData*> *fDataObjsVfd;
  Vector<FileData*> *fDataObjsCallStack;
  bool hasFile;
  bool hasVfd;
  bool hasCallStack;
  HashMap<char*, FileData*> *fDataHash;
  FileData *fDataTotal;

  // list of FileData objects using the stack id as the key
  DefaultMap<void*, FileData*> *fDataCalStkMap;

  // list of FileData objects using the VFD as the key
  DefaultMap<long, FileData*> *fDataVfdMap;

  // the cached data for mode=Hist_Data::ALL
  Hist_data *hist_data_file_all;
  Hist_data *hist_data_vfd_all;
  Hist_data *hist_data_callstack_all;
  Hist_data *hist_data_callstack;

  DbeView *dbev;
};

#endif /* _IOACTIVITY_H */
