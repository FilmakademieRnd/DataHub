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
#ifndef COMMANDHANDLER_H
#define COMMANDHANDLER_H

#include "broadcastHandler.h"

class CommandHandler : public ZeroMQHandler
{
	Q_OBJECT
public:
	//! 
	//! Constructor
	//! 
    //! @param core A reference to the DataHub core.
    //! @param zmqHandler A reference to the broadcastHandler. 
    //! @param IPAdress The IP adress the CommandHandler shall connect to. 
    //! @param debug Flag determin wether debug informations shall be printed.
    //! @param context The ZMQ context used by the CommandHandler.
    //! 
    explicit CommandHandler(DataHub::Core* core, BroadcastHandler* broadcastHandler, QString IPAdress = "", bool debug = false, zmq::context_t* context = NULL);

private:
    //! The global timeout for tracer clients.
    static const unsigned int m_pingTimeout = 3;

    //! The local elapsed time in seconds since object has been created.
    unsigned int m_time = 0;

    //! A reference to the zeroMQHandler. 
    BroadcastHandler* m_zmqHandler;

    //! The map storing the registered clients ping times.
    QMap<byte, unsigned int> m_pingMap;

private:
    void updatePingTimeouts(byte clientID);
    void checkPingTimeouts();
signals:
    //signal emitted when process is finished
    void stopped();

public slots:
    //execute operations
    void run();

private slots:
    void tickTime(int time);
};

#endif // COMMANDHANDLER_H