#!/opt/bin/perl
use strict;
use warnings;

use Cwd;
use Getopt::Std;
use File::Basename;
use FindBin;

my $Opts = {};
getopts( 'r:p:e:c:vudn', $Opts );

my $Cwd         = cwd();
my $Verbose     = 1;
my $ExcludeRe   = $Opts->{e} ? qr/$Opts->{e}/i : undef;
my $Debug       = $Opts->{v} || 0;
my $RunDiff     = $Opts->{d} || 0;
my $PkgDir      = $Opts->{p} || cwd();
my $Repo        = $Opts->{r} or die "Need repository!\n". usage();
my $Changes     = $Opts->{c} || 'Changes ChangeLog';
my $NoBranch    = $Opts->{n} || 0;

### strip trailing slashes;
$Repo =~ s|/$||;

my $CPV         = $Debug ? '-v' : '';
my $TestBin     = 'ptardiff';
my $PkgDirRe    = quotemeta( $PkgDir .'/' );
my $BranchName  = basename( $PkgDir ) . '.' . $$;
my $OrigRepo    = $Repo;

### establish working directory, either branch or full copy
if ( $NoBranch ) {
    ### create a copy of the repo directory
    my $RepoCopy = "$Repo-$BranchName";
    print "Copying repository to $RepoCopy ..." if $Verbose;

    ### --archive == -dPpR, but --archive is not portable, and neither
    ### is -d, so settling for -PpR
    system( "cp -PpR -f $Repo $RepoCopy" )
        and die "Copying master repo to $RepoCopy failed: $?";

    ### Going forward, use the copy in place of the original repo
    $Repo = $RepoCopy;

    print "done\n" if $Verbose;
}
else {
    ### create a git branch for the new package
    print "Setting up a branch from blead called '$BranchName'..." if $Verbose;
    chdir $Repo or die "Could not chdir to $Repo: $!";
    unless ( -d '.git' ) {
        die "\n$Repo is not a git repository\n";
    }
    my $status = `git status`;
    unless ( $status =~ /nothing to commit/ims ) {
      die "\nWorking directory not clean. Stopping.\n";
    }
    system( "git checkout -b $BranchName blead" )
            and die "Could not create branch '$BranchName': $?";

    print "done\n" if $Verbose;
}

### chdir there
chdir $PkgDir or die "Could not chdir to $PkgDir: $!";

### copy over all files under lib/
my @LibFiles;
{   print "Copying libdir..." if $Verbose;
    die "Can't (yet) copy from a repository (found .git or .svn)"
        if -d '.git' || -d '.svn';
    die "No lib/ directory found\n" unless -d 'lib';
    system( "cp -fR $CPV lib $Repo" ) and die "Copy of lib/ failed: $?";

    @LibFiles =    map { chomp; $_ }
                    ### should we get rid of this file?
                    grep { $ExcludeRe && $_ =~ $ExcludeRe
                        ? do {  warn "Removing $Repo/$_\n";
                                system("rm $Repo/$_") and die "rm '$Repo/$_' failed: $?";
                                undef
                            }
                        : 1
                     } `find lib -type f`
        or die "Could not detect library files\n";

    print "done\n" if $Verbose;
}

