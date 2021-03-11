#!/usr/bin/python

import sys, getopt, re

def printHelp():
    print './only_spaces_nums_letters_lowcase.py -i <inputfile> -o <outputfile>'

def run(inputfile, outputfile):
    print 'Input file is "', inputfile, '"'
    print 'Output file is "', outputfile, '"'
    rFd = open(inputfile, "r")
    wFd = open(outputfile, "w")
    data = rFd.read().lower()

    dataLen = len(data)
    res = ''
    i = 0

    while i < dataLen:
        ch = data[i]
        i += 1
        if not ch.isalpha() and not ch.isdigit() and not ch.isspace():
            continue
        res += ch

    rex = re.compile(r'\W+')
    dataOneSpaces = rex.sub(' ', res)

    wFd.write(dataOneSpaces)

    rFd.close()
    wFd.close()
    print('Done')

def main(argv):
    inputfile = ''
    outputfile = ''
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
        elif opt in ("-o", "--ofile"):
            check += 1
            outputfile = arg
    if check == 3:
        run(inputfile, outputfile)
    else:
        printHelp()

if __name__ == "__main__":
   main(sys.argv[1:])