##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=Solar2DConsole
ConfigurationName      :=Debug
WorkspacePath          :=/home/danny/Documents/Projects/Solar2D/Solar2DTux/platform/linux
ProjectPath            :=/home/danny/Documents/Projects/Solar2D/Solar2DTux/platform/linux
IntermediateDirectory  :=./Console
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=Danny
Date                   :=16/07/20
CodeLitePath           :=/home/danny/.codelite
LinkerName             :=/usr/bin/g++
SharedObjectLinkerName :=/usr/bin/g++ -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.i
DebugSwitch            :=-g 
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputFile             :=Solar2DSimulator/$(ProjectName)
Preprocessors          :=$(PreprocessorSwitch)wxUSE_MEDIACTRL=1 $(PreprocessorSwitch)wxUSE_WEBVIEW=1 $(PreprocessorSwitch)__WXGTK__ $(PreprocessorSwitch)HAVE_SSIZE_T $(PreprocessorSwitch)wxUSE_XTEST $(PreprocessorSwitch)wxHAS_INOTIFY $(PreprocessorSwitch)wxUSE_SELECT_DISPATCHER $(PreprocessorSwitch)wxUSE_LIBMSPACK $(PreprocessorSwitch)WXWIN_COMPATIBILITY_3_0 $(PreprocessorSwitch)wxUSE_TASKBARBUTTON $(PreprocessorSwitch)wxUSE_DBGHELP $(PreprocessorSwitch)wxUSE_WINRT $(PreprocessorSwitch)wxUSE_PRIVATE_FONTS $(PreprocessorSwitch)wxUSE_ACTIVITYINDICATOR $(PreprocessorSwitch)wxUSE_ADDREMOVECTRL $(PreprocessorSwitch)wxUSE_UNSAFE_WXSTRING_CONV $(PreprocessorSwitch)wxUSE_STD_CONTAINERS_COMPATIBLY $(PreprocessorSwitch)wxUSE_SECRETSTORE $(PreprocessorSwitch)wxUSE_GLCANVAS $(PreprocessorSwitch)FT2_BUILD_LIBRARY 
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :="Solar2DConsole.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
LinkOptions            :=  -export-dynamic ./wx/lib/libwx_gtk3u-3.1.a ./wx/lib/libwx_gtk3u_gl-3.1.a ./wx/lib/libwxtiff-3.1.a ./wx/lib/libwxregexu-3.1.a ./wx/lib/libwxscintilla-3.1.a -lGL -lopenal -lgdk-3 -lpangocairo-1.0 -lgthread-2.0 -pthread -lX11 -lXxf86vm -lSM -lgtk-3 -latk-1.0 -lcairo-gobject -lcairo -lgdk_pixbuf-2.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0 -lXtst -lpangoft2-1.0 -lpango-1.0 -lfontconfig -lfreetype -lpng -lz -lexpat -ljpeg -ldl -lm -lwebkit2gtk-4.0 -ljavascriptcoregtk-4.0 -lgstreamer-1.0 -lgstvideo-1.0
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch)./wx/include 
IncludePCH             := 
RcIncludePath          := 
Libs                   := 
ArLibs                 :=  
LibPath                := $(LibraryPathSwitch). 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := /usr/bin/ar rcu
CXX      := /usr/bin/g++
CC       := /usr/bin/gcc
CXXFLAGS :=  -O0 -g3 -std=c++11 -Wfatal-errors -Wno-narrowing $(Preprocessors)
CFLAGS   :=  -O0 -g3 -Wall $(Preprocessors)
ASFLAGS  := 
AS       := /usr/bin/as


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=$(IntermediateDirectory)/Rtt_LinuxConsole.cpp$(ObjectSuffix) $(IntermediateDirectory)/Rtt_LinuxIPCServer.cpp$(ObjectSuffix) $(IntermediateDirectory)/Rtt_LinuxIPCConnectionBase.cpp$(ObjectSuffix) $(IntermediateDirectory)/Rtt_LinuxConsoleApp.cpp$(ObjectSuffix) $(IntermediateDirectory)/Rtt_LinuxIPCServerConnection.cpp$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

