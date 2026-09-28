// Copyright (C) 2002-2004 Nikolaus Gebhardt
// Adapted from Irrlicht 0.7 include/IEventReceiver.h (license: third_party/irrlicht-0.7/include/irrlicht.h).
// Recovered for Harvest's ox::event namespace; not the original source. Partial: only the network
// event is recovered.

#ifndef OX_EVENT_IEVENTRECEIVER_H
#define OX_EVENT_IEVENTRECEIVER_H

namespace ox {
namespace event {

enum EEVENT_TYPE
{
    //! A network device event (the only event type recovered so far).
    EET_NETWORK_EVENT = 5
};

//! Network device events, in SEvent::NetworkEvent.Type.
enum ENETWORK_EVENT_TYPE
{
    ENET_CONNECTED = 2,
    ENET_CONNECTION_FAILED = 3,
    ENET_DATA_RECEIVED = 6,
    ENET_DISCONNECTED = 8,
    //! Sent by CHTTPConnectionHandler with the downloaded content as Data.
    ENET_HTTP_DONE = 9,
    //! Sent by CHTTPConnectionHandler with an error text as Data.
    ENET_HTTP_ERROR = 10
};

struct SEvent
{
    EEVENT_TYPE EventType;
    union
    {
        struct
        {
            int Type;
            int ClientId;
            //! Data size, or a negative error code with ENET_HTTP_ERROR.
            int Size;
            int Reserved;
            char* Data;
        } NetworkEvent;

        // Other event structs are not recovered yet. The Linux amd64 SEvent is 48 bytes, and
        // CHTTPConnectionHandler::OnEvent keeps several on the stack, so the union keeps that size.
        void* unrecovered[5];
    };
};

class CEventSubscriberList;

//! Interface of an object which can receive events.
class IEventReceiver
{
public:
    IEventReceiver()
        : SubscriberList(0)
    {
    }

    //! Called if an event happened. Returns true if the event was processed.
    virtual bool OnEvent(const SEvent& event) = 0;

    virtual ~IEventReceiver();

    void subscribe(CEventSubscriberList* list);

private:
    CEventSubscriberList* SubscriberList;
};

} // end namespace event
} // end namespace ox

#endif
