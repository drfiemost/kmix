/*
 * KMix -- KDE's full featured mini mixer
 *
 * Copyright 2011 Igor Poboiko <igor.poboiko@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef DBUSMIXSETWRAPPER_H
#define DBUSMIXSETWRAPPER_H

#include <QStringList>
#include "core/mixer.h"

class DBusMixSetWrapper : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QStringList mixers READ mixers)
	Q_PROPERTY(QString currentMasterMixer READ currentMasterMixer)
	Q_PROPERTY(QString currentMasterControl READ currentMasterControl)
	Q_PROPERTY(QString preferredMasterMixer READ preferredMasterMixer)
	Q_PROPERTY(QString preferredMasterControl READ preferredMasterControl)
	public:
		DBusMixSetWrapper(QObject* parent, QString path);
		~DBusMixSetWrapper();
	public slots:
		QStringList mixers() const;
		
		QString currentMasterMixer() const;
		QString currentMasterControl() const;
		QString preferredMasterMixer() const;
		QString preferredMasterControl() const;
        void setCurrentMaster(const QString &mixer, const QString &control);
        void setPreferredMaster(const QString &mixer, const QString &control);

		void devicePlugged( const char* driverName, const QString& udi, QString& dev );
		void deviceUnplugged( const QString& udi );

//        void slotCurrentMasterChanged();
//        void slotPreferredMasterChanged();
	private:
		QString m_dbusPath;
        
};

#endif /* DBUSMIXSETWRAPPER_H */
