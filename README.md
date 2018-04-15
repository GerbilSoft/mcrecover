# GCN MemCard Recover

Version 0.2.1+

Copyright (c) 2012-2018 by David Korth.
Email: gerbilsoft@gerbilsoft.com

This program is licensed under the GNU General Public License v2.
See [doc/gpl-2.0.txt](doc/gpl-2.0.txt) for more information.

[![Travis Build Status](https://travis-ci.org/GerbilSoft/mcrecover.svg?branch=master)](https://travis-ci.org/GerbilSoft/mcrecover)
[![AppVeyor Build status](https://ci.appveyor.com/api/projects/status/grxmj5520cbw9wwd/branch/master?svg=true)](https://ci.appveyor.com/project/GerbilSoft/mcrecover/branch/master)
[![Coverity Scan Build Status](https://scan.coverity.com/projects/11187/badge.svg)](https://scan.coverity.com/projects/11187)

## What is GCN MemCard Recover?

GCN MemCard Recover is a program that can recover save files from
corrupted Nintendo GameCube memory cards. Usually, when GameCube
software indicates that a memory card is corrupted, only a few system
areas are affected; the data area, where the actual save files are
stored, is left intact. GCN MemCard Recover works by searching for
game descriptions in the data area of memory card dumps.

In addition, GCN MemCard Recover can recover save files if a memory
card is accidentally reformatted. Formatting a memory card only
erases the directory and block tables; as long as no new save files
are written, there's a high chance that the deleted files can be
recovered successfully.

## How to compile GCN MemCard Recover

GCN MemCard Recover requires the following packages:
* CMake 2.8.12 or later.
* Qt 5.5.0 or later. (5.7.0+ recommended)
* libpng (1.6 or later with APNG patch recommended)
* zlib (1.2.5 or later recommended)

If you do not have libpcre, libpng, or zlib, GCN MemCard Recover
includes copies of each of these libraries. They will be used
automatically.

On Debian and Ubuntu Linux systems, the following packages contain
the development headers required for compiling the program:
* build-essential
* cmake
* libpcre3-dev
* libpng12-dev
* zlib1g-dev
* qtbase5-dev
* qttools5-dev

NOTE: Most Linux distributions do not have the APNG patch in their packages
for libpng. If you want APNG support, you should configure GCN MemCard
Recover to use its built-in libpng by specifying `-DUSE_INTERNAL_PNG=ON`
on the `cmake` command line.

GCN MemCard Recover is known to compile with gcc5, MSVC 2010, and later
versions. Older versions may have issues.

To compile GCN MemCard Recover on Linux, run the following commands
in a terminal window:
$ tar xpfv mcrecover-0.2.1.tar.gz
$ cd mcrecover-0.2.1/
$ mkdir build/
$ cd build/
$ cmake ../
$ make
$ sudo make install

To compile GCN MemCard Recover on Windows, you will need to install
the following: (minimum versions)
* CMake 2.8.12
* Microsoft Visual C++ 2010, or MinGW-w64

## How to use GCN MemCard Recover

In order to recover files from a GCN memory card, you will first
need to dump the memory card image to a computer-readable format
using a Nintendo Wii. Two programs are available for this purpose:

* GCMM: GameCube Memory Card Manager - http://wiibrew.org/wiki/GCMM
* ctr-gcs-DacoTaco-Edition - http://wiibrew.org/wiki/Ctr-Gcs-DacoTaco-Edition

Installing and using these programs requires installing homebrew
software on the Wii console. A tutorial for setting up homebrew
is available at http://wiibrew.org/wiki/Homebrew_setup .

GCMM is more up-to-date and has more functionality, so GCMM is recommended.

You will need an SD card or USB mass storage device in order to
dump the memory card.

To dump a GameCube memory card to an SD card in GCMM:
1. Start GCMM from The Homebrew Channel.
2. Press A if you're using an SD card or B if you're using a
   USB mass storage device.
3. Select "Raw Backup". (L+Y on GCN controller, B+'-' on Wii Remote.)
4. Press A to dump the memory card in Slot A or B for Slot B.
   The memory card image will be dumped to the MCBACKUP directory
   on the SD card or USB mass storage device.
5. Press Start (GCN) or Home (Wii Remote) to exit.

Once the memory card image is dumped, you can open the image file
in GCN MemCard Recover:
1. Start GCN MemCard Recover.
2. Drag the memory card image file (e.g. 1019b_2013_07Jul_17_23-57-22.raw)
   onto the GCN MemCard Recover window.

GCN MemCard Recover will open the image file. If there were any save
files that were normally visible on the GameCube (e.g. the card was
reformatted and reused), they will appear in the list.

To search for files deleted and/or lost due to reformatting or file
system corruption, click the "Scan" button on the toolbar. GCN MemCard
Recover will scan each block of the memory card image to see if it can
find matches for known files. Once it's finished scanning, any files
it found will appear in the list highlighted in yellow.

Highlight the file(s) you want to extract, then click the "Save" button.
The file(s) will be saved in GCI format, which you can then restore onto
the memory card using GCMM.

In some cases, a recently-deleted file may be accessible by viewing an
alternate directory table. The GameCube Memory Card has two copies of
the Directory Table and Block Table. When saving a file, only one of
each of these tables is actually updated. This also counts for deleting
files. To change the active Directory Table and Block Table, click the
A or B buttons for Directory and Block Tables in the Memory Card pane.

In addition to extracting the save files, you can also extract the
banners and icons. To do this, click the Options menu, then check off
"Extract Banners" and/or "Extract Icons". The banners and icons will
be saved with the same name as the files, but with a different extension.

Banners and non-animated icons will always be saved in PNG format.

Animated icons can be extracted in one of five formats:
* APNG
* GIF
* PNG (file per frame): Each frame is saved in its own file.
* PNG (vertical strip): Each frame is stored one above another.
* PNG (horizontal strip): Each frame is stored next to each other.

Note that GIF support on Linux requires a copy of giflib v4.0 or later
to be installed. giflib v5.1.4 is included with the Windows version.

5. File Search Limitations

GCN MemCard Recover works by searching through the file data instead
of using the file system's directory tables. Unfortunately, this means
that file recovery has a few limitations:

1. If files are heavily fragmented, GCN MemCard Recover may not be able
   to successfully recover them. If this happens, the file will appear
   to be extracted successfully, but attempting to load the save file
   in the game will result in either a "file corrupted" error or a crash.

   GCN MemCard Recover does support verifying the checksums of some files,
   which is indicated by the icon in the rightmost column:
   * Checkmark: Checksum is known and is valid. File can be recovered.
   * X: Checksum is known and is invalid. File cannot be recovered
        using the current version of GCN MemCard Recover.
   * ?: Checksum is unknown. If you know the correct checksum algorithm
        for this file, please email me so I can add support for the
        algorithm in the next version.

   Note that checksums are currently only checked for "lost" files (that is,
   files found using the scanning algorithm). Files that are visible on
   the card normally are not checked, and hence won't show any icon in the
   rightmost column.

2. Some games don't store the game description in the first block of the
   file. This includes "Mario Kart: Double Dash!!" and "The Legend of Zelda:
   Twilight Princess". Due to limitations in the current scanning engine
   code, these files cannot be recovered yet. Support for these files will
   be added in a future version.

3. Games that have multiple slots but don't have a slot identifier in the
   comment section will all be restored with the same filename. This includes
   "Animal Crossing", so if e.g. multiple "Animal Crossing" save files are
   restored from the memory card, they will all have the same name, so you
   can't restore them to the same card. This will be fixed in a future version.

4. The included databases are far from complete. If you have save files for
   any games not currently supported by GCN MemCard Recover, please send me
   copies of the GCI files so I can add support for these games to the
   databases.

# UI Language Translations

GCN MemCard Recover has a fully-localizable user interface.
The following translations are included with v0.2.1:

* English (US): Base translation.
* English (GB): British English translation. Provided by Overlord.
* Español (CL): Spanish translation. Provided by Kevin López.
* Русский (RU): Russian translation. Provided by Egor.
* "1337 5p34k": Basic en_US to 1337 conversion.
  * The following converter was used in "basic leet" mode:
    http://www.robertecker.com/hp/research/leet-converter.php?lang=en

If you are a fluent speaker of English and another language and
would like to contribute a translation, please let me know.

# Additional Tools

GCN MemCard Recover includes a command-line utility called "gcbanner".
This utility lets you extract GameCube disc banner images from opening.bnr
files (both BNR1 and BNR2 format), as well as Wii save banner and icon
images from Wii save files (both Dolphin banner.bin and encrypted Wii
save formats).

For more information, see gcbanner's built-in help:
$ gcbanner --help

# External Resources

A basic tutorial for setting up MinGW-w64 on Windows is available at:
http://kemovitra.blogspot.com/2012/11/installing-mingw-w64-on-windows.html

Yet Another GameCube Documentation has useful documentation on the
structure of GameCube memory card images:
http://hitmen.c02.at/files/yagcd/yagcd/chap12.html

Finally, support for GCN MemCard Recover is available at:
* Sonic Retro: http://forums.sonicretro.org/index.php?showtopic=32621
* GBAtemp: http://gbatemp.net/threads/gcn-memcard-recover.349406/
* Email: gerbilsoft@gerbilsoft.com
* IRC: [irc://irc.badnik.zone/GensGS](irc://irc.badnik.zone/GensGS)

# Trademarks

Nintendo, GameCube, Wii, Mario Kart, Mario Kart: Double Dash!!,
The Legend of Zelda, The Legend of Zelda: Twilight Princess,
and Animal Crossing are either trademarks or registered trademarks
of Nintendo Co., Ltd.

Microsoft, Windows is a registered trademarks of Microsoft Corporation.

Qt is a registered trademark of The Qt Company.
