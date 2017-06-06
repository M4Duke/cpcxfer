# cpcxfer
Cmd line tool to transfer files to and from M4 board via WiFi
Command 	Effect
xfer -u ipaddr file path opt   - Upload file, opt 0: no header add, 1: add ascii header
xfer -d ipaddr file path opt   - Download file, opt 0: leave header, 1: remove header
xfer -f ipaddr file slot name  - Upload rom
xfer -x ipaddr path+file 	     - Execute file on CPC
xfer -y ipaddr local_file      - Upload file on CPC and execute it immediatly (the sd card must contain folder '/tmp')
xfer -s ipaddr 	               - Reset CPC
xfer -r ipaddr 	               - Reboot M4 

Usage:
Example : If your m4 has IP number 192.168.1.11, the following code will run zynaps from given directory.

xfer -x 192.168.1.11 /games/zynaps/zynapsex.bas

Want to leave it again you could reset the cpc by:

xfer -s 192.168.1.11


In a development environment the idea is you ie. do the following to your makefile :

xfer -u 192.168.1.11 mycode.bin / 0 (to upload the file after compilation, additional files could be uploaded too)

xfer -x 192.168.1.11 /mycode.bin (to execute it)

Remote files can be basic/binary or SNA (emulator snapshot format).

If you are deving roms, they can be remotely uploaded too :

xfer -f 192.168.1.11 yancc.rom 3 "YANCC" (upload to slot 3)

xfer -f 192.168.1.11 launcher.rom 4 "The Launcher" (upload to slot 4)

xfer -r 192.168.1.11 (Reboot M4 for changes to take effect) 
