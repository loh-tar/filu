#!/bin/bash
echo "[Header]RefSymbol;Name;Type;Provider0;Symbol0;Market0;Provider1;Symbol1;Market1"
cat all_FIs_Filu.csv |grep -i -E $1
