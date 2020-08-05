//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#define _chdir chdir
#include <string.h>
#include "Core/Rtt_Build.h"
#include "Core/Rtt_Time.h"
#include "Rtt_Runtime.h"
#include "Rtt_LuaContext.h"
#include "Core/Rtt_Types.h"
#include "Rtt_LinuxContext.h"
#include "Rtt_LinuxPlatform.h"
#include "Rtt_LinuxRuntimeDelegate.h"
#include "Rtt_LuaFile.h"
#include "Core/Rtt_FileSystem.h"
#include "Rtt_Archive.h"
#include "Display/Rtt_Display.h"
#include "Display/Rtt_DisplayDefaults.h"
#include "Rtt_Freetype.h"
#include "Rtt_LuaLibSimulator.h"
#include "Rtt_LinuxSimulatorView.h"
#include "Rtt_LinuxFileUtils.h"
#include "Rtt_MPlatformServices.h"
#include "Rtt_LinuxMenuEvents.h"
#include "Rtt_ConsoleApp.h"
#include "wx/menu.h"
#include "wx/dcclient.h"
#include "wx/app.h"
#include "wx/display.h"

#if !defined(wxHAS_IMAGES_IN_RESOURCES) && defined(Rtt_SIMULATOR)
#include "resource/simulator.xpm"
#endif

//#define Rtt_DEBUG_TOUCH 1
#define TIMER_ID wxID_HIGHEST + 1

using namespace Rtt;
using namespace std;

wxDEFINE_EVENT(eventOpenProject, wxCommandEvent);
wxDEFINE_EVENT(eventRelaunchProject, wxCommandEvent);
wxDEFINE_EVENT(eventWelcomeProject, wxCommandEvent);

static bool IsHomeScreen(string appName)
{
	return appName.compare(HOMESCREEN_ID) == 0;
}

namespace Rtt
{
	static ProjectSettings *fProjectSettings;

	SolarAppContext::SolarAppContext(const char *path)
		: fRuntime(NULL), fRuntimeDelegate(new LinuxRuntimeDelegate()), fMouseListener(NULL), fKeyListener(NULL), fPlatform(NULL), fTouchDeviceExist(false), fMode("normal"), fIsDebApp(false), fSimulator(NULL), fIsStarted(false)
	{
		string exeFileName;
		const char *homeDir = LinuxFileUtils::GetHomePath();
		const char *appPath = LinuxFileUtils::GetStartupPath(&exeFileName);

		// override appPath if arg isn't null
		if (path && *path != 0)
		{
			appPath = path;
		}

		// set app name
		if (strcmp(appPath, "/usr/bin") == 0) // deb ?
		{
			// for .deb app the appName is exe file name
			fAppName = exeFileName;
		}
		else
		{
			const char *slash = strrchr(appPath, '/');

			if (slash)
			{
				fAppName = slash + 1;
			}
			else
			{
				slash = strrchr(appPath, '\\');

				if (slash)
				{
					fAppName = slash + 1;
				}
			}
		}

		Rtt_ASSERT(fAppName.size() > 0);
		string startDir(appPath);

		fSaveFolder.append(homeDir);
		fSaveFolder.append("/Documents/Solar2D Built Apps");

		string assetsDir = startDir;
		assetsDir.append("/Resources/resource.car");

		if (Rtt_FileExists(assetsDir.c_str()))
		{
			fPathToApp = startDir;
			return;
		}

		assetsDir = startDir;
		assetsDir.append("/main.lua");

		if (Rtt_FileExists(assetsDir.c_str()))
		{
			fPathToApp = startDir;
			return;
		}

		// look for welcomescereen
		startDir = LinuxFileUtils::GetStartupPath(NULL);
		startDir.append("/Resources/homescreen");
		assetsDir = startDir;
		assetsDir.append("/main.lua");

		if (Rtt_FileExists(assetsDir.c_str()))
		{
			fAppName = HOMESCREEN_ID;
			fPathToApp = startDir;
			fIsDebApp = false;
			return;
		}

		Rtt_LogException("Failed to find app\n");
		Rtt_ASSERT(0);
	}

	SolarAppContext::~SolarAppContext()
	{
		Close();
	}

	void SolarAppContext::Close()
	{
		delete fMouseListener;
		fMouseListener = NULL;
		delete fKeyListener;
		fKeyListener = NULL;
		delete fRuntime;
		fRuntime = NULL;
		delete fRuntimeDelegate;
		fRuntimeDelegate = NULL;
		delete fPlatform;
		fPlatform = NULL;
		delete fSimulator;
		fSimulator = NULL;

		setGlyphProvider(NULL);
	}

