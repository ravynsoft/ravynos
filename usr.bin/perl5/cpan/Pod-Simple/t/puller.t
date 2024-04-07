BEGIN {
    if($ENV{PERL_CORE}) {
        chdir 't';
        @INC = '../lib';
    }
}

use strict;
use warnings;
use Test;
BEGIN { plan tests => 136 };

#use Pod::Simple::Debug (5);

#sub Pod::Simple::MANY_LINES () {1}
#sub Pod::Simple::PullParser::DEBUG () {1}


use Pod::Simple::PullParser;

sub pump_it_up {
  my $p = Pod::Simple::PullParser->new;
  $p->set_source( \( $_[0] ) );
  my(@t, $t);
  while($t = $p->get_token) { push @t, $t }
  print "# Count of tokens: ", scalar(@t), "\n";
  print "#  I.e., {", join("\n#       + ",
    map ref($_) . ": " . $_->dump, @t), "} \n";
  return @t;
}

my @t;

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

@t = pump_it_up(qq{\n\nProk\n\n=head1 Things\n\n=cut\n\nBzorch\n\n});

if(not(
  ok scalar( grep { ref $_ and $_->can('type') } @t), 5
)) {
  ok 0,1, "Wrong token count. Failing subsequent tests.\n";
  for ( 1 .. 12 ) {ok 0}
} else {
  ok $t[0]->type, 'start';
  ok $t[1]->type, 'start';
  ok $t[2]->type, 'text';
  ok $t[3]->type, 'end';
  ok $t[4]->type, 'end';

  ok $t[0]->tagname, 'Document';
  ok $t[1]->tagname, 'head1';
  ok $t[2]->text,    'Things';
  ok $t[3]->tagname, 'head1';
  ok $t[4]->tagname, 'Document';

  ok $t[0]->attr('start_line'), '5';
  ok $t[1]->attr('start_line'), '5';
}



#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@t = pump_it_up(
    qq{Woowoo\n\n=over\n\n=item *\n\nStuff L<HTML::TokeParser>\n\n}
  . qq{=item *\n\nThings I<like that>\n\n=back\n\n=cut\n\n}
);

if(
  not( ok scalar( grep { ref $_ and $_->can('type') } @t) => 16 )
) {
  ok 0,1, "Wrong token count. Failing subsequent tests.\n";
  for ( 1 .. 32 ) {ok 0}
} else {
  ok $t[ 0]->type, 'start';
  ok $t[ 1]->type, 'start';
  ok $t[ 2]->type, 'start';
  ok $t[ 3]->type, 'text';
  ok $t[ 4]->type, 'start';
  ok $t[ 5]->type, 'text';
  ok $t[ 6]->type, 'end';
  ok $t[ 7]->type, 'end';

  ok $t[ 8]->type, 'start';
  ok $t[ 9]->type, 'text';
  ok $t[10]->type, 'start';
  ok $t[11]->type, 'text';
  ok $t[12]->type, 'end';
  ok $t[13]->type, 'end';
  ok $t[14]->type, 'end';
  ok $t[15]->type, 'end';



  ok $t[ 0]->tagname, 'Document';
  ok $t[ 1]->tagname, 'over-bullet';
  ok $t[ 2]->tagname, 'item-bullet';
  ok $t[ 3]->text, 'Stuff ';
  ok $t[ 4]->tagname, 'L';
  ok $t[ 5]->text, 'HTML::TokeParser';
  ok $t[ 6]->tagname, 'L';
  ok $t[ 7]->tagname, 'item-bullet';

  ok $t[ 8]->tagname, 'item-bullet';
  ok $t[ 9]->text, 'Things ';
  ok $t[10]->tagname, 'I';
  ok $t[11]->text, 'like that';
  ok $t[12]->tagname, 'I';
  ok $t[13]->tagname, 'item-bullet';
  ok $t[14]->tagname, 'over-bullet';
  ok $t[15]->tagname, 'Document';

  ok $t[4]->attr("type"), "pod";
}


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
{
print "# Testing unget_token\n";

my $p = Pod::Simple::PullParser->new;
$p->set_source( \qq{\nBzorch\n\n=pod\n\nLala\n\n\=cut\n} );

ok 1;
my $t;
$t = $p->get_token;
ok $t && $t->type, 'start';
ok $t && $t->tagname, 'Document';
print "# ungetting ($t).\n";
$p->unget_token($t);
ok 1;

$t = $p->get_token;
ok $t && $t->type, 'start';
ok $t && $t->tagname, 'Document';
my @to_save = ($t);

$t = $p->get_token;
ok $t && $t->type, 'start';
ok $t && $t->tagname, 'Para';
push @to_save, $t;

print "# ungetting (@to_save).\n";
$p->unget_token(@to_save);
splice @to_save;


$t = $p->get_token;
ok $t && $t->type, 'start';
ok $t && $t->tagname, 'Document';

$t = $p->get_token;
ok $t && $t->type, 'start';
ok $t && $t->tagname, 'Para';

ok 1;

}


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

