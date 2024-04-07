BEGIN {
    chdir 't';
    require './test.pl';
    set_up_inc('../lib');
}

plan 402;

for my $decl (qw< my CORE::state our local >) {
    for my $funny (qw< $ @ % >) {
        # Test three syntaxes with each declarator/funny char combination:
        #     my \$foo    my(\$foo)    my\($foo)    for my \$foo

        for my $code("$decl \\${funny}x", "$decl\(\\${funny}x\)",
                     "$decl\\\(${funny}x\)",
                     "for $decl \\${funny}x (\\${funny}y) {}") {
          SKIP: {
            skip "for local is illegal", 3 if $code =~ /^for local/;
            eval $code;
            like
                $@,
                qr/^The experimental declared_refs feature is not enabled/,
               "$code error when feature is disabled";

            use feature 'declared_refs';

            my($w,$c);
            local $SIG{__WARN__} = sub { $c++; $w = shift };
            eval $code;
            is $c, 1, "one warning from $code";
            like $w, qr/^Declaring references is experimental at /,
                "experimental warning for $code";
          }
        }
    }
}

use feature 'declared_refs', 'state';
no warnings 'experimental::declared_refs';

for $decl ('my', 'state', 'our', 'local') {
for $sigl ('$', '@', '%') {
    # The weird code that follows uses ~ as a sigil placeholder and MY
    # as a declarator placeholder.
    my $code = '#line ' . (__LINE__+1) . ' ' . __FILE__ . "\n" . <<'END';
    my $ret = MY \~a;
    is $ret, \~a, 'MY \$a returns ref to $a';
    isnt $ret, \~::a, 'MY \$a ret val is not pkg var';
    my @ret = MY \(~b, ~c);
    is "@ret", \~b." ".\~c, 'MY \(~b, ~c) returns correct refs';
    isnt $ret[0], \~::b, 'first retval of MY \(~b, ~c) is not pkg var';
    isnt $ret[1], \~::c, '2nd retval of MY \(~b, ~c) is not pkg var';
    @ret = MY (\(~d, ~e));
    is "@ret", \~d." ".\~e, 'MY (\(~d, ~e)) returns correct refs';
    isnt $ret[0], \~::d, 'first retval of MY (\(~d, ~e)) is not pkg var';
    isnt $ret[1], \~::e, '2nd retval of MY (\(~d, ~e)) is not pkg var';
    @ret = \MY (\~f, ~g);
    is ${$ret[0]}, \~f, 'first retval of MY (\~f, ~g) is \~f';
    isnt ${$ret[0]}, \~::f, 'first retval of MY (\~f, ~g) is not \~::f';
    is $ret[1], \~g, '2nd retval of MY (\~f, ~g) is ~g';
    isnt $ret[1], \~::g, '2nd retval of MY (\~f, ~g) is not ~::g';
    *MODIFY_SCALAR_ATTRIBUTES = sub {
        is @_, 3, 'MY \~h : risible  calls handler with right no. of args';
        is $_[2], 'risible', 'correct attr passed by MY \~h : risible';
        return;
    };
    SKIP : {
        unless ('MY' eq 'local') {
            skip_if_miniperl "No attributes on miniperl", 2;
            eval 'MY \~h : risible' or die $@ unless 'MY' eq 'local';
        }
    }
    eval 'MY \~a ** 1';
    like $@,
        qr/^Can't (?:declare|modify) exponentiation \(\*\*\) in "?MY"? at/,
       'comp error for MY \~a ** 1';
    $ret = MY \\~i;
    is $$ret, \~i, 'retval of MY \\~i is ref to ref to ~i';
    $ret = MY \\~i;
    isnt $$ret, \~::i, 'retval of MY \\~i is ref to ref to ~::i';
    $ret = MY (\\~i);
    is $$ret, \~i, 'retval of MY (\\~i) is ref to ref to ~i';
    $ret = MY (\\~i);
    isnt $$ret, \~::i, 'retval of MY (\\~i) is ref to ref to ~::i';
    *MODIFY_SCALAR_ATTRIBUTES = sub {
        is @_, 3, 'MY (\~h) : bumpy  calls handler with right no. of args';
        is $_[2], 'bumpy', 'correct attr passed by MY (\~h) : bumpy';
        return;
    };
    SKIP : {
        unless ('MY' eq 'local') {
            skip_if_miniperl "No attributes on miniperl", 2;
            eval 'MY (\~h) : bumpy' or die $@;
        }
    }
    1;
END
    $code =~ s/MY/$decl/g;
    $code =~ s/~/$sigl/g;
    $code =~ s/MODIFY_\KSCALAR/$sigl eq '@' ? "ARRAY" : "HASH"/eggnog
        if $sigl ne '$';
    if ($decl =~ /^(?:our|local)\z/) {
        $code =~ s/is ?no?t/is/g; # tests for package vars
    }
    eval $code or die $@;
}}

use feature 'refaliasing'; no warnings "experimental::refaliasing";
for $decl ('my', 'state', 'our') {
for $sigl ('$', '@', '%') {
    my $code = '#line ' . (__LINE__+1) . ' ' . __FILE__ . "\n" . <<'ENE';
    for MY \~x (\~::y) {
        is \~x, \~::y, '\~x aliased by for MY \~x';
        isnt \~x, \~::x, '\~x is not equivalent to \~::x';
    }
    1;
ENE
    $code =~ s/MY/$decl/g;
    $code =~ s/~/$sigl/g;
    $code =~ s/is ?no?t/is/g if $decl eq 'our';
    eval $code or die $@;
}}
