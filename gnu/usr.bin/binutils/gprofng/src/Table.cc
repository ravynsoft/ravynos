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
#include <stdio.h>

#include "IndexMap2D.h"
#include "DbeSession.h"
#include "FilterExp.h"
#include "Table.h"
#include "util.h"
#include "i18n.h"

char *
get_prof_data_type_name (int t)
{
  switch (t)
    {
    case DATA_SAMPLE:   return NTXT("PROFDATA_TYPE_SAMPLE");
    case DATA_GCEVENT:  return NTXT("PROFDATA_TYPE_GCEVENT");
    case DATA_HEAPSZ:   return NTXT("PROFDATA_TYPE_HEAPSZ");
    case DATA_CLOCK:    return NTXT("PROFDATA_TYPE_CLOCK");
    case DATA_HWC:      return NTXT("PROFDATA_TYPE_HWC");
    case DATA_SYNCH:    return NTXT("PROFDATA_TYPE_SYNCH");
    case DATA_HEAP:     return NTXT("PROFDATA_TYPE_HEAP");
    case DATA_OMP:      return NTXT("PROFDATA_TYPE_OMP");
    case DATA_OMP2:     return NTXT("PROFDATA_TYPE_OMP2");
    case DATA_OMP3:     return NTXT("PROFDATA_TYPE_OMP3");
    case DATA_OMP4:     return NTXT("PROFDATA_TYPE_OMP4");
    case DATA_OMP5:     return NTXT("PROFDATA_TYPE_OMP5");
    case DATA_IOTRACE:  return NTXT("PROFDATA_TYPE_IOTRACE");
    default: abort ();
      return NTXT ("PROFDATA_TYPE_ERROR");
    }
}

char *
get_prof_data_type_uname (int t)
{
  switch (t)
    {
    case DATA_SAMPLE:   return GTXT("Process-wide Resource Utilization");
    case DATA_GCEVENT:  return GTXT("Java Garbage Collection Events");
    case DATA_HEAPSZ:   return GTXT("Heap Size");
    case DATA_CLOCK:    return GTXT("Clock Profiling");
    case DATA_HWC:      return GTXT("HW Counter Profiling");
    case DATA_SYNCH:    return GTXT("Synchronization Tracing");
    case DATA_HEAP:     return GTXT("Heap Tracing");
    case DATA_OMP:      return GTXT("OpenMP Profiling");
    case DATA_OMP2:     return GTXT("OpenMP Profiling");
    case DATA_OMP3:     return GTXT("OpenMP Profiling");
    case DATA_OMP4:     return GTXT("OpenMP Profiling");
    case DATA_OMP5:     return GTXT("OpenMP Profiling");
    case DATA_IOTRACE:  return GTXT("IO Tracing");
    default: abort ();
      return NTXT ("PROFDATA_TYPE_ERROR");
    }
}

int assert_level = 0; // set to 1 to bypass problematic asserts

#define ASSERT_SKIP (assert_level)

/*
 *    class PropDescr
 */

PropDescr::PropDescr (int _propID, const char *_name)
{
  propID = _propID;
  name = strdup (_name ? _name : NTXT (""));
  uname = NULL;
  vtype = TYPE_NONE;
  flags = 0;
  stateNames = NULL;
  stateUNames = NULL;
}

PropDescr::~PropDescr ()
{
  free (name);
  free (uname);
  if (stateNames)
    {
      stateNames->destroy ();
      delete stateNames;
    }
  if (stateUNames)
    {
      stateUNames->destroy ();
      delete stateUNames;
    }
}

void
PropDescr::addState (int value, const char *stname, const char *stuname)
{
  if (value < 0 || stname == NULL)
    return;
  if (stateNames == NULL)
    stateNames = new Vector<char*>;
  stateNames->store (value, strdup (stname));
  if (stateUNames == NULL)
    stateUNames = new Vector<char*>;
  stateUNames->store (value, strdup (stuname));
}

char *
PropDescr::getStateName (int value)
{
  if (stateNames && value >= 0 && value < stateNames->size ())
    return stateNames->fetch (value);
  return NULL;
}

char *
PropDescr::getStateUName (int value)
{
  if (stateUNames && value >= 0 && value < stateUNames->size ())
    return stateUNames->fetch (value);
  return NULL;
}

/*
 *    class FieldDescr
 */

FieldDescr::FieldDescr (int _propID, const char *_name)
{
  propID = _propID;
  name = _name ? strdup (_name) : NULL;
  offset = 0;
  vtype = TYPE_NONE;
  format = NULL;
}

FieldDescr::~FieldDescr ()
{
  free (name);
  free (format);
}

/*
 *    class PacketDescriptor
 */

