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

    bought = date(year=2016, month=9, day=17)
    duration = timedelta(days=90)
    expire_date = bought + duration
    print('expire date: ', expire_date)

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
    print()

    epoch = expire_date - timedelta(days=elapsed_days)
    epoch2 = bought - timedelta(days=elapsed_days)
    print('epoch', epoch)
    print('epoch2', epoch2)


if __name__ == '__main__':
    main()
