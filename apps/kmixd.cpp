/*
 * KMix -- KDE's full featured mini mixer
 *
 * Copyright 1996-2000 Christian Esken <esken@kde.org>
 * Copyright 2000-2003 Christian Esken <esken@kde.org>, Stefan Schimanski <1Stein@gmx.de>
 * Copyright 2002-2007 Christian Esken <esken@kde.org>, Helio Chissini de Castro <helio@conectiva.com.br>
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

#include "kmixd.h"

#include <kaboutdata.h> 
#include <kcmdlineargs.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kconfig.h>
#include <kaction.h>
#include <kstandardaction.h>
#include <kdebug.h>
#include <kxmlguifactory.h>
#include <kglobal.h>
#include <kactioncollection.h>
#include <ktoggleaction.h>
#include <KUniqueApplication>
#include <kpluginfactory.h>
#include <kpluginloader.h> 

// KMix
#include "core/GlobalConfig.h"
#include "core/mixertoolbox.h"
#include "core/kmixdevicemanager.h"
#include "core/mixer.h"
#include "core/version.h"


K_PLUGIN_FACTORY(KMixDFactory,
                 registerPlugin<KMixD>();
    )
K_EXPORT_PLUGIN(KMixDFactory("kmixd"))



/*
static const char description[] =
I18N_NOOP("KMixD - KDE's full featured mini mixer Service");

extern "C" KDE_EXPORT int kdemain(int argc, char *argv[])
{
   KAboutData aboutData( "kmixd", 0, ki18n("KMixD"),
                         APP_VERSION, ki18n(description), KAboutData::License_GPL,
                         ki18n("(c) 2010 Christian Esken"));
			 
			    KCmdLineArgs::init( argc, argv, &aboutData );

// As in the KUniqueApplication example only create a instance AFTER
// calling KUniqueApplication::start()
	KUniqueApplication app; 
  KMixD kmixd;
  app.exec();
}
  
  */

/* KMixD
 * Constructs a mixer window (KMix main window)
 */
KMixD::KMixD(QObject* parent, const QList<QVariant>&) :
   KDEDModule(parent),
   m_multiDriverMode (false) // -<- I never-ever want the multi-drivermode to be activated by accident
{
    setObjectName( QLatin1String("KMixD" ));
	GlobalConfig::init();
	kWarning() << "kmixd: Triggering delayed initialization";
	QTimer::singleShot( 3000, this, SLOT(delayedInitialization()));
}

/**
 * This is called by a singleShot timer. Reason is that KMixD seems to "collide" with other applications
 * on DBUS, likely creating deadlocks. See Bug 317926 for that. By artificially delaying initialization,
 * KMixD gets hopefully out of the way of the other applications, avoiding these deadlocks. A small delay
 * should also not harm too much, as apps using the Mixer DBUS calls on KMixD must be prepared that the
 * interface is not available rihgt when they start.
 */
void KMixD::delayedInitialization()
{
	kWarning() << "kmixd: Delayed initialization running now";
   //initActions(); // init actions first, so we can use them in the loadConfig() already
   loadConfig(); // Load config before initMixer(), e.g. due to "MultiDriver" keyword
   MixerToolBox::instance()->initMixer(m_multiDriverMode, m_backendFilter, m_hwInfoString, true);
   KMixDeviceManager *theKMixDeviceManager = KMixDeviceManager::instance();
   theKMixDeviceManager->initHotplug();
   connect(theKMixDeviceManager, SIGNAL(plugged(const char*,QString,QString&)), SLOT (plugged(const char*,QString,QString&)) );
   connect(theKMixDeviceManager, SIGNAL(unplugged(QString)), SLOT (unplugged(QString)) );

	kWarning() << "kmixd: Delayed initialization done";
}


KMixD::~KMixD()
{
   MixerToolBox::instance()->deinitMixer();
}



void KMixD::saveConfig()
{
   kDebug() << "About to save config";
   saveBaseConfig();
  // saveVolumes(); // -<- removed from kmixd, as it is possibly a bad idea if both kmix and kmixd write the same config "kmixctrlrc"
#ifdef __GNUC_
#warn We must Sync here, or we will lose configuration data. The reson for that is unknown.
#endif

   kDebug() << "Saved config ... now syncing explicitly";
   KGlobal::config()->sync();
   kDebug() << "Saved config ... sync finished";
}

