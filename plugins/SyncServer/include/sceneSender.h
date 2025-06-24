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
#ifndef SCENESENDER_H
#define SCENESENDER_H

#include "zeroMQHandler.h"
#include "sceneDataHandler.h"

class SceneSender : public ZeroMQHandler
{
	Q_OBJECT
public:
	//! 
	//! Constructor
	//! 
    //! @param core A reference to the DataHub core.
    //! @param IPAddress The IP address the SceneSender shall connect to. 
    //! @param debug Flag determin wether debug informations shall be printed.
    //! @param context The ZMQ context used by the SceneSender.
    //! 
    explicit SceneSender(DataHub::Core* core, QString serverAddress = "", QString clientAddress = "", bool debug = false, zmq::context_t* context = NULL);
    ~SceneSender();

private:
	//!
	//! The list of request the reqester uses to request the packages.
	//!
	QMap<std::string, QByteArray*> m_responses;
    SceneDataHandler *m_sceneData;
    QString m_clientAddress;
    bool loadData();


public slots:
    //execute operations
    void run();

};

#endif // SCENESENDER_H