	bool SolarAppContext::Init()
	{
		const char *homeDir = LinuxFileUtils::GetHomePath();
		string appDir(homeDir);

		if (LinuxSimulatorView::IsRunningOnSimulator())
		{
			appDir.append("/.Solar2D/Sandbox/");
		}
		else
		{
			appDir.append("/.local/share/");
		}

		if (!IsHomeScreen(fAppName))
		{
			appDir.append(fAppName);

			if (LinuxSimulatorView::IsRunningOnSimulator())
			{
				appDir.append("_");
				appDir.append(LinuxFileUtils::CalculateMD5(fAppName));
			}
		}
		else
		{
			if (LinuxSimulatorView::IsRunningOnSimulator())
			{
				appDir.append("Simulator");
			}
		}

		if (!Rtt_IsDirectory(appDir.c_str()))
		{
			Rtt_MakeDirectory(appDir.c_str());
		}

		string documentsDir(appDir);
		string temporaryDir(appDir);
		string cachesDir(appDir);
		string systemCachesDir(appDir);
		string skinDir("/opt/Solar2D/Resources/Skins");

		// setup directory paths
		documentsDir.append("/Documents");
		temporaryDir.append("/TemporaryFiles");
		cachesDir.append("/CachedFiles");
		systemCachesDir.append("/.system");

		if (!Rtt_IsDirectory(documentsDir.c_str()))
		{
			Rtt_MakeDirectory(documentsDir.c_str());
		}

		if (!Rtt_IsDirectory(systemCachesDir.c_str()))
		{
			Rtt_MakeDirectory(systemCachesDir.c_str());
		}

		if (!Rtt_IsDirectory(temporaryDir.c_str()))
		{
			Rtt_MakeDirectory(temporaryDir.c_str());
		}

		if (!Rtt_IsDirectory(cachesDir.c_str()))
		{
			Rtt_MakeDirectory(cachesDir.c_str());
		}

		setGlyphProvider(new glyph_freetype_provider(fPathToApp.c_str()));
		fPlatform = new LinuxPlatform(fPathToApp.c_str(), documentsDir.c_str(), temporaryDir.c_str(), cachesDir.c_str(), systemCachesDir.c_str(), skinDir.c_str(), LinuxFileUtils::GetStartupPath(NULL));
		fRuntime = new LinuxRuntime(*fPlatform, NULL);
		fRuntime->SetDelegate(fRuntimeDelegate);

		if (LinuxSimulatorView::IsRunningOnSimulator())
		{
			fRuntime->SetProperty(Runtime::kLinuxMaskSet | Runtime::kIsApplicationNotArchived, true);
		}
		else
		{
			fRuntime->SetProperty(Runtime::kLinuxMaskSet, true);
		}

		bool fullScreen = false;
		int width = 320;
		int height = 480;
		string projectPath(fPathToApp.c_str());

		if (!LinuxSimulatorView::IsRunningOnSimulator())
		{
			projectPath.append("/Resources");
		}

		fProjectSettings->ResetBuildSettings();
		fProjectSettings->ResetConfigLuaSettings();
		fProjectSettings->LoadFromDirectory(projectPath.c_str());

		// read config.lua
		if (fProjectSettings->HasConfigLua())
		{
			if (width <= 0 || height <= 0)
			{
				width = fProjectSettings->GetContentWidth();
				height = fProjectSettings->GetContentHeight();
			}
		}

		// read build.settings
		if (fProjectSettings->HasBuildSettings())
		{
			wxString localeName = wxLocale::GetLanguageInfo(wxLocale::GetSystemLanguage())->CanonicalName.Lower();
			string langCode = localeName.ToStdString().substr(0, 2);
			string countryCode = localeName.ToStdString().substr(3, 5);
			int minWidth = fProjectSettings->GetMinWindowViewWidth();
			int minHeight = fProjectSettings->GetMinWindowViewHeight();
			const char *windowTitle = fProjectSettings->GetWindowTitleTextForLocale(langCode.c_str(), countryCode.c_str());
			const Rtt::NativeWindowMode *nativeWindowMode = fProjectSettings->GetDefaultWindowMode();
			DeviceOrientation::Type orientation = fProjectSettings->GetDefaultOrientation();

			if (windowTitle != NULL)
			{
				fTitle = windowTitle;
			}

			if (*nativeWindowMode == Rtt::NativeWindowMode::kFullscreen)
			{
				fMode = NativeWindowMode::kFullscreen.GetStringId();
			}

			fullScreen = strcmp(fMode, NativeWindowMode::kFullscreen.GetStringId()) == 0;

			if (fullScreen)
			{
				wxDisplay display(wxDisplay::GetFromWindow(wxGetApp().GetFrame()));
				wxRect screen = display.GetClientArea();
				width = screen.width;
				height = screen.height;
			}
			else
			{
				width = fProjectSettings->GetDefaultWindowViewWidth();
				height = fProjectSettings->GetDefaultWindowViewHeight();
				wxGetApp().GetFrame()->SetMinClientSize(wxSize(minWidth, minHeight));
			}

			switch(orientation)
			{
				case DeviceOrientation::kSidewaysRight:
					fRuntimeDelegate->fOrientation = DeviceOrientation::kSidewaysRight; // bottom of device is to the right

					if (width > 0 && height > 0)
					{
						fRuntimeDelegate->fContentWidth = width;
						fRuntimeDelegate->fContentHeight = height;
					}
					else
					{
						// no valid defaultViewWidth & defaultViewHeight in 'build.settings', default values of fWidth & fHeight for Portrait
						// use swapped default settings
						Swap(fRuntimeDelegate->fContentWidth, fRuntimeDelegate->fContentHeight);
					}
					break;

				case DeviceOrientation::kSidewaysLeft:
					fRuntimeDelegate->fOrientation = DeviceOrientation::kSidewaysLeft; // bottom of device is to the left

					if (width > 0 && height > 0)
					{
						fRuntimeDelegate->fContentWidth = width;
						fRuntimeDelegate->fContentHeight = height;
					}
					else
					{
						// no valid defaultViewWidth & defaultViewHeight in 'build.settings', default values of fWidth & fHeight for Portrait
						// use swapped default settings
						Swap(fRuntimeDelegate->fContentWidth, fRuntimeDelegate->fContentHeight);
					}
					break;

				case DeviceOrientation::kUpright:
					fRuntimeDelegate->fOrientation = DeviceOrientation::kUpright; // bottom of device is at the bottom

					if (width > 0 && height > 0)
					{
						fRuntimeDelegate->fContentWidth = width;
						fRuntimeDelegate->fContentHeight = height;
					}
					else
					{
						// no valid defaultViewWidth & defaultViewHeight in 'build.settings', default values of fWidth & fHeight for Portrait
						// use default settings
					}
					break;

				case DeviceOrientation::kUpsideDown:
					fRuntimeDelegate->fOrientation = DeviceOrientation::kUpsideDown; // bottom of device is at the top

					if (width > 0 && height > 0)
					{
						fRuntimeDelegate->fContentWidth = width;
						fRuntimeDelegate->fContentHeight = height;
					}
					else
					{
						// no valid defaultViewWidth & defaultViewHeight in 'build.settings', default values of fWidth & fHeight for Portrait
						// use default settings
					}
					break;
			}
		}

		fPlatform->fShowRuntimeErrors = LinuxSimulatorView::Config::showRuntimeErrors;
		fPlatform->setWindow(this);
		fMouseListener = new LinuxMouseListener(*fRuntime);
		fKeyListener = new LinuxKeyListener(*fRuntime);

		// Initialize Joystick Support:
		LinuxInputDeviceManager &deviceManager = (LinuxInputDeviceManager &)fPlatform->GetDevice().GetInputDeviceManager();
		deviceManager.init();
		wxGetApp().GetParent()->Layout();

		return fullScreen;
	}

