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
to be named as follows: “VPET-Virtual Production Editing Tool by Filmakademie 
Baden-Württemberg, Animationsinstitut (http://research.animationsinstitut.de)“.

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
#include <QMultiMap>
#include <QElapsedTimer>
#include <nzmqt/nzmqt.hpp>
#include "core.h"

class ThreadBase : public QObject
{
    Q_OBJECT

public:
    // constructor
    explicit ThreadBase(DataHub::Core* core);

    //request this process to start working
    virtual void requestStart();

    //request this process to stop working
    virtual void requestStop();

public slots:
    // default loop
    virtual void run() = 0;

protected:
    //id displayed as clientID for messages redistributed through syncServer
    byte m_targetHostID = 0;

    //protect access to _stop
    QMutex m_mutex;

    //if true process is stopped
    bool m_stop;

    //if true process is running
    bool m_working;

    //a reference to the core
    DataHub::Core* m_core;
};

class ZeroMQPoller : public ThreadBase
{
    Q_OBJECT
public:
    explicit ZeroMQPoller(DataHub::Core* core, zmq::pollitem_t* item, QWaitCondition* waitCondition);

public slots:
    void run();

private:
    // reference to the zmq poller
    zmq::pollitem_t *m_item;
    
    //reference to shared mutex for pausing the thread
    QWaitCondition *m_waitCondition;
};

class ZeroMQHandler : public ThreadBase
{
    Q_OBJECT
public:
    explicit ZeroMQHandler(DataHub::Core* core, QString IPAdress = "", bool debug = false, zmq::context_t* context = NULL);

    enum MessageType
    {
        PARAMETERUPDATE, LOCK, // node
        SYNC, PING, RESENDUPDATE, // sync
        UNDOREDOADD, RESETOBJECT, // undo redo
        DATAHUB, // DataHub
        EMPTY = 255
    };

private:
    //shall debug messages be printed
    bool m_debug;

    QMutex m_pauseMutex;

    QThread* m_zeroMQPollerThread;

    QWaitCondition* m_waitContition;

    //zeroMQ socket
    zmq::socket_t* m_socket;
    zmq::socket_t* m_sender;

    //zeroMQ context
    zmq::context_t* m_context;

    //syncMessage
    byte m_syncMessage[3] = { m_targetHostID,0,MessageType::EMPTY };

    //server IP
    QString m_IPadress;

    //map of last states
    QMap<QByteArray, QByteArray> m_objectStateMap;

    //map of ping timings
    QMap<byte, unsigned int> m_pingMap;

    //map of last states
    QMultiMap<byte, QByteArray> m_lockMap;

    //the local elapsed time in seconds since object has been created.
    unsigned int m_time = 0;

    static const unsigned int m_pingTimeout = 4;

    inline const short CharToShort(const char* buf) const
    {
        short val;
        std::memcpy(&val, buf, 2);
        return val;
    }

signals:

    //signal emitted when process is finished
    void stopped();

public slots:
    //execute operations
    void run();

private slots:
    //create a new sync message
    void createSyncMessage(int time);
};

class CommandHandler : public ThreadBase
{
    Q_OBJECT
public:
    explicit CommandHandler(DataHub::Core* core, QString IPAdress = "", bool debug = false, zmq::context_t* context = NULL);


private:
    //shall debug messages be printed
    bool m_debug;

    //zeroMQ socket
    zmq::socket_t* m_socket;

    //zeroMQ context
    zmq::context_t* m_context;

    //server IP
    QString m_IPadress;


signals:

    //signal emitted when process is finished
    void stopped();

public slots:
    //execute operations
    void run();

};

#endif // ZEROMQHANDLER_H
