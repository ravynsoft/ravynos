/* Mini EMBED (pair.c) */
#ifndef PERL_SDBM_FILE_PAIR_H_
#define PERL_SDBM_FILE_PAIR_H_

#define chkpage sdbm__chkpage
#define delpair sdbm__delpair
#define duppair sdbm__duppair
#define exipair sdbm__exipair
#define fitpair sdbm__fitpair
#define getnkey sdbm__getnkey
#define getpair sdbm__getpair
#define putpair sdbm__putpair
#define splpage sdbm__splpage

extern int fitpair(char *, int);
extern void  putpair(char *, datum, datum);
extern datum	getpair(char *, datum);
extern int  exipair(char *, datum);
extern int  delpair(char *, datum);
extern int  chkpage(char *);
extern datum getnkey(char *, int);
extern void splpage(char *, char *, long);
#ifdef SEEDUPS
extern int duppair(char *, datum);
#endif

#endif /* PERL_SDBM_FILE_PAIR_H_ */
