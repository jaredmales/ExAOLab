#include <stdio.h>
#include <gsl/gsl_fit.h>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <sstream>
#include <iostream>

int main (void)
{

  int nelements = 640*480*10;

  double *std_dev = (double*)calloc(nelements, sizeof(double)); //Array of pixel values for final image
  double *mean = (double*)calloc(nelements, sizeof(double)); //Array of pixel values for final image

  std::ifstream file("data.txt");
  std::string str;
  int i = 0;
  while (std::getline(file, str) && i < nelements)
  {
        std::istringstream iss(str);
        double n, m;
        iss >> n >> m;
        std_dev[i] = n;
        mean[i] = m;
        ++i;
  }

  double c0 = 0, c1 = 0, cov00 = 0, cov01 = 0, cov11 = 0, sumsq = 0;

  gsl_fit_linear(mean, 1, std_dev, 1, nelements, &c0, &c1, &cov00, &cov01, &cov11, &sumsq);

  std::cout << "M (Gain): " << c1 << '\n' << "B (Read Noise): " << c0 << std::endl;

  return 0;
}
