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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/time.h>

#include "stopwatch.h"

/* parameters defining various tasks */
#define BUFSIZE  16384
#define NBLKS  1024

#define SIZE ((int)(16*1024*1024))
unsigned buffer[SIZE];
extern FILE *fid2;

/*	ioerror - do some erroneous file IO operations */
int
ioerror ()
{
  FILE *fp; /* FILE pointer for stdio */
  char *fname = NULL;
  char *ptr = NULL;
  int fd; /* file descriptor for raw IO */
  int fd2; /* file descriptor for raw IO */
  int stat;
  char buf[BUFSIZE];
  unsigned long size = 0;
  char sfn[23] = "";

  /* Log the regular read */
  wlog ("start of ioerror", NULL);

  /* fname is set to NULL.
     Use various calls to create
     a file.
   */

  fd = creat (fname, 0666);
  fd = open (fname, 0666);
  fd2 = 0;
  fd = openat (fd2, fname, 0666);
  fp = fopen (fname, "w");
  fp = fopen ("/iotest", "w");
  fp = NULL;
  stat = fflush (fp);
  stat = chmod (fname, 755);
  stat = access (fname, 755);
  fname = "/tmp/synprogXXXXXX";
  strncpy (sfn, fname, sizeof (sfn));
  fd = mkstemp (sfn);
  stat = unlink (sfn);
  stat = rename (fname, NULL);
  unlink (fname);
  fp = fopen (fname, "w");
  stat = fclose (fp);
  stat = fread (buf, 100, 2, fp);
  stat = fwrite (buf, 100, 2, fp);
  ptr = fgets (buf, size, fp);
  read (10000, buf, 100);
  write (10000, buf, 100);
  stat = unlink (fname);
  fname = NULL;
  stat = mkdir (fname, 755);
  stat = unlink (fname);
  /* 
    These functions cannot be executed
    if the File Pointer (fp) is set
    to NULL. They generate segv failure
    in actual call not inside of 
    the wrapper.

    stat = fread(buf, size, 2, fp);
    stat = fwrite(buf, size, 2, fp);
    ptr = fgets(buf, size, fp);
    stat = fputs(buf, fp);
    stat = fprintf(fp, "%d\n", size);
    stat = fseek(fp, size, size);
    rewind(fp);
    ftell(fp);
    fpos_t pos;
    stat = fsetpos(fp, &pos);
    stat = fgetpos(fp, &pos);
   */
  return 0;
}

/*=======================================================*/

