#include "seg_levelset.hpp"

using namespace std;
using namespace cv;

/**
 * `phi0`: 初始轮廓 mask 二值图像，前景区域值为 -2，背景区域值为 2
 * `img`: 原始图像，要求 channel 为 1，即为灰度图
 * `prior`: 置为空即可
 * `range`: 设置为 (0, 0, 0)
 * 返回值：结果二值图像，前景区域为 0，背景区域为 255
 */
Mat segSlice(Mat phi0, Mat img, const Mat &prior, double range[3],
             int iterInner, int iterOuter) {
  int width, height;
  if (img.cols) {
    width = img.rows;
    height = img.cols;
  }

  // convert Mat to CV_32F with one channel
  // cvtColor(img, img, CV_BGR2GRAY);

  img.convertTo(img, CV_64F);

  phi0.convertTo(phi0, CV_64F);

  double minVal;
  double maxVal;
  Point minLoc;
  Point maxLoc;
  minMaxLoc(img, &minVal, &maxVal, &minLoc, &maxLoc);

  int type = img.type();
  type = phi0.type();
  type = prior.type();

  Mat phi0_cv, img_cv;
  phi0_cv = phi0;
  img_cv = img;

  // normalize(img_cv, img_cv, 255.0, 0.0, NORM_MINMAX);

  if (prior.rows) {
    Mat priorSeg = prior;
    normalize(priorSeg, priorSeg, 2, -2, NORM_MINMAX);
    /*double minVal;
            double maxVal;
            Point minLoc;
            Point maxLoc;
            minMaxLoc(priorSeg, &minVal, &maxVal, &minLoc, &maxLoc);*/
  } else {
    //    Mat priorSeg = phi0_cv;
    /*double minVal;
            double maxVal;
            Point minLoc;
            Point maxLoc;
            minMaxLoc(priorSeg, &minVal, &maxVal, &minLoc, &maxLoc);*/
  }

  int type1 = phi0_cv.type();
  int type2 = img_cv.type();

  // parameters setting
  int timeStep = 5;
  double mu =
      0.2 /
      double(
          timeStep); // coefficient of the distance regularization term R(phi)
  // int iterInner = 5;
  // int iterOuter = 10;
  // int iterOuter = 40;
  int lambda = 5;     // coefficient of the weighted length term L(phi)
  double alpha = 1.5; // coefficient of the weighted area term A(phi)
  double epsilon =
      1.5; // papramater that specifies the width of the DiracDelta function

  double sigma = 1.5; // scale parameter in Gaussian kernel

  // double alpha_d = 0.3;

  Mat cvImg, cvImg_sm;
  // convert eigen matrix to opencv matrix
  cvImg = img;
  cvImg_sm = cvImg.clone();

  /*double minVal;
      double maxVal;
      Point minLoc;
      Point maxLoc;*/

  // normalization.
  normalize(cvImg, cvImg, 255.0, 0.0, NORM_MINMAX);

  // remove make intensity value larger than 135
  // ??? need or not?
  /*threshold(cvImg, cvImg, 135, 0, CV_THRESH_TOZERO_INV);
      threshold(cvImg, cvImg, 20, 0, CV_THRESH_TOZERO);
      normalize(cvImg, cvImg, 255.0, 0.0, NORM_MINMAX);*/

  // smoothing
  GaussianBlur(cvImg, cvImg_sm, Size(5, 5), 0, 0);

  // cvImg_sm.type;

  // calculate gradient
  Mat grad2, gradX, gradY;
  Mat gradX2, gradY2;
  Mat edge(width, height, CV_32F);
  // edge.convertTo(edge, CV_16);
  Sobel(cvImg_sm, gradX, CV_16S, 1, 0, 3, 1, 0, BORDER_DEFAULT);
  gradX = gradX / 8;
  Sobel(cvImg_sm, gradY, CV_16S, 0, 1, 3, 1, 0, BORDER_DEFAULT);
  gradY = gradY / 8;
  // grad = gradX * gradX + gradY * gradY;
  // gradX = gradX / 4; gradY = gradY / 4;

  pow(gradX, 2, gradX2);
  pow(gradY, 2, gradY2);

  grad2 = ((gradX2 + gradY2));
  // calculate edge detector as 'g' in matlab code
  // suspecious
  // int temp2;
  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      // edge.at<float>(width * i + j) = 1.0f / float(grad2.at<short>(width * i
      // + j) + 1);
      edge.at<float>(i, j) = 1.0f / float(grad2.at<short>(i, j) + 1);
    }
  }

  // int c0 = 2;
  Mat phi(width, height, CV_32F);
  phi = phi0;

  // remove pixel with intensity value out of range
  /*calcHist();*/

  vector<vector<Point>> contours;
  Mat phiBinary;
  threshold(phi, phiBinary, 0, 1, THRESH_BINARY_INV);
  // findContours support this type only
  phiBinary.convertTo(phiBinary, CV_8UC1);
  findContours(phiBinary, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE,
               Point(0, 0));
  int size = contours.size();
  if (size <= 2) {
    // remove possible non-teeth pixels
    // threshold();
    for (int i = 0; i < width; i++) {
      for (int j = 0; j < height; j++) {
        if (phi.at<double>(i, j) < 0) {
          if (img_cv.at<double>(i, j) < range[2] /*range[0] + 20*/
              /*|| img_cv.at<double>(i, j) < range[2]*/) {
            phi.at<double>(i, j) = 2;
          }
        }
      }
    }

    // fill hole to avoid removing inner parts
    Mat im_th;
    threshold(phi, im_th, 0, 255, THRESH_BINARY_INV);

    // do not remove inner parts when it is very close to the root
    phi = fillHole(im_th);
  }

  phi0_cv.convertTo(phi0_cv, CV_32F);
  phi.convertTo(phi, CV_32F);
  cuda::GpuMat phi_cuda, edge_cuda, phi0_cuda;
  phi_cuda.upload(phi);
  edge_cuda.upload(edge);
  phi0_cuda.upload(phi0_cv);

  cuda::GpuMat cvImg_sm_cuda;
  cvImg_sm.convertTo(cvImg_sm, CV_32F);
  cvImg_sm_cuda.upload(cvImg_sm);

  // iteration breaking conditions
  for (int i = 0; i < iterOuter; i++) {
    // cout << "iterOuter i: " << i << endl;
    // int pre = countNonZero(phi < 0);
    phi_cuda = drlseEdge(phi_cuda, edge_cuda, lambda, mu, alpha, epsilon,
                         timeStep, iterInner, phi0_cuda);
    // phi_cuda = drlseEdgeGraDir(phi_cuda, edge_cuda, lambda, mu, alpha_d,
    // epsilon, timeStep, 	iterInner, phi0_cuda, cvImg_sm_cuda);
    // phi_cuda.download(phi);
    // int post = countNonZero(phi < 0);
    // if (pre - post <= 3) {
    //     break;
    // }
  }

  alpha = 0;
  int iterRefine = 10;
  phi_cuda = drlseEdge(phi_cuda, edge_cuda, lambda, mu, alpha, epsilon,
                       timeStep, iterRefine, phi0_cuda);

  // phi_cuda = drlseEdgeGraDir(phi_cuda, edge_cuda, lambda, mu, alpha_d,
  // epsilon, timeStep, 	iterRefine, phi0_cuda, cvImg_sm_cuda);
  phi_cuda.download(phi);
  Mat mask = (phi < 0);

  normalize(cvImg, cvImg, 255.0, 0.0, NORM_MINMAX);
  double min = 255, max = 0, average = 0, num = 0;

  int numNonZero = countNonZero(mask);
  // double* value = new double[numNonZero];
  vector<double> insideValues;

  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      if (mask.at<unsigned char>(i, j) != 0) {
        double pixelValue = cvImg.at<double>(i, j);
        insideValues.push_back(pixelValue);
        if (pixelValue < min) {
          min = pixelValue;
        }
        if (pixelValue > max) {
          max = pixelValue;
        }
        num++;
        average += pixelValue;
      }
    }
  }

  sort(insideValues.begin(), insideValues.end());
  int breakId = 0.1 * numNonZero - 1;
  int bottom = 0.05 * numNonZero, top = 0.95 * numNonZero;
  if (numNonZero != 0) {
    range[2] = insideValues[breakId];
    /*if (insideValues[top] - insideValues[bottom] < 45) {
                    range[2] = min;
            }*/
    insideValues.clear();
  } else {
    range[2] = 0;
  }

  average /= num;
  range[0] = min, range[1] = max;

  mask = fillHole(mask);
  threshold(mask, mask, 0, 255, THRESH_BINARY_INV);
  // Mat result = -phi;
  // normalize(result, result, 0, 255, NORM_MINMAX);
  // return result;
  return mask;
}