void KMixD::saveBaseConfig()
{
   kDebug() << "About to save config (Base)";
   KConfigGroup config(KGlobal::config(), "Global");

   config.writeEntry( "ConfigVersion", KMIX_CONFIG_VERSION );
   Mixer* mixerMasterCard = Mixer::getGlobalMasterMixer();
   if ( mixerMasterCard != 0 ) {
      config.writeEntry( "MasterMixer", mixerMasterCard->id() );
   }
   shared_ptr<MixDevice> mdMaster = Mixer::getGlobalMasterMD();
   if ( mdMaster ) {
      config.writeEntry( "MasterMixerDevice", mdMaster->id() );
   }
   QString mixerIgnoreExpression = MixerToolBox::instance()->mixerIgnoreExpression();
   config.writeEntry( "MixerIgnoreExpression", mixerIgnoreExpression );

   kDebug() << "Config (Base) saving done";
}



void KMixD::loadConfig()
{
   loadBaseConfig();
}

void KMixD::loadBaseConfig()
{
    KConfigGroup config(KGlobal::config(), "Global");

   m_multiDriverMode = config.readEntry("MultiDriver", false);
   QString mixerMasterCard = config.readEntry( "MasterMixer", "" );
   QString masterDev = config.readEntry( "MasterMixerDevice", "" );
   Mixer::setGlobalMaster(mixerMasterCard, masterDev, true);
   QString mixerIgnoreExpression = config.readEntry( "MixerIgnoreExpression", "Modem" );
   m_backendFilter = config.readEntry<>( "Backends", QList<QString>() );
   MixerToolBox::instance()->setMixerIgnoreExpression(mixerIgnoreExpression);
}


void KMixD::plugged( const char* driverName, const QString& /*udi*/, QString& dev)
{
//     kDebug(67100) << "Plugged: dev=" << dev << "(" << driverName << ") udi=" << udi << "\n";
    QString driverNameString;
    driverNameString = driverName;
    int devNum = dev.toInt();
    Mixer *mixer = new Mixer( driverNameString, devNum );
    if ( mixer != 0 ) {
        kDebug(67100) << "Plugged: dev=" << dev << "\n";
        MixerToolBox::instance()->possiblyAddMixer(mixer);
    }

}

void KMixD::unplugged( const QString& udi)
{
//     kDebug(67100) << "Unplugged: udi=" <<udi << "\n";
    for (int i=0; i<Mixer::mixers().count(); ++i) {
        Mixer *mixer = (Mixer::mixers())[i];
//         kDebug(67100) << "Try Match with:" << mixer->udi() << "\n";
        if (mixer->udi() == udi ) {
            kDebug(67100) << "Unplugged Match: Removing udi=" <<udi << "\n";
            //KMixToolBox::notification("MasterFallback", "aaa");
            bool globalMasterMixerDestroyed = ( mixer == Mixer::getGlobalMasterMixer() );

            MixerToolBox::instance()->removeMixer(mixer);
            // Check whether the Global Master disappeared, and select a new one if necessary
            shared_ptr<MixDevice> md = Mixer::getGlobalMasterMD();
            if ( globalMasterMixerDestroyed || md.get() == 0 ) {
                // We don't know what the global master should be now.
                // So lets play stupid, and just select the recommended master of the first device
                if ( Mixer::mixers().count() > 0 ) {
                	shared_ptr<MixDevice> master = ((Mixer::mixers())[0])->getLocalMasterMD();
                    if ( master.get() != 0 ) {
                        QString localMaster = master->id();
                        Mixer::setGlobalMaster( ((Mixer::mixers())[0])->id(), localMaster, false);

                        QString text;
                        text = i18n("The soundcard containing the master device was unplugged. Changing to control %1 on card %2.",
                                master->readableName(),
                                ((Mixer::mixers())[0])->readableName()
                        );
//                        KMixToolBox::notification("MasterFallback", text);
                    }
                }
            }
            if ( Mixer::mixers().count() == 0 ) {
                QString text;
                text = i18n("The last soundcard was unplugged.");
//                KMixToolBox::notification("MasterFallback", text);
            }
            break;
        }
    }

}


#include "moc_kmixd.cpp"
