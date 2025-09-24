#!/bin/bash

# Serial port for FZ35
port=/dev/ttyUSB0

# Output CSV file
file=measurement_$(date +%F_%H-%M-%S).csv

# Initialize device
./fz35-cli $port stop off setup
if [ $? -ne 0 ]
then
    exit 1;
fi

# Measure data and export CSV file
./fz35-cli $port 0.00A on slp getCsvRow \
    0.02A slp getCsvRow \
    0.04A slp getCsvRow \
    0.06A slp getCsvRow \
    0.08A slp getCsvRow \
    0.10A slp getCsvRow \
    0.12A slp getCsvRow \
    0.14A slp getCsvRow \
    0.16A slp getCsvRow \
    0.18A slp getCsvRow \
    0.20A slp getCsvRow \
    0.22A slp getCsvRow \
    0.24A slp getCsvRow \
    0.26A slp getCsvRow \
    0.28A slp getCsvRow \
    0.30A slp getCsvRow \
    0.00A off > $file
if [ $? -ne 0 ]
then
    exit 1;
fi

exit 0;