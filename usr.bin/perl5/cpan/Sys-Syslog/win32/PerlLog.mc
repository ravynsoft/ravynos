; // Sys::Syslog Message File 1.0.0

MessageIdTypedef = DWORD

SeverityNames = (
    Success         = 0x0:STATUS_SEVERITY_SUCCESS
    Informational   = 0x1:STATUS_SEVERITY_INFORMATIONAL
    Warning         = 0x2:STATUS_SEVERITY_WARNING
    Error           = 0x3:STATUS_SEVERITY_ERROR
)

LanguageNames = ( English = 0x0409:MSG00409 )
LanguageNames = ( French  = 0x040C:MSG0040C )


; // =================================================================
; // The following are facility name definitions

MessageId = 0x0001
SymbolicName = CAT_KERN
Language = English
Kernel
.
Language = French
Kernel
.

MessageId = 0x0002
SymbolicName = CAT_USER
Language = English
User
.
Language = French
User
.

MessageId = 0x0003
SymbolicName = CAT_MAIL
Language = English
Mail
.
Language = French
Mail
.

MessageId = 0x0004
SymbolicName = CAT_DAEMON
Language = English
Daemon
.
Language = French
Daemon
.

MessageId = 0x0005
SymbolicName = CAT_AUTH
Language = English
Auth
.
Language = French
Auth
.

MessageId = 0x0006
SymbolicName = CAT_SYSLOG
Language = English
Syslog
.
Language = French
Syslog
.

MessageId = 0x0007
SymbolicName = CAT_LPR
Language = English
LPR
.
Language = French
LPR
.

MessageId = 0x0008
SymbolicName = CAT_NEWS
Language = English
News
.
Language = French
News
.

MessageId = 0x0009
SymbolicName = CAT_UUCP
Language = English
UUCP
.
Language = French
UUCP
.

MessageId = 0x000a
SymbolicName = CAT_CRON
Language = English
Cron
.
Language = French
Cron
.

MessageId = 0x000b
SymbolicName = CAT_AUTHPRIV
Language = English
AuthPrivate
.
Language = French
AuthPrivate
.

MessageId = 0x000c
SymbolicName = CAT_FTP
Language = English
FTP
.
Language = French
FTP
.

MessageId = 0x000d
SymbolicName = CAT_LOCAL0
Language = English
Local0
.
Language = French
Local0
.

MessageId = 0x000e
SymbolicName = CAT_LOCAL1
Language = English
Local1
.
Language = French
Local1
.

MessageId = 0x000f
SymbolicName = CAT_LOCAL2
Language = English
Local2
.
Language = French
Local2
.

MessageId = 0x0010
SymbolicName = CAT_LOCAL3
Language = English
Local3
.
Language = French
Local3
.

MessageId = 0x0011
SymbolicName = CAT_LOCAL4
Language = English
Local4
.
Language = French
Local4
.

MessageId = 0x0012
SymbolicName = CAT_LOCAL5
Language = English
Local5
.
Language = French
Local5
.

MessageId = 0x0013
SymbolicName = CAT_LOCAL6
Language = English
Local6
.
Language = French
Local6
.

MessageId = 0x0014
SymbolicName = CAT_LOCAL7
Language = English
Local7
.
Language = French
Local7
.

; // Mac OS X specific facilities ------------------------------------

MessageId = 0x0015
SymbolicName = CAT_NETINFO
Language = English
NetInfo
.
Language = French
NetInfo
.

MessageId = 0x0016
SymbolicName = CAT_REMOTEAUTH
Language = English
RemoteAuth
.
Language = French
RemoteAuth
.

MessageId = 0x0017
SymbolicName = CAT_RAS
Language = English
RAS
.
Language = French
RAS
.

MessageId = 0x0018
SymbolicName = CAT_INSTALL
Language = English
Install
.
Language = French
Install
.

MessageId = 0x0019
SymbolicName = CAT_LAUNCHD
Language = English
Launchd
.
Language = French
Launchd
.

; //modern BSD specific facilities ----------------------------------

MessageId = 0x001a
SymbolicName = CAT_CONSOLE
Language = English
Console
.
Language = French
Console
.

MessageId = 0x001b
SymbolicName = CAT_NTP
Language = English
NTP
.
Language = French
NTP
.

MessageId = 0x001c
SymbolicName = CAT_SECURITY
Language = English
Security
.
Language = French
Sécurité
.

; // IRIX specific facilities ----------------------------------------

MessageId = 0x001d
SymbolicName = CAT_AUDIT
Language = English
Audit
.
Language = French
Audit
.

MessageId = 0x001e
SymbolicName = CAT_LFMT
Language = English
LogAlert
.
Language = French
LogAlert
.


; // =================================================================
; // The following are message definitions.

MessageId = 0x0080
SymbolicName = MSG_KERNEL
Language = English
Kernel message: %1
.
Language = French
Message du noyau : %1
.


MessageId = 0x0081
SymbolicName = MSG_USER
Language = English
User message: %1
.
Language = French
Message utilisateur : %1
.


MessageId = 0x0082
SymbolicName = MSG_MAIL
Language = English
Mail subsystem message: %1
.
Language = French
Message du sous-système de courrier : %1
.


