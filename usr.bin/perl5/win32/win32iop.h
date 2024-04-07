#ifndef WIN32IOP_H
#define WIN32IOP_H

#ifndef START_EXTERN_C
#ifdef __cplusplus
#  define START_EXTERN_C extern "C" {
#  define END_EXTERN_C }
#  define EXTERN_C extern "C"
#else
#  define START_EXTERN_C
#  define END_EXTERN_C
#  define EXTERN_C
#endif
#endif

#include <sys/utime.h>

/*
 * defines for flock emulation
 */
#define LOCK_SH 1
#define LOCK_EX 2
#define LOCK_NB 4
#define LOCK_UN 8

/*
 * Make this as close to original stdio as possible.
 */

/*
 * function prototypes for our own win32io layer
 */
START_EXTERN_C

DllExport  int * 	win32_errno(void);
DllExport  char *** 	win32_environ(void);
DllExport  FILE*	win32_stdin(void);
DllExport  FILE*	win32_stdout(void);
DllExport  FILE*	win32_stderr(void);
DllExport  int		win32_ferror(FILE *fp);
DllExport  int		win32_feof(FILE *fp);
DllExport  char*	win32_strerror(int e);

DllExport  int		win32_fprintf(FILE *pf, const char *format, ...);
DllExport  int		win32_printf(const char *format, ...);
DllExport  int		win32_vfprintf(FILE *pf, const char *format, va_list arg);
DllExport  int		win32_vprintf(const char *format, va_list arg);
DllExport  size_t	win32_fread(void *buf, size_t size, size_t count, FILE *pf);
DllExport  size_t	win32_fwrite(const void *buf, size_t size, size_t count, FILE *pf);
DllExport  FILE*	win32_fopen(const char *path, const char *mode);
DllExport  FILE*	win32_fdopen(int fh, const char *mode);
DllExport  FILE*	win32_freopen(const char *path, const char *mode, FILE *pf);
DllExport  int		win32_fclose(FILE *pf);
DllExport  int		win32_fputs(const char *s,FILE *pf);
DllExport  int		win32_fputc(int c,FILE *pf);
DllExport  int		win32_ungetc(int c,FILE *pf);
DllExport  int		win32_getc(FILE *pf);
DllExport  int		win32_fileno(FILE *pf);
DllExport  void		win32_clearerr(FILE *pf);
DllExport  int		win32_fflush(FILE *pf);
DllExport  Off_t	win32_ftell(FILE *pf);
DllExport  int		win32_fseek(FILE *pf,Off_t offset,int origin);
DllExport  int		win32_fgetpos(FILE *pf,fpos_t *p);
DllExport  int		win32_fsetpos(FILE *pf,const fpos_t *p);
DllExport  void		win32_rewind(FILE *pf);
DllExport  int		win32_tmpfd(void);
DllExport  int		win32_tmpfd_mode(int mode);
DllExport  FILE*	win32_tmpfile(void);
DllExport  void		win32_abort(void);
DllExport  int  	win32_fstat(int fd,Stat_t *sbufptr);
DllExport  int  	win32_stat(const char *name,Stat_t *sbufptr);
DllExport  int  	win32_lstat(const char *name,Stat_t *sbufptr);
DllExport  int		win32_pipe( int *phandles, unsigned int psize, int textmode );
DllExport  PerlIO*	win32_popen( const char *command, const char *mode );
DllExport  PerlIO*	win32_popenlist(const char *mode, IV narg, SV **args);
DllExport  int		win32_pclose( PerlIO *pf);
DllExport  int		win32_rename( const char *oname, const char *newname);
DllExport  int		win32_setmode( int fd, int mode);
DllExport  int		win32_chsize(int fd, Off_t size);
DllExport  Off_t	win32_lseek( int fd, Off_t offset, int origin);
DllExport  Off_t	win32_tell( int fd);
DllExport  int		win32_dup( int fd);
DllExport  int		win32_dup2(int h1, int h2);
DllExport  int		win32_open(const char *path, int oflag,...);
DllExport  int		win32_close(int fd);
DllExport  int		win32_eof(int fd);
DllExport  int		win32_isatty(int fd);
DllExport  int		win32_read(int fd, void *buf, unsigned int cnt);
DllExport  int		win32_write(int fd, const void *buf, unsigned int cnt);
DllExport  int		win32_spawnvp(int mode, const char *cmdname,
                              const char *const *argv);
