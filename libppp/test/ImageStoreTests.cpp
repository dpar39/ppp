#include <gtest/gtest.h>

#include "EasyExif.h"
#include "ImageStore.h"
#include "TestHelpers.h"
#include <opencv2/imgcodecs.hpp>

class ImageStoreTests : public testing::Test
{

protected:
    ImageStoreSPtr m_pImageStore = std::make_shared<ImageStore>();

    ImageStoreTests()
    {
        const auto toImage = [](const cv::Mat & image) {
            std::vector<BYTE> pictureData;
            cv::imencode(".png", image, pictureData);
            const auto data = reinterpret_cast<unsigned char *>(pictureData.data());
            return std::vector<char>(data, data + pictureData.size());
        };

        m_data1 = toImage(m_mat1);
        m_data2 = toImage(m_mat2);
        m_data3 = toImage(m_mat3);
    }

protected: // Test data
    cv::Mat m_mat1 = cv::Mat(10, 10, CV_8UC3, cv::Scalar(255, 0, 0));
    cv::Mat m_mat2 = cv::Mat(10, 10, CV_8UC3, cv::Scalar(0, 255, 0));
    cv::Mat m_mat3 = cv::Mat(10, 10, CV_8UC3, cv::Scalar(0, 0, 255));

    std::vector<char> m_data1;
    std::vector<char> m_data2;
    std::vector<char> m_data3;
};

TEST_F(ImageStoreTests, CanAddAndRetrieveImages)
{
    m_pImageStore->setStoreSize(2);

    // Add images to the store
    const auto key1 = m_pImageStore->setImage(m_data1.data(), m_data1.size());
    const auto key2 = m_pImageStore->setImage(m_data2.data(), m_data2.size());

    // Get images from the store
    const auto m1Ret = m_pImageStore->getImage(key1);
    const auto m2Ret = m_pImageStore->getImage(key2);

    // Assert
    verifyEqualImages(m_mat1, m1Ret);
    verifyEqualImages(m_mat2, m2Ret);
}

TEST_F(ImageStoreTests, ImagesArePrioritizedByAccess)
{
    m_pImageStore->setStoreSize(2);

    // Add images to the store
    auto key1 = m_pImageStore->setImage(m_data1.data(), m_data1.size());
    const auto key2 = m_pImageStore->setImage(m_data2.data(), m_data2.size());
    const auto key3 = m_pImageStore->setImage(m_data3.data(), m_data3.size());

    // Image 1 shouldn't be on the store, but 2 and 3 should
    EXPECT_FALSE(m_pImageStore->containsImage(key1));
    EXPECT_TRUE(m_pImageStore->containsImage(key2));
    EXPECT_TRUE(m_pImageStore->containsImage(key3));

    // Now touch image 2 and push image 1
    m_pImageStore->containsImage(key2);
    key1 = m_pImageStore->setImage(m_data1.data(), m_data1.size());

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

TEST_F(ImageStoreTests, ImageExifDataRetrieval)
{
    m_pImageStore->setStoreSize(1);

    // This image has EXIF information
    const auto imageFileName1 = resolvePath("research/my_database/000.jpg");
    const auto imageKey1 = m_pImageStore->setImage(imageFileName1);
    const auto image1 = m_pImageStore->getImage(imageKey1);
    const auto imgExif1 = m_pImageStore->getExifInfo(imageKey1);

    EXPECT_EQ(image1.cols, 1836);
    EXPECT_EQ(image1.rows, 3264);
    ASSERT_TRUE(static_cast<bool>(imgExif1));

    // Serialize to JSON string
    const auto json = imgExif1->toJson(false);

    EXPECT_NE(json.find("EXIFInfo"), std::string::npos);
    EXPECT_NE(json.find("Orientation"), std::string::npos);

    // This image doesn't have EXIF information
    const auto imageFileName2 = resolvePath("research/my_database/012.png");
    const auto imageKey2 = m_pImageStore->setImage(imageFileName2);
    const auto image2 = m_pImageStore->getImage(imageKey2);
    const auto imgExif2 = m_pImageStore->getExifInfo(imageKey2);

    EXPECT_EQ(image2.cols, 512);
    EXPECT_EQ(image2.rows, 512);
    ASSERT_FALSE(imgExif2);
}
