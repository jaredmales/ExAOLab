/*! \file graph_bins.cpp
\brief A file that finds median variance and median mean value for each bin so they can be graphed.
*/
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>

double inPlaceMedian( std::vector<double> v)
{
   std::vector<double> tmp = v;
   std::nth_element(tmp.begin(), tmp.begin() + tmp.size()/2, tmp.end());
   return tmp[tmp.size()/2];
}

int main ( void ) {
	std::ifstream inFile;
	std::vector <double> mean1(0);
	std::vector <double> mean2(0);
	std::vector <double> mean3(0);
	std::vector <double> mean4(0);
	std::vector <double> mean5(0);
	std::vector <double> mean6(0);
	std::vector <double> mean7(0);
	std::vector <double> mean8(0);
	std::vector <double> mean9(0);
	std::vector <double> mean10(0);
	std::vector <double> variance1(0);
	std::vector <double> variance2(0);
	std::vector <double> variance3(0);
	std::vector <double> variance4(0);
	std::vector <double> variance5(0);
	std::vector <double> variance6(0);
	std::vector <double> variance7(0);
	std::vector <double> variance8(0);
	std::vector <double> variance9(0);
	std::vector <double> variance10(0);
	double n,m;
	int i=0;
	inFile.open("data.txt");
	if (!inFile) {
        	std::cerr << "Unable to open file";
    	}
	else {
		while(inFile>>n>>m){
			if (i == 0) {
				mean1.push_back(n);
				variance1.push_back(m);
			}
			else if (i == 1) {
				mean2.push_back(n);
				variance2.push_back(m);
			}
			else if (i == 2) {
				mean3.push_back(n);
				variance3.push_back(m);
			}
			else if (i == 3) {
				mean4.push_back(n);
				variance4.push_back(m);
			}
			else if (i == 4) {
				mean5.push_back(n);
				variance5.push_back(m);
			}
			else if (i == 5) {
				mean6.push_back(n);
				variance6.push_back(m);
			}
			else if (i == 6) {
				mean7.push_back(n);
				variance7.push_back(m);
			}
			else if (i == 7) {
				mean8.push_back(n);
				variance8.push_back(m);
			}
			else if (i == 8) {
				mean9.push_back(n);
				variance9.push_back(m);
			}
			else if (i == 9) {
				mean10.push_back(n);
				variance10.push_back(m);
				i = -1;
			}
			i++;
		}
		double variance_num, mean_num;
		mean_num = inPlaceMedian(mean1);
		variance_num = inPlaceMedian(variance1);
		std::cout << mean_num << '\t' << variance_num << std::endl;

		mean_num = inPlaceMedian(mean2);
		variance_num = inPlaceMedian(variance2);
		std::cout << mean_num << '\t' << variance_num << std::endl;

		mean_num = inPlaceMedian(mean3);
		variance_num = inPlaceMedian(variance3);
		std::cout << mean_num << '\t' << variance_num << std::endl;

		mean_num = inPlaceMedian(mean4);
		variance_num = inPlaceMedian(variance4);
		std::cout << mean_num << '\t' << variance_num << std::endl;

		mean_num = inPlaceMedian(mean5);
		variance_num = inPlaceMedian(variance5);
		std::cout << mean_num << '\t' << variance_num << std::endl;

		mean_num = inPlaceMedian(mean6);
		variance_num = inPlaceMedian(variance6);
		std::cout << mean_num << '\t' << variance_num << std::endl;

		mean_num = inPlaceMedian(mean7);
		variance_num = inPlaceMedian(variance7);
		std::cout << mean_num << '\t' << variance_num << std::endl;

		mean_num = inPlaceMedian(mean8);
		variance_num = inPlaceMedian(variance8);
		std::cout << mean_num << '\t' << variance_num << std::endl;

		mean_num = inPlaceMedian(mean9);
		variance_num = inPlaceMedian(variance9);
		std::cout << mean_num << '\t' << variance_num << std::endl;

		mean_num = inPlaceMedian(mean10);
		variance_num = inPlaceMedian(variance10);
		std::cout << mean_num << '\t' << variance_num << std::endl;
	}
}