	bool SolarAppContext::LoadApp(SolarGLCanvas *canvas)
	{
		fCanvas = canvas;

		if (Runtime::kSuccess != fRuntime->LoadApplication(Runtime::kLinuxLaunchOption, fRuntimeDelegate->fOrientation))
		{
			delete fRuntime;
			delete fPlatform;
			return false;
		}

		// add Resources to LUA_PATH
		string luapath(getenv("LUA_PATH"));
		luapath.append(LinuxFileUtils::GetStartupPath(NULL));
		luapath.append("/Resources/?.lua;");

		setenv("LUA_PATH", luapath.c_str(), true);

		if (fRuntimeDelegate->fOrientation == DeviceOrientation::kSidewaysRight || fRuntimeDelegate->fOrientation == DeviceOrientation::kSidewaysLeft)
		{
			// Swap(fRuntimeDelegate->fContentWidth, fRuntimeDelegate->fContentHeight);
		}

		if (LinuxSimulatorView::IsRunningOnSimulator())
		{
			fSimulator = new LinuxSimulatorServices();
			lua_State *luaStatePointer = fRuntime->VMContext().L();
			lua_pushlightuserdata(luaStatePointer, fSimulator);
			Rtt::LuaContext::RegisterModuleLoader(luaStatePointer, Rtt::LuaLibSimulator::kName, Rtt::LuaLibSimulator::Open, 1);
		}

		return true;
	}

	void SolarAppContext::Flush()
	{
		fCanvas->Refresh(false);
	}

	void SolarAppContext::Pause()
	{
		if (!fRuntime->IsSuspended())
		{
			fRuntime->Suspend();
		}
	}

	void SolarAppContext::Resume()
	{
		if (GetRuntime()->IsSuspended())
		{
			fRuntime->DispatchEvent(ResizeEvent());
			fRuntime->Resume();
		}
	}
} // namespace Rtt

// App implementation
SolarApp::SolarApp()
{
	const char *homeDir = LinuxFileUtils::GetHomePath();
	string basePath(homeDir);
	string buildPath(homeDir);
	string projectCreationPath(homeDir);

	basePath.append("/.Solar2D");
	buildPath.append("/Documents/Solar2D Built Apps");
	projectCreationPath.append("/Documents/Solar2D Projects");

	// create default directories if missing
	if (!Rtt_IsDirectory(basePath.c_str()))
	{
		Rtt_MakeDirectory(basePath.c_str());
	}

	if (!Rtt_IsDirectory(buildPath.c_str()))
	{
		Rtt_MakeDirectory(buildPath.c_str());
	}

	if (!Rtt_IsDirectory(projectCreationPath.c_str()))
	{
		Rtt_MakeDirectory(projectCreationPath.c_str());
	}

	// start the console immediately
	if (LinuxSimulatorView::IsRunningOnSimulator())
	{
		wxExecute("./Solar2DConsole");
	}
}

SolarApp::~SolarApp()
{
}

