# cfg.mk -- Configuration for maintainer-makefile
#
#   Copyright (c) 2011-2019, 2021-2022 Free Software Foundation, Inc.
#   Written by Gary V. Vaughan, 2011
#
#   This file is part of GNU Libtool.
#
# GNU Libtool is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# GNU Libtool is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Libtool; see the file COPYING.  If not, a copy
# can be downlodad from http://www.gnu.org/licenses/gpl.html,
# or obtained by writing to the Free Software Foundation, Inc.,
# 51 Franklin Street, Boston, # MA 02111-1301, USA.

update-copyright-env := UPDATE_COPYRIGHT_FORCE=1 UPDATE_COPYRIGHT_USE_INTERVALS=1

# Set format of NEWS
old_NEWS_hash := 694e28388e83b832b0d7614e305b107e

manual_title = Portable Dynamic Shared Object Management

# Set the release announcement email addresses, maint.mk will email the
# translation-project instead of autotools-announce otherwise.
ifeq ($(RELEASE_TYPE),stable)
announcement_Cc_ = autotools-announce@gnu.org
else
announcement_Cc_ = autotools-announce@gnu.org, $(PACKAGE_BUGREPORT)
endif

# Don't syntax check the mail subdirectory.
VC_LIST_ALWAYS_EXCLUDE_REGEX = ^mail/

local-checks-to-fix =				\
	sc_require_config_h			\
	sc_require_config_h_first

local-checks-to-skip =				\
	$(local-checks-to-fix)			\
	sc_GPL_version				\
	sc_cast_of_x_alloc_return_value		\
	sc_prohibit_always-defined_macros	\
	sc_prohibit_always_true_header_tests	\
	sc_prohibit_strncpy			\
	sc_trailing_blank			\
	sc_unmarked_diagnostics

# GPL_version: checks for GPLv3, which we don't use
# cast_of_x_alloc_return_value:
#         We support C++ compilation, which does require casting here.
# prohibit_always-defined_macros:
#	we have our own argz and dirent, which use the same macros but
#	are not always-defined in our case.
# prohibit_always_true_header_tests:
#	we have our own argz and dirent, which are not *always* true,
#	so the guards cannot be removed in our case.
# prohibit_strncpy:
#	what's so bad about strncpy anyway?
# trailing_blank: flags valid rfc3676 separators
# unmarked_diagnostics: libtool isn't internationalized


# Check for correct usage of $cc_basename in libtool.m4.
sc_libtool_m4_cc_basename:
	@$(SED) -n "/case \\\$$cc_basename in/,/esac/ {			\
	  /^[	 ]*[a-zA-Z][a-zA-Z0-9+]*[^*][	 ]*)/p;			\
	}" '$(srcdir)/$(macro_dir)/libtool.m4' | grep . && {		\
	  msg="\$$cc_basename matches should include a trailing '*'."	\
	  $(_sc_say_and_exit) } || :

# Check for old-style `quoting'.
exclude_file_name_regexp--sc_old_style_quoting = (^bootstrap|^cfg.mk|\.texi)$$
sc_old_style_quoting:
	@prohibit="^[^\`]*[^=]\`[^'\`]*[a-zA-Z0-9][^'\`]*'[^\`]*[^\\\`]$$" \
	halt="quote 'like this' not \`like this' in comments and output" \
	  $(_sc_search_regexp)

