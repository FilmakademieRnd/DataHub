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

//! @file "DataHub.cpp"
//! @brief DataHub executable application implementation. 
//! @author Simon Spielmann
//! @version 1
//! @date 03.07.2023

#include <QtCore>
#include <QDebug>

using namespace std;

int main(int argc, char** argv)
{
	QCoreApplication a(argc, argv);
	QStringList cmdlineArgs = QCoreApplication::arguments();

	// search for plugins
	QDir pluginsDir(QCoreApplication::applicationDirPath());
	qDebug() << pluginsDir;

	return a.exec();
}