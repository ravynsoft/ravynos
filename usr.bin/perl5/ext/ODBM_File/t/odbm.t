#!./perl

our $DBM_Class = 'ODBM_File';

require '../../t/lib/dbmt_common.pl';

if ($^O eq 'hpux') {
    print <<EOM;
#
# If you experience failures with the odbm test in HP-UX,
# this is a well-known bug that's unfortunately very hard to fix.
# The suggested course of action is to avoid using the ODBM_File,
# but to use instead the NDBM_File extension.
#
EOM
}
