#include "Rtt_LinuxConsole.h"
#include <sys/time.h>

#ifndef wxHAS_IMAGES_IN_RESOURCES
#include "resource/console.xpm"
#include "resource/save.xpm"
#include "resource/copy.xpm"
#include "resource/erase.xpm"
#include "resource/search-left.xpm"
#include "resource/search-right.xpm"
#include "resource/match-case.xpm"
#include "resource/looping-search.xpm"
#include "resource/cog.xpm"
#endif

#define ID_BUTTON_SAVE wxID_HIGHEST + 1
#define ID_BUTTON_COPY wxID_HIGHEST + 2
#define ID_BUTTON_CLEAR wxID_HIGHEST + 3
#define ID_BUTTON_FIND_PREVIOUS wxID_HIGHEST + 4
#define ID_BUTTON_FIND_NEXT wxID_HIGHEST + 5
#define ID_BUTTON_MATCH_CASE wxID_HIGHEST + 6
#define ID_BUTTON_LOOP_SEARCH wxID_HIGHEST + 7
#define ID_BUTTON_SETTINGS wxID_HIGHEST + 8
#define WX_INDICATOR_WARNING 8
#define WX_INDICATOR_ERROR 9
#define WX_INDICATOR_WARNING_TEXT 15
#define WX_INDICATOR_ERROR_TEXT 16

static int logCurrentPos = 0;
static wxColour warningColor = wxColour(255, 255, 0);
static wxColour errorColor = wxColour(192, 0, 0);

using namespace std;

Rtt_LinuxConsole::Rtt_LinuxConsole(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos, const wxSize &size, long style):
	wxFrame(parent, id, title, pos, size, wxDEFAULT_FRAME_STYLE)
{
	SetIcon(console_xpm);
	SetSize(wxSize(640, 480));
	panelToolBar = new wxPanel(this, wxID_ANY);
	statusbar = CreateStatusBar(1);
	bitmapBtnSave = new wxBitmapButton(panelToolBar, ID_BUTTON_SAVE, wxIcon(save_xpm), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxBU_AUTODRAW | wxBU_EXACTFIT | wxBU_NOTEXT);
	bitmapBtnCopy = new wxBitmapButton(panelToolBar, ID_BUTTON_COPY, wxIcon(copy_xpm), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxBU_AUTODRAW | wxBU_EXACTFIT | wxBU_NOTEXT);
	bitmapBtnErase = new wxBitmapButton(panelToolBar, ID_BUTTON_CLEAR, wxIcon(erase_xpm), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxBU_AUTODRAW | wxBU_EXACTFIT | wxBU_NOTEXT);
	txtFind = new wxTextCtrl(panelToolBar, wxID_ANY, wxEmptyString);
	bitmapBtnFindPrevious = new wxBitmapButton(panelToolBar, ID_BUTTON_FIND_PREVIOUS, wxIcon(search_left_xpm), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxBU_AUTODRAW | wxBU_EXACTFIT | wxBU_NOTEXT);
	bitmapBtnFindNext = new wxBitmapButton(panelToolBar, ID_BUTTON_FIND_NEXT, wxIcon(search_right_xpm), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxBU_AUTODRAW | wxBU_EXACTFIT | wxBU_NOTEXT);
	bitmapBtnMatchCase = new wxBitmapButton(panelToolBar, ID_BUTTON_MATCH_CASE, wxIcon(match_case_xpm), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxBU_AUTODRAW | wxBU_EXACTFIT | wxBU_NOTEXT);
	bitmapBtnLoopingSearch = new wxBitmapButton(panelToolBar, ID_BUTTON_LOOP_SEARCH, wxIcon(looping_search_xpm), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxBU_AUTODRAW | wxBU_EXACTFIT | wxBU_NOTEXT);
	bitmapBtnMenu = new wxBitmapButton(panelToolBar, ID_BUTTON_SETTINGS, wxIcon(cog_xpm), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxBU_AUTODRAW | wxBU_EXACTFIT | wxBU_NOTEXT);
	txtLog = new wxStyledTextCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxTE_MULTILINE | wxTE_RICH2 | wxTE_READONLY);
	linuxIPCServer = new Rtt_LinuxIPCServer();
	SetProperties();
	DoLayout();
}

