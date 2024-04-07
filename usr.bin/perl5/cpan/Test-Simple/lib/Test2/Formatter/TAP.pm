package Test2::Formatter::TAP;
use strict;
use warnings;

our $VERSION = '1.302194';

use Test2::Util qw/clone_io/;

use Test2::Util::HashBase qw{
    no_numbers handles _encoding _last_fh
    -made_assertion
};

sub OUT_STD() { 0 }
sub OUT_ERR() { 1 }

BEGIN { require Test2::Formatter; our @ISA = qw(Test2::Formatter) }

my $supports_tables;
sub supports_tables {
    if (!defined $supports_tables) {
        local $SIG{__DIE__} = 'DEFAULT';
        local $@;
        $supports_tables
            = ($INC{'Term/Table.pm'} && $INC{'Term/Table/Util.pm'})
            || eval { require Term::Table; require Term::Table::Util; 1 }
            || 0;
    }
    return $supports_tables;
}

sub _autoflush {
    my($fh) = pop;
    my $old_fh = select $fh;
    $| = 1;
    select $old_fh;
}

_autoflush(\*STDOUT);
_autoflush(\*STDERR);

sub hide_buffered { 1 }

sub init {
    my $self = shift;

    $self->{+HANDLES} ||= $self->_open_handles;
    if(my $enc = delete $self->{encoding}) {
        $self->encoding($enc);
    }
}

sub _open_handles {
    my $self = shift;

    require Test2::API;
    my $out = clone_io(Test2::API::test2_stdout());
    my $err = clone_io(Test2::API::test2_stderr());

    _autoflush($out);
    _autoflush($err);

    return [$out, $err];
}

sub encoding {
    my $self = shift;

    if ($] ge "5.007003" and @_) {
        my ($enc) = @_;
        my $handles = $self->{+HANDLES};

        # https://rt.perl.org/Public/Bug/Display.html?id=31923
        # If utf8 is requested we use ':utf8' instead of ':encoding(utf8)' in
        # order to avoid the thread segfault.
        if ($enc =~ m/^utf-?8$/i) {
            binmode($_, ":utf8") for @$handles;
        }
        else {
            binmode($_, ":encoding($enc)") for @$handles;
        }
        $self->{+_ENCODING} = $enc;
    }

    return $self->{+_ENCODING};
}

if ($^C) {
    no warnings 'redefine';
    *write = sub {};
}
sub write {
    my ($self, $e, $num, $f) = @_;

    # The most common case, a pass event with no amnesty and a normal name.
    return if $self->print_optimal_pass($e, $num);

    $f ||= $e->facet_data;

    $self->encoding($f->{control}->{encoding}) if $f->{control}->{encoding};

    my @tap = $self->event_tap($f, $num) or return;

    $self->{+MADE_ASSERTION} = 1 if $f->{assert};

    my $nesting = $f->{trace}->{nested} || 0;
    my $handles = $self->{+HANDLES};
    my $indent = '    ' x $nesting;

    # Local is expensive! Only do it if we really need to.
    local($\, $,) = (undef, '') if $\ || $,;
    for my $set (@tap) {
        no warnings 'uninitialized';
        my ($hid, $msg) = @$set;
        next unless $msg;
        my $io = $handles->[$hid] or next;

        print $io "\n"
            if $ENV{HARNESS_ACTIVE}
            && $hid == OUT_ERR
            && $self->{+_LAST_FH} != $io
            && $msg =~ m/^#\s*Failed( \(TODO\))? test /;

        $msg =~ s/^/$indent/mg if $nesting;
        print $io $msg;
        $self->{+_LAST_FH} = $io;
    }
}

sub print_optimal_pass {
    my ($self, $e, $num) = @_;

    my $type = ref($e);

    # Only optimal if this is a Pass or a passing Ok
    return unless $type eq 'Test2::Event::Pass' || ($type eq 'Test2::Event::Ok' && $e->{pass});

    # Amnesty requires further processing (todo is a form of amnesty)
    return if ($e->{amnesty} && @{$e->{amnesty}}) || defined($e->{todo});

    # A name with a newline or hash symbol needs extra processing
    return if defined($e->{name}) && (-1 != index($e->{name}, "\n") || -1 != index($e->{name}, '#'));

    my $ok = 'ok';
    $ok .= " $num" if $num && !$self->{+NO_NUMBERS};
    $ok .= defined($e->{name}) ? " - $e->{name}\n" : "\n";

    if (my $nesting = $e->{trace}->{nested}) {
        my $indent = '    ' x $nesting;
        $ok = "$indent$ok";
    }

    my $io = $self->{+HANDLES}->[OUT_STD];

    local($\, $,) = (undef, '') if $\ || $,;
    print $io $ok;
    $self->{+_LAST_FH} = $io;

    return 1;
}