Mat segSlice(Mat phi0, Mat img, const Mat &prior,
             vector<vector<double>> &ranges, vector<Mat> initialLSFs,
             int start) {
  int width, height;
  if (img.cols) {
    width = img.rows, height = img.cols;
  }

  // convert Mat to CV_32F with one channel
  // cvtColor(img, img, CV_BGR2GRAY);

  img.convertTo(img, CV_64F);

  phi0.convertTo(phi0, CV_64F);

  double minVal;
  double maxVal;
  Point minLoc;
  Point maxLoc;
  minMaxLoc(img, &minVal, &maxVal, &minLoc, &maxLoc);

  int type = img.type();
  type = phi0.type();
  type = prior.type();

  Mat phi0_cv, img_cv;
  phi0_cv = phi0;
  img_cv = img;

  // normalize(img_cv, img_cv, 255.0, 0.0, NORM_MINMAX);

  if (prior.rows) {
    Mat priorSeg = prior;
    normalize(priorSeg, priorSeg, 2, -2, NORM_MINMAX);
    /*double minVal;
            double maxVal;
            Point minLoc;
            Point maxLoc;
            minMaxLoc(priorSeg, &minVal, &maxVal, &minLoc, &maxLoc);*/
  } else {
    /*Mat priorSeg = phi0_cv;
    double minVal;
    double maxVal;
    Point minLoc;
    Point maxLoc;
    minMaxLoc(priorSeg, &minVal, &maxVal, &minLoc, &maxLoc);*/
  }

  int type1 = phi0_cv.type();
  int type2 = img_cv.type();

  // parameters setting
  int timeStep = 5;
  // coefficient of the distance regularization term R(phi)
  double mu = 0.2 / double(timeStep);
  int iterInner = 5;
  int iterOuter = 30;
  // int iterOuter = 40;
  int lambda = 5;     // coefficient of the weighted length term L(phi)
  double alpha = 1.5; // coefficient of the weighted area term A(phi)
  // papramater that specifies the width of the DiracDelta function
  double epsilon = 1.5;

  double sigma = 1.5; // scale parameter in Gaussian kernel

  // double alpha_d = 0.3;

  Mat cvImg, cvImg_sm;
  // convert eigen matrix to opencv matrix
  cvImg = img;
  cvImg_sm = cvImg.clone();

  /*double minVal;
      double maxVal;
      Point minLoc;
      Point maxLoc;*/

  // normalization.
  normalize(cvImg, cvImg, 255.0, 0.0, NORM_MINMAX);

  // remove make intensity value larger than 135
  // ??? need or not?
  /*threshold(cvImg, cvImg, 135, 0, CV_THRESH_TOZERO_INV);
      threshold(cvImg, cvImg, 20, 0, CV_THRESH_TOZERO);
      normalize(cvImg, cvImg, 255.0, 0.0, NORM_MINMAX);*/

  // smoothing
  GaussianBlur(cvImg, cvImg_sm, Size(5, 5), 0, 0);

  // cvImg_sm.type;

  // calculate gradient
  Mat grad2, gradX, gradY;
  Mat gradX2, gradY2;
  Mat edge(width, height, CV_32F);
  // edge.convertTo(edge, CV_16);
  Sobel(cvImg_sm, gradX, CV_16S, 1, 0, 3, 1, 0, BORDER_DEFAULT);
  gradX = gradX / 8;
  Sobel(cvImg_sm, gradY, CV_16S, 0, 1, 3, 1, 0, BORDER_DEFAULT);
  gradY = gradY / 8;
  // grad = gradX * gradX + gradY * gradY;
  // gradX = gradX / 4; gradY = gradY / 4;

  pow(gradX, 2, gradX2);
  pow(gradY, 2, gradY2);

  grad2 = ((gradX2 + gradY2));
  // calculate edge detector as 'g' in matlab code
  // suspecious
  // int temp2;
  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      edge.at<float>(i, j) = 1.0f / float(grad2.at<short>(i, j) + 1);
    }
  }

  // showTestMatImage(edge, "edge");

  // int c0 = 2;
  Mat phi(width, height, CV_32F);
  phi = phi0;

  // remove pixel with intensity value out of range
  /*calcHist();*/

  Mat phin = Mat(width, height, phi.type(), Scalar(0));

  threshold(phi, phi, 0, 255, THRESH_BINARY_INV);

  for (int sl = start; sl < initialLSFs.size(); sl += 2) {
    Mat phis;
    initialLSFs[sl].convertTo(initialLSFs[sl], phi.type());
    multiply(phi, initialLSFs[sl], phis);

    vector<vector<Point>> contours1;
    Mat phiBinary1;

    threshold(phis, phiBinary1, 0, 1, THRESH_BINARY_INV);
    // findContours support this type only
    phiBinary1.convertTo(phiBinary1, CV_8UC1);
    findContours(phiBinary1, contours1, RETR_EXTERNAL, CHAIN_APPROX_NONE,
                 Point(0, 0));
    int size1 = contours1.size();

    if (size1 <= 2) {
      // remove possible non-teeth pixels
      // threshold();
      for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
          if (phis.at<double>(i, j) > 0) {
            if (img_cv.at<double>(i, j) < ranges[sl][2] /*range[0] + 20*/
                /*|| img_cv.at<double>(i, j) < range[2]*/) {
              phis.at<double>(i, j) = 0;
            }
          }
        }
      }

      // fill hole to avoid removing inner parts
      // Mat im_th1;
      // threshold(phis, im_th1, 0, 255, THRESH_BINARY_INV);
      // do not remove inner parts when it is very close to the root
      // phis = fillHole2(im_th1);
      phis = fillHole2(phis);
    }
    phis.convertTo(phis, phin.type());
    bitwise_or(phin, phis, phin);
  }

  normalize(phin, phin, 2.0, -2.0, NORM_MINMAX);
  phin = -1 * phin;
  phi = phin;

  phi0_cv.convertTo(phi0_cv, CV_32F);
  phi.convertTo(phi, CV_32F);
  cuda::GpuMat phi_cuda, edge_cuda, phi0_cuda;
  phi_cuda.upload(phi);
  edge_cuda.upload(edge);
  phi0_cuda.upload(phi0_cv);

  cuda::GpuMat cvImg_sm_cuda;
  cvImg_sm.convertTo(cvImg_sm, CV_32F);
  cvImg_sm_cuda.upload(cvImg_sm);

  // iteration breaking conditions
  for (int i = 0; i < iterOuter; i++) {
    int pre = countNonZero(phi < 0);
    phi_cuda = drlseEdge(phi_cuda, edge_cuda, lambda, mu, alpha, epsilon,
                         timeStep, iterInner, phi0_cuda);
    // phi_cuda = drlseEdgeGraDir(phi_cuda, edge_cuda, lambda, mu, alpha_d,
    // epsilon, timeStep, 	iterInner, phi0_cuda, cvImg_sm_cuda);
    int post = countNonZero(phi < 0);
    if (pre - post <= 3) {
      break;
    }
  }

  alpha = 0;
  int iterRefine = 10;
  phi_cuda = drlseEdge(phi_cuda, edge_cuda, lambda, mu, alpha, epsilon,
                       timeStep, iterRefine, phi0_cuda);
  // phi_cuda = drlseEdgeGraDir(phi_cuda, edge_cuda, lambda, mu, alpha_d,
  // epsilon, timeStep, 	iterRefine, phi0_cuda, cvImg_sm_cuda);
  phi_cuda.download(phi);
  Mat mask = (phi < 0);

  normalize(cvImg, cvImg, 255.0, 0.0, NORM_MINMAX);

  // showTestMatImage(mask, "mask");

  for (int sl = start; sl < initialLSFs.size(); sl += 2) {
    double min = 255, max = 0, average = 0, num = 0;
    Mat ma;
    initialLSFs[sl].convertTo(initialLSFs[sl], mask.type());
    multiply(mask, initialLSFs[sl], ma);

    // showTestMatImage(ma, "mask");

    int numNonZero = countNonZero(ma);
    // double* value = new double[numNonZero];
    vector<double> insideValues;

    for (int i = 0; i < width; i++) {
      for (int j = 0; j < height; j++) {
        if (ma.at<unsigned char>(i, j) != 0) {
          double pixelValue = cvImg.at<double>(i, j);
          insideValues.push_back(pixelValue);
          if (pixelValue < min) {
            min = pixelValue;
          }
          if (pixelValue > max) {
            max = pixelValue;
          }
          num++;
          average += pixelValue;
        }
      }
    }

    sort(insideValues.begin(), insideValues.end());
    int breakId = 0.1 * numNonZero - 1;
    int bottom = 0.05 * numNonZero, top = 0.95 * numNonZero;
    if (numNonZero != 0) {
      ranges[sl][2] = insideValues[breakId];
      /*if (insideValues[top] - insideValues[bottom] < 45) {
                  range[2] = min;
                  }*/
      insideValues.clear();
    } else {
      ranges[sl][2] = 0;
    }

    average /= num;
    ranges[sl][0] = min, ranges[sl][1] = max;
  }

  mask = fillHole(mask);
  threshold(mask, mask, 0, 255, THRESH_BINARY_INV);

  // showTestMatImage(mask, "mask");

  return mask;
}

