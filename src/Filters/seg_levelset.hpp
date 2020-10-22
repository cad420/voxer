#pragma once
#include <iostream>
#include <numeric>
#include <opencv2/core/cuda.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudafilters.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

cv::cuda::GpuMat
drlseEdgeGraDir(cv::cuda::GpuMat phi, const cv::cuda::GpuMat& g, int lambda, float mu,
                float alpha, float epsilon, int timestep, int iter,
                const cv::cuda::GpuMat& phi0, const cv::cuda::GpuMat& cvImg_sm_cuda,
                const cv::cuda::GpuMat& last_phi, float beta,
                const cv::cuda::GpuMat& logPropPart, const cv::cuda::GpuMat& areaPart);
cv::cuda::GpuMat drlseEdge(cv::cuda::GpuMat phi, const cv::cuda::GpuMat& g, int lambda,
                           float mu, float alpha, float epsilon, int timestep,
                           int iter, const cv::cuda::GpuMat& phi0);
cv::cuda::GpuMat distReg_p2(const cv::cuda::GpuMat& phi,
                            const cv::Ptr<cv::cuda::Filter>& filterX,
                            const cv::Ptr<cv::cuda::Filter>& filterY);
cv::cuda::GpuMat div(const cv::cuda::GpuMat& nx, const cv::cuda::GpuMat& ny,
                     const cv::Ptr<cv::cuda::Filter>& filterX,
                     const cv::Ptr<cv::cuda::Filter>& filterY);
cv::cuda::GpuMat dirac(const cv::cuda::GpuMat& x, float sigma);
cv::cuda::GpuMat heaviside(const cv::cuda::GpuMat& x, float sigma);
cv::cuda::GpuMat neumannBoundCond(cv::cuda::GpuMat f);
cv::cuda::GpuMat sine(const cv::cuda::GpuMat& x);
cv::cuda::GpuMat cosine(const cv::cuda::GpuMat& x);
cv::Mat fillHole(const cv::Mat& img);
cv::Mat fillHole2(const cv::Mat& img);
// std::vector<Mat> segSliceCoupled(cv::Mat phi0_1, cv::Mat phi0_2, cv::Mat img,
// double range[3], cv::Mat total_mask1, cv::Mat total_mask2, bool crown, bool
// flag);
std::vector<cv::Mat> segSliceCoupled(cv::Mat phi0_1, cv::Mat phi0_2,
                                     cv::Mat img,
                                     std::vector<std::vector<double>> &Sranges,
                                     std::vector<cv::Mat> initialLSFs,
                                     const cv::Mat& total_mask1, const cv::Mat& total_mask2,
                                     bool crown, bool flag);
// cv::Mat segSliceCoupledTest(cv::Mat phi0_1, cv::Mat img, double range[3], Mat
// total_mask1);

cv::Mat segSlice(cv::Mat phi0, cv::Mat img, const cv::Mat& prior, double range[3],
                 int iterInner = 5, int iterOuter = 10);
cv::Mat segSlice(cv::Mat phi0, cv::Mat img, const cv::Mat& prior,
                 std::vector<std::vector<double>> &ranges,
                 std::vector<cv::Mat> initialLSFs, int start);
void getPriorIntensityProp(const cv::cuda::GpuMat& img,
                           const cv::cuda::GpuMat& total_mask,
                           const cv::cuda::GpuMat& obj_mask,
                           std::vector<float> &backProp,
                           std::vector<float> &objProp, int max_intensity);
void getPriorIntensityProp_q(const cv::cuda::GpuMat& img,
                             const cv::cuda::GpuMat& total_mask,
                             const cv::cuda::GpuMat& obj_mask,
                             std::vector<float> &backProp,
                             std::vector<float> &objProp, int max_intensity);
cv::cuda::GpuMat getSelectedSDF(std::vector<cv::cuda::GpuMat> phis, bool even,
                                float c0);
cv::cuda::GpuMat phi2Mask(const cv::cuda::GpuMat& phi);
cv::cuda::GpuMat getPropMat(cv::Mat img, const cv::cuda::GpuMat& total_mask,
                            std::vector<float> &intensity_prop);
