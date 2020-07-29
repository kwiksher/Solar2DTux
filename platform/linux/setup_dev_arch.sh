
# install dependencies
sudo pacman -Sy base-devel
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
sudo ln -s /usr/lib/libreadline.so /usr/lib/libreadline.so.7
# copy wx web extensions
sudo mkdir -p /usr/local/lib/wx/3.1.3/web-extensions/
sudo cp wx/lib/webkit2_extu-3.1.3.so /usr/local/lib/wx/3.1.3/web-extensions/webkit2_extu-3.1.3.so
