#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""Python version of the XFER tool

"""

# imports
import requests
import os
from docopt import docopt


__doc__ = """
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
"""


class XFER(object):
    def __init__(self, ip):
        self._ip = ip



    def _getUrl(self, path):
        return "http://%s/%s" % (self._ip, path)
 
    
    def _getHeaders(self):
        return {"user-agent": "cpcxfer"}


    def _upload_file(self, path, destination, opt):
        assert os.path.isfile(path)

        if opt!=0:
            raise "Need to add an amsdos header"


        print "Upload " + path + " in " + destination
        url = self._getUrl("upload.html")
        r = requests.post(
            url,
            files={"upfile" : (
                    destination + "/" + os.path.basename(path),
                    open(path, 'rb'),
                    "Content-Type: application/octet-stream",
                    {'Expires': '0'}
                )}
        )

        assert r.status_code == 200


    def _upload_dir(self, path, destination, opt):
        assert os.path.isdir(path)

        removed_base = path[:-len(os.path.basename(path))]


        def _treat_dir(current_dir):
            current_base = (destination + "/" + current_dir[len(removed_base):]).replace("//", "/").replace(' ', '_')

            self.mkdir(current_base)
            for filename in os.listdir(current_dir):
                filename = os.path.join( path, filename)
                print ">", filename, current_base, opt
                self.upload(filename, current_base, opt)


        _treat_dir(path)

    def resetM4(self):
        url = self._getUrl("config.cgi?mres")

        print "Reset M4"
        r= requests.get(url, headers=self._getHeaders())
        assert r.status_code == 200

    def resetCPC(self):
        url = self._getUrl("config.cgi?cres")

        print "Reset CPC"
        r= requests.get(url, headers=self._getHeaders())
        assert r.status_code == 200

    def upload(self, path, destination, opt):
        assert(os.path.exists(path))

        if os.path.isdir(path):
            return self._upload_dir(path, destination, opt)
        else:
            return self._upload_file(path, destination, opt)


    def download(self, path, opt):
        print "Download" + path 
        url = self._getUrl("sd/%s" % path)
        r = requests.get( url)
        assert r.status_code == 200

        with open(os.path.basename(path), 'wb') as f:
            for c in r.text:
                f.write(chr(ord(c)))


    def execute(self, cpcfile):
        url = self._getUrl("config.cgi")

        print "Execute " + cpcfile

        r = requests.get(
            url,
            params = {
                "run2": cpcfile
            }
        )
        assert r.status_code == 200


    def mkdir(self, folder):
        # TODO Manage the fact that several repositories are asked
        if folder[0] != "/":
            folder = "/" + folder

        print "Create directory " + folder


        url = self._getUrl("config.cgi")
        r= requests.get(
            url, 
            params = {"mkdir": folder},
            headers=self._getHeaders())
        assert r.status_code == 200

    def cd(self, cpcfolder):
        url = self._getUrl("config.cgi")
        r = requests.get(
            url,
            params = {
                "cd": cpcfolder
            }
        )
        assert r.status_code == 200


    def rm(self, cpcfile):
        url = self._getUrl("config.cgi")
        r = requests.get(
            url,
            params = {
                "rm": cpcfile
            }
        )
        assert r.status_code == 200


    def delRom(self, romid):
        assert 0 <= int(romid) <= 31
        url = self._getUrl("roms.shtml")
        r = requests.get(
            url,
            params = {
                "rmsl": romid
            }
        )
        assert r.status_code == 200

    def putRom(self, fname, romid, name):
        assert 0 <= int(romid) <= 31
        assert os.path.isfile(fname)

        print "Put ROM "  + fname + " in " + romid + ' as ' + name
        url = self._getUrl("roms.shtml")
        r = requests.post(
            url,
            data = {
                "slotnum": romid,
                "slotname": name,
            },
            files = {
           "uploadedfile": (
                "rom.bin",
                open(fname, 'rb'),
                    "Content-Type: application/octet-stream",
                    {'Expires': '0'}               
            )}
        )
        assert r.status_code == 200


    def pause(self):
        url = self._getUrl("config.cgi")
        r = requests.get(
            url,
            params = {
                "chlt": "CPC+Pause"
            }
        )
        assert r.status_code == 200


    def ls(self, cpcfolder):
        print "LS in " + cpcfolder
        url = self._getUrl("config.cgi")

        r = requests.get(
            url,
            params = {
                "ls": cpcfolder
            }
        )

        assert r.status_code == 200


        url = self._getUrl("sd/m4/dir.txt")
        r = requests.get(
            url
        )
        assert r.status_code == 200
        print r.text

# code
if __name__ == '__main__':


    arguments = docopt(__doc__, version='Python XFER 0.1')

    
    xfer = XFER(arguments['<ip>'])

    shared = {
        'dest' : "/",
        'opt' : 0
    }


    def upload():
        fname = arguments["<file>"]
        if arguments["<path>"]:
            shared['dest'] = arguments["<path>"]
        if arguments["<opt>"]:
            shared["opt"] = arguments["<opt>"]
        xfer.upload(fname, shared["dest"], shared["opt"])

    def download():
        fname = arguments["<cpcfile>"]
        if arguments["<opt>"]:
            shared["opt"] = arguments["<opt>"]
        xfer.download(fname, shared["opt"])



    def execute():
        xfer.execute(arguments["<cpcfile>"])

    if arguments["-s"]:
        xfer.resetCPC()
    elif arguments["-r"]:
        xfer.resetM4()
    elif arguments["-u"]:
        upload()
    elif arguments["-d"]:
        shared["opt"] = 1
        download()
    elif arguments["-x"]:
        execute()
    elif arguments["-y"]:
        arguments["<path>"] = "/tmp"
        upload()
        arguments["<cpcfile>"] = "/tmp/" + os.path.basename(arguments["<file>"])
        execute()
    elif arguments["-m"]:
        xfer.mkdir(arguments["<folder>"])
    elif arguments["--cd"]:
        xfer.cd(arguments["<cpcfolder>"])
    elif arguments["--rm"]:
        xfer.rm(arguments["<cpcfile>"])
    elif arguments["--delRom"]:
        xfer.delRom(arguments["<romid>"])
    elif arguments["-f"]:
        xfer.putRom(arguments['<file>'], arguments["<romid>"], arguments["<name>"])
    elif arguments["-p"]:
        xfer.pause()
    elif arguments["--ls"]:
        if not arguments["<folder>"]:
            arguments["<folder>"] = ""
        xfer.ls(arguments["<folder>"])
    else:
        print "Option unknown"


# metadata
__author__ = 'Romain Giot'
__copyright__ = 'Copyright 2017'
__credits__ = ['Romain Giot']
__licence__ = 'GPL'
__version__ = '0.1'
__maintainer__ = 'Romain Giot'
__email__ = 'giot.romain@gmail.com'
__status__ = 'Prototype'

