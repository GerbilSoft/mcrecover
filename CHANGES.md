# GCN MemCard Recover - Changes

Version 0.2.1+

Copyright (c) 2012-2016 by David Korth.
Email: gerbilsoft@gerbilsoft.com

This program is licensed under the GNU General Public License v2.
See [doc/gpl-2.0.txt](doc/gpl-2.0.txt) for more information.

## v0.3
(released 2018/??/??)

* Database statistics in this release: [UDPATE BEFORE RELEASE]
  * USA: 119 files
  * PAL: 59 files
  * JPN: 7 files
  * KOR: 0 files :(
  * Unlicensed: 1 file
  * Homebrew: 1 file

* mcrecover is now built with Qt5 instead of Qt4. Support for Qt4 has
  been dropped. Along with this, Qt's built-in PCRE wrapper class,
  QRegularExpression, is now used, so the included copy of PCRE has
  been removed.

* Added preliminary support for Dreamcast VMU card images. This is
  for full image dumps, e.g. 128 KB raw dumps.

* Added a save file editor for Sonic Adventure (Dreamcast) and Sonic
  Adventure DX (GameCube). [FIXME BEFORE RELEASE] Due to limitations,
  only save files present on full card dumps can be edited, and the
  edits cannot be saved at the moment. Based on SASave by @MainMemory,
  available at https://github.com/sonicretro/sa_tools .

* Animated icons can now be exported in GIF format. Note that if the
  animated icon has more than 256 colors, a dithering algorithm will
  be used. APNG is preferred, since it supports 32-bit color.

* Added taskbar button progress support for Windows 7.

* Enabled extra compiler security settings in default builds, including
  /sdl and /guard:cf in MSVC builds and -fstack-protector-strong in gcc
  builds.

* Updated the internal copies of zlib and libpng, including support for
  SSE2 optimizations in libpng where available.

* [MORE user-visible changes?]

* Removed gcbanner. This functionality is handled more in-depth by my
  [ROM Properties Page Shell Extension](https://github.com/GerbilSoft/rom-properties]),
  which includes a command line interface with more functionality.

* Fixed timezone offset issues with recovered files that have timestamp
  modifiers.

## v0.2.1
(released 2015/01/04)

* Database statistics in this release:
  * USA: 105 files
  * PAL: 59 files
  * JPN: 7 files
  * KOR: 0 files :(
  * Unlicensed: 1 file
  * Homebrew: 1 file

* Disabled High DPI mode on Windows. Region icons and GameCube icons and
  banners were not drawn with the correct scaling. Properly implementing
  High DPI support will probably require switching to Qt 5.

* Fixed an issue with the Windows build that caused variable expansions
  to be removed from the XML database files, resulting in unusable
  databases. This was caused by using cmake's CONFIGURE_FILE() to convert
  the files from UNIX line endings to DOS line endings.

* Use `unix2dos' to convert text and XML files from UNIX line endings to
  DOS line endings when building for Win32. If `unix2dos' isn't available,
  use the included program `miniu2d' to do the conversion instead.

* Fixed an issue where the preferred region was not correctly loaded on
  startup if it was set to USA. This could be worked around by selecting
  a different region and then switching back to USA.

* Eliminated the lag that happened after scanning for lost files on cards
  with lots of lost files (or with "Scan Used Blocks" enabled).

* Icon animations are now disabled when the window is minimized.
  This should reduce CPU usage and battery usage (on laptops).

## v0.2
(released 2014/02/08)

* Database statistics in this release:
  * USA: 83 files
  * PAL: 59 files
  * JPN: 4 files
  * KOR: 0 files :(
  * Unlicensed: 1 file
  * Homebrew: 1 file

* Stability has been improved. In particular, the custom model for
  the QTreeView now handles all required signals correctly.

* Some parts of the program now utilize C++ 2011 functionality.
  A compatibility header has been included for older compilers.

* The toolbar that was formerly located in the "Memory Card" view is
  now a window toolbar, and contains additional items, such as the
  "Preferred Region" selection.

* Added preliminary support for displaying scanning progress in the
  taskbar. Currently, only the D-Bus DockManager protocol is supported.
  Support for Ubuntu's Unity and Windows 7 will be added later.

* GcImage and Checksum functions have been split out into their own
  library, libgctools. This library depends on libpng but does not
  depend on Qt.

* Added support for the Qt translation system. Currently, translations
  for en_US, en_GB, es_CL, and "1337" are included.

* Fixed some corner cases with full memory card images and certain
  save files.

* Support for Japanese save files is improved.

* Added preliminary support for compiling with Microsoft Visual C++.
  The primary distributions will still be compiled with gcc/MinGW.

* Banners and icons can now be exported as image files. Banners and
  non-animated icons are always exported as PNG. Icons can be exported
  as APNG, PNG (file per frame), PNG (vertical strip), and
  PNG (horizontal strip).

* Added "Preferred Region" support. Some games don't have any way to
  determine the region by simply looking at the description, and in
  some cases, might be identical in every way other than the region
  code in the game ID. "Preferred Region" allows you to specify which
  region you want to prefer in the case that multiple save files in
  different regions are detected.

* Added support for multiple database files. The included databases
  are now split by region, e.g. USA, JPN, etc. Homebrew and Unlicensed
  titles are also contained in their own databases.

* Added a new utility "gcbanner". This utility can extract banner
  images and icons from GameCube BNR1 and BNR2 opening.bnr files
  as well as Wii save files (both raw banner.bin and encrypted save
  files). Animated icons can be extracted to the same formats
  supported by GCN MemCard Recover's icon extraction function.
  Banners and static icons are always extracted in PNG format.

* The current directory and block tables can now be switched on the fly.
  This may allow for easier recovery of files that were deleted in the
  GameCube file manager, as long as no other files have been saved or
  updated in the meantime.

* Added a "top-secret" easter egg. :)

# v0.1
(released 2013/07/21)

* Database statistics in this release:
  * USA: 59 files
  * Unlicensed: 1 file
  * Homebrew: 1 file

* Initial release.

* Supports extracting regular files from GCN memory card dumps.
  "Regular" files in this case means files that are normally
  visible in the system memory card manager.

* Supports searching for "lost" files in GCN memory card dumps.
  "Lost" files are files that aren't normally accessible because
  their file entries were removed, either due to corruption in
  the directory tables or accidental reformatting.

* Includes a database with 61 different file entries. This initial
  database only has NTSC-U games, and some file entries are not
  usable at the moment due to limitations in the scanning engine.

* Scanning engine cannot currently search for files whose descriptions
  are not stored in the first block of the file.

* Variable modification system does not yet support files with
  multiple slots whose slot IDs are not described in the file
  description field.
