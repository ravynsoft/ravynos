package Test::Builder::Module;

use strict;

use Test::Builder;

require Exporter;
our @ISA = qw(Exporter);

our $VERSION = '1.302194';


=head1 NAME

Test::Builder::Module - Base class for test modules

=head1 SYNOPSIS

  # Emulates Test::Simple
  package Your::Module;

  my $CLASS = __PACKAGE__;

  use parent 'Test::Builder::Module';
  @EXPORT = qw(ok);

  sub ok ($;$) {
      my $tb = $CLASS->builder;
      return $tb->ok(@_);
  }
  
  1;


=head1 DESCRIPTION

This is a superclass for L<Test::Builder>-based modules.  It provides a
handful of common functionality and a method of getting at the underlying
L<Test::Builder> object.


=head2 Importing

Test::Builder::Module is a subclass of L<Exporter> which means your
module is also a subclass of Exporter.  @EXPORT, @EXPORT_OK, etc...
all act normally.

A few methods are provided to do the C<< use Your::Module tests => 23 >> part
for you.

=head3 import

Test::Builder::Module provides an C<import()> method which acts in the
same basic way as L<Test::More>'s, setting the plan and controlling
exporting of functions and variables.  This allows your module to set
the plan independent of L<Test::More>.

All arguments passed to C<import()> are passed onto 
C<< Your::Module->builder->plan() >> with the exception of 
C<< import =>[qw(things to import)] >>.

    use Your::Module import => [qw(this that)], tests => 23;

says to import the functions C<this()> and C<that()> as well as set the plan
to be 23 tests.

C<import()> also sets the C<exported_to()> attribute of your builder to be
the caller of the C<import()> function.

Additional behaviors can be added to your C<import()> method by overriding
C<import_extra()>.

=cut

sub import {
    my($class) = shift;

    Test2::API::test2_load() unless Test2::API::test2_in_preload();

    # Don't run all this when loading ourself.
    return 1 if $class eq 'Test::Builder::Module';

    my $test = $class->builder;

    my $caller = caller;

    $test->exported_to($caller);

    $class->import_extra( \@_ );
    my(@imports) = $class->_strip_imports( \@_ );

    $test->plan(@_);

    local $Exporter::ExportLevel = $Exporter::ExportLevel + 1;
    $class->Exporter::import(@imports);
}

sub _strip_imports {
    my $class = shift;
    my $list  = shift;

    my @imports = ();
    my @other   = ();
    my $idx     = 0;
    while( $idx <= $#{$list} ) {
        my $item = $list->[$idx];

        if( defined $item and $item eq 'import' ) {
            push @imports, @{ $list->[ $idx + 1 ] };
            $idx++;
        }
        else {
            push @other, $item;
        }

        $idx++;
    }

    @$list = @other;

    return @imports;
}

=head3 import_extra

    Your::Module->import_extra(\@import_args);

C<import_extra()> is called by C<import()>.  It provides an opportunity for you
to add behaviors to your module based on its import list.

Any extra arguments which shouldn't be passed on to C<plan()> should be
stripped off by this method.

See L<Test::More> for an example of its use.

B<NOTE> This mechanism is I<VERY ALPHA AND LIKELY TO CHANGE> as it
feels like a bit of an ugly hack in its current form.

=cut

sub import_extra { }

=head2 Builder

Test::Builder::Module provides some methods of getting at the underlying
Test::Builder object.

=head3 builder

  my $builder = Your::Class->builder;

This method returns the L<Test::Builder> object associated with Your::Class.
It is not a constructor so you can call it as often as you like.

This is the preferred way to get the L<Test::Builder> object.  You should
I<not> get it via C<< Test::Builder->new >> as was previously
recommended.

The object returned by C<builder()> may change at runtime so you should
call C<builder()> inside each function rather than store it in a global.

  sub ok {
      my $builder = Your::Class->builder;

      return $builder->ok(@_);
  }


=cut

sub builder {
    return Test::Builder->new;
}

=head1 SEE ALSO

L<< Test2::Manual::Tooling::TestBuilder >> describes the improved
options for writing testing modules provided by L<< Test2 >>.

=cut

1;
