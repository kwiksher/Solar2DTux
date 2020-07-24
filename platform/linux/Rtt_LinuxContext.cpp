//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include <limits.h>
#include <unistd.h>
#include <dirent.h>
#define _chdir chdir
#include <pwd.h>
#include <sys/types.h>
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
#include "Rtt_KeyName.h"
#include "Rtt_Freetype.h"
#include "Rtt_LuaLibSimulator.h"
#include "Rtt_LinuxSimulatorView.h"
#include "Rtt_MPlatformServices.h"
#include "Rtt_LinuxPreferencesDialog.h"
#include "Rtt_LinuxCloneProjectDialog.h"
#include "Rtt_LinuxNewProjectDialog.h"
#include "wx/menu.h"
#include "wx/dcclient.h"
#include "wx/app.h"
#include "wx/display.h"

#if !defined(wxHAS_IMAGES_IN_RESOURCES) && defined(Rtt_SIMULATOR)
#include "resource/simulator.xpm"
#endif

//#define Rtt_DEBUG_TOUCH 1
#define TIMER_ID wxID_HIGHEST + 11
#define ID_MENU_WELCOME wxID_HIGHEST + 12
#define ID_MENU_BUILD_ANDROID wxID_HIGHEST + 13
#define ID_MENU_RELAUNCH wxID_HIGHEST + 14
#define ID_MENU_CLOSE wxID_HIGHEST + 15
#define ID_MENU_BUILD_WEB wxID_HIGHEST + 16
#define ID_MENU_BUILD_LINUX wxID_HIGHEST + 17
#define ID_MENU_OPEN_IN_EDITOR wxID_HIGHEST + 18
#define ID_MENU_SHOW_PROJECT_SANDBOX wxID_HIGHEST + 19
#define ID_MENU_CLEAR_PROJECT_SANDBOX wxID_HIGHEST + 20
#define ID_MENU_OPEN_SAMPLE_CODE wxID_HIGHEST + 21
#define ID_MENU_OPEN_DOCUMENTATION wxID_HIGHEST + 22
#define ID_MENU_SUSPEND wxID_HIGHEST + 23

using namespace Rtt;
using namespace std;

wxDEFINE_EVENT(eventOpenProject, wxCommandEvent);
wxDEFINE_EVENT(eventRelaunchProject, wxCommandEvent);
wxDEFINE_EVENT(eventWelcomeProject, wxCommandEvent);
wxDEFINE_EVENT(eventOpenPreferences, wxCommandEvent);
wxDEFINE_EVENT(eventCloneProject, wxCommandEvent);
wxDEFINE_EVENT(eventNewProject, wxCommandEvent);

static const char *getStartupPath(string *exeFileName)
{
	static char buf[PATH_MAX + 1];
	ssize_t count = readlink("/proc/self/exe", buf, PATH_MAX);
	buf[count] = 0;

	// remove file name
	char *filename = strrchr(buf, '/');
	Rtt_ASSERT(filename);

	if (exeFileName)
	{
		*exeFileName = filename + 1;
	}

	*filename = 0;

	return buf;
}

static const char *getHomePath()
{
	const char *homeDir = NULL;

	if ((homeDir = getenv("HOME")) == NULL)
	{
		homeDir = getpwuid(getuid())->pw_dir;
	}

	return homeDir;
}

static char *CalculateMD5(string filename)
{
	LinuxCrypto crypto;
	U8 digest[MCrypto::kMaxDigestSize];
	size_t digestLen = crypto.GetDigestLength(MCrypto::kMD5Algorithm);
	Rtt::Data<const char> data(filename.c_str(), (int)filename.length());
	crypto.CalculateDigest(MCrypto::kMD5Algorithm, data, digest);

	char *hex = (char *)calloc(sizeof(char), digestLen * 2 + 1);

	for (unsigned int i = 0; i < digestLen; i++)
	{
		char *p = hex;
		p += sprintf(hex + 2 * i, "%02x", digest[i]);
	}

	return hex;
}

static bool IsHomeScreen(string appName)
{
	return appName.compare(HOMESCREEN_ID) == 0;
}

namespace Rtt
{
	MouseListener::MouseListener(Runtime &runtime)
		: fRuntime(runtime), fScaleX(1), fScaleY(1)
	{
	}

	//  touch
	void MouseListener::TouchDown(int x, int y, int fid)
	{
		bool notifyMultitouch = fRuntime.Platform().GetDevice().DoesNotify(MPlatformDevice::kMultitouchEvent);
		x = (int)(x / fScaleX);
		y = (int)(y / fScaleY);

		// sanity check
		if (fStartPoint.find(fid) != fStartPoint.end() || (notifyMultitouch == false && fStartPoint.size() > 0))
		{
			return;
		}

		fStartPoint[fid] = pt(x, y);

		TouchEvent t((float)x, (float)y, (float)x, (float)y, TouchEvent::kBegan);

		// it must not be ZERO!
		t.SetId((void *)(fid + 1));

#if Rtt_DEBUG_TOUCH
		printf("TouchDown %d(%d, %d)\n", fid, x, y);
#endif

		if (notifyMultitouch)
		{
			MultitouchEvent t2(&t, 1);
			DispatchEvent(t2);
		}
		else
		{
			DispatchEvent(t);
		}
	}

	void MouseListener::TouchMoved(int x, int y, int fid)
	{
		bool notifyMultitouch = fRuntime.Platform().GetDevice().DoesNotify(MPlatformDevice::kMultitouchEvent);

		x = (int)(x / fScaleX);
		y = (int)(y / fScaleY);

#if Rtt_DEBUG_TOUCH
// Commented out b/c it's very noisy
//printf("TouchMoved %d(%d, %d)\n", id, x, y);
#endif

		// sanity check
		if (fStartPoint.find(fid) == fStartPoint.end())
		{
			return;
		}

		TouchEvent t((float)x, (float)y, (float)fStartPoint[fid].x, (float)fStartPoint[fid].y, TouchEvent::kMoved);

		// it must not be ZERO!
		t.SetId((void *)(fid + 1));

		if (notifyMultitouch)
		{
			MultitouchEvent t2(&t, 1);
			DispatchEvent(t2);
		}
		else
		{
			DispatchEvent(t);
		}
	}

	void MouseListener::TouchUp(int x, int y, int fid)
	{
		bool notifyMultitouch = fRuntime.Platform().GetDevice().DoesNotify(MPlatformDevice::kMultitouchEvent);

		x = (int)(x / fScaleX);
		y = (int)(y / fScaleY);

		// sanity check
		if (fStartPoint.find(fid) == fStartPoint.end())
		{
			return;
		}

		TouchEvent t((float)x, (float)y, (float)fStartPoint[fid].x, (float)fStartPoint[fid].y, TouchEvent::kEnded);

		// it must not be ZERO!
		t.SetId((void *)(fid + 1));

#if Rtt_DEBUG_TOUCH
		printf("TouchUp %d(%d, %d)\n", fid, x, y);
#endif

		if (notifyMultitouch)
		{
			MultitouchEvent t2(&t, 1);
			DispatchEvent(t2);
		}
		else
		{
			DispatchEvent(t);
		}

		// Dispatch a "tap" event if mouse button was released in the same position it was pressed in.
		if (fStartPoint[fid].x == x && fStartPoint[fid].y == y)
		{
			Rtt::TapEvent event(x, y, fid + 1);
			DispatchEvent(event);
		}

		fStartPoint.erase(fid);
	}

