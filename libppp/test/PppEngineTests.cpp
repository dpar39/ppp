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
    std::shared_ptr<MockDetector> m_pFaceDetector;

    std::shared_ptr<MockDetector> m_pEyesDetector;

    std::shared_ptr<MockDetector> m_pLipsDetector;

    std::shared_ptr<MockImageStore> m_pImageStore;

    std::shared_ptr<MockPhotoPrintMaker> m_pPhotoPrintMaker;

    std::shared_ptr<PppEngine> m_pppEngine;

public:
    void SetUp() override
    {
        m_pFaceDetector = std::make_shared<MockDetector>();
        m_pEyesDetector = std::make_shared<MockDetector>();
        m_pLipsDetector = std::make_shared<MockDetector>();


        m_pImageStore = std::make_shared<MockImageStore>();
        m_pPhotoPrintMaker = std::make_shared<MockPhotoPrintMaker>();

        m_pppEngine = std::make_shared<PppEngine>(m_pFaceDetector,
                                                  m_pEyesDetector,
                                                  m_pLipsDetector,
                                                  m_pPhotoPrintMaker,
                                                  m_pImageStore);
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


    EXPECT_CALL(*m_pFaceDetector, configure(Ref(config))).Times(1);
    EXPECT_CALL(*m_pEyesDetector, configure(Ref(config))).Times(1);
    EXPECT_CALL(*m_pLipsDetector, configure(Ref(config))).Times(1);

    EXPECT_CALL(*m_pImageStore, setStoreSize(imageStoreSize));

    EXPECT_CALL(*m_pPhotoPrintMaker, configure(Ref(config))).Times(1);

    // Act
    m_pppEngine->configure(config);
}



TEST_F(PppEngineTests, CanSetInputImage)
{
    cv::Mat dummyImage1(3, 3, CV_8UC3, cv::Scalar(0, 0, 0));

    const auto crc1e = "1a7a52b3";

    EXPECT_CALL(*m_pImageStore, setImage(Ref(dummyImage1))).WillOnce(Return(crc1e));

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
