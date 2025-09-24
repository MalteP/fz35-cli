#!/bin/bash
for file in measurement_*.csv; do
    outputfile="${file%.csv}.png"
    gnuplot -e "csvfile='${file}'; outputfile='${outputfile}'" $(dirname "$0")/plot_csv.gnuplot
done