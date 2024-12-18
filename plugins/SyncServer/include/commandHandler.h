/*
-----------------------------------------------------------------------------
Copyright (c) 2024 Filmakademie Baden-Wuerttemberg, Animationsinstitut R&D Labs
https://research.animationsinstitut.de/datahub
https://github.com/FilmakademieRnd/DataHub

Datahub is a development by Filmakademie Baden-Wuerttemberg, Animationsinstitut
R&D Labs in the scope of the EU funded project MAX-R (101070072) and funding on
the own behalf of Filmakademie Baden-Wuerttemberg.  Former EU projects Dreamspace
(610005) and SAUCE (780470) have inspired the DataHub development.

The DataHub is intended for research and development purposes only.
Commercial use of any kind is not permitted.

There is no support by Filmakademie. Since the Data Hub is available for free,
Filmakademie shall only be liable for intent and gross negligence; warranty
is limited to malice. DataHub may under no circumstances be used for racist,
sexual or any illegal purposes. In all non-commercial productions, scientific
publications, prototypical non-commercial software tools, etc. using the DataHub
Filmakademie has to be named as follows: "DataHub by Filmakademie
Baden-Wuerttemberg, Animationsinstitut (http://research.animationsinstitut.de)".

In case a company or individual would like to use the Data Hub in a commercial
surrounding or for commercial purposes, software based on these components or
any part thereof, the company/individual will have to contact Filmakademie
(research<at>filmakademie.de) for an individual license agreement.
-----------------------------------------------------------------------------
*/
#ifndef COMMANDHANDLER_H
#define COMMANDHANDLER_H

#include "messageReceiver.h"

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
    explicit CommandHandler(DataHub::Core* core, MessageSender* messageSender, MessageReceiver* messageReceiver, QString IPAdress = "", bool debug = false, zmq::context_t* context = NULL);

private:
    //! The global timeout for tracer clients.
    static const unsigned int m_pingTimeout = 3;

    //! The local elapsed time in seconds since object has been created.
    unsigned int m_time = 0;

    //! A reference to the message sender. 
    MessageSender* m_sender;

    //! A reference to the message receiver. 
    MessageReceiver* m_receiver;

    //! The map storing the registered clients ping times.
    QMap<byte, unsigned int> m_pingMap;

private:
    void updatePingTimeouts(byte clientID, bool isServer);
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