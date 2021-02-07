#!/bin/bash

# Copyright (C) 2014 Free Software Foundation, Inc.
#
# Author: Ivan Vucica <ivan@vucica.net>
#
#  This file is part of the GNUstep Makefile Package.
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 3
#  of the License, or (at your option) any later version.
#   
#  You should have received a copy of the GNU General Public
#  License along with this library; see the file COPYING.
#  If not, write to the Free Software Foundation,
#  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

if [ -z "${1}" ] ; then
  echo "usage: "$(basename "${0}")" [source_code_directory]"
  exit 1
fi

cd ${1} && destination="$(pwd)" && cd -
destination="${destination}"/debian
if mkdir -p "${destination}" ; then : else exit 1 ; fi
rm "${destination}"/control 2> /dev/null
rm "${destination}"/changelog 2> /dev/null

deb_lowercase_package_name=$(echo ${PACKAGE_NAME} | sed -e 's/\(.*\)/\L\1/')
if [ ! -z "${SVNPREFIX}" ] ; then
  svn_path="${SVNPREFIX}"
fi
if [ ! -z "${SVN_BASE_URL}" ] && [ ! -z "${SVN_MODULE_NAME}" ] ; then
  svn_path=${SVN_BASE_URL}/${SVN_MODULE_NAME}
fi
default_distribution="unstable"
distrib_id="$(grep DISTRIB_ID /etc/lsb-release | sed 's/DISTRIB_ID=\(.*\)$/\1/')"
if [[ "${distrib_id}" == "Ubuntu" ]] ; then
  default_distribution=$(grep DISTRIB_CODENAME /etc/lsb-release | sed 's/DISTRIB_CODENAME=//')
fi

if true ; then
  # Forcing 'any' as we'd like Launchpad to build packages for all architectures.
  # DEB_ARCHITECTURE can still be overridden.
  target_arch=any
else
  target_arch=${GNUSTEP_TARGET_CPU:-any}
  if [[ "${target_arch}" == "i686" ]] ; then
    target_arch=i386
  elif [[ "${target_arch}" == "x86_64" ]] ; then
    target_arch=amd64
  fi
fi

PACKAGE_VERSION=${PACKAGE_VERSION:-${VERSION}}
DEB_SOURCE=${DEB_SOURCE:-${deb_lowercase_package_name}}
DEB_PACKAGE=${DEB_PACKAGE:-${deb_lowercase_package_name}}
DEB_ARCHITECTURE=${DEB_ARCHITECTURE:-${target_arch}} #$(shell (/bin/bash -c "$(CC) -dumpmachine | sed -e 's,\\([^-]*\\).*,\\1,g'"))}
DEB_SECTION=${DEB_SECTION:-gnustep}
DEB_PRIORITY=${DEB_PRIORITY:-optional}
DEB_VCS_SVN=${DEB_VCS_SVN:-${svn_path}}
DEB_VERSION=${DEB_VERSION:-${TARBALL_VERSION:-${PACKAGE_VERSION}}}
if [ -z "${DEB_BUILD_DEPENDS}" ] ; then
DEB_BUILD_DEPENDS="debhelper (>= 9), cdbs"
else
DEB_BUILD_DEPENDS="${DEB_BUILD_DEPENDS}, debhelper (>= 9), cdbs"
fi
if [ ! -z "${DEB_DEPENDS}" ]; then
DEB_DEPENDS=", ${DEB_DEPENDS}"
fi
DEB_DEPENDS='${shlibs:Depends}, ${misc:Depends}'" ${DEB_DEPENDS}"

DEB_DISTRIBUTION=${DEB_DISTRIBUTION:-${default_distribution}}

# DEB_VERSION_SUFFIX intentionally unset.