cuda::GpuMat drlseEdgeGraDir(cuda::GpuMat phi_cuda, const cuda::GpuMat &g_cuda,
                             int lambda, float mu, float alpha, float epsilon,
                             int timestep, int iter,
                             const cuda::GpuMat &phi0_cuda,
                             const cuda::GpuMat &cvImg_sm_cuda,
                             const cuda::GpuMat &last_phi, float beta,
                             const cuda::GpuMat &logPropPart,
                             const cuda::GpuMat &areaPart) {
  // originally take around 1.7s
  // with cuda support around xxxs
  clock_t start;
  float duration;
  start = clock();

  // cuda::GpuMat phi_cuda_out;

  int width, height;
  if (phi_cuda.cols) {
    width = phi_cuda.cols, height = phi_cuda.cols;
  }

  cuda::GpuMat gradX_cuda, gradY_cuda, gradX_sm_cuda, gradY_sm_cuda,
      g_cuda_direction;
  Ptr<cuda::Filter> filterX = cuda::createSobelFilter(
      CV_32F, CV_32F, 1, 0, 3, 1, BORDER_DEFAULT); // x direction
  Ptr<cuda::Filter> filterY = cuda::createSobelFilter(
      CV_32F, CV_32F, 0, 1, 3, 1, BORDER_DEFAULT); // y direction
  filterX->apply(g_cuda, gradX_cuda);
  filterY->apply(g_cuda, gradY_cuda);
  cuda::divide(gradX_cuda, 8, gradX_cuda);
  cuda::divide(gradY_cuda, 8, gradY_cuda);

  Ptr<cuda::Filter> filterX_sm = cuda::createSobelFilter(
      CV_32F, CV_32F, 1, 0, 3, 1, BORDER_DEFAULT); // x direction
  Ptr<cuda::Filter> filterY_sm = cuda::createSobelFilter(
      CV_32F, CV_32F, 0, 1, 3, 1, BORDER_DEFAULT); // y direction
  filterX_sm->apply(cvImg_sm_cuda, gradX_sm_cuda);
  filterY_sm->apply(cvImg_sm_cuda, gradY_sm_cuda);
  cuda::divide(gradX_sm_cuda, 8, gradX_sm_cuda);
  cuda::divide(gradY_sm_cuda, 8, gradY_sm_cuda);

  cuda::GpuMat directions, temp;
  for (int time = 0; time < iter; time++) {
    cuda::GpuMat phiX_cuda, phiY_cuda;
    cuda::GpuMat phiX2_cuda, phiY2_cuda;
    int type1 = phi_cuda.type();
    int type2 = phiX_cuda.type();
    filterX->apply(phi_cuda, phiX_cuda);
    filterY->apply(phi_cuda, phiY_cuda);
    cuda::divide(phiX_cuda, 8, phiX_cuda);
    cuda::divide(phiY_cuda, 8, phiY_cuda);

    cuda::multiply(phiX_cuda, gradX_sm_cuda, directions);
    cuda::multiply(phiY_cuda, gradY_sm_cuda, temp);
    cuda::add(directions, temp, directions);
    cuda::multiply(directions, float(-1), directions);
    /*cuda::threshold(directions, temp, 0, 1, THRESH_BINARY_INV);*/
    cuda::threshold(directions, directions, 0, 1, THRESH_BINARY);
    cuda::threshold(directions, temp, 0, 1, THRESH_BINARY_INV);

    cuda::multiply(directions, g_cuda, directions);
    cuda::add(directions, temp, g_cuda_direction);

    cuda::GpuMat s_cuda, phiX2Y2sum_cuda;
    cuda::pow(phiX_cuda, 2, phiX2_cuda);
    cuda::pow(phiY_cuda, 2, phiY2_cuda);
    cuda::add(phiX2_cuda, phiY2_cuda, phiX2Y2sum_cuda);
    cuda::pow(phiX2Y2sum_cuda, 0.5, s_cuda);

    float smallNumber = 1e-10;
    cuda::GpuMat Nx_cuda, Ny_cuda;
    cuda::add(s_cuda, smallNumber, s_cuda);
    cuda::divide(phiX_cuda, s_cuda, Nx_cuda);
    cuda::divide(phiY_cuda, s_cuda, Ny_cuda);

    cuda::GpuMat curvature_cuda = div(Nx_cuda, Ny_cuda, filterX, filterY);

    cuda::GpuMat distRegTerm_cuda = distReg_p2(phi_cuda, filterX, filterY);

    cuda::GpuMat diracPhi_cuda = dirac(phi_cuda, epsilon);

    cuda::GpuMat areaTerm_cuda;
    // cuda::multiply(diracPhi_cuda, g_cuda, areaTerm_cuda);
    // cuda::multiply(g_cuda, alpha, temp);
    // cuda::add(temp, areaPart, areaTerm_cuda);
    // cuda::multiply(diracPhi_cuda, areaTerm_cuda, areaTerm_cuda);
    cuda::add(logPropPart, areaPart, areaTerm_cuda);
    cuda::multiply(diracPhi_cuda, areaTerm_cuda, areaTerm_cuda);

    cuda::GpuMat edgeTerm_cuda, first_cuda, second_cuda, third_cuda;
    cuda::multiply(gradX_cuda, Nx_cuda, first_cuda);
    cuda::multiply(gradY_cuda, Ny_cuda, second_cuda);
    cuda::add(first_cuda, second_cuda, first_cuda);
    cuda::multiply(diracPhi_cuda, first_cuda, first_cuda);
    // cuda::multiply(g_cuda, curvature_cuda, second_cuda);
    cuda::multiply(g_cuda_direction, curvature_cuda, second_cuda);
    cuda::multiply(diracPhi_cuda, second_cuda, second_cuda);
    cuda::add(first_cuda, second_cuda, edgeTerm_cuda);
    cuda::multiply(diracPhi_cuda, curvature_cuda, third_cuda);
    cuda::multiply(third_cuda, last_phi, third_cuda);
    cuda::multiply(third_cuda, last_phi, third_cuda);
    cuda::multiply(third_cuda, beta, third_cuda);
    cuda::add(edgeTerm_cuda, third_cuda, edgeTerm_cuda);

    cuda::GpuMat binaryPhi_cuda, binaryPhi0_cuda;
    cuda::threshold(phi_cuda, binaryPhi_cuda, -0.0001, 1, THRESH_BINARY);
    cuda::threshold(phi0_cuda, binaryPhi0_cuda, -0.0001, 1, THRESH_BINARY);
    cuda::GpuMat shapeTerm_cuda;
    cuda::multiply(binaryPhi0_cuda, -1, binaryPhi0_cuda);
    cuda::add(binaryPhi_cuda, binaryPhi0_cuda, shapeTerm_cuda);
    cuda::abs(shapeTerm_cuda, shapeTerm_cuda);

    cuda::multiply(distRegTerm_cuda, float(mu), distRegTerm_cuda);
    cuda::multiply(edgeTerm_cuda, float(lambda), edgeTerm_cuda);
    // cuda::multiply(areaTerm_cuda, float(alpha), areaTerm_cuda);
    cuda::multiply(shapeTerm_cuda, float(-0.01 * alpha), shapeTerm_cuda);
    cuda::GpuMat termsSum_cuda(width, height, CV_32F);
    cuda::add(termsSum_cuda, distRegTerm_cuda, termsSum_cuda);
    cuda::add(termsSum_cuda, edgeTerm_cuda, termsSum_cuda);
    cuda::add(termsSum_cuda, areaTerm_cuda, termsSum_cuda);
    cuda::add(termsSum_cuda, shapeTerm_cuda, termsSum_cuda);
    cuda::multiply(float(timestep), termsSum_cuda, termsSum_cuda);
    cuda::add(phi_cuda, termsSum_cuda, phi_cuda);
  }
  duration = (std::clock() - start) / (float)CLOCKS_PER_SEC;

  return phi_cuda;
}

