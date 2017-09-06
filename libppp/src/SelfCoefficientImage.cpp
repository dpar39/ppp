#include "SelfCoefficientImage.h"
#include <opencv2/imgproc/imgproc.hpp>




void SelfCoefficientImage::compute(const cv::Mat& inputImg, cv::Mat &outputImg, int maskSize)
{

    auto halfsize = static_cast<int>(ceil(maskSize / 2));

    auto sigma = maskSize / 5.0;

    cv::Mat kernel = cv::getGaussianKernel(maskSize, sigma, CV_64F);

    outputImg = cv::Mat(inputImg.size(), inputImg.type(), cv::Scalar::all(0));

    if (inputImg.channels() != 1)
    {
        throw std::runtime_error("Image should be grayscale");
    }

    auto height = inputImg.size().height;
    auto width = inputImg.size().width;

    double windowSizeSqr = maskSize*maskSize;

    std::vector<double> windowData(windowSizeSqr);
    std::vector<double> kernel1;

    for (auto r = 0; r < height; ++r)
    {
        for (auto c = 0; c < width; ++c)
        {
            auto meanWindow = 0.0;
            auto idx = 0;

            // FIRST LOOP: compute region_mean
            for (auto kprime = -halfsize; kprime <= halfsize; kprime++)
            {
                auto index_k = r - kprime;
                if (index_k < 0)
                {
                    index_k = abs(index_k) - 1;
                }
                if (index_k >= height)
                {
                    index_k = (2 * height) - index_k - 1;
                }
                for (auto lprime = -halfsize; lprime <= halfsize; lprime++)
                {
                    auto index_l = c - lprime;
                    if (index_l < 0)
                    {
                        index_l = abs(index_l) - 1;
                    }
                    if (index_l >= width)
                    {
                        index_l = (2 * width) - index_l - 1;
                    }
                    auto v = inputImg.at<uchar>(kprime, lprime);
                    windowData[idx++] = v;
                    meanWindow += v;
                }
            }
            meanWindow /= windowSizeSqr;

            // SECOND LOOP: count number of pixels bigger/smaller than the mean
            auto overMeanCnt = std::count_if(windowData.begin(), windowData.end(), [meanWindow](double v) {return v > meanWindow; });
            auto underMeanCnt = windowSizeSqr - overMeanCnt;
            auto above = overMeanCnt > underMeanCnt;

            // THIRD LOOP : update filter weights

            std::copy(kernel.datastart, kernel.dataend, kernel1.begin());

        }
    }
}
