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

class ZeroMQHandler : public QObject
{
    Q_OBJECT
public:
    explicit ZeroMQHandler(DataHub::Core* core, QString IPAdress = "", bool debug = false, zmq::context_t* context = NULL);

    //request this process to start working
    void requestStart();

    //request this process to stop working
    void requestStop();

    enum MessageType
    {
        PARAMETERUPDATE, LOCK, // node
        SYNC, PING, RESENDUPDATE, // sync
        UNDOREDOADD, RESETOBJECT, // undo redo
        DATAHUB, // DataHub
        EMPTY = 255
    };

private:
    DataHub::Core* _core;

    //id displayed as clientID for messages redistributed through syncServer
    byte targetHostID = 0;

    //if true process is stopped
    bool _stop;

    //shall debug messages be printed
    bool _debug;

    //if true process is running
    bool _working;

    //protect access to _stop
    QMutex mutex;

    //zeroMQ socket
    zmq::socket_t* socket_;
    zmq::socket_t* sender_;

    //zeroMQ context
    zmq::context_t* context_;

    //syncMessage
    byte syncMessage[3] = {targetHostID,0,MessageType::EMPTY};

    //server IP
    QString IPadress;

	//map of last states
    QMap<QByteArray, QByteArray> objectStateMap;

    //map of ping timings
    QMap<byte, unsigned int> pingMap;

    //map of last states
    QMultiMap<byte, QByteArray> lockMap;

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
    //signal emitted when process requests to work
    void startRequested();

    //signal emitted when process is finished
    void stopped();

public slots:    
    //execute operations
    void run();

private slots:
    //create a new sync message
    void createSyncMessage(int time);
};

#endif // ZEROMQHANDLER_H