# Check for uses of Xsed without corresponding echo "X
exclude_file_name_regexp--sc_prohibit_Xsed_without_X = ^cfg.mk$$
sc_prohibit_Xsed_without_X:
	@files=$$($(VC_LIST_EXCEPT));					\
	if test -n "$$files"; then					\
	  grep -nE '\$$Xsed' $$files | grep -vE '(func_echo_all|\$$ECHO) \\*"X' && {	\
	    msg="occurrences of '\$$Xsed' without '\$$ECHO "\""X' on the same line" \
	    $(_sc_say_and_exit) } || :;					\
	else :;								\
	fi || :

# Use a consistent dirname and basename idiom.
sc_prohibit_bare_basename:
	@prohibit='\|[	 ]*\$$(base|dir)name' \
	halt='use '\''|$$SED "$$basename"'\'' instead of '\''|$$basename'\'	\
	  $(_sc_search_regexp)

sc_prohibit_basename_with_dollar_sed:
	@prohibit='(base|dir)name="?(\$$SED|sed)[ "]' \
	halt='use '\''basename='\''s|^.*/||'\'' instead of '\''basename="$$SED...'	\
	  $(_sc_search_regexp)

# Check for using '[' instead of 'test'.
exclude_file_name_regexp--sc_prohibit_bracket_as_test = ^cfg.mk$$
sc_prohibit_bracket_as_test:
	@prohibit='if[	 ]+\['						\
	halt="use 'if test' instead of 'if ['"			\
	  $(_sc_search_regexp)

# : ${foo=`bar`} is not perfectly portable (see Shellology in autoconf's manual)
exclude_file_name_regexp--sc_prohibit_command_in_subst = ^cfg.mk$$
sc_prohibit_command_in_subst:
	@prohibit='\$$\{[^`}]*`[^`]*`[^}]*}'				\
	halt='do not use `command` in $${ } substitution`'		\
	  $(_sc_search_regexp)

# Check for quotes within backquotes within quotes "`"bar"`"
exclude_file_name_regexp--sc_prohibit_nested_quotes = ^cfg.mk$$
sc_prohibit_nested_quotes:
	@prohibit='"[^`"]*`[^"`]*"[^"`]*".*`[^`"]*"'			\
	halt='found nested double quotes'				\
	  $(_sc_search_regexp_or_exclude)

# Commas in filenames are quite common, so using them routinely for sed is
# asking for trouble!
sc_prohibit_sed_s_comma:
	@explicit='($$SED|sed)[	 ]+(-e[	 ]+)?['\''"]?s,'		\
	implicit='['\''";][	 ]*s,[^,]*,[^,]*,g?['\''";]'		\
	literal='^[	 ]*s,[^,]*,[^,]*,g?['\''";]?$$'			\
	prohibit='('$$implicit'|'$$explicit'|'$$literal')'		\
	halt='found use of comma separator in sed substitution'		\
	  $(_sc_search_regexp)

# Check for using shift after set dummy (same or following line).
exclude_file_name_regexp--sc_prohibit_set_dummy_without_shift = ^cfg.mk$$
sc_prohibit_set_dummy_without_shift:
	@files=$$($(VC_LIST_EXCEPT));					\
	if test -n "$$files"; then					\
	  grep -nE '(set dummy|shift)' $$files |			\
	    $(SED) -n "/set[	 ][	 ]*dummy/{			\
	      /set.*dummy.*;.*shift/d;					\
	      N;							\
	      /\n.*shift/D;						\
	      p;							\
	    }" | grep -n . && {						\
	    msg="use 'shift' after 'set dummy'"				\
	    $(_sc_say_and_exit) } || :;					\
	else :;								\
	fi || :

# Check for using set -- instead of set dummy
exclude_file_name_regexp--sc_prohibit_set_minus_minus = ^cfg.mk$$
sc_prohibit_set_minus_minus:
	@prohibit='set[	 ]+--[	 ]+'					\
	halt="use 'set dummy ...' instead of 'set -- ...'"		\
	  $(_sc_search_regexp)

# Make sure there is no spurious whitespace before trailing semi-colons
sc_prohibit_space_semicolon:
	@prohibit='[^	 ][	 ]+;[	 ]*((do|done|elif|else|then)[	 ]*)?$$'	\
	halt='found spurious whitespace before trailing semi-colon'	\
	  $(_sc_search_regexp)

# Check for using test X"... instead of test "X...
exclude_file_name_regexp--sc_prohibit_test_X = ^cfg.mk$$
sc_prohibit_test_X:
	@prohibit='test[	 ]+(![	 ])?(-.[	 ]+)?[Xx]["'\'']'	\
	halt='use '\''test "X..."'\'' instead of '\''test X"'\'		\
	  $(_sc_search_regexp)

# Check for bad binary operators.
sc_prohibit_test_binary_operators:
	@prohibit='if[	 ]+["'\'']?\$$[^	 ]+[	 ]+(=|-[lg][te]|-eq|-ne)' \
	halt="Use 'if test \$$something =' instead of 'if \$$something ='" \
	  $(_sc_search_regexp)

# Check for using test $... instead of test "$...
exclude_file_name_regexp--sc_prohibit_test_dollar = ^cfg.mk$$
sc_prohibit_test_dollar:
	@prohibit='test[	 ]+(![	 ])?(-.[	 ]+)?X?\$$[^?#]' \
	exclude='test \$${[A-Za-z_][A-Za-z0-9_]+\+y}'			\
	halt='use '\''test "$$..."'\'' instead of '\''test $$'\'	\
	  $(_sc_search_regexp)

