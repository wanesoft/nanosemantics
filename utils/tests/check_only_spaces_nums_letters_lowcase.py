#!/usr/bin/python

import sys, getopt

def printHelp():
    print './check_only_spaces_nums_letters_lowcase.py -i <inputfile>'

def onError(str, pos, ch):
    print 'Error: ', str, ', pos: ', pos, ', character: \'', ch, '\''
    sys.exit()

def run(inputfile):
    print 'Input file is "', inputfile, '"'
    rFd = open(inputfile, "r")
    data = rFd.read()
    dataLen = len(data)
    i = 0
    while i < dataLen - 1:
        ch = data[i]
        nextCh = data[i + 1]
        if not ch.lower():
            onError('Not lowercase detected', i, ch)
        if ch.isspace() and nextCh.isspace():
            onError('Double spaces detected', i, ch)
        if not ch.isdigit() and not ch.isalpha() and not ch.isspace():
            onError('Not digit or letter detected', i, ch)
        i += 1
    rFd.close()
    print('Done, file is correct')

def main(argv):
    inputfile = ''
    check = 0
    try:
        opts, args = getopt.getopt(argv,"hi:o:",["ifile=","ofile="])
    except getopt.GetoptError:
        printHelp()
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            printHelp()
            sys.exit()
        elif opt in ("-i", "--ifile"):
            check += 2
            inputfile = arg
    if check == 2:
        run(inputfile)
    else:
        printHelp()

if __name__ == "__main__":
   main(sys.argv[1:])