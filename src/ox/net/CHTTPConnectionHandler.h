// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.

#ifndef OX_NET_CHTTPCONNECTIONHANDLER_H
#define OX_NET_CHTTPCONNECTIONHANDLER_H

#include "../event/IEventReceiver.h"
#include "../core/CString.h"
#include "../core/CCriticalSection.h"

namespace ox {

class IOxDevice;
namespace core { class CThread; }

namespace net {

class INetworkDevice;

//! Downloads a file over HTTP on a network device, reporting the content (or an error) to a
//! receiver as an EET_NETWORK_EVENT.
class CHTTPConnectionHandler : public event::IEventReceiver
{
public:
    CHTTPConnectionHandler();
    virtual ~CHTTPConnectionHandler();

    void init(IOxDevice* device, event::IEventReceiver* receiver);

    bool isConnected();
    void disconnect();

    //! Starts downloading file from host. Returns false if a download is already running.
    bool doGet(const core::CString<char>& file, const core::CString<wchar_t>& host, unsigned int port);

    virtual bool OnEvent(const event::SEvent& event);

private:
    static void joinThread(void* handler);

    //! Parses the response header: the Content-Length, -1 without one, or the negated status code.
    int getContentLength(core::CString<char>& header);

    INetworkDevice* NetworkDevice;
    bool Connected;
    core::CString<char> File;
    core::CString<char> Host;
    unsigned int Port;
    event::IEventReceiver* Receiver;
    core::CString<char> Buffer;
    core::CString<char> Content;
    int State;
    int ContentLength;
    core::CCriticalSection Lock;
    core::CThread* Thread;
};

} // end namespace net
} // end namespace ox

#endif