	void MouseListener::DispatchEvent(const MEvent &e) const
	{
		fRuntime.DispatchEvent(e);
	}

	KeyListener::KeyListener(Runtime &runtime)
		: fRuntime(runtime)
	{
		fKeyName['A'] = KeyName::kA;
		fKeyName['B'] = KeyName::kB;
		fKeyName['C'] = KeyName::kC;
		fKeyName['D'] = KeyName::kD;
		fKeyName['E'] = KeyName::kE;
		fKeyName['F'] = KeyName::kF;
		fKeyName['G'] = KeyName::kG;
		fKeyName['H'] = KeyName::kH;
		fKeyName['I'] = KeyName::kI;
		fKeyName['J'] = KeyName::kJ;
		fKeyName['K'] = KeyName::kK;
		fKeyName['L'] = KeyName::kL;
		fKeyName['M'] = KeyName::kM;
		fKeyName['N'] = KeyName::kN;
		fKeyName['O'] = KeyName::kO;
		fKeyName['P'] = KeyName::kP;
		fKeyName['Q'] = KeyName::kQ;
		fKeyName['R'] = KeyName::kR;
		fKeyName['S'] = KeyName::kS;
		fKeyName['T'] = KeyName::kT;
		fKeyName['U'] = KeyName::kU;
		fKeyName['V'] = KeyName::kV;
		fKeyName['W'] = KeyName::kW;
		fKeyName['X'] = KeyName::kX;
		fKeyName['Y'] = KeyName::kY;
		fKeyName['Z'] = KeyName::kZ;
		fKeyName['0'] = KeyName::k0;
		fKeyName['1'] = KeyName::k1;
		fKeyName['2'] = KeyName::k2;
		fKeyName['3'] = KeyName::k3;
		fKeyName['4'] = KeyName::k4;
		fKeyName['5'] = KeyName::k5;
		fKeyName['6'] = KeyName::k6;
		fKeyName['7'] = KeyName::k7;
		fKeyName['8'] = KeyName::k8;
		fKeyName['9'] = KeyName::k9;
		fKeyName[340] = KeyName::kF1;
		fKeyName[341] = KeyName::kF2;
		fKeyName[342] = KeyName::kF3;
		fKeyName[343] = KeyName::kF4;
		fKeyName[344] = KeyName::kF5;
		fKeyName[345] = KeyName::kF6;
		fKeyName[346] = KeyName::kF7;
		fKeyName[347] = KeyName::kF8;
		fKeyName[348] = KeyName::kF9;
		fKeyName[349] = KeyName::kF10;
		fKeyName[350] = KeyName::kF11;
		fKeyName[351] = KeyName::kF12;
		fKeyName[32] = KeyName::kSpace;
		fKeyName[307] = KeyName::kLeftAlt;
		//		fKeyName["Right Alt"] = KeyName::kRightAlt;
		fKeyName[308] = KeyName::kLeftControl;
		//		fKeyName["Right Ctrl"] = KeyName::kRightControl;
		fKeyName[306] = KeyName::kLeftShift;
		//		fKeyName["Right Shift"] = KeyName::kRightShift;
		fKeyName[393] = KeyName::kLeftCommand;
		//		fKeyName["Right Windows"] = KeyName::kRightCommand;
		fKeyName[315] = KeyName::kUp;
		fKeyName[317] = KeyName::kDown;
		fKeyName[314] = KeyName::kLeft;
		fKeyName[316] = KeyName::kRight;
		fKeyName[313] = KeyName::kHome;
		fKeyName[312] = KeyName::kEnd;
		fKeyName[366] = KeyName::kPageUp;
		fKeyName[367] = KeyName::kPageDown;
		fKeyName[322] = KeyName::kInsert;
		fKeyName[127] = KeyName::kDeleteForward;
		fKeyName[8] = KeyName::kDeleteBack; //kBack;
		fKeyName[47] = KeyName::kForwardSlash;
		fKeyName[92] = KeyName::kBackSlash;
		fKeyName['='] = KeyName::kPlus;
		fKeyName['-'] = KeyName::kMinus;
		fKeyName[','] = ",";
		fKeyName['.'] = KeyName::kPeriod;
		fKeyName['`'] = "`";
		fKeyName[';'] = ";";
		fKeyName['['] = KeyName::kLeftBracket;
		fKeyName[']'] = KeyName::kRightBracket;
		fKeyName['\''] = KeyName::kApostrophe;
		fKeyName[27] = KeyName::kEscape;
		fKeyName[13] = KeyName::kEnter;
		fKeyName[321] = KeyName::kPrintScreen;
		fKeyName[310] = KeyName::kMediaPause;
		fKeyName[365] = KeyName::kScrollLock;
		fKeyName[384] = KeyName::kNumPad0;
		fKeyName[382] = KeyName::kNumPad1;
		fKeyName[379] = KeyName::kNumPad2;
		fKeyName[381] = KeyName::kNumPad3;
		fKeyName[376] = KeyName::kNumPad4;
		fKeyName[383] = KeyName::kNumPad5;
		fKeyName[378] = KeyName::kNumPad6;
		fKeyName[375] = KeyName::kNumPad7;
		fKeyName[377] = KeyName::kNumPad8;
		fKeyName[380] = KeyName::kNumPad9;
		fKeyName[392] = KeyName::kNumPadDivide;
		fKeyName[387] = KeyName::kNumPadMultiply;
		fKeyName[390] = KeyName::kNumPadSubtract;
		fKeyName[388] = KeyName::kNumPadAdd;
		fKeyName[370] = KeyName::kNumPadEnter;
		fKeyName[385] = KeyName::kNumPadComma;
		//		fKeyName["Keypad ."] = KeyName::kNumPadPeriod;
		//		fKeyName["Keypad ("] = KeyName::kNumPadLeftParentheses;
		//		fKeyName["Keypad )"] = KeyName::kNumPadRightParentheses;
		fKeyName[364] = KeyName::kNumLock;
		fKeyName[9] = KeyName::kTab;
		fKeyName[426] = KeyName::kVolumeUp;
		fKeyName[425] = KeyName::kVolumeDown;
		fKeyName[424] = KeyName::kVolumeMute;
		//		fKeyName["Left GUI"] = KeyName::kLeftCommand;		// web
		//		fKeyName["Right GUI"] = KeyName::kRightCommand;		// web
		fKeyName[311] = KeyName::kCapsLock;
		//		fKeyName["ZoomIn"] = KeyName::kZoomIn;
		//		fKeyName["ZoomOut"] = KeyName::kZoomOut;
		//		fKeyName["Break"] = KeyName::kBreak;
		fKeyName[309] = KeyName::kMenu;
		//		fKeyName["Application"] = KeyName::kMenu;		// web
	}

	void KeyListener::notifyCharEvent(wxKeyEvent &e)
	{
		wxChar unicodeCharacter = e.GetUnicodeKey();

		if (unicodeCharacter != WXK_NONE)
		{
			wxCharBuffer utf8Buffer = wxString(e.GetUnicodeKey()).ToUTF8();
			const char *utf8Character = utf8Buffer.data();

			if (strlen(utf8Character) > 1 || isprint(utf8Character[0]))
			{
				CharacterEvent characterEvent(NULL, utf8Character);
				fRuntime.DispatchEvent(characterEvent);
			}
		}
	}

