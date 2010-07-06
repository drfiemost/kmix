//-*-C++-*-
/*
 * KMix -- KDE's full featured mini mixer
 *
 *
 * Copyright Chrisitan Esken <esken@kde.org>
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

#ifndef MDWSLIDER_H
#define MDWSLIDER_H

#include <KShortcut>
#include <QCheckBox>
#include <QList>
#include <QWidget>
#include <qlist.h>
#include <qpixmap.h>

class QBoxLayout;
class QToolButton;
class QLabel;

class KAction;
class KMenu;
#include <kshortcut.h>

class MixDevice;
class VerticalText;
class Mixer;
class ViewBase;

#include "mixdevicewidget.h"
#include "volume.h"


class MDWSlider : public MixDeviceWidget
{
    Q_OBJECT

public:
    MDWSlider( MixDevice* md,
	       bool includePlayback, bool includeCapture,
	       bool small, Qt::Orientation,
	       QWidget* parent, ViewBase* view, ProfControl *pctl);
    ~MDWSlider() { }

    void addActionToPopup( KAction *action );
    void createActions();
    void createShortcutActions(ViewBase* view = 0);
    
    // GUI
    bool isStereoLinked() const { return m_linked; }
    void setStereoLinked( bool value );
    void setLabeled( bool value );
    void setTicks( bool ticks );
    void setIcons( bool value );
    void setColors( QColor high, QColor low, QColor back );
    void setMutedColors( QColor high, QColor low, QColor back );
    
    bool eventFilter( QObject* obj, QEvent* e );
    QString iconName();
    // Layout
    QSizePolicy sizePolicy() const;
	QSize sizeHint() const;
	int labelExtentHint() const;
	void setLabelExtent(int extent);
	bool hasMuteButton() const;
	void setMuteButtonSpace(bool);
	void setCaptureLEDSpace(bool);
	bool hasCaptureLED() const;


    
public slots:
    void toggleRecsrc();
    void toggleMuted();
    void toggleStereoLinked();

    void setDisabled();
    void setDisabled( bool value );
    void update();
    void showMoveMenu();
    virtual void showContextMenu();


signals:
//    void newVolume( int num, Volume volume );
//    void newMasterVolume( Volume volume );
//    void masterMuted( bool );
    void toggleMenuBar(bool value);

private slots:
    void setRecsrc( bool value );
    void setMuted(bool value);
    void volumeChange( int );

    void increaseVolume();
    void decreaseVolume();

    void moveStreamAutomatic();
    void moveStream( QString destId );

private:
    KShortcut dummyShortcut;
    void setIcon( QString iconname );
    QPixmap loadIcon( QString filename );
    void createWidgets( bool showMuteLED, bool showCaptureLED );
    void addSliders( QBoxLayout *volLayout, char type, bool addLabel);
    void addDefaultLabel(QBoxLayout *layout, Qt::Orientation orientation);

    // Methods that are called two times from a wrapper. Once for playabck, once for capture
    void setStereoLinkedInternal( QList<QWidget *>& ref_sliders );
    void setTicksInternal( QList<QWidget *>& ref_sliders,  bool ticks );
    void volumeChangeInternal(Volume& vol, QList<Volume::ChannelID>& slidersChids, QList<QWidget *>& ref_sliders );
    void updateInternal(Volume& vol, QList<QWidget *>& ref_sliders, QList<Volume::ChannelID>& slidersChids);


    bool m_linked;

	QWidget *muteButtonSpacer;
	QWidget *captureSpacer;
	QWidget *labelSpacer;

    // GUI: Top portion ( Icon + Mute)
	QLabel      *m_iconLabelSimple;
	QToolButton* m_qcb;
	QLabel* m_muteText;
        
    QWidget *m_extraCaptureLabel; // extra capture label (if you got playback AND capture on the same control)
	QLabel *m_label; // is either QLabel or VerticalText
    
	QCheckBox* m_captureCheckbox;
    QLabel* m_captureText;

	int labelSpacing;
	bool muteButtonSpacing;
	bool captureLEDSpacing;

    QList<QWidget *> m_slidersPlayback;
    KActionCollection*   _mdwMoveActions;
    KMenu *m_moveMenu;
    QList<QWidget *> m_slidersCapture;
    QList<Volume::ChannelID> _slidersChidsPlayback;
    QList<Volume::ChannelID> _slidersChidsCapture;
};

#endif
