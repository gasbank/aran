set term wxt 0
set title 'Z position'
plot 'hatbiped_result.txt' using 1:4 title columnheader with lines,\
            ''       using 1:10 title columnheader with lines,\
            ''       using 1:40 title columnheader with lines,\
            ''       using 1:46 title columnheader with lines
#

set term wxt 1
set title 'X,Y position'
plot 'hatbiped_result.txt' using 1:2 title columnheader with lines, \
            ''       using 1:3 title columnheader with lines, \
            ''       using 1:8 title columnheader with lines, \
            ''       using 1:9 title columnheader with lines
     
set term wxt 2
plot "hatbiped_result.txt" using 1:14 title columnheader with lines