bool SolarApp::OnInit()
{
	if (wxApp::OnInit())
	{
		bool fullScreen = false;
		int windowStyle = wxCAPTION;
		int width = 320;
		int height = 480;
		int minWidth = width;
		int minHeight = height;
		string projectPath(LinuxFileUtils::GetStartupPath(NULL));

		if (LinuxSimulatorView::IsRunningOnSimulator())
		{
			projectPath.append("/Resources/homescreen");
		}
		else
		{
			projectPath.append("/Resources");
		}

		fProjectSettings = new ProjectSettings();
		fProjectSettings->LoadFromDirectory(projectPath.c_str());

		// grab the required config settings (we only need width/height at this stage)
		if (fProjectSettings->HasConfigLua())
		{
			width = fProjectSettings->GetContentWidth();
			height = fProjectSettings->GetContentHeight();
		}

		// grab the build settings (we only need width/height at this stage)
		if (fProjectSettings->HasBuildSettings())
		{
			const Rtt::NativeWindowMode *nativeWindowMode = fProjectSettings->GetDefaultWindowMode();
			bool isWindowMinimizeButtonEnabled = fProjectSettings->IsWindowMinimizeButtonEnabled();
			bool isWindowMaximizeButtonEnabled = fProjectSettings->IsWindowMaximizeButtonEnabled();
			bool isWindowCloseButtonEnabled = fProjectSettings->IsWindowCloseButtonEnabled();
			bool isWindowResizable = fProjectSettings->IsWindowResizable();
			width = fProjectSettings->GetDefaultWindowViewWidth();
			height = fProjectSettings->GetDefaultWindowViewHeight();
			minWidth = fProjectSettings->GetMinWindowViewWidth();
			minHeight = fProjectSettings->GetMinWindowViewHeight();

			if (*nativeWindowMode == Rtt::NativeWindowMode::kNormal)
			{
			}
			else if (*nativeWindowMode == Rtt::NativeWindowMode::kMinimized)
			{
			}
			else if (*nativeWindowMode == Rtt::NativeWindowMode::kMaximized)
			{
			}
			else if (*nativeWindowMode == Rtt::NativeWindowMode::kFullscreen)
			{
				fullScreen = true;
			}

			if (isWindowMinimizeButtonEnabled)
			{
				windowStyle |= wxMINIMIZE_BOX;
			}

			if (isWindowMaximizeButtonEnabled)
			{
				windowStyle |= wxMAXIMIZE_BOX | wxRESIZE_BORDER;
			}

			if (isWindowCloseButtonEnabled)
			{
				windowStyle |= wxCLOSE_BOX;
			}

			if (isWindowResizable)
			{
				windowStyle |= wxRESIZE_BORDER;
			}

			if (fullScreen)
			{
				windowStyle = wxDEFAULT_FRAME_STYLE;
			}
		}

		// sanity checks
		if (width <= 0)
		{
			width = 320;
			minWidth = width;
		}

		if (height <= 0)
		{
			height = 480;
			minHeight = height;
		}

		if (LinuxSimulatorView::IsRunningOnSimulator())
		{
			// read from the simulator config file (it'll be created if it doesn't exist)
			LinuxSimulatorView::Config::Load();
		}

		// create the main application window
		fSolarFrame = new SolarFrame(windowStyle);

		if (fullScreen)
		{
			wxDisplay display(wxDisplay::GetFromWindow(fSolarFrame));
			wxRect screen = display.GetClientArea();
			width = screen.width;
			height = screen.height;
		}

		// test if the OGL context could be created
		if (fSolarFrame->fSolarGLCanvas->IsGLContextAvailable())
		{
			fSolarFrame->SetClientSize(wxSize(width, height));
			fSolarFrame->SetSize(wxSize(width, height));
			fSolarFrame->SetMinClientSize(wxSize(minWidth, minHeight));

			if (fullScreen)
			{
				fSolarFrame->ShowFullScreen(true);
			}
			else
			{
				fSolarFrame->SetPosition(wxPoint(LinuxSimulatorView::Config::windowXPos, LinuxSimulatorView::Config::windowYPos));
				fSolarFrame->Show(true);
			}

			wxInitAllImageHandlers();
			return true;
		}
	}

	return false;
}

void SolarApp::OnEventLoopEnter(wxEventLoopBase *WXUNUSED(loop))
{
	static bool firstRun = true;

	if (firstRun)
	{
		wxCommandEvent eventOpen(eventOpenProject);

		if (LinuxSimulatorView::IsRunningOnSimulator())
		{
			if (LinuxSimulatorView::Config::openLastProject && !LinuxSimulatorView::Config::lastProjectDirectory.IsEmpty())
			{
				wxString fullPath(LinuxSimulatorView::Config::lastProjectDirectory);
				fullPath.append("/main.lua");
				eventOpen.SetInt(ID_MENU_OPEN_LAST_PROJECT);
				eventOpen.SetString(fullPath);
			}
		}

		fSolarFrame->OnOpen(eventOpen);
		firstRun = false;
	}
}

wxWindow *SolarApp::GetParent()
{
	return GetFrame();
}

LinuxPlatform *SolarApp::GetPlatform() const
{
	return fSolarFrame->GetContext()->GetPlatform();
}

// setup frame events
wxBEGIN_EVENT_TABLE(SolarFrame, wxFrame)
	EVT_MENU(ID_MENU_OPEN_WELCOME_SCREEN, SolarFrame::OnOpenWelcome)
	EVT_MENU(ID_MENU_RELAUNCH_PROJECT, SolarFrame::OnRelaunch)
	EVT_MENU(ID_MENU_SUSPEND, SolarFrame::OnSuspendOrResume)
	EVT_MENU(ID_MENU_CLOSE_PROJECT, SolarFrame::OnOpenWelcome)
	EVT_COMMAND(wxID_ANY, eventOpenProject, SolarFrame::OnOpen)
	EVT_COMMAND(wxID_ANY, eventRelaunchProject, SolarFrame::OnRelaunch)
	EVT_COMMAND(wxID_ANY, eventWelcomeProject, SolarFrame::OnOpenWelcome)
	EVT_CLOSE(SolarFrame::OnClose)
wxEND_EVENT_TABLE()

