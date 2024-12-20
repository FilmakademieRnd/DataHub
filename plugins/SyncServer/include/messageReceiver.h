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
#ifndef MESSAGERECEIVER_H
#define MESSAGERECEIVER_H

#include "zeroMQHandler.h"
#include "messageSender.h"
#include <QMultiMap>


class MessageReceiver : public ZeroMQHandler
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
    explicit MessageReceiver(DataHub::Core* core, QList<MessageSender*> messageSenders, QString IPAdress = "", bool debug = false, bool webSockets = false, bool parameterHistory = true, bool lockHistory = true, zmq::context_t* context = NULL);

    ~MessageReceiver()
    {    }

private:
    bool m_parameterHistory;
    bool m_lockHistory;

    //! Default mutex used to lock the thread.
    QMutex m_lockMapMutex;

    //! The map storing last scene object states. 
    QMap<QByteArray, QByteArray> m_objectStateMap;

    //! The map storing scene objects lock status. 
    QMultiMap<byte, QByteArray> m_lockMap;

    //! List of references to all message senders.
    QList<MessageSender*> m_senders;

private:
    //! function queing message into all registered senders send ques.
     inline void QueMessage(zmq::message_t&& message)
     {
         for (int i = 1; i < m_senders.count(); i++) {
             zmq::message_t msgCopy;
             msgCopy.copy(message);
             m_senders[i]->QueMessage(std::move(msgCopy));
         }

         m_senders[0]->QueMessage(std::move(message));
     }

     //! function queing message into all registered senders send ques.
     inline void QueBroadcastMessage(zmq::message_t&& message)
     {
         for (int i = 1; i < m_senders.count(); i++) {
             zmq::message_t msgCopy;
             msgCopy.copy(message);
             m_senders[i]->QueBroadcastMessage(std::move(msgCopy));
         }

         m_senders[0]->QueBroadcastMessage(std::move(message));
     }

public:

    void CheckLocks(byte clientID);

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

};



#endif // MESSAGERECEIVER_H