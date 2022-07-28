# Contributing to ravynOS

Thank you for considering contributing to the ravynOS project! It's people like you who will make this project thrive.

Following these guidelines helps communicate that you respect the time of the project developers. In return, they should reciprocate that respect in addressing your issue, assessing changes, and helping you finalize your Pull Requests.

### Code of Conduct

We expect everyone interacting with this project to adhere to the [Contributor Covenant](CODE_OF_CONDUCT.md).

### How can I contribute?

As an open-source project, we love receiving contributions from our community - you! There are so many ways to help out, from writing documentation or tutorials, testing and submitting bug reports, creating art assets, answering user questions, editing wiki pages, and writing code in the project itself.

As of July 2022, these are the top priorities:
* Ongoing work on the desktop shell: WindowServer, LoginWindow, SystemUIServer (implementing MenuExtras, StatusItems), Dock, Filer
* Continue updating the Cocoa frameworks to add missing features and update existing ones
* Help convert everything to Mach-O: remove ELF assumptions, port Apple's dyld, write a better kernel image activator
* Replace CoreFoundation with the one from Swift 

Some other ideas for code contributions can be found on our [Project Ideas](https://wiki.ravynos.com/wiki/Project_Ideas) page or by looking through the [open issues](https://github.com/ravynsoft/ravynos/issues).

The skills we need immediately are: *C/C++, Objective-C, x86-64 assembly, linker/toolchain, kernel programming, OpenGL graphics programming, Wayland development*.


### Join the community

Join us on Discord, Matrix or IRC (see [README.md](README.md)) to meet other users and developers. We're also [ravynsoft](https://twitter.com/ravynsoft) on Twitter.

### Reporting Issues

If you have found a security issue, please _do not_ open a GitHub issue. Please email zoe@pixin.net ([PGP key](https://pixin.net/zoe.asc)) instead.

For everything else, please first check the [open issues](https://github.com/ravynsoft/ravynos/issues) to make sure it doesn't already exist. If it does, you can add a "me too" comment with additional information or give it a "+1" response to bump the priority. If not, please log a new issue giving as much detail as you can how to reproduce the problem.

### Ground rules

Please don't submit general questions or support requests to the issue tracker. There are [discussion boards](https://github.com/ravynsoft/ravynos/discussions) for that, or ask on the Discord, Matrix or IRC channels.

Follow the code of conduct.

The MIT and BSD 2-clause licenses are preferred for any new code.

If you have _ever_ seen Apple's proprietary code (not APSL-licensed or other open-source stuff) then you may *not* contribute code to ravynOS. Sorry :( However, you may be able to contribute in other ways!

Have fun and do great things!
