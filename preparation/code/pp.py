#!/home/joshua/miniconda2/bin/python
# -*- coding: utf8 -*-
#
# Author:   Joshua Chen <iesugrace@gmail.com>
# Date:     2016-06-18 07:21:28
# Location: Shenzhen
# Desc:     Get all parent processes information
#

from __future__ import unicode_literals, print_function
from subprocess import Popen, PIPE
import re, os, sys

def getparents(pid):
    gens   = []
    proc   = Popen(['ps', '-eo', 'pid,ppid,comm'], stdout=PIPE)
    lines  = proc.stdout.readlines()[1:]
    proc.wait()
    regex  = re.compile(r'^\s*([0-9]+)\s+([0-9]+)\s+(.*)\n$')
    fields = [regex.findall(x)[0] for x in lines]
    while True:
        info = [x[1:] for x in fields if x[0] == pid]
        if not info: break
        ppid, name = info[0]
        gens.append((pid, name))
        if ppid == '0': break
        pid = ppid
    for pid, name in gens[::-1]:
        print('%s (%s)' % (pid, name))


if __name__ == '__main__':
    if len(sys.argv) == 1:      # no cmdline args
        pid = str(os.getpid())
    elif len(sys.argv) == 2:    # one arg
        pid = sys.argv[1]
    else:                       # wrong argument
        bname = os.path.basename(sys.argv[0])
        print("Usage: %s [pid]" % bname, file=sys.stderr)
        exit(1)
    if not pid.isdigit():
        print("invalid pid: %s" % pid, file=sys.stderr)
        exit(1)
    for i in range(200):
        getparents(pid)
