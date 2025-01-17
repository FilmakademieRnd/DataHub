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

//! @file "DataHub.cpp"
//! @brief DataHub executable application implementation. 
//! @author Simon Spielmann
//! @version 1
//! @date 03.07.2023

#include <QtCore>
#include <QDebug>
#include <signal.h>
#include "core.h"
//#include <signal.h>

using namespace std;
using namespace DataHub;

class DataHubApp : public QCoreApplication
{
private:
	Core *m_core;
public:
	DataHubApp(int argc, char** argv) : QCoreApplication(argc, argv) 
	{
		QStringList cmdlineArgs = QCoreApplication::arguments();
		m_core = new Core(cmdlineArgs);
		connect(this, SIGNAL(aboutToQuit()), m_core, SLOT(coreQuit()));
		QTimer::singleShot(0, m_core, SLOT(loadPlugins()));

	}
	~DataHubApp() 
	{
		delete m_core;
	}
};

void sigHandler(int s)
{
	DataHubApp::quit();
}

int main(int argc, char** argv)
{
	DataHubApp  a(argc, argv);

	signal(SIGINT, sigHandler);
	signal(SIGTERM, sigHandler);
	signal(SIGBREAK, sigHandler);
	signal(SIGABRT, sigHandler);

 	return a.exec();
}
