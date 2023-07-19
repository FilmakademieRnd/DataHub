/*
-----------------------------------------------------------------------------
This source file is part of TRACER -
Toolset for Realtime Animation, Collaboration & Extended Reality
https://animationsinstitut.de/en/research/tools/tracer
http://github.com/FilmakademieRnd/VPET

Copyright (c) 2023 Filmakademie Baden-Wuerttemberg, Animationsinstitut R&D Lab

This project has been initiated in the scope of the EU funded project MAX-R
(https://www.upf.edu/web/max-r) under grant agreement no 101070072 in the years
2022-2024.

Since the DataHub is available for free, Filmakademie shall only be liable for
intent and gross negligence; warranty is limited to malice. DataHub may under
no circumstances be used for racist, sexual or any illegal purposes. In all
non-commercial productions, scientific publications, prototypical non-commercial
software tools, etc. using the DataHub Filmakademie has to be named as follows:
"TRACER - Toolset for Realtime Animation, Collaboration & Extended Reality by
Filmakademie Baden-Wuerttemberg, Animationsinstitut
(http://research.animationsinstitut.de)".

In case a company or individual would like to use DataHub in a commercial
surrounding or for commercial purposes, software based on these components or
any part thereof, the company/individual will have to contact Filmakademie
(research<at>filmakademie.de).
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

namespace DataHub {

	void SyncServer::run()
	{
        while (true)
        {
            m_ownIP = "";
            m_debug = false;

            QTextStream stream(stdin);
            QStringList commands;
            std::cout << "> ";
            commands = stream.readLine().split(" ");

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
                else
                    InitServer();
            }
        }
	}

    void SyncServer::stop()
    {
        m_zeroMQHandler->requestStop();
        m_zeroMQHandlerThread->exit();
    }

    void SyncServer::InitServer()
    {
        //create Thread to receive zeroMQ messages from tablets
        m_zeroMQHandlerThread = new QThread();
        m_zeroMQHandler = new ZeroMQHandler(m_ownIP, m_debug, m_context);

        m_zeroMQHandler->moveToThread(m_zeroMQHandlerThread);
        QObject::connect(m_zeroMQHandlerThread, SIGNAL(started()), m_zeroMQHandler, SLOT(run()));

        m_zeroMQHandlerThread->start();
        m_zeroMQHandler->requestStart();
    }

    void SyncServer::printHelp()
    {
        std::cout << "-h:       display this help" << std::endl;
        std::cout << "-ownIP:   IP address of this computer (required)" << std::endl;
        std::cout << "-d:       run with debug output" << std::endl;
    }

}