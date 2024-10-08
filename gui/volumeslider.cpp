//-*-C++-*-
/*
 * KMix -- KDE's full featured mini mixer
 *
 *
 * Copyright Christian Esken
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
#include <QStyleOption>
#include <QFontMetrics>

#include "volumeslider.h"
#include <kdebug.h>

VolumeSlider::VolumeSlider(Qt::Orientation orientation, QWidget* parent) : QSlider(orientation, parent)
,m_orientation(orientation),m_tooltip(new QLabel(parent,Qt::ToolTip))
{
	QFontMetrics metrics(m_tooltip->font());
	//Setting a size big enough for all values less than 100
	QRect labelRect = metrics.boundingRect("88");
//	m_tooltip->setContentsMargins(3,1,3,1);
	m_tooltip->setMinimumWidth(labelRect.width()+5);
	m_tooltip->setMinimumHeight(labelRect.height()+3);
	m_tooltip->setAlignment(Qt::AlignCenter);
//	m_tooltip->setAlignment(Qt::AlignVCenter);
}

void VolumeSlider::mousePressEvent(QMouseEvent* event)
{
	QSlider::mousePressEvent(event);

	QStyleOptionSlider opt;
	initStyleOption(&opt);
	QRect sliderHandle = style()->subControlRect(QStyle::CC_Slider,&opt,QStyle::SC_SliderHandle,this);

	if(sliderHandle.contains(event->pos()))
	{
		if (m_orientation == Qt::Vertical)
		{
			m_tooltip->move(mapToGlobal(sliderHandle.topLeft()).x()+width(),mapToGlobal(sliderHandle.topLeft()).y());
		}
		else
		{
			m_tooltip->move(mapToGlobal(sliderHandle.topLeft()).x(),mapToGlobal(sliderHandle.topLeft()).y()+height());
		}

		/**
		 * 1) The following operates on the value of one individual channel slider, and thus is representing only the
		 * value of that channel. This is technically not 100% sound for a corner case: The average of all channels
		 * should be displayed in the "joined slider mode". On the other hand, in "joined slider mode", the slider
		 * should already represent the average of all channels. And as soon as you move the slider, all channels
		 * are changed. So it is likely really exact enough.
		 *
		 * 2) Future directions: It would be better to do the percentage calculation via the Volume class, as it
		 * handles corner cases like muting. But due to "auto-unmuting" the value is factually also correct. As the
		 * VolumeSlider class currently holds no pointer/reference to the "underlying" Volume object, a bit code
		 * "duplication" is acceptable here.
		 */
		qreal percentReal = ((qreal)100 * value() ) / ( maximum() - minimum());
		int percent = qRound(percentReal);
		m_tooltip->setText(QString::number(percent));
		m_tooltip->show();
	}
}

void VolumeSlider::mouseReleaseEvent(QMouseEvent* event)
{
	QSlider::mouseReleaseEvent(event);
	m_tooltip->hide();
}

void VolumeSlider::mouseMoveEvent(QMouseEvent* event)
{
	QSlider::mouseMoveEvent(event);

	QStyleOptionSlider opt;
	initStyleOption(&opt);
	QRect sliderHandle = style()->subControlRect(QStyle::CC_Slider,&opt,QStyle::SC_SliderHandle,this);

	qreal percentReal = ((qreal)100 * value() ) / ( maximum() - minimum() );
	int percent = qRound(percentReal);

	//Change width of label if percent becomes 100
	if (percent == 100)
	{
		QFontMetrics metrics(m_tooltip->font());
		QRect labelRect = metrics.boundingRect("100");
		m_tooltip->resize(labelRect.width(),m_tooltip->height());
	}
	else if(m_tooltip->width() > m_tooltip->minimumWidth())
	{
		m_tooltip->resize(m_tooltip->minimumWidth(),m_tooltip->height());
	}
	m_tooltip->setText(QString::number(percent));

	if (m_orientation == Qt::Vertical)
	{
		m_tooltip->move(mapToGlobal(sliderHandle.topLeft()).x()+width(),mapToGlobal(sliderHandle.topLeft()).y());
	}
	else
	{
		m_tooltip->move(mapToGlobal(sliderHandle.topLeft()).x(),mapToGlobal(sliderHandle.topLeft()).y()+height());
	}

	//kDebug() << "Position is"<<mapToGlobal(sliderHandle.topLeft()).x()<<","<<mapToGlobal(sliderHandle.topLeft()).y();
	//kDebug() << "Volume is "<<percent;
}