void Rtt_LinuxConsole::SetProperties()
{
	SetTitle(wxT("Solar2DTux Console"));
	int statusbar_widths[] = { -1 };
	statusbar->SetStatusWidths(1, statusbar_widths);

	// statusbar fields
	const wxString statusbar_fields[] =
	{
		wxT("Errors #0 Alerts #0"),
	};

	for (int i = 0; i < statusbar->GetFieldsCount(); ++i)
	{
		statusbar->SetStatusText(statusbar_fields[i], i);
	}

	wxColor backgroundColour = wxColour(128, 128, 128);
	bitmapBtnSave->SetBackgroundColour(backgroundColour);
	bitmapBtnSave->SetSize(bitmapBtnSave->GetBestSize());
	bitmapBtnCopy->SetBackgroundColour(backgroundColour);
	bitmapBtnCopy->SetSize(bitmapBtnCopy->GetBestSize());
	bitmapBtnErase->SetBackgroundColour(backgroundColour);
	bitmapBtnErase->SetSize(bitmapBtnErase->GetBestSize());
	txtFind->SetMinSize(wxSize(250, -1));
	txtFind->SetBackgroundColour(backgroundColour);
	txtFind->SetForegroundColour(*wxWHITE);
	bitmapBtnFindPrevious->SetBackgroundColour(backgroundColour);
	bitmapBtnFindPrevious->SetSize(bitmapBtnFindPrevious->GetBestSize());
	bitmapBtnFindNext->SetBackgroundColour(backgroundColour);
	bitmapBtnFindNext->SetSize(bitmapBtnFindNext->GetBestSize());
	bitmapBtnMatchCase->SetBackgroundColour(backgroundColour);
	bitmapBtnMatchCase->SetSize(bitmapBtnMatchCase->GetBestSize());
	bitmapBtnLoopingSearch->SetBackgroundColour(backgroundColour);
	bitmapBtnLoopingSearch->SetSize(bitmapBtnLoopingSearch->GetBestSize());
	bitmapBtnMenu->SetBackgroundColour(backgroundColour);
	bitmapBtnMenu->SetSize(bitmapBtnMenu->GetBestSize());
	panelToolBar->SetBackgroundColour(backgroundColour);
	statusbar->SetBackgroundColour(backgroundColour);
	statusbar->SetForegroundColour(*wxWHITE);
	txtLog->SetFocus();
	txtLog->SetReadOnly(true);
	txtLog->SetWrapMode(1);
	txtLog->StyleSetBackground(wxSTC_STYLE_DEFAULT, *wxBLACK);
	txtLog->StyleSetForeground(wxSTC_STYLE_DEFAULT, wxColour(192, 192, 192));
	txtLog->StyleClearAll();
	txtLog->SetFocus();

	// Create a new server
	if (!linuxIPCServer->Create(IPC_SERVICE))
	{
		//wxLogMessage("%s server failed to start on %s", kind, IPC_SERVICE);
	}
	else
	{
		//wxLogMessage("%s server started on %s", kind, IPC_SERVICE);
	}
}