{
print "# Testing pullparsing from an arrayref\n";
my $p = Pod::Simple::PullParser->new;
ok 1;
$p->set_source( ['','Bzorch', '','=pod', '', 'Lala', 'zaza', '', '=cut'] );
ok 1;
my( @t, $t );
while($t = $p->get_token) {
  print "# Got a token: ", $t->dump, "\n#\n";
  push @t, $t;
}
ok scalar(@t), 5; # count of tokens
ok $t[0]->type, 'start';
ok $t[1]->type, 'start';
ok $t[2]->type, 'text';
ok $t[3]->type, 'end';
ok $t[4]->type, 'end';

ok $t[0]->tagname, 'Document';
ok $t[1]->tagname, 'Para';
ok $t[2]->text,    'Lala zaza';
ok $t[3]->tagname, 'Para';
ok $t[4]->tagname, 'Document';

}

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

{
print "# Testing pullparsing from an arrayref with terminal newlines\n";
my $p = Pod::Simple::PullParser->new;
ok 1;
$p->set_source( [ map "$_\n",
  '','Bzorch', '','=pod', '', 'Lala', 'zaza', '', '=cut'] );
ok 1;
my( @t, $t );
while($t = $p->get_token) {
  print "# Got a token: ", $t->dump, "\n#\n";
  push @t, $t;
}
ok scalar(@t), 5; # count of tokens
ok $t[0]->type, 'start';
ok $t[1]->type, 'start';
ok $t[2]->type, 'text';
ok $t[3]->type, 'end';
ok $t[4]->type, 'end';

ok $t[0]->tagname, 'Document';
ok $t[1]->tagname, 'Para';
ok $t[2]->text,    'Lala zaza';
ok $t[3]->tagname, 'Para';
ok $t[4]->tagname, 'Document';

}

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

END { unlink "temp.pod" }
{
print "# Testing pullparsing from a file\n";
my $p = Pod::Simple::PullParser->new;
ok 1;
open(OUT, ">temp.pod") || die "Can't write-open temp.pod: $!";
print OUT
 map "$_\n",
  '','Bzorch', '','=pod', '', 'Lala', 'zaza', '', '=cut'
;
close(OUT);
ok 1;
sleep 1;

$p->set_source("temp.pod");

my( @t, $t );
while($t = $p->get_token) {
  print "# Got a token: ", $t->dump, "\n#\n";
  push @t, $t;
  print "#  That's token number ", scalar(@t), "\n";
}
ok scalar(@t), 5; # count of tokens
ok $t[0]->type, 'start';
ok $t[1]->type, 'start';
ok $t[2]->type, 'text';
ok $t[3]->type, 'end';
ok $t[4]->type, 'end';

ok $t[0]->tagname, 'Document';
ok $t[1]->tagname, 'Para';
ok $t[2]->text,    'Lala zaza';
ok $t[3]->tagname, 'Para';
ok $t[4]->tagname, 'Document';

}

# ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

{
print "# Testing pullparsing from a glob\n";
my $p = Pod::Simple::PullParser->new;
ok 1;
open(IN, "<temp.pod") || die "Can't read-open temp.pod: $!";
$p->set_source(*IN);

my( @t, $t );
while($t = $p->get_token) {
  print "# Got a token: ", $t->dump, "\n#\n";
  push @t, $t;
  print "#  That's token number ", scalar(@t), "\n";
}
ok scalar(@t), 5; # count of tokens
ok $t[0]->type, 'start';
ok $t[1]->type, 'start';
ok $t[2]->type, 'text';
ok $t[3]->type, 'end';
ok $t[4]->type, 'end';

ok $t[0]->tagname, 'Document';
ok $t[1]->tagname, 'Para';
ok $t[2]->text,    'Lala zaza';
ok $t[3]->tagname, 'Para';
ok $t[4]->tagname, 'Document';
close(IN);

}

# ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

{
print "# Testing pullparsing from a globref\n";
my $p = Pod::Simple::PullParser->new;
ok 1;
open(IN, "<temp.pod") || die "Can't read-open temp.pod: $!";
$p->set_source(\*IN);

my( @t, $t );
while($t = $p->get_token) {
  print "# Got a token: ", $t->dump, "\n#\n";
  push @t, $t;
  print "#  That's token number ", scalar(@t), "\n";
}
ok scalar(@t), 5; # count of tokens
ok $t[0]->type, 'start';
ok $t[1]->type, 'start';
ok $t[2]->type, 'text';
ok $t[3]->type, 'end';
ok $t[4]->type, 'end';

ok $t[0]->tagname, 'Document';
ok $t[1]->tagname, 'Para';
ok $t[2]->text,    'Lala zaza';
ok $t[3]->tagname, 'Para';
ok $t[4]->tagname, 'Document';
close(IN);

}

# ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

{
print "# Testing pullparsing from a filehandle\n";
my $p = Pod::Simple::PullParser->new;
ok 1;
open(IN, "<temp.pod") || die "Can't read-open temp.pod: $!";
$p->set_source(*IN{IO});

my( @t, $t );
while($t = $p->get_token) {
  print "# Got a token: ", $t->dump, "\n#\n";
  push @t, $t;
  print "#  That's token number ", scalar(@t), "\n";
}
ok scalar(@t), 5; # count of tokens
ok $t[0]->type, 'start';
ok $t[1]->type, 'start';
ok $t[2]->type, 'text';
ok $t[3]->type, 'end';
ok $t[4]->type, 'end';

ok $t[0]->tagname, 'Document';
ok $t[1]->tagname, 'Para';
ok $t[2]->text,    'Lala zaza';
ok $t[3]->tagname, 'Para';
ok $t[4]->tagname, 'Document';
close(IN);

}



#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


print "# Wrapping up... one for the road...\n";
ok 1;
print "# --- Done with ", __FILE__, " --- \n";

__END__

