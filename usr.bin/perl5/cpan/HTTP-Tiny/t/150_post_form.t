#!perl

use strict;
use warnings;
use open IN => ':raw';

use File::Basename;
use Test::More 0.88;
use lib 't';
use Util qw[tmpfile rewind slurp monkey_patch dir_list parse_case
  clear_socket_source set_socket_source sort_headers $CRLF $LF];
use HTTP::Tiny;
BEGIN { monkey_patch() }

for my $file ( dir_list("corpus", qr/^form/ ) ) {
  my $data = do { local (@ARGV,$/) = $file; <> };
  $data =~ s/$CRLF/$LF/gm if $^O eq 'MSWin32';
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

  my @params = split "\\|", $case->{content}[0];
  my $formdata;
  if ( $case->{datatype}[0] eq 'HASH' ) {
    while ( @params ) {
      my ($key, $value) = splice( @params, 0, 2 );
      if ($value eq "<undef>") {
          $value = undef;
      }
      if ( ref $formdata->{$key} ) {
        push @{$formdata->{$key}}, $value;
      }
      elsif ( exists $formdata->{$key} ) {
        $formdata->{$key} = [ $formdata->{$key}, $value ];
      }
      else {
        $formdata->{$key} = $value;
      }
    }
  }
  else {
    $formdata = [ map { $_ eq "<undef>" ? undef : $_ } @params ];
  }

  # setup mocking and test
  my $res_fh = tmpfile($give_res);
  my $req_fh = tmpfile();

  my $http = HTTP::Tiny->new( keep_alive => 0 );
  clear_socket_source();
  set_socket_source($req_fh, $res_fh);

  (my $url_basename = $url) =~ s{.*/}{};

  my $response  = $http->post_form( $url, $formdata, %options ? (\%options) : ());

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
