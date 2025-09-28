#define PERL_NO_GET_CONTEXT

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#if defined(PERL_IMPLICIT_SYS)
#  undef open
#  define open PerlLIO_open3
#endif

#ifdef I_DBM
#  include <dbm.h>
#else
#  ifdef I_RPCSVC_DBM
#    include <rpcsvc/dbm.h>
#  endif
#endif

#ifndef HAS_DBMINIT_PROTO
int	dbminit(char* filename);
int	dbmclose(void);
datum	fetch(datum key);
int	store(datum key, datum dat);
int	delete(datum key); 
datum	firstkey(void);
datum	nextkey(datum key);
#endif

#ifdef DBM_BUG_DUPLICATE_FREE 
/*
 * DBM on at least HPUX call dbmclose() from dbminit(),
 * resulting in duplicate free() because dbmclose() does *not*
 * check if it has already been called for this DBM.
 * If some malloc/free calls have been done between dbmclose() and
 * the next dbminit(), the memory might be used for something else when
 * it is freed.
 * Probably will work on HP/UX.
 * Set DBM_BUG_DUPLICATE_FREE in the extension hint file.
 */
/* Close the previous dbm, and fail to open a new dbm */
#define dbmclose()	((void) dbminit("/non/exist/ent"))
#endif

#include <fcntl.h>

#define fetch_key 0
#define store_key 1
#define fetch_value 2
#define store_value 3

typedef struct {
	void * 	dbp ;
	SV *    filter[4];
	int     filtering ;
	} ODBM_File_type;

typedef ODBM_File_type * ODBM_File ;
typedef datum datum_key ;
typedef datum datum_key_copy ;
typedef datum datum_value ;

#define odbm_FETCH(db,key)			fetch(key)
#define odbm_STORE(db,key,value,flags)		store(key,value)
#define odbm_DELETE(db,key)			delete(key)
#define odbm_FIRSTKEY(db)			firstkey()
#define odbm_NEXTKEY(db,key)			nextkey(key)

#define MY_CXT_KEY "ODBM_File::_guts" XS_VERSION

typedef struct {
    int		x_dbmrefcnt;
} my_cxt_t;

START_MY_CXT

#define dbmrefcnt	(MY_CXT.x_dbmrefcnt)

#ifndef DBM_REPLACE
#define DBM_REPLACE 0
#endif

MODULE = ODBM_File	PACKAGE = ODBM_File	PREFIX = odbm_

BOOT:
{
    MY_CXT_INIT;
}

ODBM_File
odbm_TIEHASH(dbtype, filename, flags, mode)
	char *		dbtype
	char *		filename
	int		flags
	int		mode
	CODE:
	{
	    char *tmpbuf;
	    void * dbp ;
	    dMY_CXT;

	    if (dbmrefcnt++)
		croak("Old dbm can only open one database");
	    Newx(tmpbuf, strlen(filename) + 5, char);
	    SAVEFREEPV(tmpbuf);
	    sprintf(tmpbuf,"%s.dir",filename);
            if ((flags & O_CREAT)) {
               const int oflags = O_CREAT | O_TRUNC | O_WRONLY | O_EXCL;
               int created = 0;
               int fd;
               if (mode < 0)
                   goto creat_done;
               if ((fd = open(tmpbuf,oflags,mode)) < 0 && errno != EEXIST)
                   goto creat_done;
               if (close(fd) < 0)
                   goto creat_done;
               sprintf(tmpbuf,"%s.pag",filename);
               if ((fd = open(tmpbuf,oflags,mode)) < 0 && errno != EEXIST)
                   goto creat_done;
               if (close(fd) < 0)
                   goto creat_done;
               created = 1;
            creat_done:
               if (!created)
                   croak("ODBM_File: Can't create %s", filename);
            }
            else {
               int opened = 0;
               int fd;
               if ((fd = open(tmpbuf,O_RDONLY,mode)) < 0)
                   goto rdonly_done;
               if (close(fd) < 0)
                   goto rdonly_done;
               opened = 1;
            rdonly_done:
               if (!opened)
                   croak("ODBM_FILE: Can't open %s", filename);
	    }
	    dbp = (void*)(dbminit(filename) >= 0 ? &dbmrefcnt : 0);
	    RETVAL = (ODBM_File)safecalloc(1, sizeof(ODBM_File_type));
	    RETVAL->dbp = dbp ;
	}
	OUTPUT:
	  RETVAL

void
DESTROY(db)
	ODBM_File	db
	PREINIT:
	dMY_CXT;
	int i = store_value;
	CODE:
	dbmrefcnt--;
	dbmclose();
	do {
	    if (db->filter[i])
		SvREFCNT_dec(db->filter[i]);
	} while (i-- > 0);
	safefree(db);

datum_value
odbm_FETCH(db, key)
	ODBM_File	db
	datum_key_copy	key

int
odbm_STORE(db, key, value, flags = DBM_REPLACE)
	ODBM_File	db
	datum_key	key
	datum_value	value
	int		flags
    CLEANUP:
	if (RETVAL) {
	    if (RETVAL < 0 && errno == EPERM)
		croak("No write permission to odbm file");
	    croak("odbm store returned %d, errno %d, key \"%s\"",
			RETVAL,errno,key.dptr);
	}
        PERL_UNUSED_VAR(flags);

int
odbm_DELETE(db, key)
	ODBM_File	db
	datum_key	key
	CODE:
            /* don't warn about 'delete' being a C++ keyword */
            GCC_DIAG_IGNORE_STMT(-Wc++-compat);
	    RETVAL = odbm_DELETE(db, key);
            GCC_DIAG_RESTORE_STMT;
	OUTPUT:
	  RETVAL


datum_key
odbm_FIRSTKEY(db)
	ODBM_File	db

datum_key
odbm_NEXTKEY(db, key)
	ODBM_File	db
	datum_key	key


#define setFilter(type)					\
	{						\
	    if (db->type)				\
	        RETVAL = sv_mortalcopy(db->type) ; 	\
	    ST(0) = RETVAL ;				\
	    if (db->type && (code == &PL_sv_undef)) {	\
                SvREFCNT_dec(db->type) ;		\
	        db->type = Nullsv ;			\
	    }						\
	    else if (code) {				\
	        if (db->type)				\
	            sv_setsv(db->type, code) ;		\
	        else					\
	            db->type = newSVsv(code) ;		\
	    }	    					\
	}



SV *
filter_fetch_key(db, code)
	ODBM_File	db
	SV *		code
	SV *		RETVAL = &PL_sv_undef ;
	ALIAS:
	ODBM_File::filter_fetch_key = fetch_key
	ODBM_File::filter_store_key = store_key
	ODBM_File::filter_fetch_value = fetch_value
	ODBM_File::filter_store_value = store_value
	CODE:
	    DBM_setFilter(db->filter[ix], code);