SolarFrame::SolarFrame(int style)
	: wxFrame(NULL, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(320, 480), style), fSolarGLCanvas(NULL), fContext(NULL), fMenuMain(NULL), fMenuProject(NULL), fWatcher(NULL),
	  fProjectPath("")
{
#ifdef Rtt_SIMULATOR
	SetIcon(simulator_xpm);
#endif

	wxGLAttributes vAttrs;
	vAttrs.PlatformDefaults().Defaults().EndList();
	suspendedPanel = NULL;
	fRelaunchedViaFileEvent = false;
	bool accepted = wxGLCanvas::IsDisplaySupported(vAttrs);

	if (!accepted)
	{
		// try again without sample buffers
		vAttrs.Reset();
		vAttrs.PlatformDefaults().RGBA().DoubleBuffer().Depth(16).EndList();

		accepted = wxGLCanvas::IsDisplaySupported(vAttrs);

		if (!accepted)
		{
			Rtt_LogException("Failed to init OpenGL");
			return;
		}
	}

	CreateMenus();
	fSolarGLCanvas = new SolarGLCanvas(this, vAttrs);
	fRelaunchProjectDialog = new LinuxRelaunchProjectDialog(NULL, wxID_ANY, wxEmptyString);
	const char *homeDir = LinuxFileUtils::GetHomePath();
	fProjectPath = string(homeDir);
	fProjectPath.append("/Documents/Solar2D Projects");

	if (!Rtt_IsDirectory(fProjectPath.c_str()))
	{
		Rtt_MakeDirectory(fProjectPath.c_str());
	}

	Bind(wxEVT_MENU, &LinuxMenuEvents::OnNewProject, ID_MENU_NEW_PROJECT);
	Bind(wxEVT_MENU, &LinuxMenuEvents::OnOpenFileDialog, ID_MENU_OPEN_PROJECT);
	Bind(wxEVT_MENU, &LinuxMenuEvents::OnRelaunchLastProject, ID_MENU_OPEN_LAST_PROJECT);
	Bind(wxEVT_MENU, &LinuxMenuEvents::OnOpenInEditor, ID_MENU_OPEN_IN_EDITOR);
	Bind(wxEVT_MENU, &LinuxMenuEvents::OnShowProjectFiles, ID_MENU_SHOW_PROJECT_FILES);
	Bind(wxEVT_MENU, &LinuxMenuEvents::OnShowProjectSandbox, ID_MENU_SHOW_PROJECT_SANDBOX);
	Bind(wxEVT_MENU, &LinuxMenuEvents::OnClearProjectSandbox, ID_MENU_CLEAR_PROJECT_SANDBOX);
	Bind(wxEVT_MENU, &LinuxMenuEvents::OnBuildForAndroid, ID_MENU_BUILD_ANDROID);
	Bind(wxEVT_MENU, &LinuxMenuEvents::OnBuildForWeb, ID_MENU_BUILD_WEB);
	Bind(wxEVT_MENU, &LinuxMenuEvents::OnBuildForLinux, ID_MENU_BUILD_LINUX);
	Bind(wxEVT_MENU, &LinuxMenuEvents::OnOpenPreferences, wxID_PREFERENCES);
	Bind(wxEVT_MENU, &LinuxMenuEvents::OnHardwareRotateLeft, ID_MENU_ROTATE_LEFT);
	Bind(wxEVT_MENU, &LinuxMenuEvents::OnHardwareRotateRight, ID_MENU_ROTATE_RIGHT);
	Bind(wxEVT_MENU, &LinuxMenuEvents::OnQuit, wxID_EXIT);
	Bind(wxEVT_MENU, &LinuxMenuEvents::OnOpenDocumentation, ID_MENU_OPEN_DOCUMENTATION);
	Bind(wxEVT_MENU, &LinuxMenuEvents::OnOpenSampleProjects, ID_MENU_OPEN_SAMPLE_CODE);
	Bind(wxEVT_MENU, &LinuxMenuEvents::OnAbout, wxID_ABOUT);
}

SolarFrame::~SolarFrame()
{
	LinuxSimulatorView::Config::Cleanup();
	delete fWatcher;
	delete fSolarGLCanvas;

	fContext->Close();
	delete fContext;

	SetMenuBar(NULL);
	delete fMenuMain;
	delete fMenuProject;
}

void SolarFrame::WatchFolder(const char *path, const char *appName)
{
	if (IsHomeScreen(string(appName)))
	{
		// do not watch main screen folder
		return;
	}

	// wxFileSystemWatcher
	if (fWatcher == NULL)
	{
		fWatcher = new wxFileSystemWatcher();
		fWatcher->SetOwner(this);
		Connect(wxEVT_FSWATCHER, wxFileSystemWatcherEventHandler(SolarFrame::OnFileSystemEvent));
	}

	wxFileName fn = wxFileName::DirName(path);
	fn.DontFollowLink();
	fWatcher->RemoveAll();
	fWatcher->AddTree(fn);
}

void SolarFrame::ResetSize()
{
	wxSize clientSize = GetClientSize();

	if (IsFullScreen())
	{
		fContext->GetRuntimeDelegate()->fContentWidth = clientSize.GetWidth();
		fContext->GetRuntimeDelegate()->fContentHeight = clientSize.GetHeight();
	}

	SetClientSize(wxSize(fContext->GetWidth(), fContext->GetHeight()));
	Refresh(false);
	Update();
}

