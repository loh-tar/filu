#!/bin/bash
#cd /usr/local/lib/Filu/provider/Filu/
echo "HEADER=Name;iSymbol"
grep -h --include=Index_*.csv  IndexName * | sed  s/*IndexName=//
#grep -h --include=Index_*.csv  IndexName * |grep -h -i $1 | sed  s/*IndexName=//