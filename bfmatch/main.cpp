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

    // Create SIFT detector object
    cv::Ptr<cv::SiftFeatureDetector> detector = cv::SiftFeatureDetector::create(0, 3, 0.04, 10.0, 1.6);
    // Have SIFT detect keypoints and store it in keypoints vector
    
    std::vector<cv::KeyPoint> keypoints1;
    detector->detect(input1, keypoints1);

    std::vector<cv::KeyPoint> keypoints2;
    detector->detect(input2, keypoints2);

    cv::Mat desc1;
    cv::Mat desc2;

    detector->compute(input1, keypoints1, desc1);
    detector->compute(input2, keypoints2, desc2);

    std::cout << keypoints1.size() << std::endl;
    std::cout << keypoints2.size() << std::endl;

    auto bf = cv::BFMatcher::create(cv::NORM_L2, false);

    std::vector<cv::DMatch> matches;
    bf->match(desc1, desc2, matches);        

    cv::Mat finalOutput;
    cv::drawMatches(input1,keypoints1,input2,keypoints2,matches,finalOutput);
    cv::imwrite("finalout.jpg", finalOutput);

    // Create output image and draw keypoints on image
    cv::Mat output;
    cv::Mat output2;
    
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
    cv::drawKeypoints(input1, keypoints1, output, cv::Scalar_<double>::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    cv::imwrite("sift_output1.jpg", output);

    cv::drawKeypoints(input2, keypoints2, output2, cv::Scalar_<double>::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    cv::imwrite("sift_output2.jpg", output2);

    return 0;
}