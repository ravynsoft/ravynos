# uses GDBM dbm compatibility feature - at least on SuSE 8.0
$self->{LIBS} = ['-lgdbm'];

# Debian/Ubuntu have libgdbm_compat.so but not this file,
# so linking may fail
foreach (split / /, $Config{libpth}) {
    $self->{LIBS}->[0] .= ' -lgdbm_compat' if -e $_.'/libgdbm_compat.so';
}