PacketDescriptor::PacketDescriptor (DataDescriptor *_ddscr)
{
  ddscr = _ddscr;
  fields = new Vector<FieldDescr*>;
}

PacketDescriptor::~PacketDescriptor ()
{
  fields->destroy ();
  delete fields;
}

void
PacketDescriptor::addField (FieldDescr *fldDscr)
{
  if (fldDscr == NULL)
    return;
  fields->append (fldDscr);
}

/*
 *    class Data
 */

/* Check compatibility between Datum and Data */
static void
checkCompatibility (VType_type v1, VType_type v2)
{
  switch (v1)
    {
    case TYPE_NONE:
    case TYPE_STRING:
    case TYPE_DOUBLE:
    case TYPE_OBJ:
    case TYPE_DATE:
      assert (v1 == v2);
      break;
    case TYPE_INT32:
    case TYPE_UINT32:
      assert (v2 == TYPE_INT32 ||
	      v2 == TYPE_UINT32);
      break;
    case TYPE_INT64:
    case TYPE_UINT64:
      assert (v2 == TYPE_INT64 ||
	      v2 == TYPE_UINT64);
      break;
    default:
      assert (0);
    }
}

class DataINT32 : public Data
{
public:

  DataINT32 ()
  {
    data = new Vector<int32_t>;
  }

  virtual
  ~DataINT32 ()
  {
    delete data;
  }

  virtual VType_type
  type ()
  {
    return TYPE_INT32;
  }

  virtual void
  reset ()
  {
    data->reset ();
  }

  virtual long
  getSize ()
  {
    return data->size ();
  }

  virtual int
  fetchInt (long i)
  {
    return (int) data->fetch (i);
  }

  virtual unsigned long long
  fetchULong (long i)
  {
    return (unsigned long long) data->fetch (i);
  }

  virtual long long
  fetchLong (long i)
  {
    return (long long) data->fetch (i);
  }

  virtual char *
  fetchString (long i)
  {
    return dbe_sprintf (NTXT ("%d"), data->fetch (i));
  }

  virtual double
  fetchDouble (long i)
  {
    return (double) data->fetch (i);
  }

  virtual void *
  fetchObject (long)
  {
    assert (ASSERT_SKIP);
    return NULL;
  }

  virtual void
  setDatumValue (long idx, const Datum *val)
  {
    data->store (idx, val->i);
  }

  virtual void
  setValue (long idx, uint64_t val)
  {
    data->store (idx, (int32_t) val);
  }

  virtual void
  setObjValue (long, void*)
  {
    assert (ASSERT_SKIP);
    return;
  }

  virtual int
  cmpValues (long idx1, long idx2)
  {
    int32_t i1 = data->fetch (idx1);
    int32_t i2 = data->fetch (idx2);
    return i1 < i2 ? -1 : i1 > i2 ? 1 : 0;
  }

  virtual int
  cmpDatumValue (long idx, const Datum *val)
  {
    int32_t i1 = data->fetch (idx);
    int32_t i2 = val->i;
    return i1 < i2 ? -1 : i1 > i2 ? 1 : 0;
  }

private:
  Vector<int32_t> *data;
};

class DataUINT32 : public Data
{
public:

  DataUINT32 ()
  {
    data = new Vector<uint32_t>;
  }

  virtual
  ~DataUINT32 ()
  {
    delete data;
  }

  virtual VType_type
  type ()
  {
    return TYPE_UINT32;
  }

  virtual void
  reset ()
  {
    data->reset ();
  }

  virtual long
  getSize ()
  {
    return data->size ();
  }

  virtual int
  fetchInt (long i)
  {
    return (int) data->fetch (i);
  }

  virtual unsigned long long
  fetchULong (long i)
  {
    return (unsigned long long) data->fetch (i);
  }

  virtual long long
  fetchLong (long i)
  {
    return (long long) data->fetch (i);
  }

  virtual char *
  fetchString (long i)
  {
    return dbe_sprintf (NTXT ("%u"), data->fetch (i));
  }

  virtual double
  fetchDouble (long i)
  {
    return (double) data->fetch (i);
  }

  virtual void *
  fetchObject (long)
  {
    assert (ASSERT_SKIP);
    return NULL;
  }

  virtual void
  setDatumValue (long idx, const Datum *val)
  {
    data->store (idx, val->i);
  }

  virtual void
  setValue (long idx, uint64_t val)
  {
    data->store (idx, (uint32_t) val);
  }

  virtual void
  setObjValue (long, void*)
  {
    assert (ASSERT_SKIP);
    return;
  }