void SolarFrame::CreateMenus()
{
	if (LinuxSimulatorView::IsRunningOnSimulator())
	{
		{
			fMenuMain = new wxMenuBar();

			// file Menu
			wxMenu *fileMenu = new wxMenu();
			fileMenu->Append(ID_MENU_NEW_PROJECT, _T("&New Project	\tCtrl-N"));
			fileMenu->Append(ID_MENU_OPEN_PROJECT, _T("&Open Project	\tCtrl-O"));
			fileMenu->AppendSeparator();
			fileMenu->Append(ID_MENU_OPEN_LAST_PROJECT, _T("&Relaunch Last Project	\tCtrl-R"));
			fileMenu->AppendSeparator();
			fileMenu->Append(wxID_PREFERENCES, _T("&Preferences..."));
			fileMenu->AppendSeparator();
			fileMenu->Append(wxID_EXIT, _T("&Exit"));
			fMenuMain->Append(fileMenu, _T("&File"));

			// about menu
			wxMenu *m_pHelpMenu = new wxMenu();
			m_pHelpMenu->Append(ID_MENU_OPEN_DOCUMENTATION, _T("&Online Documentation..."));
			m_pHelpMenu->Append(ID_MENU_OPEN_SAMPLE_CODE, _T("&Sample projects..."));
			m_pHelpMenu->Append(wxID_ABOUT, _T("&About Simulator..."));
			fMenuMain->Append(m_pHelpMenu, _T("&Help"));
		}

		// project's menu
		{
			fMenuProject = new wxMenuBar();

			// file Menu
			wxMenu *fileMenu = new wxMenu();
			fileMenu->Append(wxID_NEW, _T("&New Project	\tCtrl-N"));
			fileMenu->Append(wxID_OPEN, _T("&Open Project	\tCtrl-O"));
			fileMenu->AppendSeparator();

			wxMenu *buildMenu = new wxMenu();
			buildMenu->Append(ID_MENU_BUILD_ANDROID, _T("Android	\tCtrl-B"));
			wxMenuItem *buildForWeb = buildMenu->Append(ID_MENU_BUILD_WEB, _T("HTML5	\tCtrl-Shift-Alt-B"));
			wxMenu *buildForLinuxMenu = new wxMenu();
			buildForLinuxMenu->Append(ID_MENU_BUILD_LINUX, _T("x64	\tCtrl-Alt-B"));
			wxMenuItem *buildForARM = buildForLinuxMenu->Append(ID_MENU_BUILD_LINUX, _T("ARM	\tCtrl-Alt-A"));
			buildMenu->AppendSubMenu(buildForLinuxMenu, _T("&Linux"));
			fileMenu->AppendSubMenu(buildMenu, _T("&Build"));
			buildForWeb->Enable(false);
			buildForARM->Enable(false);

			fileMenu->Append(ID_MENU_OPEN_IN_EDITOR, _T("&Open In Editor	\tCtrl-Shift-O"));
			fileMenu->Append(ID_MENU_SHOW_PROJECT_FILES, _T("&Show Project Files"));
			fileMenu->Append(ID_MENU_SHOW_PROJECT_SANDBOX, _T("&Show Project Sandbox"));
			fileMenu->AppendSeparator();
			fileMenu->Append(ID_MENU_CLEAR_PROJECT_SANDBOX, _T("&Clear Project Sandbox"));
			fileMenu->AppendSeparator();
			fileMenu->Append(ID_MENU_RELAUNCH_PROJECT, _T("Relaunch	\tCtrl-R"));
			fileMenu->Append(ID_MENU_CLOSE_PROJECT, _T("Close Project	\tCtrl-W"));
			fileMenu->AppendSeparator();
			fileMenu->Append(wxID_PREFERENCES, _T("&Preferences..."));
			fileMenu->AppendSeparator();
			fileMenu->Append(wxID_EXIT, _T("&Exit"));
			fMenuProject->Append(fileMenu, _T("&File"));

			// hardware menu
			fHardwareMenu = new wxMenu();
			wxMenuItem *rotateLeft = fHardwareMenu->Append(ID_MENU_ROTATE_LEFT, _T("&Rotate Left"));
			wxMenuItem *rotateRight = fHardwareMenu->Append(ID_MENU_ROTATE_RIGHT, _T("&Rotate Right"));
			//fHardwareMenu->Append(wxID_ABOUT, _T("&Shake"));
			fHardwareMenu->AppendSeparator();
			wxMenuItem *back = fHardwareMenu->Append(wxID_ABOUT, _T("&Back"));
			fHardwareMenu->AppendSeparator();
			fHardwareMenu->Append(ID_MENU_SUSPEND, _T("&Suspend	\tCtrl-Down"));
			fMenuProject->Append(fHardwareMenu, _T("&Hardware"));
			rotateLeft->Enable(true);
			rotateRight->Enable(true);
			back->Enable(false);

			// view menu
			wxMenu *viewMenu = new wxMenu();
			wxMenuItem *zoomIn = viewMenu->Append(wxID_HELP_CONTENTS, _T("&Zoom In"));
			wxMenuItem *zoomOut = viewMenu->Append(wxID_HELP_INDEX, _T("&Zoom Out"));
			viewMenu->AppendSeparator();
			wxMenuItem *viewAs = viewMenu->Append(wxID_ABOUT, _T("&View As"));
			zoomIn->Enable(false);
			zoomOut->Enable(false);
			viewAs->Enable(false);

			viewMenu->AppendSeparator();
			viewMenu->Append(ID_MENU_OPEN_WELCOME_SCREEN, _T("&Welcome Screen"));
			fMenuProject->Append(viewMenu, _T("&View"));

			// about menu
			wxMenu *helpMenu = new wxMenu();
			helpMenu->Append(ID_MENU_OPEN_DOCUMENTATION, _T("&Online Documentation..."));
			helpMenu->Append(ID_MENU_OPEN_SAMPLE_CODE, _T("&Sample projects..."));
			helpMenu->Append(wxID_ABOUT, _T("&About Simulator..."));
			fMenuProject->Append(helpMenu, _T("&Help"));
		}
	}
}

void SolarFrame::SetMenu(const char *appPath)
{
	if (LinuxSimulatorView::IsRunningOnSimulator())
	{
		const string &appName = GetContext()->GetAppName();
		SetMenuBar(IsHomeScreen(appName) ? fMenuMain : fMenuProject);
	}
}

void SolarFrame::OnClose(wxCloseEvent &event)
{
	fContext->GetRuntime()->End();

	LinuxSimulatorView::Config::windowXPos = GetPosition().x;
	LinuxSimulatorView::Config::windowYPos = GetPosition().y;
	LinuxSimulatorView::Config::Save();

	// quit the simulator console
	if (LinuxSimulatorView::IsRunningOnSimulator())
	{
		ConsoleApp::Quit();
		wxExit();
	}
}

