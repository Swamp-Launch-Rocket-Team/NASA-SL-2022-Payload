# Install tools
sudo apt update
sudo apt install -y cmake g++ wget unzip

# Download OpenCV version
curl -L "https://github.com/opencv/opencv/archive/refs/tags/4.5.5.zip" -o opencv.zip
#Unzip
unzip opencv.zip

# Create build directory
mkdir -p build
# Create install directory
mkdir -p install

# Move into build directory
cd build

# Configure
cmake -DCMAKE_INSTALL_PREFIX=../install ../opencv-4.5.5

# Build
cmake --build .