# This file contains tests where \t characters should not be expanded into
# spaces.

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
}

{
      like("\t", qr/[a	b]/x, '\t not ignored under /x');
    unlike("\t", qr/[a	b]/xx, '\t ignored under /xx');
    like("a", qr/[a	b]/xx, '"a" matches qr/[a	b]/xx');
    like("b", qr/[a	b]/xx, '"b" matches qr/[a	b]/xx');
    like("\t", qr/[a\	b]/xx, '"\t" matches qr/[a\	b]/xx');
    like("a", qr/[a\	b]/xx, '"a" matches qr/[a\	b]/xx');
    like("b", qr/[a\	b]/xx, '"b" matches qr/[a\	b]/xx');

      like("\t", qr/(?x:[a	b])/, '\t not ignored under /x');
    unlike("\t", qr/(?xx:[a	b])/, '\t ignored under /xx');
    like("a", qr/(?xx:[a	b])/, '"a" matches qr/(?xx:[a	b])/');
    like("b", qr/(?xx:[a	b])/, '"b" matches qr/(?xx:[a	b])/');
    like("\t", qr/(?xx:[a\	b])/, '"\t" matches qr/(?xx:[a\	b])/');
    like("a", qr/(?xx:[a\	b])/, '"a" matches qr/(?xx:[a\	b])/');
    like("b", qr/(?xx:[a\	b])/, '"b" matches qr/(?xx:[a\	b])/');
}

done_testing;

# ex softtabstop=0 noexpandtab