DllExport  int		win32_mkdir(const char *dir, int mode);
DllExport  int		win32_rmdir(const char *dir);
DllExport  int		win32_chdir(const char *dir);
DllExport  int		win32_flock(int fd, int oper);
DllExport  int		win32_execv(const char *cmdname, const char *const *argv);
DllExport  int		win32_execvp(const char *cmdname, const char *const *argv);
DllExport  void		win32_perror(const char *str);
DllExport  void		win32_setbuf(FILE *pf, char *buf);
DllExport  int		win32_setvbuf(FILE *pf, char *buf, int type, size_t size);
DllExport  int		win32_flushall(void);
DllExport  int		win32_fcloseall(void);
DllExport  char*	win32_fgets(char *s, int n, FILE *pf);
DllExport  char*	win32_gets(char *s);
DllExport  int		win32_fgetc(FILE *pf);
DllExport  int		win32_putc(int c, FILE *pf);
DllExport  int		win32_puts(const char *s);
DllExport  int		win32_getchar(void);
DllExport  int		win32_putchar(int c);
DllExport  void*	win32_malloc(size_t size);
DllExport  void*	win32_calloc(size_t numitems, size_t size);
DllExport  void*	win32_realloc(void *block, size_t size);
DllExport  void		win32_free(void *block);

DllExport  int		win32_open_osfhandle(intptr_t handle, int flags);
DllExport  intptr_t	win32_get_osfhandle(int fd);
DllExport  FILE*	win32_fdupopen(FILE *pf);

DllExport  DIR*		win32_opendir(const char *filename);
DllExport  struct direct*	win32_readdir(DIR *dirp);
DllExport  long		win32_telldir(DIR *dirp);
DllExport  void		win32_seekdir(DIR *dirp, long loc);
DllExport  void		win32_rewinddir(DIR *dirp);
DllExport  int		win32_closedir(DIR *dirp);
DllExport  DIR*		win32_dirp_dup(DIR *const dirp, CLONE_PARAMS *const param);

DllExport  char*        win32_getenvironmentstrings(void);
/* also see win32_freeenvironmentstrings macro */
DllExport  char*	win32_getenv(const char *name);
DllExport  int		win32_putenv(const char *name);

DllExport  unsigned 	win32_sleep(unsigned int);
DllExport  int		win32_pause(void);
DllExport  int		win32_times(struct tms *timebuf);
DllExport  unsigned 	win32_alarm(unsigned int sec);
DllExport  char*	win32_longpath(char *path);
DllExport  char*	win32_ansipath(const WCHAR *path);
DllExport  int		win32_ioctl(int i, unsigned int u, char *data);
DllExport  int          win32_link(const char *oldname, const char *newname);
DllExport  int          win32_symlink(const char *oldname, const char *newname);
DllExport  int          win32_readlink(const char *path, char *buf, size_t bufsiz);
DllExport  int		win32_unlink(const char *f);
DllExport  int		win32_utime(const char *f, struct utimbuf *t);
DllExport  int		win32_gettimeofday(struct timeval *tp, void *not_used);
DllExport  int		win32_uname(struct utsname *n);
DllExport  int		win32_wait(int *status);
DllExport  int		win32_waitpid(int pid, int *status, int flags);
DllExport  int		win32_kill(int pid, int sig);
DllExport  unsigned long	win32_os_id(void);
DllExport  void*	win32_dynaload(const char*filename);
DllExport  int		win32_access(const char *path, int mode);
DllExport  int		win32_chmod(const char *path, int mode);
DllExport  int		win32_getpid(void);

DllExport char *	win32_crypt(const char *txt, const char *salt);

DllExport void *	win32_get_childenv(void);
DllExport void		win32_free_childenv(void* d);
DllExport void		win32_clearenv(void);
DllExport char *	win32_get_childdir(void);
DllExport void		win32_free_childdir(char* d);
DllExport Sighandler_t	win32_signal(int sig, Sighandler_t subcode);


END_EXTERN_C

/* see comment in win32_getenvironmentstrings */
#define win32_freeenvironmentstrings(x) win32_free(x)
#undef alarm
#define alarm				win32_alarm
#undef strerror
#define strerror			win32_strerror

/*
 * the following six(6) is #define in stdio.h
 */
#ifndef WIN32IO_IS_STDIO
#undef errno
#undef environ
#undef stderr
#undef stdin
#undef stdout
#undef ferror
#undef feof
#undef fclose
#undef pipe
#undef pause
#undef sleep
#undef times
#undef ioctl
#undef unlink
#undef utime
#undef gettimeofday
#undef uname
#undef wait

#if defined(__MINGW32__)
#undef fileno
#endif

#define stderr				win32_stderr()
#define stdout				win32_stdout()
#define stdin                           win32_stdin()
#define feof(f)				win32_feof(f)
#define ferror(f)			win32_ferror(f)
#define errno 				(*win32_errno())
#define environ				(*win32_environ())

/*
 * redirect to our own version
 */
