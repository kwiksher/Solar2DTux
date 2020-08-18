#!/bin/bash 
# this file is run inside ubuntu environment
if [[ "$OSTYPE" != "linux-gnu" ]]
then
    echo "This script is meant to run on Linux"
    exit 1
fi

if [ -f /etc/os-release ]; then
    # freedesktop.org and systemd
    . /etc/os-release
    OS=$NAME
elif type lsb_release >/dev/null 2>&1; then
    # linuxbase.org
    OS=$(lsb_release -si)
elif [ -f /etc/lsb-release ]; then
    # For some versions of Debian/Ubuntu without lsb_release command
    . /etc/lsb-release
    OS=$DISTRIB_ID
elif [ -f /etc/debian_version ]; then
    # Older Debian/Ubuntu/etc.
    OS=Debian
elif [ -f /etc/SuSe-release ]; then
    # Older SuSE/etc.
    ...
elif [ -f /etc/redhat-release ]; then
    # Older Red Hat, CentOS, etc.
    ...
else
    # Fall back to uname, e.g. "Linux <version>", also works for BSD, etc.
    OS=$(uname -s)
fi

USE_APT=0
USE_PACMAN=0

if [[ "${OS,,}" == *"manjaro"* ]]; then
  USE_PACMAN=1
elif [[ "${OS,,}" == *"arch"* ]]; then
  USE_PACMAN=1
elif [[ "${OS,,}" == *"endeavour"* ]]; then
  USE_PACMAN=1
elif [[ "${OS,,}" == *"ubuntu"* ]]; then
  USE_APT=1
elif [[ "${OS,,}" == *"mint"* ]]; then
  USE_APT=1
elif [[ "${OS,,}" == *"pop"* ]]; then
  USE_APT=1
fi

# remove the existing solar platform tools directory
rm -rf ~/.local/share/Corona
# extract the solar platform tools directory
tar -xzf Resources/platform_tools.tgz -C ~/.local/share/
# move Solar2DBuilder into the platform tools directory
mv Solar2DBuilder ~/.local/share/Corona/Native/Corona/lin/bin/

# install required dependencies via Apt
if [[ $USE_APT == 1 ]]; then
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
  sudo apt-get install libgstreamer1.0-0-dbg -y
  sudo apt-get install libzopfli1 -y
  sudo apt-get install openjdk-8-jdk-headless -y
  sudo apt-get install openjdk-8-jre-headless -y
  sudo apt-get install p7zip -y
  sudo apt-get install p7zip-full -y
  sudo apt-get install lua5.1 -y
  sudo apt-get install gradle -y
# install required dependencies via Pacman
elif [[ $USE_PACMAN == 1 ]]; then
  sudo pacman -Sy base-devel
  sudo pacman -Sy gcc
  sudo pacman -Sy readline
  sudo pacman -Sy openal
  sudo pacman -Sy gtk3
  sudo pacman -Sy libpng
  sudo pacman -Sy zlib
  sudo pacman -Sy gstreamer
  sudo pacman -Sy gst-plugins-base
  sudo pacman -Sy libjpeg-turbo
  sudo pacman -Sy openssl
  sudo pacman -Sy libvorbis
  sudo pacman -Sy libogg
  sudo pacman -Sy util-linux
  sudo pacman -Sy libxxf86vm
  sudo pacman -Sy webkit2gtk
  sudo pacman -Sy zopfli
  sudo pacman -Sy jdk14-openjdk
  sudo pacman -Sy jre14-openjdk
  sudo pacman -Sy p7zip
  sudo pacman -Sy lua51
  sudo pacman -Sy gradle
  sudo ln -s /usr/lib/libreadline.so /usr/lib/libreadline.so.7
fi

# copy wx web extensions
sudo mkdir -p /usr/local/lib/wx/3.1.3/web-extensions/
sudo mv webkit2_extu-3.1.3.so /usr/local/lib/wx/3.1.3/web-extensions/webkit2_extu-3.1.3.so

# move the desktop file
sudo mv Resources/Solar2DTux.desktop /usr/share/applications/

# move everything to /opt
OPT_LOCATION=/opt/Solar2D
sudo mkdir -p $OPT_LOCATION
sudo mv Solar2DSimulator $OPT_LOCATION
sudo mv Solar2DConsole $OPT_LOCATION
sudo mv start.sh $OPT_LOCATION
sudo mv Resources $OPT_LOCATION

# set permissions
sudo chown $USER:$USER $OPT_LOCATION
sudo chmod -R a+rwx $OPT_LOCATION/start.sh

if [[ "$PATH" =~ (^|:)"/opt/Solar2D"(|/)(:|$) ]]; then
    echo "Application path already configured"
else
    echo 'export PATH="/opt/Solar2D:$PATH"' >> ~/.bashrc
    echo 'export PATH="~/.local/share/Corona/Native/Corona/lin/bin/:$PATH"' >> ~/.bashrc
fi

# clone sample code (or just fetch latest if it exists)
SAMPLE_CODE_REMOTE=https://github.com/DannyGlover/Solar2DTux-Samples.git
SAMPLE_CODE_DIR=/opt/Solar2D/SampleCode

if [ ! -d $SAMPLE_CODE_DIR ]
then
    git clone $SAMPLE_CODE_REMOTE $SAMPLE_CODE_DIR
else
    cd $SAMPLE_CODE_DIR/
    git pull $SAMPLE_CODE_REMOTE
fi

echo "In order to build for Android, you need to install Android Studio, install Android Api level 28 via the SDK manager and accept the license agreements."
echo "Then you can build via Solar2DTux for Android."

## remove this directory
rm -rf ${PWD}