#!/bin/sh
#
# mkmakemod.sh: generate Makefile.in files for module building
#
# Options:
#   -m = file is already generated; only build the second stage
#   -i = do not build second stage
#
# Args:
#   $1 = subdirectory to look in, relative to $top_srcdir
#   $2 = final output filename, within the $1 directory
#
# This script must be run from the top-level build directory, and $top_srcdir
# must be set correctly in the environment.
#
# This looks in $1, and uses all the *.mdd files there.  Each .mdd file
# defines one module.  The .mdd file is actually a shell script, which will
# be sourced.  It may define the following shell variables:
#
#   name            name of this module
#   moddeps         modules on which this module depends (default none)
#   nozshdep        non-empty indicates no dependence on the `zsh/main' pseudo-module
#   alwayslink      if non-empty, always link the module into the executable
#   autofeatures    features defined by the module, for autoloading
#   autofeatures_emu As autofeatures, but for non-zsh emulation modes
#   objects         .o files making up this module (*must* be defined)
#   proto           .syms files for this module (default generated from $objects)
#   headers         extra headers for this module (default none)
#   hdrdeps         extra headers on which the .mdh depends (default none)
#   otherincs       extra headers that are included indirectly (default none)
#
# The .mdd file may also include a Makefile.in fragment between lines
# `:<<\Make' and `Make' -- this will be copied into Makemod.in.
#
# The resulting Makemod.in knows how to build each module that is defined.
# For each module it also knows how to build a .mdh file.  Each source file
# should #include the .mdh file for the module it is a part of.  The .mdh
# file #includes the .mdh files for any module dependencies, then each of
# $headers, and then each .epro (for global declarations).  It will
# be recreated if any of the dependency .mdh files changes, or if any of
# $headers or $hdrdeps changes.  When anything depends on it, all the .epros
# and $otherincs will be made up to date, but the .mdh file won't actually
# be rebuilt if those files change.
#
# The order of sections of the output file is thus:
#   simple generated macros
#   macros generated from *.mdd
#   included Makemod.in.in
#   rules generated from *.mdd
# The order dependencies are basically that the generated macros are required
# in Makemod.in.in, but some of the macros that it creates are needed in the
# later rules.
#

# sed script to normalise a pathname
sed_normalise='
    s,^,/,
    s,$,/,
    :1
    s,/\./,/,
    t1
    :2
    s,/[^/.][^/]*/\.\./,/,
    s,/\.[^/.][^/]*/\.\./,/,
    s,/\.\.[^/][^/]*/\.\./,/,
    t2
    s,^/$,.,
    s,^/,,
    s,\(.\)/$,\1,
'

# decide which stages to process
first_stage=true
second_stage=true
if test ."$1" = .-m; then
    shift
    first_stage=false
elif test ."$1" = .-i; then
    shift
    second_stage=false
fi

top_srcdir=`echo $top_srcdir | sed "$sed_normalise"`
the_subdir=$1
the_makefile=$2

