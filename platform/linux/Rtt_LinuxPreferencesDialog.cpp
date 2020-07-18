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
#include <pwd.h>
#include <libgen.h>
#include <string.h>
#include "Rtt_LinuxPreferencesDialog.h"
#include <fstream>
#include <streambuf>

using namespace std;
using namespace Rtt;

namespace Rtt
{
	NewPreferencesDialog::NewPreferencesDialog(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos, const wxSize &size, long style) : wxDialog(parent, id, title, pos, size, wxDEFAULT_DIALOG_STYLE)
	{
		SetSize(wxSize(520, 320));
		showRuntimeErrors = new wxCheckBox(this, -1, "Show Runtime Errors", wxPoint(0, 0));
		automaticallyLaunchLastProject = new wxCheckBox(this, -1, "Automatically open last project", wxPoint(0, 0));
		relaunchSimulatorOptionText = new wxStaticText(this, wxID_ANY, wxT("Relaunch Simulator when project is modified?"));
		relaunchOnModifyAlways = new wxRadioButton(this, wxID_ANY, "Always", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
		relaunchOnModifyNever = new wxRadioButton(this, wxID_ANY, "Never", wxDefaultPosition, wxDefaultSize);
		relaunchOnModifyAskEveryTime = new wxRadioButton(this, wxID_ANY, "Ask Every Time", wxDefaultPosition, wxDefaultSize);
		btnOK = new wxButton(this, wxID_OK, wxT("OK"));
		btnCancel = new wxButton(this, wxID_CANCEL, wxT("Cancel"));

		SetProperties();
		SetLayout();
	}

	void NewPreferencesDialog::SetProperties()
	{
		SetTitle(wxT("Preferences"));
		SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
		// todo: read from settings
		showRuntimeErrors->SetValue(true);
		automaticallyLaunchLastProject->SetValue(true);
		relaunchOnModifyAlways->SetValue(true);
		relaunchOnModifyNever->SetValue(false);
		relaunchOnModifyAskEveryTime->SetValue(false);
		btnOK->SetDefault();
	}

	void NewPreferencesDialog::SetLayout()
	{
		wxBoxSizer *dialogLayout = new wxBoxSizer(wxVERTICAL);
		wxBoxSizer *dialogTop = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer *dialogMiddle = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer *dialogBottom = new wxBoxSizer(wxVERTICAL);
		wxBoxSizer *dialogButtons = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer *boxSizerTopColumn1 = new wxBoxSizer(wxVERTICAL);
		wxBoxSizer *boxSizerTopColumn2 = new wxBoxSizer(wxVERTICAL);
		wxBoxSizer *boxSizerTopColumn3 = new wxBoxSizer(wxVERTICAL);
		wxStaticLine *staticLineSeparator = new wxStaticLine(this, wxID_ANY);

		// set fonts

		// add to box sizers
		boxSizerTopColumn1->Add(showRuntimeErrors, 0, wxALIGN_LEFT | wxBOTTOM | wxTOP, 6);
		boxSizerTopColumn1->Add(automaticallyLaunchLastProject, 0, wxALIGN_LEFT | wxBOTTOM | wxTOP, 6);
		boxSizerTopColumn1->Add(relaunchSimulatorOptionText, 0, wxALIGN_LEFT | wxBOTTOM | wxTOP, 6);
		boxSizerTopColumn1->Add(relaunchOnModifyAlways, 0, wxALIGN_LEFT | wxBOTTOM | wxTOP, 6);
		boxSizerTopColumn1->Add(relaunchOnModifyNever, 0, wxALIGN_LEFT | wxBOTTOM | wxTOP, 6);
		boxSizerTopColumn1->Add(relaunchOnModifyAskEveryTime, 0, wxALIGN_LEFT | wxBOTTOM | wxTOP, 6);

		// add to dialog buttons
		dialogButtons->Add(btnOK, 0, wxRIGHT, 5);
		dialogButtons->Add(btnCancel, 0, 0, 0);

		// add to dialog layouts
		dialogTop->Add(boxSizerTopColumn1, 0, wxLEFT | wxRIGHT, 7);
		dialogTop->Add(boxSizerTopColumn2, 1, wxLEFT | wxRIGHT, 0);
		dialogTop->Add(boxSizerTopColumn3, 0, wxEXPAND | wxLEFT | wxRIGHT, 7);
		dialogBottom->Add(staticLineSeparator, 0, wxEXPAND | wxLEFT | wxRIGHT, 7);
		dialogBottom->Add(dialogButtons, 1, wxALIGN_CENTER_HORIZONTAL | wxALL, 7);
		dialogLayout->Add(dialogTop, 0, wxBOTTOM | wxEXPAND | wxTOP, 8);
		dialogLayout->Add(dialogBottom, 0, wxEXPAND, 0);

		SetSizer(dialogLayout);
		Layout();
	}

	BEGIN_EVENT_TABLE(NewPreferencesDialog, wxDialog)
		EVT_BUTTON(wxID_OK, NewPreferencesDialog::OnOKClicked)
		EVT_BUTTON(wxID_CANCEL, NewPreferencesDialog::OnCancelClicked)
	END_EVENT_TABLE();

	void NewPreferencesDialog::OnOKClicked(wxCommandEvent &event)
	{
		// todo: save settings
		EndModal(wxID_OK);
	}

	void NewPreferencesDialog::OnCancelClicked(wxCommandEvent &event)
	{
		EndModal(wxID_CLOSE);
	}
} // namespace Rtt
