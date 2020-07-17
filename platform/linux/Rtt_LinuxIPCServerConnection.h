#ifndef LINUXIPCSERVERCONNECTION_H
#define LINUXIPCSERVERCONNECTION_H

#include <wx/wx.h>
#include <string.h>
#include "Rtt_LinuxConsoleApp.h"
#include "Rtt_LinuxIPCConnectionBase.h"

class Rtt_LinuxIPCServerConnection: public Rtt_LinuxIPCConnectionBase, public wxTimer
{
public:

	virtual bool Disconnect() wxOVERRIDE { return wxConnection::Disconnect(); }
	virtual bool OnExecute(const wxString &topic, const void *data, size_t size, wxIPCFormat format) wxOVERRIDE;
	virtual const void *OnRequest(const wxString &topic, const wxString &item, size_t *size, wxIPCFormat format) wxOVERRIDE;
	virtual bool OnPoke(const wxString &topic, const wxString &item, const void *data, size_t size, wxIPCFormat format) wxOVERRIDE;
	virtual bool OnStartAdvise(const wxString &topic, const wxString &item) wxOVERRIDE;
	virtual bool OnStopAdvise(const wxString &topic, const wxString &item) wxOVERRIDE;
	virtual bool DoAdvise(const wxString &item, const void *data, size_t size, wxIPCFormat format) wxOVERRIDE;
	virtual bool OnDisconnect() wxOVERRIDE;
	virtual void Notify() wxOVERRIDE;

private:
	wxString m_sAdvise;
	wxString m_sRequestDate;
	char m_achRequestBytes[3];
};
/*
enum
{
    ID_START = 10000,
    ID_DISCONNECT,
    ID_ADVISE,
    ID_SERVERNAME,
};
*/



#endif //LINUXIPCSERVERCONNECTION_H
