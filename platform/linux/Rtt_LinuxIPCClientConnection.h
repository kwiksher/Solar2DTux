#ifndef LINUXIPCCLIENTCONNECTION_H
#define LINUXIPCCLIENTCONNECTION_H

#include <wx/wx.h>
#include <wx/ipc.h>
#include <string.h>

class Rtt_LinuxIPCClientConnection: public wxConnection
{
public:
    virtual bool DoPoke(const wxString& item, const void* data, size_t size, wxIPCFormat format) wxOVERRIDE;
    virtual bool OnDisconnect() wxOVERRIDE;
};

#endif //LINUXIPCCLIENTCONNECTION_H