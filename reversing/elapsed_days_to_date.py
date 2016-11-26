#!/usr/bin/python3

'''
Calculates and outputs a date, given a number of days elapsed since 1991-01-01
as an argument

Argument can be supplied either as hex (see the regex patter in code)
or as decimal string
'''

import sys
import re
from datetime import date, timedelta


def main():
    if len(sys.argv) != 2:
        raise ValueError('Supply only 1 argument: elapsed days')

    epoch = date(year=1992, month=1, day=1)
    arg = sys.argv[1]
    hex_regex = re.compile('^0x[0-9A-Fa-f]+$')
    dec_regex = re.compile('^[0-9]+$')

    if hex_regex.match(arg):
        elapsed_days = int(arg, 16)
    elif dec_regex.match(arg):
        elapsed_days = int(arg, 10)
        print('dec')
    else:
        raise ValueError('Wrong argument format')
    print('%d days (0x%X)' % (elapsed_days, elapsed_days))

    expire_date = epoch + timedelta(days=elapsed_days)
    print(expire_date)


if __name__ == '__main__':
    main()
