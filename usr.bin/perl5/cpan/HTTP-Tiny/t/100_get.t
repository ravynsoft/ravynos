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

for my $file ( dir_list("corpus", qr/^get/ ) ) {
  my $label = basename($file);
  my $data = do { local (@ARGV,$/) = $file; <> };
  my ($params, $expect_req, $give_res) = split /--+\n/, $data;
  my $case = parse_case($params);

  my $url = $case->{url}[0];
  my %headers = hashify( $case->{headers} );
  my %new_args = hashify( $case->{new_args} );

  my %options;
  $options{headers} = \%headers if %headers;
  if ( $case->{data_cb} ) {
    $main::data = '';
    $options{data_callback} = eval join "\n", @{$case->{data_cb}};
    die unless ref( $options{data_callback} ) eq 'CODE';
  }

  my $version = HTTP::Tiny->VERSION || 0;
  my $agent = $new_args{agent} || "HTTP-Tiny/$version";

  # cleanup source data
  $expect_req =~ s{HTTP-Tiny/VERSION}{$agent};
  s{\n}{$CRLF}g for ($expect_req, $give_res);

  # setup mocking and test
  my $res_fh = tmpfile($give_res);
  my $req_fh = tmpfile();

  my $http = HTTP::Tiny->new(keep_alive => 0, %new_args);
  clear_socket_source();
  set_socket_source($req_fh, $res_fh);

  (my $url_basename = $url) =~ s{.*/}{};

  my @call_args = %options ? ($url, \%options) : ($url);
  my $response  = $http->get(@call_args);

  my ($got_host, $got_port) = connect_args();
  my ($exp_host, $exp_port) = (
    ($new_args{proxy} || $url ) =~ m{^http://([^:/]+?):?(\d*)/}g
  );
  $exp_host ||= 'localhost';
  $exp_port ||= 80;

  my $got_req = slurp($req_fh);

  is ($got_host, $exp_host, "$label host $exp_host");
  is ($got_port, $exp_port, "$label port $exp_port");
  is( sort_headers($got_req), sort_headers($expect_req), "$label request data");

  my ($rc) = $give_res =~ m{\S+\s+(\d+)}g;
  # maybe override
  $rc = $case->{expected_rc}[0] if defined $case->{expected_rc};

  is( $response->{status}, $rc, "$label response code $rc" )
    or diag $response->{content};

  if ( substr($rc,0,1) eq '2' ) {
    ok( $response->{success}, "$label success flag true" );
  }
  else {
    ok( ! $response->{success}, "$label success flag false" );
  }

  is ( $response->{url}, $url, "$label response URL" );

  if (defined $case->{expected_headers}) {
    my %expected = hashify( $case->{expected_headers} );
    is_deeply($response->{headers}, \%expected, "$label expected headers");
  }

  my $check_expected = $case->{expected_like}
    ?  sub {
        my ($text, $msg) = @_;
        like( $text, "/".$case->{expected_like}[0]."/", $msg );
      }
    : sub {
        my ($text, $msg) = @_;
        my $exp_content =
          $case->{expected} ? join("$CRLF", @{$case->{expected}}, '') : '';
        is ( $text, $exp_content, $msg );
      }
    ;



  if ( $options{data_callback} ) {
    $check_expected->( $main::data, "$label cb got content" );
    is ( $response->{content}, '', "$label resp content empty" );
  }
  else {
    $check_expected->( $response->{content}, "$label content" );
  }

  ok ( ! exists $response->{redirects}, "$label redirects array doesn't exist")
    or diag explain $response->{redirects};
}

done_testing;
