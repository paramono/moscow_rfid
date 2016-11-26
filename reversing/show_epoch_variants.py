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
    bought = date(year=2016, month=9, day=17)
    duration = timedelta(days=90)
    expire_date = bought + duration

    for year in range(1930, 2016):
        epoch = date(year=year, month=1, day=1)
        difference = expire_date - epoch
        days = difference.days
        print('%s, %4d days (%04X)' % (epoch, days, days))


if __name__ == '__main__':
    main()
