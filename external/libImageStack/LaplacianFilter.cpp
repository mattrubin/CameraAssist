#include "main.h"
#include "Arithmetic.h"
#include "Convolve.h"
#include "File.h"
#include "Geometry.h"
#include "LaplacianFilter.h"
#include "header.h"

void LaplacianFilter::help() {
  pprintf("-laplacianpyramid does something.\n"); // TODO
}

void LaplacianFilter::parse(vector<string> args) {
  assert(args.size() == 0, "-laplacianpyramid does not take arguments.\n");
  push(apply(stack(0), 0.f, 0.f, 0.f));
}

Image LaplacianFilter::apply(Window source, float sigma_r, float alpha, float beta) {
  LaplacianPyramid in(source);
  LaplacianPyramid out(in);

  float filter_values[5] = {0.05f, 0.25f, 0.4f, 0.25f, 0.05f};
  Image filter_x(5, 1, 1, 1, filter_values);
  Image filter_y(1, 5, 1, 1, filter_values);

  Image gauss = in.getLevel(in.getTotalLevels());
  for (int i = in.getTotalLevels(); i >= 0; i--) {

    // Generate the layer of the Gaussian pyramid at the current level
    Image layer = in.getLevel(i);
    if (i < in.getTotalLevels()) {
      Image tmp = LaplacianFilter::undecimate(gauss, layer.width, layer.height);
      tmp = Convolve::apply(tmp, filter_x, Convolve::Homogeneous);
      tmp = Convolve::apply(tmp, filter_y, Convolve::Homogeneous);
      gauss = Image(tmp.width, tmp.height, 1, gauss.channels);
      for (int y = 0; y < tmp.height; y++)
	for (int x = 0; x < tmp.width; x++) {
	  float weight = 1.f / tmp(x, y)[gauss.channels];
	  for (int c = 0; c < gauss.channels; c++) {
	    gauss(x, y)[c] = layer(x, y)[c] + tmp(x, y)[c] * weight;
	  }
	}
    }

    // For each coefficient of the Gaussian layer, map the neighborhood
    for (int y = 0; y < layer.height; y++) {
      for (int x = 0; x < layer.width; x++) {
	// Need to compute the footprint of Pyramid(x,y,i) and recompute.
	// Math:
	// The laplacian coefficient is a weighted sum of the pixels. Is there a easy way to compute the
	// weights?
	

	
      }
    }
  }
  return out.reconstruct();
}

Image LaplacianFilter::decimate(Window source, int x_offset, int y_offset) {
  int new_width = (source.width - x_offset + 1) >> 1;
  int new_height = (source.height - y_offset + 1) >> 1;
  Image ret(new_width, new_height, source.frames, source.channels);
  for (int t = 0; t < source.frames; t++) {
    int old_y = y_offset;
    for (int y = 0; y < new_height && old_y < source.height; y++, old_y+=2) {
      int old_x = x_offset;
      for (int x = 0; x < new_width && old_x < source.width; x++, old_x+=2) {
	for (int c = 0; c < source.channels; c++) {
	  ret(x, y, t)[c] = source(old_x, old_y, t)[c];
	}
      }
    }
  }
  return ret;
}

Image LaplacianFilter::undecimate(Window source, int new_width, int new_height, int x_offset, int y_offset) {
  new_width = (new_width == -1 ? source.width * 2 : new_width);
  new_height = (new_height == -1 ? source.height * 2 : new_height);
  Image ret(new_width, new_height, source.frames, source.channels + 1);
  for (int t = 0; t < source.frames; t++) {
    int old_y = 0;
    for (int y = y_offset; y < new_height && old_y < source.height; y+=2, old_y++) {
      int old_x = 0;
      for (int x = x_offset; x < new_width && old_x < source.width; x+=2, old_x++) {
	for (int c = 0; c < source.channels; c++) {
	  ret(x, y, t)[c] = source(old_x, old_y, t)[c];
	}
	ret(x, y, t)[source.channels ] = 1.f;
      }
    }
  }
  return ret;
}

LaplacianPyramid::LaplacianPyramid(LaplacianPyramid& source) {
  total_levels = source.getTotalLevels();
  for (int i = 0; i <= total_levels; i++) {
    layers.push_back(source.getLevel(i).copy());
  }
}

LaplacianPyramid::LaplacianPyramid(Window window, int level) {
  // It must be that the window has a single frame.
  assert(window.frames == 1, "Cannot construct a Laplacian pyramid for a multi-frame image.\n");

  total_levels = 0;
  float filter_values[5] = {0.05f, 0.25f, 0.4f, 0.25f, 0.05f};
  Image filter_x(5, 1, 1, 1, filter_values);
  Image filter_y(1, 5, 1, 1, filter_values);

  Image current(window);
  while (current.width > 1 && current.height > 1 && (total_levels < level || level < 0)) {
    printf("Generating level: %d\n", total_levels);
    layers.push_back(current);
    //   Step 1) Convolution with filters in order to blur.
    Image convolved_x = Convolve::apply(current, filter_x, Convolve::Homogeneous);
    Image convolved_xy = Convolve::apply(convolved_x, filter_y, Convolve::Homogeneous);
    //   Step 2) Decimation
    current = LaplacianFilter::decimate(convolved_xy);
    //   Step 3) Upsample
    Image undecimated = LaplacianFilter::undecimate(current, convolved_xy.width, convolved_xy.height);
    Image reconvolved_x = Convolve::apply(undecimated, filter_x, Convolve::Zero);
    Image reconvolved_xy = Convolve::apply(reconvolved_x, filter_y, Convolve::Zero);
    for (int y = 0; y < reconvolved_xy.height; y++)
      for (int x = 0; x < reconvolved_xy.width; x++) {
	float weight = 1.f / reconvolved_xy(x, y)[current.channels];
	for (int c = 0; c < current.channels; c++) {
	  layers[total_levels](x, y)[c] -= reconvolved_xy(x, y)[c] * weight;
	}
      }
    total_levels++;
  }
  layers.push_back(current);
}

int LaplacianPyramid::getTotalLevels() const {
  return total_levels;
}

Image& LaplacianPyramid::getLevel(int level) {
  assert(level >= 0 && level <= total_levels, "Level range is invalid.");
  return layers[level];
}

Image LaplacianPyramid::reconstruct() {
  fflush(stdout);
  float filter_values[5] = {0.05f, 0.25f, 0.4f, 0.25f, 0.05f};
  Image filter_x(5, 1, 1, 1, filter_values);
  Image filter_y(1, 5, 1, 1, filter_values);

  Image ret = layers[total_levels];
  for (int i = 0; i < total_levels; i++) {
    Image& prev = layers[total_levels - 1 - i];
    Image tmp = LaplacianFilter::undecimate(ret, prev.width, prev.height);
    tmp = Convolve::apply(tmp, filter_x, Convolve::Homogeneous);
    tmp = Convolve::apply(tmp, filter_y, Convolve::Homogeneous);
    ret = Image(tmp.width, tmp.height, 1, ret.channels);
    for (int y = 0; y < tmp.height; y++)
      for (int x = 0; x < tmp.width; x++) {
	float weight = 1.f / tmp(x, y)[ret.channels];
	for (int c = 0; c < ret.channels; c++) {
	  ret(x, y)[c] = prev(x, y)[c] + tmp(x, y)[c] * weight;
	}
      }
  }
  return ret;
}

#include "footer.h"