	void KeyListener::notifyKeyEvent(wxKeyEvent &e, bool down)
	{
		int mod = e.GetModifiers();
		bool isNumLockDown = false; // fixme
		bool isCapsLockDown = false; // fixme
		bool isShiftDown = mod & wxMOD_SHIFT ? true : false;
		bool isCtrlDown = mod & wxMOD_CONTROL ? true : false;
		bool isAltDown = mod & wxMOD_ALT ? true : false;
		bool isCommandDown = mod & wxMOD_WIN ? true : false;
		int keycode = e.GetKeyCode();
		PlatformInputDevice *dev = NULL;
		auto it = fKeyName.find(keycode);
		const char *keyName = it == fKeyName.end() ? KeyName::kUnknown : it->second.c_str();

		KeyEvent ke(dev, down ? KeyEvent::kDown : KeyEvent::kUp, keyName, keycode, isShiftDown, isAltDown, isCtrlDown, isCommandDown);
		fRuntime.DispatchEvent(ke);
	}

	CoronaAppContext::CoronaAppContext(const char *path)
		: fRuntime(NULL), fRuntimeDelegate(new LinuxRuntimeDelegate()), fMouseListener(NULL), fKeyListener(NULL), fPlatform(NULL), fTouchDeviceExist(false), fMode("normal"), fIsDebApp(false), fSimulator(NULL), fIsStarted(false)
	{
		string exeFileName;
		const char *homeDir = getHomePath();
		const char *appPath = getStartupPath(&exeFileName);

		// override appPath if arg isn't NULL
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

		// look for welcomescereen, Simulator ?
		startDir = getStartupPath(NULL);
		startDir.append("/Resources/homescreen");
		assetsDir = startDir;
		assetsDir.append("/main.lua");
		//Rtt_LogException("Checking %s\n", assetsDir.c_str());

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

	CoronaAppContext::~CoronaAppContext()
	{
		close();
	}

	void CoronaAppContext::close()
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

	bool CoronaAppContext::Init()
	{
		const char *homeDir = getHomePath();
		string appDir(homeDir);

#ifdef Rtt_SIMULATOR
		appDir.append("/.Solar2D/Sandbox/");
#else
		appDir.append("/.local/share/");
#endif

		if (!IsHomeScreen(fAppName))
		{
			appDir.append(fAppName);
#ifdef Rtt_SIMULATOR
			appDir.append("_");
			appDir.append(CalculateMD5(fAppName));
#endif
		}
		else
		{
#ifdef Rtt_SIMULATOR
			appDir.append("Simulator");
#endif
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
		documentsDir.append("/Documents/");
		temporaryDir.append("/TemporaryFiles/");
		cachesDir.append("/CachedFiles/");
		systemCachesDir.append("/.system/");

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
		fPlatform = new LinuxPlatform(fPathToApp.c_str(), documentsDir.c_str(), temporaryDir.c_str(), cachesDir.c_str(), systemCachesDir.c_str(), skinDir.c_str(), getStartupPath(NULL));
		fRuntime = new LinuxRuntime(*fPlatform, NULL);
		fRuntime->SetDelegate(fRuntimeDelegate);
		fRuntime->SetProperty(Runtime::kLinuxMaskSet, true);

		// For debugging, use main.lua if it exists in the app folder
		{
			string main_lua(fPathToApp);
			main_lua.append("/main.lua");
			FILE *fi = fopen(main_lua.c_str(), "r");

			if (fi)
			{
				fclose(fi);
#ifdef Rtt_SIMULATOR
				fRuntime->SetProperty(Runtime::kLinuxMaskSet | Runtime::kIsApplicationNotArchived | Runtime::kShouldVerifyLicense, true);
#else
				fRuntime->SetProperty(Runtime::kLinuxMaskSet, true);
#endif
			}
		}

		// settings
		string orientation;
		int w = 0;
		int h = 0;
		fRuntime->readSettings(&w, &h, &orientation, &fTitle, &fMode);

		if (orientation == "landscapeRight")
		{
			fRuntimeDelegate->fOrientation = DeviceOrientation::kSidewaysRight; // bottom of device is to the right

			if (w > 0 && h > 0)
			{
				fRuntimeDelegate->fContentWidth = w;
				fRuntimeDelegate->fContentHeight = h;
			}
			else
			{
				// no valid defaultViewWidth & defaultViewHeight in 'build.settings', default values of fWidth & fHeight for Portrait
				// use swapped default settings
				Swap(fRuntimeDelegate->fContentWidth, fRuntimeDelegate->fContentHeight);
			}
		}
		else if (orientation == "landscapeLeft")
		{
			fRuntimeDelegate->fOrientation = DeviceOrientation::kSidewaysLeft; // bottom of device is to the left

			if (w > 0 && h > 0)
			{
				fRuntimeDelegate->fContentWidth = w;
				fRuntimeDelegate->fContentHeight = h;
			}
			else
			{
				// no valid defaultViewWidth & defaultViewHeight in 'build.settings', default values of fWidth & fHeight for Portrait
				// use swapped default settings
				Swap(fRuntimeDelegate->fContentWidth, fRuntimeDelegate->fContentHeight);
			}
		}
		else if (orientation == "portrait")
		{
			fRuntimeDelegate->fOrientation = DeviceOrientation::kUpright; // bottom of device is at the bottom

			if (w > 0 && h > 0)
			{
				fRuntimeDelegate->fContentWidth = w;
				fRuntimeDelegate->fContentHeight = h;
			}
			else
			{
				// no valid defaultViewWidth & defaultViewHeight in 'build.settings', default values of fWidth & fHeight for Portrait
				// use default settings
			}
		}
		else if (orientation == "portraitUpsideDown")
		{
			fRuntimeDelegate->fOrientation = DeviceOrientation::kUpsideDown; // bottom of device is at the top

			if (w > 0 && h > 0)
			{
				fRuntimeDelegate->fContentWidth = w;
				fRuntimeDelegate->fContentHeight = h;
			}
			else
			{
				// no valid defaultViewWidth & defaultViewHeight in 'build.settings', default values of fWidth & fHeight for Portrait
				// use default settings
			}
		}
		else
		{
			//Rtt_LogException("Unsupported orientation: '%s'", orientation.c_str());
		}

		bool fullScreen = (fMode == "maximized");

		if (fullScreen)
		{
			wxDisplay screen;
			wxRect rect = screen.GetGeometry();
			//fWidth = rect.GetWidth();
			//fHeight = rect.GetHeight() - 1;
			//Rtt_LogException("Full Screen mode: %dx%d\n", fWidth, fHeight);
		}

		fPlatform->setWindow(this);

		fMouseListener = new MouseListener(*fRuntime);
		fKeyListener = new KeyListener(*fRuntime);

		// Initialize Joystick Support:
		LinuxInputDeviceManager &deviceManager = (LinuxInputDeviceManager &)fPlatform->GetDevice().GetInputDeviceManager();
		deviceManager.init();

		return fullScreen;
	}

	bool CoronaAppContext::loadApp(MyGLCanvas *canvas)
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
		luapath.append(getStartupPath(NULL));
		luapath.append("/Resources/?.lua;");

		setenv("LUA_PATH", luapath.c_str(), true);

		if (fRuntimeDelegate->fOrientation == DeviceOrientation::kSidewaysRight || fRuntimeDelegate->fOrientation == DeviceOrientation::kSidewaysLeft)
		{
			// Swap(fRuntimeDelegate->fContentWidth, fRuntimeDelegate->fContentHeight);
		}

#ifdef Rtt_SIMULATOR
		fSimulator = new LinuxSimulatorServices();
		lua_State *luaStatePointer = fRuntime->VMContext().L();
		lua_pushlightuserdata(luaStatePointer, fSimulator);
		Rtt::LuaContext::RegisterModuleLoader(luaStatePointer, Rtt::LuaLibSimulator::kName, Rtt::LuaLibSimulator::Open, 1);
#endif

		return true;
	}

	void CoronaAppContext::flush()
	{
		fCanvas->Refresh(false);
	}

	void CoronaAppContext::pause()
	{
		if (fRuntime->IsSuspended() == false)
		{
			fRuntime->Suspend();
		}
	}

	void CoronaAppContext::resume()
	{
		if (GetRuntime()->IsSuspended())
		{
			fRuntime->DispatchEvent(ResizeEvent());
			fRuntime->Resume();
		}
	}

	bool LinuxRuntime::readTable(lua_State *L, const char *table, int *w, int *h, string *title, string *mode) const
	{
		bool rc = false;
		int top = lua_gettop(L);

		lua_getfield(L, -1, table);

		if (lua_istable(L, -1))
		{
			rc = true;
			lua_getfield(L, -1, "defaultViewWidth");

			if ((!lua_isnil(L, -1)) && (lua_isnumber(L, -1)))
			{
				*w = lua_tointeger(L, -1);
			}

			lua_pop(L, 1);

			lua_getfield(L, -1, "defaultViewHeight");

			if ((!lua_isnil(L, -1)) && (lua_isnumber(L, -1)))
			{
				*h = lua_tointeger(L, -1);
			}

			lua_pop(L, 1);

			lua_getfield(L, -1, "defaultMode");

			if ((!lua_isnil(L, -1)) && (lua_isstring(L, -1)))
			{
				*mode = lua_tostring(L, -1);
			}

			lua_pop(L, 1);

			lua_getfield(L, -1, "titleText");

			if (lua_istable(L, -1))
			{
				lua_getfield(L, -1, "default");

				if ((!lua_isnil(L, -1)) && (lua_isstring(L, -1)))
				{
					*title = lua_tostring(L, -1);
				}

				lua_pop(L, 1); // remove default
			}

			lua_pop(L, 1); // remove titleText
		}

		lua_settop(L, top);
		return rc;
	}

	void LinuxRuntime::readSettings(int *w, int *h, string *orientation, string *title, string *mode)
	{
		Rtt_ASSERT(w != NULL && h != NULL);

		lua_State *L = VMContext().L();
		int status = 0;
		const MPlatform &p = Platform();
		int top = lua_gettop(L);

		// read build.settings
		const char kBuildSettings[] = "build.settings";
		String filePath(&p.GetAllocator());
		p.PathForFile(kBuildSettings, MPlatform::kResourceDir, MPlatform::kTestFileExists, filePath);
		const char *path = filePath.GetString();

		if (path)
		{
			status = VMContext().DoFile(path, 0, true);
		}
		else
		{
			// other cases assume a non-zero status means there's an error msg on the stack
			// so push a "fake" error msg on the stack so we are consistent with those cases
			lua_pushnil(L);
		}

		lua_pop(L, 1); // remove DoFile result

		if (status == 0)
		{
			lua_getglobal(L, "settings"); // settings

			if (lua_istable(L, -1))
			{
				lua_getfield(L, -1, "orientation"); // settings.orientation

				if (lua_istable(L, -1))
				{
					lua_getfield(L, -1, "default");

					if ((!lua_isnil(L, -1)) && (lua_isstring(L, -1)))
					{
						*orientation = lua_tostring(L, -1);
					}

					lua_pop(L, 1);
				}

				lua_pop(L, 1); // remove orientation

				// first try settings from 'web' table
				if (readTable(L, "linux", w, h, title, mode) == false)
				{
					// next try settings from 'window' table
					readTable(L, "window", w, h, title, mode);
				}
			}

			lua_pop(L, 1); // remove settings
		}
		else if (status == 3)
		{
			Rtt_LogException("Invalid build.settings file\n");
		}
		else
		{
			Rtt_LogException("Failed to read build.settings file\n");
		}

		lua_settop(L, top);
	}

	int jsSystemEvent::Push(lua_State *L) const
	{
		if (Rtt_VERIFY(VirtualEvent::Push(L)))
		{
			lua_pushstring(L, fEventName.c_str());
			lua_setfield(L, -2, kTypeKey);
		}

		return 1;
	}
} // namespace Rtt

//
// wxWidgets
//

MyApp::MyApp()
{
	// start the console immediately
#ifdef Rtt_SIMULATOR
	wxExecute("./Solar2DConsole");
#endif
}

MyApp::~MyApp()
{
}

// 'Main program' equivalent: the program execution "starts" here
bool MyApp::OnInit()
{
	if (wxApp::OnInit() == true)
	{
		// create the main application window
		fFrame = new MyFrame();
		bool fullScreen = false;

		// As of October 2015 GTK+ needs the frame to be shown before we call SetCurrent()

		//Test if OGL context could be created.
		if (fFrame->m_mycanvas->OglCtxAvailable() == true)
		{
			if (fullScreen)
			{
				fFrame->ShowFullScreen(true);
			}
			else
			{
				fFrame->Show(true);
			}

			wxInitAllImageHandlers();
			return true;
		}
	}

	return false;
}

wxWindow *MyApp::getParent()
{
	return getFrame();
}

LinuxPlatform *MyApp::getPlatform() const
{
	return fFrame->getContext()->getPlatform();
}

// main frame

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
	EVT_MENU(wxID_EXIT, MyFrame::OnQuit)
	EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)
	EVT_MENU(wxID_OPEN, MyFrame::OnOpenFileDialog)
	EVT_MENU(wxID_NEW, MyFrame::OnNewProject)
	EVT_MENU(wxID_PREFERENCES, MyFrame::OnOpenPreferences)
	EVT_MENU(ID_MENU_WELCOME, MyFrame::OnOpenWelcome)
	EVT_MENU(ID_MENU_BUILD_ANDROID, MyFrame::OnBuildAndroid)
	EVT_MENU(ID_MENU_BUILD_WEB, MyFrame::OnBuildWeb)
	EVT_MENU(ID_MENU_BUILD_LINUX, MyFrame::OnBuildLinux)
	EVT_MENU(ID_MENU_RELAUNCH, MyFrame::OnRelaunch)
	EVT_MENU(ID_MENU_SUSPEND, MyFrame::OnSuspendOrResume)
	EVT_MENU(ID_MENU_CLOSE, MyFrame::OnOpenWelcome)
	EVT_MENU(ID_MENU_OPEN_IN_EDITOR, MyFrame::OnOpenInEditor)
	EVT_MENU(ID_MENU_SHOW_PROJECT_SANDBOX, MyFrame::OnShowProjectSandbox)
	EVT_MENU(ID_MENU_CLEAR_PROJECT_SANDBOX, MyFrame::OnClearProjectSandbox)
	EVT_MENU(ID_MENU_OPEN_SAMPLE_CODE, MyFrame::OnOpenSampleProjects)
	EVT_MENU(ID_MENU_OPEN_DOCUMENTATION, MyFrame::OnOpenDocumentation)
	EVT_COMMAND(wxID_ANY, eventOpenProject, MyFrame::OnOpen)
	EVT_COMMAND(wxID_ANY, eventCloneProject, MyFrame::OnCloneProject)
	EVT_COMMAND(wxID_ANY, eventNewProject, MyFrame::OnNewProject)
	EVT_COMMAND(wxID_ANY, eventRelaunchProject, MyFrame::OnRelaunch)
	EVT_COMMAND(wxID_ANY, eventWelcomeProject, MyFrame::OnOpenWelcome)