# Attempt to extract information from a .spec or a .spec.in file.
if which python > /dev/null ; then
FN=$(python - << _EOF

import tempfile
import sys

def process_specfile(specfilename):
  description_mode=False
  with open(specfilename) as specfile:
    with tempfile.NamedTemporaryFile(delete=False) as tf:

      print tf.name
      vars={}

      for line in specfile.readlines():
        line=line.rstrip()
        for var in vars:
          line=line.replace('%{' + var + '}', vars[var])

        if description_mode:
          if len(line.lstrip()) > 0 and line.lstrip()[0] == '#':
            continue
          if len(line.lstrip()) > 0 and line.lstrip()[0] == '%':
            description_mode=False
            continue
          tf.write("RPM_DESCRIPTION=\"\${RPM_DESCRIPTION}%s\\n\"\n" % line.replace('\\'', '\\\\\\'').replace('\\"', '\\\\\\"').replace('(', '\\(').replace(')', '\\)'))
          continue

        if not len(line):
          continue
        components=line.split(':')
        if len(components)>=2:
          key=components[0]
          value=':'.join(components[1:]).lstrip()
          if key=="Name":
            tf.write("DEB_PACKAGE=\${DEB_PACKAGE:-%s}\n" % value)
            tf.write("DEB_SOURCE=\${DEB_SOURCE:-%s}\n" % value)
          elif key=="Release":
            tf.write("DEB_VERSION_SUFFIX=\${DEB_VERSION_SUFFIX:-%s}\n" % value)
          elif key=="Source":
            # Source URL is not mappable to anything useful.
            # Possibly only includable in README.Debian.
            pass
          elif key=="License":
            tf.write("DEB_LICENSE=\${DEB_LICENSE:-%s}\n" % value)
          elif key=="Copyright":
            # Seems to do the same as License?
            pass
          elif key=="Group":
            # Ignore; not easily mappable, plus Debian has separate 
            # 'gnustep' section.
            pass
          elif key=="Summary":
            # First line of 'Description' field
            tf.write("RPM_DESCRIPTION=\"%s\n\"\n" % value)
          elif key=="Packager":
            tf.write("DEB_MAINTAINER=\${DEB_MAINTAINER:-\"%s\"}\n" % value)
          elif key=="Vendor":
            # Ignore; not useful anywhere in .deb
            pass
          elif key=="URL":
            tf.write("DEB_HOMEPAGE=\${DEB_HOMEPAGE:-%s}\n" % value)
          elif key=="Requires":
            # We support only comma separated dependencies (to simplify version handling).
            # Also, version mapping is very trivial.
            rpmdeps = value.split(',')
            debdeps = []
            for rpmdep in rpmdeps:
              rpmdepcomponents = rpmdep.split(' ')
              debdep = rpmdepcomponents[0]
              if len(rpmdepcomponents) > 1:
                debdep += ' (%s)' % ' '.join(rpmdepcomponents[1:])
              debdeps.append(debdep)
            tf.write("if [ -z \\"\${DEB_DEPENDS}\\" ] ; then\n")
            tf.write("  DEB_DEPENDS=\"%s\"\n" % ', '.join(debdeps))
            tf.write("else\n")
            tf.write("  DEB_DEPENDS=\"\${DEB_DEPENDS}, %s\"\n" % ', '.join(debdeps))
            tf.write("fi\n")
          elif key=="Provides":
            tf.write("if [ -z \\"\${DEB_PROVIDES}\\" ] ; then\n")
            tf.write("  DEB_PROVIDES=\"%s\"\n" % value)
            tf.write("else\n")
            tf.write("  DEB_PROVIDES=\"\${DEB_PROVIDES}, %s\"\n" % value)
          elif key=="Version":
            tf.write("DEB_VERSION=\${DEB_VERSION:-%s}\n" % value)
        else:
          if line == "%description":
            description_mode = True
          elif line.startswith('%define'):
            segs=line[len('%define')+1:].lstrip().replace('\\t', ' ').split(' ')
            segs=[seg.rstrip().lstrip() for seg in segs]
            vars[segs[0]] = ' '.join(segs[1:])
            

try:
  process_specfile('${DEB_PACKAGE}.spec')
except:
  try:
    process_specfile('${DEB_PACKAGE}.spec.in')
  except Exception as e:
    sys.stderr.write('could not process either ${DEB_PACKAGE}.spec or ${DEB_PACKAGE}.spec.in\n')
    raise
_EOF
)
if [ ! -z "${FN}" ] ; then
  . $FN
  DEB_DESCRIPTION="${DEB_DESCRIPTION:-${RPM_DESCRIPTION}}"
