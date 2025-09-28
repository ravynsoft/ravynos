#!perl

use strict;
use warnings;

use File::Basename;
use Test::More 0.88;
use lib 't';
use Util qw[tmpfile rewind slurp monkey_patch dir_list parse_case
  hashify connect_args clear_socket_source set_socket_source sort_headers
  $CRLF $LF];

use HTTP::Tiny;
BEGIN { monkey_patch() }

for my $file ( dir_list("corpus", qr/^redirect/ ) ) {
  my $label = basename($file);
  my $data = do { local (@ARGV,$/) = $file; <> };
  my ($params, @case_pairs) = split /--+\n/, $data;
  my $case = parse_case($params);
  my $number_of_requests = @case_pairs / 2;

  my $url = $case->{url}[0];
  my $method = $case->{method}[0] || 'GET';
  my %headers = hashify( $case->{headers} );
  my %new_args = hashify( $case->{new_args} );

  my %options;
  $options{headers} = \%headers if %headers;
  my $call_args = %options ? [$method, $url, \%options] : [$method, $url];

  my $version = HTTP::Tiny->VERSION || 0;
  my $agent = $new_args{agent} || "HTTP-Tiny/$version";

  my (@socket_pairs);
  while ( @case_pairs ) {
    my ($expect_req, $give_res) = splice( @case_pairs, 0, 2 );
    # cleanup source data
    $expect_req =~ s{HTTP-Tiny/VERSION}{$agent};
    s{\n}{$CRLF}g for ($expect_req, $give_res);

    # setup mocking and test
    my $req_fh = tmpfile();
    my $res_fh = tmpfile($give_res);

    push @socket_pairs, [$req_fh, $res_fh, $expect_req];
  }

  clear_socket_source();
  set_socket_source(@$_) for @socket_pairs;

  my $http = HTTP::Tiny->new(keep_alive => 0, %new_args);
  my $response  = $http->request(@$call_args);
  my $max_redirects = defined($new_args{max_redirect}) ? $new_args{max_redirect} : 5;
  my $calls = 0 + $max_redirects;

  for my $i ( 0 .. $calls ) {
    last unless @socket_pairs;
    my ($req_fh, $res_fh, $expect_req) = @{ shift @socket_pairs };
    my $got_req = slurp($req_fh);
    is( sort_headers($got_req), sort_headers($expect_req), "$label request ($i)");
    $i++;
  }

  my $exp_content = $case->{expected}
                  ? join("$CRLF", @{$case->{expected}}) : '';

  is ( $response->{content}, $exp_content, "$label content" );

  my $number_of_redirects =
    $max_redirects < $number_of_requests ? $max_redirects : $number_of_requests - 1;
  is ( @{ $response->{redirects} || [] }, $number_of_redirects,
      "$label redirects array size"
  ) or diag explain $response->{redirects};

  if ( $case->{expected_url} ) {
    is ( $response->{url}, $case->{expected_url}[0], "$label response URL" );
  }

}

done_testing;