  virtual int
  cmpValues (long idx1, long idx2)
  {
    uint32_t u1 = data->fetch (idx1);
    uint32_t u2 = data->fetch (idx2);
    return u1 < u2 ? -1 : u1 > u2 ? 1 : 0;
  }

  virtual int
  cmpDatumValue (long idx, const Datum *val)
  {
    uint32_t u1 = data->fetch (idx);
    uint32_t u2 = (uint32_t) val->i;
    return u1 < u2 ? -1 : u1 > u2 ? 1 : 0;
  }

private:
  Vector<uint32_t> *data;
};

class DataINT64 : public Data
{
public:

  DataINT64 ()
  {
    data = new Vector<int64_t>;
  }

  virtual
  ~DataINT64 ()
  {
    delete data;
  }

  virtual VType_type
  type ()
  {
    return TYPE_INT64;
  }

  virtual void
  reset ()
  {
    data->reset ();
  }

  virtual long
  getSize ()
  {
    return data->size ();
  }

  virtual int
  fetchInt (long i)
  {
    return (int) data->fetch (i);
  }

  virtual unsigned long long
  fetchULong (long i)
  {
    return (unsigned long long) data->fetch (i);
  }

  virtual long long
  fetchLong (long i)
  {
    return (long long) data->fetch (i);
  }

  virtual char *
  fetchString (long i)
  {
    return dbe_sprintf (NTXT ("%lld"), (long long) data->fetch (i));
  }

  virtual double
  fetchDouble (long i)
  {
    return (double) data->fetch (i);
  }

  virtual void *
  fetchObject (long)
  {
    assert (ASSERT_SKIP);
    return NULL;
  }

  virtual void
  setDatumValue (long idx, const Datum *val)
  {
    data->store (idx, val->ll);
  }

  virtual void
  setValue (long idx, uint64_t val)
  {
    data->store (idx, (int64_t) val);
  }

  virtual void
  setObjValue (long, void*)
  {
    assert (ASSERT_SKIP);
    return;
  }

  virtual int
  cmpValues (long idx1, long idx2)
  {
    int64_t i1 = data->fetch (idx1);
    int64_t i2 = data->fetch (idx2);
    return i1 < i2 ? -1 : i1 > i2 ? 1 : 0;
  }

  virtual int
  cmpDatumValue (long idx, const Datum *val)
  {
    int64_t i1 = data->fetch (idx);
    int64_t i2 = val->ll;
    return i1 < i2 ? -1 : i1 > i2 ? 1 : 0;
  }

private:
  Vector<int64_t> *data;
};

class DataUINT64 : public Data
{
public:

  DataUINT64 ()
  {
    data = new Vector<uint64_t>;
  }

  virtual
  ~DataUINT64 ()
  {
    delete data;
  }

  virtual VType_type
  type ()
  {
    return TYPE_UINT64;
  }

  virtual void
  reset ()
  {
    data->reset ();
  }

  virtual long
  getSize ()
  {
    return data->size ();
  }

  virtual int
  fetchInt (long i)
  {
    return (int) data->fetch (i);
  }

  virtual unsigned long long
  fetchULong (long i)
  {
    return (unsigned long long) data->fetch (i);
  }

  virtual long long
  fetchLong (long i)
  {
    return (long long) data->fetch (i);
  }

  virtual char *
  fetchString (long i)
  {
    return dbe_sprintf (NTXT ("%llu"), (long long) data->fetch (i));
  }

  virtual double
  fetchDouble (long i)
  {
    return (double) data->fetch (i);
  }

  virtual void *
  fetchObject (long)
  {
    assert (ASSERT_SKIP);
    return NULL;
  }

  virtual void
  setDatumValue (long idx, const Datum *val)
  {
    data->store (idx, val->ll);
  }

  virtual void
  setValue (long idx, uint64_t val)
  {
    data->store (idx, val);
  }

  virtual void
  setObjValue (long, void*)
  {
    assert (ASSERT_SKIP);
    return;
  }

  virtual int
  cmpValues (long idx1, long idx2)
  {
    uint64_t u1 = data->fetch (idx1);
    uint64_t u2 = data->fetch (idx2);
    return u1 < u2 ? -1 : u1 > u2 ? 1 : 0;
  }

  virtual int
  cmpDatumValue (long idx, const Datum *val)
  {
    uint64_t u1 = data->fetch (idx);
    uint64_t u2 = (uint64_t) val->ll;
    return u1 < u2 ? -1 : u1 > u2 ? 1 : 0;
  }

private:
  Vector<uint64_t> *data;
};

class DataOBJECT : public Data
{
public:

  DataOBJECT ()
  {
    dtype = TYPE_OBJ;
    data = new Vector<void*>;
  }