#ifdef Rtt_SIMULATOR
	EVT_CLOSE(MyFrame::OnClose)
#endif
wxEND_EVENT_TABLE()

MyFrame::MyFrame()
	: wxFrame(NULL, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(320, 480), wxCAPTION | wxMINIMIZE_BOX | wxCLOSE_BOX), m_mycanvas(NULL), fContext(NULL), fMenuMain(NULL), fMenuProject(NULL), fWatcher(NULL),
	  fProjectPath("")
{
#ifdef Rtt_SIMULATOR
	SetIcon(simulator_xpm);
#endif
	wxGLAttributes vAttrs;
	vAttrs.PlatformDefaults().Defaults().EndList();
	suspendedPanel = NULL;
	bool accepted = wxGLCanvas::IsDisplaySupported(vAttrs);

	if (accepted == false)
	{
		// Try again without sample buffers
		vAttrs.Reset();
		vAttrs.PlatformDefaults().RGBA().DoubleBuffer().Depth(16).EndList();

		accepted = wxGLCanvas::IsDisplaySupported(vAttrs);

		if (accepted == false)
		{
			Rtt_LogException("Failed to init OpenGL");
			return;
		}
	}

	createMenus();
	m_mycanvas = new MyGLCanvas(this, vAttrs);

	SetWindowStyle(wxCAPTION | wxMINIMIZE_BOX | wxCLOSE_BOX);

	const char *homeDir = getHomePath();
	fProjectPath = string(homeDir);
	fProjectPath.append("/Documents/Solar2D Projects");

	if (!Rtt_IsDirectory(fProjectPath.c_str()))
	{
		Rtt_MakeDirectory(fProjectPath.c_str());
	}
}

