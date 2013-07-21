GCN MemCard Recover
Version 0.1

Copyright (c) 2012-2013 by David Korth.
Email: gerbilsoft@gerbilsoft.com

Licensed under the GNU General Public License v2.
See Section X for more information.

================================================================

Table of Contents

1. What is GCN MemCard Recover?
2. System Requirements
3. How to compile GCN MemCard Recover
4. How to use GCN MemCard Recover
5. File Search Limitations
6. External Resources
X. License
Y. Trademarks

================================================================

1. What is GCN MemCard Recover?

GCN MemCard Recover is a program that can recover save files from
corrupted Nintendo GameCube memory cards. Usually when GameCube
software indicates that a memory card is corrupted, only a few system
areas are affected; the data area, where the actual save files are
stored, is left intact. GCN MemCard Recover works by searching for
game descriptions in the data area of memory card dumps.

In addition, GCN MemCard Recover can recover save files if a memory
card is accidentally reformatted. Formatting a memory card only
erases the directory and block tables; as long as no new save files
are written, there's a high chance that the deleted files can be
recovered successfully.

================================================================

2. System Requirements

GCN MemCard Recover has no specific minimum hardware requirements;
however, faster computers will obviously be able to scan through
memory card images faster. The program was developed on a system
with the following specifications:

ThinkPad T60p:
- Intel Core 2 Duo T7200 (2.0 GHz)
- 3.0 GB DDR2 SDRAM
- Gentoo Linux with Linux Kernel 3.10.0

The program will likely work well on systems with specifications as
low as Intel Pentium II with 256 MB RAM.

Supported operating systems:
- Microsoft Windows XP SP3, Vista SP1, 7
- Microsoft Windows Server 2003, 2008, 2008 R2
- Ubuntu Linux 10.04+ (i386, amd64)
- Gentoo Linux (i386, amd64)
- Other Linux 2.6.x / 3.x systems

Windows 2000 may work; however, the Qt runtime lists Windows XP SP3
as its oldest-supported Windows operating system. If you are
attempting to run GCN MemCard Recover on Windows 2000 and
encounter issues, please let me know.

Windows XP and Windows Vista are only supported with thier latest
service packs. Older service packs may work, but you may encounter
issues.

Windows 8 is not officially supported, though GCN MemCard Recover
should run as long as the Win32 subsystem is working.

Windows RT is not supported.

Mac OS X is not currently supported; however, support will be added
in a future version.

================================================================

3. How to compile GCN MemCard Recover

GCN MemCard Recover requires the following libraries:
- Qt 4.6.0 or later. (4.8.5+ recommended)
  - NOTE: Qt 5 is not currently supported.
- libpcre (8.33 or later recommended)
- cmake 2.6 or later. (2.8.10.2+ recommended)

On Ubuntu Linux systems, the following packages contain the
development headers required for compiling the program:
- build-essential
- libqt4-dev
- libpcre3-dev

GCN MemCard Recover has been compiled with gcc-4.4 and 4.8.
Other versions should work, though versions earlier than 4.4
may encounter problems.

To compile GCN MemCard Recover on Linux, run the following commands
in a terminal window:
$ tar xpfv mcrecover-0.1.tar.gz
$ cd mcrecover-0.1/
$ mkdir build/
$ cd build/
$ cmake ../
$ make
$ sudo make install

To compile GCN MemCard Recover on Windows, you will need a
MinGW-w64 compile environment. Setting up MinGW-w64 is beyond
the scope of this README. See External Resources for more
information on setting up MinGW-w64.

================================================================

4. How to use GCN MemCard Recover

In order to recover files from a GCN memory card, you will first
need to dump the memory card image to a computer-readable format
using a Nintendo Wii. Two programs are available for this purpose:

- GCMM: GameCube Memory Card Manager
  http://wiibrew.org/wiki/GCMM

- ctr-gcs-DacoTaco-Edition
  http://wiibrew.org/wiki/Ctr-Gcs-DacoTaco-Edition

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
system corruption, click the "Scan" button in the Memory Card Details
toolbar. GCN MemCard Recover will scan each block of the memory card
image to see if it can find matches for known files. Once it's finished
scanning, any files it found will appear in the list highlighted in
yellow.

Highlight the file(s) you want to extract, then click the "Save" button.
The file(s) will be saved in GCI format, which you can then restore onto
the memory card using GCMM.

================================================================

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
   - Checkmark: Checksum is known and is valid. File can be recovered.
   - X: Checksum is known and is invalid. File cannot be recovered
        using the current version of GCN MemCard Recover.
   - ?: Checksum is unknown. If you know the correct checksum algorithm
        for this file, please email me so I can add support for the
        algorithm in the next version.

   Note that checksums are only currently checked for "lost" files (that is,
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

4. The included database currently only has entries for NTSC-U titles.
   If you have save files for PAL, Japanese, or Korean GameCube games,
   or other USA games that aren't supported by GCN MemCard Recover yet,
   please send me copies of the GCI files so I can add support for these
   games to the database.

================================================================

6. External Resources

A basic tutorial for setting up MinGW-w64 on Windows is available at:
http://kemovitra.blogspot.com/2012/11/installing-mingw-w64-on-windows.html

Yet Another GameCube Documentation has useful documentation on the
structure of GameCube memory card images:
http://hitmen.c02.at/files/yagcd/yagcd/chap12.html

Finally, support for GCN MemCard Recover is available at:
- Sonic Retro: http://forums.sonicretro.org/index.php?showtopic=27687
  - [TODO: New topic on Retro]
- GBAtemp: http://gbatemp.net/threads/gcn-memcard-recover.349406/
- Email: gerbilsoft@gerbilsoft.com
- IRC: irc.badnik.net #retrotech

================================================================

X. License

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along *
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

================================================================

Y. Trademarks

Nintendo, GameCube, Wii, Mario Kart, Mario Kart: Double Dash!!,
The Legend of Zelda, The Legend of Zelda: Twilight Princess,
and Animal Crossing are either trademarks or registered trademarks
of Nintendo Co., Ltd.

ThinkPad is a registered trademark of Lenovo Group, Ltd.

Intel is a registered trademark of Intel Corporation.

Microsoft, Windows, Windows 2000, Windows XP, Windows Vista,
Windows 7, Windows Server 2003, Windows Server 2008,
Windows Server 2008 R2, Windows 8, and Windows RT are either
trademarks or registered trademarks of Microsoft Corporation.

Mac OS is a registered trademark of Apple, Inc.

Qt is a registered trademark of Digia PLC.

================================================================

EOF
