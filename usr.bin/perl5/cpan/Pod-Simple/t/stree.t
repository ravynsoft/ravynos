

use strict;
use warnings;
use Test;
BEGIN { plan tests => 33 };

#use Pod::Simple::Debug (6);

ok 1;

use Pod::Simple::SimpleTree;
print "# Pod::Simple version $Pod::Simple::VERSION\n";

my $hashes_dont_matter = 0;


my $x = 'Pod::Simple::SimpleTree';
sub x {
 my $p = $x->new;
 $p->merge_text(1);
 $p->parse_string_document( shift )->root;
}

ok 1;

print "# a bit of meta-testing...\n";
&ok( deq( 1,     1     ));
&ok(!deq( 2,     1     ));

&ok( deq( undef, undef ));
&ok(!deq( undef, 1     ));
&ok(!deq( 1,     undef ));

&ok( deq( [ ],   [ ]    ));
&ok(!deq( [ ],   1      ));
&ok(!deq( 1,     [ ]    ));

&ok( deq( [1],   [1]    ));
&ok(!deq( [1],   1      ));
&ok(!deq( 1,     [1]    ));
&ok(!deq( [1],   [ ]    ));
&ok(!deq( [ ],   [1]    ));
&ok(!deq( [1],   [2]    ));
&ok(!deq( [2],   [1]    ));

&ok( deq( [ ],   [ ]    ));
&ok(!deq( [ ],   1      ));
&ok(!deq( 1,     [ ]    ));

&ok( deq( {},    {}     ));
&ok(!deq( {},    1      ));
&ok(!deq( 1,     {}     ));
&ok(!deq( {1,2}, {}     ));
&ok(!deq( {},    {1,2}  ));
&ok( deq( {1,2}, {1,2}  ));
&ok(!deq( {2,1}, {1,2}  ));




print '# ', Pod::Simple::pretty(x( "=pod\n\nI like pie.\n" )), "\n";
print "# Making sure we get a tree at all...\n";
ok x( "=pod\n\nI like pie.\n" );


print "# Some real tests...\n";
&ok( deq( x( "=pod\n\nI like pie.\n"),
  [ "Document", {"start_line"=>1},
    [ "Para",   {"start_line"=>3},
      "I like pie."
    ]
  ]
));

$hashes_dont_matter = 1;

&ok( deq( x("=pod\n\nB<foo\t>\n"),
  [ "Document", {},
    [ "Para",   {},
      ["B",     {},
        "foo "
      ]
    ]
  ]
));


&ok( deq( x("=pod\n\nB<pieF<zorch>X<foo>I<pling>>\n"),
  [ "Document", {},
    [ "Para",   {},
      ["B",     {},
        "pie",
        ['F',{}, 'zorch'],
        ['X',{}, 'foo'  ],
        ['I',{}, 'pling'],
      ]
    ]
  ]
));

&ok( deq( x("=over\n\n=item B<pieF<zorch>X<foo>I<pling>>!\n\n=back"),
  [ "Document", {},
    [ "over-text", {},
      [ "item-text", {},
        ["B",     {},
          "pie",
          ['F',{}, 'zorch'],
          ['X',{}, 'foo'  ],
          ['I',{}, 'pling'],
        ],
        '!'
      ]
    ]
  ]
));

print "# Wrapping up... one for the road...\n";
ok 1;
print "# --- Done with ", __FILE__, " --- \n";

sub deq { # deep-equals
  #print "# deq ", Pod::Simple::pretty($_[0], $_[1]), "\n";
  return 1 unless defined $_[0] or defined $_[1]; # two undefs = same
  return '' if defined $_[0] xor defined $_[1];
  return '' if ref($_[0]) ne ref($_[1]); # unequal referentiality
  return $_[0] eq $_[1] unless ref $_[0];
  # So it's a ref:
  if(UNIVERSAL::isa($_[0], 'ARRAY')) {
    return '' unless @{$_[0]} == @{$_[1]};
    for(my $i = 0; $i < @{$_[0]}; $i++) {
      print("# NEQ ", Pod::Simple::pretty($_[0]),
          "\n#  != ", Pod::Simple::pretty($_[1]), "\n"),
       return '' unless deq($_[0][$i], $_[1][$i]); # recurse!
    }
    return 1;
  } elsif(UNIVERSAL::isa($_[0], 'HASH')) {
    return 1 if $hashes_dont_matter;
    return '' unless keys %{$_[0]} == keys %{$_[1]};
    foreach my $k (keys %{$_[0]}) {
      return '' unless exists $_[1]{$k};
      return '' unless deq($_[0]{$k}, $_[1]{$k});
    }
    return 1;
  } else {
    print "# I don't know how to deque $_[0] & $_[1]\n";
    return 1;
  }
}


