This file explains how to build the optional LDAP functionality of SUDO to
store /etc/sudoers information.  This feature is distinct from LDAP passwords.

For general sudo LDAP configuration details, see the sudoers.ldap manual that
comes with the sudo distribution.  A pre-formatted version of the manual may
be found in the sudoers.ldap.cat file.

The sudo binary compiled with LDAP support should be totally backward
compatible and be syntactically and source code equivalent to its
non LDAP-enabled build.

## LDAP philosophy

As times change and servers become cheap, an enterprise can easily have 500+
UNIX servers.  Using LDAP to synchronize Users, Groups, Hosts, Mounts, and
others across an enterprise can greatly reduce the administrative overhead.

In the past, sudo has used a single local configuration file, /etc/sudoers.
While the same sudoers file can be shared among machines, no built-in
mechanism exists to distribute it.  Some have attempted to workaround this
by synchronizing changes via CVS/RSYNC/RDIST/RCP/SCP and even NFS.

By using LDAP for sudoers we gain a centrally administered, globally
available configuration source for sudo.

For information on OpenLDAP, see https://www.openldap.org/.

## Definitions

Many times the word 'Directory' is used in the document to refer to the LDAP
server, structure, and contents.

Many times 'options' are used in this document to refer to sudoer 'defaults'.
They are one and the same.

## Build instructions

The simplest way to build sudo with LDAP support is to include the
`--with-ldap` option.

    $ ./configure --with-ldap

If your ldap libraries and headers are in a non-standard place, you will need
to specify them at configure time.  E.g.

    $ ./configure --with-ldap=/usr/local/ldapsdk

Sudo is developed using OpenLDAP but Netscape-based LDAP libraries
(such as those present in Solaris) and IBM LDAP are also known to work.

If special configuration was required to build an LDAP-enabled sudo,
let the sudo workers mailing list <sudo-workers@sudo.ws> know so
we can improve sudo.

## Schema Changes

You must add the appropriate schema to your LDAP server before it
can store sudoers content.

For OpenLDAP, there are two options, depending on how slapd is configured.

The first option is to copy the file schema.OpenLDAP to the schema
directory (e.g. /etc/openldap/schema).  You must then edit your
slapd.conf and add an include line the new schema, for example:

    # Sudo LDAP schema
    include	/etc/openldap/schema/sudo.schema

In order for sudoRole LDAP queries to be efficient, the server must index
the attribute 'sudoUser', for example:

    # Indices to maintain
    index	sudoUser	eq

After making the changes to slapd.conf, restart slapd.

The second option is only for OpenLDAP 2.3 and higher where slapd.conf
has been configured to use on-line configuration.  If your slapd.conf
file includes the line:

    database config

it should be possible to use the schema.olcSudo file.

You can apply schema.olcSudo using the ldapadd utility or another
suitable LDAP browser.  For example:

    # ldapadd -f schema.olcSudo -H ldap://ldapserver -W -x \
      -D cn=Manager,dc=example,dc=com

There is no need to restart slapd when updating on-line configuration.

For Netscape-derived LDAP servers such as SunONE, iPlanet, or Fedora Directory,
copy the schema.iPlanet file to the schema directory with the name 99sudo.ldif.

On Solaris, schemas are stored in /var/Sun/mps/slapd-\`hostname\`/config/schema/.
For Fedora Directory Server, they are stored in /etc/dirsrv/schema/.

After copying the schema file to the appropriate directory, restart
the LDAP server.

Finally, using an LDAP browser/editor, enable indexing by editing the
client profile to provide a Service Search Descriptor (SSD) for sudoers,
replacing example.com with your domain:

    serviceSearchDescriptor: sudoers: ou=sudoers,dc=example,dc=com

If using an Active Directory server, copy schema.ActiveDirectory
to your Windows domain controller and run the following command:

    ldifde -i -f schema.ActiveDirectory -c dc=X dc=example,dc=com

## Importing /etc/sudoers into LDAP

Importing sudoers is a two-step process.

1. Ask your LDAP Administrator where to create the ou=SUDOers container.
   For instance, if using OpenLDAP:
```
    dn: ou=SUDOers,dc=example,dc=com
    objectClass: top
    objectClass: organizationalUnit
    ou: SUDOers
