# cpcxfer

Cmd line tool to transfer files to and from [M4 board](http://www.spinpoint.org/2016/12/31/m4-recap-2016/) via WiFi.

## Commands


| Command  | 	Effect |
|----------|-----------|
| `xfer -u ipaddr file path opt`   | Upload file, opt 0: no header add, 1: add ascii header, 2: add binary header (specify start and exec addr. in hex) |
| `xfer -d ipaddr file path opt`   | Download file, opt 0: leave header, 1: remove header |
| `xfer -f ipaddr file slot name`  | Upload rom |
| `xfer -x ipaddr path+file` 	 | Execute file on CPC |
| `xfer -y ipaddr local_file`      | Upload file on CPC and execute it immediatly (the sd card must contain folder '/tmp') |
| `xfer -s ipaddr`	         | Reset CPC |
| `xfer -r ipaddr` 	         | Reboot M4 |


## Usage:

Example : If your m4 has IP number 192.168.1.11, the following code will run zynaps from given directory.

`xfer -x 192.168.1.11 /games/zynaps/zynapsex.bas`

Want to leave it again you could reset the cpc by:

`xfer -s 192.168.1.11`


In a development environment the idea is you ie. do the following to your makefile :

`xfer -u 192.168.1.11 mycode.bin / 0` (to upload the file after compilation, additional files could be uploaded too)

`xfer -x 192.168.1.11 /mycode.bin` (to execute it)

Remote files can be basic/binary or SNA (emulator snapshot format).



If you are deving roms, they can be remotely uploaded too :

`xfer -f 192.168.1.11 yancc.rom 3 "YANCC"` (upload to slot 3)

`xfer -f 192.168.1.11 launcher.rom 4 "The Launcher"` (upload to slot 4)

`xfer -r 192.168.1.11` (Reboot M4 for changes to take effect) 



## Python version

A python client is also provided in `pyxfer.py`. It can be used as it is is or as a library for another application.
It has globally the same arguments than the native version. 
Here are some differences:

 - It is able to create directories or remove files and directories.
 - The arguments for downloading a file are slightly different (full path instead of filename + path).
 - The managmeent of headers has still not be coded; will be done if requested


Here is the documentation of this version:

~~~
Python xfer.

Usage:
    pyxfer -s <ip>
    pyxfer -r <ip>
    pyxfer -u <ip> <file> [<path>] [<opt>]
    pyxfer -d <ip> <cpcfile> [<opt>]
    pyxfer -x <ip> <cpcfile>
    pyxfer -y <ip> <file> [<opt>]
    pyxfer -m <ip> <folder>
    pyxfer --cd <ip> <cpcfolder>
    pyxfer --rm <ip> <cpcfile>
    pyxfer --delRom <ip> <romid>
    pyxfer -f <ip> <file> <romid> <name>
    pyxfer -p <ip>
    pyxfer --ls <ip> [<folder>]


Options:
    -s      Reset CPC.
    -r      Reboot M4.
    -u      Upload a file on the SD card of the M4.
    -d      Download a file from the SD card of the M4.
    -x      Execute the program on the SD card of the M4.
    -y      Upload and execute the program on the /tmp of SD card of the M4.
    -m      Create directory on the SD card.
    --cd    Change current directory in CPC.
    --rm    Delete file or empty directory on CPC.
    -f      Install ROM <file> at position <romid>.
    --delRom    Delete the ROM.
    -p      Put CPC in pause.
    --ls    List files on CPC.
~~~
