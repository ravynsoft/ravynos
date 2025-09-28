use strict;
use warnings;
use Test::More;
use Config;

{
    my $p = Impostor->new( 'Donald Duck');
    is( $p->greeting, "Hi, I'm Donald Duck", "blank title");
    $p->assume_title( 'Mr');
    is( $p->greeting, "Hi, I'm Mr Donald Duck", "changed title");
}

# thread support?
subtest 'threads' => sub {
    plan skip_all => "No thread support" unless $Config{usethreads};

    require threads;
    treads->import if threads->can( 'import');

    my $ans;
    my $p = Impostor->new( 'Donald Duck');
    $ans = threads->create( sub { $p->greeting })->join;
    is( $ans, "Hi, I'm Donald Duck", "thread: blank title");
    $p->assume_title( 'Mr');
    $ans = threads->create( sub { $p->greeting })->join;
    is( $ans, "Hi, I'm Mr Donald Duck", "thread: changed title");
    $ans = threads->create(
        sub {
            $p->assume_title( 'Uncle');
            $p->greeting;
        }
    )->join;
    is( $ans, "Hi, I'm Uncle Donald Duck", "thread: local change");
    is( $p->greeting, "Hi, I'm Mr Donald Duck", "thread: change is local");

    # second generation thread
    $ans = threads->create(
        sub {
            threads->create( sub { $p->greeting })->join;
        }
    )->join;
    is( $ans, "Hi, I'm Mr Donald Duck", "double thread: got greeting");
};


############################################################################

# must do this in BEGIN so that field hashes are declared before
# first use above

BEGIN {
    package CFF;
    use Hash::Util::FieldHash qw( :all);

    package Person;

    {
        CFF::fieldhash my %name;
        CFF::fieldhash my %title;

        sub init {
            my $p = shift;
            $name{ $p} = shift || '';
            $title{ $p} = shift || '';
            $p;
        }

        sub name { $name{ shift()} }
        sub title { $title{ shift() } }
    }

    sub new {
        my $class = shift;
        bless( \ my $x, $class)->init( @_);
    }

    sub greeting {
        my $p = shift;
        my $greet = "Hi, I'm ";
        $_ and $greet .= "$_ " for $p->title;
        $greet .= $p->name;
        $greet;
    }

    package Impostor;
    use parent '-norequire', 'Person';

    {
        CFF::fieldhash my %assumed_title;

        sub init {
            my $p = shift;
            my ( $name, $title) = @_;
            $p->Person::init( $name, $title);
            $p->assume_title( $title);
            $p;
        }

        sub title { $assumed_title{ shift()} }

        sub assume_title {
            my $p = shift;
            $assumed_title{ $p} = shift || '';
            $p;
        }
    }
}

done_testing;
