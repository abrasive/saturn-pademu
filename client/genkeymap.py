#!/usr/bin/env python2

print "std::map<int, const char*> saturn_scancodes = {"
print '   {0x0e, "F1"},'    # hankaku
print '   {0x13, "F2"},'    # kana toggle


for line in open('keymap').readlines():
    if line.startswith('#'):
        continue
    cols = line.strip().split('  ')

    scancode, keyname = cols
    keystr = keyname.replace('\\', '\\\\')
    print '    {0x%s, "%s"},' % (scancode, keystr)

print "};"
