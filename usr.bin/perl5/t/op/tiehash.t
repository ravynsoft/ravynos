#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

# This is purposefully simple - hence the O(n) linear searches.
package TestIterators {
    sub TIEHASH {
        bless [], $_[0];
    }

    sub STORE {
        my ($self, $key, $value) = @_;
        push @{$self->[0]}, $key;
        push @{$self->[1]}, $value;
        return $value;
    }

    sub FETCH {
        my ($self, $key) = @_;
        my $i = 0;
        while ($i < @{$self->[0]}) {
            return $self->[1][$i]
                if $self->[0][$i] eq $key;
            ++$i;
        }
        die "$key not found in FETCH";
    }

    sub FIRSTKEY {
        my $self = shift;
        $self->[0][0];
    }

    # As best I can tell, none of our other tie tests actually use the first
    # parameter to nextkey. It's actually (a copy of) the previously returned
    # key. We're not *so* thorough here as to actually hide some state and
    # cross-check that, but the longhand tests below should effectively validate
    # it.
    sub NEXTKEY {
        my ($self, $key) = @_;
        my $i = 0;
        while ($i < @{$self->[0]}) {
            return $self->[0][$i + 1]
                if $self->[0][$i] eq $key;
            ++$i;
        }
        die "$key not found in NEXTKEY";
    }
};

{
    my %h;
    tie %h, 'TestIterators';

    $h{beer} = "foamy";
    $h{perl} = "rules";

    is($h{beer}, "foamy", "found first key");
    is($h{perl}, "rules", "found second key");
    is(eval {
        my $k = $h{decaf};
        1;
    }, undef, "missing key was not found");
    like($@, qr/\Adecaf not found in FETCH/, "with the correct error");

    is(each %h, 'beer', "first iterator");
    is(each %h, 'perl', "second iterator");
    is(each %h, undef, "third iterator is undef");
}

{
    require Tie::Hash;

    my %h = (
        lolcat => "OH HAI!",
        lolrus => "I HAS A BUCKET",
    );

    my @want = sort keys %h;

    my @have;
    while (1) {
        my $k = each %h;
        last
            unless defined $k;
        push @have, $k;
    }
    @have = sort @have;

    # This is a sanity test:
    is("@have", "@want", "get all keys from a loop");

    @have = ();
    keys %h;

    my $k1 = each %h;

    ok(defined $k1, "Got a key");

    # no tie/untie here

    while(1) {
        my $k = each %h;
        last
            unless defined $k;
        push @have, $k;
    }

    # As are these:
    is(scalar @have, 1, "just 1 key from the loop this time");
    isnt($k1, $have[0], "two different keys");

    @have = sort @have, $k1;
    is("@have", "@want", "get all keys just once");

    # And this is the real test.
    #
    # Previously pp_tie would mangle the hash iterator state - it would reset
    # EITER but not RITER, meaning that if the iterator happened to be partway
    # down a chain of entries, the rest of that chain would be skipped, but if
    # the iterator's next position was the start of a (new) chain, nothing would
    # be skipped.
    # We don't have space to store the complete older iterator state (and really
    # nothing should be relying on it), so it seems better to correctly reset
    # the iterator (every time), than leave it in a mess just occasionally.

    @have = ();
    keys %h;

    my $k1 = each %h;

    ok(defined $k1, "Got a key");

    tie %h, 'Tie::StdHash';
    untie %h;

    while(1) {
        my $k = each %h;
        last
            unless defined $k;
        push @have, $k;
    }

    @have = sort @have;
    is(scalar @have, 2, "2 keys from the loop this time");
    is("@have", "@want", "tie/untie resets the hash iterator");
}

{
    require Tie::Hash;
    my $count;

    package Tie::Count {
        use parent -norequire, 'Tie::StdHash';
        sub FETCH {
            ++$count;
            return $_[0]->SUPER::FETCH($_[1]);
        }
    }

    $count = 0;
    my %tied;
    tie %tied, "Tie::Count";
    %tied = qw(perl rules beer foamy);
    my @a = %tied;
    if ($a[0] eq 'beer') {
        is("@a", "beer foamy perl rules", "tied hash in list context");
    } else {
        is("@a", "perl rules beer foamy", "tied hash in list context");
    }
    is($count, 2, "two FETCHes for tied hash in list context");

    $count = 0;

    @a = keys %tied;
    @a = sort @a;
    is("@a", "beer perl", "tied hash keys in list context");
    is($count, 0, "no FETCHes for tied hash keys in list context");

    $count = 0;
    @a = values %tied;
    @a = sort @a;

    is("@a", "foamy rules", "tied hash values in list context");
    is($count, 2, "two FETCHes for tied hash values in list context");
}

{
    # tie/untie on a hash resets the iterator

    # This is not intended as a test of *correctness*. This behaviour is
    # observable by code on CPAN, so potentially some of it will inadvertently
    # be relying on it (and likely not in any regression test). Hence this
    # "test" here is intended as a way to alert us if any core code change has
    # the side effect of alerting this observable behaviour.

    my @keys = qw(bactrianus dromedarius ferus);
    my %Camelus;
    ++$Camelus{$_}
        for @keys;

    my @got;
    push @got, scalar each %Camelus;
    push @got, scalar each %Camelus;
    push @got, scalar each %Camelus;
    is(scalar each %Camelus, undef, 'Fourth each returned undef');
    is(join(' ', sort @got), "@keys", 'The correct three keys');

    @got = ();
    keys %Camelus;

    push @got, scalar each %Camelus;

    # This resets the hash iterator:
    tie %Camelus, 'Tie::StdHash';
    my @all = keys %Camelus;
    is(scalar @all, 0, 'Zero keys when tied');
    untie %Camelus;

    push @got, scalar each %Camelus;
    push @got, scalar each %Camelus;
    my $fourth = scalar each %Camelus;
    isnt($fourth, undef, 'Fourth each did not return undef');
    push @got, $fourth;
    is(scalar each %Camelus, undef, 'Fifth each returned undef');
    my %have;
    @have{@got} = ();
    is(join(' ', sort keys %have), "@keys", 'Still the correct three keys');
}
done_testing();