void SolarFrame::OnFileSystemEvent(wxFileSystemWatcherEvent &event)
{
	if (fContext->GetRuntime()->IsSuspended())
	{
		return;
	}

	int type = event.GetChangeType();
	const wxFileName &f = event.GetPath();
	wxString fn = f.GetFullName();
	wxString fp = f.GetFullPath();
	wxString ext = f.GetExt();

	switch (type)
	{
		case wxFSW_EVENT_CREATE:
		case wxFSW_EVENT_DELETE:
		case wxFSW_EVENT_RENAME:
		case wxFSW_EVENT_MODIFY:
		{
			if (ext.IsSameAs("lua"))
			{
				fRelaunchedViaFileEvent = true;
				wxCommandEvent ev(eventRelaunchProject);
				wxPostEvent(wxGetApp().GetFrame(), ev);
			}

			break;
		}

		default:
			break;
	}
}

void SolarFrame::OnOpenWelcome(wxCommandEvent &event)
{
	string path(LinuxFileUtils::GetStartupPath(NULL));
	path.append("/Resources/homescreen/main.lua");

	wxCommandEvent eventOpen(eventOpenProject);
	eventOpen.SetString(path.c_str());
	wxPostEvent(this, eventOpen);
}

void SolarFrame::OnRelaunch(wxCommandEvent &event)
{
	if (fAppPath.size() > 0 && !IsHomeScreen(fContext->GetAppName()))
	{
		bool doRelaunch = !fRelaunchedViaFileEvent;

		if (fContext->GetPlatform()->GetRuntimeErrorDialog()->IsShown() || fRelaunchProjectDialog->IsShown())
		{
			return;
		}

		// workaround for wxFileSystem events firing twice (known wx bug)
		if (fFileSystemEventTimestamp >= wxGetUTCTimeMillis() - 250)
		{
			return;
		}

		if (fRelaunchedViaFileEvent)
		{
			switch (LinuxSimulatorView::Config::relaunchOnFileChange)
			{
				case LinuxPreferencesDialog::RelaunchType::Always:
					doRelaunch = true;
					break;

				case LinuxPreferencesDialog::RelaunchType::Ask:
					if (fRelaunchProjectDialog->ShowModal() == wxID_OK)
					{
						doRelaunch = true;
					}
					break;

				default:
					break;
			}

			fRelaunchedViaFileEvent = false;
		}

		if (!doRelaunch)
		{
			return;
		}

		fContext->GetRuntime()->End();
		delete fContext;
		fContext = new SolarAppContext(fAppPath.c_str());
		_chdir(fContext->GetAppPath());
		RemoveSuspendedPanel();

		if (LinuxSimulatorView::IsRunningOnSimulator())
		{
			WatchFolder(fContext->GetAppPath(), fContext->GetAppName().c_str());
			SetCursor(wxCURSOR_ARROW);
		}

		bool fullScreen = fContext->Init();

		if (LinuxSimulatorView::IsRunningOnSimulator())
		{
			LinuxSimulatorView::OnLinuxPluginGet(fContext->GetAppPath(), fContext->GetAppName().c_str(), fContext->GetPlatform());
		}

		fContext->LoadApp(fSolarGLCanvas);
		ResetSize();
		fSolarGLCanvas->fContext = fContext;
		fContext->SetCanvas(fSolarGLCanvas);

		SetTitle(fContext->GetTitle().c_str());
		SetMenu(fAppPath.c_str());
		fSolarGLCanvas->StartTimer(1000.0f / (float)fContext->GetFPS());
		fFileSystemEventTimestamp = wxGetUTCTimeMillis();
	}
}

void SolarFrame::CreateSuspendedPanel()
{
	if (LinuxSimulatorView::IsRunningOnSimulator())
	{
		if (suspendedPanel == NULL)
		{
			suspendedPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(fContext->GetWidth(), fContext->GetHeight()));
			suspendedPanel->SetBackgroundColour(wxColour(*wxBLACK));
			suspendedPanel->SetForegroundColour(wxColour(*wxBLACK));
			suspendedText = new wxStaticText(this, -1, "Suspended", wxDefaultPosition, wxDefaultSize);
			suspendedText->SetForegroundColour(*wxWHITE);
			suspendedText->CenterOnParent();
		}
	}
}

void SolarFrame::RemoveSuspendedPanel()
{
	if (LinuxSimulatorView::IsRunningOnSimulator())
	{
		if (suspendedPanel == NULL)
		{
			return;
		}

		suspendedPanel->Destroy();
		suspendedText->Destroy();
		suspendedPanel = NULL;
	}
}

void SolarFrame::OnSuspendOrResume(wxCommandEvent &event)
{
	if (LinuxSimulatorView::IsRunningOnSimulator())
	{
		if (fContext->GetRuntime()->IsSuspended())
		{
			RemoveSuspendedPanel();
			fHardwareMenu->SetLabel(ID_MENU_SUSPEND, "&Suspend	\tCtrl-Down");
			fContext->Resume();
		}
		else
		{
			CreateSuspendedPanel();
			fHardwareMenu->SetLabel(ID_MENU_SUSPEND, "&Resume	\tCtrl-Down");
			fContext->Pause();
		}
	}
}

