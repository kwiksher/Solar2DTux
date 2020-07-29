# this file is run inside ubuntu environment
if [[ "$OSTYPE" != "linux-gnu" ]]
then
    echo "This script is meant to run on Linux"
    exit 1
fi
# download android sdk
wget -c http://vlad-test.s3.amazonaws.com/tmp/android-sdk.tar.gz -P /tmp
tar -xzf /tmp/android-sdk.tar.gz -C Solar2DSimulator
# nake sure we have the latest package lists
sudo apt-get update
# install dependencies
sudo apt-get install build-essential -y
sudo apt-get install libopenal-dev -y
sudo apt-get install libgtk-3-dev -y
sudo apt-get install libpng-dev -y
sudo apt-get install zlib1g-dev -y
sudo apt-get install libgstreamer1.0-dev -y
sudo apt-get install libgstreamer-plugins-base1.0-dev -y
sudo apt-get install libjpeg-dev -y
sudo apt-get install libssl-dev -y
sudo apt-get install libvorbis-dev -y
sudo apt-get install libogg-dev -y
sudo apt-get install uuid-dev -y
sudo apt-get install libxxf86vm-dev -y
sudo apt-get install libwebkit2gtk-4.0-37 -y
sudo apt-get install libwebkit2gtk-4.0-dev -y
sudo apt-get install android-libandroidfw -y
sudo apt-get install android-libbacktrace -y
sudo apt-get install android-libbase -y
sudo apt-get install android-libcutils -y
sudo apt-get install android-liblog -y
sudo apt-get install android-libunwind -y
sudo apt-get install android-libutils -y
sudo apt-get install android-libziparchive -y
sudo apt-get install libgstreamer1.0-0-dbg -y
sudo apt-get install libzopfli1 -y
sudo apt-get install openjdk-8-jdk-headless -y
sudo apt-get install openjdk-8-jre-headless -y
sudo apt-get install p7zip -y
sudo apt-get install p7zip-full -y
sudo apt-get install lua5.1