MakeIntermediateDirs:
	@test -d ./Console || $(MakeDirCommand) ./Console


$(IntermediateDirectory)/.d:
	@test -d ./Console || $(MakeDirCommand) ./Console

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/Rtt_LinuxConsole.cpp$(ObjectSuffix): Rtt_LinuxConsole.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Rtt_LinuxConsole.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Rtt_LinuxConsole.cpp$(DependSuffix) -MM Rtt_LinuxConsole.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/danny/Documents/Projects/Solar2D/Solar2DTux/platform/linux/Rtt_LinuxConsole.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Rtt_LinuxConsole.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Rtt_LinuxConsole.cpp$(PreprocessSuffix): Rtt_LinuxConsole.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Rtt_LinuxConsole.cpp$(PreprocessSuffix) Rtt_LinuxConsole.cpp

$(IntermediateDirectory)/Rtt_LinuxIPCServer.cpp$(ObjectSuffix): Rtt_LinuxIPCServer.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Rtt_LinuxIPCServer.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Rtt_LinuxIPCServer.cpp$(DependSuffix) -MM Rtt_LinuxIPCServer.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/danny/Documents/Projects/Solar2D/Solar2DTux/platform/linux/Rtt_LinuxIPCServer.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Rtt_LinuxIPCServer.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Rtt_LinuxIPCServer.cpp$(PreprocessSuffix): Rtt_LinuxIPCServer.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Rtt_LinuxIPCServer.cpp$(PreprocessSuffix) Rtt_LinuxIPCServer.cpp

$(IntermediateDirectory)/Rtt_LinuxIPCConnectionBase.cpp$(ObjectSuffix): Rtt_LinuxIPCConnectionBase.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Rtt_LinuxIPCConnectionBase.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Rtt_LinuxIPCConnectionBase.cpp$(DependSuffix) -MM Rtt_LinuxIPCConnectionBase.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/danny/Documents/Projects/Solar2D/Solar2DTux/platform/linux/Rtt_LinuxIPCConnectionBase.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Rtt_LinuxIPCConnectionBase.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Rtt_LinuxIPCConnectionBase.cpp$(PreprocessSuffix): Rtt_LinuxIPCConnectionBase.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Rtt_LinuxIPCConnectionBase.cpp$(PreprocessSuffix) Rtt_LinuxIPCConnectionBase.cpp

$(IntermediateDirectory)/Rtt_LinuxConsoleApp.cpp$(ObjectSuffix): Rtt_LinuxConsoleApp.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Rtt_LinuxConsoleApp.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Rtt_LinuxConsoleApp.cpp$(DependSuffix) -MM Rtt_LinuxConsoleApp.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/danny/Documents/Projects/Solar2D/Solar2DTux/platform/linux/Rtt_LinuxConsoleApp.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Rtt_LinuxConsoleApp.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Rtt_LinuxConsoleApp.cpp$(PreprocessSuffix): Rtt_LinuxConsoleApp.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Rtt_LinuxConsoleApp.cpp$(PreprocessSuffix) Rtt_LinuxConsoleApp.cpp

$(IntermediateDirectory)/Rtt_LinuxIPCServerConnection.cpp$(ObjectSuffix): Rtt_LinuxIPCServerConnection.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Rtt_LinuxIPCServerConnection.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/Rtt_LinuxIPCServerConnection.cpp$(DependSuffix) -MM Rtt_LinuxIPCServerConnection.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/danny/Documents/Projects/Solar2D/Solar2DTux/platform/linux/Rtt_LinuxIPCServerConnection.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Rtt_LinuxIPCServerConnection.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Rtt_LinuxIPCServerConnection.cpp$(PreprocessSuffix): Rtt_LinuxIPCServerConnection.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Rtt_LinuxIPCServerConnection.cpp$(PreprocessSuffix) Rtt_LinuxIPCServerConnection.cpp


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r ./Console/