MyFrame::~MyFrame()
{
	delete fWatcher;
	delete m_mycanvas;

	fContext->close();
	delete fContext;

	SetMenuBar(NULL);
	delete fMenuMain;
	delete fMenuProject;
}

void MyFrame::watchFolder(const char *path, const char *appName)
{
	// wxFileSystemWatcher
	if (fWatcher == NULL)
	{
		fWatcher = new wxFileSystemWatcher();
		fWatcher->SetOwner(this);
		Connect(wxEVT_FSWATCHER, wxFileSystemWatcherEventHandler(MyFrame::OnFileSystemEvent));
	}

	if (IsHomeScreen(string(appName)))
	{
		// do not watch main screen folder
		return;
	}

	wxFileName fn = wxFileName::DirName(path);
	fn.DontFollowLink();

	fWatcher->RemoveAll();
	fWatcher->Add(fn);
}

void MyFrame::resetSize()
{
	this->SetClientSize(wxSize(fContext->getWidth(), fContext->getHeight()));
	this->Refresh(false);
	this->Update();
}

void MyFrame::createMenus()
{
	wxMenuItem *mi;
	{
		fMenuMain = new wxMenuBar();

		// File Menu
		wxMenu *m_pFileMenu = new wxMenu();
		mi = m_pFileMenu->Append(wxID_NEW, _T("&New Project	\tCtrl-N")); //mi->Enable(false);
		mi = m_pFileMenu->Append(wxID_OPEN, _T("&Open Project	\tCtrl-O"));
		m_pFileMenu->AppendSeparator();
		mi = m_pFileMenu->Append(wxID_SAVE, _T("&Relaunch Last Project	\tCtrl-R"));
		m_pFileMenu->AppendSeparator();
		mi = m_pFileMenu->Append(wxID_PREFERENCES, _T("&Preferences..."));
		m_pFileMenu->AppendSeparator();
		mi = m_pFileMenu->Append(wxID_EXIT, _T("&Exit"));
		fMenuMain->Append(m_pFileMenu, _T("&File"));

		// About menu
		wxMenu *m_pHelpMenu = new wxMenu();
		mi = m_pHelpMenu->Append(ID_MENU_OPEN_DOCUMENTATION, _T("&Online Documentation..."));
		mi = m_pHelpMenu->Append(ID_MENU_OPEN_SAMPLE_CODE, _T("&Sample projects..."));
		mi = m_pHelpMenu->Append(wxID_ABOUT, _T("&About Simulator..."));
		fMenuMain->Append(m_pHelpMenu, _T("&Help"));
	}

	// project's menu
	{
		fMenuProject = new wxMenuBar();

		// File Menu
		wxMenu *m_pFileMenu = new wxMenu();
		mi = m_pFileMenu->Append(wxID_NEW, _T("&New Project	\tCtrl-N")); //mi->Enable(false);
		mi = m_pFileMenu->Append(wxID_OPEN, _T("&Open Project	\tCtrl-O"));
		m_pFileMenu->AppendSeparator();

		wxMenu *m_pBuildMenu = new wxMenu();
		mi = m_pBuildMenu->Append(ID_MENU_BUILD_ANDROID, _T("Android	\tCtrl-B"));
		mi = m_pBuildMenu->Append(ID_MENU_BUILD_WEB, _T("HTML5	\tCtrl-Shift-Alt-B"));
		wxMenu *m_pBuildForLinuxMenu = new wxMenu();
		mi = m_pBuildForLinuxMenu->Append(ID_MENU_BUILD_LINUX, _T("x64	\tCtrl-Alt-B"));
		mi = m_pBuildForLinuxMenu->Append(ID_MENU_BUILD_LINUX, _T("ARM	\tCtrl-Alt-A"));
		mi->Enable(false);
		m_pBuildMenu->AppendSubMenu(m_pBuildForLinuxMenu, _T("&Linux"));
		mi = m_pFileMenu->AppendSubMenu(m_pBuildMenu, _T("&Build"));

		mi = m_pFileMenu->Append(ID_MENU_OPEN_IN_EDITOR, _T("&Open In Editor	\tCtrl-Shift-O"));
		mi = m_pFileMenu->Append(wxID_SAVE, _T("&Show Project Files"));
		mi = m_pFileMenu->Append(ID_MENU_SHOW_PROJECT_SANDBOX, _T("&Show Project Sandbox"));
		m_pFileMenu->AppendSeparator();
		mi = m_pFileMenu->Append(ID_MENU_CLEAR_PROJECT_SANDBOX, _T("&Clear Project Sandbox"));
		m_pFileMenu->AppendSeparator();
		mi = m_pFileMenu->Append(ID_MENU_RELAUNCH, _T("Relaunch	\tCtrl-R"));
		mi = m_pFileMenu->Append(ID_MENU_CLOSE, _T("Close Project	\tCtrl-W"));
		m_pFileMenu->AppendSeparator();
		mi = m_pFileMenu->Append(wxID_PREFERENCES, _T("&Preferences..."));
		m_pFileMenu->AppendSeparator();
		mi = m_pFileMenu->Append(wxID_EXIT, _T("&Exit"));
		fMenuProject->Append(m_pFileMenu, _T("&File"));

		// hardware menu
		m_pHardwareMenu = new wxMenu();
		mi = m_pHardwareMenu->Append(wxID_HELP_CONTENTS, _T("&Rotate Left"));
		mi->Enable(false);
		mi = m_pHardwareMenu->Append(wxID_HELP_INDEX, _T("&Rotate Right"));
		mi->Enable(false);
		//mi = m_pHardwareMenu->Append(wxID_ABOUT, _T("&Shake"));
		m_pHardwareMenu->AppendSeparator();
		mi = m_pHardwareMenu->Append(wxID_ABOUT, _T("&Back"));
		mi->Enable(false);
		m_pHardwareMenu->AppendSeparator();
		mi = m_pHardwareMenu->Append(ID_MENU_SUSPEND, _T("&Suspend	\tCtrl-Down"));
		fMenuProject->Append(m_pHardwareMenu, _T("&Hardware"));

		// View menu
		wxMenu *m_pViewMenu = new wxMenu();
		mi = m_pViewMenu->Append(wxID_HELP_CONTENTS, _T("&Zoom In"));
		mi->Enable(false);
		mi = m_pViewMenu->Append(wxID_HELP_INDEX, _T("&Zoom Out"));
		mi->Enable(false);
		m_pViewMenu->AppendSeparator();
		mi = m_pViewMenu->Append(wxID_ABOUT, _T("&View As"));
		mi->Enable(false);

		m_pViewMenu->AppendSeparator();
		mi = m_pViewMenu->Append(ID_MENU_WELCOME, _T("&Welcome Screen"));
		mi = m_pViewMenu->Append(wxID_ABOUT, _T("&Console"));
		mi->Enable(false);
		fMenuProject->Append(m_pViewMenu, _T("&View"));

		// About menu
		wxMenu *m_pHelpMenu = new wxMenu();
		mi = m_pHelpMenu->Append(ID_MENU_OPEN_DOCUMENTATION, _T("&Online Documentation..."));
		mi = m_pHelpMenu->Append(ID_MENU_OPEN_SAMPLE_CODE, _T("&Sample projects..."));
		mi = m_pHelpMenu->Append(wxID_ABOUT, _T("&About Simulator...")); //mi->Enable(false);
		fMenuProject->Append(m_pHelpMenu, _T("&Help"));
	}
}