cuda::GpuMat drlseEdge(cuda::GpuMat phi_cuda, const cuda::GpuMat &g_cuda,
                       int lambda, float mu, float alpha, float epsilon,
                       int timestep, int iter, const cuda::GpuMat &phi0_cuda) {
  // originally take around 1.7s
  // with cuda support around xxxs
  clock_t start;
  float duration;
  start = clock();

  int width, height;
  if (phi_cuda.cols) {
    // width = phi_cuda.cols, height = phi_cuda.cols;
    width = phi_cuda.rows;
    height = phi_cuda.cols;
  }

  cuda::GpuMat gradX_cuda, gradY_cuda;
  Ptr<cuda::Filter> filterX = cuda::createSobelFilter(
      CV_32F, CV_32F, 1, 0, 3, 1, BORDER_DEFAULT); // x direction
  Ptr<cuda::Filter> filterY = cuda::createSobelFilter(
      CV_32F, CV_32F, 0, 1, 3, 1, BORDER_DEFAULT); // y direction
  filterX->apply(g_cuda, gradX_cuda);
  filterY->apply(g_cuda, gradY_cuda);
  cuda::divide(gradX_cuda, 8, gradX_cuda);
  cuda::divide(gradY_cuda, 8, gradY_cuda);

  for (int time = 0; time < iter; time++) {
    phi_cuda = neumannBoundCond(phi_cuda);
    cuda::GpuMat phiX_cuda, phiY_cuda;
    cuda::GpuMat phiX2_cuda, phiY2_cuda;
    int type1 = phi_cuda.type();
    int type2 = phiX_cuda.type();
    filterX->apply(phi_cuda, phiX_cuda);
    filterY->apply(phi_cuda, phiY_cuda);
    cuda::divide(phiX_cuda, 8, phiX_cuda);
    cuda::divide(phiY_cuda, 8, phiY_cuda);

    cuda::GpuMat s_cuda, phiX2Y2sum_cuda;
    cuda::pow(phiX_cuda, 2, phiX2_cuda);
    cuda::pow(phiY_cuda, 2, phiY2_cuda);
    cuda::add(phiX2_cuda, phiY2_cuda, phiX2Y2sum_cuda);
    cuda::pow(phiX2Y2sum_cuda, 0.5, s_cuda);

    float smallNumber = 1e-10;
    cuda::GpuMat Nx_cuda, Ny_cuda;
    cuda::add(s_cuda, smallNumber, s_cuda);
    cuda::divide(phiX_cuda, s_cuda, Nx_cuda);
    cuda::divide(phiY_cuda, s_cuda, Ny_cuda);

    cuda::GpuMat curvature_cuda = div(Nx_cuda, Ny_cuda, filterX, filterY);

    cuda::GpuMat distRegTerm_cuda = distReg_p2(phi_cuda, filterX, filterY);

    cuda::GpuMat diracPhi_cuda = dirac(phi_cuda, epsilon);

    cuda::GpuMat areaTerm_cuda;
    cuda::multiply(diracPhi_cuda, g_cuda, areaTerm_cuda);

    cuda::GpuMat edgeTerm_cuda, first_cuda, second_cuda;
    cuda::multiply(gradX_cuda, Nx_cuda, first_cuda);
    cuda::multiply(gradY_cuda, Ny_cuda, second_cuda);
    cuda::add(first_cuda, second_cuda, first_cuda);
    cuda::multiply(diracPhi_cuda, first_cuda, first_cuda);
    cuda::multiply(g_cuda, curvature_cuda, second_cuda);
    cuda::multiply(diracPhi_cuda, second_cuda, second_cuda);
    cuda::add(first_cuda, second_cuda, edgeTerm_cuda);

    // cuda::GpuMat binaryPhi_cuda, binaryPhi0_cuda;
    // cuda::threshold(phi_cuda, binaryPhi_cuda, -0.0001, 1, THRESH_BINARY);
    // cuda::threshold(phi0_cuda, binaryPhi0_cuda, -0.0001, 1, THRESH_BINARY);
    // cuda::GpuMat shapeTerm_cuda;
    // cuda::multiply(binaryPhi0_cuda, -1, binaryPhi0_cuda);
    //   cuda::add(binaryPhi_cuda, binaryPhi0_cuda, shapeTerm_cuda);
    // cuda::abs(shapeTerm_cuda, shapeTerm_cuda);

    cuda::multiply(distRegTerm_cuda, float(mu), distRegTerm_cuda);
    cuda::multiply(edgeTerm_cuda, float(lambda), edgeTerm_cuda);
    cuda::multiply(areaTerm_cuda, float(alpha), areaTerm_cuda);
    // cuda::multiply(shapeTerm_cuda, float(-0.01 * alpha), shapeTerm_cuda);
    cuda::GpuMat termsSum_cuda(width, height, CV_32F, Scalar(0));
    cuda::add(termsSum_cuda, distRegTerm_cuda, termsSum_cuda);
    cuda::add(termsSum_cuda, edgeTerm_cuda, termsSum_cuda);
    cuda::add(termsSum_cuda, areaTerm_cuda, termsSum_cuda);
    // cuda::add(termsSum_cuda, shapeTerm_cuda, termsSum_cuda);
    cuda::multiply(float(timestep), termsSum_cuda, termsSum_cuda);
    cuda::add(phi_cuda, termsSum_cuda, phi_cuda);
  }
  duration = (std::clock() - start) / (float)CLOCKS_PER_SEC;

  return phi_cuda;
}

cuda::GpuMat div(const cuda::GpuMat &nx, const cuda::GpuMat &ny,
                 const Ptr<cuda::Filter> &filterX,
                 const Ptr<cuda::Filter> &filterY) {
  cuda::GpuMat nxx, nyy, sum;
  filterX->apply(nx, nxx);
  filterY->apply(ny, nyy);
  cuda::divide(nxx, 8, nxx);
  cuda::divide(nyy, 8, nyy);
  cuda::add(nxx, nyy, sum);

  return sum;
}

// to be checked further
cuda::GpuMat distReg_p2(const cuda::GpuMat &phi_cuda,
                        const Ptr<cuda::Filter> &filterX,
                        const Ptr<cuda::Filter> &filterY) {
  int width, height;
  if (phi_cuda.cols) {
    // width = phi_cuda.cols, height = phi_cuda.cols;
    width = phi_cuda.rows, height = phi_cuda.cols;
  }

  cuda::GpuMat phiX_cuda, phiY_cuda, phiX2_cuda, phiY2_cuda;
  cuda::GpuMat s_cuda, ps_cuda(width, height, CV_32F),
      dps_cuda(width, height, CV_32F);
  cuda::GpuMat a_cuda, b_cuda;
  cuda::GpuMat phiX2Y2sum_cuda;
  float pi = 3.1416;

  // gradient calculation
  filterX->apply(phi_cuda, phiX_cuda);
  filterY->apply(phi_cuda, phiY_cuda);
  cuda::divide(phiX_cuda, 8, phiX_cuda);
  cuda::divide(phiY_cuda, 8, phiY_cuda);

  // s calculation
  cuda::pow(phiX_cuda, 2, phiX2_cuda);
  cuda::pow(phiY_cuda, 2, phiY2_cuda);
  cuda::add(phiX2_cuda, phiY2_cuda, phiX2Y2sum_cuda);
  cuda::pow(phiX2Y2sum_cuda, 0.5, s_cuda);

  // a and b calculation
  cuda::threshold(s_cuda, a_cuda, 1, 1, THRESH_BINARY_INV);
  cuda::threshold(s_cuda, b_cuda, 1, 1, THRESH_BINARY);

  // ps calculation (attension: loss due to unaccurate sine calculation)
  cuda::GpuMat first_cuda, sinx_cuda, pow3_cuda;
  cuda::GpuMat second_cuda, sm_cuda;
  cuda::multiply(s_cuda, 2 * pi, sinx_cuda);
  sinx_cuda = sine(sinx_cuda);
  cuda::multiply(sinx_cuda, a_cuda, first_cuda);
  cuda::divide(first_cuda, 2 * pi, first_cuda);
  cuda::add(s_cuda, -1, sm_cuda);
  cuda::multiply(b_cuda, sm_cuda, second_cuda);
  cuda::add(first_cuda, second_cuda, ps_cuda);

  // calculation below need to be improved
  cuda::GpuMat psMask0_cuda, psMask1_cuda, sMask0_cuda, sMask1_cuda;
  cuda::GpuMat psInv_cuda, sInv_cuda;
  cuda::GpuMat tempPos_cuda, tempNeg_cuda;
  cuda::multiply(ps_cuda, -1, psInv_cuda);
  cuda::multiply(s_cuda, -1, sInv_cuda);

  float zero = 0; // avoid loss of sine calculation
  cuda::threshold(ps_cuda, tempPos_cuda, zero, 1, THRESH_BINARY);
  cuda::threshold(psInv_cuda, tempNeg_cuda, zero, 1, THRESH_BINARY);
  cuda::bitwise_and(tempPos_cuda, tempNeg_cuda, psMask1_cuda);
  cuda::threshold(psMask1_cuda, psMask0_cuda, 0, 1, THRESH_BINARY_INV);

  cuda::threshold(s_cuda, tempPos_cuda, zero, 1, THRESH_BINARY);
  cuda::threshold(sInv_cuda, tempNeg_cuda, zero, 1, THRESH_BINARY);
  cuda::bitwise_and(tempPos_cuda, tempNeg_cuda, sMask1_cuda);
  cuda::threshold(sMask1_cuda, sMask0_cuda, 0, 1, THRESH_BINARY_INV);
  // calculation above need to be improved

  cuda::multiply(psMask1_cuda, ps_cuda, first_cuda);
  cuda::add(first_cuda, psMask0_cuda, first_cuda);
  cuda::multiply(sMask1_cuda, s_cuda, second_cuda);
  cuda::add(second_cuda, sMask0_cuda, second_cuda);
  cuda::divide(1, second_cuda, second_cuda);
  cuda::multiply(first_cuda, second_cuda, dps_cuda);

  // laplacian calculation
  cuda::GpuMat phiLap_cuda;
  Ptr<cuda::Filter> filterLap =
      cuda::createLaplacianFilter(CV_32F, CV_32F, 3, 1, 0, BORDER_DEFAULT);
  filterLap->apply(phi_cuda, phiLap_cuda);
  cuda::divide(phiLap_cuda, 4, phiLap_cuda);

  cuda::GpuMat f;
  cuda::multiply(dps_cuda, phiX_cuda, first_cuda);
  cuda::multiply(phiX_cuda, -1, phiX_cuda);
  cuda::add(first_cuda, phiX_cuda, first_cuda);
  cuda::multiply(dps_cuda, phiY_cuda, second_cuda);
  cuda::multiply(phiY_cuda, -1, phiY_cuda);
  cuda::add(second_cuda, phiY_cuda, second_cuda);
  cuda::divide(first_cuda, second_cuda, first_cuda);
  cuda::add(first_cuda, phiLap_cuda, f);

  return f;
}

