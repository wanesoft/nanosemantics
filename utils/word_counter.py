#!/usr/bin/python

import sys, getopt

def printHelp():
    print './word_counter.py -i <inputfile>'

def onError(str, pos, ch):
    print 'Error: ', str, ', pos: ', pos, ', character: \'', ch, '\''
    sys.exit()

def run(inputfile):
    print 'Input file is "', inputfile, '"'
    rFd = open(inputfile, "r")
    data = rFd.read()
    dataLen = len(data)
    i = 0

    while i < dataLen - 6:
        ch = data[i]
        if (data[i] == ' ' and data[i + 1] == 'l' and data[i + 2] == 'o' and data[i + 3] == 'v' and data[i + 4] == 'e' and data[i + 5] == ' '):
            print i + 1
        i += 1

    rFd.close()
    print('Done')

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