void SolarFrame::OnOpen(wxCommandEvent &event)
{
	wxString path = event.GetString();
	path = path.SubString(0, path.size() - 10); // without main.lua

	delete fContext;
	fContext = new SolarAppContext(path.c_str());
	_chdir(fContext->GetAppPath());

	// clear the simulator log
	if (LinuxSimulatorView::IsRunningOnSimulator())
	{
		ConsoleApp::Clear();
	}

	string appName = fContext->GetAppName();
	RemoveSuspendedPanel();

	if (LinuxSimulatorView::IsRunningOnSimulator())
	{
		WatchFolder(fContext->GetAppPath(), appName.c_str());
		SetCursor(wxCURSOR_ARROW);
	}

	if (!IsHomeScreen(appName))
	{
		fAppPath = fContext->GetAppPath(); // save for relaunch
	}

	bool fullScreen = fContext->Init();

	if (!IsHomeScreen(appName))
	{
		if (LinuxSimulatorView::IsRunningOnSimulator())
		{
			LinuxSimulatorView::Config::lastProjectDirectory = fAppPath;
			LinuxSimulatorView::Config::Save();
			LinuxSimulatorView::OnLinuxPluginGet(fContext->GetAppPath(), appName.c_str(), fContext->GetPlatform());
		}
	}

	fContext->LoadApp(fSolarGLCanvas);
	ResetSize();
	fSolarGLCanvas->fContext = fContext;
	fContext->SetCanvas(fSolarGLCanvas);
	SetTitle(fContext->GetTitle().c_str());
	SetMenu(path.c_str());
	fSolarGLCanvas->StartTimer(1000.0f / (float)fContext->GetFPS());

	if (LinuxSimulatorView::IsRunningOnSimulator())
	{
		if (!IsHomeScreen(appName))
		{
			Rtt_Log("Loading project from: %s\n", fContext->GetAppPath());
			Rtt_Log("Project sandbox folder: %s%s\n", "~/.Solar2D/Sandbox/", fContext->GetTitle().c_str());
		}
	}
}

// setup glcanvas events
wxBEGIN_EVENT_TABLE(SolarGLCanvas, wxGLCanvas)
	EVT_PAINT(SolarGLCanvas::OnPaint)
	EVT_TIMER(TIMER_ID, SolarGLCanvas::OnTimer)
	EVT_WINDOW_CREATE(SolarGLCanvas::OnWindowCreate)
	EVT_SIZE(SolarGLCanvas::OnSize)
wxEND_EVENT_TABLE()

SolarGLCanvas::SolarGLCanvas(SolarFrame *parent, const wxGLAttributes &canvasAttrs)
	: wxGLCanvas(parent, canvasAttrs), fContext(NULL), fTimer(this, TIMER_ID)
{
	fSolarFrame = parent;
	fGLContext = new wxGLContext(this, NULL, 0);
	SetSize(parent->GetSize());

	if (!fGLContext->IsOK())
	{
		delete fGLContext;
		fGLContext = NULL;
	}

	Bind(wxEVT_CHAR, &LinuxKeyListener::OnChar);
	Bind(wxEVT_KEY_DOWN, &LinuxKeyListener::OnKeyDown);
	Bind(wxEVT_KEY_UP, &LinuxKeyListener::OnKeyUp);
	Bind(wxEVT_LEFT_DCLICK, &LinuxMouseListener::OnMouseLeftDoubleClick);
	Bind(wxEVT_LEFT_DOWN, &LinuxMouseListener::OnMouseLeftDown);
	Bind(wxEVT_LEFT_UP, &LinuxMouseListener::OnMouseLeftUp);
	Bind(wxEVT_RIGHT_DCLICK, &LinuxMouseListener::OnMouseRightDoubleClick);
	Bind(wxEVT_RIGHT_DOWN, &LinuxMouseListener::OnMouseRightDown);
	Bind(wxEVT_RIGHT_UP, &LinuxMouseListener::OnMouseRightUp);
	Bind(wxEVT_MIDDLE_DCLICK, &LinuxMouseListener::OnMouseMiddleDoubleClick);
	Bind(wxEVT_MIDDLE_DOWN, &LinuxMouseListener::OnMouseMiddleDown);
	Bind(wxEVT_MIDDLE_UP, &LinuxMouseListener::OnMouseMiddleUp);
	Bind(wxEVT_MOTION, &LinuxMouseListener::OnMouseMove);
	Bind(wxEVT_MOUSEWHEEL, &LinuxMouseListener::OnMouseWheel);
}

SolarGLCanvas::~SolarGLCanvas()
{
	if (fGLContext)
	{
		SetCurrent(*fGLContext);
	}

	if (fGLContext)
	{
		delete fGLContext;
		fGLContext = NULL;
	}
}

void SolarGLCanvas::StartTimer(float frameDuration)
{
	fTimer.Start((int)frameDuration);
}

void SolarGLCanvas::OnTimer(wxTimerEvent &event)
{
	if (!fContext->fIsStarted)
	{
		fContext->fIsStarted = true;
		fContext->GetRuntime()->BeginRunLoop();
	}

	Rtt::Runtime *runtime = fContext->GetRuntime();

	if (!runtime->IsSuspended())
	{
		LinuxInputDeviceManager &deviceManager = (LinuxInputDeviceManager &)fContext->GetPlatform()->GetDevice().GetInputDeviceManager();
		deviceManager.dispatchEvents(runtime);
		(*runtime)();
	}
}

void SolarGLCanvas::OnPaint(wxPaintEvent &WXUNUSED(event))
{
	if (fWindowHeight > 0)
	{
		SwapBuffers();
	}
}

void SolarGLCanvas::OnWindowCreate(wxWindowCreateEvent &event)
{
	// SetCurrent() must have an active window created before being called, making this hte perfect place to do it.
	Rtt_ASSERT(fGLContext);

	// the current context must be set before we get OGL pointers
	SetCurrent(*fGLContext);

	if (fContext && fContext->GetRuntime())
	{
		fContext->GetRuntime()->GetDisplay().Invalidate();
	}

	// generate paint event
	Refresh(false);
	Update();
}

void SolarGLCanvas::OnSize(wxSizeEvent &event)
{
	event.Skip();

	// if the window is not fully initialized, return
	if (!IsShownOnScreen())
	{
		return;
	}

	fWindowHeight = event.GetSize().y;

	if (fContext && fContext->GetRuntime())
	{
		fContext->GetRuntime()->GetDisplay().Invalidate();
	}

	// generate a paint event
	Refresh(false);
	Update();
}
