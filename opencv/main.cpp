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
    const cv::Mat input = cv::imread("../images/SmallerImage.jpg");

    // Create SIFT detector object
    cv::Ptr<cv::SiftFeatureDetector> detector = cv::SiftFeatureDetector::create(0, 3, 0.04, 10.0, 1.6);
    // Have SIFT detect keypoints and store it in keypoints vector
    std::vector<cv::KeyPoint> keypoints;
    detector->detect(input, keypoints);

    std::cout << keypoints.size() << std::endl;

    // Create output image and draw keypoints on image
    cv::Mat output;
    
    // std::vector<cv::KeyPoint> oneKeypoint;
    // for (cv::KeyPoint point : keypoints)
    // {
    //     oneKeypoint.clear();
    //     oneKeypoint.push_back(point);
    //     cv::drawKeypoints(input, oneKeypoint, output, cv::Scalar_<double>::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    //     cv::imwrite("sift_output.jpg", output);

    //     std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // }

    // Output resulting Image
    cv::drawKeypoints(input, keypoints, output, cv::Scalar_<double>::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    cv::imwrite("sift_output.jpg", output);

    return 0;
}