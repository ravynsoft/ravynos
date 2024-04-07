#!perl

use strict;
use warnings;

use File::Basename;
use Test::More 0.88;
use lib 't';
use Util qw[tmpfile rewind slurp monkey_patch dir_list parse_case
  clear_socket_source set_socket_source sort_headers $CRLF $LF];
use HTTP::Tiny;
use File::Temp qw/tempdir/;
use File::Spec;

BEGIN { monkey_patch() }

my $tempdir = tempdir( TMPDIR => 1, CLEANUP => 1 );
my $tempfile = File::Spec->catfile( $tempdir, "tempfile.txt" );

my $known_epoch = 760233600;
my $day = 24*3600;

my %timestamp = (
  'modified.txt'      => $known_epoch - 2 * $day,
  'not-modified.txt'  => $known_epoch - 2 * $day,
);

for my $file ( dir_list("corpus", qr/^mirror/ ) ) {
  1 while unlink $tempfile;
  my $data = do { local (@ARGV,$/) = $file; <> };
  my ($params, $expect_req, $give_res) = split /--+\n/, $data;
  # cleanup source data
  my $version = HTTP::Tiny->VERSION || 0;
  $expect_req =~ s{VERSION}{$version};
  s{\n}{$CRLF}g for ($expect_req, $give_res);

  # figure out what request to make
  my $case = parse_case($params);
  my $url = $case->{url}->[0];
  my %options;

  my %headers;
  for my $line ( @{ $case->{headers} } ) {
    my ($k,$v) = ($line =~ m{^([^:]+): (.*)$}g);
    $headers{$k} = $v;
  }
  $options{headers} = \%headers if %headers;

  # maybe create a file
  (my $url_basename = $url) =~ s{.*/}{};
  if ( my $mtime = $timestamp{$url_basename} ) {
    open my $fh, ">", $tempfile;
    close $fh;
    utime $mtime, $mtime, $tempfile;
    if ($^O eq 'MSWin32') {
        # Deal with stat and daylight savings issues on Windows
        # by reading back mtime
        $timestamp{$url_basename} = (stat $tempfile)[9];
    }
  }

  # setup mocking and test
  my $res_fh = tmpfile($give_res);
  my $req_fh = tmpfile();

  my $http = HTTP::Tiny->new( keep_alive => 0 );
  clear_socket_source();
  set_socket_source($req_fh, $res_fh);

  my @call_args = %options ? ($url, $tempfile, \%options) : ($url, $tempfile);
  my $response  = $http->mirror(@call_args);

  my $got_req = slurp($req_fh);

  my $label = basename($file);

  is( sort_headers($got_req), sort_headers($expect_req), "$label request" );

  my ($rc) = $give_res =~ m{\S+\s+(\d+)}g;
  is( $response->{status}, $rc, "$label response code $rc" )
    or diag $response->{content};

  if ( substr($rc,0,1) eq '2' ) {
    ok( $response->{success}, "$label success flag true" );
    ok( -e $tempfile, "$label file created" );
  }
  elsif ( $rc eq '304' ) {
    ok( $response->{success}, "$label success flag true" );
    is( (stat($tempfile))[9], $timestamp{$url_basename},
      "$label file not overwritten" );
  }
  else {
    ok( ! $response->{success}, "$label success flag false" );
    ok( ! -e $tempfile, "$label file not created" );
  }
}

done_testing;
