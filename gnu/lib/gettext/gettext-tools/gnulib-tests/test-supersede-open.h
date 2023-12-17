/* Tests for opening a file without destroying an old file with the same name.

   Copyright (C) 2020-2023 Free Software Foundation, Inc.

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

/* Written by Bruno Haible, 2020.  */

static void
test_open_supersede (bool supersede_if_exists, bool supersede_if_does_not_exist)
{
  char xtemplate[] = "gnulibtestXXXXXX";
  char *dir = mkdtemp (xtemplate);
  char *filename = file_name_concat (dir, "test.mo", NULL);
  struct stat statbuf;

  /* Test the case that the file does not yet exist.  */
  {
    ASSERT (stat (filename, &statbuf) < 0);

    struct supersede_final_action action;
    int fd = open_supersede (filename, O_RDWR | O_BINARY | O_TRUNC, 0666,
                             supersede_if_exists, supersede_if_does_not_exist,
                             &action);
    ASSERT (fd >= 0);
    ASSERT (write (fd, "Hello world\n", 12) == 12);
    if (supersede_if_does_not_exist)
      ASSERT (stat (filename, &statbuf) < 0);
    else
      ASSERT (stat (filename, &statbuf) == 0);
    ASSERT (close_supersede (fd, &action) == 0);

    ASSERT (stat (filename, &statbuf) == 0);

    size_t file_size;
    char *file_contents = read_file (filename, RF_BINARY, &file_size);
    ASSERT (file_size == 12);
    ASSERT (memcmp (file_contents, "Hello world\n", 12) == 0);
  }

  /* Test the case that the file exists and is a regular file.  */
  {
    ASSERT (stat (filename, &statbuf) == 0);
    dev_t orig_dev = statbuf.st_dev;
    ino_t orig_ino = statbuf.st_ino;

    struct supersede_final_action action;
    int fd = open_supersede (filename, O_RDWR | O_BINARY | O_TRUNC, 0666,
                             supersede_if_exists, supersede_if_does_not_exist,
                             &action);
    ASSERT (fd >= 0);
    ASSERT (write (fd, "Foobar\n", 7) == 7);
    ASSERT (stat (filename, &statbuf) == 0);
    {
      size_t file_size;
      char *file_contents = read_file (filename, RF_BINARY, &file_size);
      if (supersede_if_exists)
        {
          ASSERT (file_size == 12);
          ASSERT (memcmp (file_contents, "Hello world\n", 12) == 0);
        }
      else
        {
          ASSERT (file_size == 7);
          ASSERT (memcmp (file_contents, "Foobar\n", 7) == 0);
        }
    }
    ASSERT (close_supersede (fd, &action) == 0);

    ASSERT (stat (filename, &statbuf) == 0);

    size_t file_size;
    char *file_contents = read_file (filename, RF_BINARY, &file_size);
    ASSERT (file_size == 7);
    ASSERT (memcmp (file_contents, "Foobar\n", 7) == 0);

    if (supersede_if_exists)
      {
        /* Verify that the file now has a different inode number, on the same
           device.  */
#if !(defined _WIN32 && !defined __CYGWIN__)
        /* Note: On Linux/mips, statbuf.st_dev is smaller than a dev_t!  */
        dev_t new_dev = statbuf.st_dev;
        ASSERT (memcmp (&orig_dev, &new_dev, sizeof (dev_t)) == 0);
        ASSERT (memcmp (&orig_ino, &statbuf.st_ino, sizeof (ino_t)) != 0);
#endif
      }
  }

  /* Test the case that the file exists and is a character device.  */
  {
    ASSERT (stat (DEV_NULL, &statbuf) == 0);

    struct supersede_final_action action;
    int fd = open_supersede (DEV_NULL, O_RDWR | O_BINARY | O_TRUNC, 0666,
                             supersede_if_exists, supersede_if_does_not_exist,
                             &action);
    ASSERT (fd >= 0);
    ASSERT (write (fd, "Foobar\n", 7) == 7);
    ASSERT (stat (DEV_NULL, &statbuf) == 0);
    ASSERT (close_supersede (fd, &action) == 0);

    ASSERT (stat (DEV_NULL, &statbuf) == 0);
  }

  /* Test the case that the file is a symbolic link to an existing regular
     file.  */
  {
    const char *linkname = "link1";
    unlink (linkname);
    if (symlink (filename, linkname) >= 0)
      {
        ASSERT (stat (linkname, &statbuf) == 0);
        dev_t orig_dev = statbuf.st_dev;
        ino_t orig_ino = statbuf.st_ino;

        struct supersede_final_action action;
        int fd =
          open_supersede (linkname, O_RDWR | O_BINARY | O_TRUNC, 0666,
                          supersede_if_exists, supersede_if_does_not_exist,
                          &action);
        ASSERT (fd >= 0);
        ASSERT (write (fd, "New\n", 4) == 4);
        ASSERT (stat (linkname, &statbuf) == 0);
        {
          size_t file_size;
          char *file_contents = read_file (linkname, RF_BINARY, &file_size);
          if (supersede_if_exists)
            {
              ASSERT (file_size == 7);
              ASSERT (memcmp (file_contents, "Foobar\n", 7) == 0);
            }
          else
            {
              ASSERT (file_size == 4);
              ASSERT (memcmp (file_contents, "New\n", 4) == 0);
            }
        }
        ASSERT (close_supersede (fd, &action) == 0);

        ASSERT (stat (linkname, &statbuf) == 0);

        size_t file_size;
        char *file_contents = read_file (linkname, RF_BINARY, &file_size);
        ASSERT (file_size == 4);
        ASSERT (memcmp (file_contents, "New\n", 4) == 0);

        if (supersede_if_exists)
          {
            /* Verify that the file now has a different inode number, on the
               same device.  */
#if !(defined _WIN32 && !defined __CYGWIN__)
            /* Note: On Linux/mips, statbuf.st_dev is smaller than a dev_t!  */
            dev_t new_dev = statbuf.st_dev;
            ASSERT (memcmp (&orig_dev, &new_dev, sizeof (dev_t)) == 0);
            ASSERT (memcmp (&orig_ino, &statbuf.st_ino, sizeof (ino_t)) != 0);
#endif
          }

        /* Clean up.  */
        unlink (linkname);
      }
  }

  /* Test the case that the file is a symbolic link to an existing character
     device.  */
  {
    const char *linkname = "link2";
    unlink (linkname);
    if (symlink (DEV_NULL, linkname) >= 0)
      {
        ASSERT (stat (linkname, &statbuf) == 0);

        struct supersede_final_action action;
        int fd =
          open_supersede (linkname, O_RDWR | O_BINARY | O_TRUNC, 0666,
                          supersede_if_exists, supersede_if_does_not_exist,
                          &action);
        ASSERT (fd >= 0);
        ASSERT (write (fd, "New\n", 4) == 4);
        ASSERT (stat (linkname, &statbuf) == 0);
        ASSERT (close_supersede (fd, &action) == 0);

        ASSERT (stat (linkname, &statbuf) == 0);

        /* Clean up.  */
        unlink (linkname);
      }
  }

  /* Clean up.  */
  unlink (filename);

  /* Test the case that the file is a symbolic link to a nonexistent file in an
     existing directory.  */
  {
    const char *linkname = "link3";
    unlink (linkname);
    if (symlink (filename, linkname) >= 0)
      {
        ASSERT (stat (linkname, &statbuf) < 0);

        struct supersede_final_action action;
        int fd =
          open_supersede (linkname, O_RDWR | O_BINARY | O_TRUNC, 0666,
                          supersede_if_exists, supersede_if_does_not_exist,
                          &action);
        ASSERT (fd >= 0);
        ASSERT (write (fd, "Hello world\n", 12) == 12);
        if (supersede_if_does_not_exist)
          ASSERT (stat (linkname, &statbuf) < 0);
        else
          ASSERT (stat (linkname, &statbuf) == 0);
        ASSERT (close_supersede (fd, &action) == 0);

        ASSERT (stat (linkname, &statbuf) == 0);

        size_t file_size;
        char *file_contents = read_file (linkname, RF_BINARY, &file_size);
        ASSERT (file_size == 12);
        ASSERT (memcmp (file_contents, "Hello world\n", 12) == 0);

        /* Clean up.  */
        unlink (linkname);
      }
  }

  /* Test the case that the file is a symbolic link to a nonexistent file in a
     nonexistent directory.  */
  {
    const char *linkname = "link4";
    unlink (linkname);
    if (symlink ("/nonexistent/gnulibtest8237/24715863701440", linkname) >= 0)
      {
        ASSERT (stat (linkname, &statbuf) < 0);

        struct supersede_final_action action;
        int fd =
          open_supersede (linkname, O_RDWR | O_BINARY | O_TRUNC, 0666,
                          supersede_if_exists, supersede_if_does_not_exist,
                          &action);
        ASSERT (fd < 0);
        ASSERT (errno == ENOENT);

        ASSERT (stat (linkname, &statbuf) < 0);

        /* Clean up.  */
        unlink (linkname);
      }
  }

  /* Clean up.  */
  unlink (filename);
  rmdir (dir);
}
