#!/bin/sh

# This is a placeholder file for symbols that should be exported
# into config_h.SH and Porting/Glossary. See also metaconfig.h
#
# First version was created from the part in Makefile.SH
# H.Merijn Brand 21 Dec 2010 (Tux)
#
# Mentioned variables are forced to be included into config_h.SH
# as they are only included if meta finds them referenced. That
# implies that noone can use them unless they are available and
# they won't be available unless used. When new symbols are probed
# in Configure, this is the way to force them into availability.

# H.Merijn Brand [17 Feb 2004]
# This comment is just to ensure that Configure will find variables that
# are removed/replaced in patches on blead, but are still needed in the
# 5.8.x, 5.6.x and 5.005.x maintenance tracks.
# metaconfig -m will scan all .SH files on this level (not deeper), and
# not in other subfolders. This file is as good as any .SH
# patch   references
# #22227 $baserev
# #22302 $yacc $byacc

# H.Merijn Brand [30 Oct 2004]
# Mentioned for the same reason for future reference
# #23434 $d_strlcat $d_strlcpy

# H.Merijn Brand [21 Dec 2010]
# cbeaa1895 removed the reference to $perlpath

true # eof - does nothing.