// to be checked further
cuda::GpuMat dirac(const cuda::GpuMat &x_cuda, float sigma) {
  // Mat x; x_cuda.download(x);

  float pi = 3.14159;

  cuda::GpuMat f_cuda, xCos_cuda;
  cuda::multiply(pi / sigma, x_cuda, xCos_cuda);
  xCos_cuda = cosine(xCos_cuda);
  cuda::add(1, xCos_cuda, xCos_cuda);
  cuda::multiply((1.0 / (2.0 * sigma)), xCos_cuda, f_cuda);

  // to be improved
  cuda::GpuMat rangeToLeft_cuda, rangeToRight_cuda, range_cuda;
  cuda::threshold(x_cuda, rangeToLeft_cuda, sigma, 1, THRESH_BINARY_INV);
  cuda::threshold(x_cuda, rangeToRight_cuda, -sigma, 1, THRESH_BINARY);
  cuda::bitwise_and(rangeToLeft_cuda, rangeToRight_cuda, range_cuda);

  cuda::GpuMat result;
  cuda::multiply(f_cuda, range_cuda, result);
  return result;
}

cuda::GpuMat heaviside(const cuda::GpuMat &x_cuda, float sigma) {
  Mat x;
  x_cuda.download(x);

  float pi = 3.14159;

  cuda::GpuMat f_cuda, xSin_cuda, xSigma_cuda;
  cuda::multiply(pi / sigma, x_cuda, xSin_cuda);
  xSin_cuda = sine(xSin_cuda);
  cuda::multiply(1.0f / pi, xSin_cuda, xSin_cuda);
  cuda::multiply(1.0f / sigma, xSin_cuda, xSigma_cuda);
  cuda::add(1, xSin_cuda, xSin_cuda);
  cuda::add(xSigma_cuda, xSin_cuda, xSin_cuda);
  // cuda::multiply((1 / 2), xSin_cuda, f_cuda);
  cuda::multiply(0.5f, xSin_cuda, f_cuda);

  // to be improved
  cuda::GpuMat rangeToLeft_cuda, rangeToRight_cuda, range_cuda;
  cuda::threshold(x_cuda, rangeToLeft_cuda, sigma, 1, THRESH_BINARY_INV);
  cuda::threshold(x_cuda, rangeToRight_cuda, -sigma, 1, THRESH_BINARY);
  cuda::bitwise_and(rangeToLeft_cuda, rangeToRight_cuda, range_cuda);

  cuda::GpuMat result;
  cuda::multiply(f_cuda, range_cuda, result);

  cuda::threshold(x_cuda, rangeToRight_cuda, sigma, 1, THRESH_BINARY);
  cuda::add(result, rangeToRight_cuda, result);
  return result;
}

cuda::GpuMat neumannBoundCond(cuda::GpuMat f) {
  // optional, to be done
  typedef float TYPE;
  Mat phi;
  f.download(phi);
  int width = phi.rows, height = phi.cols;
  phi.at<TYPE>(0, 0) = phi.at<TYPE>(2, 2);
  phi.at<TYPE>(0, height - 1) = phi.at<TYPE>(2, height - 3);
  phi.at<TYPE>(width - 1, 0) = phi.at<TYPE>(width - 3, 2);
  phi.at<TYPE>(width - 1, height - 1) = phi.at<TYPE>(width - 3, height - 3);
  for (int i = 1; i < height - 1; i++) {
    phi.at<TYPE>(0, i) = phi.at<TYPE>(2, i);
    phi.at<TYPE>(width - 1, i) = phi.at<TYPE>(width - 3, i);
  }
  for (int i = 1; i < width - 1; i++) {
    phi.at<TYPE>(i, 0) = phi.at<TYPE>(i, 2);
    phi.at<TYPE>(i, height - 1) = phi.at<TYPE>(i, height - 3);
  }
  f.upload(phi);
  return f;
}

// According to tylor series: sine(x) = x - (x ^ 3) / 3! + (x ^ 5) / 5! - (x ^
// 7) / 7!
cuda::GpuMat sine(const cuda::GpuMat &x_cuda) {
  cuda::GpuMat sinx_cuda, pow2_cuda, pow3_cuda, pow5_cuda, pow7_cuda, pow9_cuda;

  cuda::pow(x_cuda, 2, pow2_cuda);
  cuda::multiply(x_cuda, pow2_cuda, pow3_cuda);
  cuda::multiply(pow3_cuda, pow2_cuda, pow5_cuda);
  cuda::multiply(pow5_cuda, pow2_cuda, pow7_cuda);
  cuda::multiply(pow7_cuda, pow2_cuda, pow9_cuda);

  cuda::divide(pow3_cuda, -6, pow3_cuda);
  cuda::add(x_cuda, pow3_cuda, sinx_cuda);

  cuda::divide(pow5_cuda, 120, pow5_cuda);
  cuda::add(sinx_cuda, pow5_cuda, sinx_cuda);

  cuda::divide(pow7_cuda, -5040, pow7_cuda);
  cuda::add(sinx_cuda, pow7_cuda, sinx_cuda);

  cuda::divide(pow9_cuda, 362880, pow9_cuda);
  cuda::add(sinx_cuda, pow9_cuda, sinx_cuda);

  return sinx_cuda;
}

// According to: cosine(x) = sine(pi / 2 - x)
// No fuck above, use taylor series directly
cuda::GpuMat cosine(const cuda::GpuMat &x_cuda) {
  cuda::GpuMat cosx_cuda, pow2_cuda, pow4_cuda, pow6_cuda, pow8_cuda;

  cuda::pow(x_cuda, 2, pow2_cuda);
  cuda::multiply(pow2_cuda, pow2_cuda, pow4_cuda);
  cuda::multiply(pow4_cuda, pow2_cuda, pow6_cuda);
  cuda::multiply(pow6_cuda, pow2_cuda, pow8_cuda);

  cuda::divide(pow2_cuda, -2, pow2_cuda);
  cuda::add(1, pow2_cuda, cosx_cuda);

  cuda::divide(pow4_cuda, 24, pow4_cuda);
  cuda::add(cosx_cuda, pow4_cuda, cosx_cuda);

  cuda::divide(pow6_cuda, -720, pow6_cuda);
  cuda::add(cosx_cuda, pow6_cuda, cosx_cuda);

  cuda::divide(pow8_cuda, 40320, pow8_cuda);
  cuda::add(cosx_cuda, pow8_cuda, cosx_cuda);

  return cosx_cuda;
}

// fill holes in image
Mat fillHole(const Mat &img) {
  Mat im_th;
  img.convertTo(im_th, CV_8U);
  Mat im_floodfill = im_th.clone();
  floodFill(im_floodfill, Point(0, 0), Scalar(255));
  Mat im_floodfill_inv;
  bitwise_not(im_floodfill, im_floodfill_inv);
  Mat im_out = (im_th | im_floodfill_inv);
  im_out.convertTo(im_out, CV_32F);
  Mat phi = im_out.clone();
  normalize(phi, phi, 2.0, -2.0, NORM_MINMAX);
  phi = -1 * phi;
  return phi;
}

// fill holes in image
Mat fillHole2(const Mat &img) {
  Mat im_th;
  img.convertTo(im_th, CV_8U);
  Mat im_floodfill = im_th.clone();
  floodFill(im_floodfill, Point(0, 0), Scalar(255));
  Mat im_floodfill_inv;
  bitwise_not(im_floodfill, im_floodfill_inv);
  Mat im_out = (im_th | im_floodfill_inv);
  im_out.convertTo(im_out, CV_32F);
  // Mat phi = im_out.clone();
  // normalize(phi, phi, 2.0, -2.0, NORM_MINMAX);
  // phi = -1 * phi;
  // return phi;
  return im_out;
}

