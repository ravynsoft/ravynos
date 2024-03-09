# Turbo

Turbo is an experimental text editor for the terminal, based on the [Scintilla](https://www.scintilla.org/index.html) code editing component by Neil Hodgson and the [Turbo Vision](https://github.com/magiblot/tvision) application framework.

It was created to demonstrate new features in Turbo Vision. In particular, it has served as testing ground for Turbo Vision's Unicode capabilities.

As a text editor, Turbo aims at being intuitive and easy to use. Usability and productivity are its two other major objectives, although it has not got that far yet. And only for being a Turbo Vision application, it offers a vintage look and feel.

The original location of this project is https://github.com/magiblot/turbo.

![Turbo](https://user-images.githubusercontent.com/20713561/89552632-b7053380-d80c-11ea-92e0-a8c30f80cd49.png)

## Downloads

* Unix systems: you'll have to build Turbo yourself. You may follow the build instructions in the next section.
* Windows: you can find up-to-date binaries in the [Actions](https://github.com/magiblot/turbo/actions?query=branch:master+event:push) page. Click on the first successful workflow (with a green tick) in the list. At the bottom of the workflow page, as long as you have logged in to GitHub, you'll find an *Artifacts* section with the following files:
    * `turbo-x86.zip`: 32-bit executable built with MSVC. Windows Vista or later required.
    * `turbo-x64.zip`: 64-bit executable built with MSVC. x64 Windows Vista or later required.

## Building

First of all, you should clone this repository along its submodules with the `--recursive` option of `git clone`.

Then, make sure the following dependencies are installed:

* CMake.
* A compiler supporting C++17.
* `libncursesw` (note the 'w') (Unix only).

Additionally, you may also want to install these optional dependencies:

* `libmagic` for better recognition of file types (Unix only).
* `libgpm` for mouse support on the linux console (Linux only).
* `xsel`, `xclip` and/or `wl-clipboard` for system clipboard integration (Unix only, except macOS).

Turbo can be built with the following commands:

```sh
cmake . -DCMAKE_BUILD_TYPE=Release && # Or 'RelWithDebInfo', or 'MinSizeRel', or 'Debug'.
cmake --build .
```

The above will generate the `turbo` binary.

<details>
<summary><b>Detailed build instructions for Ubuntu 20.04</b></summary>

```sh
sudo apt update && sudo apt upgrade
sudo apt install build-essential cmake gettext-base git libgpm-dev libmagic-dev libncursesw5-dev xsel
git clone --recursive https://github.com/magiblot/turbo.git
cd turbo
cmake . -DCMAKE_BUILD_TYPE=Release
cmake --build . -- -j$(nproc) # Build Turbo.
sudo cp turbo /usr/local/bin/ # Install (optional).
```
</details>
<details>
<summary><b>Detailed build instructions for Ubuntu 18.04</b></summary>

```sh
sudo apt update && sudo apt upgrade
sudo apt install build-essential cmake g++-8 gettext-base git libgpm-dev libmagic-dev libncursesw5-dev xsel
git clone --recursive https://github.com/magiblot/turbo.git
cd turbo
CXX=g++-8 cmake . -DCMAKE_BUILD_TYPE=Release
cmake --build . -- -j$(nproc) # Build Turbo.
sudo cp turbo /usr/local/bin/ # Install (optional).
```
</details>

## Usage

### From the command line

* **turbo** [*file*...]

In order to open several files in a directory tree you should use wildcards or subcommands, if they are supported by your command shell. For example, in Unix:

```sh
# Open all .c and .h files in the current directory and its subdirectories
turbo `find . -type f -name '*.c' -o -name '*.h'`
# Open all files in the current directory and its subdirectories, excluding executables and hidden files or directories
turbo `find . -type f \! -executable \! -path '*/.*'`
```

### In-app

As said earlier, Turbo has been designed to be intuitive. So you probably already know how to use it!

Some keybindings are:

* `Ctrl+C`/`Ctrl+Ins`: copy.
* `Ctrl+V`/`Shift+Ins`: paste.
* `Ctrl+X`/`Shift+Del`: cut.
* `Ctrl+Z`, `Ctrl+Y`: undo/redo.
* `Tab`, `Shift+Tab`: indent/unindent.
* `Ctrl+E`: toggle comment.
* `Ctrl+A`: select all.
* `Shift+Arrow`: extend selection.
* `Ctrl+F`: find.
* `Ctrl+R`: replace.
* `Ctrl+G`: go to line.
* `Ctrl+Back`/`Alt+Back`, `Ctrl+Del`: erase one word left/right.
* `Ctrl+Left`/`Alt+Left`, `Ctrl+Right`/`Alt+Right`: move one word left/right.
* `Ctrl+Shift+Up`/`Alt+Shift+Up`, `Ctrl+Shift+Down`/`Alt+Shift+Down`: move selected lines up/down.
* `Ctrl+N`: create new document.
* `Ctrl+O`: "open file" dialog.
* `Ctrl+S`: save document.
* `Ctrl+W`: close focused document.
* `F6`, `Shift+F6`: next/previous document (in MRU order).
* `Ctrl+Q`/`Alt+X`: exit the application.

In environments with extended keyboard support (e.g. the Linux console, Windows or Kitty ≥ 0.20.0), the following key shortcuts may also work:

* `Ctrl+Shift+Z`: redo.
* `Ctrl+Tab`/`Alt+Tab`, `Ctrl+Shift+Tab`/`Alt+Shift+Tab`: next/previous document (in MRU order).
* `Shift+Enter`: find previous (in the "find" text box).
* `Ctrl+/`/`Ctrl+_`: toggle comment.

Support for these key combinations may vary among terminal applications, but any issue on this should be reported to [Turbo Vision](https://github.com/magiblot/tvision/issues) instead.

### Clipboard support

See the Turbo Vision [documentation](https://github.com/magiblot/tvision#clipboard).

## Features

Scintilla has [lots of features](https://www.scintilla.org/ScintillaDoc.html), of which Turbo only offers a few. Making more of them available is just a matter of time, so contributions are welcome.

Below is a TO-DO list of features I would like to implement in Turbo:

- [x] Several files open at the same time.
- [x] Line numbers.
- [x] Word wrap.
- [x] Suspend to shell.
- [x] Unicode in documents (in particular, UTF-8).
- [x] Double-width characters.
- [x] Opening binary files without freaking out.
- [x] List of open documents in MRU order.
- [x] Tree view of open documents.
- [x] Tree view sorted alphabetically.
- [x] Case-insensitive search.
- [x] Find as you type.
- [x] Replace.
- [x] Go to line.
- [ ] List of recently opened files.
- [x] Remove trailing whitespaces on save.
- [x] Ensure newline at end of file.
- [ ] Detect open files modified on disk.
- [ ] Persistent configuration.
- [ ] Keybinding customization.
- [ ] Color scheme customization.
- [x] Syntax highlighting for some languages (C/C++, Rust, Python, JavaScript, Make, Bash, Ruby, JSON, YAML, HTML, INI).
- [ ] Syntax highlighting for the rest of [languages supported by Scintilla](https://github.com/RaiKoHoff/scintilla/blob/master/include/SciLexer.h).
- [x] Comment toggling.
- [x] Brace match highlighting.
- [ ] VIM input mode.
- [ ] Localization.
- [x] Integration with the system clipboard.
- [x] Unicode in dialog text boxes (this depends on Turbo Vision).
- [ ] True Color support (this depends on Turbo Vision).