MessageId = 0x0083
SymbolicName = MSG_DAEMON
Language = English
Message from a system daemon without separate facility value: %1
.
Language = French
Message d'un daemon sans catégorie spécifique : %1
.


MessageId = 0x0084
SymbolicName = MSG_AUTH
Language = English
Security/authorization message: %1
.
Language = French
Message de sécurite ou d'authorisation : %1
.


MessageId = 0x0085
SymbolicName = MSG_SYSLOG
Language = English
Message generated internally by syslogd: %1
.
Language = French
Message interne généré par le daemon syslogd : %1
.


MessageId = 0x0086
SymbolicName = MSG_LPR
Language = English
Line printer subsystem message: %1
.
Language = French
Message du sous-système d'impression : %1
.


MessageId = 0x0087
SymbolicName = MSG_NEWS
Language = English
USENET news subsystem message: %1
.
Language = French
Message du sous-système de nouvelles USENET : %1
.


MessageId = 0x0088
SymbolicName = MSG_UUCP
Language = English
UUCP subsystem message: %1
.
Language = French
Message du sous-système UUCP : %1
.


MessageId = 0x0089
SymbolicName = MSG_CRON
Language = English
Message generated by the clock daemons (cron and at): %1
.
Language = French
Message généré par les daemons d'exécution programmée (cron et at) : %1
.


MessageId = 0x008A
SymbolicName = MSG_AUTHPRIV
Language = English
Security or authorization private message: %1
.
Language = French
Message privé de sécurité ou d'authorisation : %1
.


MessageId = 0x008B
SymbolicName = MSG_FTP
Language = English
FTP daemon message: %1
.
Language = French
Message du daemon FTP : %1
.


MessageId = 0x008C
SymbolicName = MSG_LOCAL0
Language = English
Local message on channel 0: %1
.
Language = French
Message local sur le canal 0 : %1
.


MessageId = 0x008D
SymbolicName = MSG_LOCAL1
Language = English
Local message on channel 1: %1
.
Language = French
Message local sur le canal 1 : %1
.


MessageId = 0x008E
SymbolicName = MSG_LOCAL2
Language = English
Local message on channel 2: %1
.
Language = French
Message local sur le canal 2 : %1
.


MessageId = 0x008F
SymbolicName = MSG_LOCAL3
Language = English
Local message on channel 3: %1
.
Language = French
Message local sur le canal 3 : %1
.


MessageId = 0x0090
SymbolicName = MSG_LOCAL4
Language = English
Local message on channel 4: %1
.
Language = French
Message local sur le canal 4 : %1
.


MessageId = 0x0091
SymbolicName = MSG_LOCAL5
Language = English
Local message on channel 5: %1
.
Language = French
Message local sur le canal 5 : %1
.


MessageId = 0x0092
SymbolicName = MSG_LOCAL6
Language = English
Local message on channel 6: %1
.
Language = French
Message local sur le canal 6 : %1
.


MessageId = 0x0093
SymbolicName = MSG_LOCAL7
Language = English
Local message on channel 7: %1
.
Language = French
Message local sur le canal 7 : %1
.


; // Mac OS X specific facilities ------------------------------------

MessageId = 0x0094
SymbolicName = MSG_NETINFO
Language = English
NetInfo subsystem message: %1
.
Language = French
Message du sous-système NetInfo : %1
.


MessageId = 0x0095
SymbolicName = MSG_REMOTEAUTH
Language = English
Remote authentication or authorization message: %1
.
Language = French
Message d'authentification ou d'authorisation distante : %1
.


MessageId = 0x0096
SymbolicName = MSG_RAS
Language = English
Message generated by the Remote Access Service (VPN / PPP): %1
.
Language = French
Message généré par le Service d'Accès Distant (Remote Access Service) (VPN / PPP) : %1
.


MessageId = 0x0097
SymbolicName = MSG_INSTALL
Language = English
Installer subsystem message: %1
.
Language = French
Message du sous-système d'installation : %1
.


MessageId = 0x0098
SymbolicName = MSG_LAUNCHD
Language = English
Message generated by launchd, the general bootstrap daemon: %1
.
Language = French
Message généré par launchd, le daemon générique de démarrage : %1
.

; //modern BSD specific facilities ----------------------------------

MessageId = 0x0099
SymbolicName = MSG_CONSOLE
Language = English
Message for the console: %1
.
Language = French
Message pour la console : %1
.


MessageId = 0x009a
SymbolicName = MSG_NTP
Language = English
NTP subsystem message: %1
.
Language = French
Message du sous-système NTP : %1
.


MessageId = 0x009b
SymbolicName = MSG_SECURITY
Language = English
Security subsystem message (firewalling, etc.): %1
.
Language = French
Message du sous-système de sécurité (pare-feu, etc.) : %1
.


; // IRIX specific facilities ----------------------------------------

MessageId = 0x009c
SymbolicName = MSG_AUDIT
Language = English
Audit daemon message: %1
.
Language = French
Message du daemon d'audit NTP : %1
.


MessageId = 0x009d
SymbolicName = MSG_LFMT
Language = English
Logalert facility: %1
.
Language = French
Message de logalert : %1
.

