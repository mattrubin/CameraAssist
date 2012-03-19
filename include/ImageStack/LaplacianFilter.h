#ifndef IMAGESTACK_LAPLACIANFILTER_H
#define IMAGESTACK_LAPLACIANFILTER_H
#include "header.h"
#include <vector>

class LaplacianFilter : public Operation {
 public:
  void help();
  void parse(vector<string> args);
  static Image apply(Window source, float sigma_r, float alpha, float beta);
  static Image decimate(Window source, int x_offset = 0, int y_offset = 0);
  static Image undecimate(Window source, int new_width = -1, int new_height = -1,
			  int x_offset = 0, int y_offset = 0);
 private:
};

class LaplacianPyramid {

 public:
  LaplacianPyramid(Window window, int level = -1);
  LaplacianPyramid(LaplacianPyramid& source);
  int getTotalLevels() const;
  Image& getLevel(int level);
  Image reconstruct();
 private:
  int total_levels;
  std::vector<Image> layers;
};

#include "footer.h"
#endif
