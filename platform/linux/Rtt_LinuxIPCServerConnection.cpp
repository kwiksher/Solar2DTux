#include "Rtt_LinuxIPCServerConnection.h"

bool Rtt_LinuxIPCServerConnection::OnExecute(const wxString &topic, const void *data, size_t size, wxIPCFormat format)
{
	string logMsg(static_cast<const char *>(data), size);
	wxString msg;
	msg.append(logMsg.c_str());
	wxGetApp().UpdateLog(msg.c_str(), 0);
	return true;
}

bool Rtt_LinuxIPCServerConnection::OnPoke(const wxString &topic, const wxString &item, const void *data, size_t size, wxIPCFormat format)
{
	int logType = 0;
	string input(static_cast<const char *>(data), size);
	wxString msg;
	msg.append(input.c_str());

	if (item.IsSameAs("warning"))
	{
		logType = 1;
	}
	else if (item.IsSameAs("error"))
	{
		logType = 2;
	}
	else if (item.IsSameAs("clear"))
	{
		wxGetApp().ClearLog();
		return true;
	}

	wxGetApp().UpdateLog(msg.c_str(), logType);
	return wxConnection::OnPoke(topic, item, data, size, format);
}

const void *Rtt_LinuxIPCServerConnection::OnRequest(const wxString &topic, const wxString &item, size_t *size, wxIPCFormat format)
{
	wxGetApp().UpdateLog("This is the request function..... \n", 2);
}

bool Rtt_LinuxIPCServerConnection::OnStartAdvise(const wxString &topic, const wxString &item)
{
	//wxLogMessage("OnStartAdvise(\"%s\",\"%s\")", topic, item);
	//wxLogMessage("Returning true");
	m_sAdvise = item;
	Start(3000); // schedule our Notify() to be called in 3 seconds
	return true;
}

bool Rtt_LinuxIPCServerConnection::OnStopAdvise(const wxString &topic, const wxString &item)
{
	//wxLogMessage("OnStopAdvise(\"%s\",\"%s\")", topic, item);
	//wxLogMessage("Returning true");
	m_sAdvise.clear();
	Stop();
	return true;
}

void Rtt_LinuxIPCServerConnection::Notify()
{
	if (!m_sAdvise.empty())
	{
		wxString s = wxDateTime::Now().Format();
		Advise(m_sAdvise, s);
		s = wxDateTime::Now().FormatTime() + " "
		    + wxDateTime::Now().FormatDate();

		Advise(m_sAdvise, s.mb_str(), s.length() + 1);

#if wxUSE_DDE_FOR_IPC
		wxLogMessage("DDE Advise type argument cannot be wxIPC_PRIVATE. "
		             "The client will receive it as wxIPC_TEXT, "
		             "and receive the correct no of bytes, "
		             "but not print a correct log entry.");
#endif // DDE

		char bytes[3];
		bytes[0] = '1';
		bytes[1] = '2';
		bytes[2] = '3';
		Advise(m_sAdvise, bytes, 3, wxIPC_PRIVATE);
		// this works, but the log treats it as a string now
		// mConnection->Advise(mConnection->m_sAdvise, bytes, 3, wxIPC_TEXT );
	}
}

bool Rtt_LinuxIPCServerConnection::DoAdvise(const wxString &item, const void *data, size_t size, wxIPCFormat format)
{
	Log("Advise", "", item, data, size, format);

	return wxConnection::DoAdvise(item, data, size, format);
}

bool Rtt_LinuxIPCServerConnection::OnDisconnect()
{
	//wxLogMessage("OnDisconnect()");
	return true;
}