void MyFrame::setMenu(const char *appPath)
{
#ifdef Rtt_SIMULATOR
	const string &appName = getContext()->getAppName();
	SetMenuBar(IsHomeScreen(appName) ? fMenuMain : fMenuProject);
#endif
}

// event handlers
void MyFrame::OnQuit(wxCommandEvent &WXUNUSED(event))
{
	// quit the simulator console
#ifdef Rtt_SIMULATOR
	LinuxConsoleLog(LINUX_CONSOLE_QUIT_CMD);
#endif

	Close(true);
}

void MyFrame::OnClose(wxCloseEvent &ev)
{
	// quit the simulator console
#ifdef Rtt_SIMULATOR
	fContext->GetRuntime()->End();
	LinuxConsoleLog(LINUX_CONSOLE_QUIT_CMD);
	wxExit();
#endif
}

void MyFrame::OnAbout(wxCommandEvent &WXUNUSED(event))
{
	wxAboutDialogInfo info;
	wxIcon icon = wxIcon("/opt/Solar2D/Resources/logo_small.png", wxBITMAP_TYPE_PNG, 60, 60);
	string version("Version: ");
	version.append(to_string(Rtt_BUILD_YEAR)).append(".").append(to_string(Rtt_LOCAL_BUILD_REVISION));
	info.SetName("Solar2DTux");
	info.SetVersion(version);
	info.SetCopyright(Rtt_STRING_COPYRIGHT);
	info.AddDeveloper("Danny Glover, Robert Craig. Based on initial port by the CoronaLabs team.");
	info.SetWebSite("https://github.com/DannyGlover/Solar2DTux");
	info.SetIcon(icon);
	info.SetLicence(_("THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND,\nEXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES\nOF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND\nNONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS\nBE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN\nACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN\nCONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\nSOFTWARE."));
	info.SetDescription(_("Solar2D is a cross-platform framework ideal for rapidly creating apps and games for mobile devices, TV, desktop systems and HTML5.\n\nThat means you can create your project once and publish it to multiple types of devices, including Apple iPhone and iPad, Android phones and tablets, Amazon Fire, Mac Desktop, Windows Desktop, Linux, HTML5 and even connected TVs such as Apple TV, Fire TV, and Android TV."));
	::wxAboutBox(info, this);
}

void MyFrame::OnFileSystemEvent(wxFileSystemWatcherEvent &event)
{
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
			if (ext == "lua")
			{
				// relaunch
				wxCommandEvent ev(eventRelaunchProject);
				wxPostEvent(wxGetApp().getFrame(), ev);
			}

			break;
		}

		default:
			break;
	}
}

// open home screen
void MyFrame::OnOpenWelcome(wxCommandEvent &ev)
{
	string path(getStartupPath(NULL));
	path.append("/Resources/homescreen/main.lua");

	wxCommandEvent eventOpen(eventOpenProject);
	eventOpen.SetString(path.c_str());
	wxPostEvent(this, eventOpen);
}

void MyFrame::OnBuildAndroid(wxCommandEvent &ev)
{
	LinuxSimulatorView::OnBuildForAndroid(getContext());
}

void MyFrame::OnBuildWeb(wxCommandEvent &ev)
{
	LinuxSimulatorView::OnBuildForWeb(getContext());
}

void MyFrame::OnBuildLinux(wxCommandEvent &ev)
{
	LinuxSimulatorView::OnBuildForLinux(getContext());
}

