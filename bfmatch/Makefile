main:
	export LD_LIBRARY_PATH=/home/chris/ufl/swamplaunch/opencv/install/lib
	g++ main.cpp \
	-g \
 	-I../opencv/install/include/opencv4 \
	-Wl,-rpath,../opencv/install/lib \
	-L../opencv/install/lib \
	-lopencv_calib3d \
	-lopencv_core \
	-lopencv_dnn \
	-lopencv_features2d \
	-lopencv_flann \
	-lopencv_gapi \
	-lopencv_highgui \
	-lopencv_imgcodecs \
	-lopencv_imgproc \
	-lopencv_ml \
	-lopencv_objdetect \
	-lopencv_photo \
	-lopencv_stitching \
	-lopencv_video \
	-lopencv_videoio
	