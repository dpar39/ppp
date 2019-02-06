#include <gtest/gtest.h>

#include "ImageStore.h"
#include "TestHelpers.h"

class ImageStoreTests : public testing::Test
{

protected:
    ImageStoreSPtr m_pImageStore = std::make_shared<ImageStore>();

protected: // Test data
    cv::Mat m_mat1 = cv::Mat(2, 3, CV_8UC1, 1);
    cv::Mat m_mat2 = cv::Mat(4, 5, CV_8UC1, 2);
    cv::Mat m_mat3 = cv::Mat(6, 8, CV_8UC1, 3);
};

TEST_F(ImageStoreTests, CanAddAndRetrieveImages)
{
    m_pImageStore->setStoreSize(2);

    // Add images to the store
    auto key1 = m_pImageStore->setImage(m_mat1);
    auto key2 = m_pImageStore->setImage(m_mat2);

    // Get images from the store
    auto m1Ret = m_pImageStore->getImage(key1);
    auto m2Ret = m_pImageStore->getImage(key2);

    // Assert
    verifyEqualImages(m_mat1, m1Ret);
    verifyEqualImages(m_mat2, m2Ret);
}

TEST_F(ImageStoreTests, ImagesArePrioritizedByAccess)
{
    m_pImageStore->setStoreSize(2);

    // Add images to the store
    auto key1 = m_pImageStore->setImage(m_mat1);
    auto key2 = m_pImageStore->setImage(m_mat2);
    auto key3 = m_pImageStore->setImage(m_mat3);

    // Image 1 shoudn't be on the store, but 2 and 3 should
    EXPECT_FALSE(m_pImageStore->containsImage(key1));
    EXPECT_TRUE(m_pImageStore->containsImage(key2));
    EXPECT_TRUE(m_pImageStore->containsImage(key3));

    // Now touch image 2 and push image 1
    m_pImageStore->containsImage(key2);
    key1 = m_pImageStore->setImage(m_mat1);

    // Image 3 shouldn't be on the store but 1 and 2 will
    EXPECT_FALSE(m_pImageStore->containsImage(key3));
    EXPECT_TRUE(m_pImageStore->containsImage(key1));
    EXPECT_TRUE(m_pImageStore->containsImage(key2));

    // Reduce store size to 1
    m_pImageStore->setStoreSize(1);

    // Now only image 2 (the last one touched) should be present
    EXPECT_TRUE(m_pImageStore->containsImage(key2));
    EXPECT_FALSE(m_pImageStore->containsImage(key1));
    EXPECT_FALSE(m_pImageStore->containsImage(key3));
}
