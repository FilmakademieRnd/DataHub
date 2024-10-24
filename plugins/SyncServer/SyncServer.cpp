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

//! @file "SyncServer.cpp"
//! @brief Datahub Plugin: Sync Server defines the network bridge between TRACER clients and servers.
//! @author Simon Spielmann
//! @version 1
//! @date 03.07.2023

#include "SyncServer.h"
#include <QtNetwork/QNetworkInterface>
#include <QtNetwork/QHostAddress>
#include <iostream>
#include "core.h"


namespace DataHub {

    void SyncServer::init()
    {
        // fill me!
    }

	void SyncServer::run()
	{
		QStringList commands = core()->getAppArguments();
		commands.removeAt(0);

        while (true)
        {
            m_ownIP = "";
            m_debug = false;

            if (commands.length() < 1)
            {
                QTextStream stream(stdin);
                std::cout << "> ";
                commands = stream.readLine().split(" ");
            }

            if (commands.length() < 1) {
                printHelp();
                continue;
            }
            else
            {
                QList<QHostAddress> availableIpAdresses = QNetworkInterface::allAddresses();
                int i = 0;
                while (i < availableIpAdresses.length())
                {
                    if (availableIpAdresses[i].toString().contains(":"))
                        availableIpAdresses.removeAt(i);
                    else
                        i++;
                }

                for (int i=0; i<commands.length(); i++)
                {
                    if (commands[i] == "-h")
                        printHelp();
                    else if (commands[i] == "-d")
                    {
                        std::cout << "Debug output enabled." << std::endl;
                        m_debug = true;
                    }
                    else if (commands[i] == "-np")
                    {
                        std::cout << "No parameter history." << std::endl;
                        m_paramHistory = false;
                    }
                    else if (commands[i] == "-nl")
                    {
                        std::cout << "No lock history." << std::endl;
                        m_lockHistory = false;
                    }
                    else if (commands[i] == "-ownIP" && commands.length() > i+1)
                    {
                        m_ownIP = "f";
                        foreach(QHostAddress ipAdress, availableIpAdresses)
                        {
                            QString ipCommand = commands[i + 1];
                            if (ipAdress.toString() == ipCommand)
                            {
                                m_ownIP = ipCommand;
                                break;
                            }
                        }
                    }
                }
                if (m_ownIP == "f")
                {
                    std::cout << "No valid IP address for this server (own IP adress) was defined." << std::endl;
                    std::cout << "Choose from the following valid IP adresses of this PC:" << std::endl;
                    for (int l = 0; l < availableIpAdresses.length(); l++)
                        std::cout << availableIpAdresses[l].toString().toLatin1().data() << std::endl;
                }
                else if (m_ownIP == "")
                {
                    printHelp();
                }
                else {
                    InitServer();
                }
            }
            commands.clear();
        }
	}

    void SyncServer::stop()
    {
        m_broadcastHandler->requestStop();
        m_broadcastHandlerThread->exit();

        m_commandHandler->requestStop();
        m_commandHandlerThread->exit();
    }

    void SyncServer::InitServer()
    {
        //create thread to receive zeroMQ messages from clients
       
        m_broadcastHandler = new BroadcastHandler(core(), m_ownIP, m_debug, m_paramHistory, m_lockHistory, m_context);
        m_broadcastHandlerThread = new QThread(this);

        //create thread to receive command messages from clients

        m_commandHandler = new CommandHandler(core(), m_broadcastHandler, m_ownIP, m_debug, m_context);
        m_commandHandlerThread = new QThread(this);

        m_broadcastHandler->moveToThread(m_broadcastHandlerThread);
        QObject::connect(m_broadcastHandlerThread, &QThread::started, m_broadcastHandler, &BroadcastHandler::run);

        m_commandHandler->moveToThread(m_commandHandlerThread);
        QObject::connect(m_commandHandlerThread, &QThread::started, m_commandHandler, &CommandHandler::run);

        m_broadcastHandlerThread->start();
        m_broadcastHandler->requestStart();

        m_commandHandlerThread->start();
        m_commandHandler->requestStart();
    }

    void SyncServer::printHelp()
    {
        std::cout << "-h:       display this help" << std::endl;
        std::cout << "-ownIP:   IP address of this computer (required)" << std::endl;
        std::cout << "-d:       run with debug output" << std::endl;
        std::cout << "-np:      run without parameter history" << std::endl;
        std::cout << "-nl:      run without lock history" << std::endl;
    }

}