sub event_tap {
    my ($self, $f, $num) = @_;

    my @tap;

    # If this IS the first event the plan should come first
    # (plan must be before or after assertions, not in the middle)
    push @tap => $self->plan_tap($f) if $f->{plan} && !$self->{+MADE_ASSERTION};

    # The assertion is most important, if present.
    if ($f->{assert}) {
        push @tap => $self->assert_tap($f, $num);
        push @tap => $self->debug_tap($f, $num) unless $f->{assert}->{no_debug} || $f->{assert}->{pass};
    }

    # Almost as important as an assertion
    push @tap => $self->error_tap($f) if $f->{errors};

    # Now lets see the diagnostics messages
    push @tap => $self->info_tap($f) if $f->{info};

    # If this IS NOT the first event the plan should come last
    # (plan must be before or after assertions, not in the middle)
    push @tap => $self->plan_tap($f) if $self->{+MADE_ASSERTION} && $f->{plan};

    # Bail out
    push @tap => $self->halt_tap($f) if $f->{control}->{halt};

    return @tap if @tap;
    return @tap if $f->{control}->{halt};
    return @tap if grep { $f->{$_} } qw/assert plan info errors/;

    # Use the summary as a fallback if nothing else is usable.
    return $self->summary_tap($f, $num);
}

sub error_tap {
    my $self = shift;
    my ($f) = @_;

    my $IO = ($f->{amnesty} && @{$f->{amnesty}}) ? OUT_STD : OUT_ERR;

    return map {
        my $details = $_->{details};

        my $msg;
        if (ref($details)) {
            require Data::Dumper;
            my $dumper = Data::Dumper->new([$details])->Indent(2)->Terse(1)->Pad('# ')->Useqq(1)->Sortkeys(1);
            chomp($msg = $dumper->Dump);
        }
        else {
            chomp($msg = $details);
            $msg =~ s/^/# /;
            $msg =~ s/\n/\n# /g;
        }

        [$IO, "$msg\n"];
    } @{$f->{errors}};
}

sub plan_tap {
    my $self = shift;
    my ($f) = @_;
    my $plan = $f->{plan} or return;

    return if $plan->{none};

    if ($plan->{skip}) {
        my $reason = $plan->{details} or return [OUT_STD, "1..0 # SKIP\n"];
        chomp($reason);
        return [OUT_STD, '1..0 # SKIP ' . $reason . "\n"];
    }

    return [OUT_STD, "1.." . $plan->{count} . "\n"];
}

