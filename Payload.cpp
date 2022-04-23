#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>

int main(int argc, char* argv[])
{
    std::cout << "Reading camera image..." << std::endl;
    cv::Mat img_object = cv::imread("images/473.jpg");
    std::cout << "Done!" << std::endl;

    std::cout << "Reading reference image..." << std::endl;
    cv::Mat img_scene = cv::imread("images/lf.png");
    std::cout << "Done!" << std::endl;

    //-- Step 1: Detect the keypoints using SURF Detector, compute the descriptors
    int minHessian = 400;
    cv::Ptr<cv::SIFT> detector = cv::SIFT::create(0, 3, 0.04, 10.0, 1.6);
    std::vector<cv::KeyPoint> keypoints_object, keypoints_scene;
    cv::Mat descriptors_object, descriptors_scene;

    std::cout << "Detecting keypoints for camera image..." << std::endl;
    detector->detectAndCompute(img_object, cv::noArray(), keypoints_object, descriptors_object);
    std::cout << "Done!" << std::endl;

    std::cout << "Detecting keypoints for reference image..." << std::endl;
    detector->detectAndCompute(img_scene, cv::noArray(), keypoints_scene, descriptors_scene);
    std::cout << "Done!" << std::endl;

    // Ouputs the image of the computed keypoints for the camera image
    // cv::Mat blahout;
    // cv::drawKeypoints(img_object, keypoints_scene, blahout, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    // cv::imwrite("blah.png", blahout);


    //-- Step 2: cv::Matching descriptor vectors with a FLANN based cv::matcher
    // Since SURF is a floating-point descriptor NORM_L2 is used
    cv::Ptr<cv::DescriptorMatcher> matcher = cv::DescriptorMatcher::create(cv::DescriptorMatcher::FLANNBASED);
    std::vector< std::vector<cv::DMatch> > knn_matches;
    matcher->knnMatch(descriptors_object, descriptors_scene, knn_matches, 2);

    //-- Filter cv::matches using the Lowe's ratio test
    const float ratio_thresh = 0.75f;
    std::vector<cv::DMatch> good_matches;
    for (size_t i = 0; i < knn_matches.size(); i++)
    {
        if (knn_matches[i][0].distance < ratio_thresh * knn_matches[i][1].distance)
        {
            good_matches.push_back(knn_matches[i][0]);
        }
    }

    //-- Draw cv::matches
    cv::Mat img_matches;
    drawMatches(img_object, keypoints_object, img_scene, keypoints_scene, good_matches, img_matches, cv::Scalar::all(-1),
        cv::Scalar::all(-1), std::vector<char>(), cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

    //-- Localize the object
    std::vector<cv::Point2f> obj;
    std::vector<cv::Point2f> scene;
    for (size_t i = 0; i < good_matches.size(); i++)
    {
        //-- Get the keypoints from the good cv::matches
        obj.push_back(keypoints_object[good_matches[i].queryIdx].pt);
        scene.push_back(keypoints_scene[good_matches[i].trainIdx].pt);
    }
    cv::Mat H = findHomography(obj, scene, cv::RANSAC);

    //-- Get the corners from the image_1 ( the object to be "detected" )
    std::vector<cv::Point2f> obj_corners(4);
    obj_corners[0] = cv::Point2f(0, 0);
    obj_corners[1] = cv::Point2f((float)img_object.cols, 0);
    obj_corners[2] = cv::Point2f((float)img_object.cols, (float)img_object.rows);
    obj_corners[3] = cv::Point2f(0, (float)img_object.rows);
    std::vector<cv::Point2f> scene_corners(4);
    perspectiveTransform(obj_corners, scene_corners, H);

    //-- Draw lines between the corners (the mapped object in the scene - image_2 )
    cv::line(img_matches, scene_corners[0] + cv::Point2f((float)img_object.cols, 0),
        scene_corners[1] + cv::Point2f((float)img_object.cols, 0), cv::Scalar(0, 255, 0), 4);
    cv::line(img_matches, scene_corners[1] + cv::Point2f((float)img_object.cols, 0),
        scene_corners[2] + cv::Point2f((float)img_object.cols, 0), cv::Scalar(0, 255, 0), 4);
    cv::line(img_matches, scene_corners[2] + cv::Point2f((float)img_object.cols, 0),
        scene_corners[3] + cv::Point2f((float)img_object.cols, 0), cv::Scalar(0, 255, 0), 4);
    cv::line(img_matches, scene_corners[3] + cv::Point2f((float)img_object.cols, 0),
        scene_corners[0] + cv::Point2f((float)img_object.cols, 0), cv::Scalar(0, 255, 0), 4);

    cv::Point2f center = cv::Point2f(
        (float)img_object.cols + ((scene_corners[0].x + scene_corners[1].x) / 2),
        ((scene_corners[1].y + scene_corners[3].y) / 2)
    );
    circle(img_matches, center, 5, (255, 0, 0), cv::LineTypes::FILLED);

    cv::Point2f center2 = cv::Point2f(
        (scene_corners[0].x + scene_corners[1].x) / 2,
        ((scene_corners[1].y + scene_corners[3].y) / 2)
    );

    int gridLoc = 1;
    int x = ((scene_corners[0].x + scene_corners[1].x) / 2) / (img_scene.cols / 20);
    int y = ((scene_corners[1].y + scene_corners[3].y) / 2) / (img_scene.rows / 20);

    gridLoc = ((y - 1) * 20) + x;
    std::cout << gridLoc << std::endl;


    cv::Mat gridMat = img_scene;
    cv::Point2f temp = cv::Point2f(0, 0);
    for (int i = 0; i <= 200; i++)
    {
        for (int j = 0; j <= 20; j++)
        {
            circle(gridMat, temp, 2, (0, 0, 255), cv::LineTypes::FILLED);

            temp.x += gridMat.cols/20;
        }

        temp.x = 0;

        temp.y += gridMat.rows/20;   
    }

    cv::imwrite("grid.jpg", gridMat);

    //-- Show detected cv::matches
    cv::imwrite("output.jpg", img_matches);
    cv::waitKey();
    return 0;
}