#undef fprintf
#define fprintf                 win32_fprintf
#define vfprintf                win32_vfprintf
#define printf                  win32_printf
#define vprintf                 win32_vprintf
#define fread(buf,size,count,f)	win32_fread(buf,size,count,f)
#define fwrite(buf,size,count,f)	win32_fwrite(buf,size,count,f)
#define fopen			win32_fopen
#undef fdopen
#define fdopen			win32_fdopen
#define freopen			win32_freopen
#define fclose(f)               win32_fclose(f)
#define fputs(s,f)		win32_fputs(s,f)
#define fputc(c,f)		win32_fputc(c,f)
#define ungetc(c,f)		win32_ungetc(c,f)
#undef getc
#define getc(f)			win32_getc(f)
#define fileno(f)		win32_fileno(f)
#define clearerr(f)		win32_clearerr(f)
#define fflush(f)		win32_fflush(f)
#define ftell(f)		win32_ftell(f)
#define fseek(f,o,w)		win32_fseek(f,o,w)
#define fgetpos(f,p)		win32_fgetpos(f,p)
#define fsetpos(f,p)		win32_fsetpos(f,p)
#define rewind(f)		win32_rewind(f)
#define tmpfile()		win32_tmpfile()
#define abort()			win32_abort()
#ifdef __MINGW32__
#  undef fstat
#endif
#define fstat(fd,bufptr)   	win32_fstat(fd,bufptr)
#ifdef __MINGW32__
#  undef stat
#endif
#define stat(pth,bufptr)   	win32_stat(pth,bufptr)
#define lstat(pth,bufptr)   	win32_lstat(pth,bufptr)
#define longpath(pth)   	win32_longpath(pth)
#define ansipath(pth)   	win32_ansipath(pth)
#define rename(old,new)		win32_rename(old,new)
#define setmode(fd,mode)	win32_setmode(fd,mode)
#define chsize(fd,sz)		win32_chsize(fd,sz)
#define lseek(fd,offset,orig)	win32_lseek(fd,offset,orig)
#define tell(fd)		win32_tell(fd)
#define dup(fd)			win32_dup(fd)
#define dup2(fd1,fd2)		win32_dup2(fd1,fd2)
#define open			win32_open
#define close(fd)		win32_close(fd)
#define eof(fd)			win32_eof(fd)
#define isatty(fd)		win32_isatty(fd)
#define read(fd,b,s)		win32_read(fd,b,s)
#define write(fd,b,s)		win32_write(fd,b,s)
#define _open_osfhandle		win32_open_osfhandle
#define _get_osfhandle		win32_get_osfhandle
#define spawnvp			win32_spawnvp
#define mkdir			win32_mkdir
#define rmdir			win32_rmdir
#define chdir			win32_chdir
#define flock(fd,o)		win32_flock(fd,o)
#define execv			win32_execv
#define execvp			win32_execvp
#define perror			win32_perror
#define setbuf			win32_setbuf
#define setvbuf			win32_setvbuf
#undef flushall
#define flushall		win32_flushall
#undef fcloseall
#define fcloseall		win32_fcloseall
#define fgets			win32_fgets
#define gets			win32_gets
#define fgetc			win32_fgetc
#undef putc
#define putc			win32_putc
#define puts			win32_puts
#undef getchar
#define getchar			win32_getchar
#undef putchar
#define putchar			win32_putchar
#define access(p,m)		win32_access(p,m)
#define chmod(p,m)		win32_chmod(p,m)

#if !defined(MYMALLOC) || !defined(PERL_CORE)
#undef malloc
#undef calloc
#undef realloc
#undef free
#define malloc			win32_malloc
#define calloc			win32_calloc
#define realloc			win32_realloc
#define free			win32_free
#endif

/* XXX Why are APIs like sleep(), times() etc. inside a block
 * XXX guarded by "#ifndef WIN32IO_IS_STDIO"?
 */

#define pipe(fd)		win32_pipe((fd), 512, O_BINARY)
#define pause			win32_pause
#define sleep			win32_sleep
#define times			win32_times
#define ioctl			win32_ioctl
#define link			win32_link
#define symlink			win32_symlink
#define readlink		win32_readlink
#define unlink			win32_unlink
#define utime			win32_utime
#define gettimeofday		win32_gettimeofday
#define uname			win32_uname
#define wait			win32_wait
#define waitpid			win32_waitpid
#define kill			win32_kill
#define killpg(pid, sig)	win32_kill(pid, -(sig))


#define opendir			win32_opendir
#define readdir			win32_readdir
#define telldir			win32_telldir
#define seekdir			win32_seekdir
#define rewinddir		win32_rewinddir
#define closedir		win32_closedir
#define os_id			win32_os_id
#define getpid			win32_getpid

#undef crypt
#define crypt(t,s)		win32_crypt(t,s)

#undef get_childenv
#undef free_childenv
#undef clearenv
#undef get_childdir
#undef free_childdir
#define get_childenv()		win32_get_childenv()
#define free_childenv(d)	win32_free_childenv(d)
#define clearenv()		win32_clearenv()
#define get_childdir()		win32_get_childdir()
#define free_childdir(d)	win32_free_childdir(d)

#undef getenv
#define getenv win32_getenv
#undef putenv
#define putenv win32_putenv

#endif /* WIN32IO_IS_STDIO */
#endif /* WIN32IOP_H */
