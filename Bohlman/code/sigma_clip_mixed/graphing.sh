#!/bin/bash
# Graphing script

graph -T ps -m 0 -L "variance vs. mean value of pixels" -X "mean value" -Y "variance" < data.txt > plot.ps
