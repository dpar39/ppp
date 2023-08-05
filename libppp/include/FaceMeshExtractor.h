#pragma once

#include "IFaceMeshExtractor.h"

#include "opencv2/core/types.hpp"

namespace mediapipe {
class CalculatorGraph;
} // namespace mediapipe

namespace ppp {

class FaceMeshExtractor : public IFaceMeshExtractor
{
public:
    FaceMeshExtractor() = default;

    ~FaceMeshExtractor() override;

    bool detectLandMarks(const cv::Mat & inputImage, LandMarks & landmarks) override;

private:
    void configureInternal(const ConfigLoaderSPtr & config) override;

    std::shared_ptr<mediapipe::CalculatorGraph> m_graph;

    std::vector<cv::Point3f> m_landmarks;

    size_t m_timeStamp { 0 };

public:
    // NOLINTBEGIN (hicpp-avoid-c-arrays)
    constexpr static std::array SILHOUETTE = { 10,  338, 297, 332, 284, 251, 389, 356, 454, 323, 361, 288,
                                               397, 365, 379, 378, 400, 377, 152, 148, 176, 149, 150, 136,
                                               172, 58,  132, 93,  234, 127, 162, 21,  54,  103, 67,  109 };

    constexpr static std::array LIPS_UPPER_OUTER = { 61, 185, 40, 39, 37, 0, 267, 269, 270, 409, 291 };
    constexpr static std::array LIPS_LOWER_OUTER = { 146, 91, 181, 84, 17, 314, 405, 321, 375, 291 };
    constexpr static std::array LIPS_UPPER_INNER = { 78, 191, 80, 81, 82, 13, 312, 311, 310, 415, 308 };
    constexpr static std::array LIPS_LOWER_INNER = { 78, 95, 88, 178, 87, 14, 317, 402, 318, 324, 308 };

    constexpr static std::array RIGHT_EYE_UPPER0 = { 246, 161, 160, 159, 158, 157, 173 };
    constexpr static std::array RIGHT_EYE_LOWER0 = { 33, 7, 163, 144, 145, 153, 154, 155, 133 };
    constexpr static std::array RIGHT_EYE_UPPER1 = { 247, 30, 29, 27, 28, 56, 190 };
    constexpr static std::array RIGHT_EYE_LOWER1 = { 130, 25, 110, 24, 23, 22, 26, 112, 243 };
    constexpr static std::array RIGHT_EYE_UPPER2 = { 113, 225, 224, 223, 222, 221, 189 };
    constexpr static std::array RIGHT_EYE_LOWER2 = { 226, 31, 228, 229, 230, 231, 232, 233, 244 };
    constexpr static std::array RIGHT_EYE_LOWER3 = { 143, 111, 117, 118, 119, 120, 121, 128, 245 };

    constexpr static std::array RIGHT_EYEBROW_UPPER = { 156, 70, 63, 105, 66, 107, 55, 193 };
    constexpr static std::array RIGHT_EYEBROW_LOWER = { 35, 124, 46, 53, 52, 65 };

    // constexpr static std::array RIGHT_EYE_IRIS = { 473, 474, 475, 476, 477 };
    constexpr static std::array RIGHT_EYE_IRIS = { 473 };

    constexpr static std::array LEFT_EYE_UPPER0 = { 466, 388, 387, 386, 385, 384, 398 };
    constexpr static std::array LEFT_EYE_LOWER0 = { 263, 249, 390, 373, 374, 380, 381, 382, 362 };
    constexpr static std::array LEFT_EYE_UPPER1 = { 467, 260, 259, 257, 258, 286, 414 };
    constexpr static std::array LEFT_EYE_LOWER1 = { 359, 255, 339, 254, 253, 252, 256, 341, 463 };
    constexpr static std::array LEFT_EYE_UPPER2 = { 342, 445, 444, 443, 442, 441, 413 };
    constexpr static std::array LEFT_EYE_LOWER2 = { 446, 261, 448, 449, 450, 451, 452, 453, 464 };
    constexpr static std::array LEFT_EYE_LOWER3 = { 372, 340, 346, 347, 348, 349, 350, 357, 465 };

    constexpr static std::array LEFT_EYEBROW_UPPER = { 383, 300, 293, 334, 296, 336, 285, 417 };
    constexpr static std::array LEFT_EYEBROW_LOWER = { 265, 353, 276, 283, 282, 295 };

    // constexpr static std::array LEFT_EYE_IRIS = { 468, 469, 470, 471, 472 };
    constexpr static std::array LEFT_EYE_IRIS = { 468 };

    constexpr static std::array MIDWAY_BETWEEN_EYES = { 168 };

    constexpr static std::array NOSE_TIP = { 1 };
    constexpr static std::array NOSE_BOTTOM = { 2 };
    constexpr static std::array NOSE_RIGHT_CORNER = { 98 };
    constexpr static std::array NOSE_LEFT_CORNER = { 327 };

    constexpr static std::array RIGHT_CHEEK = { 205 };
    constexpr static std::array LEFT_CHEEK = { 42 };
    constexpr static std::array CHIN_POINT = { 152 };
    // NOLINTEND(hicpp-avoid-c-arrays)
};
} // namespace ppp
