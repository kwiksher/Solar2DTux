## Jetson Nano

* jetson boot bug 

	https://forums.developer.nvidia.com/t/jetson-nano-blank-screen-during-and-after-boot/160356/27

	ctrl-alt-F5 and get a log in prompt. Not sure why f5 and not f2-f4

	```
	systemctl restart gdm3
	systemctl status gdm3
	```


* core pattern

	sudo sysctl -w kernel.core_pattern=/home/pi/work/tmp/core-%e-%s-%u-%g-%p-%t

* memory check with valgrind

	valgrind --leak-check=full ./Solar2DSimulator

* [libgl debug](https://askubuntu.com/questions/541343/problems-with-libgl-fbconfigs-swrast-through-each-update)

	LIBGL_DEBUG=verbose glxgears



---
## Mac

* [setup xQuartz](https://medium.com/swlh/gimp-running-desktop-applications-inside-a-docker-container-b3c82a58fe1c)
	```
	xhost +local:docker
	```
	Test

	```
	brew cask install xquartz

	defaults write org.macosforge.xquartz.X11 login_shell "/bin/bash"
	defaults write org.macosforge.xquartz.X11 no_auth -bool false
	defaults write org.macosforge.xquartz.X11 nolisten_tcp -bool false

	#OpenGL
	defaults write org.macosforge.xquartz.X11 enable_iglx -bool true

	docker run --rm \
	--net=host -e DISPLAY=kwiksher.local:0 \
	-v ~/.Xauthority:/root/.Xauthority  \
	alpine sh -c 'apk add --no-cache xeyes && xeyes'
	```


* [How do I prevent X11 opening an xterm when it starts?](https://apple.stackexchange.com/questions/53734/how-do-i-prevent-x11-opening-an-xterm-when-it-starts)

	```
	defaults write org.macosforge.xquartz.X11 app_to_run ""
	```



---
## Windows 

* WSL Liux Subsystem

	* /etc/wsl.conf

		Edit or create (using sudo) /etc/wsl.conf and add the following:

		```
		[automount]
		options = "metadata"
		```
* Docker

	use an external drive with docker

		```
		wsl --export docker-desktop-data "E:\Docker\wsl\data\docker-desktop-data.tar"
		wsl --unregister docker-desktop-data
		wsl --import docker-desktop-data "E:\Docker\wsl\data" "E:\Docker\wsl\data\docker-desktop-data.tar" --version 2

		```

* X11 Forwarding with vcxsrv

	https://medium.com/@potatowagon/how-to-use-gui-apps-in-linux-docker-container-from-windows-host-485d3e1c64a3

	* do note to check “Disable access control”

	In linux
	```
	export DISPLAY=:0
	```

	or remort 
	```
	export DISPLAY=192.168.99.1:0.0
	```

	test

	```
	xcalc&
	```

---
## Docker

* arm64 ubuntu 18.04

	```
	docker run -it arm64v8/ubuntu:18.04

	```

	working directory

	```
	adduser tux
	```
	
	for wxWidgets

	```
	sudo apt-get install freeglut3-dev
	sudo apt-get install libglfw3-dev libgl1-mesa-dev libglu1-mesa-dev
	sudo apt install libxxf86vm-dev libopenal-dev libjpeg-dev libwebkit2gtk-4.0-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev
	dpkg -L freeglut3-dev
	```

	```
	sudo apt full-upgrade
	sudo apt install libcairo2-dev
	```

	commit docker
	```
	docker commit CONTAINER_ID ubuntu_arm64:2021-01-07
	```

	restart docker with -v E:/Jetson_Nano_2GB:/home/tux/work

	```
	docker container run  --privileged --interactive  --tty --net=host -e DISPLAY="$(hostname):0" -v E:/Jetson_Nano_2GB:/home/tux/work -u tux  --restart=always --name ubuntu_arm64 ubuntu_arm64:2021-01-07 /bin/bash
	```
	
	copy sysroot folders to /home/tux/work 

	```
	cd /home/tux/work
	mdir sysroot_ubuntu & cd sysroot_ubuntu

	mkdir lib
	cp -rL /lib/aarch64-linux-gnu ./lib

	mkdir usr
	cp -rL /usr/lib ./usr
	cp -rL /usr/include ./usr

	cd ./usr
	mkdir share
	cp -rL /usr/share/pkgconfig ./share
	``

* ssh with keys

	```
	rsync -auz -e "ssh -i ~./.ssh/authorized_keys_nano" ymmtny@192.168.100.119:/Users/ymmtny/Documents/work/Solar2DTux/platform/linux/Solar2DSimulator/ /home/ymmtny/tmp/
	```