# Never use test -e.
exclude_file_name_regexp--sc_prohibit_test_minus_e = ^cfg.mk$$
sc_prohibit_test_minus_e:
	@prohibit='test[	 ]+(![	 ])?-e'				\
	halt="use 'test -f' instead of 'test -e'"			\
	  $(_sc_search_regexp)

# Check for bad unary operators.
sc_prohibit_test_unary_operators:
	@prohibit='if[	 ]+-[a-z]'					\
	halt="use 'if test -X' instead of 'if -X'"			\
	  $(_sc_search_regexp)

# Don't add noisy characters on the front of the left operand of a test
# to prevent arguments being passed inadvertently (e.g. LHS is '-z'),
# when the other operand is a constant -- just swap them, and remove the
# spurious leading 'x'.
sc_prohibit_test_const_follows_var:
	@var='[	 ]+"[^$$"]*\$$[0-9A-Za-z_][^"]*"'			\
	op='[	 ]+(!?=|-[lgn][et]|-eq)'				\
	const='[	 ]+[^-$$][^$$;	 ]*'				\
	prohibit='test'$$var$$op$$const'[	 ]*(&&|\|\||;|\\?$$)'	\
	halt='use '\''test a = "$$b"'\'' instead of '\''test "x$$b" = xa'\'	\
	  $(_sc_search_regexp)

# Check for opening brace on next line in shell function definition.
exclude_file_name_regexp--sc_require_function_nl_brace = (^HACKING|\.[ch]|\.texi)$$
sc_require_function_nl_brace:
	@for file in $$($(VC_LIST_EXCEPT)); do				\
	  $(SED) -n "/^func_[^	 ]*[	 ]*(/{				\
	    N;								\
	    /^func_[^	 ]* ()\n{$$/d;					\
	    p;								\
	  }" $$file | grep -E . && {					\
	    msg="found malformed function_definition in $$file"		\
	    $(_sc_say_and_exit) } || :;					\
	done

sc_trailing_blank-non-rfc3676:
	@prohibit='([^-][^-][	 ][	 ]*|^[	 ][	 ]*)$$'		\
	halt='found trailing blank(s)'					\
	  $(_sc_search_regexp)

# Avoid useless quotes around assignments with no shell metacharacters.
# Backtick and dollar expansions are not resplit on the RHS of an
# assignment, so those metachars are not listed in the prohibit regex,
# although @ is listed, since it most likely indicates that something
# will be spliced in before the shell executes, and it may need to be
# quoted if it contains any metacharacters after splicing.
define _sc_search_regexp_or_exclude
  files=$$($(VC_LIST_EXCEPT));						\
  if test -n "$$files"; then						\
    grep -nE "$$prohibit" $$files | grep -v '## exclude from $@'	\
      && { msg="$$halt" $(_sc_say_and_exit) } || :;			\
  else :;								\
  fi || :;
endef

exclude_file_name_regexp--sc_useless_braces_in_variable_derefs = \
	test-funclib-quote.sh$$
sc_useless_braces_in_variable_derefs:
	@prohibit='\$${[0-9A-Za-z_]+}[^0-9A-Za-z_]'			\
	halt='found spurious braces around variable dereference'	\
	  $(_sc_search_regexp)

sc_useless_quotes_in_assignment:
	@prohibit='^[	 ]*[A-Za-z_][A-Za-z0-9_]*="[^	 !#&()*;<>?@~^{|}]*"$$'	\
	halt='found spurious quotes around assignment value'		\
	  $(_sc_search_regexp_or_exclude)

# Avoid useless quotes around case arguments such as:
#   case "$foo" in ...
exclude_file_name_regexp--sc_useless_quotes_in_case = ^cfg.mk$$
sc_useless_quotes_in_case:
	@prohibit='case "[^	 "]*"[	 ][	 ]*in'			\
	halt='found spurious quotes around case argument'		\
	  $(_sc_search_regexp)

# List syntax-check exempted files.
exclude_file_name_regexp--sc_prohibit_strcmp = \
  ^doc/libtool.texi$$
exclude_file_name_regexp--sc_prohibit_test_minus_ao = \
  ^m4/libtool.m4$$
exclude_file_name_regexp--sc_space_tab = (\.diff|test-funclib-quote.sh)$$
exclude_file_name_regexp--sc_trailing_blank-non-rfc3676 = \.diff$$
