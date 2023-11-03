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
#ifndef BROADCASTHANDLER_H
#define BROADCASTHANDLER_H

#include "zeroMQHandler.h"
#include <QMultiMap>

class BroadcastHandler : public ZeroMQHandler
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
    explicit BroadcastHandler(DataHub::Core* core, QString IPAdress = "", bool debug = false, zmq::context_t* context = NULL);

private:

    //! Mutex used for pausing the BroadcastHandler thread.
    QMutex m_pauseMutex;

    //! Wait condition used for pausing the BroadcastHandler thread.
    QWaitCondition* m_waitContition;

    //! Co thread for polling.
    QThread* m_zeroMQPollerThread;

    //! Buffer used for broadcasting sync messages.
    byte m_syncMessage[3] = { m_targetHostID,0,MessageType::EMPTY };

    //! Buffer used for broadcasting general purpose messages.
    QByteArray m_broadcastMessage;

    //! The map storing last scene object states. 
    QMap<QByteArray, QByteArray> m_objectStateMap;

    //! The map storing scene objects lock status. 
    QMultiMap<byte, QByteArray> m_lockMap;

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

public:
    //!
    //! Function to broadcast a messagte to all connected clients.
    //!
    //! @param message The message to be broadcasted. 
    //!
    void BroadcastMessage(QByteArray message);

    //!
    //! Function to get a reference to the Lock map.
    //!
    //! @return The reference to the lockMap.
    //!
    QMultiMap<byte, QByteArray>* GetLockMap();

signals:
    //!
    //! Signal emitted when process is finished.
    //!
    void stopped();

public slots:
    //!
    //! The broadcast thread's main worker loop.
    //!
    void run();
    //!
    //! Slot to create a new sync message.
    //!
    //! @param time The current tracer time.
    //!
    void createSyncMessage(int time);
};


#endif // BROADCASTHANDLER_H