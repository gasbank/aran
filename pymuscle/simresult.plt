set term wxt 0
plot "simresult.txt" using 1:4 title 'P1[z]' with lines,\
     "simresult.txt" using 1:10 title 'P2[z]' with lines
     
set term wxt 1
plot "simresult.txt" using 1:2 title 'P1[x]' with lines, \
     "simresult.txt" using 1:3 title 'P1[y]' with lines, \
     "simresult.txt" using 1:8 title 'P2[x]' with lines, \
     "simresult.txt" using 1:9 title 'P2[y]' with lines
     
set term wxt 2
plot "simresult.txt" using 1:14 title 'Tension' with lines