void MyFrame::OnOpenFileDialog(wxCommandEvent &event)
{
	wxFileDialog openFileDialog(wxGetApp().getParent(), _("Open"), fProjectPath, wxEmptyString, "Simulator Files (main.lua)|main.lua", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if (openFileDialog.ShowModal() == wxID_CANCEL)
	{
		return;
	}

	wxString path = openFileDialog.GetPath();

	if (Rtt_FileExists(path.c_str()) == false)
	{
		return;
	}

	// open project
	wxCommandEvent eventOpen(eventOpenProject);
	eventOpen.SetString(path.c_str());
	wxPostEvent(this, eventOpen);
}

void MyFrame::OnOpenInEditor(wxCommandEvent &ev)
{
	string command("xdg-open ");
	command.append(fContext->getAppPath());
	command.append("/main.lua");

	system(command.c_str());
}

void MyFrame::OnShowProjectSandbox(wxCommandEvent &ev)
{
	const char *homeDir = getHomePath();
	string command("xdg-open ");
	command.append(homeDir);
	command.append("/.Solar2D/Sandbox/");
	command.append(fContext->getAppName());
	command.append("_");
	command.append(CalculateMD5(fContext->getAppName()));

	system(command.c_str());
}

void MyFrame::OnClearProjectSandbox(wxCommandEvent &ev)
{
	const char *homeDir = getHomePath();
	string command("rm -rf ");
	command.append(homeDir);
	command.append("/.Solar2D/Sandbox/");
	command.append(fContext->getAppName());
	command.append("_");
	command.append(CalculateMD5(fContext->getAppName()));

	system(command.c_str());
	// relaunch
	wxCommandEvent relaunchEvent(eventRelaunchProject);
	wxPostEvent(wxGetApp().getFrame(), relaunchEvent);
}

void MyFrame::OnOpenSampleProjects(wxCommandEvent &ev)
{
	wxFileDialog openFileDialog(wxGetApp().getParent(), _("Open"), "/opt/Solar2D/SampleCode", wxEmptyString, "Simulator Files (main.lua)|main.lua", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if (openFileDialog.ShowModal() == wxID_CANCEL)
	{
		return;
	}

	wxString path = openFileDialog.GetPath();

	if (Rtt_FileExists(path.c_str()) == false)
	{
		return;
	}

	// open project
	wxCommandEvent eventOpen(eventOpenProject);
	eventOpen.SetString(path.c_str());
	wxPostEvent(this, eventOpen);
}

void MyFrame::OnOpenDocumentation(wxCommandEvent &ev)
{
	string command("xdg-open https://docs.coronalabs.com/api/index.html");
	system(command.c_str());
}

void MyFrame::OnRelaunch(wxCommandEvent &event)
{
	if (fAppPath.size() > 0 && !IsHomeScreen(fContext->getAppName()))
	{
		if (fContext->getPlatform()->GetRuntimeErrorDialog()->IsShown())
		{
			return;
		}

		fContext->GetRuntime()->End();
		delete fContext;
		fContext = new CoronaAppContext(fAppPath.c_str());
		_chdir(fContext->getAppPath());

		RemoveSuspendedPanel();
#ifdef Rtt_SIMULATOR
		watchFolder(fContext->getAppPath(), fContext->getAppName().c_str());
		SetCursor(wxCURSOR_ARROW);
#endif

		bool fullScreen = fContext->Init();

#ifdef Rtt_SIMULATOR
		LinuxSimulatorView::OnLinuxPluginGet(fContext->getAppPath(), fContext->getAppName().c_str(), fContext->getPlatform());
#endif
		fContext->loadApp(m_mycanvas);
		resetSize();
		m_mycanvas->fContext = fContext;
		fContext->setCanvas(m_mycanvas);

		SetTitle(fContext->getTitle().c_str());
		setMenu(fAppPath.c_str());
		m_mycanvas->startTimer(1000.0f / (float)fContext->getFPS());
	}
}

void MyFrame::CreateSuspendedPanel()
{
#ifdef Rtt_SIMULATOR
	if (suspendedPanel == NULL)
	{
		suspendedPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(fContext->getWidth(), fContext->getHeight()));
		suspendedPanel->SetBackgroundColour(wxColour(*wxBLACK));
		suspendedPanel->SetForegroundColour(wxColour(*wxBLACK));
		suspendedText = new wxStaticText(this, -1, "Suspended", wxDefaultPosition, wxDefaultSize);
		suspendedText->SetForegroundColour(*wxWHITE);
		suspendedText->CenterOnParent();
	}
#endif
}

void MyFrame::RemoveSuspendedPanel()
{
#ifdef Rtt_SIMULATOR
	if (suspendedPanel == NULL)
	{
		return;
	}

	suspendedPanel->Destroy();
	suspendedText->Destroy();
	suspendedPanel = NULL;
#endif
}

void MyFrame::OnSuspendOrResume(wxCommandEvent &event)
{
#ifdef Rtt_SIMULATOR
	if (fContext->GetRuntime()->IsSuspended())
	{
		RemoveSuspendedPanel();
		m_pHardwareMenu->SetLabel(ID_MENU_SUSPEND, "&Suspend	\tCtrl-Down");
		fContext->resume();
	}
	else
	{
		CreateSuspendedPanel();
		m_pHardwareMenu->SetLabel(ID_MENU_SUSPEND, "&Resume	\tCtrl-Down");
		fContext->pause();
	}
#endif
}

void MyFrame::OnOpenPreferences(wxCommandEvent &event)
{
	NewPreferencesDialog *newPreferencesDialog = new NewPreferencesDialog(this, wxID_ANY, wxEmptyString);

	if (newPreferencesDialog->ShowModal() == wxID_OK)
	{
	}

	newPreferencesDialog->Destroy();
}

void MyFrame::OnCloneProject(wxCommandEvent &event)
{
	NewCloneDialog *newCloneDlg = new NewCloneDialog(this, wxID_ANY, wxEmptyString);

	if (newCloneDlg->ShowModal() == wxID_OK)
	{
	}

	newCloneDlg->Destroy();
}

void MyFrame::OnNewProject(wxCommandEvent &event)
{
	NewProjectDialog *newProjectDlg = new NewProjectDialog(this, wxID_ANY, wxEmptyString);

	if (newProjectDlg->ShowModal() == wxID_OK)
	{
	}

	newProjectDlg->Destroy();
}

void MyFrame::OnOpen(wxCommandEvent &event)
{
	wxString path = event.GetString();
	path = path.SubString(0, path.size() - 10); // without main.lua

	delete fContext;
	fContext = new CoronaAppContext(path.c_str());
	_chdir(fContext->getAppPath());

	// clear the simulator log
#ifdef Rtt_SIMULATOR
	LinuxConsoleLog(LINUX_CONSOLE_CLEAR_CMD);
#endif

	string appName = fContext->getAppName();
	RemoveSuspendedPanel();

#ifdef Rtt_SIMULATOR
	watchFolder(fContext->getAppPath(), appName.c_str());
	SetCursor(wxCURSOR_ARROW);
#endif

	if (!IsHomeScreen(appName))
	{
		fAppPath = fContext->getAppPath(); // save for relaunch
	}

	bool fullScreen = fContext->Init();

	if (!IsHomeScreen(appName))
	{
#ifdef Rtt_SIMULATOR
		LinuxSimulatorView::OnLinuxPluginGet(fContext->getAppPath(), appName.c_str(), fContext->getPlatform());
#endif
	}

	fContext->loadApp(m_mycanvas);
	resetSize();
	m_mycanvas->fContext = fContext;
	fContext->setCanvas(m_mycanvas);
	SetTitle(fContext->getTitle().c_str());
	setMenu(path.c_str());
	m_mycanvas->startTimer(1000.0f / (float)fContext->getFPS());

#ifdef Rtt_SIMULATOR
	if (!IsHomeScreen(appName))
	{
		Rtt_Log("Loading project from: %s\n", fContext->getAppPath());
		Rtt_Log("Project sandbox folder: %s%s\n", "~/.Solar2D/Sandbox/", fContext->getTitle().c_str());
	}
#endif
}

// ----------------------------------------------------------------------------
// The canvas inside the frame. Our OpenGL connection
// ----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(MyGLCanvas, wxGLCanvas)
	EVT_PAINT(MyGLCanvas::OnPaint)
	EVT_SIZE(MyGLCanvas::OnSize)
	EVT_MOUSE_EVENTS(MyGLCanvas::OnMouse)
	EVT_CHAR(MyGLCanvas::OnChar)
	EVT_KEY_UP(MyGLCanvas::OnKeyUp)
	EVT_KEY_DOWN(MyGLCanvas::OnKeyDown)
	EVT_TIMER(TIMER_ID, MyGLCanvas::OnTimer)
wxEND_EVENT_TABLE()

//We create a wxGLContext in this constructor, do OGL initialization at OnSize().
MyGLCanvas::MyGLCanvas(MyFrame *parent, const wxGLAttributes &canvasAttrs)
	: wxGLCanvas(parent, canvasAttrs), fContext(NULL), m_timer(this, TIMER_ID)
{
	m_parent = parent;
	m_oglContext = new wxGLContext(this, NULL, 0);

	if (!m_oglContext->IsOK())
	{
		delete m_oglContext;
		m_oglContext = NULL;
	}
}

MyGLCanvas::~MyGLCanvas()
{
	if (m_oglContext)
	{
		SetCurrent(*m_oglContext);
	}

	if (m_oglContext)
	{
		delete m_oglContext;
		m_oglContext = NULL;
	}
}

void MyGLCanvas::startTimer(float frameDuration)
{
	m_timer.Start((int)frameDuration);
}

void MyGLCanvas::OnTimer(wxTimerEvent &event)
{
	if (fContext->fIsStarted == false)
	{
		fContext->fIsStarted = true;
		fContext->GetRuntime()->BeginRunLoop();
	}

	Rtt::Runtime *runtime = fContext->GetRuntime();

	if (runtime->IsSuspended() == false)
	{
		LinuxInputDeviceManager &deviceManager = (LinuxInputDeviceManager &)fContext->getPlatform()->GetDevice().GetInputDeviceManager();
		deviceManager.dispatchEvents(runtime);
		(*runtime)();
	}
}

void MyGLCanvas::OnKeyDown(wxKeyEvent &event)
{
	event.Skip();

	if (event.GetKeyCode() == WXK_ESCAPE)
	{
		m_parent->Close(); // close main window
	}
	else
	{
		fContext->GetKeyListener()->notifyKeyEvent(event, true);
	}
}

void MyGLCanvas::OnKeyUp(wxKeyEvent &event)
{
	event.Skip();

	if (fContext && event.GetKeyCode() != WXK_ESCAPE)
	{
		fContext->GetKeyListener()->notifyKeyEvent(event, false);
	}
}

void MyGLCanvas::OnChar(wxKeyEvent &event)
{
	event.Skip();
	fContext->GetKeyListener()->notifyCharEvent(event);
}

void MyGLCanvas::OnPaint(wxPaintEvent &WXUNUSED(event))
{
	if (m_winHeight > 0)
	{
		if (fContext && fContext->GetRuntime())
		{
			m_parent->getContext()->GetRuntime()->GetDisplay().Invalidate();
		}

		SwapBuffers();
	}
}

//Note:
// You may wonder why OpenGL initialization was not done at wxGLCanvas ctor.
// The reason is due to GTK+/X11 working asynchronously, we can't call
// SetCurrent() before the window is shown on screen (GTK+ doc's say that the
// window must be realized first).
// In wxGTK, window creation and sizing requires several size-events. At least
// one of them happens after GTK+ has notified the realization. We use this
// circumstance and do initialization then.

void MyGLCanvas::OnSize(wxSizeEvent &event)
{
	// If this window is not fully initialized, dismiss this event
	if (!IsShownOnScreen())
	{
		return;
	}

	static bool isInited = false;

	//Now we have a context, retrieve pointers to OGL functions
	if (isInited == false)
	{
		isInited = true;
		Rtt_ASSERT(m_oglContext);

		// The current context must be set before we get OGL pointers
		SetCurrent(*m_oglContext);

		// open home screen
		wxCommandEvent eventOpen(eventOpenProject);
		m_parent->OnOpen(eventOpen);
	}

	// This is normally only necessary if there is more than one wxGLCanvas
	// or more than one wxGLContext in the application.
	SetCurrent(*m_oglContext);

	// It's up to the application code to update the OpenGL viewport settings.
	m_winHeight = event.GetSize().y;
	// m_oglManager->SetViewport(0, 0, event.GetSize().x, m_winHeight);

	if (fContext && fContext->GetRuntime())
	{
		fContext->GetRuntime()->GetDisplay().Invalidate();
	}

	// Generate paint event
	Refresh(false);
	Update();

	//event.Skip();
}

void MyGLCanvas::OnMouse(wxMouseEvent &e)
{
	int x = e.GetX();
	int y = e.GetY(); // Y-coordinate is at bottom of the window

	float scrollWheelDeltaX = 0;
	float scrollWheelDeltaY = 0;

	// Fetch the current state of the "shift", "alt", and "ctrl" keys.
	const Uint8 *key = 0;		//SDL_GetKeyboardState(NULL);
	bool IsAltDown = false;		//key[SDL_SCANCODE_LALT] | key[SDL_SCANCODE_RALT];
	bool IsShiftDown = false;	//key[SDL_SCANCODE_LSHIFT] | key[SDL_SCANCODE_RSHIFT];
	bool IsControlDown = false; //key[SDL_SCANCODE_LCTRL] | key[SDL_SCANCODE_RCTRL];
	bool IsCommandDown = false; //key[SDL_SCANCODE_LGUI] | key[SDL_SCANCODE_RGUI];

	// Fetch the mouse's current up/down buttons states.
	bool isPrimaryDown = e.LeftIsDown();
	bool isSecondaryDown = e.RightIsDown();
	bool isMiddleDown = e.MiddleIsDown();

	Rtt::Runtime *runtime = fContext->GetRuntime();

	if (e.ButtonDown())
	{
		Rtt::MouseEvent::MouseEventType eventType = Rtt::MouseEvent::kDown;
		Rtt::MouseEvent mouseEvent(eventType, x, y, Rtt_FloatToReal(scrollWheelDeltaX), Rtt_FloatToReal(scrollWheelDeltaY), 0, isPrimaryDown, isSecondaryDown, isMiddleDown, IsShiftDown, IsAltDown, IsControlDown, IsCommandDown);

		runtime->DispatchEvent(mouseEvent);
		fContext->GetMouseListener()->TouchDown(x, y, 0);
	}
	else
	{
		if (e.ButtonUp())
		{
			Rtt::MouseEvent::MouseEventType eventType = Rtt::MouseEvent::kUp;
			Rtt::MouseEvent mouseEvent(eventType, x, y, Rtt_FloatToReal(scrollWheelDeltaX), Rtt_FloatToReal(scrollWheelDeltaY), 0, isPrimaryDown, isSecondaryDown, isMiddleDown, IsShiftDown, IsAltDown, IsControlDown, IsCommandDown);

			runtime->DispatchEvent(mouseEvent);
			fContext->GetMouseListener()->TouchUp(x, y, 0);
		}
		else
		{
			Rtt::MouseEvent::MouseEventType eventType = Rtt::MouseEvent::kMove;

			// Determine if this is a "drag" event.
			if (isPrimaryDown || isSecondaryDown || isMiddleDown)
			{
				eventType = Rtt::MouseEvent::kDrag;
			}

			Rtt::MouseEvent mouseEvent(eventType, x, y, Rtt_FloatToReal(scrollWheelDeltaX), Rtt_FloatToReal(scrollWheelDeltaY), 0,
			                           isPrimaryDown, isSecondaryDown, isMiddleDown, IsShiftDown, IsAltDown, IsControlDown, IsCommandDown);

			runtime->DispatchEvent(mouseEvent);
			fContext->GetMouseListener()->TouchMoved(x, y, 0);
		}

		// mousewheel events
		if (e.GetWheelRotation() > 0 || e.GetWheelRotation() < 0)
		{
			if (e.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL)
			{
				scrollWheelDeltaY = e.GetWheelRotation() * -1;
			}
			else
			{
				scrollWheelDeltaX = e.GetWheelRotation() * -1;
			}

			Rtt::MouseEvent mouseEvent(Rtt::MouseEvent::kScroll, x, y, Rtt_FloatToReal(scrollWheelDeltaX), Rtt_FloatToReal(scrollWheelDeltaY), 0,
			                           isPrimaryDown, isSecondaryDown, isMiddleDown, IsShiftDown, IsAltDown, IsControlDown, IsCommandDown);
			runtime->DispatchEvent(mouseEvent);
		}
	}
}
