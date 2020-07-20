INSTALLER_FOLDER=installer
BUILD_FOLDER=Debian
DEBIAN_CONTENTS_INSTALL_FOLDER=$INSTALLER_FOLDER/opt/Solar2D
SAMPLE_CODE_DIR=../../samples
SAMPLE_CODE_REMOTE=https://github.com/DannyGlover/Solar2DTux-Samples.git
mkdir -p $DEBIAN_CONTENTS_INSTALL_FOLDER
# remove old build
find $BUILD_FOLDER/ -name "*.deb" -type f -delete

# clone sample code (or just fetch latest if it exists)
if [ ! -d $SAMPLE_CODE_DIR ]
then
    git clone $SAMPLE_CODE_REMOTE $SAMPLE_CODE_DIR
else
    cd $SAMPLE_CODE_DIR/
    git pull $SAMPLE_CODE_REMOTE
    cd ../platform/linux/
fi

# copy debian folder
rsync -r $BUILD_FOLDER/DEBIAN/ $INSTALLER_FOLDER/DEBIAN/
# copy plugin dependencies
sh copy_plugin_deps.sh
# copy corona simulator binary
rsync -avz -q --exclude "android*" Solar2DSimulator/ $DEBIAN_CONTENTS_INSTALL_FOLDER/
# remove the debug binary
rm -rf $DEBIAN_CONTENTS_INSTALL_FOLDER/Solar2DSimulator_debug
# copy corona resources
rsync -r Solar2DSimulator/Resources/ $DEBIAN_CONTENTS_INSTALL_FOLDER/Resources/
# copy wx web extensions
cp -u wx/lib/webkit2_extu-3.1.3.so $DEBIAN_CONTENTS_INSTALL_FOLDER
# copy desktop file
cp -u $BUILD_FOLDER/Solar2DTux.desktop $DEBIAN_CONTENTS_INSTALL_FOLDER/Resources/
# copy logo
cp -u $BUILD_FOLDER/logo.png $DEBIAN_CONTENTS_INSTALL_FOLDER/Resources/
# copy sample code
rsync -r $SAMPLE_CODE_DIR/ $DEBIAN_CONTENTS_INSTALL_FOLDER/SampleCode/
# make deb installer
dpkg-deb --build $INSTALLER_FOLDER
mv $INSTALLER_FOLDER.deb $BUILD_FOLDER/solar2dtux.deb
# remove installer directory
rm -rf $INSTALLER_FOLDER