```

(An example location is shown below).  Then use the cvtsudoers utility to
convert your sudoers file into LDIF format.
```
    # SUDOERS_BASE=ou=SUDOers,dc=example,dc=com
    # export SUDOERS_BASE
    # cvtsudoers -f ldif -o /tmp/sudoers.ldif /etc/sudoers
```

2. Import into your directory server.  The following example is for
   OpenLDAP.  If you are using another directory, provide the LDIF
   file to your LDAP Administrator.
```
    # ldapadd -f /tmp/sudoers.ldif -H ldap://ldapserver \
       -D cn=Manager,dc=example,dc=com -W -x
```

3.  Verify the sudoers LDAP data:
```
    # ldapsearch -b "$SUDOERS_BASE" -D cn=Manager,dc=example,dc=com -W -x
```

## Managing LDAP entries

Doing a one-time bulk load of your ldap entries is fine.  However what if you
need to make minor changes on a daily basis?  It doesn't make sense to delete
and re-add objects.  (You can, but this is tedious).

I recommend using any of the following LDAP browsers to administer your SUDOers.

 * GQ - The gentleman's LDAP client - Open Source - I use this a lot on Linux
   and since it is Schema aware, I don't need to create a sudoRole template.

    https://sourceforge.net/projects/gqclient/

 * phpQLAdmin - Open Source - phpQLAdmin is an administration tool,
   originally for QmailLDAP, that supports editing sudoRole objects
   in version 2.3.2 and higher.

    http://phpqladmin.com/

 * LDAP Browser/Editor - by Jarek Gawor - I use this a lot on Windows
   and Solaris.  It runs anywhere in a Java Virtual Machine including
   web pages.  You have to make a template from an existing sudoRole entry.

    http://pi.hv.pl/Gawor%20ldapbrowser/

 * Apache Directory Studio - Open Source - an Eclipse-based LDAP
   development platform.  Includes an LDAP browser, and LDIF editor,
   a schema editor and more.

    https://directory.apache.org/studio

  There are dozens of others, some Open Source, some free, some not.

## Configure your /etc/ldap.conf and /etc/nsswitch.conf

The /etc/ldap.conf file is meant to be shared between sudo, pam_ldap, nss_ldap
and other ldap applications and modules.  IBM Secureway unfortunately uses
the same file name but has a different syntax.  If you need to change where
this file is stored, re-run configure with the `--with-ldap-conf-file=PATH`
option.

See the "Configuring ldap.conf" section in the sudoers.ldap manual
for a list of supported ldap.conf parameters and an example ldap.conf

Make sure you sudoers_base matches the location you specified when you
imported the sudoers ldif data.

After configuring /etc/ldap.conf, you must add a line in the
/etc/nsswitch.conf file to tell sudo to look in LDAP for sudoers.
See the "Configuring nsswitch.conf" section in the sudoers.ldap
manual for details.  Sudo will use /etc/nsswitch.conf even if the
underlying operating system does not support it.  To disable nsswitch
support, run configure with the `--with-nsswitch=no` option.  This
will cause sudo to consult LDAP first and /etc/sudoers second,
unless the ignore_sudoers_file flag is set in the global LDAP options.

## Debugging your LDAP configuration

Enable debugging if you believe sudo is not parsing LDAP the way you think it
should.  Setting the 'sudoers_debug' parameter to a value of 1 shows moderate
debugging.  A value of 2 shows the results of the matches themselves.  Make
sure to set the value back to zero so that other users don't get confused by
the debugging messages.