fi
fi

DEB_DESCRIPTION="${DEB_DESCRIPTION:-$(printf "Debian packaging for GNUstep based software ${PACKAGE_NAME}.\nThis package was built using gnustep-make.\n")}"

# Check that maintainer and package builder are set.
if [ -z "${DEB_MAINTAINER}" ] ; then
  echo "error: You must set DEB_MAINTAINER in GNUmakefile, in .spec file, or on command line."
  exit 1
fi
if [ -z "${DEB_PACKAGE_BUILDER}" ] ; then
  echo "error: You must set DEB_PACKAGE_BUILDER in GNUmakefile or on command line."
  echo "(It's recommended you set it on the command line, so other people"
  echo "don't accidentally claim you built their package.)"
  exit 1
fi

# Check some other fields
if [ -z "${DEB_PACKAGE}" ] ; then
  echo "error: Package name was not properly set in GNUmakefile."
  exit 1
fi
if [ -z "${DEB_VERSION}" ] ; then
  echo "error: Package version was not properly set in GNUmakefile."
  exit 1
fi
if [ ! -z "${DEB_VERSION_SUFFIX}" ] ; then
  DEB_VERSION=${DEB_VERSION}-${DEB_VERSION_SUFFIX}
  #ln -s ${destination}/../../${PACKAGE_NAME}-${VERSION}.orig.tar.gz ${destination}/../../${PACKAGE_NAME}-${DEB_VERSION}.orig.tar.gz
fi

echo ${destination}
# For documentation, see:
# https://www.debian.org/doc/debian-policy/ch-controlfields.html
echo "Source: " ${DEB_SOURCE} >> "${destination}"/control
echo "Maintainer:" ${DEB_MAINTAINER} >> "${destination}"/control
echo "Section:" ${DEB_SECTION} >> "${destination}"/control
echo "Priority:" ${DEB_PRIORITY} >> "${destination}"/control
if [ ! -z "${DEB_BUILD_DEPENDS}" ] ; then
  echo "Build-Depends:" ${DEB_BUILD_DEPENDS} >> "${destination}"/control
fi
echo "Standards-Version: 3.9.4" >> "${destination}"/control
if [ ! -z "${DEB_HOMEPAGE}" ] ; then
  echo "Homepage:" ${DEB_HOMEPAGE} >> "${destination}"/control
fi
if [ ! -z "${DEB_VCS_BROWSER}" ] ; then
  echo "Vcs-Browser:" ${DEB_VCS_BROWSER} >> "${destination}"/control
fi
if [ ! -z "${DEB_VCS_ARCH}" ] ; then
  echo "Vcs-Arch:" ${DEB_VCS_ARCH} >> "${destination}"/control
fi
if [ ! -z "${DEB_VCS_BZR}" ] ; then
  echo "Vcs-Bzr:" ${DEB_VCS_BZR} >> "${destination}"/control
fi
if [ ! -z "${DEB_VCS_CVS}" ] ; then
  echo "Vcs-Cvs:" ${DEB_VCS_CVS} >> "${destination}"/control
fi
if [ ! -z "${DEB_VCS_DARCS}" ] ; then
  echo "Vcs-Darcs:" ${DEB_VCS_DARCS} >> "${destination}"/control
fi
if [ ! -z "${DEB_VCS_GIT}" ] ; then
  echo "Vcs-Git:" ${DEB_VCS_GIT} >> "${destination}"/control
fi
if [ ! -z "${DEB_VCS_HG}" ] ; then
  echo "Vcs-Hg:" ${DEB_VCS_HG} >> "${destination}"/control
fi
if [ ! -z "${DEB_VCS_MTN}" ] ; then
  echo "Vcs-Mtn:" ${DEB_VCS_MTN} >> "${destination}"/control
fi
if [ ! -z "${DEB_VCS_SVN}" ] ; then
  echo "Vcs-Svn:" ${DEB_VCS_SVN} >> "${destination}"/control
fi

echo "" >> "${destination}"/control

echo "Package:" ${DEB_PACKAGE} >> "${destination}"/control
echo "Architecture:" ${DEB_ARCHITECTURE} >> "${destination}"/control

