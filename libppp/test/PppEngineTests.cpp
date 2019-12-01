#include <gtest/gtest.h>
#include <memory>

#include "LandMarks.h"
#include "PhotoStandard.h"
#include "PrintDefinition.h"

#include "MockCrownChinEstimator.h"
#include "MockDetector.h"
#include "MockImageStore.h"
#include "MockPhotoPrintMaker.h"
#include "PppEngine.h"

using namespace testing;

namespace ppp
{
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
    EXPECT_CALL(*m_pFaceDetector, configure(_)).Times(1);
    EXPECT_CALL(*m_pEyesDetector, configure(_)).Times(1);
    EXPECT_CALL(*m_pLipsDetector, configure(_)).Times(1);
    EXPECT_CALL(*m_pCrownChinEstimator, configure(_)).Times(1);

    EXPECT_CALL(*m_pImageStore, setStoreSize(42));

    EXPECT_CALL(*m_pPhotoPrintMaker, configure(_)).Times(1);

    m_pppEngine->configure("{ imageStoreSize: 42 }");
}

TEST_F(PppEngineTests, LandMarkDetectionWorkflowHappyPath)
{
    const cv::Mat dummyImage(2, 3, CV_8UC3, cv::Scalar(10, 20, 30));

    std::string imgKey = "a1b2c3d4";

    const auto landmarks = LandMarks::create();

    EXPECT_CALL(*m_pImageStore, containsImage(Ref(imgKey))).WillOnce(Return(true));

    EXPECT_CALL(*m_pImageStore, getImage(Ref(imgKey))).WillOnce(Return(dummyImage));

    EXPECT_CALL(*m_pImageStore, getLandMarks(Ref(imgKey))).WillOnce(Return(landmarks));

    EXPECT_CALL(*m_pEyesDetector, detectLandMarks(_, Ref(*landmarks))).WillOnce(Return(true));

    EXPECT_CALL(*m_pLipsDetector, detectLandMarks(_, Ref(*landmarks))).WillOnce(Return(true));

    EXPECT_CALL(*m_pFaceDetector, detectLandMarks(_, Ref(*landmarks))).WillOnce(Return(true));

    EXPECT_CALL(*m_pCrownChinEstimator, estimateCrownChin(Ref(*landmarks))).WillOnce(Return(true));

    // Act
    EXPECT_EQ(true, m_pppEngine->detectLandMarks(imgKey));
}
} // namespace ppp
