Contributing to Sudo
====================

Thank you for your interest in contributing to Sudo!  There are a
number of way you can help make Sudo better.

## Getting started

To get an overview of Sudo, see the [README.md](../README.md) file.
There are multiple ways to contribute, some of which don't require
writing a single line of code.

## Filing bug reports/issues

If you believe you have found a bug, you can either file a bug
report in the sudo bug database, https://bugzilla.sudo.ws/, or open
a [GitHub issue](https://github.com/sudo-project/sudo/issues),
whichever you find easier.  If you would prefer to use email,
messages may be sent to the [sudo-workers@sudo.ws mailing
list](https://www.sudo.ws/mailman/listinfo/sudo-workers) (public)
or to sudo@sudo.ws (private).

For sudo's security policy and how to report security issues, see
[SECURITY.md](SECURITY.md).

Please check [TROUBLESHOOTING.md](TROUBLESHOOTING.md) *before*
submitting a bug report.  When reporting bugs, be sure to include
the version of sudo you are using, the operating system and/or
distro that is affected, and, if possible, step-by-step instructions
to reproduce the problem.

## Making changes to Sudo

If you are interested in making changes to Sudo there are two main
work flows:

 * clone the [sudo repo](https://github.com/sudo-project/sudo), make
   your changes, and submit a Pull Request (PR).
   
 * send a diff with your changes to the [sudo-workers@sudo.ws mailing
   list](https://www.sudo.ws/mailman/listinfo/sudo-workers) to start
   a discussion.

In addition to the [GitHub repo](https://github.com/sudo-project/sudo),
there is also a [mercurial repo](https://www.sudo.ws/repos/sudo).

## sudo-workers mailing list

If you would like to discuss your changes before submitting a
PR, you may do so on the [sudo-workers@sudo.ws mailing
list](https://www.sudo.ws/mailman/listinfo/sudo-workers).
Otherwise, discussion can simply occur as part of the PR work flow.

## Fuzzing

Sudo uses the [oss-fuzz project](https://github.com/google/oss-fuzz.git)
to perform fuzzing.  Each commit to the _main_ branch will trigger
a short fuzzing run via the [CIFuzz
action](https://github.com/sudo-project/sudo/actions/workflows/main.yml).
The history of that action shows successful and failed fuzzing runs.

Longer fuzzing runs occur using the ClusterFuzz infrastructure.  These
fuzzing runs are longer than those used by CIFuzz.  A [public list of
failures](https://bugs.chromium.org/p/oss-fuzz/issues/list?q=sudoers)
is available.

For more information, see https://www.sudo.ws/security/fuzzing/.

## Translations

Sudo uses [GNU gettext](https://www.gnu.org/software/gettext/) for
its National Language Support (NLS).  Strings in sudo and related
programs are collected in `.pot` files that can be translated into
multiple languages.

Translations for sudo are coordinated by the [Translation
Project](https://translationproject.org).  If you would like to
contribute to Sudo's translations, please join a translation team
at the Translation Project instead of contributing a `.po` file
directly.  This will avoid duplicated work if there is already a
translation in progress.  If you would like to become a member of
a translation team, please follow the [instructions for
translators](https://translationproject.org/html/translators.html).

There are currently two translation domains: [one for the sudo
front-end](https://translationproject.org/domain/sudo.html) and a
[separate one for the sudoers module and related
utilities](https://translationproject.org/domain/sudoers.html).
