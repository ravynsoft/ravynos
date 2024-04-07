use ExtUtils::MakeMaker;
use ExtUtils::Constant 0.11 'WriteConstants';
WriteMakefile(
    NAME => 'GDBM_File',
    LIBS => ["-lgdbm", "-ldbm"],
    XSPROTOARG => '-noprototypes', 		# XXX remove later?
    VERSION_FROM => 'GDBM_File.pm',
    realclean => {FILES=> 'const-c.inc const-xs.inc'},
    XS_VERSION => eval MM->parse_version('GDBM_File.pm'), #silence warnings if we are a dev release
);

my @names = qw(GDBM_CACHESIZE GDBM_CENTFREE GDBM_COALESCEBLKS
	       GDBM_FAST GDBM_FASTMODE GDBM_INSERT GDBM_NEWDB GDBM_NOLOCK
	       GDBM_OPENMASK GDBM_READER GDBM_REPLACE GDBM_SYNC GDBM_SYNCMODE
	       GDBM_WRCREAT GDBM_WRITER GDBM_NOMMAP GDBM_CLOEXEC GDBM_BSEXACT
               GDBM_XVERIFY GDBM_PREREAD GDBM_NUMSYNC);

push @names, {
    name  => $_,
    type  => "IV",
    macro => [ "#if  GDBM_VERSION_MAJOR > 1 || GDBM_VERSION_MINOR >= 21\n",
               "#endif\n" ],
    value => "$_",
} foreach qw(GDBM_SNAPSHOT_OK GDBM_SNAPSHOT_BAD GDBM_SNAPSHOT_ERR
             GDBM_SNAPSHOT_SAME GDBM_SNAPSHOT_SUSPICIOUS);

# Basic error codes - these are supported by all versions of gdbm
push @names, {
    name  => $_,
    value => "$_",
    type  => "IV",
    macro => 1
} foreach qw(
    GDBM_NO_ERROR
    GDBM_MALLOC_ERROR
    GDBM_BLOCK_SIZE_ERROR
    GDBM_FILE_OPEN_ERROR
    GDBM_FILE_WRITE_ERROR
    GDBM_FILE_SEEK_ERROR
    GDBM_FILE_READ_ERROR
    GDBM_BAD_MAGIC_NUMBER
    GDBM_EMPTY_DATABASE
    GDBM_CANT_BE_READER
    GDBM_CANT_BE_WRITER
    GDBM_READER_CANT_DELETE
    GDBM_READER_CANT_STORE
    GDBM_READER_CANT_REORGANIZE
    GDBM_UNKNOWN_UPDATE
    GDBM_ITEM_NOT_FOUND
    GDBM_REORGANIZE_FAILED
    GDBM_CANNOT_REPLACE
    GDBM_ILLEGAL_DATA
    GDBM_OPT_ALREADY_SET
    GDBM_OPT_ILLEGAL);

# Error codes added in various versions of gdbm
push @names, {
    name  => $_->[0],
    value => "$_->[0]",
    type  => "IV",
    macro => [ "#if GDBM_VERSION_MAJOR > 1 || $_->[1]\n",
               "#endif\n" ],
} foreach map {
    my @vref = @{$_}[1..$#{$_}];
    my $cond;
    if ($_->[2]) {
        $cond = "GDBM_VERSION_MINOR > $_->[1] || ( GDBM_VERSION_MINOR == $_->[1] && GDBM_VERSION_PATCH >= $_->[2] )";
    } else {
        $cond = "GDBM_VERSION_MINOR >= $_->[1]";
    }
    map { [$_, $cond ] } @{$_->[0]}
} ( # [ [ ERROR_CODE_NAMES ], MAJ [, MIN [, PAT]] ]
    # where MAJ,MIN,PAT are major, minor and patchlevel numbers of the gdbm
    # version which introduced ERROR_CODE_NAMES.
    [[qw(GDBM_BYTE_SWAPPED GDBM_BAD_FILE_OFFSET GDBM_BAD_OPEN_FLAGS)], 9],
    [[qw(GDBM_FILE_STAT_ERROR GDBM_FILE_EOF)], 10],
    [[qw(GDBM_NO_DBNAME GDBM_ERR_FILE_OWNER GDBM_ERR_FILE_MODE)], 11],
    [[qw(GDBM_UNKNOWN_ERROR GDBM_NEED_RECOVERY GDBM_BACKUP_FAILED
         GDBM_DIR_OVERFLOW)], 13],
    [[qw(GDBM_BAD_BUCKET GDBM_BAD_HEADER GDBM_BAD_AVAIL GDBM_BAD_HASH_TABLE
         GDBM_BAD_DIR_ENTRY)], 15],
    [[qw(GDBM_FILE_CLOSE_ERROR GDBM_FILE_SYNC_ERROR)], 17],
    [[qw(GDBM_FILE_TRUNCATE_ERROR)], 18, 1],
    [[qw(GDBM_BUCKET_CACHE_CORRUPTED GDBM_BAD_HASH_ENTRY)], 20],
    [[qw(GDBM_MALFORMED_DATA GDBM_OPT_BADVAL GDBM_ERR_SNAPSHOT_CLONE
         GDBM_ERR_REALPATH GDBM_ERR_USAGE)], 21]
   );


WriteConstants(
    NAME => 'GDBM_File',
    DEFAULT_TYPE => 'IV',
    BREAKOUT_AT => 8,
    PROXYSUBS => {autoload => 1},
    NAMES => \@names
);
