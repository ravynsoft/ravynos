# Testing the LinkSection class
BEGIN {
    if($ENV{PERL_CORE}) {
        chdir 't';
        @INC = '../lib';
    }
}

### Test the basic sanity of the link-section treelet class

use strict;
use warnings;
use Test;
BEGIN { plan tests => 8 };

#use Pod::Simple::Debug (6);

ok 1;

use Pod::Simple::LinkSection;
use Pod::Simple::BlackBox; # for its pretty()

my $bare_treelet =
  ['B', {'pie' => 'no'},
   'a',
   ['C', {'bzrok' => 'plip'},
    'b'
   ],
   'c'
  ]
;
my $treelet = Pod::Simple::LinkSection->new($bare_treelet);

# Make sure they're not the same

ok ref($bare_treelet), 'ARRAY';
ok ref($treelet), 'Pod::Simple::LinkSection';

print "# Testing stringification...\n";

ok $treelet->stringify, 'abc';  # explicit
ok join('', $treelet),  'abc';  # implicit


print "# Testing non-coreferentiality...\n";
{
  my @stack = ($bare_treelet);
  my $this;
  while(@stack) {
    $this = shift @stack;
    if(ref($this || '') eq 'ARRAY') {
      push @stack, splice @$this;
      push @$this, ("BAD!") x 3;
    } elsif(ref($this || '') eq 'Pod::Simple::LinkSection') {
      push @stack, splice @$this;
      push @$this, ("BAD!") x 3;
    } elsif(ref($this || '') eq 'HASH') {
      %$this = ();
    }
  }
  # These will fail if $treelet and $bare_treelet are coreferential,
  # since we just conspicuously nuked $bare_treelet
  
  ok $treelet->stringify, 'abc';  # explicit
  ok join('', $treelet),  'abc';  # implicit
}


print "# Byebye...\n";
ok 1;
print "# --- Done with ", __FILE__, " --- \n";

