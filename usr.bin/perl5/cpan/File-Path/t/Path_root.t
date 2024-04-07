use strict;
use Test::More;
use Config;
use lib './t';
use FilePathTest qw(
    _run_for_warning
);
use File::Path qw(rmtree mkpath make_path remove_tree);
use File::Spec::Functions;


my $prereq = prereq();
plan skip_all  => $prereq if defined $prereq;
plan tests     => 11;

my $pwent = max_u();
my $grent = max_g();
my ( $max_uid, $max_user ) = @{ $pwent };
my ( $max_gid, $max_group ) = @{ $grent };

my $tmp_base = catdir(
    curdir(),
    sprintf( 'test-%x-%x-%x', time, $$, rand(99999) ),
);

# invent some names
my @dir = (
    catdir($tmp_base, qw(a b)),
    catdir($tmp_base, qw(a c)),
    catdir($tmp_base, qw(z b)),
    catdir($tmp_base, qw(z c)),
);

# create them
my @created = mkpath([@dir]);

my $dir;
my $dir2;

my $dir_stem = $dir = catdir($tmp_base, 'owned-by');

$dir = catdir($dir_stem, 'aaa');
@created = make_path($dir, {owner => $max_user});
is(scalar(@created), 2, "created a directory owned by $max_user...");

my $dir_uid = (stat $created[0])[4];
is($dir_uid, $max_uid, "... owned by $max_uid");

$dir = catdir($dir_stem, 'aab');
@created = make_path($dir, {group => $max_group});
is(scalar(@created), 1, "created a directory owned by group $max_group...");

my $dir_gid = (stat $created[0])[5];
is($dir_gid, $max_gid, "... owned by group $max_gid");

$dir = catdir($dir_stem, 'aac');
@created = make_path( $dir, { user => $max_user,
                              group => $max_group});
is(scalar(@created), 1, "created a directory owned by $max_user:$max_group...");

($dir_uid, $dir_gid) = (stat $created[0])[4,5];
is($dir_uid, $max_uid, "... owned by $max_uid");
is($dir_gid, $max_gid, "... owned by group $max_gid");

{
  # invent a user and group that don't exist
  my $phony_user = get_phony_user();
  my $phony_group = get_phony_group();

  $dir = catdir($dir_stem, 'aad');
  my $rv = _run_for_warning( sub {
      make_path(
          $dir,
          { user => $phony_user, group => $phony_group }
      )
  } );
  like( $rv,
    qr{unable to map $phony_user to a uid, ownership not changed:}s,
    "created a directory not owned by $phony_user:$phony_group...",
  );
  like( $rv,
    qr{unable to map $phony_group to a gid, group ownership not changed:}s,
    "created a directory not owned by $phony_user:$phony_group...",
  );
}

{
    # cleanup
    my $x;
    my $opts = { error => \$x };
    remove_tree($tmp_base, $opts);
    ok(! -d $tmp_base, "directory '$tmp_base' removed, as expected");
    is(scalar(@{$x}), 0, "no error messages using remove_tree() with \$opts");
}

sub max_u {
  # find the highest uid ('nobody' or similar)
  my $max_uid   = 0;
  my $max_user = undef;
  while (my @u = getpwent()) {
    if ($max_uid < $u[2]) {
      $max_uid  = $u[2];
      $max_user = $u[0];
    }
  }
  setpwent(); # in case we want to run again later
  return [ $max_uid, $max_user ];
}

sub max_g {
  # find the highest gid ('nogroup' or similar)
  my $max_gid   = 0;
  my $max_group = undef;
  while ( my @g = getgrent() ) {
    if ($max_gid < $g[2]) {
      $max_gid = $g[2];
      $max_group = $g[0];
    }
  }
  setgrent(); # in case we want to run again later
  return [ $max_gid, $max_group ];
}

sub prereq {
  return "getpwent() not implemented on $^O" unless $Config{d_getpwent};
  return "getgrent() not implemented on $^O" unless $Config{d_getgrent};
  return "not running as root" unless $< == 0;
  return "darwin's nobody and nogroup are -1 or -2" if $^O eq 'darwin';

  my $pwent = max_u();
  my $grent = max_g();
  my ( $max_uid, $max_user ) = @{ $pwent };
  my ( $max_gid, $max_group ) = @{ $grent };

  return "getpwent() appears to be insane" unless $max_uid > 0;
  return "getgrent() appears to be insane" unless $max_gid > 0;
  return undef;
}

sub get_phony_user {
    return "getpwent() not implemented on $^O" unless $Config{d_getpwent};
    return "not running as root" unless $< == 0;
    my %real_users = ();
    while(my @a=getpwent()) {
        $real_users{$a[0]}++;
    }
    my $phony_stem = 'phonyuser';
    my $phony = '';
    do { $phony = $phony_stem . int(rand(10000)); } until (! $real_users{$phony});
    return $phony;
}

sub get_phony_group {
    return "getgrent() not implemented on $^O" unless $Config{d_getgrent};
    return "not running as root" unless $< == 0;
    my %real_groups = ();
    while(my @a=getgrent()) {
        $real_groups{$a[0]}++;
    }
    my $phony_stem = 'phonygroup';
    my $phony = '';
    do { $phony = $phony_stem . int(rand(10000)); } until (! $real_groups{$phony});
    return $phony;
}

