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
			m_timer = new QTimer();
			m_timer->moveToThread(this);
			connect(m_timer, SIGNAL(timeout()), this, SIGNAL(tick()), Qt::DirectConnection);
			if (m_random)
				connect(m_timer, SIGNAL(timeout()), this, SLOT(setrandominterval()), Qt::DirectConnection);
			m_timer->setTimerType(Qt::PreciseTimer);
			m_timer->start(m_interval);
			exec();
		}

	public:
		TimerThread(QObject* parent = nullptr) : m_interval(1000), m_timer(0), m_random(false), QThread(parent) {}
		TimerThread(bool random, QObject* parent = nullptr) : m_interval(1000), m_timer(0), m_random(random), QThread(parent) {}
		TimerThread(float interval, bool random, QObject* parent = nullptr) : m_interval(qFloor(interval)), m_timer(0), m_random(random), QThread(parent) {}

	private:
		bool m_random;
		int m_interval;
		QTimer *m_timer; 

	signals:
		void tick();
	
	private slots:
		void setrandominterval() 
		{ 
			m_interval = QRandomGenerator::global()->bounded(1000, 1700);
			m_timer->setInterval(m_interval);
		}

	};

	class CORESHARED_EXPORT Core : public QObject
	{
		Q_OBJECT

	public:
		Core();
		Core(QStringList cmdlineArgs);

	public:
		unsigned char m_time = 0;
	private:
		QMultiMap<QString, PluginInterface*> s_plugins;
		QStringList m_cmdlineArgs;
		TimerThread *m_tthread;
		TimerThread *m_trandthread;

		unsigned char m_timesteps = 0;
		static const int s_framerate = 60;
		static const int s_timestepsBase = 128;

	public:
		QStringList getAppArguments();

	public:
		void recordData(QByteArray data);
		void sceneReceive(QString ip);

	private slots:
		void updateTime();
		void updateTimeRand();
		
	public slots:
		void loadPlugins();
		void coreQuit();

	signals:
		void tickTick(int time);
		void tickHalf(int time);
		void tickSecond(int time);
		void tickSecondRandom(int time);
		void recordDataSignal(QByteArray data);
		void sceneReceiveSignal(QString ip);
        
	};

}

#endif