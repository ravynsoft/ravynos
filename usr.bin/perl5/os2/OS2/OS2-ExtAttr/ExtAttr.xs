#ifdef __cplusplus
extern "C" {
#endif
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#ifdef __cplusplus
}
#endif

#include "myea.h"

SV *
my_eadvalue(pTHX_ _ead ead, int index)
{
    SV *sv;
    int size = _ead_value_size(ead, index);
    const char *p;

    if (size == -1) {
	Perl_die(aTHX_ "Error getting size of EA: %s", strerror(errno));
    }
    p = _ead_get_value(ead, index);
    return  newSVpv(p, size);
}

#define my_eadreplace(ead, index, sv, flag)	\
	_ead_replace((ead), (index), flag, SvPVX(sv), SvCUR(sv))

#define my_eadadd(ead, name, sv, flag)	\
	_ead_add((ead), (name), flag, SvPVX(sv), SvCUR(sv))


MODULE = OS2::ExtAttr		PACKAGE = OS2::ExtAttr	PREFIX = my_ead

SV *
my_eadvalue(ead, index)
	_ead	ead
	int	index
    CODE:
	RETVAL = my_eadvalue(aTHX_ ead, index);
    OUTPUT:
	RETVAL

int
my_eadreplace(ead, index, sv, flag = 0)
	_ead	ead
	int	index
	SV *	sv
	int	flag

int
my_eadadd(ead, name, sv, flag = 0)
	_ead	ead
	char *	name
	SV *	sv
	int	flag

MODULE = OS2::ExtAttr		PACKAGE = OS2::ExtAttr	PREFIX = _ea


void
_ea_free(ptr)
	struct _ea *	ptr

int
_ea_get(dst, path, handle, name)
	struct _ea *	dst
	char *	path
	int	handle
	char *	name

int
_ea_put(src, path, handle, name)
	struct _ea *	src
	char *	path
	int	handle
	char *	name

int
_ea_remove(path, handle, name)
	char *	path
	int	handle
	char *	name

MODULE = OS2::ExtAttr		PACKAGE = OS2::ExtAttr	PREFIX = _ead

int
_ead_add(ead, name, flags, value, size)
	_ead	ead
	char *	name
	int	flags
	void *	value
	int	size

void
_ead_clear(ead)
	_ead	ead

int
_ead_copy(dst_ead, src_ead, src_index)
	_ead	dst_ead
	_ead	src_ead
	int	src_index

int
_ead_count(ead)
	_ead	ead

_ead
_ead_create()

int
_ead_delete(ead, index)
	_ead	ead
	int	index

void
_ead_destroy(ead)
	_ead	ead

int
_ead_fea2list_size(ead)
	_ead	ead

void *
_ead_fea2list_to_fealist(src)
	void *	src

void *
_ead_fealist_to_fea2list(src)
	void *	src

int
_ead_find(ead, name)
	_ead	ead
	char *	name

const void *
_ead_get_fea2list(ead)
	_ead	ead

int
_ead_get_flags(ead, index)
	_ead	ead
	int	index

const char *
_ead_get_name(ead, index)
	_ead	ead
	int	index

const void *
_ead_get_value(ead, index)
	_ead	ead
	int	index

int
_ead_name_len(ead, index)
	_ead	ead
	int	index

int
_ead_read(ead, path, handle, flags)
	_ead	ead
	char *	path
	int	handle
	int	flags

int
_ead_replace(ead, index, flags, value, size)
	_ead	ead
	int	index
	int	flags
	void *	value
	int	size

void
_ead_sort(ead)
	_ead	ead

int
_ead_use_fea2list(ead, src)
	_ead	ead
	void *	src

int
_ead_value_size(ead, index)
	_ead	ead
	int	index

int
_ead_write(ead, path, handle, flags)
	_ead	ead
	char *	path
	int	handle
	int	flags
