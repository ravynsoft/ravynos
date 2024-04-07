#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    skip_all("VMS too picky about line endings for record-oriented pipes")
	if $^O eq 'VMS';
}

use strict;

++$|;

my $Perl = which_perl();

my $data = <<'EOD';
x
 yy
z
EOD

(my $data2 = $data) =~ s/\n/\n\n/g;

my $t1 = { data => $data,  write_c => [1,2,length $data],  read_c => [1,2,3,length $data]};
my $t2 = { data => $data2, write_c => [1,2,length $data2], read_c => [1,2,3,length $data2]};

$_->{write_c} = [1..length($_->{data})],
  $_->{read_c} = [1..length($_->{data})+1, 0xe000]  # Need <0xffff for REx
    for (); # $t1, $t2;

my $c;	# len write tests, for each: one _all test, and 3 each len+2
$c += @{$_->{write_c}} * (1 + 3*@{$_->{read_c}}) for $t1, $t2;
$c *= 3*2*2;	# $how_w, file/pipe, 2 reports

$c += 6;	# Tests with sleep()...

print "1..$c\n";

my $set_out = "binmode STDOUT, ':raw'";
$set_out = "binmode STDOUT, ':raw:crlf'"
    if defined  $main::use_crlf && $main::use_crlf == 1;

sub testread ($$$$$$$) {
  my ($fh, $str, $read_c, $how_r, $write_c, $how_w, $why) = @_;
  my $buf = '';
  if ($how_r eq 'readline_all') {
    $buf .= $_ while <$fh>;
  } elsif ($how_r eq 'readline') {
    $/ = \$read_c;
    $buf .= $_ while <$fh>;
  } elsif ($how_r eq 'read') {
    my($in, $c);
    $buf .= $in while $c = read($fh, $in, $read_c);
  } elsif ($how_r eq 'sysread') {
    my($in, $c);
    $buf .= $in while $c = sysread($fh, $in, $read_c);
  } else {
    die "Unrecognized read: '$how_r'";
  }
  close $fh or die "close: $!";
  # The only contamination allowed is with sysread/prints
  $buf =~ s/\r\n/\n/g if $how_r eq 'sysread' and $how_w =~ /print/;
  is(length $buf, length $str, "length with wrc=$write_c, rdc=$read_c, $how_w, $how_r, $why");
  is($buf, $str, "content with wrc=$write_c, rdc=$read_c, $how_w, $how_r, $why");
}

sub testpipe ($$$$$$) {
  my ($str, $write_c, $read_c, $how_w, $how_r, $why) = @_;
  (my $quoted = $str) =~ s/\n/\\n/g;;
  my $fh;
  if ($how_w eq 'print') {	# AUTOFLUSH???
    # Should be shell-neutral:
    open $fh, '-|', qq[$Perl -we "$set_out;print for grep length, split /(.{1,$write_c})/s, qq($quoted)"] or die "open: $!";
  } elsif ($how_w eq 'print/flush') {
    # shell-neutral and miniperl-enabled autoflush? qq(\x24\x7c) eq '$|'
    if ($::IS_ASCII) {
        open $fh, '-|', qq[$Perl -we "$set_out;eval qq(\\x24\\x7c = 1) or die;print for grep length, split /(.{1,$write_c})/s, qq($quoted)"] or die "open: $!";
    }
    else {
        open $fh, '-|', qq[$Perl -we "$set_out;eval qq(\\x5b\\x4f = 1) or die;print for grep length, split /(.{1,$write_c})/s, qq($quoted)"] or die "open: $!";
    }
  } elsif ($how_w eq 'syswrite') {
    ### How to protect \$_
    if ($::IS_ASCII) {
        open $fh, '-|', qq[$Perl -we "$set_out;eval qq(sub w {syswrite STDOUT, \\x24_} 1) or die; w() for grep length, split /(.{1,$write_c})/s, qq($quoted)"] or die "open: $!";
    }
    else {
        open $fh, '-|', qq[$Perl -we "$set_out;eval qq(sub w {syswrite STDOUT, \\x5B_} 1) or die; w() for grep length, split /(.{1,$write_c})/s, qq($quoted)"] or die "open: $!";
    }
  } else {
    die "Unrecognized write: '$how_w'";
  }
  binmode $fh; # remove any :utf8 set by PERL_UNICODE
  binmode $fh, ':crlf'
      if defined $main::use_crlf && $main::use_crlf == 1;
  testread($fh, $str, $read_c, $how_r, $write_c, $how_w, "pipe$why");
}

sub testfile ($$$$$$) {
  my ($str, $write_c, $read_c, $how_w, $how_r, $why) = @_;
  my @data = grep length, split /(.{1,$write_c})/s, $str;

  my $filename = tempfile();
  open my $fh, '>', $filename or die "open: > $filename: $!";
  select $fh;
  binmode $fh; # remove any :utf8 set by PERL_UNICODE
  binmode $fh, ':crlf' 
      if defined $main::use_crlf && $main::use_crlf == 1;
  if ($how_w eq 'print') {	# AUTOFLUSH???
    $| = 0;
    print $fh $_ for @data;
  } elsif ($how_w eq 'print/flush') {
    $| = 1;
    print $fh $_ for @data;
  } elsif ($how_w eq 'syswrite') {
    syswrite $fh, $_ for @data;
  } else {
    die "Unrecognized write: '$how_w'";
  }
  close $fh or die "close: $!";
  open $fh, '<', $filename or die "open: < $filename: $!";
  binmode $fh;
  binmode $fh, ':crlf'
      if defined $main::use_crlf && $main::use_crlf == 1;
  testread($fh, $str, $read_c, $how_r, $write_c, $how_w, "file$why");
}

# shell-neutral and miniperl-enabled autoflush? qq(\x24\x7c) eq '$|'
my $fh;
if ($::IS_ASCII) {
    open $fh, '-|', qq[$Perl -we "eval qq(\\x24\\x7c = 1) or die; binmode STDOUT; sleep 1, print for split //, qq(a\nb\n\nc\n\n\n)"] or die "open: $!";
}
else {
    open $fh, '-|', qq[$Perl -we "eval qq(\\x5B\\x4f = 1) or die; binmode STDOUT; sleep 1, print for split //, qq(a\nb\n\nc\n\n\n)"] or die "open: $!";
}
ok(1, 'open pipe');
binmode $fh, q(:crlf);
ok(1, 'binmode');
$c = undef;
my @c;
push @c, ord $c while $c = getc $fh;
ok(1, 'got chars');
is(scalar @c, 9, 'got 9 chars');
is("@c", join(" ", utf8::unicode_to_native(97),
                   utf8::unicode_to_native(10),
                   utf8::unicode_to_native(98),
                   utf8::unicode_to_native(10),
                   utf8::unicode_to_native(10),
                   utf8::unicode_to_native(99),
                   utf8::unicode_to_native(10),
                   utf8::unicode_to_native(10),
                   utf8::unicode_to_native(10)),
         'got expected chars');
ok(close($fh), 'close');

for my $s (1..2) {
  my $t = ($t1, $t2)[$s-1];
  my $str = $t->{data};
  my $r = $t->{read_c};
  my $w = $t->{write_c};
  for my $read_c (@$r) {
    for my $write_c (@$w) {
      for my $how_r (qw(readline_all readline read sysread)) {
	next if $how_r eq 'readline_all' and $read_c != 1;
        for my $how_w (qw(print print/flush syswrite)) {
	  testfile($str, $write_c, $read_c, $how_w, $how_r, $s);
	  testpipe($str, $write_c, $read_c, $how_w, $how_r, $s);
        }
      }
    }
  }
}

1;
