/*
-----------------------------------------------------------------------------
This source file is part of VPET - Virtual Production Editing Tool
http://vpet.research.animationsinstitut.de/
http://github.com/FilmakademieRnd/VPET

Copyright (c) 2018 Filmakademie Baden-Wuerttemberg, Animationsinstitut R&D Lab

This project has been initiated in the scope of the EU funded project 
Dreamspace under grant agreement no 610005 in the years 2014, 2015 and 2016.
http://dreamspaceproject.eu/
Post Dreamspace the project has been further developed on behalf of the 
research and development activities of Animationsinstitut.

The VPET components Scene Distribution and Synchronization Server are intended
for research and development purposes only. Commercial use of any kind is not 
permitted.

There is no support by Filmakademie. Since the Scene Distribution and 
Synchronization Server are available for free, Filmakademie shall only be 
liable for intent and gross negligence; warranty is limited to malice. Scene 
Distribution and Synchronization Server may under no circumstances be used for 
racist, sexual or any illegal purposes. In all non-commercial productions, 
scientific publications, prototypical non-commercial software tools, etc. 
using the Scene Distribution and/or Synchronization Server Filmakademie has 
to be named as follows: "VPET-Virtual Production Editing Tool by Filmakademie 
Baden-Wuerttemberg, Animationsinstitut (http://research.animationsinstitut.de)".

In case a company or individual would like to use the Scene Distribution and/or 
Synchronization Server in a commercial surrounding or for commercial purposes, 
software based on these components or any part thereof, the company/individual 
will have to contact Filmakademie (research<at>filmakademie.de).
-----------------------------------------------------------------------------
*/
#ifndef ZEROMQHANDLER_H
#define ZEROMQHANDLER_H

#include <QObject>
#include <QMutex>
#include <nzmqt/nzmqt.hpp>
#include "core.h"

class ThreadBase : public QObject
{
    Q_OBJECT

public:
    //! 
    //! Constructor
    //! 
    //! @param core A reference to the DataHub core.
    //! 
    explicit ThreadBase(DataHub::Core* core);

    //! Request this process to start working.
    virtual void requestStart();

    //! Request this process to stop working.
    virtual void requestStop();

protected:
    //! Default mutex used to lock the thread.
    QMutex m_mutex;

    //! If true process is stopped.
    bool m_stop;

    //! If true process is running.
    bool m_working;

    //! A reference to the DataHub core.
    DataHub::Core* m_core;

signals:
    //!
    //! Signal emitted when process is finished.
    //!
    void stopped();

public slots:
    //! Default thread worker loop.
    virtual void run() = 0;
};

class BroadcastPoller : public ThreadBase
{
    Q_OBJECT
public:
    //! 
    //! Constructor
    //! 
    //! @param core A reference to the DataHub core.
    //! @param item A reference to the ZMQ polling intem.
    //! @param waitCondition A reference to the wait condition for pausing the broadcast thread.
    explicit BroadcastPoller(DataHub::Core* core, zmq::pollitem_t* item, QWaitCondition* waitCondition) : ThreadBase(core), m_item(item), m_waitCondition(waitCondition) {}

public slots:
    //! Default thread worker loop.
    void run();

private:
    //! A reference to the zmq poller.
    zmq::pollitem_t *m_item;
    
    //! A reference to shared mutex for pausing the broadcast thread.
    QWaitCondition *m_waitCondition;
};

class ZeroMQHandler : public ThreadBase
{
    Q_OBJECT
public:
	//! 
    //! Constructor
    //! 
    //! @param core A reference to the DataHub core.
    //! @param IPAdress The IP adress the BroadcastHandler shall connect to. 
    //! @param debug Flag determin wether debug informations shall be printed.
    //! @param context The ZMQ context used by the BroadcastHandler.
    //! 
    explicit ZeroMQHandler(DataHub::Core* core, QString IPAdress, bool debug, zmq::context_t* context) : ThreadBase(core), m_IPadress(IPAdress), m_debug(debug), m_context(context) {}

	//! Tracer message types.
	enum MessageType
    {
        PARAMETERUPDATE, LOCK, // node
        SYNC, PING, RESENDUPDATE, // sync
        UNDOREDOADD, RESETOBJECT, // undo redo
        DATAHUB, // DataHub
        RPC, // RPC
        EMPTY = 255
    };

protected:
    //! Shall debug messages be printed.
    bool m_debug;

    //! ID displayed as clientID for messages redistributed through syncServer.
    byte m_targetHostID = 0;

    //! ZeroMQ context.
    zmq::context_t* m_context;

    //! The server IP adress.
    QString m_IPadress;

    //! 
    //! Helper function to convert characters to shorts. 
    //! 
    //! @param buf The char buffer to be converted.
    //! @return The converted and copyed buffer as short.
    //! 
    inline const short CharToShort(const char* buf) const
    {
        short val;
        std::memcpy(&val, buf, 2);
        return val;
    }

signals:
    //!
    //! Signal emitted when process is finished.
    //!
    void stopped();
};


#endif // ZEROMQHANDLER_H
