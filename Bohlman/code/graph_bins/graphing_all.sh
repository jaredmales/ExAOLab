graph -T ps -L "variance vs. mean value of pixels binned" -X "mean value" -Y "variance" -m 0 ../raw_data_091217/data.txt -C -m 0 ../raw_data_101717/data.txt -m 3 -S 1 .1 ../raw_data_091217/data.out -m 2 -S 1 .1 ../raw_data_101717/data.out > super_plot.ps