  DataOBJECT (VType_type _dtype)
  {
    dtype = _dtype;
    data = new Vector<void*>;
  }

  virtual
  ~DataOBJECT ()
  {
    delete data;
  }

  virtual VType_type
  type ()
  {
    return dtype;
  }

  virtual void
  reset ()
  {
    data->reset ();
  }

  virtual long
  getSize ()
  {
    return data->size ();
  }

  virtual int
  fetchInt (long)
  {
    assert (ASSERT_SKIP);
    return 0;
  }

  virtual unsigned long long
  fetchULong (long)
  {
    assert (ASSERT_SKIP);
    return 0LL;
  }

  virtual long long
  fetchLong (long)
  {
    assert (ASSERT_SKIP);
    return 0LL;
  }

  virtual char *
  fetchString (long i)
  {
    return dbe_sprintf (NTXT ("%lu"), (unsigned long) data->fetch (i));
  }

  virtual double
  fetchDouble (long)
  {
    assert (ASSERT_SKIP);
    return 0.0;
  }

  virtual void *
  fetchObject (long i)
  {
    return data->fetch (i);
  }

  virtual void
  setDatumValue (long idx, const Datum *val)
  {
    data->store (idx, val->p);
  }

  virtual void
  setValue (long, uint64_t)
  {
    assert (ASSERT_SKIP);
    return;
  }

  virtual void
  setObjValue (long idx, void *p)
  {
    data->store (idx, p);
  }

  virtual int
  cmpValues (long, long)
  {
    return 0;
  }

  virtual int
  cmpDatumValue (long, const Datum *)
  {
    return 0;
  }

private:
  VType_type dtype;
  Vector<void*> *data;
};

class DataSTRING : public Data
{
public:

  DataSTRING ()
  {
    data = new Vector<char*>;
  }

  virtual
  ~DataSTRING ()
  {
    data->destroy ();
    delete data;
  }

  virtual VType_type
  type ()
  {
    return TYPE_STRING;
  }

  virtual void
  reset ()
  {
    data->reset ();
  }

  virtual long
  getSize ()
  {
    return data->size ();
  }

  virtual int
  fetchInt (long)
  {
    return 0;
  }

  virtual unsigned long long
  fetchULong (long)
  {
    return 0LL;
  }

  virtual long long
  fetchLong (long)
  {
    return 0LL;
  }

  virtual char *
  fetchString (long i)
  {
    return strdup (data->fetch (i));
  }

  virtual double
  fetchDouble (long)
  {
    return 0.0;
  }

  virtual void *
  fetchObject (long i)
  {
    return data->fetch (i);
  }

  virtual void
  setDatumValue (long idx, const Datum *val)
  {
    data->store (idx, val->l);
  }

  virtual void
  setValue (long, uint64_t)
  {
    return;
  }

  virtual void
  setObjValue (long idx, void *p)
  {
    data->store (idx, (char*) p);
  }

  virtual int
  cmpValues (long, long)
  {
    return 0;
  }

  virtual int
  cmpDatumValue (long, const Datum *)
  {
    return 0;
  }

private:
  Vector<char*> *data;
};

class DataDOUBLE : public Data
{
public:

  DataDOUBLE ()
  {
    data = new Vector<double>;
  }

  virtual
  ~DataDOUBLE ()
  {
    delete data;
  }

  virtual VType_type
  type ()
  {
    return TYPE_DOUBLE;
  }

  virtual void
  reset ()
  {
    data->reset ();
  }

  virtual long
  getSize ()
  {
    return data->size ();
  }

  virtual int
  fetchInt (long i)
  {
    return (int) data->fetch (i);
  }

  virtual unsigned long long
  fetchULong (long i)
  {
    return (unsigned long long) data->fetch (i);
  }

  virtual long long
  fetchLong (long i)
  {
    return (long long) data->fetch (i);
  }

  virtual char *
  fetchString (long i)
  {
    return dbe_sprintf (NTXT ("%f"), data->fetch (i));
  }

  virtual double
  fetchDouble (long i)
  {
    return data->fetch (i);
  }

  virtual void
  setDatumValue (long idx, const Datum *val)
  {
    data->store (idx, val->d);
  }

  virtual void
  setValue (long idx, uint64_t val)
  {
    data->store (idx, (double) val);
  }

  virtual void
  setObjValue (long, void*)
  {
    return;
  }

  virtual void *
  fetchObject (long)
  {
    return NULL;
  }

  virtual int
  cmpValues (long idx1, long idx2)
  {
    double d1 = data->fetch (idx1);
    double d2 = data->fetch (idx2);
    return d1 < d2 ? -1 : d1 > d2 ? 1 : 0;
  }

