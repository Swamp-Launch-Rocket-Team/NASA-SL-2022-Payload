#include <iostream>
#include <vector>

#include <chrono>
#include <thread>


#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/features2d.hpp>

int main()
{
    // Read image
    const cv::Mat input1 = cv::imread("../images/BigImage.jpg");
    const cv::Mat input2 = cv::imread("../images/Pic1.png");

    int minHessian = 400;
    cv::Ptr<cv::SiftFeatureDetector> detector = cv::SiftFeatureDetector::create(0, 3, 0.04, 10.0, 1.6);    std::vector<cv::KeyPoint> keypoints1, keypoints2;
    cv::Mat descriptors1, descriptors2;
    detector->detectAndCompute( input1, cv::noArray(), keypoints1, descriptors1 );
    detector->detectAndCompute( input2, cv::noArray(), keypoints2, descriptors2 );
    //-- Step 2: Matching descriptor vectors with a FLANN based matcher
    // Since SURF is a floating-point descriptor NORM_L2 is used
    cv::Ptr<cv::DescriptorMatcher> matcher = cv::DescriptorMatcher::create(cv::DescriptorMatcher::FLANNBASED);
    std::vector< std::vector<cv::DMatch> > knn_matches;
    matcher->knnMatch( descriptors1, descriptors2, knn_matches, 2 );
    //-- Filter matches using the Lowe's ratio test
    const float ratio_thresh = 0.7f;
    std::vector<cv::DMatch> good_matches;
    for (size_t i = 0; i < knn_matches.size(); i++)
    {
        if (knn_matches[i][0].distance < ratio_thresh * knn_matches[i][1].distance)
        {
            good_matches.push_back(knn_matches[i][0]);
        }
    }
    //-- Draw matches
    cv::Mat img_matches;
    cv::drawMatches( input1, keypoints1, input2, keypoints2, good_matches, img_matches, cv::Scalar::all(-1),
                 cv::Scalar::all(-1), std::vector<char>(), cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );
    //-- Show detected matches
    cv::imwrite("THISISTEST.jpg", img_matches);
    cv::imshow("Good Matches", img_matches );

    // // Output resulting Image
    // cv::drawKeypoints(input1, keypoints1, output, cv::Scalar_<double>::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    // cv::imwrite("sift_output1.jpg", output);

    // cv::drawKeypoints(input2, keypoints2, output2, cv::Scalar_<double>::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    // cv::imwrite("sift_output2.jpg", output2);

    return 0;
}