<div align="right">Dec 14, 2020</div>
<div align="right">yamamoto@kwiksher</div>
<br>

ARM build for jetson nano and raspberry pi

* Download the source files for the following dependencies to **Solar2DTux/external/**

    * CryptoPP/
        * cryptopp-CRYPTOPP_8_2_0
    * openssl/
        * openssl-1.1.1h
    * curl/
        * curl-7.64.1
    * wx/
        * wxWidgets-3.1.4

* lib and include from the dependencies will be installed in

    * **plaftorm/linux/include/**
        * cryptopp
        * curl
        * openssl
        * wx-3.1/wx

        **platform/linux/wx** is a symbolic link to ./include/wx-3.1/wx
        ```
        ln -s ./include/wx-3.1/wx
        ```

    * **platform/linux/lib/**
        * libcrypto.a
        * libcryptopp.a
        * ...
        * wx/3.1.4/web-extensions
            * webkit2_extu-3.1.4.so
    
* wxWidgets
    * https://wiki.wxwidgets.org/Compiling_and_getting_started
    * https://yasriady.blogspot.com/2015/10/how-to-build-wxwidgets-for-raspberry-pi.html

        ```
        cd external/wx/wxWidgets-3.1.4
        mk gtk-build
        cd gtk-build
        ```

    * build.sh from Solar2DTux/platform/linux/wx

        ```
        ./configure --with-opengl --with-gtk3 --disable-shared --enable-monolithic --enable-webview --enable-webviewwebkit --enable-mediactrl

        sudo apt-get install freeglut3-dev
        sudo apt-get install libglw1-mesa libglw1-mesa-dev
        ```
    
    1. make static wxWidgets libraries

        ```
        cd external/wx/wxWidgets-3.1.4

        ../configure --prefix=$(pwd)/../../../../platform/linux --with-opengl --disable-shared --enable-monolithic --enable-webview --enable-webviewwebkit --enable-mediactrl  CXX="g++ -O2 -g3  -std=c++11"
        
        make -j 3
        make install
        ```

        
    1. make shared for webkit2_extu

        ```
        ../configure --prefix=$(pwd)/../../../../platform/linux --with-opengl  --enable-webview --enable-webviewwebkit --enable-mediactrl  CXX="g++ -O2 -g3  -std=c++11"
        
        make -j 3
        make instal
        ```

* openssl
    
    * https://wiki.openssl.org/index.php/Compilation_and_Installation
    
    <br>
    make lib
    
    ```
    cd external/openssl/openssl-1.1.1h
    
    ./config --prefix=$(pwd)/../../../platform/linux --openssldir=/usr/local/ssl
    
    make depend
    make all
    make install
    ```

* curl

    * https://curl.se/docs/install.html
    * https://raspberrypi.stackexchange.com/questions/96646/installing-multiple-versions-of-curl-on-raspbian-stretch
    
    <br>
    make lib
    
    ```
    cd external/curl/curl-7.64.1
    
    ./configure --prefix=$(pwd)/../../../platform/linux  CPPFLAGS="-I../../../platform/linux/include/openssl" LDFLAGS="-L../../../platform/linux/lib"  --with-ssl --disable-shared
    
    make
    make install
    ```

* CryptoPP
    * https://www.cryptopp.com/wiki/Linux_(Command_Line)
    * https://github.com/weidai11/cryptopp/blob/master/Install.txt
    
    <br>
    make lib
    
    ```
    cd external/CryptoPP/cryptopp-master
    
    export CXXFLAGS="-DNDEBUG -std=c++11"
    make libcryptopp.a  
    make install PREFIX=$(pwd)/../../../platform/linux
    ```

* codelite

    * https://github.com/eranif/codelite
   <br>
    build codelite

    ```
    sudo apt-get install libgtk-3-dev pkg-config build-essential git cmake libssh-dev libwxgtk3.0-gtk3-dev libsqlite3-dev
    
    git clone https://github.com/eranif/codelite.git
    
    cd codelite
    
    mkdir build-release
    cd build-release
    
    cmake .. -DCMAKE_BUILD_TYPE=Release
    cmake --build . -j $(nproc)
    sudo cmake --build . --target install
    ```

* Solar2DTux/plaform/linux

    1. symbolic link to wx include folder

        ```
        ln -s ./include/wx-3.1/wx
        ```
    1. copy setup.h to wx folder

        ```
        cp lib/wx/include/gtk3-unicode-static-3.1/wx/setup.h wx/
        ```
   <br>
    build each project of solar2D

    * renamed and modifed Solar2DTux.workspace 
    
        * linux_rtt.project to Solar2DSimulator.project
        * car.project       to Solar2Dcar.project

         <br>
        Solar2DTux.workspace

            ```
            <Project Name="Solar2DSimulator" Path="Solar2DSimulator.project" Active="Yes"/>
            <Project Name="Solar2Dcar" Path="Solar2Dcar.project" Active="No"/>
            <Project Name="Solar2DConsole" Path="Solar2DTuxConsole.project" Active="No"/>
            <Project Name="Solar2DBuilder" Path="Solar2DBuilder.project" Active="No"/>
            ```
        
        * Solar2DBuilder
        * Solar2Dcar
        * Solar2DSimulator
        * Solar2DConsole

        <br>
        changes in .project files due to lib and include path

        ```
        <IncludePath Value="./include"/>
        <IncludePath Value="./wx/include"/>
        
        <Linker Options="-export-dynamic ./lib/libcryptopp.a ./lib/libcurl.a ./lib/libssl.a ./lib/libcrypto.a ./lib/libwx_gtk3u-3.1.a ./lib/libwx_gtk3u_gl-3.1.a ./lib/libwxtiff-3.1.a ./lib/libwxregexu-3.1.a -lGL -lopenal -lgdk-3 -lpangocairo-1.0 -lgthread-2.0 -pthread -lX11 -lXxf86vm -lSM -lgtk-3 -latk-1.0 -lcairo-gobject -lcairo -lgdk_pixbuf-2.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0 -lXtst -lpangoft2-1.0 -lpango-1.0 -lfontconfig -lfreetype -lpng -lz -lexpat -ljpeg -ldl -lm -lwebkit2gtk-4.0 -ljavascriptcoregtk-4.0 -lgstreamer-1.0 -lgstvideo-1.0" Required="yes"/>

        ```

    * for each .project of codelite, postBuild to copy each built binary to **Solar2DSimulator** folder

        ```
        <postBuild>
            <Command Enabled="yes">cd Solar2DSimulator $amp;$amp; mv ../build-$(ConfigurationName)/bin/$(ProjectName) $(ProjectName) 
        </postBuild>
        ```

    * changes because due to the changes of include and lib path

        * lua_to_native.sh for lua5.1 path
            ```
            BIN_DIR=/usr/bin
            ```
        * RTT_HTTPClientLinux.cpp
            ```
            #include <curl/curl.h>
            ```
        * RTT_LinuxCrypto.cpp
            ```
            #include <cryptopp/hmac.h>
            #include <cryptopp/md4.h>
            #include <cryptopp/md5.h>
            #include <cryptopp/sha.h>
            ```

    1. make Builder

        ```    
        codelite-make --workspace=Solar2DTux.workspace --project=Solar2DBuilder --config=Release --execute
        ```
         <br>
        Fixes in the project file for compiling errors

        * external/loop-2.3-beta/lua/precompiler.constant.lua
        
        > bin/linux/lua2c.sh is a symbolic link unless downloaded the repo as a zipped file. clone it. If downloading the repo as a zip file, you need to modify to work linking to the mac's lua2c.sh
    
            ```
            ../../bin/mac/lua2c.sh $1 $2 $3 $4 /usr/bin
            ```

    1. setup_dev.sh for downloading other dependencies
    
        edit the script to set the version number to match **3.1.4**
        ```
        sudo cp ../../external/wx/wxWidgets-3.1.4/gtk-build-so/webkit2_extu-3.1.4.so /usr/local/lib/wx/3.1.4/web-extensions/webkit2_extu-3.1.4.so
        ```

        run the script
        ```
        ./setup_dev.sh
        ```

    1. make Console

        ```
        codelite-make --workspace=Solar2DTux.workspace --project=Solar2DConsole --config=Release --execute
        ```

    1. make car

        ```
        codelite-make --workspace=Solar2DTux.workspace --project=Solar2Dcar --config=Release --execute
        ```

    1. clean 

        ```
        codelite-make --workspace=Solar2DTux.workspace --project=Solar2DSimulator --config=Release --execute -d clean
        ```

    1. make Simulator

        ```
        codelite-make --workspace=Solar2DTux.workspace --project=Solar2DSimulator --config=Release --execute
        ```

    * caveats
        * lua.project is not used but lua2cpp.mk is called in PreBuild of Solar2DSimulator

        * using jdk header in Solar2DSimulator?

            ```
            $(IncludeSwitch)/usr/lib/jvm/java-14-openjdk/include/ $(IncludeSwitch)/usr/lib/jvm/java-14-openjdk/include/linux/ 
            ```
