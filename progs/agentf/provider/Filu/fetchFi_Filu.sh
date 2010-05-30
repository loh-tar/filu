#!/bin/bash
echo "HEADER=MySymbol;Name;Type;Native Market;Provider-Symbol-Market;Provider-Symbol-Market"
cat all_FIs_Filu.csv |grep -i -E $1
