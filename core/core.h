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

//! @file "core.h"
//! @brief DataHub core implementation. Central class for DataHub initalization. Also manages all plugins.
//! @author Simon Spielmann
//! @version 1
//! @date 03.07.2023

#ifndef CORE_H
#define CORE_H

#include "plugininterface.h"
#include <QtCore>
#include <QMultiMap>


namespace DataHub {

	class CORESHARED_EXPORT TimerThread : public QThread
	{
		Q_OBJECT

		void run()
		{
			QTimer timer;
			connect(&timer, SIGNAL(timeout()), this, SIGNAL(tick()), Qt::DirectConnection);
			timer.setTimerType(Qt::PreciseTimer);
			timer.start(m_interval);
			timer.moveToThread(this);
			exec();
		}

	public:
		TimerThread() : m_interval(1000) {}
		TimerThread(float interval) : m_interval(qCeil(interval)) {}

	private:
		int m_interval;

	signals:
		void tick();

	};

	class CORESHARED_EXPORT Core : public QObject
	{
		Q_OBJECT

	public:
		Core();
		~Core();
	public:
		unsigned char m_time = 0;
	private:
		QMultiMap<QString, PluginInterface*> s_plugins;
		QTimer* m_timer;
		QThread* m_TimerThread;

		unsigned char m_timesteps = 0;
		static const int s_framerate = 60;
		static const int s_timestepsBase = 128;

	private:
		void loadPlugins();

	private slots:
		void updateTime();

	signals:
		void tickSecond(int time);
        
	};

}

#endif