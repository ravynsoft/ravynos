#!/usr/bin/perl
#
# Quick and dirty .pbxproj file parser that outputs a skeleton CMakeLists.txt
# Copyright (C) 2026 Zoe Knox <zoe@pixin.net>
# SPDX: MIT
#

use strict;

my %targets;
my %sources;
my %fileopts;

sub readTargets {
    while(1) {
        my $line;
        my $tgt = "";
        while($line = <FH>) {
            last if($line =~ /^\s+};/);
            return if($line =~ /End PBXNativeTarget section/);

            if($line =~ /^[\t ]+([_A-Z0-9]+)\s+\/\*.*\*\/ = {/) {
                $tgt = $1;
            }
            if($line =~ /name\s*=\s*(.*);/) {
                ${targets{$tgt}}{"name"} = $1;
            }
            if($line =~ /productName\s+=\s+(.*);/) {
                ${targets{$tgt}}{"product"} = $1;
            }
            if($line =~ /^[\t ]+([_A-Z0-9]+)\s+\/\*\s+Sources\s+\*\//) {
                ${$targets{$tgt}}{"sources"} = $1;
            }
        }
    }
}

sub readSources {
    while(1) {
        my $line;
        my $src = "";
        while($line = <FH>) {
            last if($line =~ /^\s+};/);
            return if($line =~ /End PBXSourcesBuildPhase section/);

            if($line =~ /^[\t ]+([_A-Z0-9]+)\s+\/\*.*\*\/ = {/) {
                $src = $1;
            }
            if($line =~ /^[\t ]+([_A-Z0-9]+)\s+\/\*.* in Sources\s+\*\/,/) {
                push @{$sources{$src}}, $1;
            }
        }
    }
}

sub readFiles {
    while(1) {
        my $line;
        my $ref = "";
        while($line = <FH>) {
            last if($line =~ /^\s+};/);
            return if($line =~ /End PBXBuildFile section/);

            if($line =~ /^[\t ]+([_A-Z0-9]+)\s+\/\*(.*?) in Sources\s+\*\/\s+=/) {
                $ref = $1;
                ${fileopts{$ref}}{"name"} = $2;
            }
            if($line =~ /COMPILER_FLAGS\s+=\s+(.*?);/) {
                ${fileopts{$ref}}{"COMPILE_OPTIONS"} = $1;
            }
        }
    }
}

sub dumper {
    foreach my $tgt (sort keys %targets) {
        print "$tgt ", ${targets{$tgt}}{"name"}, ", Sources: ", 
            ${targets{$tgt}}{"sources"}, "\n";
    }

    foreach my $src (sort keys %sources) {
        print "$src ";
        my @a = @{$sources{$src}};
        foreach my $fileref (@a) {
            print "\t$fileref\n";
        }
    }

    foreach my $fileref (sort keys %fileopts) {
        print "$fileref ", ${fileopts{$fileref}}{"name"};
        if(${fileopts{$fileref}}{"path"}) {
            print " (", ${fileopts{$fileref}}{"path"}, ")";
        }
        if(${fileopts{$fileref}}{"COMPILE_OPTIONS"}) {
            print " COMPILE_OPTIONS=", ${fileopts{$fileref}}{"COMPILE_OPTIONS"};
        }
        print "\n";
    }
}

sub resolvePaths {
    print "Resolving paths for sources\n";
    foreach my $file (sort keys %fileopts) {
        my $name = ${fileopts{$file}}{"name"};
        open(FIND, "find . -name $name|");
        my $path = <FIND>;
        chomp $path;
        close(FIND);
        if($path) {
            ${fileopts{$file}}{"path"} = $path;
        } else {
            ${fileopts{$file}}{"path"} = $name;
        }   
    }
}

sub writeMakefile {
    foreach my $target (sort keys %targets) {
        my $opts = "";
        my $name = ${targets{$target}}{"name"};
        my $sourceref = ${targets{$target}}{"sources"};

        print "add_library($name STATIC\n";
        my @files = @{$sources{$sourceref}};
        foreach my $file (@files) {
            my $path = ${fileopts{$file}}{"path"};
            my $flags = ${fileopts{$file}}{"COMPILE_OPTIONS"};
            print "\t$path\n";
            if($flags) {
                $opts = "${opts}set_source_files_properties($path TARGET_DIRECTORY ".
                    "${name} PROPERTIES COMPILE_OPTIONS ${flags})\n";
            }
        }
        print(")\n");
        print $opts;
    }
}

my $proj = shift @ARGV;
open FH, $proj;
while(my $line = <FH>) {
    if($line =~ /Begin PBXNativeTarget section/) {
        readTargets;
    }
    if($line =~ /Begin PBXSourcesBuildPhase section/) {
        readSources;
    }
    if($line =~ /Begin PBXBuildFile section/) {
        readFiles;
    }
}
close FH;
resolvePaths;
writeMakefile;
# dumper;
