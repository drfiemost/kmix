/*
 * KMix -- KDE's full featured mini mixer
 *
 *
 * Copyright (C) 2000 Stefan Schimanski <1Stein@gmx.de>
 * Copyright (C) 2001 Preston Brown <pbrown@kde.org>
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
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <qlayout.h>
#include <qwhatsthis.h>

#include <klocale.h>

#include "kmix.h"
#include "kmixprefdlg.h"
#include "kmixerwidget.h"


KMixPrefDlg::KMixPrefDlg( QWidget *parent )
    : KDialogBase(  Tabbed, i18n( "KMix Preferences" ),
          Ok|Cancel|Apply, Ok, parent )
{
   // general buttons
   m_generalTab = addPage( i18n("&General") );

   QBoxLayout *layout = new QVBoxLayout( m_generalTab );
   layout->setSpacing( KDialog::spacingHint() );

   m_dockingChk = new QCheckBox( i18n("&Dock into panel"), m_generalTab );
   layout->addWidget( m_dockingChk );
   QWhatsThis::add(m_dockingChk, i18n("Docks the mixer into the KDE panel"));

   m_volumeChk = new QCheckBox(i18n("Enable system tray &volume control"),
			       m_generalTab);
   layout->addWidget(m_volumeChk);

   m_showTicks = new QCheckBox( i18n("Show &tickmarks"), m_generalTab );
   layout->addWidget( m_showTicks );
   QWhatsThis::add(m_showTicks, 
           i18n("Enable/disable tickmark scales on the sliders"));

   m_showLabels = new QCheckBox( i18n("Show &labels"), m_generalTab );
   layout->addWidget( m_showLabels );
   QWhatsThis::add(m_showLabels, 
           i18n("Enables/disables description labels above the sliders"));

   connect( this, SIGNAL(applyClicked()), this, SLOT(apply()) );
}

KMixPrefDlg::~KMixPrefDlg()
{
}

void KMixPrefDlg::apply()
{
   emit signalApplied( this );
}

#include "kmixprefdlg.moc"
