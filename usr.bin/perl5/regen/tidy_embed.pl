use lib "regen";
use HeaderParser;
use strict;
use warnings;

my $parser= HeaderParser->new(
        pre_process_content => sub {
            my ($self,$line_data)= @_;
            $self->tidy_embed_fnc_entry($line_data);
            my $embed= $line_data->{embed}
                or return;
        },
        post_process_grouped_content => sub {
            my ($self, $group_ary)= @_;
            my $last=chr(0x10FFFF);
            for(my $i= $#$group_ary; $i>=0; $i--) {
                my $entry= $group_ary->[$i];
                if ($entry->{embed}) {
                    $last = $entry->{embed}{name};
                }
                $entry->{sort}{klc}= lc($last)=~s/[^a-z]+//gr;
                $entry->{sort}{key}= $last;
                $entry->{sort}{idx}= $i;
            }
            @{$group_ary}=
                sort {
                    $a->{sort}{klc} cmp $b->{sort}{klc} ||
                    $a->{sort}{key} cmp $b->{sort}{key} ||
                    $a->{sort}{idx} <=> $b->{sort}{idx}
                } @{$group_ary};
            delete $_->{sort} for @$group_ary;
        },
    );
my $tap;
if (@ARGV and $ARGV[0] eq "--tap") {
    $tap = shift @ARGV;
}
my $file= "embed.fnc";
if (@ARGV) {
    $file= shift @ARGV;
}
my $new= "$file.new";
my $bak= "$file.bak";
$parser->read_file($file);
my $lines= $parser->lines;
my (@head, @tail);
# strip off comments at the start of the file
while ($lines->[0]{type} eq "content" and !$lines->[0]{embed}) {
    push @head, shift @$lines;
}

# strip off comments at the bottom of the file
while ($lines->[-1]{type} eq "content" and !$lines->[-1]{embed})
{
    unshift @tail, pop @$lines;
}

my $grouped_content_ary= $parser->group_content();
my $grouped_content_txt= $parser->lines_as_str(
    [ @head, @$grouped_content_ary, @tail ]);
if ($grouped_content_txt ne $parser->{orig_content}) {
    if ($tap) {
        print "not ok - $0 $file\n";
    } elsif (-t) {
        print "Updating $file\n";
    }
    open my $fh,">",$new
        or die "Failed to open '$new' for write: $!";
    print $fh $grouped_content_txt
        or die "Failed to print to '$new': $!";
    close $fh
        or die "Failed to close '$new': $!";
    rename $file, $bak
        or die "Couldn't move '$file' to '$bak': $!";
    rename $new, $file
        or die "Couldn't move embed.fnc.new to embed.fnc: $!";
} elsif ($tap) {
    print "ok - $0 $file\n";
}