if $first_stage; then

    dir_top=`echo $the_subdir | sed 's,[^/][^/]*,..,g'`

    trap "rm -f $the_subdir/${the_makefile}.in; exit 1" 1 2 15
    echo "creating $the_subdir/${the_makefile}.in"
    exec 3>&1 >$the_subdir/${the_makefile}.in
    echo "##### ${the_makefile}.in generated automatically by mkmakemod.sh"
    echo "##### DO NOT EDIT!"
    echo
    echo "##### ===== DEFINITIONS ===== #####"
    echo
    echo "makefile = ${the_makefile}"
    echo "dir_top = ${dir_top}"
    echo "subdir = ${the_subdir}"
    echo

    bin_mods=`grep link=static ./config.modules | \
    sed -e '/^#/d' -e 's/ .*/ /' -e 's/^name=/ /'`
    dyn_mods="`grep link=dynamic ./config.modules | \
    sed -e '/^#/d' -e 's/ .*/ /' -e 's/^name=/ /'`"
    module_list="${bin_mods}${dyn_mods}"

    if grep '^#define DYNAMIC ' config.h >/dev/null; then
	is_dynamic=true
    else
	is_dynamic=false
    fi

    here_mddnames=
    all_subdirs=
    all_modobjs=
    all_modules=
    all_mdds=
    all_mdhs=
    all_proto=
    lastsub=//
    for module in $module_list; do
        modfile="`grep '^name='$module' ' ./config.modules | \
	  sed -e 's/^.* modfile=//' -e 's/ .*//'`"
	case $modfile in
	    $the_subdir/$lastsub/*) ;;
	    $the_subdir/*/*)
		lastsub=`echo $modfile | sed 's,^'$the_subdir'/,,;s,/[^/]*$,,'`
		case "$all_subdirs " in
		    *" $lastsub "* ) ;;
		    * )
			all_subdirs="$all_subdirs $lastsub"
		    ;;
		esac
		;;
	    $the_subdir/*)
		mddname=`echo $modfile | sed 's,^.*/,,;s,\.mdd$,,'`
		here_mddnames="$here_mddnames $mddname"
		build=$is_dynamic
		case $is_dynamic@$bin_mods in
		    *" $module "*)
			build=true
			all_modobjs="$all_modobjs modobjs.${mddname}" ;;
		    true@*)
			all_modules="$all_modules ${mddname}.\$(DL_EXT)" ;;
		esac
		all_mdds="$all_mdds ${mddname}.mdd"
		$build && all_mdhs="$all_mdhs ${mddname}.mdh"
		$build && all_proto="$all_proto proto.${mddname}"
		;;
	esac
    done
    echo "MODOBJS =$all_modobjs"
    echo "MODULES =$all_modules"
    echo "MDDS    =$all_mdds"
    echo "MDHS    =$all_mdhs"
    echo "PROTOS  =$all_proto"
    echo "SUBDIRS =$all_subdirs"
    echo
    echo "ENTRYOBJ = \$(dir_src)/modentry..o"
    echo "NNTRYOBJ ="
    echo "ENTRYOPT = -emodentry"
    echo "NNTRYOPT ="
    echo

    echo "##### ===== INCLUDING Makemod.in.in ===== #####"
    echo
    cat $top_srcdir/Src/Makemod.in.in
    echo

    case $the_subdir in
	Src) modobjs_sed= ;;
	Src/*) modobjs_sed="| sed 's\" \" "`echo $the_subdir | sed 's,^Src/,,'`"/\"g' " ;;
	*) modobjs_sed="| sed 's\" \" ../$the_subdir/\"g' " ;;
    esac

    other_mdhs=
    remote_mdhs=
    other_exports=
    remote_exports=
    other_modules=
    remote_modules=
    for mddname in $here_mddnames; do

	unset name moddeps nozshdep alwayslink hasexport
	unset autofeatures autofeatures_emu
	unset objects proto headers hdrdeps otherincs
	. $top_srcdir/$the_subdir/${mddname}.mdd
	q_name=`echo $name | sed 's,Q,Qq,g;s,_,Qu,g;s,/,Qs,g'`
	test -n "${moddeps+set}" || moddeps=
	test -n "$nozshdep" || moddeps="$moddeps zsh/main"
	test -n "${proto+set}" ||
	    proto=`echo $objects '' | sed 's,\.o ,.syms ,g'`

	dobjects=`echo $objects '' | sed 's,\.o ,..o ,g'`
	modhdeps=
	mododeps=
	exportdeps=
	imports=
	q_moddeps=
	for dep in $moddeps; do
	    depfile="`grep '^name='$dep' ' ./config.modules | \
	      sed -e 's/^.* modfile=//' -e 's/ .*//'`"
	    q_dep=`echo $dep | sed 's,Q,Qq,g;s,_,Qu,g;s,/,Qs,g'`
	    q_moddeps="$q_moddeps $q_dep"
	    eval `echo $depfile | sed 's,/\([^/]*\)\.mdd$,;depbase=\1,;s,^,loc=,'`
	    case "$binmod" in
		*" $dep "* )
		    dep=zsh/main
		;;
	    esac

	    case $the_subdir in
		$loc)
		    mdh="${depbase}.mdh"
		    export="${depbase}.export"
		    case "$dep" in
			zsh/main )
			    mdll="\$(dir_top)/Src/libzsh-\$(VERSION).\$(DL_EXT) "
			;;
			* )
			    mdll="${depbase}.\$(DL_EXT) "
			;;
		    esac
		    ;;
		$loc/*)
		    mdh="\$(dir_top)/$loc/${depbase}.mdh"
		    case "$other_mdhs " in
			*" $mdh "*) ;;
			*) other_mdhs="$other_mdhs $mdh" ;;
		    esac
		    export="\$(dir_top)/$loc/${depbase}.export"
		    case "$other_exports " in
			*" $export "*) ;;
			*) other_exports="$other_exports $export" ;;
		    esac
		    case "$dep" in
			zsh/main )
			    mdll="\$(dir_top)/Src/libzsh-\$(VERSION).\$(DL_EXT) "
			;;
			* )
			    mdll="\$(dir_top)/$loc/${depbase}.\$(DL_EXT) "
			;;
		    esac
		    case "$other_modules " in
			*" $mdll "*) ;;
			*) other_modules="$other_modules $mdll" ;;
		    esac
		    ;;
		*)
		    mdh="\$(dir_top)/$loc/${depbase}.mdh"
		    case "$remote_mdhs " in
			*" $mdh "*) ;;
			*) remote_mdhs="$remote_mdhs $mdh" ;;
		    esac
		    export="\$(dir_top)/$loc/${depbase}.export"
		    case "$remote_exports " in
			*" $export "*) ;;
			*) remote_exports="$remote_exports $export" ;;
		    esac
		    case "$dep" in
			zsh/main )
			    mdll="\$(dir_top)/Src/libzsh-\$(VERSION).\$(DL_EXT) "
			;;
			* )
			    mdll="\$(dir_top)/$loc/${depbase}.\$(DL_EXT) "
			;;
		    esac
		    case "$remote_modules " in
			*" $mdll "*) ;;
			*) remote_modules="$remote_modules $mdll" ;;
		    esac
		    ;;
	    esac
	    modhdeps="$modhdeps $mdh"
	    exportdeps="$exportdeps $export"
	    imports="$imports \$(IMPOPT)$export"
	    case "$mododeps " in
		*" $mdll "* )
		    :
		;;
		* )
		    mododeps="$mododeps $mdll"
		;;
	    esac
	done

	echo "##### ===== DEPENDENCIES GENERATED FROM ${mddname}.mdd ===== #####"
	echo
	echo "MODOBJS_${mddname} = $objects"
	echo "MODDOBJS_${mddname} = $dobjects \$(@E@NTRYOBJ)"
	echo "SYMS_${mddname} = $proto"
	echo "EPRO_${mddname} = "`echo $proto '' | sed 's,\.syms ,.epro ,g'`
	echo "INCS_${mddname} = \$(EPRO_${mddname}) $otherincs"
	echo "EXPIMP_${mddname} = $imports \$(EXPOPT)$mddname.export"
	echo "NXPIMP_${mddname} ="
	echo "LINKMODS_${mddname} = $mododeps"
	echo "NOLINKMODS_${mddname} = "
	echo
	echo "proto.${mddname}: \$(EPRO_${mddname})"
	echo "\$(SYMS_${mddname}): \$(PROTODEPS)"
	echo
	echo "${mddname}.export: \$(SYMS_${mddname})"
	echo "	@( echo '#!'; cat \$(SYMS_${mddname}) | sed -n '/^X/{s/^X//;p;}' | sort -u ) > \$@"
	echo
	echo "modobjs.${mddname}: \$(MODOBJS_${mddname})"
	echo "	@echo '' \$(MODOBJS_${mddname}) $modobjs_sed>> \$(dir_src)/stamp-modobjs.tmp"
	echo
	if test -z "$alwayslink"; then
	    case " $all_modules" in *" ${mddname}."*)
		echo "install.modules-here: install.modules.${mddname}"
		echo "uninstall.modules-here: uninstall.modules.${mddname}"
		echo
	    ;; esac
	    instsubdir=`echo $name | sed 's,^,/,;s,/[^/]*$,,'`
	    echo "install.modules.${mddname}: ${mddname}.\$(DL_EXT)"
	    echo "	\$(SHELL) \$(sdir_top)/mkinstalldirs \$(DESTDIR)\$(MODDIR)${instsubdir}"
	    echo "	\$(INSTALL_PROGRAM) \$(STRIPFLAGS) ${mddname}.\$(DL_EXT) \$(DESTDIR)\$(MODDIR)/${name}.\$(DL_EXT)"
	    echo
	    echo "uninstall.modules.${mddname}:"
	    echo "	rm -f \$(DESTDIR)\$(MODDIR)/${name}.\$(DL_EXT)"
	    echo
	    echo "${mddname}.\$(DL_EXT): \$(MODDOBJS_${mddname}) ${mddname}.export $exportdeps \$(@LINKMODS@_${mddname})"
	    echo '	rm -f $@'
	    echo "	\$(DLLINK) \$(@E@XPIMP_$mddname) \$(@E@NTRYOPT) \$(MODDOBJS_${mddname}) \$(@LINKMODS@_${mddname}) \$(LIBS) "
	    echo
	fi
	echo "${mddname}.mdhi: ${mddname}.mdhs \$(INCS_${mddname})"
	echo "	@test -f \$@ || echo 'do not delete this file' > \$@"
	echo
	echo "${mddname}.mdhs: ${mddname}.mdd"
	echo "	@\$(MAKE) -f \$(makefile) \$(MAKEDEFS) ${mddname}.mdh.tmp"
	echo "	@if cmp -s ${mddname}.mdh ${mddname}.mdh.tmp; then \\"
	echo "	    rm -f ${mddname}.mdh.tmp; \\"
	echo "	    echo \"\\\`${mddname}.mdh' is up to date.\"; \\"
	echo "	else \\"
	echo "	    mv -f ${mddname}.mdh.tmp ${mddname}.mdh; \\"
	echo "	    echo \"Updated \\\`${mddname}.mdh'.\"; \\"
	echo "	fi"
	echo "	echo 'timestamp for ${mddname}.mdh against ${mddname}.mdd' > \$@"
	echo
	echo "${mddname}.mdh: ${modhdeps} ${headers} ${hdrdeps} ${mddname}.mdhi"
	echo "	@\$(MAKE) -f \$(makefile) \$(MAKEDEFS) ${mddname}.mdh.tmp"
	echo "	@mv -f ${mddname}.mdh.tmp ${mddname}.mdh"
	echo "	@echo \"Updated \\\`${mddname}.mdh'.\""
	echo
	echo "${mddname}.mdh.tmp:"
	echo "	@( \\"
	echo "	    echo '#ifndef have_${q_name}_module'; \\"
	echo "	    echo '#define have_${q_name}_module'; \\"
	echo "	    echo; \\"
	echo "	    echo '# ifndef IMPORTING_MODULE_${q_name}'; \\"
	echo "	    if test @SHORTBOOTNAMES@ = yes; then \\"
	echo "		echo '#  ifndef MODULE'; \\"
	echo "	    fi; \\"
	echo "	    echo '#   define boot_ boot_${q_name}'; \\"
	echo "	    echo '#   define cleanup_ cleanup_${q_name}'; \\"
	echo "	    echo '#   define features_ features_${q_name}'; \\"
	echo "	    echo '#   define enables_ enables_${q_name}'; \\"
	echo "	    echo '#   define setup_ setup_${q_name}'; \\"
	echo "	    echo '#   define finish_ finish_${q_name}'; \\"
	echo "	    if test @SHORTBOOTNAMES@ = yes; then \\"
	echo "		echo '#  endif /* !MODULE */'; \\"
	echo "	    fi; \\"
	echo "	    echo '# endif /* !IMPORTING_MODULE_${q_name} */'; \\"
	echo "	    echo; \\"
	if test -n "$moddeps"; then (
	    set x $q_moddeps
	    echo "	    echo '/* Module dependencies */'; \\"
	    for hdep in $modhdeps; do
		shift
		echo "	    echo '# define IMPORTING_MODULE_${1} 1'; \\"
		echo "	    echo '# include \"${hdep}\"'; \\"
	    done
	    echo "	    echo; \\"
	) fi
	if test -n "$headers"; then
	    echo "	    echo '/* Extra headers for this module */'; \\"
	    echo "	    for hdr in $headers; do \\"
	    echo "		echo '# include \"'\$\$hdr'\"'; \\"
	    echo "	    done; \\"
	    echo "	    echo; \\"
	fi
	if test -n "$proto"; then
	    echo "	    echo '# undef mod_import_variable'; \\"
	    echo "	    echo '# undef mod_import_function'; \\"
	    echo "	    echo '# if defined(IMPORTING_MODULE_${q_name}) &&  defined(MODULE)'; \\"
	    echo "	    echo '#  define mod_import_variable @MOD_IMPORT_VARIABLE@'; \\"
	    echo "	    echo '#  define mod_import_function @MOD_IMPORT_FUNCTION@'; \\"
	    echo "	    echo '# else'; \\"
	    echo "	    echo '#  define mod_import_function'; \\"
	    echo "	    echo '#  define mod_import_variable'; \\"
	    echo "	    echo '# endif /* IMPORTING_MODULE_${q_name} && MODULE */'; \\"
	    echo "	    for epro in \$(EPRO_${mddname}); do \\"
	    echo "		echo '# include \"'\$\$epro'\"'; \\"
	    echo "	    done; \\"
	    echo "	    echo '# undef mod_import_variable'; \\"
	    echo "	    echo '# define mod_import_variable'; \\"
	    echo "	    echo '# undef mod_import_variable'; \\"
	    echo "	    echo '# define mod_import_variable'; \\"
	    echo "	    echo '# ifndef mod_export'; \\"
	    echo "	    echo '#  define mod_export @MOD_EXPORT@'; \\"
	    echo "	    echo '# endif /* mod_export */'; \\"
	    echo "	    echo; \\"
	fi
	echo "	    echo '#endif /* !have_${q_name}_module */'; \\"
	echo "	) > \$@"
	echo
	echo "\$(MODOBJS_${mddname}) \$(MODDOBJS_${mddname}): ${mddname}.mdh"
	sed -e '/^ *: *<< *\\Make *$/,/^Make$/!d' \
	    -e 's/^ *: *<< *\\Make *$//; /^Make$/d' \
	    < $top_srcdir/$the_subdir/${mddname}.mdd
	echo

    done

    if test -n "$remote_mdhs$other_mdhs$remote_exports$other_exports$remote_modules$other_modules"; then
	echo "##### ===== DEPENDENCIES FOR REMOTE MODULES ===== #####"
	echo
	for mdh in $remote_mdhs; do
	    echo "$mdh: FORCE"
	    echo "	@cd @%@ && \$(MAKE) \$(MAKEDEFS) @%@$mdh"
	    echo
	done | sed 's,^\(.*\)@%@\(.*\)@%@\(.*\)/\([^/]*\)$,\1\3\2\4,'
	if test -n "$other_mdhs"; then
	    echo "${other_mdhs}:" | sed 's,^ ,,'
	    echo "	false # should only happen with make -n"
	    echo
	fi
	for export in $remote_exports; do
	    echo "$export: FORCE"
	    echo "	@cd @%@ && \$(MAKE) \$(MAKEDEFS) @%@$export"
	    echo
	done | sed 's,^\(.*\)@%@\(.*\)@%@\(.*\)/\([^/]*\)$,\1\3\2\4,'
	if test -n "$other_exports"; then
	    echo "${other_exports}:" | sed 's,^ ,,'
	    echo "	false # should only happen with make -n"
	    echo
	fi
	for mdll in $remote_modules; do
	    echo "$mdll: FORCE"
	    echo "	@cd @%@ && \$(MAKE) \$(MAKEDEFS) @%@$mdll"
	    echo
	done | sed 's,^\(.*\)@%@\(.*\)@%@\(.*\)/\([^/]*\)$,\1\3\2\4,'
	if test -n "$other_modules"; then
	    echo "${other_modules}:" | sed 's,^ ,,'
	    echo "	false # should only happen with make -n"
	    echo
	fi
    fi

    echo "##### End of ${the_makefile}.in"

    exec >&3 3>&-

fi

if $second_stage ; then
    trap "rm -f $the_subdir/${the_makefile}; exit 1" 1 2 15

    ${CONFIG_SHELL-/bin/sh} ./config.status \
	--file=$the_subdir/${the_makefile}:$the_subdir/${the_makefile}.in ||
    exit 1
fi

exit 0
