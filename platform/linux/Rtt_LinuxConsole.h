#ifndef LINUXCONSOLE_H
#define LINUXCONSOLE_H

#include <pwd.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <wx/wx.h>
#include <wx/image.h>
#include <wx/stc/stc.h>
#include <wx/icon.h>
#include "Rtt_LinuxIPCServer.h"
#include "Rtt_LinuxIPCServerConnection.h"

using namespace std;

#define TIMER_ID 1000

class Rtt_LinuxConsole: public wxFrame
{
public:
	Rtt_LinuxConsole(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = wxDEFAULT_FRAME_STYLE);

private:
	static const int intTimerInterval = 250;
	void SetProperties();
	void DoLayout();

protected:
	wxStatusBar *statusbar;
	wxBitmapButton *bitmapBtnSave;
	wxBitmapButton *bitmapBtnCopy;
	wxBitmapButton *bitmapBtnErase;
	wxTextCtrl *txtFind;
	wxBitmapButton *bitmapBtnFindPrevious;
	wxBitmapButton *bitmapBtnFindNext;
	wxBitmapButton *bitmapBtnMatchCase;
	wxBitmapButton *bitmapBtnLoopingSearch;
	wxBitmapButton *bitmapBtnMenu;
	wxPanel *panelToolBar;
	wxStyledTextCtrl *txtLog;
	wxTimer *timer;
	Rtt_LinuxIPCServer *linuxIPCServer;

	DECLARE_EVENT_TABLE();

public:
	void OnBtnSaveClick(wxCommandEvent &event);
	void OnBtnCopyClick(wxCommandEvent &event);
	void OnBtnEraseClick(wxCommandEvent &event);
	void OnBtnFindPreviousClick(wxCommandEvent &event);
	void OnBtnFindNextClick(wxCommandEvent &event);
	void OnBtnMatchCaseClick(wxCommandEvent &event);
	void OnBtnLoopingSearchClick(wxCommandEvent &event);
	void OnProgressTimer(wxTimerEvent &event);
	void ClearLog();
	void HighlightLine(int indicatorNo, wxColor colour);
	void UpdateLog(wxString message);
	void UpdateLogWarning(wxString message);
	void UpdateLogError(wxString message);
};

#endif // LINUXCONSOLE_H
