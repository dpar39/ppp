#include <gtest/gtest.h>
#include <memory>

#include "PhotoStandard.h"
#include "LandMarks.h"
#include "CanvasDefinition.h"

#include "PppEngine.h"
#include "MockDetector.h"
#include "MockPhotoPrintMaker.h"

using namespace testing;

class PppEngineTests : public Test
{
protected:
    std::shared_ptr<MockDetector> m_faceDetector;

    std::shared_ptr<MockDetector> m_eyesDetector;

    std::shared_ptr<MockDetector> m_lipsDetector;


    std::shared_ptr<PppEngine> m_pppEngine;

    std::shared_ptr<MockPhotoPrintMaker> m_photoPrintMaker;


public:
    void SetUp() override
    {

        m_faceDetector = std::make_shared<MockDetector>();
        m_eyesDetector = std::make_shared<MockDetector>();
        m_lipsDetector = std::make_shared<MockDetector>();

        m_pppEngine = std::make_shared<PppEngine>(m_faceDetector, m_eyesDetector, m_lipsDetector, m_photoPrintMaker);

    }

};

TEST_F(PppEngineTests, ConfigureWorks)
{
    // Arange
    rapidjson::Value config;

    EXPECT_CALL(*m_faceDetector, configure(Ref(config))).Times(1);
    EXPECT_CALL(*m_eyesDetector, configure(Ref(config))).Times(1);
    EXPECT_CALL(*m_lipsDetector, configure(Ref(config))).Times(1);

    // Act
    m_pppEngine->configure(config);

}

TEST_F(PppEngineTests, CanSetInputImage)
{
    cv::Mat dummyImage1(3, 3, CV_8UC3, cv::Scalar(0, 0, 0));
    cv::Mat dummyImage2(3, 3, CV_8UC3, cv::Scalar(0, 10, 20));
    cv::Mat dummyImage3(3, 3, CV_8UC3, cv::Scalar(0, 0, 0));


    const auto crc1e = "1a7a52b3";
    const auto crc2e = "510301a7";

    auto crc1 = m_pppEngine->setInputImage(dummyImage1);

    //EXPECT_EQ(m_pppEngine->m_imageCollection.size(), 1);


    auto crc2 = m_pppEngine->setInputImage(dummyImage2);


}

TEST_F(PppEngineTests, LandMarkDetectionWorkflowHappyPath)
{
    cv::Mat dummyImage;

    m_pppEngine->setInputImage(dummyImage);

    // Act
    //m_pppEngine->detectLandMarks(config);
}
