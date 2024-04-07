#define PERL_NO_GET_CONTEXT

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#undef NDBM_HEADER_USES_PROTOTYPES
#if defined(I_GDBM_NDBM)
#  ifdef GDBM_NDBM_H_USES_PROTOTYPES
#    define NDBM_HEADER_USES_PROTOTYPES
START_EXTERN_C
#  endif
#  include <gdbm-ndbm.h> /* Debian compatibility version */
#elif defined(I_GDBMNDBM)
#  ifdef GDBMNDBM_H_USES_PROTOTYPES
#    define NDBM_HEADER_USES_PROTOTYPES
START_EXTERN_C
#  endif
#  include <gdbm/ndbm.h> /* RedHat compatibility version */
#elif defined(I_NDBM)
#  ifdef NDBM_H_USES_PROTOTYPES
#    define NDBM_HEADER_USES_PROTOTYPES
START_EXTERN_C
#  endif
#  include <ndbm.h>
#endif
#ifdef NDBM_HEADER_USES_PROTOTYPES
END_EXTERN_C
#endif

#define fetch_key 0
#define store_key 1
#define fetch_value 2
#define store_value 3

typedef struct {
	DBM * 	dbp ;
	SV *    filter[4];
	int     filtering ;
	} NDBM_File_type;

typedef NDBM_File_type * NDBM_File ;
typedef datum datum_key ;
typedef datum datum_value ;


#if defined(__cplusplus) && !defined(NDBM_HEADER_USES_PROTOTYPES)
/* gdbm's header file used for compatibility with gdbm */
/* isn't compatible to C++ syntax, so we need these */
/* declarations to make everyone happy. */
EXTERN_C DBM *dbm_open(const char *, int, mode_t);
EXTERN_C void dbm_close(DBM *);
EXTERN_C datum dbm_fetch(DBM *, datum);
EXTERN_C int dbm_store(DBM *, datum, datum, int);
EXTERN_C int dbm_delete(DBM *, datum);
EXTERN_C datum dbm_firstkey(DBM *);
EXTERN_C datum dbm_nextkey(DBM *);
#endif

MODULE = NDBM_File	PACKAGE = NDBM_File	PREFIX = ndbm_

NDBM_File
ndbm_TIEHASH(dbtype, filename, flags, mode)
	char *		dbtype
	char *		filename
	int		flags
	int		mode
	CODE:
	{
	    DBM * 	dbp ;

	    RETVAL = NULL ;
	    if ((dbp =  dbm_open(filename, flags, mode))) {
	        RETVAL = (NDBM_File)safecalloc(1, sizeof(NDBM_File_type));
		RETVAL->dbp = dbp ;
	    }
	    
	}
	OUTPUT:
	  RETVAL

void
ndbm_DESTROY(db)
	NDBM_File	db
	PREINIT:
	int i = store_value;
	CODE:
	dbm_close(db->dbp);
	do {
	    if (db->filter[i])
		SvREFCNT_dec(db->filter[i]);
	} while (i-- > 0);
	safefree(db);

#define ndbm_FETCH(db,key)			dbm_fetch(db->dbp,key)
datum_value
ndbm_FETCH(db, key)
	NDBM_File	db
	datum_key	key

#define ndbm_EXISTS(db,key)			dbm_fetch(db->dbp,key).dptr
bool
ndbm_EXISTS(db, key)
	NDBM_File	db
	datum_key	key

#define ndbm_STORE(db,key,value,flags)		dbm_store(db->dbp,key,value,flags)
int
ndbm_STORE(db, key, value, flags = DBM_REPLACE)
	NDBM_File	db
	datum_key	key
	datum_value	value
	int		flags
    CLEANUP:
	if (RETVAL) {
	    if (RETVAL < 0 && errno == EPERM)
		croak("No write permission to ndbm file");
	    croak("ndbm store returned %d, errno %d, key \"%s\"",
                  RETVAL, errno, (const char *)key.dptr);
	    dbm_clearerr(db->dbp);
	}

#define ndbm_DELETE(db,key)			dbm_delete(db->dbp,key)
int
ndbm_DELETE(db, key)
	NDBM_File	db
	datum_key	key

#define ndbm_FIRSTKEY(db)			dbm_firstkey(db->dbp)
datum_key
ndbm_FIRSTKEY(db)
	NDBM_File	db

#define ndbm_NEXTKEY(db,key)			dbm_nextkey(db->dbp)
datum_key
ndbm_NEXTKEY(db, key)
	NDBM_File	db
	datum_key	key = NO_INIT
    CLEANUP:
	PERL_UNUSED_VAR(key);

#define ndbm_error(db)				dbm_error(db->dbp)
int
ndbm_error(db)
	NDBM_File	db
    CLEANUP:
	PERL_UNUSED_VAR(db);

#define ndbm_clearerr(db)			dbm_clearerr(db->dbp)
void
ndbm_clearerr(db)
	NDBM_File	db
    CLEANUP:
	PERL_UNUSED_VAR(db);


SV *
filter_fetch_key(db, code)
	NDBM_File	db
	SV *		code
	SV *		RETVAL = &PL_sv_undef ;
	ALIAS:
	NDBM_File::filter_fetch_key = fetch_key
	NDBM_File::filter_store_key = store_key
	NDBM_File::filter_fetch_value = fetch_value
	NDBM_File::filter_store_value = store_value
	CODE:
	    DBM_setFilter(db->filter[ix], code);
