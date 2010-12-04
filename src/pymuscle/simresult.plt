set term wxt 0
set title '...'
plot 'simresult.txt' using 1:2 title columnheader with lines,\
            ''       using 1:3 title columnheader with lines,\
            ''       using 1:4 title columnheader with lines

set term wxt 1
set title '...'
plot 'simresult.txt' using 1:8 title columnheader with lines, \
            ''       using 1:9 title columnheader with lines, \
            ''       using 1:10 title columnheader with lines

set term wxt 2
set title 'Cost'
plot 'simresult.txt' using 1:18 title columnheader with lines

set term wxt 3
plot 'simresult.txt' using 1:14 title columnheader with lines, \
           ''        using 1:19 title columnheader with lines

set term wxt 4
plot 'simresult.txt' using 1:15 title columnheader with lines, \
           ''        using 1:20 title columnheader with lines

set term wxt 5
plot 'simresult.txt' using 1:16 title columnheader with lines, \
           ''        using 1:21 title columnheader with lines

set term wxt 6
plot 'simresult.txt' using 1:17 title columnheader with lines, \
           ''        using 1:22 title columnheader with lines

