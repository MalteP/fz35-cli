set terminal pngcairo font ",72" fontscale 0.19 size 800, 600 
set output outputfile

set datafile separator ","

set format x '%.2fA'
set format y '%.0fV'

#set nomxtics

set grid layerdefault lt 0 linecolor black linewidth 2

set tmargin 1
set rmargin 3
# set bmargin 3
# set lmargin 5

# title
set label 1 ARG1
set label 1 at graph 0.0, 1.05

set xrange [ * : * ] noreverse writeback
set x2range [ * : * ] noreverse writeback
set yrange [ * : * ] noreverse writeback
set y2range [ * : * ] noreverse writeback
set zrange [ * : * ] noreverse writeback
set cbrange [ * : * ] noreverse writeback
set rrange [ * : * ] noreverse writeback

plot \
csvfile using 1:2 with lines linecolor black linewidth 2 dashtype 1 title 'V(A)', \
csvfile using 1:4 with lines linecolor black linewidth 2 dashtype 4 title 'W(A)'