### find the directory to put the t/ and bin/ files under
my $RelTopDir;      # topdir from the repo root
my $TopDir;         # full path to the top dir
my $ModName;        # name of the module
my @ModFiles;       # the .PMs in this package
{   print "Creating top level dir..." if $Verbose;

    ### make sure we get the shortest file, so we don't accidentally get
    ### a subdir
    @ModFiles   =  sort { length($a) <=> length($b) }
                   map  { chomp; $_ }
                   grep { $ExcludeRe ? $_ !~ $ExcludeRe : 1 }
                   grep /\.p(?:m|od)$/,
                    `find $PkgDir/lib -type f`
                        or die "No TopDir detected\n";

    $RelTopDir  = $ModFiles[0];
    $RelTopDir  =~ s/^$PkgDirRe//;
    $RelTopDir  =~ s/\.p(m|od)$//;
    $TopDir     = "$Repo/$RelTopDir";

    ### create the dir if it's not there yet
    unless( -d $TopDir ) {
        system( "mkdir $TopDir" ) and die "Creating dir $TopDir failed: $?";
    }

    ### the module name, like Foo::Bar
    ### slice syntax not elegant, but we need to remove the
    ### leading 'lib/' entry
    ### stupid temp vars! stupid perl! it doesn't do @{..}[0..-1] :(
    {   my @list = @{[split '/', $RelTopDir]};
        $ModName = join '::', @list[1 .. $#list];
    }

    ### the .pm files in this package
    @ModFiles = map { s|^$PkgDirRe||; $_ } @ModFiles
        or die "Could not detect modfiles\n";

    print "done\n" if $Verbose;
}

my $TopDirRe = quotemeta( $TopDir . '/' );

### copy over t/ and bin/ directories to the $TopDir
my @TestFiles;
{   print "Copying t/* files to $TopDir..." if $Verbose;

   -d 't'
       ? system( "cp -fR $CPV t $TopDir" ) && die "Copy of t/ failed: $?"
       : warn "No t/ directory found\n";

    @TestFiles =    map { chomp; s|^$TopDirRe||; s|//|/|g; $_ }
                    ### should we get rid of this file?
                    grep { $ExcludeRe && $_ =~ $ExcludeRe
                        ? do {  warn "Removing $_\n";
                                system("rm $TopDir/$_") and die "rm '$_' failed: $?";
                                undef
                            }
                        : 1
                     } `find t -type f`
        or die "Could not detect testfiles\n";

    print "done\n" if $Verbose;
}

my $BinDir;
my @BinFiles;
my $TopBinDir;
BIN: {
    $BinDir = -d 'bin'      ? 'bin' :
              -d 'scripts'  ? 'scripts' : undef ;
    unless ($BinDir) {
        print "No bin/ or scripts/ directory found\n" if $Verbose;
        last BIN;
    }
    my $TopBinDir = "$TopDir/$BinDir/";
    print "Copying $BinDir/* files to $TopBinDir..." if $Verbose;

    my $CopyCmd = "cp -fR $CPV $BinDir $TopDir";
    print "Running '$CopyCmd'..." if $Verbose;

    system($CopyCmd) && die "Copy of $BinDir failed: $?";

    @BinFiles = map { chomp; s|^$TopDirRe||; s|//|/|g; $_ }
                ### should we get rid of this file?
                grep { $ExcludeRe && $_ =~ $ExcludeRe
                    ? do {  warn "Removing $_\n";
                            system("rm $TopDir/$_") and die "rm '$_' failed: $?";
                            undef
                        }
                    : 1
                 } `find $BinDir -type f`
        or die "Could not detect binfiles\n";

    print "done\n" if $Verbose;
}

### copy over change log
my @Changes;
foreach my $cl (split m/\s+/ => $Changes) {
    -f $cl or next;
    push @Changes, $cl;
    print "Copying $cl files to $TopDir..." if $Verbose;

    system( "cp -f $CPV $cl $TopDir" )
        and die "Copy of $cl failed: $?";
}


### add files where they are required
my @NewFiles;
my @ChangedFiles;
{   for my $bin ( map { basename( $_ ) } @BinFiles ) {
        print "Registering $bin with system files...\n";

        ### fix installperl, so these files get installed by other utils
        ### ./installperl:    return if $name =~
        ### /^(?:cpan|instmodsh|prove|corelist|ptar|ptardiff)\z/;
        {   my $file = 'installperl';

            ### not there already?
            unless( `grep $TestBin $Repo/$file| grep $bin` ) {
                print "   Adding $bin to $file..." if $Verbose;

                ### double \\| required --> once for in this script, once
                ### for the cli
                system("$^X -pi -e 's/($TestBin\\|)/$bin|\$1/' $Repo/$file")
                    and die "Could not add $bin to $file: $?";
                print "done\n" if $Verbose;
                push @ChangedFiles, $file;
            } else {
                print "    $bin already mentioned in $file\n" if $Verbose;
            }
        }

        ### fix utils.lst, so the new tools are mentioned
        {   my $file = 'utils.lst';

            ### not there already?
            unless( `grep $bin $Repo/$file` ) {
                print "    Adding $bin to $file..." if $Verbose;

                ### double \\| required --> once for in this script, once
                ### for the cli
                system("$^X -pi -e 's!($TestBin)!\$1\nutils/$bin!' $Repo/$file")
                    and die "Could not add $bin to $file: $?";
                print "done\n" if $Verbose;
                push @ChangedFiles, $file;
            } else {
                print "    $bin already mentioned in $file\n" if $Verbose;
            }
        }

        ### make a $bin.PL file and fix it up
        {   my $src  = "utils/${TestBin}.PL";
            my $file = "utils/${bin}.PL";

            ### not there already?
            unless( -e "$Repo/$file" ) {
                print "    Creating $file..." if $Verbose;

                ### important part of the template looks like this
                ### (we'll need to change it):
                # my $script = File::Spec->catfile(
                #    File::Spec->catdir(
                #        File::Spec->updir, qw[lib Archive Tar bin]
                #    ), "module-load.pl");

                ### copy another template file
                system( "cp -f $Repo/$src $Repo/$file" )
                    and die "Could not create $file from $src: $?";

                ### change the 'updir' path
                ### make sure to escape the \[ character classes
                my $updir = join ' ', (split('/', $RelTopDir), $BinDir);
                system( "$^X -pi -e'".
                        's/^(.*?File::Spec->updir, qw\[).+?(\].*)$/'.
                        "\$1 $updir \$2/' $Repo/$file"
                ) and die "Could not fix updir for $bin in $file: $?";


                ### change the name of the file from $TestBin to $bin
                system( "$^X -pi -e's/$TestBin/$bin/' $Repo/$file" )
                    and die "Could not update $file with '$bin' as name: $?";

                print "done\n" if $Verbose;

            } else {
                print "    $file already exists\n" if $Verbose;
            }

            ### we've may just have created a new file, it will have to
            ### go into the manifest
            push @NewFiles, $file;
        }

        ### add an entry to utils/Makefile.PL for $bin
        {   my $file = "utils/Makefile.PL";

            ### not there already?
            unless( `grep $bin $Repo/$file` ) {
                print "    Adding $bin entries to $file..." if $Verbose;

                ### $bin appears on 4 lines in this file, so replace all 4
                ### first, pl =
                system( "$^X -pi -e'/^pl\\s+=/ && s/(${TestBin}.PL)/".
                        "\$1 ${bin}.PL/' $Repo/$file"
                ) and die "Could not add $bin to the pl = entry: $?";

                ### next, plextract =
                system( "$^X -pi -e'/^plextract\\s+=/ " .
                        "&& s/(${TestBin})/\$1 $bin/' $Repo/$file"
                ) and die "Could not add $bin to the plextract = entry: $?";

                ### third, plextractexe =
                system( "$^X -pi -e'/^plextractexe\\s+=/ " .
                        "&& s!(\./${TestBin})!\$1 ./$bin!' $Repo/$file"
                ) and die "Could not add $bin to the plextractexe = entry: $?";

                ### last, the make directive $bin:
                system( "$^X -pi -e'/^(${TestBin}:.+)/; \$x=\$1 or next;" .
                        "\$x =~ s/$TestBin/$bin/g;" . '$_.=$/.$x.$/;' .
                        "' $Repo/$file"
                ) and die "Could not add $bin as a make directive: $?";

                push @ChangedFiles, $file;
                print "done\n" if $Verbose;
            } else {
                print "    $bin already added to $file\n" if $Verbose;
            }
        }

        ### add entries to win32/Makefile
        ### they contain the following lines:
        # ./win32/makefile.mk:            ..\utils\ptardiff       \
        # ./win32/makefile.mk:        xsubpp instmodsh prove ptar ptardiff
        for my $file ( qw[win32/Makefile] ) {
            unless ( `grep $bin $Repo/$file` ) {
                print "    Adding $bin entries to $file..." if $Verbose;

               system( "$^X -pi -e'/^(.+?utils.${TestBin}.+)/;".
                        '$x=$1 or next;' .
                        "\$x =~ s/$TestBin/$bin/g;" . '$_.=$x.$/;' .
                        "' $Repo/$file"
                ) and die "Could not add $bin to UTILS section in $file: $?\n";

                system( "$^X -pi -e's/( $TestBin)/\$1 $bin/' $Repo/$file" )
                    and die "Could not add $bin to $file: $?\n";

                push @ChangedFiles, $file;
                print "done\n" if $Verbose;
            } else {
                print "    $bin already added to $file\n" if $Verbose;
            }
        }

        ### we need some entries in a vms specific file as well..
        ### except, I don't understand how it works or what it does, and it
        ### looks all a bit odd... so lets just print a warning...
        ### the entries look something like this:
        # ./vms/descrip_mms.template:utils4 = [.utils]enc2xs.com
        #   [.utils]piconv.com [.utils]cpan.com [.utils]prove.com
        #   [.utils]ptar.com [.utils]ptardiff.com [.utils]shasum.com
        # ./vms/descrip_mms.template:[.utils]ptardiff.com : [.utils]ptardiff.PL
        #   $(ARCHDIR)Config.pm
        {   my $file = 'vms/descrip_mms.template';

            unless( `grep $bin $Repo/$file` ) {
                print $/.$/;
                print "    WARNING! You should add entries like the following\n"
                    . "    to $file (Using $TestBin as an example)\n"
                    . "    Unfortunately I don't understand what these entries\n"
                    . "    do, so I won't change them automatically:\n\n";

                print `grep -nC1 $TestBin $Repo/$file`;
                print $/.$/;

            } else {
                print "    $bin already added to $file\n" if $Verbose;
            }
        }
    }
}

### update the manifest
{   my $file        = $Repo . '/MANIFEST';
    my @manifest;
    {   open my $fh, '<', $file or die "Could not open $file: $!";
        @manifest    = <$fh>;
        close $fh;
    }

    ### fill it with files from our package
    my %pkg_files;
    for ( @ModFiles ) {
        $pkg_files{$_}              = "$_\t$ModName\n";
    }

    for ( @TestFiles ) {
        $pkg_files{"$RelTopDir/$_"} = "$RelTopDir/$_\t$ModName tests\n"
    }

    for ( @BinFiles ) {
        $pkg_files{"$RelTopDir/$_"} = "$RelTopDir/$_\tthe ".
                                            basename($_) ." utility\n";
    }

    for ( @Changes ) {
        $pkg_files{"$RelTopDir/$_"} = "$RelTopDir/$_\t$ModName change log\n";
    }

    for ( @NewFiles ) {
        $pkg_files{$_}              = "$_\tthe ".
                                        do { m/(.+?)\.PL$/; basename($1) } .
                                        " utility\n"
    }

    ### remove all the files that are already in the manifest;
    delete $pkg_files{ [split]->[0] } for @manifest;

    print "Adding the following entries to the MANIFEST:\n" if $Verbose;
    print "\t$_" for sort values %pkg_files;
    print $/.$/;

    push @manifest, values %pkg_files;

    {   chmod 0644, $file;
        open my $fh, '>', $file or die "Could not open $file for writing: $!";
        #print $fh sort { lc $a cmp lc $b } @manifest;
        ### XXX stolen from pod/buildtoc:sub do_manifest
        print $fh
            map  { $_->[0] }
            sort { $a->[1] cmp $b->[1] || $a->[0] cmp $b->[0] }
            map  { my $f = lc $_; $f =~ s/[^a-z0-9\s]//g; [ $_, $f ] }
            @manifest;

        close $fh;
    }
    push @ChangedFiles, 'MANIFEST';
}


### would you like us to show you a diff?
if( $RunDiff ) {
    if ( $NoBranch ) {

        my $diff = $Repo; $diff =~ s/$$/patch/;

        ### weird RV ;(
        my $master = basename( $OrigRepo );
        my $repo   = basename( $Repo );
        my $chdir  = dirname( $OrigRepo );

        ### the .patch file is added by an rsync from the APC
        ### but isn't actually in the p4 repo, so exclude it
        my $cmd = "cd $chdir; diff -ruN --exclude=.patch $master $repo > $diff";

        print "Running: '$cmd'\n";

        print "Generating diff..." if $Verbose;

        system( $cmd );
            #and die "Could not write diff to '$diff': $?";
        die "Could not write diff to '$diff'" unless -e $diff && -s _;

        print "done\n" if $Verbose;
        print "\nDiff can be applied with patch -p1 in $OrigRepo\n\n";
        print "  Diff written to: $diff\n\n" if $Verbose;
    }
    else {
        my $diff = "$Repo/$BranchName"; $diff =~ s/$$/patch/;
        my $cmd = "cd $Repo; git diff > $diff";

        print "Running: '$cmd'\n";

        print "Generating diff..." if $Verbose;

        system( $cmd );
            #and die "Could not write diff to '$diff': $?";
        die "Could not write diff to '$diff'" unless -e $diff && -s _;

        print "done\n" if $Verbose;
        print "  Diff written to: $diff\n\n" if $Verbose;
    }
}


# add files to git index
unless ( $NoBranch ) {
    chdir $Repo;
    system( "git add $CPV $_" )
        for ( @LibFiles, @NewFiles, @ChangedFiles,
              map { "$RelTopDir/$_" } @TestFiles, @BinFiles, @Changes );
}

# return to original directory
chdir $Cwd;

sub usage {
    my $me = basename($0);
    return qq[

Usage: $me -r PERL_REPO_DIR [-p PACKAGE_DIR] [-v] [-d] [-e REGEX]

Options:
  -r    Path to perl-core git repository
  -v    Run verbosely
  -c    File containing changelog (default 'Changes' or 'ChangeLog')
  -e    Perl regex matching files that shouldn't be included
  -d    Create a diff as patch file
  -p    Path to the package to add. Defaults to cwd()
  -n    No branching; repository is not a git repo

    \n];

}
