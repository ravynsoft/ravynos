#define PERL_constant_NOTFOUND	1
#define PERL_constant_NOTDEF	2
#define PERL_constant_ISIV	3
#define PERL_constant_ISNO	4
#define PERL_constant_ISNV	5
#define PERL_constant_ISPV	6
#define PERL_constant_ISPVN	7
#define PERL_constant_ISSV	8
#define PERL_constant_ISUNDEF	9
#define PERL_constant_ISUV	10
#define PERL_constant_ISYES	11

#ifndef NVTYPE
typedef double NV; /* 5.6 and later define NVTYPE, and typedef NV to it.  */
#endif
#ifndef aTHX_
#define aTHX_ /* 5.6 or later define this for threading support.  */
#endif
#ifndef pTHX_
#define pTHX_ /* 5.6 or later define this for threading support.  */
#endif

static int
constant_7 (pTHX_ const char *name, IV *iv_return) {
  /* When generated this function returned values for the list of names given
     here.  However, subsequent manual editing may have added or removed some.
     LOG_ERR LOG_FTP LOG_LPR LOG_NTP LOG_PID LOG_RAS */
  /* Offset 4 gives the best switch position.  */
  switch (name[4]) {
  case 'E':
    if (memEQ(name, "LOG_ERR", 7)) {
    /*                   ^        */
#ifdef LOG_ERR
      *iv_return = LOG_ERR;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'F':
    if (memEQ(name, "LOG_FTP", 7)) {
    /*                   ^        */
#ifdef LOG_FTP
      *iv_return = LOG_FTP;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'L':
    if (memEQ(name, "LOG_LPR", 7)) {
    /*                   ^        */
#ifdef LOG_LPR
      *iv_return = LOG_LPR;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'N':
    if (memEQ(name, "LOG_NTP", 7)) {
    /*                   ^        */
#ifdef LOG_NTP
      *iv_return = LOG_NTP;
      return PERL_constant_ISIV;
#else
      *iv_return = LOG_DAEMON;
      return PERL_constant_ISIV;
#endif
    }
    break;
  case 'P':
    if (memEQ(name, "LOG_PID", 7)) {
    /*                   ^        */
#ifdef LOG_PID
      *iv_return = LOG_PID;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'R':
    if (memEQ(name, "LOG_RAS", 7)) {
    /*                   ^        */
#ifdef LOG_RAS
      *iv_return = LOG_RAS;
      return PERL_constant_ISIV;
#else
      *iv_return = LOG_AUTH;
      return PERL_constant_ISIV;
#endif
    }
    break;
  }
  return PERL_constant_NOTFOUND;
}

static int
constant_8 (pTHX_ const char *name, IV *iv_return) {
  /* When generated this function returned values for the list of names given
     here.  However, subsequent manual editing may have added or removed some.
     LOG_AUTH LOG_CONS LOG_CRIT LOG_CRON LOG_INFO LOG_KERN LOG_LFMT LOG_MAIL
     LOG_NEWS LOG_USER LOG_UUCP */
  /* Offset 6 gives the best switch position.  */
  switch (name[6]) {
  case 'C':
    if (memEQ(name, "LOG_UUCP", 8)) {
    /*                     ^       */
#ifdef LOG_UUCP
      *iv_return = LOG_UUCP;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'E':
    if (memEQ(name, "LOG_USER", 8)) {
    /*                     ^       */
#ifdef LOG_USER
      *iv_return = LOG_USER;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'F':
    if (memEQ(name, "LOG_INFO", 8)) {
    /*                     ^       */
#ifdef LOG_INFO
      *iv_return = LOG_INFO;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'I':
    if (memEQ(name, "LOG_CRIT", 8)) {
    /*                     ^       */
#ifdef LOG_CRIT
      *iv_return = LOG_CRIT;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    if (memEQ(name, "LOG_MAIL", 8)) {
    /*                     ^       */
#ifdef LOG_MAIL
      *iv_return = LOG_MAIL;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'M':
    if (memEQ(name, "LOG_LFMT", 8)) {
    /*                     ^       */
#ifdef LOG_LFMT
      *iv_return = LOG_LFMT;
      return PERL_constant_ISIV;
#else
      *iv_return = LOG_USER;
      return PERL_constant_ISIV;
#endif
    }
    break;
  case 'N':
    if (memEQ(name, "LOG_CONS", 8)) {
    /*                     ^       */
#ifdef LOG_CONS
      *iv_return = LOG_CONS;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'O':
    if (memEQ(name, "LOG_CRON", 8)) {
    /*                     ^       */
#ifdef LOG_CRON
      *iv_return = LOG_CRON;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'R':
    if (memEQ(name, "LOG_KERN", 8)) {
    /*                     ^       */
#ifdef LOG_KERN
      *iv_return = LOG_KERN;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'T':
    if (memEQ(name, "LOG_AUTH", 8)) {
    /*                     ^       */
#ifdef LOG_AUTH
      *iv_return = LOG_AUTH;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'W':
    if (memEQ(name, "LOG_NEWS", 8)) {
    /*                     ^       */
#ifdef LOG_NEWS
      *iv_return = LOG_NEWS;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  }
  return PERL_constant_NOTFOUND;
}

static int
constant_9 (pTHX_ const char *name, IV *iv_return, const char **pv_return) {
  /* When generated this function returned values for the list of names given
     here.  However, subsequent manual editing may have added or removed some.
     LOG_ALERT LOG_AUDIT LOG_DEBUG LOG_EMERG _PATH_LOG */
  /* Offset 5 gives the best switch position.  */
  switch (name[5]) {
  case 'E':
    if (memEQ(name, "LOG_DEBUG", 9)) {
    /*                    ^         */
#ifdef LOG_DEBUG
      *iv_return = LOG_DEBUG;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'L':
    if (memEQ(name, "LOG_ALERT", 9)) {
    /*                    ^         */
#ifdef LOG_ALERT
      *iv_return = LOG_ALERT;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'M':
    if (memEQ(name, "LOG_EMERG", 9)) {
    /*                    ^         */
#ifdef LOG_EMERG
      *iv_return = LOG_EMERG;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'U':
    if (memEQ(name, "LOG_AUDIT", 9)) {
    /*                    ^         */
#ifdef LOG_AUDIT
      *iv_return = LOG_AUDIT;
      return PERL_constant_ISIV;
#else
      *iv_return = LOG_AUTH;
      return PERL_constant_ISIV;
#endif
    }
    break;
  case '_':
    if (memEQ(name, "_PATH_LOG", 9)) {
    /*                    ^         */
#ifdef _PATH_LOG
      *pv_return = _PATH_LOG;
      return PERL_constant_ISPV;
#else
      *pv_return = "/var/run/syslog";
      return PERL_constant_ISPV;
#endif
    }
    break;
  }
  return PERL_constant_NOTFOUND;
}

static int
constant_10 (pTHX_ const char *name, IV *iv_return) {
  /* When generated this function returned values for the list of names given
     here.  However, subsequent manual editing may have added or removed some.
     LOG_DAEMON LOG_LOCAL0 LOG_LOCAL1 LOG_LOCAL2 LOG_LOCAL3 LOG_LOCAL4
     LOG_LOCAL5 LOG_LOCAL6 LOG_LOCAL7 LOG_NDELAY LOG_NOTICE LOG_NOWAIT
     LOG_ODELAY LOG_PERROR LOG_SYSLOG */
  /* Offset 9 gives the best switch position.  */
  switch (name[9]) {
  case '0':
    if (memEQ(name, "LOG_LOCAL", 9)) {
    /*                        0     */
#ifdef LOG_LOCAL0
      *iv_return = LOG_LOCAL0;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case '1':
    if (memEQ(name, "LOG_LOCAL", 9)) {
    /*                        1     */
#ifdef LOG_LOCAL1
      *iv_return = LOG_LOCAL1;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case '2':
    if (memEQ(name, "LOG_LOCAL", 9)) {
    /*                        2     */
#ifdef LOG_LOCAL2
      *iv_return = LOG_LOCAL2;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case '3':
    if (memEQ(name, "LOG_LOCAL", 9)) {
    /*                        3     */
#ifdef LOG_LOCAL3
      *iv_return = LOG_LOCAL3;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case '4':
    if (memEQ(name, "LOG_LOCAL", 9)) {
    /*                        4     */
#ifdef LOG_LOCAL4
      *iv_return = LOG_LOCAL4;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case '5':
    if (memEQ(name, "LOG_LOCAL", 9)) {
    /*                        5     */
#ifdef LOG_LOCAL5
      *iv_return = LOG_LOCAL5;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case '6':
    if (memEQ(name, "LOG_LOCAL", 9)) {
    /*                        6     */
#ifdef LOG_LOCAL6
      *iv_return = LOG_LOCAL6;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case '7':
    if (memEQ(name, "LOG_LOCAL", 9)) {
    /*                        7     */
#ifdef LOG_LOCAL7
      *iv_return = LOG_LOCAL7;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'E':
    if (memEQ(name, "LOG_NOTIC", 9)) {
    /*                        E     */
#ifdef LOG_NOTICE
      *iv_return = LOG_NOTICE;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'G':
    if (memEQ(name, "LOG_SYSLO", 9)) {
    /*                        G     */
#ifdef LOG_SYSLOG
      *iv_return = LOG_SYSLOG;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'N':
    if (memEQ(name, "LOG_DAEMO", 9)) {
    /*                        N     */
#ifdef LOG_DAEMON
      *iv_return = LOG_DAEMON;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'R':
    if (memEQ(name, "LOG_PERRO", 9)) {
    /*                        R     */
#ifdef LOG_PERROR
      *iv_return = LOG_PERROR;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'T':
    if (memEQ(name, "LOG_NOWAI", 9)) {
    /*                        T     */
#ifdef LOG_NOWAIT
      *iv_return = LOG_NOWAIT;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'Y':
    if (memEQ(name, "LOG_NDELA", 9)) {
    /*                        Y     */
#ifdef LOG_NDELAY
      *iv_return = LOG_NDELAY;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    if (memEQ(name, "LOG_ODELA", 9)) {
    /*                        Y     */
#ifdef LOG_ODELAY
      *iv_return = LOG_ODELAY;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  }
  return PERL_constant_NOTFOUND;
}

static int
constant_11 (pTHX_ const char *name, IV *iv_return) {
  /* When generated this function returned values for the list of names given
     here.  However, subsequent manual editing may have added or removed some.
     LOG_CONSOLE LOG_FACMASK LOG_INSTALL LOG_LAUNCHD LOG_NETINFO LOG_PRIMASK
     LOG_WARNING */
  /* Offset 6 gives the best switch position.  */
  switch (name[6]) {
  case 'C':
    if (memEQ(name, "LOG_FACMASK", 11)) {
    /*                     ^           */
#ifdef LOG_FACMASK
      *iv_return = LOG_FACMASK;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'I':
    if (memEQ(name, "LOG_PRIMASK", 11)) {
    /*                     ^           */
#ifdef LOG_PRIMASK
      *iv_return = LOG_PRIMASK;
      return PERL_constant_ISIV;
#else
      *iv_return = 7;
      return PERL_constant_ISIV;
#endif
    }
    break;
  case 'N':
    if (memEQ(name, "LOG_CONSOLE", 11)) {
    /*                     ^           */
#ifdef LOG_CONSOLE
      *iv_return = LOG_CONSOLE;
      return PERL_constant_ISIV;
#else
      *iv_return = LOG_USER;
      return PERL_constant_ISIV;
#endif
    }
    break;
  case 'R':
    if (memEQ(name, "LOG_WARNING", 11)) {
    /*                     ^           */
#ifdef LOG_WARNING
      *iv_return = LOG_WARNING;
      return PERL_constant_ISIV;
#else
      return PERL_constant_NOTDEF;
#endif
    }
    break;
  case 'S':
    if (memEQ(name, "LOG_INSTALL", 11)) {
    /*                     ^           */
#ifdef LOG_INSTALL
      *iv_return = LOG_INSTALL;
      return PERL_constant_ISIV;
#else
      *iv_return = LOG_USER;
      return PERL_constant_ISIV;
#endif
    }
    break;
  case 'T':
    if (memEQ(name, "LOG_NETINFO", 11)) {
    /*                     ^           */
#ifdef LOG_NETINFO
      *iv_return = LOG_NETINFO;
      return PERL_constant_ISIV;
#else
      *iv_return = LOG_DAEMON;
      return PERL_constant_ISIV;
#endif
    }
    break;
  case 'U':
    if (memEQ(name, "LOG_LAUNCHD", 11)) {
    /*                     ^           */
#ifdef LOG_LAUNCHD
      *iv_return = LOG_LAUNCHD;
      return PERL_constant_ISIV;
#else
      *iv_return = LOG_DAEMON;
      return PERL_constant_ISIV;
#endif
    }
    break;
  }
  return PERL_constant_NOTFOUND;
}

static int
constant (pTHX_ const char *name, STRLEN len, IV *iv_return, const char **pv_return) {
  /* Initially switch on the length of the name.  */
  /* When generated this function returned values for the list of names given
     in this section of perl code.  Rather than manually editing these functions
     to add or remove constants, which would result in this comment and section
     of code becoming inaccurate, we recommend that you edit this section of
     code, and use it to regenerate a new set of constant functions which you
     then use to replace the originals.

     Regenerate these constant functions by feeding this entire source file to
     perl -x

#!perl -w
use ExtUtils::Constant qw (constant_types C_constant XS_constant);

my $types = {map {($_, 1)} qw(IV PV)};
my @names = (qw(LOG_ALERT LOG_AUTH LOG_AUTHPRIV LOG_CONS LOG_CRIT LOG_CRON
	       LOG_DAEMON LOG_DEBUG LOG_EMERG LOG_ERR LOG_FACMASK LOG_FTP
	       LOG_INFO LOG_KERN LOG_LOCAL0 LOG_LOCAL1 LOG_LOCAL2 LOG_LOCAL3
	       LOG_LOCAL4 LOG_LOCAL5 LOG_LOCAL6 LOG_LOCAL7 LOG_LPR LOG_MAIL
	       LOG_NDELAY LOG_NEWS LOG_NOTICE LOG_NOWAIT LOG_ODELAY LOG_PERROR
	       LOG_PID LOG_SYSLOG LOG_USER LOG_UUCP LOG_WARNING),
            {name=>"LOG_AUDIT", type=>"IV", default=>["IV", "LOG_AUTH"]},
            {name=>"LOG_CONSOLE", type=>"IV", default=>["IV", "LOG_USER"]},
            {name=>"LOG_INSTALL", type=>"IV", default=>["IV", "LOG_USER"]},
            {name=>"LOG_LAUNCHD", type=>"IV", default=>["IV", "LOG_DAEMON"]},
            {name=>"LOG_LFMT", type=>"IV", default=>["IV", "LOG_USER"]},
            {name=>"LOG_NETINFO", type=>"IV", default=>["IV", "LOG_DAEMON"]},
            {name=>"LOG_NFACILITIES", type=>"IV", default=>["IV", "30"]},
            {name=>"LOG_NTP", type=>"IV", default=>["IV", "LOG_DAEMON"]},
            {name=>"LOG_PRIMASK", type=>"IV", default=>["IV", "7"]},
            {name=>"LOG_RAS", type=>"IV", default=>["IV", "LOG_AUTH"]},
            {name=>"LOG_REMOTEAUTH", type=>"IV", default=>["IV", "LOG_AUTH"]},
            {name=>"LOG_SECURITY", type=>"IV", default=>["IV", "LOG_AUTH"]},
            {name=>"_PATH_LOG", type=>"PV", default=>["PV", "\"/var/run/syslog\""]});

print constant_types(); # macro defs
foreach (C_constant ("Sys::Syslog", 'constant', 'IV', $types, undef, 3, @names) ) {
    print $_, "\n"; # C constant subs
}
print "#### XS Section:\n";
print XS_constant ("Sys::Syslog", $types);
__END__
   */

  switch (len) {
  case 7:
    return constant_7 (aTHX_ name, iv_return);
    break;
  case 8:
    return constant_8 (aTHX_ name, iv_return);
    break;
  case 9:
    return constant_9 (aTHX_ name, iv_return, pv_return);
    break;
  case 10:
    return constant_10 (aTHX_ name, iv_return);
    break;
  case 11:
    return constant_11 (aTHX_ name, iv_return);
    break;
  case 12:
    /* Names all of length 12.  */
    /* LOG_AUTHPRIV LOG_SECURITY */
    /* Offset 8 gives the best switch position.  */
    switch (name[8]) {
    case 'P':
      if (memEQ(name, "LOG_AUTHPRIV", 12)) {
      /*                       ^          */
#ifdef LOG_AUTHPRIV
        *iv_return = LOG_AUTHPRIV;
        return PERL_constant_ISIV;
#else
        return PERL_constant_NOTDEF;
#endif
      }
      break;
    case 'R':
      if (memEQ(name, "LOG_SECURITY", 12)) {
      /*                       ^          */
#ifdef LOG_SECURITY
        *iv_return = LOG_SECURITY;
        return PERL_constant_ISIV;
#else
        *iv_return = LOG_AUTH;
        return PERL_constant_ISIV;
#endif
      }
      break;
    }
    break;
  case 14:
    if (memEQ(name, "LOG_REMOTEAUTH", 14)) {
#ifdef LOG_REMOTEAUTH
      *iv_return = LOG_REMOTEAUTH;
      return PERL_constant_ISIV;
#else
      *iv_return = LOG_AUTH;
      return PERL_constant_ISIV;
#endif
    }
    break;
  case 15:
    if (memEQ(name, "LOG_NFACILITIES", 15)) {
#ifdef LOG_NFACILITIES
      *iv_return = LOG_NFACILITIES;
      return PERL_constant_ISIV;
#else
      *iv_return = 30;
      return PERL_constant_ISIV;
#endif
    }
    break;
  }
  return PERL_constant_NOTFOUND;
}