# Comma-separated lists of packages
if [ ! -z "${DEB_PRE_DEPENDS}" ] ; then
  echo "Pre-Depends:" ${DEB_PRE_DEPENDS} >> "${destination}"/control
fi
if [ ! -z "${DEB_DEPENDS}" ] ; then
  echo "Depends:" ${DEB_DEPENDS} >> "${destination}"/control
fi
if [ ! -z "${DEB_RECOMMENDS}" ] ; then
  echo "Recommends:" ${DEB_RECOMMENDS} >> "${destination}"/control
fi
if [ ! -z "${DEB_SUGGESTS}" ] ; then
  echo "Suggests:" ${DEB_SUGGESTS} >> "${destination}"/control
fi
if [ ! -z "${DEB_PROVIDES}" ] ; then
  echo "Provides:" ${DEB_PROVIDES} >> "${destination}"/control
fi
if [ ! -z "${DEB_REPLACES}" ] ; then
  echo "Replaces:" ${DEB_REPLACES} >> "${destination}"/control
fi

if [ ! -z "${DEB_ESSENTIAL}" ] ; then
  echo "Essential:" ${DEB_ESSENTIAL} >> "${destination}"/control
fi

echo "Description:" "$(echo "${DEB_DESCRIPTION}" | sed 's/^[\s]*$/./' | sed 's/\(.*\)/ \1/' | tail -c+2)" >> "${destination}"/control

###########

echo "${DEB_SOURCE} (${DEB_VERSION}) ${DEB_DISTRIBUTION}; urgency=low" >> "${destination}"/changelog
echo "" >> "${destination}"/changelog
echo "   * New build." >> "${destination}"/changelog
echo "" >> "${destination}"/changelog
echo "" >> "${destination}"/changelog
echo " -- ${DEB_PACKAGE_BUILDER}  $(date -R)" >> "${destination}"/changelog

##########

# Intentionally overwriting.
echo "9" > "${destination}"/compat
mkdir -p "${destination}"/source
echo "3.0 (quilt)" > "${destination}"/source/format

###########

# Intentionally overwriting.
cat > "${destination}"/rules << _EOF
#!/usr/bin/make -f
include /usr/share/cdbs/1/rules/debhelper.mk
ifneq (\$(wildcard configure),)
include /usr/share/cdbs/1/class/autotools.mk
else
include /usr/share/cdbs/1/class/makefile.mk
DEB_MAKE_INSTALL_TARGET := install DESTDIR=\$(CURDIR)/debian/${deb_lowercase_package_name}
endif

DEB_BUILD_PARALLEL = 1

DEB_CONFIGURE_EXTRA_FLAGS += ${DEB_CONFIGURE_EXTRA_FLAGS}
DEB_CONFIGURE_SCRIPT_ENV += ${DEB_CONFIGURE_SCRIPT_ENV}
DEB_DH_LINK_ARGS += ${DEB_DH_LINK_ARGS}

DEB_SHLIBS_ARGS_ALL += ${DEB_SHLIBS_ARGS_ALL}
DEB_SHLIBS_ARGS += ${DEB_SHLIBS_ARGS}
DEB_SHLIBS_INCLUDE += ${DEB_SHLIBS_INCLUDE}

DEB_MAKE_ENVVARS += BUILDING_DEB=1

export build : 
ifneq (${PACKAGE_NAME}, gnustep-make)
  GNUSTEP_MAKEFILES = \$(shell gnustep-config --variable=GNUSTEP_MAKEFILES)
  ifneq (\$(GNUSTEP_MAKEFILES), )
    DEB_MAKE_ENVVARS += \$(shell sh -c ". \$(GNUSTEP_MAKEFILES)/GNUstep.sh && env |grep GNUSTEP")
  else
    \$(error Failed to get GNUSTEP_MAKEFILES variable. Is gnustep-config properly installed?)
    exit 1
  endif
endif

_EOF

chmod 755 "${destination}"/rules

##########

if [ -e COPYING ] ; then
  cp COPYING ${destination}/copyright
fi
if [ -e LICENSE ] ; then
  cp LICENSE ${destination}/copyright
fi

