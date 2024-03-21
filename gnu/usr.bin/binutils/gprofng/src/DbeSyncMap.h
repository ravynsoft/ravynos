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

#ifndef _DbeSyncMap_h
#define _DbeSyncMap_h

#include "DbeLock.h"
#include "DbeLinkList.h"
#include "vec.h"

typedef unsigned long hash_ty;

template <class ITEM> class DbeSyncMap : public DbeLock
{
public:
  DbeSyncMap (int _chunkSize = DefaultChunkSize);
  virtual ~DbeSyncMap ();
  void reset ();
  ITEM *get (const char *nm, int64_t chksum);
  ITEM *sync_create_item (const char *nm, int64_t chksum);
  ITEM *get (const char *nm, const char *path, DbeFile *df);
  ITEM *sync_create_item (const char *nm, const char *path, DbeFile *df);
  virtual void dump ();

  Vector<ITEM *> *
  values ()
  {
    return items;
  };

private:
  hash_ty hash (const char *key);

  DbeLinkList<ITEM *> **chunk;
  Vector<ITEM *> *items;
  long chunkSize;

  enum
  {
    DefaultChunkSize = 1024
  };
};

template <class ITEM>
DbeSyncMap<ITEM>::DbeSyncMap (int _chunkSize)
{
  chunkSize = _chunkSize;
  chunk = new DbeLinkList<ITEM *> * [chunkSize];
  for (long i = 0; i < chunkSize; i++)
    chunk[i] = NULL;
  items = new Vector<ITEM *>(512);
}

template <class ITEM>
DbeSyncMap<ITEM>::~DbeSyncMap ()
{
  for (long i = 0; i < chunkSize; i++)
    Destroy (chunk[i]);
  delete[] chunk;
  delete items;
}

template <class ITEM>
void
DbeSyncMap<ITEM>::reset ()
{
  for (long i = 0; i < chunkSize; i++)
    {
      Destroy (chunk[i]);
      chunk[i] = NULL;
    }
  items->reset ();
}

template <class ITEM>
ITEM *
DbeSyncMap<ITEM>::get (const char *nm, int64_t chksum)
{
  hash_ty h = hash (nm);
  for (DbeLinkList<ITEM *> *dl = chunk[h]; dl; dl = dl->get_next ())
    {
      ITEM *item = dl->get_item ();
      if (item->compare (nm, chksum))
	return item;
    }
  return NULL;
}

template <class ITEM>
hash_ty
DbeSyncMap<ITEM>::hash (const char *key)
{
  return (hash_ty) (crc64 (key, strlen (key)) % chunkSize);
}

template <class ITEM>
ITEM *
DbeSyncMap<ITEM>::sync_create_item (const char *nm, int64_t chksum)
{
  hash_ty h = hash (nm);
  for (DbeLinkList<ITEM *> *dl = chunk[h]; dl; dl = dl->get_next ())
    {
      ITEM *item = dl->get_item ();
      if (item->compare (nm, chksum))
	return item;
    }
  aquireLock ();
  for (DbeLinkList<ITEM *> *dl = chunk[h]; dl; dl = dl->get_next ())
    {
      ITEM *item = dl->get_item ();
      if (item->compare (nm, chksum))
	{
	  releaseLock ();
	  return item;
	}
    }
  ITEM *item = ITEM::create_item (nm, chksum);
  DbeLinkList<ITEM *> *dl = new DbeLinkList<ITEM *>(item);
  dl->set_next (chunk[h]);
  chunk[h] = dl;
  items->append (item);
  releaseLock ();
  return item;
}

template <class ITEM>
ITEM *
DbeSyncMap<ITEM>::get (const char *nm, const char *path, DbeFile *df)
{
  int mask = 1 + (path != NULL ? 2 : 0) + (df != NULL ? 4 : 0);
  hash_ty h = hash (nm);
  for (DbeLinkList<ITEM *> *dl = chunk[h]; dl; dl = dl->get_next ())
    {
      ITEM *item = dl->get_item ();
      if (mask == item->compare (nm, path, df))
	return item;
    }
  return NULL;
}

template <class ITEM>
ITEM *
DbeSyncMap<ITEM>::sync_create_item (const char *nm, const char *path, DbeFile *df)
{
  int mask = CMP_PATH;
  if (path != NULL)
    mask += CMP_RUNTIMEPATH;
  if (df != NULL)
    mask += CMP_CHKSUM;
  hash_ty h = hash (nm);
  for (DbeLinkList<ITEM *> *dl = chunk[h]; dl; dl = dl->get_next ())
    {
      ITEM *item = dl->get_item ();
      if (mask == item->compare (nm, path, df))
	return item;
    }
  aquireLock ();
  for (DbeLinkList<ITEM *> *dl = chunk[h]; dl; dl = dl->get_next ())
    {
      ITEM *item = dl->get_item ();
      if (mask == item->compare (nm, path, df))
	{
	  releaseLock ();
	  return item;
	}
    }
  ITEM *item = ITEM::create_item (nm, path, df);
  DbeLinkList<ITEM *> *dl = new DbeLinkList<ITEM *>(item);
  dl->set_next (chunk[h]);
  chunk[h] = dl;
  items->append (item);
  releaseLock ();
  return item;
}

template <class ITEM>
void
DbeSyncMap<ITEM>::dump ()
{
  Dprintf (1, NTXT ("\nDbeSyncMap::dump:  vals=%ld\n"), (long) VecSize (items));
  int tot = 0;
  int max_cnt = 0;
  for (long i = 0; i < chunkSize; i++)
    {
      DbeLinkList<ITEM *> *lp = chunk[i];
      if (lp)
	{
	  int cnt = 0;
	  for (DbeLinkList<ITEM *> *lp1 = lp; lp1; lp1 = lp1->get_next ())
	    cnt++;
	  tot += cnt;
	  if (max_cnt < cnt)
	    max_cnt = cnt;
	  cnt = 1;
	  for (DbeLinkList<ITEM *> *lp1 = lp; lp1; lp1 = lp1->get_next ())
	    {
	      ITEM *p = lp1->get_item ();
	      Dprintf (1, NTXT ("      %2d %s\n"), cnt, p->get_name ());
	      cnt++;
	    }
	}
    }
  Dprintf (1, NTXT ("\nDbeSyncMap::dump: vals=%ld max_cnt=%d tot=%d\n"),
	   (long) VecSize (items), max_cnt, tot);
}

#endif /* _DbeSyncMap_h */
