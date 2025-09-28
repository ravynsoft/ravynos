#!perl

use 5.008001;

use strict;
use warnings;

use Test::More;

BEGIN {
    if (!eval { require Socket }) {
        plan skip_all => "no Socket";
    }
    elsif (ord('A') == 193 && !eval { require Convert::EBCDIC }) {
        plan skip_all => "EBCDIC but no Convert::EBCDIC";
    }
    else {
        plan tests => 54;
    }
}

BEGIN {
  package Foo;

  use IO::File;
  use Net::Cmd;
  our @ISA = qw(Net::Cmd IO::File);

  sub timeout { 0 }

  sub new {
    my $fh = shift->new_tmpfile;
    binmode($fh);
    $fh;
  }

  sub output {
    my $self = shift;
    seek($self,0,0);
    local $/ = undef;
    scalar(<$self>);
  }

  sub response {
    return Net::Cmd::CMD_OK;
  }
}

sub check {
  my $expect = pop;
  my $cmd = Foo->new;
  ok($cmd->datasend, 'datasend') unless @_;
  foreach my $line (@_) {
    ok($cmd->datasend($line), 'datasend');
  }
  ok($cmd->dataend, 'dataend');
  is(
    unpack("H*",$cmd->output),
    unpack("H*",$expect)
  );
}

my $cmd;

check(
  # nothing

  ".\015\012"
);

check(
  "a",

  "a\015\012.\015\012",
);

check(
  "a\r",

  "a\015\015\012.\015\012",
);

check(
  "a\rb",

  "a\015b\015\012.\015\012",
);

check(
  "a\rb\n",

  "a\015b\015\012.\015\012",
);

check(
  "a\rb\n\n",

  "a\015b\015\012\015\012.\015\012",
);

check(
  "a\r",
  "\nb",

  "a\015\012b\015\012.\015\012",
);

check(
  "a\r",
  "\nb\n",

  "a\015\012b\015\012.\015\012",
);

check(
  "a\r",
  "\nb\r\n",

  "a\015\012b\015\012.\015\012",
);

check(
  "a\r",
  "\nb\r\n\n",

  "a\015\012b\015\012\015\012.\015\012",
);

check(
  "a\n.b\n",

  "a\015\012..b\015\012.\015\012",
);

check(
  ".a\n.b\n",

  "..a\015\012..b\015\012.\015\012",
);

check(
  ".a\n",
  ".b\n",

  "..a\015\012..b\015\012.\015\012",
);

check(
  ".a",
  ".b\n",

  "..a.b\015\012.\015\012",
);

check(
  "a\n.",

  "a\015\012..\015\012.\015\012",
);

# Test that datasend() plays nicely with bytes in an upgraded string,
# even though the input should really be encode()d already.
check(
  substr("\x{100}", 0, 0) . "\x{e9}",

  "\x{e9}\015\012.\015\012"
);