/* iofile - do some file io operations */
int
iofile ()
{
  FILE *fp; /* FILE pointer for stdio */
  int k; /* temp value for loop */
  int i;
  char *buf;
  hrtime_t start;
  hrtime_t vstart;
  char sfn[23] = "";
  char *fname = "/tmp/synprogXXXXXX";
  int ret;
  int readCnt = 0;
  int bRead = 0;
  int writeCnt = 0;
  int bWritten = 0;
  int otherIOCnt = 0;
  int bytes = 0;

  start = gethrtime ();
  vstart = gethrvtime ();

  /* Log the event */
  bytes = wlog ("start of iofile -- stdio", NULL);
  bWritten += bytes;
  writeCnt++;

  strncpy (sfn, fname, sizeof (sfn));
  ret = mkstemp (sfn);
  otherIOCnt++;
  if (ret == -1)
    {
      fprintf (stderr, "Unable to make a temporary name\n");
      exit (1);
    }
  bytes = fprintf (stderr, "\tUsing %s as scratch file\n", sfn);
  bWritten += bytes;
  writeCnt++;

  /* allocate a buffer for the reading */
  /* note that this buffer is leaked! */
  buf = (char *) malloc (BUFSIZE);

  /* open the file */
  fp = fdopen (ret, "w");
  otherIOCnt++;
  if (fp == NULL)
    {
      fprintf (stderr, "++ERROR opening %s, error %d\n", sfn, errno);
      exit (1);
    }

  /* loop, writing the buffer to the file... */
  for (i = 0; i < NBLKS; i++)
    {
      k = fwrite (buf, sizeof (char), BUFSIZE, fp);
      writeCnt++;
      if (k != BUFSIZE)
        {
          fprintf (stderr, "++ERROR writing %s, error %d\n", sfn, errno);
          exit (1);
        }
      bWritten += k;
    }

  fclose (fp);
  fp = NULL;
  otherIOCnt++;

  sprintf (buf, "fwrite: %d blocks of %d", i, BUFSIZE);
  bytes = whrvlog (gethrtime () - start, gethrvtime () - vstart, buf, NULL);
  bWritten += bytes;
  writeCnt++;


  /* now reopen the file, and read it */
  start = gethrtime ();
  vstart = gethrvtime ();

  fp = fopen (sfn, "r");
  otherIOCnt++;
  if (fp == NULL)
    {
      fprintf (stderr, "++ERROR opening %s, error %d\n", sfn, errno);
      exit (1);
    }
  i = 0;
  for (;;)
    {
      k = fread (buf, sizeof (char), BUFSIZE, fp);
      readCnt++;
      if (k < 0)
        fprintf (stderr, "++ERROR reading %s, error %d\n", sfn, errno);


      if (k == 0)
        {
          /* close the file */
          fclose (fp);
          fp = NULL;
          otherIOCnt++;
          break;

        }
      else if (k != BUFSIZE)
        {
          /* short read */
          sprintf (buf, "\tunexpecter short read %d on %s\n", k, sfn);
          fprintf (stderr, buf);
          bRead += k;
          break;
        }
      else
        {
          /* bump the block counter */
          i++;
          bRead += k;
        }
    }

  if (fp != NULL)
    {
      fclose (fp);
      fp = NULL;
    }
  sprintf (buf, "fread: %d blocks of %d", i, BUFSIZE);
  bytes = whrvlog (gethrtime () - start, gethrvtime () - vstart, buf, NULL);
  bWritten += bytes;
  writeCnt++;

  bWritten += 99; /* the number of bytes are written by the next fprintf */
  writeCnt++;

  unlink (sfn);
  otherIOCnt++;
  fprintf (fid2, "X   %14d  %14d  %17d  %15d  %17d   iofile\n",
           bRead, readCnt, bWritten, writeCnt, otherIOCnt);
  return 0;
}

/*  iotest - do various io syscalls */
int
iotest ()
{
  char *fname = "/tmp/foobar";
  int fd;   /* file descriptor for raw IO */
  int fd2;  /* file descriptor for raw IO */
  int k;    /* temp value for loop */
  char buf[BUFSIZE];
  unsigned long size = 0;
  int readCnt = 0;
  int bRead = 0;
  int writeCnt = 0;
  int bWritten = 0;
  int otherIOCnt = 0;
  int bytes = 0;

  /* Log the regular read */
  bytes = wlog ("start of iotest", NULL);
  bWritten += bytes;
  writeCnt++;

  /* create an empty file */
  fd = creat (fname, 0666);
  otherIOCnt++;

  /* dup the file descriptor */
  fd2 = dup (fd);
  otherIOCnt++;
  close (fd2);
  otherIOCnt++;
  close (fd);
  otherIOCnt++;

  /* now open the empty file */
  fd = open (fname, O_RDONLY);
  otherIOCnt++;

  /* loop, reading into the buffer */
  size = 0;
  for (;;)
    {
      k = read (fd, buf, BUFSIZE);
      readCnt++;
      if (k < 0)
        fprintf (stderr, "++ERROR reading %s, error %d\n", fname, errno);
      else
        {
          size = size + k;
          bRead += k;
        }
      if (k != BUFSIZE)
        {
          /* close the file */
          close (fd);
          fd = -1;
          otherIOCnt++;
          bRead += k;

          /* short eread = EOF */
          break;
        }
    }
  if (fd != -1)
    {
      close (fd);
      fd = -1;
    }
  bWritten += 99; /* the number of bytes are written by the next fprintf */
  writeCnt++;

  /* remove the file */
  unlink (fname);
  otherIOCnt++;
  fprintf (fid2, "X   %14d  %14d  %17d  %15d  %17d   iotest\n",
           bRead, readCnt, bWritten, writeCnt, otherIOCnt);

  return 0;
}

/*
 * Memory mapping routines-
 *
 *  Allocate and deallocate memory using mmap and malloc.
 *
 *  There is one parameter--the total number of megabytes to write,
 *  written in as many 16 megabyte files as are needed
 */

