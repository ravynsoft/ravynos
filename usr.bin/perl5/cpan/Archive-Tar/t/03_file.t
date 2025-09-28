### This program tests Archive::Tar::File ###
use Test::More 'no_plan';
use strict;

use File::Spec::Unix  ();

use Archive::Tar::File;
use Archive::Tar::Constant;

my $all_chars         = join '', "\r\n", map( chr, 0..255 ), "zzz\n\r";
my $start_time        = time() - 1 - TIME_OFFSET;
my $replace_contents  = $all_chars x 42;

my $rename_path                 = 'x/yy/42';
my ($rename_dir, $rename_file)  = dir_and_file( $rename_path );

my @test_files = (
    ###  pathname         contents          optional hash of attributes ###
    [    'x/bIn1',        $all_chars                                      ],
    [    'bIn2',          $all_chars x 2                                  ],
    [    'bIn0',          ''                                              ],

    ### we didnt handle 'false' filenames very well across A::T as of version
    ### 1.32, as reported in #28687. Test for the handling of such files here.
    [    0,               '',                                             ],

    ### keep this one as the last entry
    [    'x/yy/z',        '',               { type  => DIR,
                                              mode  => 0777,
                                              uid   => 42,
                                              gid   => 43,
                                              uname => 'Ford',
                                              gname => 'Prefect',
                                              mtime => $start_time }      ],
);

### new( data => ... ) tests ###
for my $f ( @test_files ) {
    my $unix_path     = $f->[0];
    my $contents      = $f->[1];
    my $attr          = $f->[2] || {};
    my ($dir, $file)  = dir_and_file( $unix_path );

    my $obj = Archive::Tar::File->new( data => $unix_path, $contents, $attr );

    isa_ok( $obj,       'Archive::Tar::File',    "Object created" );
    is( $obj->name,     $file,                   "   name '$file' ok" );
    is( $obj->prefix,   $dir,                    "   prefix '$dir' ok" );
    is( $obj->size,     length($contents),       "   size ok" );
    is( $obj->mode,     exists($attr->{mode}) ? $attr->{mode} : MODE,
                                                 "   mode ok" );
    is( $obj->uid,      exists($attr->{uid}) ? $attr->{uid} : UID,
                                                 "   uid ok" );
    is( $obj->gid,      exists($attr->{gid}) ? $attr->{gid} : GID,
                                                 "   gid ok" );
    is( $obj->uname,    exists($attr->{uname}) ? $attr->{uname} : UNAME->(UID ),
                                                 "   uname ok" );
    is( $obj->gname,    exists($attr->{gname}) ? $attr->{gname} : GNAME->( GID ),
                                                 "   gname ok" );
    is( $obj->type,     exists($attr->{type}) ? $attr->{type} : FILE,
                                                 "   type ok" );
    if (exists($attr->{mtime})) {
        is( $obj->mtime, $attr->{mtime},         "   mtime matches" );
    } else {
        cmp_ok( $obj->mtime, '>', $start_time,   "   mtime after start time" );
    }
    ok( $obj->chksum,                            "   chksum ok" );
    ok( $obj->version,                           "   version ok" );
    ok( ! $obj->linkname,                        "   linkname ok" );
    ok( ! $obj->devmajor,                        "   devmajor ok" );
    ok( ! $obj->devminor,                        "   devminor ok" );
    ok( ! $obj->raw,                             "   raw ok" );

    ### test type checkers
    SKIP: {
        skip "Attributes defined, may not be plainfile", 11 if keys %$attr;

        ok( $obj->is_file,                      "   Object is a file" );

        for my $name (qw[dir hardlink symlink chardev blockdev fifo
                         socket unknown longlink label ]
        ) {
            my $method = 'is_' . $name;

            ok(!$obj->$method(),               "   Object is not a '$name'");
        }
    }

    ### Use "ok" not "is" to avoid binary data screwing up the screen ###
    ok( $obj->get_content eq $contents,          "   get_content ok" );
    ok( ${$obj->get_content_by_ref} eq $contents,
                                                 "   get_content_by_ref ok" );
    is( $obj->has_content, length($contents) ? 1 : 0,
                                                 "   has_content ok" );
    ok( $obj->replace_content( $replace_contents ),
                                                 "   replace_content ok" );
    ok( $obj->get_content eq $replace_contents,  "   get_content ok" );
    ok( $obj->replace_content( $contents ),      "   replace_content ok" );
    ok( $obj->get_content eq $contents,          "   get_content ok" );

    ok( $obj->rename( $rename_path ),            "   rename ok" );
    ok( $obj->chown( 'root' ),                   "   chown 1 arg ok" );
    is( $obj->uname,    'root',                  "   chown to root ok" );
    ok( $obj->chown( 'rocky', 'perl'),           "   chown 2 args ok" );
    is( $obj->uname,    'rocky',                 "   chown to rocky ok" );
    is( $obj->gname,    'perl',                  "   chown to rocky:perl ok" );
    is( $obj->name,     $rename_file,            "   name '$file' ok" );
    is( $obj->prefix,   $rename_dir,             "   prefix '$dir' ok" );
    ok( $obj->rename( $unix_path ),              "   rename ok" );
    is( $obj->name,     $file,                   "   name '$file' ok" );
    is( $obj->prefix,   $dir,                    "   prefix '$dir' ok" );

    ### clone tests ###
    my $clone = $obj->clone;
    isnt( $obj, $clone,                         "Clone is different object" );
    is_deeply( $obj, $clone,                    "   Clone holds same data" );
}

### _downgrade_to_plainfile
{   my $aref        = $test_files[-1];
    my $unix_path   = $aref->[0];
    my $contents    = $aref->[1];
    my $attr        = $aref->[2];

    my $obj = Archive::Tar::File->new( data => $unix_path, $contents, $attr );

    ### check if the object is as expected
    isa_ok( $obj,                           'Archive::Tar::File' );
    ok( $obj->is_dir,                       "   Is a directory" );

    ### do the downgrade
    ok( $obj->_downgrade_to_plainfile,      "   Downgraded to plain file" );

    ### now check if it's downgraded
    ok( $obj->is_file,                      "   Is now a file" );
    is( $obj->linkname, '',                 "   No link entered" );
    is( $obj->mode, MODE,                   "   Mode as expected" );
}

### helper subs ###
sub dir_and_file {
    my $unix_path = shift;
    my ($vol, $dirs, $file) = File::Spec::Unix->splitpath( $unix_path );
    my $dir = File::Spec::Unix->catdir( grep { length } $vol,
                                        File::Spec::Unix->splitdir( $dirs ) );
    return ( $dir, $file );
}
