#include <gtest/gtest.h>
#include <memory>

#include "PhotoStandard.h"
#include "LandMarks.h"
#include "CanvasDefinition.h"

#include "PppEngine.h"
#include "MockDetector.h"
#include "MockPhotoPrintMaker.h"
#include "MockImageStore.h"

using namespace testing;

class PppEngineTests : public Test
{
protected:
    std::shared_ptr<MockDetector> m_faceDetector;

    std::shared_ptr<MockDetector> m_eyesDetector;

    std::shared_ptr<MockDetector> m_lipsDetector;

    std::shared_ptr<MockImageStore> m_imageStore;

    std::shared_ptr<MockPhotoPrintMaker> m_photoPrintMaker;

    std::shared_ptr<PppEngine> m_pppEngine;

public:
    void SetUp() override
    {
        m_faceDetector = std::make_shared<MockDetector>();
        m_eyesDetector = std::make_shared<MockDetector>();
        m_lipsDetector = std::make_shared<MockDetector>();


        m_imageStore = std::make_shared<MockImageStore>();
        m_photoPrintMaker = std::make_shared<MockPhotoPrintMaker>();

        m_pppEngine = std::make_shared<PppEngine>(m_faceDetector, m_eyesDetector, m_lipsDetector, m_photoPrintMaker, m_imageStore);
    }

};

TEST_F(PppEngineTests, ConfigureWorks)
{
    // Arange
    const int imageStoreSize = 42;
    rapidjson::Document config;
    config.SetObject();
    auto& alloc = config.GetAllocator();
    config.AddMember("imageStoreSize", imageStoreSize, alloc);


    EXPECT_CALL(*m_faceDetector, configure(Ref(config))).Times(1);
    EXPECT_CALL(*m_eyesDetector, configure(Ref(config))).Times(1);
    EXPECT_CALL(*m_lipsDetector, configure(Ref(config))).Times(1);

    EXPECT_CALL(*m_imageStore, setStoreSize(imageStoreSize));

    // Act
    m_pppEngine->configure(config);

}

TEST_F(PppEngineTests, CanSetInputImage)
{
    cv::Mat dummyImage1(3, 3, CV_8UC3, cv::Scalar(0, 0, 0));

    const auto crc1e = "1a7a52b3";

    EXPECT_CALL(*m_imageStore, setImage(Ref(dummyImage1))).WillOnce(Return(crc1e));

    auto crc1 = m_pppEngine->setInputImage(dummyImage1);

    ASSERT_EQ(crc1e, crc1) << "CRC should be returned by the mocked image store as per setup";
}

TEST_F(PppEngineTests, LandMarkDetectionWorkflowHappyPath)
{
    cv::Mat dummyImage;

    m_pppEngine->setInputImage(dummyImage);

    // Act
    //m_pppEngine->detectLandMarks(config);
}
