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

#ifndef _DbeLinkList_h
#define _DbeLinkList_h

template <typename ITEM> class DbeLinkList
{
public:

  DbeLinkList (ITEM _item)
  {
    item = _item;
    next = NULL;
  };

  ~DbeLinkList () { };

  ITEM
  get_item ()
  {
    return item;
  }

  DbeLinkList<ITEM> *
  get_next ()
  {
    return next;
  }

  void
  set_next (DbeLinkList<ITEM> *p)
  {
    next = p;
  }

  void
  destroy (bool deleteItem = false)
  {
    for (DbeLinkList<ITEM> *p = next; p;)
      {
	DbeLinkList<ITEM> *p1 = p->get_next ();
	if (deleteItem)
	  delete p->get_item ();
	delete p;
	p = p1;
      }
    next = NULL;
  }

private:
  ITEM item;
  DbeLinkList<ITEM> *next;
};

#endif /* _DbeLinkList_h */
