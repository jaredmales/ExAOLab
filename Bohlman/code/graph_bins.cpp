#include <fstream>
#include <iostream>
#include <vector>
#include <iomanip>
#include <numeric>
#include <algorithm>


int main ( void ) {
	std::ifstream inFile;
	std::vector <double> mean(30720);
	std::vector <double> variance(30720);
	double n,m;
	int i=0;
	inFile.open("data.txt");
	if (!inFile) {
        	std::cerr << "Unable to open file";
    	}
	else {
		while(inFile>>n>>m){
			mean[i] = n;
			variance [i] = m;
			i++;
		}
		std::sort(mean.begin(),mean.end());
		std::sort(variance.begin(),variance.end());
		double mean_num,variance_num;
		mean_num = std::accumulate(mean.begin(), mean.begin()+3072*1, 0.0) / 3072;
		variance_num = std::accumulate(variance.begin(), variance.begin()+3072*1, 0.0) / 3072;
		std::cout << mean_num << '\t' << variance_num << std::endl;
		mean_num = std::accumulate(mean.begin()+3072*1 + 1, mean.begin()+3072*2, 0.0) / 3072;
		variance_num = std::accumulate(variance.begin()+3072*1 + 1, variance.begin()+3072*2, 0.0) / 3072;
		std::cout << mean_num << '\t' << variance_num << std::endl;
		mean_num = std::accumulate(mean.begin()+3072*2 + 1, mean.begin()+3072*3, 0.0) / 3072;
		variance_num = std::accumulate(variance.begin()+3072*2 + 1, variance.begin()+3072*3, 0.0) / 3072;
		std::cout << mean_num << '\t' << variance_num << std::endl;
		mean_num = std::accumulate(mean.begin()+3072*3 + 1, mean.begin()+3072*4, 0.0) / 3072;
		variance_num = std::accumulate(variance.begin()+3072*3 + 1, variance.begin()+3072*4, 0.0) / 3072;
		std::cout << mean_num << '\t' << variance_num << std::endl;
		mean_num = std::accumulate(mean.begin()+3072*4 + 1, mean.begin()+3072*5, 0.0) / 3072;
		variance_num = std::accumulate(variance.begin()+3072*4 + 1, variance.begin()+3072*5, 0.0) / 3072;
		std::cout << mean_num << '\t' << variance_num << std::endl;
		mean_num = std::accumulate(mean.begin()+3072*5 + 1, mean.begin()+3072*6, 0.0) / 3072;
		variance_num = std::accumulate(variance.begin()+3072*5 + 1, variance.begin()+3072*6, 0.0) / 3072;
		std::cout << mean_num << '\t' << variance_num << std::endl;
		mean_num = std::accumulate(mean.begin()+3072*6 + 1, mean.begin()+3072*7, 0.0) / 3072;
		variance_num = std::accumulate(variance.begin()+3072*6 + 1, variance.begin()+3072*7, 0.0) / 3072;
		std::cout << mean_num << '\t' << variance_num << std::endl;
		mean_num = std::accumulate(mean.begin()+3072*7 + 1, mean.begin()+3072*8, 0.0) / 3072;
		variance_num = std::accumulate(variance.begin()+3072*7 + 1, variance.begin()+3072*8, 0.0) / 3072;
		std::cout << mean_num << '\t' << variance_num << std::endl;
		mean_num = std::accumulate(mean.begin()+3072*8 + 1, mean.begin()+3072*9, 0.0) / 3072;
		variance_num = std::accumulate(variance.begin()+3072*8 + 1, variance.begin()+3072*9, 0.0) / 3072;
		std::cout << mean_num << '\t' << variance_num << std::endl;
		mean_num = std::accumulate(mean.begin()+3072*9 + 1, mean.begin()+3072*10, 0.0) / 3072;
		variance_num = std::accumulate(variance.begin()+3072*9 + 1, variance.begin()+3072*10, 0.0) / 3072;
		std::cout << mean_num << '\t' << variance_num << std::endl;
	}
}
