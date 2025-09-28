#!perl

use strict;
use warnings;

use File::Basename;
use Test::More 0.88;
use lib 't';
use Util qw[tmpfile rewind slurp monkey_patch dir_list parse_case
  clear_socket_source set_socket_source sort_headers $CRLF $LF];
use HTTP::Tiny;
BEGIN { monkey_patch() }

for my $file ( dir_list("corpus", qr/^head/ ) ) {
  my $data = do { local (@ARGV,$/) = $file; <> };
  my ($params, $expect_req, $give_res) = split /--+\n/, $data;
  # cleanup source data
  my $version = HTTP::Tiny->VERSION || 0;
  $expect_req =~ s{VERSION}{$version};
  s{\n}{$CRLF}g for ($expect_req, $give_res);

  # figure out what request to make
  my $case = parse_case($params);
  my $url = $case->{url}[0];
  my %options;

  my %headers;
  for my $line ( @{ $case->{headers} } ) {
    my ($k,$v) = ($line =~ m{^([^:]+): (.*)$}g);
    $headers{$k} = $v;
  }
  $options{headers} = \%headers if %headers;

  if ( $case->{content} ) {
    $options{content} = $case->{content}[0];
  }
  elsif ( $case->{content_cb} ) {
    $options{content} = eval join "\n", @{$case->{content_cb}};
  }

  if ( $case->{trailer_cb} ) {
    $options{trailer_callback} = eval join "\n", @{$case->{trailer_cb}};
  }

  # setup mocking and test
  my $res_fh = tmpfile($give_res);
  my $req_fh = tmpfile();

  my $http = HTTP::Tiny->new( keep_alive => 0 );
  clear_socket_source();
  set_socket_source($req_fh, $res_fh);

  (my $url_basename = $url) =~ s{.*/}{};

  my @call_args = %options ? ($url, \%options) : ($url);
  my $response  = $http->head(@call_args);

  my $got_req = slurp($req_fh);

  my $label = basename($file);

  is( sort_headers($got_req), sort_headers($expect_req), "$label request" );

  my ($rc) = $give_res =~ m{\S+\s+(\d+)}g;
  is( $response->{status}, $rc, "$label response code $rc" )
    or diag $response->{content};

  if ( substr($rc,0,1) eq '2' ) {
    ok( $response->{success}, "$label success flag true" );
  }
  else {
    ok( ! $response->{success}, "$label success flag false" );
  }
}

done_testing;