  virtual int
  cmpDatumValue (long idx, const Datum *val)
  {
    double d1 = data->fetch (idx);
    double d2 = val->d;
    return d1 < d2 ? -1 : d1 > d2 ? 1 : 0;
  }

private:
  Vector<double> *data;
};

Data *
Data::newData (VType_type vtype)
{
  switch (vtype)
    {
    case TYPE_INT32:
      return new DataINT32;
    case TYPE_UINT32:
      return new DataUINT32;
    case TYPE_INT64:
      return new DataINT64;
    case TYPE_UINT64:
      return new DataUINT64;
    case TYPE_OBJ:
      return new DataOBJECT;
    case TYPE_STRING:
      return new DataSTRING;
    case TYPE_DOUBLE:
      return new DataDOUBLE;
    default:
      return NULL;
    }
}

/*
 *    class DataDescriptor
 */
DataDescriptor::DataDescriptor (int _id, const char *_name, const char *_uname,
				int _flags)
{
  isMaster = true;
  id = _id;
  name = _name ? strdup (_name) : strdup (NTXT (""));
  uname = _uname ? strdup (_uname) : strdup (NTXT (""));
  flags = _flags;

  // master data, shared with reference copies:
  master_size = 0;
  master_resolveFrameInfoDone = false;
  props = new Vector<PropDescr*>;
  data = new Vector<Data*>;
  setsTBR = new Vector<Vector<long long>*>;

  // master references point to self:
  ref_size = &master_size;
  ref_resolveFrameInfoDone = &master_resolveFrameInfoDone;
}

DataDescriptor::DataDescriptor (int _id, const char *_name, const char *_uname,
				DataDescriptor* dDscr)
{
  isMaster = false;
  id = _id;
  name = _name ? strdup (_name) : strdup (NTXT (""));
  uname = _uname ? strdup (_uname) : strdup (NTXT (""));
  flags = dDscr->flags;

  // references point to master DataDescriptor
  ref_size = &dDscr->master_size;
  ref_resolveFrameInfoDone = &dDscr->master_resolveFrameInfoDone;
  props = dDscr->props;
  data = dDscr->data;
  setsTBR = dDscr->setsTBR;

  // data that should never be accessed in reference copy
  master_size = -1;
  master_resolveFrameInfoDone = false;
}

DataDescriptor::~DataDescriptor ()
{
  free (name);
  free (uname);
  if (!isMaster)
    return;
  props->destroy ();
  delete props;
  data->destroy ();
  delete data;
  setsTBR->destroy ();
  delete setsTBR;
}

void
DataDescriptor::reset ()
{
  if (!isMaster)
    return;
  for (int i = 0; i < data->size (); i++)
    {
      Data *d = data->fetch (i);
      if (d != NULL)
	d->reset ();
      Vector<long long> *set = setsTBR->fetch (i);
      if (set != NULL)
	set->reset ();
    }
  master_size = 0;
}

PropDescr *
DataDescriptor::getProp (int prop_id)
{
  for (int i = 0; i < props->size (); i++)
    {
      PropDescr *propDscr = props->fetch (i);
      if (propDscr->propID == prop_id)
	return propDscr;
    }
  return NULL;
}

Data *
DataDescriptor::getData (int prop_id)
{
  if (prop_id < 0 || prop_id >= data->size ())
    return NULL;
  return data->fetch (prop_id);
}

void
DataDescriptor::addProperty (PropDescr *propDscr)
{
  if (propDscr == NULL)
    return;
  if (propDscr->propID < 0)
    return;
  PropDescr *oldProp = getProp (propDscr->propID);
  if (oldProp != NULL)
    {
      checkCompatibility (propDscr->vtype, oldProp->vtype); //YXXX depends on experiment correctness
      delete propDscr;
      return;
    }
  props->append (propDscr);
  data->store (propDscr->propID, Data::newData (propDscr->vtype));
  setsTBR->store (propDscr->propID, NULL);
}

long
DataDescriptor::addRecord ()
{
  if (!isMaster)
    return -1;
  return master_size++;
}

static void
checkEntity (Vector<long long> *set, long long val)
{
  // Binary search
  int lo = 0;
  int hi = set->size () - 1;
  while (lo <= hi)
    {
      int md = (lo + hi) / 2;
      long long ent = set->fetch (md);
      if (ent < val)
	lo = md + 1;
      else if (ent > val)
	hi = md - 1;
      else
	return;
    }
  set->insert (lo, val);
}