// In ideal condition, obj_mask must be in total_mask
// But in reality, obj_mask can be partly out of total_mask
// To simplify calculation, use 'obj_mask_in_total' replace 'obj_mask'
void getPriorIntensityProp(const cuda::GpuMat &img,
                           const cuda::GpuMat &total_mask,
                           const cuda::GpuMat &obj_mask,
                           std::vector<float> &backProp,
                           std::vector<float> &objProp, int max_intensity) {
  int rows = img.rows, cols = img.cols;
  // showTestGpuMatImage(img, "img_before");
  // showTestGpuMatImage(total_mask, "total_mask");
  cuda::GpuMat img_mask, obj_mask_in_total;
  cuda::multiply(img, total_mask, img_mask);
  double total_pixels = cuda::sum(total_mask).val[0];
  // unsigned long zero_counts = rows * cols - total_pixels;
  cuda::GpuMat obj_intensitys;
  cuda::bitwise_and(total_mask, obj_mask, obj_mask_in_total);
  cuda::multiply(img, obj_mask_in_total, obj_intensitys);
  cuda::GpuMat back_intensitys, temp, temp2, back_mask;
  cuda::bitwise_xor(total_mask, obj_mask_in_total, back_mask);
  cuda::bitwise_and(total_mask, back_mask, back_mask);
  cuda::multiply(img, back_mask, back_intensitys);

  // showTestGpuMatImage(img, "img_after");
  // showTestGpuMatImage(obj_intensitys, "obj");
  // showTestGpuMatImage(back_mask, "back_mask");
  // showTestGpuMatImage(back_intensitys, "back");

  unsigned long zero_out_back = rows * cols - cuda::sum(back_mask).val[0];
  unsigned long zero_out_obj =
      rows * cols - cuda::sum(obj_mask_in_total).val[0];

  // double temp3, temp4;
  for (int i = 0; i <= max_intensity; i++) {
    cuda::threshold(obj_intensitys, temp, double(i - 1), 1, THRESH_BINARY);
    cuda::threshold(obj_intensitys, temp2, double(i), 1, THRESH_BINARY_INV);
    cuda::bitwise_and(temp, temp2, temp);
    // if (i != 0)
    objProp[i] = double(cuda::sum(temp).val[0]) / total_pixels;
    // else objProp[i] = cuda::sum(temp).val[0];

    cuda::threshold(back_intensitys, temp, double(i - 1), 1, THRESH_BINARY);
    cuda::threshold(back_intensitys, temp2, double(i), 1, THRESH_BINARY_INV);
    cuda::bitwise_and(temp, temp2, temp);
    // if (i != 0)
    backProp[i] = double(cuda::sum(temp).val[0]) / total_pixels;
    // else backProp[i] = cuda::sum(temp).val[0];
  }
  // backProp[0] = double(backProp[0] - zero_out_back) / double(total_pixels);
  // objProp[0] = double(objProp[0] - zero_out_obj) / double(total_pixels);
  backProp[0] -= double(zero_out_back) / total_pixels;
  objProp[0] -= double(zero_out_obj) / total_pixels;
  backProp[0] = (backProp[0] > 0.0) ? backProp[0] : 0.0;
  objProp[0] = (objProp[0] > 0.0) ? objProp[0] : 0.0;

  // histogram flipping
  vector<float>::iterator max = max_element(begin(objProp), end(objProp));
  float max_val = *max;
  int max_index = distance(begin(objProp), max);
  if (max_index < max_intensity) {
    for (int i = max_index + 1; i <= max_intensity; i++) {
      objProp[i] = 2 * max_val - objProp[i];
    }
  }
}

void getPriorIntensityProp_q(const cuda::GpuMat &img,
                             const cuda::GpuMat &total_mask,
                             const cuda::GpuMat &obj_mask,
                             std::vector<float> &backProp,
                             std::vector<float> &objProp, int max_intensity) {
  int rows = img.rows, cols = img.cols;
  cuda::GpuMat img_mask, obj_mask_in_total;
  cuda::multiply(img, total_mask, img_mask);
  double total_pixels = cuda::sum(total_mask).val[0];
  cuda::GpuMat obj_intensitys;
  cuda::bitwise_and(total_mask, obj_mask, obj_mask_in_total);
  cuda::multiply(img, obj_mask_in_total, obj_intensitys);
  cuda::GpuMat back_intensitys, temp, temp2, back_mask;
  cuda::bitwise_xor(total_mask, obj_mask_in_total, back_mask);
  cuda::bitwise_and(total_mask, back_mask, back_mask);
  cuda::multiply(img, back_mask, back_intensitys);

  unsigned long zero_out_back = rows * cols - cuda::sum(back_mask).val[0];
  unsigned long zero_out_obj =
      rows * cols - cuda::sum(obj_mask_in_total).val[0];

  cuda::GpuMat obj_histogram, back_histogram;
  Mat hist;

  // cuda::calcHist(total_mask, total_histogram);
  // total_histogram.download(hist);
  // hist = hist.reshape(1, max_intensity + 1);
  // if (hist.isContinuous()) {
  //	objProp.assign((float*)hist.datastart, (float*)hist.dataend);
  //}

  cuda::calcHist(obj_intensitys, obj_histogram);
  obj_histogram.download(hist);
  hist = hist.reshape(1, max_intensity + 1);
  if (hist.isContinuous()) {
    objProp.assign((float *)hist.datastart, (float *)hist.dataend);
  }

  cuda::calcHist(back_intensitys, back_histogram);
  back_histogram.download(hist);
  hist = hist.reshape(1, max_intensity + 1);
  if (hist.isContinuous()) {
    backProp.assign((float *)hist.datastart, (float *)hist.dataend);
  }
  // else {
  //	//objProp.insert(objProp.end(), hist.ptr<float>(0), hist.ptr<float>(0) +
  // max_intensity+1);
  //}
  objProp[0] = 0.0;
  backProp[0] = 0.0;
  // histogram flipping
  vector<float>::iterator max = max_element(begin(objProp), end(objProp));
  float max_val = *max;

  for (int i = 0; i < max_intensity; i++) {
    objProp[i] /= max_val;
    backProp[i] /= max_val;
  }
  vector<float> temp_back = vector<float>(backProp);
  vector<float> temp_obj = vector<float>(objProp);
  for (int i = 1; i < max_intensity - 1; i++) {
    if (backProp[i] == 0)
      backProp[i] = (temp_back[i - 1] + temp_back[i + 1]) / 2;
    if (objProp[i] == 0)
      objProp[i] = (temp_obj[i - 1] + temp_obj[i + 1]) / 2;
  }
  max_val = 1;
  int max_index = distance(begin(objProp), max);
  if (max_index < max_intensity) {
    for (int i = max_index + 1; i <= max_intensity; i++) {
      objProp[i] = 2 * max_val - objProp[i];
    }
  }
}

cuda::GpuMat getSelectedSDF(std::vector<cuda::GpuMat> phis, bool even,
                            float c0) {
  Mat total_phi = Mat(phis[0].rows, phis[0].cols, CV_32F, Scalar(c0));
  cuda::GpuMat total_phi_cuda, temp1, temp2, minus_total_phi_cuda, temp3, temp4;
  total_phi_cuda.upload(total_phi);
  int size = phis.size();
  for (int i = 0; i < size; i++) {
    if (even && i % 2 == 0 || !even && i % 2 != 0) {
      cuda::multiply(total_phi, float(-1), minus_total_phi_cuda);
      cuda::add(phis[i], minus_total_phi_cuda, temp1);
      cuda::threshold(temp1, temp2, 0, 1,
                      THRESH_BINARY); // temp2 = (phis[i]-total_phi_cuda) > 0
      cuda::threshold(temp2, temp1, 0, 1, THRESH_BINARY_INV); // temp1 = ~temp2
      cuda::multiply(temp2, total_phi_cuda, temp3);
      cuda::multiply(temp1, phis[i], temp4);
      cuda::add(temp3, temp4, total_phi_cuda);
    }
  }
  return total_phi_cuda;
}

cuda::GpuMat phi2Mask(const cuda::GpuMat &phi) {
  cuda::GpuMat mask;
  cuda::threshold(phi, mask, 0, 1, THRESH_BINARY);
  cuda::threshold(mask, mask, 0, 1, THRESH_BINARY_INV);
  return mask;
}

cuda::GpuMat getPropMat(Mat img, const cuda::GpuMat &total_mask,
                        std::vector<float> &intensity_prop) {
  int rows = img.rows, cols = img.cols;
  Mat proImg = Mat(rows, cols, CV_32F, Scalar(0));
  cuda::GpuMat proImg_cuda;
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      float temp1 = img.at<float>(i, j);
      proImg.at<float>(i, j) = intensity_prop[int(img.at<float>(i, j))];
    }
  }
  proImg_cuda.upload(proImg);
  cuda::multiply(total_mask, proImg_cuda, proImg_cuda);
  return proImg_cuda;
}