unsigned char *start = (unsigned char*) 0x80000000;
unsigned char *stop;
int nblocks;

void
memorymap (int megabytes)
{
  int readCnt = 0;
  int bRead = 0;
  int writeCnt = 0;
  int bWritten = 0;
  int otherIOCnt = 0;
  int bytes;

  /*
   * First, see how much time it takes to mmap all the files.
   *
   * Second, pull in just a few pages of information to see how much
   * time the "How much IBM do I hold?" question would take.
   *
   * Next, compare updating the database shared with updating it private
   * and then recopying the changed segments.

   * (We could catch the pages that we have altered by mapping the
   * entire BIS read-only and then punching holes in it via an
   * mprotect call as we catch segfaults.  This gives us a list
   * of the pages that we need to write, at the added expense of
   * handling lots of interrupts.)
   * (Notice that we don't test the case where we are adding to
   * the BIS files.  This is an interesting situation as we either
   * have to open the last page past the last write point or reopen
   * extendable in some way.  We could do that by opening /dev/zero
   * with MAP_ANON for addresses above our current usage point.
   */

  int i;
  stop = start + 1024 * 1024 * (long long) megabytes;

  printf ("Creating %d random numbers\n", SIZE);
  for (i = 0; i < SIZE; i++)
    buffer[i] = random ();  // set pseudo-bis to noise
  printf ("Done creating random numbers\n");


  /*
   * Write a database consisting of 16 megabyte files.
   * Each filename contains the memory address into which
   * the file should be reloaded.
   */

  printf ("Writing pseudo-bis files\n");
  unsigned char* base = start;
  nblocks = 0;
  for (i = 0; i < megabytes; i += 16)
    {
      nblocks++;
      // write data in 16MB files
      char filename[256];
      sprintf (filename, "bistest.%p.%d", base, i);
      int fd = open (filename, O_CREAT | O_TRUNC | O_WRONLY, 0660);
      otherIOCnt++;
      if (fd == -1)
        {
          printf ("open of %s failed: %s\n", filename, strerror (errno));
          exit (0);
        }
      bytes = write (fd, buffer, SIZE);
      bWritten += bytes;
      writeCnt++;
      close (fd);
      otherIOCnt++;
      printf ("\twrote %d megabytes\n", i + 16);
      base += 16 * 1024 * 1024;
    }
  printf ("Done writing files from %p to %p\n", start, stop);

  int j;

  printf ("Memory map all the files (private)\n");
  for (i = 0; i < megabytes; i += 16)
    {
      unsigned char* base = start;
      base += i * 1024 * 1024;
      char filename[256];
      sprintf (filename, "bistest.%p.%d", base, i);
      int fd = open (filename, O_RDWR);
      otherIOCnt++;
      if (fd < 0)
        printf ("open of %s failed: %s\n", filename, strerror (errno));
      unsigned char *mp = (unsigned char*) mmap ((char*) base,
                                                 SIZE, PROT_READ | PROT_WRITE,
                                                 MAP_PRIVATE | MAP_FIXED, fd, 0);
      if (mp == MAP_FAILED || mp != base)
        {
          printf ("mmap of %s failed: %s\n", filename, strerror (errno));
          exit (1);
        }

      printf ("mapped %d bytes at %p\n", SIZE, base);
      close (fd); // mmap will hold the file open for us
      otherIOCnt++;
    }

  printf ("Mapping done\n");
  fflush (stdout);
  otherIOCnt++;

  int ranlimit = 1000;
  printf ("Access %d bytes at random\n", ranlimit);
  int sum = 0;
  for (i = 0; i < ranlimit; i++)
    {
      unsigned char *where = start +
              (((unsigned long) random ()) % (stop - start));
      sum += (int) *where;
    }
  printf ("Random byte access done\n");

  ranlimit = 1000;
  int ranrange = 256;
  printf ("Alter %d random locations, %d bytes each (private)\n",
          ranlimit, ranrange);

  for (i = 0; i < ranlimit; i++)
    {
      unsigned char *where = start +
              (((unsigned long) random ()) % (stop - start));
      for (j = 0; j < ranrange; j++)
        *where++ = j;
    }

  printf ("Memory alteration done\n");
  fflush (stdout);
  otherIOCnt++;

  printf ("Copy all memory back to disk\n");

  for (i = 0; i < megabytes; i += 16)
    {
      unsigned char* base = start;
      base += i * 1024 * 1024;
      char filename[256];
      sprintf (filename, "bistest2.%p.%d", base, i);
      int fd = open (filename, O_RDWR | O_CREAT | O_TRUNC, 0660);
      otherIOCnt++;
      if ((bytes = write (fd, base, SIZE)) == -1)
        {
          printf ("write of %s failed: %s\n", filename, strerror (errno));
          exit (1);
        }
      bWritten += bytes;
      writeCnt++;
      close (fd);
      otherIOCnt++;
    }

  printf ("Disk copy complete\n");
  fflush (stdout);
  otherIOCnt++;

  printf ("Unmap all segments\n");
  for (i = 0; i < megabytes; i += 16)
    {
      unsigned char* base = start;
      base += i * 1024 * 1024;
      if (munmap ((char*) base, SIZE) == -1)
        {
          printf ("munmap failed: %s\n", strerror (errno));
          exit (1);
        }
      printf ("unmapped %d bytes at %p\n", SIZE, base);
    }
  printf ("Segment unmapping complete\n");
  fflush (stdout);
  otherIOCnt++;

  printf ("Remap all segments as shared\n");
  for (i = 0; i < megabytes; i += 16)
    {
      unsigned char* base = start;
      base += i * 1024 * 1024;
      char filename[256];
      sprintf (filename, "bistest.%p.%d", base, i);
      int fd = open (filename, O_RDWR);
      otherIOCnt++;
      char* mp = mmap ((char*) base, SIZE, PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_FIXED, fd, 0);
      if (mp == MAP_FAILED || (unsigned char*) mp != base)
        {
          printf ("re mmap of %s failed: %s\n", filename, strerror (errno));
          exit (1);
        }
      printf ("remapped %d bytes at %p\n", SIZE, base);
      close (fd); // mmap will hold the file open for us
      otherIOCnt++;
    }
  printf ("Remapping complete\n");
  fflush (stdout);
  otherIOCnt++;

  ranlimit = 1000;
  ranrange = 256;
  printf ("Alter %d random locations, %d bytes each (shared)\n",
          ranlimit, ranrange);
  for (i = 0; i < ranlimit; i++)
    {
      unsigned char* where = start +
              (((unsigned long) random ()) % (stop - start));
      for (j = 0; j < ranrange; j++)
        *where++ = j;
    }
  printf ("Memory alteration done\n");
  fflush (stdout);
  otherIOCnt++;

  printf ("Unmap all segments\n");
  for (i = 0; i < megabytes; i += 16)
    {
      unsigned char *base = start;
      base += i * 1024 * 1024;
      if (munmap ((char*) base, SIZE) == -1)
        {
          printf ("munmap failed: %s\n", strerror (errno));
          exit (1);
        }
      printf ("unmapped %d bytes at %p\n", SIZE, base);
    }
  printf ("Segment unmapping complete\n");
  fflush (stdout);
  otherIOCnt++;

  base = start;

  for (i = 0; i < megabytes; i += 16)
    {
      // write data in 16MB files
      char filename[256];
      sprintf (filename, "bistest.%p.%d", base, i);
      if (unlink (filename) != 0)
        {
          printf ("unlink of %s failed: %s\n", filename, strerror (errno));
        }
      base += 16 * 1024 * 1024;
      otherIOCnt++;
    }

  for (i = 0; i < megabytes; i += 16)
    {
      unsigned char* base = start;
      base += i * 1024 * 1024;
      char filename[256];
      sprintf (filename, "bistest2.%p.%d", base, i);
      if (unlink (filename) != 0)
        {
          printf ("unlink of %s failed: %s\n", filename, strerror (errno));
        }
      otherIOCnt++;
    }
  bWritten += 102; /* the number of bytes are written by the next fprintf */
  writeCnt++;

  fflush (fid2);
  otherIOCnt++;

  /* Record accounting record */
  fprintf (fid2, "X   %14d  %14d  %17d  %15d  %17d   memorymap\n",
           bRead, readCnt, bWritten, writeCnt, otherIOCnt);
  printf ("Deleted scratch files\n");
}
