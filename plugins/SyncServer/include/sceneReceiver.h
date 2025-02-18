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
#ifndef SCENERECEIVER_H
#define SCENERECEIVER_H

#include "zeroMQHandler.h"
#include "sceneDataHandler.h"

class SceneReceiver : public ZeroMQHandler
{
	Q_OBJECT
public:
	//! 
	//! Constructor
	//! 
    //! @param core A reference to the DataHub core.
    //! @param IPAdress The IP adress the SceneReceiver shall connect to. 
    //! @param debug Flag determin wether debug informations shall be printed.
    //! @param context The ZMQ context used by the SceneReceiver.
    //! 
    explicit SceneReceiver(DataHub::Core* core, QString IPAdress = "", bool debug = false, zmq::context_t* context = NULL);
    ~SceneReceiver();

private:
	//!
	//! The list of request the reqester uses to request the packages.
	//!
	QList<QString> m_requests;
    SceneDataHandler *m_sceneData;
    QByteArray toByteArray(zmq::message_t& message) const
    {
        if (message.size() > 0)
            return QByteArray(static_cast<char*>(message.data()), static_cast<qsizetype>(message.size()));
        else
            return QByteArray();
    }

public slots:
    //execute operations
    void run();

};

#endif // SCENERECEIVER_H