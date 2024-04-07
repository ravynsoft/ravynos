#!perl -w
use strict;
use Pod::Simple::SimpleTree;

my ($tap, $test, %Missing);

BEGIN {
    @ARGV = grep { not($_ eq '--tap' and $tap = 1) } @ARGV;
    if ($tap) {
        require Test::More;
        Test::More->import;
    }
}

my (%Kinds, %Flavor, @Types);
my %Omit;

my $p = Pod::Simple::SimpleTree->new;
$p->accept_targets('Pod::Functions');
my $tree = $p->parse_file(shift)->root;

foreach my $TL_node (@$tree[2 .. $#$tree]) {
    next unless $TL_node->[0] eq 'over-text';
    my $i = 2;
    while ($i <= $#$TL_node) {
        if ($TL_node->[$i][0] ne 'item-text') {
            ++$i;
            next;
        }

        my $item_text = $TL_node->[$i][2];
        die "Confused by $item_text at line $TL_node->[$i][1]{start_line}"
            if ref $item_text;
        $item_text =~ s/\s+\z//s;

        if ($TL_node->[$i+1][0] ne 'for'
           || $TL_node->[$i+1][1]{target} ne 'Pod::Functions') {
            ++$i;
            ++$Missing{$item_text} unless $Omit{$item_text};
            next;
        }
        my $data = $TL_node->[$i+1][2];
        die "Confused by $data at line $TL_node->[$i+1][1]{start_line}"
            unless ref $data eq 'ARRAY';
        my $text = $data->[2];
        die "Confused by $text at line $TL_node->[$i+1][1]{start_line}"
            if ref $text;

        $i += 2;

        if ($text =~ s/^=//) {
            # We are in "Perl Functions by Category"
            die "Expected a paragraph after =item at $TL_node->[$i-2][1]{start_line}"
                unless $TL_node->[$i][0] eq 'Para';
            my $para = $TL_node->[$i];
            # $text is the "type" of the built-in
            # Anything starting ! is not for inclusion in Pod::Functions

            foreach my $func (@$para[2 .. $#$para]) {
                next unless ref $func eq 'ARRAY';
                my $c_node =
                    $func->[0] eq 'C' && !ref $func->[2] ? $func :
                    $func->[0] eq 'L' && ref $func->[2]
                        && $func->[2][0] eq 'C' && !ref $func->[2][2] ? $func->[2] :
                    die "Expected only C<> blocks in paragraph after item at $TL_node->[$i-2][1]{start_line}";
                # Everything is plain text (ie $c_node->[2] is everything)
                # except for C<-I<X>>. So untangle up to one level of nested <>
                my $funcname = join '', map {
                    ref $_ ? $_->[2] : $_
                } @$c_node[2..$#$c_node];
                $funcname =~ s!(q.?)//!$1/STRING/!;
                push @{$Kinds{$text}}, $funcname;
            }
            if ($text =~ /^!/) {
                ++$Omit{$_} foreach @{$Kinds{$text}};
            } else {
                push @Types, [$text, $item_text];
            }
        } else {
            $item_text =~ s/ .*//;
            # For now, just remove any metadata about when it was added:
            $text =~ s/^\+\S+ //;
            $Flavor{$item_text} = $text;
            ++$Omit{$item_text} if $text =~ /^!/;
        }
    }
}

# Take the lists of functions for each type group, and invert them to get the
# type group (or groups) for each function:
my %Type;
while (my ($type, $funcs) = each %Kinds) {
    push @{$Type{$_}}, $type foreach @$funcs;
}

# We sort __SUB__ after sub, but before substr, but __PACKAGE__ after package,
# and __END__ after END.  (We create a temporary array of two elements, where
# the second has the underscores squeezed out, and sort on that element
# first.)
sub sort_funcs {
    map { $_->[0] }
        sort { uc $a->[1] cmp uc $b->[1]
               || $b->[1] cmp $a->[1]
               || $a->[0] =~ /^_/   # here $a and $b are identical when
                                    # underscores squeezed out; so if $a
                                    # begins with an underscore, it should
                                    # sort after $b
               || $a->[0] cmp $b->[0]
             } map  { my $f = tr/_//dr; [ $_, $f ] }
                @_;
}

if ($tap) {
    foreach my $func (sort_funcs(keys %Flavor)) {
       ok ( $Type{$func}, "$func is mentioned in at least one category group");
    }
    foreach (sort keys %Missing) {
        # Ignore anything that looks like an alternative for a function we've
        # already seen;
        s!(?: [A-Z].*| \(\)|\( LIST \)| /PATTERN/.*)!!;
        next if $Flavor{$_};
        if (/^[_a-z]/) {
            fail( "function '$_' has no summary for Pod::Functions" );
        } else {
            fail( "for Pod::Functions" );
        }
    }
    foreach my $kind (sort keys %Kinds) {
        my $funcs = $Kinds{$kind};
        ++$test;
        my $want = join ' ', sort_funcs(@$funcs);
        is ("@$funcs", $want, "category $kind is correctly sorted" );
    }
    done_testing();
    exit;
}

# blead will run this with miniperl, hence we can't use autodie
my $real = 'Functions.pm';
my $temp = "Functions.$$";

END {
    return if !defined $temp || !-e $temp;
    unlink $temp or warn "Can't unlink '$temp': $!";
}

foreach ($real, $temp) {
    next if !-e $_;
    unlink $_ or die "Can't unlink '$_': $!";
}

open my $fh, '>', $temp or die "Can't open '$temp' for writing: $!";
print $fh <<'EOT';
package Pod::Functions;
use strict;

=head1 NAME

Pod::Functions - Group Perl's functions a la perlfunc.pod

=head1 SYNOPSIS

    use Pod::Functions;

    my @misc_ops = @{ $Kinds{ 'Misc' } };
    my $misc_dsc = $Type_Description{ 'Misc' };

or

    perl /path/to/lib/Pod/Functions.pm

This will print a grouped list of Perl's functions, like the 
L<perlfunc/"Perl Functions by Category"> section.

=head1 DESCRIPTION

It exports the following variables:

=over 4

=item %Kinds

This holds a hash-of-lists. Each list contains the functions in the category
the key denotes.

=item %Type

In this hash each key represents a function and the value is the category.
The category can be a comma separated list.

=item %Flavor

In this hash each key represents a function and the value is a short 
description of that function.

=item %Type_Description

In this hash each key represents a category of functions and the value is 
a short description of that category.

=item @Type_Order

This list of categories is used to produce the same order as the
L<perlfunc/"Perl Functions by Category"> section.

=back

=cut

our $VERSION = '1.14';

use Exporter 'import';

our @EXPORT = qw(%Kinds %Type %Flavor %Type_Description @Type_Order);

our(%Kinds, %Type, %Flavor, %Type_Description, @Type_Order);

foreach (
EOT

foreach (@Types) {
    my ($type, $desc) = @$_;
    $type = "'$type'" if $type =~ /[^A-Za-z]/;
    $desc =~ s!([\\'])!\\$1!g;
    printf $fh "    [%-9s  => '%s'],\n", $type, $desc;
}

print $fh <<'EOT';
	) {
    push @Type_Order, $_->[0];
    $Type_Description{$_->[0]} = $_->[1];
};

while (<DATA>) {
    chomp;
    s/^#.*//;
    next unless $_;
    my($name, @data) = split "\t", $_;
    $Flavor{$name} = pop @data;
    $Type{$name} = join ',', @data;
    for my $t (@data) {
        push @{$Kinds{$t}}, $name;
    }
}

close DATA;

my( $typedesc, $list );
unless (caller) { 
    foreach my $type ( @Type_Order ) {
	$list = join(", ", sort @{$Kinds{$type}});
	$typedesc = $Type_Description{$type} . ":";
	write;
    } 
}

format = 

^<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    $typedesc 
~~ ^<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    $typedesc 
 ~~  ^<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	$list
.

1;

__DATA__
EOT

foreach my $func (sort_funcs(keys %Flavor)) {
    my $desc = $Flavor{$func};
    die "No types listed for $func" unless $Type{$func};
    next if $Omit{$func};
    print $fh join("\t", $func, (sort @{$Type{$func}}), $desc), "\n";
}

close $fh or die "Can't close '$temp': $!";
rename $temp, $real or die "Can't rename '$temp' to '$real': $!";
