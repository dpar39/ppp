#include <gtest/gtest.h>
#include <memory>

#include "PhotoStandard.h"
#include "LandMarks.h"
#include "CanvasDefinition.h"

#include "PppEngine.h"
#include "MockDetector.h"
#include "MockPhotoPrintMaker.h"
#include "MockImageStore.h"
#include "MockCrownChinEstimator.h"

using namespace testing;

class PppEngineTests : public Test
{
protected:
    std::shared_ptr<MockDetector> m_pFaceDetector;

    std::shared_ptr<MockDetector> m_pEyesDetector;

    std::shared_ptr<MockDetector> m_pLipsDetector;

    std::shared_ptr<MockImageStore> m_pImageStore;

    std::shared_ptr<MockCrownChinEstimator> m_pCrownChinEstimator;

    std::shared_ptr<MockPhotoPrintMaker> m_pPhotoPrintMaker;

    std::shared_ptr<PppEngine> m_pppEngine; /* SUT */

public:
    void SetUp() override
    {
        m_pFaceDetector = std::make_shared<MockDetector>();
        m_pEyesDetector = std::make_shared<MockDetector>();
        m_pLipsDetector = std::make_shared<MockDetector>();

        m_pCrownChinEstimator = std::make_shared<MockCrownChinEstimator>();

        m_pImageStore = std::make_shared<MockImageStore>();
        m_pPhotoPrintMaker = std::make_shared<MockPhotoPrintMaker>();

        m_pppEngine = std::make_shared<PppEngine>(m_pFaceDetector,
                                                  m_pEyesDetector,
                                                  m_pLipsDetector,
                                                  m_pCrownChinEstimator,
                                                  m_pPhotoPrintMaker,
                                                  m_pImageStore);
    }
};

TEST_F(PppEngineTests, DISABLED_ConfigureWorks)
{
    // Arrange
    const int imageStoreSize = 42;
    rapidjson::Document config;
    config.SetObject();
    auto& alloc = config.GetAllocator();
    config.AddMember("imageStoreSize", imageStoreSize, alloc);


    EXPECT_CALL(*m_pFaceDetector, configure(Ref(config))).Times(1);
    EXPECT_CALL(*m_pEyesDetector, configure(Ref(config))).Times(1);
    EXPECT_CALL(*m_pLipsDetector, configure(Ref(config))).Times(1);
    EXPECT_CALL(*m_pCrownChinEstimator, configure(Ref(config))).Times(1);

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
    cv::Mat dummyImage(2, 3, CV_8UC3, cv::Scalar(10, 20, 30));

    std::string imgKey = "a1b2c3d4";

    LandMarks landmarks;

    EXPECT_CALL(*m_pImageStore, containsImage(Ref(imgKey))).WillOnce(Return(true));

    EXPECT_CALL(*m_pImageStore, getImage(Ref(imgKey))).WillOnce(Return(dummyImage));

    EXPECT_CALL(*m_pEyesDetector, detectLandMarks(_, Ref(landmarks))).WillOnce(Return(true));

    EXPECT_CALL(*m_pLipsDetector, detectLandMarks(_, Ref(landmarks))).WillOnce(Return(true));

    EXPECT_CALL(*m_pFaceDetector, detectLandMarks(_, Ref(landmarks))).WillOnce(Return(true));

    EXPECT_CALL(*m_pCrownChinEstimator, estimateCrownChin(Ref(landmarks))).WillOnce(Return(true));

    // Act
    EXPECT_EQ(true, m_pppEngine->detectLandMarks(imgKey, landmarks));
}