void
DataDescriptor::setDatumValue (int prop_id, long idx, const Datum *val)
{
  if (idx >= *ref_size)
    return;
  Data *d = getData (prop_id);
  if (d != NULL)
    {
      VType_type datum_type = val->type;
      VType_type data_type = d->type ();
      checkCompatibility (datum_type, data_type);
      d->setDatumValue (idx, val);
      Vector<long long> *set = setsTBR->fetch (prop_id);
      if (set != NULL)// Sets are maintained
	checkEntity (set, d->fetchLong (idx));
    }
}

void
DataDescriptor::setValue (int prop_id, long idx, uint64_t val)
{
  if (idx >= *ref_size)
    return;
  Data *d = getData (prop_id);
  if (d != NULL)
    {
      d->setValue (idx, val);
      Vector<long long> *set = setsTBR->fetch (prop_id);
      if (set != NULL)// Sets are maintained
	checkEntity (set, d->fetchLong (idx));
    }
}

void
DataDescriptor::setObjValue (int prop_id, long idx, void *val)
{
  if (idx >= *ref_size)
    return;
  Data *d = getData (prop_id);
  if (d != NULL)
    d->setObjValue (idx, val);
}

DataView *
DataDescriptor::createView ()
{
  return new DataView (this);
}

DataView *
DataDescriptor::createImmutableView ()
{
  return new DataView (this, DataView::DV_IMMUTABLE);
}

DataView *
DataDescriptor::createExtManagedView ()
{
  return new DataView (this, DataView::DV_EXT_MANAGED);
}

int
DataDescriptor::getIntValue (int prop_id, long idx)
{
  Data *d = getData (prop_id);
  if (d == NULL || idx >= d->getSize ())
    return 0;
  return d->fetchInt (idx);
}

unsigned long long
DataDescriptor::getULongValue (int prop_id, long idx)
{
  Data *d = getData (prop_id);
  if (d == NULL || idx >= d->getSize ())
    return 0L;
  return d->fetchULong (idx);
}

long long
DataDescriptor::getLongValue (int prop_id, long idx)
{
  Data *d = getData (prop_id);
  if (d == NULL || idx >= d->getSize ())
    return 0L;
  return d->fetchLong (idx);
}

void *
DataDescriptor::getObjValue (int prop_id, long idx)
{
  Data *d = getData (prop_id);
  if (d == NULL || idx >= d->getSize ())
    return NULL;
  return d->fetchObject (idx);
}

static int
pcmp (const void *p1, const void *p2, const void *arg)
{
  long idx1 = *(long*) p1; // index1 into Data
  long idx2 = *(long*) p2; // index2 into Data
  for (Data **dsorted = (Data**) arg; *dsorted != DATA_SORT_EOL; dsorted++)
    {
      Data *data = *dsorted;
      if (data == NULL)// sort property not in this data, skip this criteria
	continue;
      int res = data->cmpValues (idx1, idx2);
      if (res)
	return res;
    }
  // Provide stable sort
  return idx1 < idx2 ? -1 : idx1 > idx2 ? 1 : 0;
}

Vector<long long> *
DataDescriptor::getSet (int prop_id)
{
  if (prop_id < 0 || prop_id >= setsTBR->size ())
    return NULL;
  Vector<long long> *set = setsTBR->fetch (prop_id);
  if (set != NULL)
    return set;

  Data *d = getData (prop_id);
  if (d == NULL)
    return NULL;
  set = new Vector<long long>;
  for (long i = 0; i<*ref_size; ++i)
    checkEntity (set, d->fetchLong (i));
  setsTBR->store (prop_id, set);

  return set;
}

/*
 *    class DataView
 */
DataView::DataView (DataDescriptor *_ddscr)
{
  init (_ddscr, DV_NORMAL);
}

DataView::DataView (DataDescriptor *_ddscr, DataViewType _type)
{
  init (_ddscr, _type);
}

void
DataView::init (DataDescriptor *_ddscr, DataViewType _type)
{
  ddscr = _ddscr;
  type = _type;
  switch (type)
    {
    case DV_IMMUTABLE:
      ddsize = ddscr->getSize ();
      index = NULL;
      break;
    case DV_NORMAL:
    case DV_EXT_MANAGED:
      ddsize = 0;
      index = new Vector<long>;
      break;
    }
  for (int ii = 0; ii < (MAX_SORT_DIMENSIONS + 1); ii++)
    sortedBy[ii] = DATA_SORT_EOL;
  filter = NULL;
}

DataView::~DataView ()
{
  delete filter;
  delete index;
}

void
DataView::appendDataDescriptorId (long pkt_id /* ddscr index */)
{
  if (type != DV_EXT_MANAGED)
    return; // updates allowed only on externally managed DataViews
  long curr_ddsize = ddscr->getSize ();
  if (pkt_id < 0 || pkt_id >= curr_ddsize)
    return; // error!
  index->append (pkt_id);
}