sub no_subtest_space { 0 }
sub assert_tap {
    my $self = shift;
    my ($f, $num) = @_;

    my $assert = $f->{assert} or return;
    my $pass = $assert->{pass};
    my $name = $assert->{details};

    my $ok = $pass ? 'ok' : 'not ok';
    $ok .= " $num" if $num && !$self->{+NO_NUMBERS};

    # The regex form is ~250ms, the index form is ~50ms
    my @extra;
    defined($name) && (
        (index($name, "\n") != -1 && (($name, @extra) = split(/\n\r?/, $name, -1))),
        ((index($name, "#" ) != -1  || substr($name, -1) eq '\\') && (($name =~ s|\\|\\\\|g), ($name =~ s|#|\\#|g)))
    );

    my $extra_space = @extra ? ' ' x (length($ok) + 2) : '';
    my $extra_indent = '';

    my ($directives, $reason, $is_skip);
    if ($f->{amnesty}) {
        my %directives;

        for my $am (@{$f->{amnesty}}) {
            next if $am->{inherited};
            my $tag = $am->{tag} or next;
            $is_skip = 1 if $tag eq 'skip';

            $directives{$tag} ||= $am->{details};
        }

        my %seen;

        # Sort so that TODO comes before skip even on systems where lc sorts
        # before uc, as other code depends on that ordering.
        my @order = grep { !$seen{$_}++ } sort { lc $b cmp lc $a } keys %directives;

        $directives = ' # ' . join ' & ' => @order;

        for my $tag ('skip', @order) {
            next unless defined($directives{$tag}) && length($directives{$tag});
            $reason = $directives{$tag};
            last;
        }
    }

    $ok .= " - $name" if defined $name && !($is_skip && !$name);

    my @subtap;
    if ($f->{parent} && $f->{parent}->{buffered}) {
        $ok .= ' {';

        # In a verbose harness we indent the extra since they will appear
        # inside the subtest braces. This helps readability. In a non-verbose
        # harness we do not do this because it is less readable.
        if ($ENV{HARNESS_IS_VERBOSE} || !$ENV{HARNESS_ACTIVE}) {
            $extra_indent = "    ";
            $extra_space = ' ';
        }

        # Render the sub-events, we use our own counter for these.
        my $count = 0;
        @subtap = map {
            my $f2 = $_;

            # Bump the count for any event that should bump it.
            $count++ if $f2->{assert};

            # This indents all output lines generated for the sub-events.
            # index 0 is the filehandle, index 1 is the message we want to indent.
            map { $_->[1] =~ s/^(.*\S.*)$/    $1/mg; $_ } $self->event_tap($f2, $count);
        } @{$f->{parent}->{children}};

        push @subtap => [OUT_STD, "}\n"];
    }

    if ($directives) {
        $directives = ' # TODO & SKIP' if $directives eq ' # TODO & skip';
        $ok .= $directives;
        $ok .= " $reason" if defined($reason);
    }

    $extra_space = ' ' if $self->no_subtest_space;

    my @out = ([OUT_STD, "$ok\n"]);
    push @out => map {[OUT_STD, "${extra_indent}#${extra_space}$_\n"]} @extra if @extra;
    push @out => @subtap;

    return @out;
}

sub debug_tap {
    my ($self, $f, $num) = @_;

    # Figure out the debug info, this is typically the file name and line
    # number, but can also be a custom message. If no trace object is provided
    # then we have nothing useful to display.
    my $name  = $f->{assert}->{details};
    my $trace = $f->{trace};

    my $debug = "[No trace info available]";
    if ($trace->{details}) {
        $debug = $trace->{details};
    }
    elsif ($trace->{frame}) {
        my ($pkg, $file, $line) = @{$trace->{frame}};
        $debug = "at $file line $line." if $file && $line;
    }

    my $amnesty = $f->{amnesty} && @{$f->{amnesty}}
        ? ' (with amnesty)'
        : '';

    # Create the initial diagnostics. If the test has a name we put the debug
    # info on a second line, this behavior is inherited from Test::Builder.
    my $msg = defined($name)
        ? qq[# Failed test${amnesty} '$name'\n# $debug\n]
        : qq[# Failed test${amnesty} $debug\n];

    my $IO = $f->{amnesty} && @{$f->{amnesty}} ? OUT_STD : OUT_ERR;

    return [$IO, $msg];
}

sub halt_tap {
    my ($self, $f) = @_;

    return if $f->{trace}->{nested} && !$f->{trace}->{buffered};
    my $details = $f->{control}->{details};

    return [OUT_STD, "Bail out!\n"] unless defined($details) && length($details);
    return [OUT_STD, "Bail out!  $details\n"];
}

sub info_tap {
    my ($self, $f) = @_;

    return map {
        my $details = $_->{details};
        my $table   = $_->{table};

        my $IO = $_->{debug} && !($f->{amnesty} && @{$f->{amnesty}}) ? OUT_ERR : OUT_STD;

        my $msg;
        if ($table && $self->supports_tables) {
            $msg = join "\n" => map { "# $_" } Term::Table->new(
                header      => $table->{header},
                rows        => $table->{rows},
                collapse    => $table->{collapse},
                no_collapse => $table->{no_collapse},
                sanitize    => 1,
                mark_tail   => 1,
                max_width   => $self->calc_table_size($f),
            )->render();
        }
        elsif (ref($details)) {
            require Data::Dumper;
            my $dumper = Data::Dumper->new([$details])->Indent(2)->Terse(1)->Pad('# ')->Useqq(1)->Sortkeys(1);
            chomp($msg = $dumper->Dump);
        }
        else {
            chomp($msg = $details);
            $msg =~ s/^/# /;
            $msg =~ s/\n/\n# /g;
        }

        [$IO, "$msg\n"];
    } @{$f->{info}};
}

sub summary_tap {
    my ($self, $f, $num) = @_;

    return if $f->{about}->{no_display};

    my $summary = $f->{about}->{details} or return;
    chomp($summary);
    $summary =~ s/^/# /smg;

    return [OUT_STD, "$summary\n"];
}

sub calc_table_size {
    my $self = shift;
    my ($f) = @_;

    my $term = Term::Table::Util::term_size();
    my $nesting = 2 + (($f->{trace}->{nested} || 0) * 4); # 4 spaces per level, also '# ' prefix
    my $total = $term - $nesting;

    # Sane minimum width, any smaller and we are asking for pain
    return 50 if $total < 50;

    return $total;
}

1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test2::Formatter::TAP - Standard TAP formatter

=head1 DESCRIPTION

This is what takes events and turns them into TAP.

=head1 SYNOPSIS

    use Test2::Formatter::TAP;
    my $tap = Test2::Formatter::TAP->new();

    # Switch to utf8
    $tap->encoding('utf8');

    $tap->write($event, $number); # Output an event

=head1 METHODS

=over 4

=item $bool = $tap->no_numbers

=item $tap->set_no_numbers($bool)

Use to turn numbers on and off.

=item $arrayref = $tap->handles

=item $tap->set_handles(\@handles);

Can be used to get/set the filehandles. Indexes are identified by the
C<OUT_STD> and C<OUT_ERR> constants.

=item $encoding = $tap->encoding

=item $tap->encoding($encoding)

Get or set the encoding. By default no encoding is set, the original settings
of STDOUT and STDERR are used.

This directly modifies the stored filehandles, it does not create new ones.

=item $tap->write($e, $num)

Write an event to the console.

=back

=head1 SOURCE

The source code repository for Test2 can be found at
F<http://github.com/Test-More/test-more/>.

=head1 MAINTAINERS

=over 4

=item Chad Granum E<lt>exodist@cpan.orgE<gt>

=back

=head1 AUTHORS

=over 4

=item Chad Granum E<lt>exodist@cpan.orgE<gt>

=item Kent Fredric E<lt>kentnl@cpan.orgE<gt>

=back

=head1 COPYRIGHT

Copyright 2020 Chad Granum E<lt>exodist@cpan.orgE<gt>.

This program is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.

See F<http://dev.perl.org/licenses/>

=cut
