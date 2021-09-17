# Kate

## Licensing

Contributions to Kate shall be licensed under LGPLv2+.

At the moment the following source files still have licensing issues:

### *.h

cullmann@altair:/local/cullmann/kf5/src/kate> find kate -name "*.h" -exec ../ninka/ninka.pl {} \; | grep -v LibraryGPLv2\+
kate/app/kateappcommands.h;LibraryGPLv2;1;1;4;2;0;Copyright,,1,-1,-1,-1,-1
kate/app/katemainwindow.h;LibraryGPLv2;1;1;4;2;0;Copyright,,1,-1,-1,-1,-1
kate/app/kateviewspace.h;LibraryGPLv2;1;1;4;2;0;Copyright,,1,-1,-1,-1,-1
kate/app/kateviewmanager.h;LibraryGPLv2;1;1;4;2;0;Copyright,,1,-1,-1,-1,-1
kate/app/katedocmanager.h;LibraryGPLv2;1;1;4;2;0;Copyright,,1,-1,-1,-1,-1

### *.cpp

cullmann@altair:/local/cullmann/kf5/src/kate> find kate -name "*.cpp" -exec ../ninka/ninka.pl {} \; | grep -v LibraryGPLv2\+
kate/app/kateappadaptor.cpp;LibraryGPLv2;1;1;4;2;0;Copyright,,1,-1,-1,-1,-1
kate/app/kateappcommands.cpp;LibraryGPLv2;1;1;4;2;0;Copyright,,1,-1,-1,-1,-1
kate/app/kateconfigdialog.cpp;LibraryGPLv2;1;1;4;2;0;Copyright,,1,-1,-1,-1,-1
kate/app/kateconfigplugindialogpage.cpp;LibraryGPLv2;1;1;4;2;0;Copyright,,1,-1,-1,-1,-1
kate/app/katemain.cpp;LibraryGPLv2;1;1;4;2;0;Copyright,,1,-1,-1,-1,-1
kate/app/katemainwindow.cpp;LibraryGPLv2;1;1;4;2;0;Copyright,,1,-1,-1,-1,-1
kate/app/katemwmodonhddialog.cpp;LibraryGPLv2;1;1;4;2;0;,1,-1,-1,-1,-1,Copyright
kate/app/katepluginmanager.cpp;LibraryGPLv2;1;1;4;2;0;Copyright,,1,-1,-1,-1,-1
kate/app/katerunninginstanceinfo.cpp;LibraryGPLv2;1;1;4;2;0;Copyright,,1,-1,-1,-1,-1
kate/app/katesavemodifieddialog.cpp;LibraryGPLv2;1;1;4;2;0;Copyright,,1,-1,-1,-1,-1
kate/app/katesession.cpp;LibraryGPLv2;1;1;4;2;0;Copyright,,1,-1,-1,-1,-1
kate/app/kateviewspace.cpp;LibraryGPLv2;1;1;4;2;0;Copyright,,1,-1,-1,-1,-1
kate/app/kateviewmanager.cpp;LibraryGPLv2;1;1;4;2;0;Copyright,,1,-1,-1,-1,-1
kate/app/katedocmanager.cpp;LibraryGPLv2;1;1;4;2;0;Copyright,,1,-1,-1,-1,-1
kate/app/kateapp.cpp;LibraryGPLv2;1;1;4;2;0;Copyright,,1,-1,-1,-1,-1