void
DataView::setDataDescriptorValue (int prop_id, long pkt_id, uint64_t val)
{
  ddscr->setValue (prop_id, pkt_id, val);
}

long long
DataView::getDataDescriptorValue (int prop_id, long pkt_id)
{
  return ddscr->getLongValue (prop_id, pkt_id);
}

Vector<PropDescr*>*
DataView::getProps ()
{
  return ddscr->getProps ();
};

PropDescr*
DataView::getProp (int prop_id)
{
  return ddscr->getProp (prop_id);
};

void
DataView::filter_in_chunks (fltr_dbe_ctx *dctx)
{
  Expression::Context *e_ctx = new Expression::Context (dctx->fltr->ctx->dbev, dctx->fltr->ctx->exp);
  Expression *n_expr = dctx->fltr->expr->copy ();
  bool noParFilter = dctx->fltr->noParFilter;
  FilterExp *nFilter = new FilterExp (n_expr, e_ctx, noParFilter);
  long iter = dctx->begin;
  long end = dctx->end;
  long orig_ddsize = dctx->orig_ddsize;
  while (iter < end)
    {
      nFilter->put (dctx->tmpView, iter);
      if (nFilter->passes ())
	dctx->idxArr[iter - orig_ddsize] = 1;
      iter += 1;
    }
  delete nFilter;
}

bool
DataView::checkUpdate ()
{
  long newSize = ddscr->getSize ();
  if (ddsize == newSize)
    return false;
  if (index == NULL)
    return false;
  if (type == DV_EXT_MANAGED)
    return false;
  bool updated = false;
  if (filter)
    {
      DataView *tmpView = ddscr->createImmutableView ();
      assert (tmpView->getSize () == newSize);
      while (ddsize < newSize)
	{
	  filter->put (tmpView, ddsize);
	  if (filter->passes ())
	    index->append (ddsize);
	  ddsize += 1;
	}
      delete tmpView;
      return updated;
    }
  while (ddsize < newSize)
    {
      index->append (ddsize);
      updated = true;
      ddsize += 1;
    }
  return updated;
}

long
DataView::getSize ()
{
  if (checkUpdate () && sortedBy[0] != DATA_SORT_EOL)
    // note: after new filter is set, getSize() incurs cost of
    // sorting even if caller isn't interested in sort
    index->sort ((CompareFunc) pcmp, sortedBy);

  if (index == NULL)
    return ddscr->getSize ();
  return index->size ();
}

void
DataView::setDatumValue (int prop_id, long idx, const Datum *val)
{
  ddscr->setDatumValue (prop_id, getIdByIdx (idx), val);
}

void
DataView::setValue (int prop_id, long idx, uint64_t val)
{
  ddscr->setValue (prop_id, getIdByIdx (idx), val);
}

void
DataView::setObjValue (int prop_id, long idx, void *val)
{
  ddscr->setObjValue (prop_id, getIdByIdx (idx), val);
}

int
DataView::getIntValue (int prop_id, long idx)
{
  return ddscr->getIntValue (prop_id, getIdByIdx (idx));
}

unsigned long long
DataView::getULongValue (int prop_id, long idx)
{
  return ddscr->getULongValue (prop_id, getIdByIdx (idx));
}

long long
DataView::getLongValue (int prop_id, long idx)
{
  return ddscr->getLongValue (prop_id, getIdByIdx (idx));
}

void *
DataView::getObjValue (int prop_id, long idx)
{
  return ddscr->getObjValue (prop_id, getIdByIdx (idx));
}

void
DataView::sort (const int props[], int prop_count)
{
  if (index == NULL)
    {
      assert (ASSERT_SKIP);
      return;
    }
  assert (prop_count >= 0 && prop_count < MAX_SORT_DIMENSIONS);
  bool sort_changed = false; // see if sort has changed...
  for (int ii = 0; ii <= prop_count; ii++)
    { // sortedBy size is prop_count+1
      Data *data;
      if (ii == prop_count)
	data = DATA_SORT_EOL; // special end of array marker
      else
	data = ddscr->getData (props[ii]);
      if (sortedBy[ii] != data)
	{
	  sortedBy[ii] = data;
	  sort_changed = true;
	}
    }
  if (!checkUpdate () && !sort_changed)
    return;
  index->sort ((CompareFunc) pcmp, sortedBy);
}

void
DataView::sort (int prop0)
{
  sort (&prop0, 1);
}

void
DataView::sort (int prop0, int prop1)
{
  int props[2] = {prop0, prop1};
  sort (props, 2);
}