void Rtt_LinuxConsole::DoLayout()
{
	wxBoxSizer *sizer1 = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *sizer2 = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText* lblFind = new wxStaticText(panelToolBar, wxID_ANY, wxT("Find:"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
	sizer2->Add(bitmapBtnSave, 0, 0, 0);
	sizer2->Add(bitmapBtnCopy, 0, 0, 0);
	sizer2->Add(bitmapBtnErase, 0, 0, 0);
	lblFind->SetForegroundColour(*wxWHITE);
	sizer2->Add(lblFind, 0, wxALIGN_CENTER_VERTICAL, 0);
	sizer2->Add(txtFind, 0, wxALIGN_CENTER_VERTICAL, 0);
	sizer2->Add(bitmapBtnFindPrevious, 0, 0, 0);
	sizer2->Add(bitmapBtnFindNext, 0, 0, 0);
	sizer2->Add(bitmapBtnMatchCase, 0, 0, 0);
	sizer2->Add(bitmapBtnLoopingSearch, 0, 0, 0);
	sizer2->Add(bitmapBtnMenu, 0, 0, 0);
	panelToolBar->SetSizer(sizer2);
	sizer1->Add(panelToolBar, 0, wxEXPAND, 0);
	sizer1->Add(txtLog, 1, wxEXPAND, 0);
	SetSizer(sizer1);
	Layout();
}

BEGIN_EVENT_TABLE(Rtt_LinuxConsole, wxFrame)
	EVT_BUTTON(ID_BUTTON_SAVE, Rtt_LinuxConsole::OnBtnSaveClick)
	EVT_BUTTON(ID_BUTTON_COPY, Rtt_LinuxConsole::OnBtnCopyClick)
	EVT_BUTTON(ID_BUTTON_CLEAR, Rtt_LinuxConsole::OnBtnEraseClick)
	EVT_BUTTON(ID_BUTTON_FIND_PREVIOUS, Rtt_LinuxConsole::OnBtnFindPreviousClick)
	EVT_BUTTON(ID_BUTTON_FIND_NEXT, Rtt_LinuxConsole::OnBtnFindNextClick)
	EVT_BUTTON(ID_BUTTON_MATCH_CASE, Rtt_LinuxConsole::OnBtnMatchCaseClick)
	EVT_BUTTON(ID_BUTTON_LOOP_SEARCH, Rtt_LinuxConsole::OnBtnLoopingSearchClick)
END_EVENT_TABLE();

void Rtt_LinuxConsole::OnBtnSaveClick(wxCommandEvent &event)
{
	struct passwd *pw = getpwuid(getuid());
	const char *homedir = pw->pw_dir;
	time_t timeNow = time(NULL);
	char buffer[32];
	char msBuffer[32];
	struct tm* timeInfo;
	struct timeval timeValue;
	gettimeofday(&timeValue, NULL);

	timeInfo = localtime(&timeValue.tv_sec);
	strftime(buffer, 26, "%m-%d-%y-%H.%M.%S", timeInfo);
	string filePath(homedir);
	filePath.append("/.Solar2D/").append("log_").append(buffer).append(".txt");
	bool saved = txtLog->SaveFile(filePath.c_str());

	if (saved)
	{
		string resultMsg("Saved log to: ");
		resultMsg.append(filePath.c_str());

		wxMessageDialog *msgDialog = new wxMessageDialog(this, resultMsg.c_str(), wxT("Save Log"), wxOK);
		msgDialog->ShowModal();
	}
}

void Rtt_LinuxConsole::OnBtnCopyClick(wxCommandEvent &event)
{
	txtLog->SetFocus();
	txtLog->Copy();
}

void Rtt_LinuxConsole::OnBtnEraseClick(wxCommandEvent &event)
{
	ClearLog();
}

void Rtt_LinuxConsole::OnBtnFindPreviousClick(wxCommandEvent &event)
{
	if (!txtFind->IsEmpty())
	{
		wxString searchText = txtFind->GetLineText(0);
		txtLog->SetFocus();
		txtLog->SearchAnchor();
		logCurrentPos = txtLog->SearchPrev(0, searchText);

		if (logCurrentPos > 0)
		{
			txtLog->EnsureCaretVisible();
			txtLog->SetCurrentPos(txtLog->GetAnchor() - searchText.Length());
		}
		else
		{
			txtLog->SetCurrentPos(txtLog->GetValue().Length());
			txtLog->SetSelection(txtLog->GetValue().Length(), txtLog->GetValue().Length());
			txtLog->SearchAnchor();
		}
	}
}

void Rtt_LinuxConsole::OnBtnFindNextClick(wxCommandEvent &event)
{
	if (!txtFind->IsEmpty())
	{
		wxString searchText = txtFind->GetLineText(0);
		txtLog->SetFocus();

		if (logCurrentPos > 0)
		{
			txtLog->SetCurrentPos(txtLog->GetAnchor() + searchText.Length() + 1);
		}

		txtLog->SearchAnchor();
		logCurrentPos = txtLog->SearchNext(0, searchText);

		if (logCurrentPos > 0)
		{
			txtLog->EnsureCaretVisible();
		}
		else
		{
			txtLog->SetCurrentPos(0);
			txtLog->SetSelection(0, 0);
		}
	}
}

void Rtt_LinuxConsole::OnBtnMatchCaseClick(wxCommandEvent &event)
{
	event.Skip();
	txtLog->SetFocus();
	// notify the user that he hasn't implemented the event handler yet
	wxLogDebug(wxT("Event handler (Rtt_LinuxConsole::onBtnMatchCaseClick) not implemented yet"));
}

void Rtt_LinuxConsole::OnBtnLoopingSearchClick(wxCommandEvent &event)
{
	event.Skip();
	txtLog->SetFocus();
	// notify the user that he hasn't implemented the event handler yet
	wxLogDebug(wxT("Event handler (Rtt_LinuxConsole::onBtnLoopingSearchClick) not implemented yet"));
}

void Rtt_LinuxConsole::ClearLog()
{
	txtLog->SetFocus();
	txtLog->SetReadOnly(false);
	txtLog->ClearAll();
	txtLog->SetReadOnly(true);
}

void Rtt_LinuxConsole::HighlightLine(int indicatorNo, wxColor colour)
{
	txtLog->IndicatorSetAlpha(indicatorNo, 255);
	txtLog->IndicatorSetUnder(indicatorNo, true);
	txtLog->IndicatorSetStyle(indicatorNo, wxSTC_INDIC_FULLBOX);
	txtLog->IndicatorSetForeground(indicatorNo, colour);

	bool shouldChangeText = (indicatorNo == WX_INDICATOR_WARNING || indicatorNo == WX_INDICATOR_ERROR);
	int textTargetID = 0;
	wxColour textTargetColor;
	int lineNo = txtLog->GetCurrentLine();
	int startPosition = txtLog->PositionFromLine(lineNo);
	int endPosition = txtLog->GetLineEndPosition(lineNo);
	int length = (endPosition - startPosition);
	txtLog->SetIndicatorCurrent(indicatorNo);
	txtLog->IndicatorFillRange(startPosition, length);

	switch (indicatorNo)
	{
		case WX_INDICATOR_WARNING:
			textTargetID = WX_INDICATOR_WARNING_TEXT;
			textTargetColor = *wxBLACK;
			break;

		case WX_INDICATOR_ERROR:
			textTargetID = WX_INDICATOR_ERROR_TEXT;
			textTargetColor = *wxWHITE;
			break;
	}

	if (shouldChangeText)
	{
		txtLog->IndicatorSetStyle(textTargetID, wxSTC_INDIC_TEXTFORE);
		txtLog->IndicatorSetForeground(textTargetID, textTargetColor);
		txtLog->SetIndicatorCurrent(textTargetID);
		txtLog->IndicatorFillRange(startPosition, length);
	}
}

void Rtt_LinuxConsole::UpdateLog(wxString message)
{
	txtLog->SetReadOnly(false);
	txtLog->SetInsertionPointEnd();
	txtLog->AppendText(message);
	txtLog->SelectNone();
	txtLog->SetReadOnly(true);
	txtLog->ScrollToEnd();
}

void Rtt_LinuxConsole::UpdateLogWarning(wxString message)
{
	txtLog->SetReadOnly(false);
	txtLog->SetInsertionPointEnd();
	txtLog->AppendText(message);
	HighlightLine(8, warningColor);
	txtLog->SelectNone();
	txtLog->SetReadOnly(true);
	txtLog->ScrollToEnd();
}

void Rtt_LinuxConsole::UpdateLogError(wxString message)
{
	txtLog->SetReadOnly(false);
	txtLog->SetInsertionPointEnd();
	txtLog->AppendText(message);
	HighlightLine(9, errorColor);
	txtLog->SelectNone();
	txtLog->SetReadOnly(true);
	txtLog->ScrollToEnd();
}
