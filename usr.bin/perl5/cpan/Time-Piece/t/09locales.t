use Test::More;
use Time::Piece;

# Skip if doing a regular install
# These are mostly for reverse parsing tests, not required for installation
plan skip_all => "Reverse parsing not required for installation"
  unless ( $ENV{AUTOMATED_TESTING} );

my $t = gmtime(1373371631);    # 2013-07-09T12:07:11

#locale should be undef
is( $t->_locale, undef );
&Time::Piece::_default_locale();

ok( $t->_locale );

#use localized names
cmp_ok( $t->monname,   'eq', &Time::Piece::_locale()->{mon}[ $t->_mon ] );
cmp_ok( $t->month,     'eq', &Time::Piece::_locale()->{mon}[ $t->_mon ] );
cmp_ok( $t->fullmonth, 'eq', &Time::Piece::_locale()->{month}[ $t->_mon ] );

#use localized names
cmp_ok( $t->wdayname, 'eq', &Time::Piece::_locale()->{wday}[ $t->_wday ] );
cmp_ok( $t->day,      'eq', &Time::Piece::_locale()->{wday}[ $t->_wday ] );
cmp_ok( $t->fullday,  'eq', &Time::Piece::_locale()->{weekday}[ $t->_wday ] );

my @frdays = qw( Dimanche Lundi Merdi Mercredi Jeudi Vendredi Samedi );
$t->day_list(@frdays);
cmp_ok( $t->day,     'eq', &Time::Piece::_locale()->{wday}[ $t->_wday ] );
cmp_ok( $t->fullday, 'eq', &Time::Piece::_locale()->{weekday}[ $t->_wday ] );


#load local locale
Time::Piece->use_locale();

#test reverse parsing
sub check_parsed
{
    my ( $t, $parsed, $t_str, $strp_format ) = @_;

    cmp_ok( $parsed->epoch, '==', $t->epoch,
        "Epochs match for $t_str with $strp_format" );
    cmp_ok(
        $parsed->strftime($strp_format),
        'eq',
        $t->strftime($strp_format),
        "Outputs formatted with $strp_format match"
    );
    cmp_ok( $parsed->strftime(), 'eq', $t->strftime(),
        'Outputs formatted as default match' );
}

my @dates = (
    '%Y-%m-%d %H:%M:%S',
    '%Y-%m-%d %T',
    '%A, %e %B %Y at %H:%M:%S',
    '%a, %e %b %Y at %r',
    '%s',
    '%c',
    '%F %T',

#TODO
#    '%u %U %Y %T',                    #%U,W,V currently skipped inside strptime
#    '%w %W %y %T',
#    '%A, %e %B %Y at %I:%M:%S %p',    #%I and %p can be locale dependant
    '%x %X',    #hard coded to American localization
);

for my $time (
    time(),        # Now, whenever that might be
    1451606400,    # 2016-01-01 00:00
    1451649600,    # 2016-01-01 12:00
  )
{
    my $t = gmtime($time);
    for my $strp_format (@dates) {

        my $t_str = $t->strftime($strp_format);
        my $parsed;
      SKIP: {
            eval { $parsed = $t->strptime( $t_str, $strp_format ); };
            skip "gmtime strptime parse failed", 3 if $@;
            check_parsed( $t, $parsed, $t_str, $strp_format );
        }

    }

}

for my $time (
    time(),        # Now, whenever that might be
    1451606400,    # 2016-01-01 00:00
    1451649600,    # 2016-01-01 12:00
  )
{
    my $t = localtime($time);
    for my $strp_format (@dates) {

        my $t_str = $t->strftime($strp_format);
        my $parsed;
      SKIP: {
            eval { $parsed = $t->strptime( $t_str, $strp_format ); };
            skip "localtime strptime parse failed", 3 if $@;
            check_parsed( $t, $parsed, $t_str, $strp_format );
        }

    }

}

done_testing(154);