void
DataView::sort (int prop0, int prop1, int prop2)
{
  int props[3] = {prop0, prop1, prop2};
  sort (props, 3);
}

void
DataView::setFilter (FilterExp *f)
{
  if (index == NULL)
    {
      assert (ASSERT_SKIP);
      return;
    }
  delete filter;
  filter = f;
  index->reset ();
  ddsize = 0;
  checkUpdate ();
}

long
DataView::getIdByIdx (long idx)
{
  if (index == NULL)
    return idx;
  return index->fetch (idx);
}

static int
tvalcmp (long data_id, const Datum valColumns[], Data *sortedBy[])
{
  for (int ii = 0; ii < MAX_SORT_DIMENSIONS; ii++)
    {
      if (sortedBy[ii] == DATA_SORT_EOL)
	break;
      Data *d = sortedBy[ii];
      if (d == NULL)// property doesn't exist in data; compare always matches
	continue;
      const Datum *tvalue = &valColumns[ii];
      int res = d->cmpDatumValue (data_id, tvalue);
      if (res)
	return res;
    }
  return 0;
}

static void
checkSortTypes (const Datum valColumns[], Data *sortedBy[])
{
#ifndef NDEBUG
  for (int ii = 0; ii < MAX_SORT_DIMENSIONS; ii++)
    {
      if (sortedBy[ii] == DATA_SORT_EOL)
	break;
      Data *d = sortedBy[ii];
      if (d == NULL)// property doesn't exist in data; compare always matches
	continue;
      VType_type datum_type = valColumns[ii].type;
      VType_type data_type = d->type ();
      checkCompatibility (datum_type, data_type);
    }
#endif
}

bool
DataView::idxRootDimensionsMatch (long idx, const Datum valColumns[])
{
  // compares idx vs. valColumns[] - If all dimensions match
  // (except sort leaf), then the leaf value is valid => return true.
  // Otherwise, return false.
  checkSortTypes (valColumns, sortedBy);
  if (idx < 0 || idx >= index->size ()) // fell off end of array
    return false;
  long data_id = index->fetch (idx);

  // we will check all dimensions for a match except the "leaf" dimension
  for (int ii = 0; ii < (MAX_SORT_DIMENSIONS - 1); ii++)
    {
      if (sortedBy[ii + 1] == DATA_SORT_EOL)
	break; // we are at leaf dimension, don't care about it's value
      if (sortedBy[ii] == DATA_SORT_EOL)
	break; // end of list
      Data *d = sortedBy[ii];
      if (d == NULL) // property doesn't exist in data; compare always matches
	continue;
      const Datum *tvalue = &valColumns[ii];
      int res = d->cmpDatumValue (data_id, tvalue);
      if (res)
	return false;
    }
  return true;
}

long
DataView::getIdxByVals (const Datum valColumns[], Relation rel)
{
  // checks sortedBy[] columns for match; relation only used on last column
  return getIdxByVals (valColumns, rel, -1, -1);
}

long
DataView::getIdxByVals (const Datum valColumns[], Relation rel,
			long minIdx, long maxIdx)
{
  // checks sortedBy[] columns for match; relation only used on last column
  checkSortTypes (valColumns, sortedBy);
  if (index == NULL || sortedBy[0] == DATA_SORT_EOL)
    return -1;

  long lo;
  if (minIdx < 0)
    lo = 0;
  else
    lo = minIdx;

  long hi;
  if (maxIdx < 0 || maxIdx >= index->size ())
    hi = index->size () - 1;
  else
    hi = maxIdx;

  long md = -1;
  while (lo <= hi)
    {
      md = (lo + hi) / 2;
      int cmp = tvalcmp (index->fetch (md), valColumns, sortedBy);
      if (cmp < 0)
	{
	  lo = md + 1;
	  continue;
	}
      else if (cmp > 0)
	{
	  hi = md - 1;
	  continue;
	}

      // cmp == 0, we have an exact match
      switch (rel)
	{
	case REL_LT:
	  hi = md - 1; // continue searching
	  break;
	case REL_GT:
	  lo = md + 1; // continue searching
	  break;
	case REL_LTEQ:
	case REL_GTEQ:
	case REL_EQ:
	  // note: "md" may not be deterministic if multiple matches exist
	  return md; // a match => done.
	}
    }

  // no exact match found
  switch (rel)
    {
    case REL_LT:
    case REL_LTEQ:
      md = hi;
      break;
    case REL_GT:
    case REL_GTEQ:
      md = lo;
      break;
    case REL_EQ:
      return -1;
    }
  if (idxRootDimensionsMatch (md, valColumns))
    return md;
  return -1;
}

void
DataView::removeDbeViewIdx (long idx)
{
  index->remove (idx);
}