std::vector<Mat> segSliceCoupled(Mat phi0_1, Mat phi0_2, Mat img,
                                 vector<vector<double>> &ranges,
                                 vector<Mat> initialLSFs,
                                 const Mat &total_mask1, const Mat &total_mask2,
                                 bool crown, bool flag) {
  /*if (flag) {
              showTestMatMask(phi0_1, "phi0_1");
              showTestMatMask(phi0_2, "phi0_2");
      }*/
  // imshow("bitch", phi0); waitKey();
  int width, height;
  if (img.cols) {
    width = img.cols, height = img.rows;
  }
  // convert Mat to CV_32F with one channel
  // cvtColor(img, img, CV_BGR2GRAY);

  img.convertTo(img, CV_64F);
  phi0_1.convertTo(phi0_1, CV_64F);
  phi0_2.convertTo(phi0_2, CV_64F);
  double minVal;
  double maxVal;
  Point minLoc;
  Point maxLoc;
  minMaxLoc(img, &minVal, &maxVal, &minLoc, &maxLoc);

  int type = img.type();
  // type = phi0_1.type();
  // type = prior1.type();

  Mat phi01_cv, phi02_cv, img_cv;
  phi01_cv = phi0_1;
  phi02_cv = phi0_2;
  img_cv = img;

  // parameters setting
  int timeStep = 5;
  double mu =
      0.2 /
      double(
          timeStep); // coefficient of the distance regularization term R(phi)
  int iterInner = 5;
  int iterOuter = 40; // 40
  // int lambda = 5; // coefficient of the weighted length term L(phi)
  double alpha = 0.3; // coefficient of the weighted area term A(phi)
  double epsilon =
      1.5; // papramater that specifies the width of the DiracDelta function

  double sigma = 1.5; // scale parameter in Gaussian kernel
  // double alpha_d = 2; 0.8
  double alpha_d = 0.8;
  double v_s = 0.3;
  double v_e = 0.02;
  double beta_d = 0.02;
  // double beta_d = 0;
  int lambda_d = 5;
  // int lambda_d = 8;

  if (!crown) {
    v_s = 0.1;
    // alpha_d = 0;
    // v_e = 0.02;
    lambda_d = 1;
  }

  Mat cvImg, cvImg_sm;
  // convert eigen matrix to opencv matrix
  cvImg = img;
  cvImg_sm = cvImg.clone();

  // normalization.
  normalize(cvImg, cvImg, 255.0, 0.0, NORM_MINMAX);

  // smoothing
  GaussianBlur(cvImg, cvImg_sm, Size(5, 5), 0, 0);

  // calculate gradient
  Mat grad2, gradX, gradY;
  Mat gradX2, gradY2;
  Mat edge(width, height, CV_32F);
  // edge.convertTo(edge, CV_16);
  Sobel(cvImg_sm, gradX, CV_16S, 1, 0, 3, 1, 0, BORDER_DEFAULT);
  gradX = gradX / 8;
  Sobel(cvImg_sm, gradY, CV_16S, 0, 1, 3, 1, 0, BORDER_DEFAULT);
  gradY = gradY / 8;

  pow(gradX, 2, gradX2);
  pow(gradY, 2, gradY2);
  grad2 = ((gradX2 + gradY2));
  // calculate edge detector as 'g' in matlab code
  // suspecious
  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      edge.at<float>(width * i + j) =
          1.0f / float(grad2.at<short>(width * i + j) + 1);
    }
  }

  // int c0 = 2;
  Mat phi1(width, height, CV_32F);
  phi1 = phi0_1;
  Mat phi2(width, height, CV_32F);
  phi2 = phi0_2;

  // remove pixel with intensity value out of range
  // imshow("before", phi1);
  // waitKey();

  Mat phi1n = Mat(width, height, phi1.type(), Scalar(0)),
      phi2n = Mat(width, height, phi1.type(), Scalar(0));

  // phi1 = -phi1;
  // phi2 = -phi2;
  threshold(phi1, phi1, 0, 255, THRESH_BINARY_INV);
  threshold(phi2, phi2, 0, 255, THRESH_BINARY_INV);

  // showTestMatImage(phi1, "phi1");
  // showTestMatImage(phi2, "phi2");

  for (int sl = 0; sl < initialLSFs.size(); sl++) {
    Mat phis;
    initialLSFs[sl].convertTo(initialLSFs[sl], phi1.type());
    if (sl % 2 == 0)
      multiply(phi1, initialLSFs[sl], phis);
    else
      multiply(phi2, initialLSFs[sl], phis);

    // showTestMatImage(phis, "phis");
    // showTestMatImage(initialLSFs[sl], "initi");
    vector<vector<Point>> contours1;
    Mat phiBinary1;

    threshold(phis, phiBinary1, 0, 1, THRESH_BINARY_INV);
    // findContours support this type only
    phiBinary1.convertTo(phiBinary1, CV_8UC1);
    findContours(phiBinary1, contours1, RETR_EXTERNAL, CHAIN_APPROX_NONE,
                 Point(0, 0));
    int size1 = contours1.size();

    if (size1 <= 2) {
      // remove possible non-teeth pixels
      // threshold();
      for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
          if (phis.at<double>(i, j) > 0) {
            if (img_cv.at<double>(i, j) < ranges[sl][2] /*range[0] + 20*/
                /*|| img_cv.at<double>(i, j) < range[2]*/) {
              phis.at<double>(i, j) = 0;
            }
          }
        }
      }

      // fill hole to avoid removing inner parts
      // Mat im_th1;
      // threshold(phis, im_th1, 0, 255, THRESH_BINARY_INV);
      // do not remove inner parts when it is very close to the root
      // phis = fillHole2(im_th1);
      phis = fillHole2(phis);
    }
    // else {
    //	threshold(phis, phis, 0, 255, THRESH_BINARY_INV);
    //}
    phis.convertTo(phis, phi1n.type());
    if (sl % 2 == 0)
      bitwise_or(phi1n, phis, phi1n);
    else
      bitwise_or(phi2n, phis, phi2n);
  }
  // showTestMatImage(phi1n, "phi1");
  // showTestMatImage(phi2n, "phi2");

  normalize(phi1n, phi1n, 2.0, -2.0, NORM_MINMAX);
  phi1n = -1 * phi1n;
  normalize(phi2n, phi2n, 2.0, -2.0, NORM_MINMAX);
  phi2n = -1 * phi2n;
  phi1 = phi1n;
  phi2 = phi2n;
  // imshow("after", phi1);
  // waitKey();

  phi01_cv.convertTo(phi01_cv, CV_32F);
  phi02_cv.convertTo(phi02_cv, CV_32F);
  phi1.convertTo(phi1, CV_32F);
  phi2.convertTo(phi2, CV_32F);
  cuda::GpuMat phi1_cuda, phi2_cuda, edge_cuda, phi01_cuda, phi02_cuda,
      last_phi1_cuda, last_phi2_cuda;
  phi1_cuda.upload(phi1);
  phi2_cuda.upload(phi2);
  edge_cuda.upload(edge);
  phi01_cuda.upload(phi01_cv);
  phi02_cuda.upload(phi02_cv);
  Mat last_phi1 = phi1.clone();
  Mat last_phi2 = phi2.clone();
  last_phi1_cuda.upload(last_phi1);
  last_phi2_cuda.upload(last_phi2);

  cuda::GpuMat cvImg_sm_cuda_8uc1;
  cvImg_sm.convertTo(cvImg_sm, CV_8UC1);
  cvImg_sm_cuda_8uc1.upload(cvImg_sm);

  cuda::GpuMat cvImg_sm_cuda;
  cvImg_sm.convertTo(cvImg_sm, CV_32F);
  cvImg_sm_cuda.upload(cvImg_sm);

  Mat obj_mask1 = (phi1 <= 0);
  Mat obj_mask2 = (phi2 <= 0);
  threshold(obj_mask1, obj_mask1, 0, 1, THRESH_BINARY);
  threshold(obj_mask2, obj_mask2, 0, 1, THRESH_BINARY);

  cuda::GpuMat mask1_cuda, mask2_cuda, obj_mask1_cuda, obj_mask2_cuda;
  Mat _total_mask1 = total_mask1.clone();
  Mat _total_mask2 = total_mask2.clone();
  _total_mask1.convertTo(_total_mask1, CV_8UC1);
  _total_mask2.convertTo(_total_mask2, CV_8UC1);
  obj_mask1.convertTo(obj_mask1, CV_8UC1);
  obj_mask2.convertTo(obj_mask2, CV_8UC1);
  /*_total_mask1.convertTo(_total_mask1, CV_32F);
      _total_mask2.convertTo(_total_mask2, CV_32F);
      obj_mask1.convertTo(obj_mask1, CV_32F);
      obj_mask2.convertTo(obj_mask2, CV_32F);*/
  mask1_cuda.upload(_total_mask1);
  obj_mask1_cuda.upload(obj_mask1);
  mask2_cuda.upload(_total_mask2);
  obj_mask2_cuda.upload(obj_mask2);

  // std::vector<float> backProp1 = std::vector<float>(256, 0);
  // std::vector<float> backProp2 = std::vector<float>(256, 0);
  // std::vector<float> objProp1 = std::vector<float>(256, 0);
  // std::vector<float> objProp2 = std::vector<float>(256, 0);

  std::vector<float> backProp1;
  std::vector<float> backProp2;
  std::vector<float> objProp1;
  std::vector<float> objProp2;

  Mat img_prop = img.clone();
  img_prop.convertTo(img_prop, CV_32F);

  // clock_t start = std::clock();
  getPriorIntensityProp_q(cvImg_sm_cuda_8uc1, mask1_cuda, obj_mask1_cuda,
                          backProp1, objProp1, 255);
  getPriorIntensityProp_q(cvImg_sm_cuda_8uc1, mask2_cuda, obj_mask2_cuda,
                          backProp2, objProp2, 255);
  // clock_t end = std::clock();
  // double duration1 = (end-start)/(double)CLOCKS_PER_SEC;

  _total_mask1.convertTo(_total_mask1, CV_32F);
  _total_mask2.convertTo(_total_mask2, CV_32F);
  mask1_cuda.upload(_total_mask1);
  mask2_cuda.upload(_total_mask2);
  cuda::GpuMat propMat1a, propMat1b, propMat2a, propMat2b, logProp1, logProp2;
  propMat1a = getPropMat(img_prop, mask1_cuda, objProp1);
  propMat1b = getPropMat(img_prop, mask1_cuda, backProp1);
  propMat2a = getPropMat(img_prop, mask2_cuda, objProp2);
  propMat2b = getPropMat(img_prop, mask2_cuda, backProp2);

  double sm_value = 1e-10;

  cuda::add(propMat1b, sm_value, propMat1b);
  cuda::add(propMat1a, sm_value, propMat1a);
  cuda::add(propMat2b, sm_value, propMat2b);
  cuda::add(propMat2a, sm_value, propMat2a);
  cuda::divide(propMat1b, propMat1a, logProp1);
  cuda::divide(propMat2b, propMat2a, logProp2);
  cuda::log(logProp1, logProp1);
  cuda::log(logProp2, logProp2);

  cuda::GpuMat nu1, nu2, logPropPart1, logPropPart2, temp1, temp2;

  cuda::threshold(logProp1, temp1, 0, 1, THRESH_BINARY_INV);
  cuda::threshold(temp1, temp2, 0, 1, THRESH_BINARY_INV);
  cuda::multiply(temp1, v_e, temp1);
  cuda::multiply(temp2, v_s, temp2);
  cuda::add(temp1, temp2, nu1);

  cuda::threshold(logProp2, temp1, 0, 1, THRESH_BINARY_INV);
  cuda::threshold(temp1, temp2, 0, 1, THRESH_BINARY_INV);
  cuda::multiply(temp1, v_e, temp1);
  cuda::multiply(temp2, v_s, temp2);
  cuda::add(temp1, temp2, nu2);

  cuda::multiply(nu1, logProp1, logPropPart1);
  cuda::multiply(nu2, logProp2, logPropPart2);
  // showTestGpuMatImage(logPropPart1, "part1");
  // showTestGpuMatImage(logPropPart2, "part2");

  obj_mask1.convertTo(obj_mask1, CV_32F);
  obj_mask2.convertTo(obj_mask2, CV_32F);

  cuda::GpuMat areaPart1, areaPart2, phi1_minus, phi2_minus;
  cuda::multiply(obj_mask1, alpha_d, areaPart1);
  cuda::multiply(obj_mask2, alpha_d, areaPart2);
  /*clock_t end2 = std::clock();
      double duration2 = (end2 - end) / (double)CLOCKS_PER_SEC;*/

  /*if (flag) {
              showTestGpuMatMask(phi1_cuda, "phi1_cuda");
              showTestGpuMatMask(phi2_cuda, "phi2_cuda");
      }*/

  // iteration breaking conditions
  for (int i = 0; i < iterOuter; i++) {
    int pre1 = countNonZero(phi1 < 0);
    int pre2 = countNonZero(phi2 < 0);

    phi1_cuda =
        drlseEdgeGraDir(phi1_cuda, edge_cuda, lambda_d, mu, alpha, epsilon,
                        timeStep, iterInner, phi01_cuda, cvImg_sm_cuda,
                        last_phi1_cuda, beta_d, logPropPart1, areaPart2);

    phi2_cuda =
        drlseEdgeGraDir(phi2_cuda, edge_cuda, lambda_d, mu, alpha, epsilon,
                        timeStep, iterInner, phi02_cuda, cvImg_sm_cuda,
                        last_phi2_cuda, beta_d, logPropPart2, areaPart1);

    /*if (flag) {
                    showTestGpuMatMask(phi1_cuda, "phi1_cuda");
                    showTestGpuMatMask(phi2_cuda, "phi2_cuda");
            }*/

    cuda::multiply(float(-1), phi1_cuda, phi1_minus);
    cuda::multiply(float(-1), phi2_cuda, phi2_minus);

    areaPart1 = heaviside(phi1_minus, epsilon);
    // areaPart1 = phi2Mask(areaPart1);
    cuda::multiply(areaPart1, alpha_d, areaPart1);
    areaPart2 = heaviside(phi2_minus, epsilon);
    // areaPart2 = phi2Mask(areaPart2);
    cuda::multiply(areaPart2, alpha_d, areaPart2);

    int post1 = countNonZero(phi1 < 0);
    int post2 = countNonZero(phi2 < 0);
    if (pre1 - post1 <= 3 && pre2 - post2 <= 3) {
      break;
    }
  }

  // alpha = 0;
  int iterRefine = 10;

  phi1_cuda =
      drlseEdgeGraDir(phi1_cuda, edge_cuda, lambda_d, mu, alpha, epsilon,
                      timeStep, iterRefine, phi01_cuda, cvImg_sm_cuda,
                      last_phi1_cuda, beta_d, logPropPart1, areaPart2);
  phi1_cuda.download(phi1);

  phi2_cuda =
      drlseEdgeGraDir(phi2_cuda, edge_cuda, lambda_d, mu, alpha, epsilon,
                      timeStep, iterRefine, phi02_cuda, cvImg_sm_cuda,
                      last_phi2_cuda, beta_d, logPropPart2, areaPart1);
  phi2_cuda.download(phi2);

  Mat mask1 = (phi1 < 0);
  Mat mask2 = (phi2 < 0);

  normalize(cvImg, cvImg, 255.0, 0.0, NORM_MINMAX);

  for (int sl = 0; sl < initialLSFs.size(); sl++) {
    double min = 255, max = 0, average = 0, num = 0;
    Mat mask;
    initialLSFs[sl].convertTo(initialLSFs[sl], mask1.type());
    if (sl % 2 == 0)
      multiply(mask1, initialLSFs[sl], mask);
    else
      multiply(mask2, initialLSFs[sl], mask);
    int numNonZero = countNonZero(mask);
    // double* value = new double[numNonZero];
    vector<double> insideValues;

    for (int i = 0; i < width; i++) {
      for (int j = 0; j < height; j++) {
        if (mask.at<unsigned char>(i, j) != 0) {
          double pixelValue = cvImg.at<double>(i, j);
          insideValues.push_back(pixelValue);
          if (pixelValue < min) {
            min = pixelValue;
          }
          if (pixelValue > max) {
            max = pixelValue;
          }
          num++;
          average += pixelValue;
        }
      }
    }

    sort(insideValues.begin(), insideValues.end());
    int breakId = 0.1 * numNonZero - 1;
    int bottom = 0.05 * numNonZero, top = 0.95 * numNonZero;
    if (numNonZero != 0) {
      ranges[sl][2] = insideValues[breakId];
      /*if (insideValues[top] - insideValues[bottom] < 45) {
                  range[2] = min;
                  }*/
      insideValues.clear();
    } else {
      ranges[sl][2] = 0;
    }

    average /= num;
    ranges[sl][0] = min, ranges[sl][1] = max;
  }

  mask1 = fillHole(mask1);
  threshold(mask1, mask1, 0, 255, THRESH_BINARY_INV);

  mask2 = fillHole(mask2);
  threshold(mask2, mask2, 0, 255, THRESH_BINARY_INV);

  // showTestMatImage(mask1, "mask1");
  // showTestMatImage(mask2, "mask2");

  std::vector<Mat> masks;
  masks.push_back(mask1);
  masks.push_back(mask2);
  return masks;
}