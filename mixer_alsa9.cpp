/*
 *              KMix -- KDE's full featured mini mixer
 *              Alsa 0.9x and 1.0 - Based on original alsamixer code
 *              from alsa-project ( www/alsa-project.org )
 *
 *
 * Copyright (C) 2002 Helio Chissini de Castro <helio@conectiva.com.br>
 *               2004 Christian Esken <esken@kde.org>
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
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

// STD Headers
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

extern "C"
{
	#include <alsa/asoundlib.h>
}

// KDE Headers
#include <klocale.h>
#include <kdebug.h>

// Local Headers
#include "mixer_alsa.h"
#include "volume.h"
// #define if you want MUCH debugging output
//#define ALSA_SWITCH_DEBUG
//#define KMIX_ALSA_VOLUME_DEBUG

// @todo Add file descriptor based notifying for seeing changes

Mixer*
ALSA_getMixer( int device, int card )
{
	Mixer *l_mixer;
	l_mixer = new Mixer_ALSA( device, card );
	l_mixer->setupMixer();
	return l_mixer;
}

Mixer*
ALSA_getMixerSet( MixSet set, int device, int card )
{
	Mixer *l_mixer;
	l_mixer = new Mixer_ALSA( device, card );
	l_mixer->setupMixer( set );
	return l_mixer;
}

Mixer_ALSA::Mixer_ALSA( int device, int card ) :
	Mixer( device, card ), handle(0)
{
	masterChosen = false;
}

Mixer_ALSA::~Mixer_ALSA()
{
}

int
Mixer_ALSA::identify( snd_mixer_selem_id_t *sid )
{
	QString name = snd_mixer_selem_id_get_name( sid );

	if ( name == "Master" )
	{
		if (!masterChosen) {
			m_masterDevice = snd_mixer_selem_id_get_index( sid );
			masterChosen = true; // -<- this makes KMix select the *first* master device
		}
		return MixDevice::VOLUME;
	}
	if ( name == "Master Mono" ) return MixDevice::VOLUME;
	if ( name.find( "Headphone", 0, false ) != -1 ) return MixDevice::HEADPHONE;
	if ( name == "Bass" ) return MixDevice::BASS;
	if ( name == "Treble" ) return MixDevice::TREBLE;
	if ( name == "CD" ) return MixDevice::CD;
	if ( name == "Video" ) return MixDevice::VIDEO;
	if ( name == "PCM" || name == "Wave" || name == "Line" )	return MixDevice::AUDIO;
	if ( name == "Surround" ) return MixDevice::SURROUND_BACK;
	if ( name == "Center" ) return MixDevice::SURROUND_CENTERFRONT;
	if ( name.find( "surround", 0, false ) != -1 ) return MixDevice::SURROUND;
	if ( name.find( "ac97", 0, false ) != -1 ) return MixDevice::AC97;
	if ( name.find( "coaxial", 0, false ) != -1 ) return MixDevice::DIGITAL;
	if ( name.find( "optical", 0, false ) != -1 ) return MixDevice::DIGITAL;
	if ( name.find( "IEC958", 0, false ) != -1 ) return MixDevice::DIGITAL;
	if ( name.find( "Mic" ) != -1 ) return MixDevice::MICROPHONE;
	if ( name.find( "LFE" ) != -1 ) return MixDevice::SURROUND_LFE;
	if ( name.find( "3D", 0, false ) != -1 ) return MixDevice::SURROUND;  // Should be probably some own icon

	return MixDevice::EXTERNAL;
}

int
Mixer_ALSA::openMixer()
{
	bool virginOpen = m_mixDevices.isEmpty();
	bool validDevice = false;
	int err;

	release();

	snd_ctl_t *ctl_handle;
	snd_ctl_card_info_t *hw_info;
	snd_ctl_card_info_alloca(&hw_info);

	snd_mixer_elem_t *elem;
	snd_mixer_selem_id_t *sid;
	snd_mixer_selem_id_alloca( &sid );

	// Card information
	QString devName;
	if ( (unsigned)m_devnum > 31 )
	{
		devName = "default";
	}
	else
	{
		devName = QString( "hw:%1" ).arg( m_devnum );
	}

	if (virginOpen)
		kdDebug(67100) << "Trying ALSA Device " << devName << endl;

	if ( ( err = snd_ctl_open ( &ctl_handle, devName.latin1(), 0 ) ) < 0 )
	{
	    kdError(67100) << "snd_ctl_open err=" << snd_strerror(err) << endl;
	    //errormsg( Mixer::ERR_OPEN );
	    return Mixer::ERR_OPEN;
	}

	if ( ( err = snd_ctl_card_info ( ctl_handle, hw_info ) ) < 0 )
	{
	    kdError(67100) << "snd_ctl_card_info err=" << snd_strerror(err) << endl;
	    //errormsg( Mixer::ERR_READ );
	    snd_ctl_close( ctl_handle );
	    return Mixer::ERR_READ;
	}

	// Device and mixer names
	mixer_card_name =  snd_ctl_card_info_get_name( hw_info );
	mixer_device_name = snd_ctl_card_info_get_mixername( hw_info );

	snd_ctl_close( ctl_handle );

	// release mixer before (re-)opening
	release();

	/* open mixer device */
	if ( ( err = snd_mixer_open ( &handle, 0 ) ) < 0 )
	{
	    kdError(67100) << "snd_mixer_open err=" << snd_strerror(err) << endl;
	    //errormsg( Mixer::ERR_OPEN );
	    return Mixer::ERR_OPEN;
	}

	if ( ( err = snd_mixer_attach ( handle, devName.latin1() ) ) < 0 )
	{
	    kdError(67100) << "snd_mixer_attach err=" << snd_strerror(err) << endl;
	    //errormsg( Mixer::ERR_PERM );
	    return Mixer::ERR_OPEN;
	}

	if ( ( err = snd_mixer_selem_register ( handle, NULL, NULL ) ) < 0 )
	{
	    kdError(67100) << "snd_mixer_selem_register err=" << snd_strerror(err) << endl;
	    //errormsg( Mixer::ERR_READ );
	    return Mixer::ERR_READ;
	}

	if ( ( err = snd_mixer_load ( handle ) ) < 0 )
	{
                kdError(67100) << "snd_mixer_load err=" << snd_strerror(err) << endl;
		//errormsg( Mixer::ERR_READ );
		releaseMixer();
		return Mixer::ERR_READ;
	}

	// default mixers?
	if( m_cardnum == -1 )
	{
		m_cardnum = 0;
	}

	if( m_devnum == -1 )
	{
		m_devnum = 0;
	}
	
	int selem_count = snd_mixer_get_count(handle);
	mixerIDs = new unsigned int[selem_count];

	unsigned int mixerIdx = 0;
	for ( elem = snd_mixer_first_elem( handle ); elem; elem = snd_mixer_elem_next( elem ), mixerIdx++ )
	{
		// If element is not active, just skip
		if ( ! snd_mixer_selem_is_active ( elem ) ) {
			continue;
		}

		sid = (snd_mixer_selem_id_t*)malloc(snd_mixer_selem_id_sizeof());  // I believe *we* must malloc it for ourself
		snd_mixer_selem_get_id( elem, sid );

		bool canRecord = false;
		bool hasMute = false;
		long maxVolume, minVolume;
		validDevice = true;

		if ( snd_mixer_selem_has_capture_switch( elem ) )
		{
			canRecord = true;
		}

		snd_mixer_selem_get_playback_volume_range( elem, &minVolume, &maxVolume );

		// New mix device
		MixDevice::ChannelType ct = (MixDevice::ChannelType)identify( sid );

		if( virginOpen )
		{
			int chn = 1; // Assuming default mono

			MixDevice::DeviceCategory cc;

			if(	! snd_mixer_selem_is_capture_mono( elem )  ||
					! snd_mixer_selem_is_playback_mono( elem ) )
				chn = 2; // Stereo channel ?

			/* !!!! The next line looked like this:   !!!! Other mixer_* implementations might be buggy !!!!
			   Volume vol( chn, ( int )maxVolume );
			   I wonder why it has ever worked.
			   Probably by accident because the Volume objects were copied so much until KMix2.0 (inclusive)
			*/
			Volume* vol = new Volume( chn, ( int )maxVolume );
			//mixer_elem_list.append( elem );
			mixer_sid_list.append( sid );

			if ( snd_mixer_selem_has_playback_volume ( elem ) ||
					snd_mixer_selem_has_capture_volume ( elem ) )
			{
				cc = MixDevice::SLIDER;
				if ( snd_mixer_selem_has_playback_switch ( elem ) ||
						snd_mixer_selem_has_capture_switch ( elem ) )
					hasMute = true;    // !! I do not like the *_has_capture_switch()  here - cesken
			}
			else if ( ! snd_mixer_selem_has_playback_volume ( elem ) ||
						snd_mixer_selem_has_capture_volume ( elem ) )
			{
				cc = MixDevice::SWITCH;  // !! Why is something with *_has_capture_volume()  a switch? - cesken
				hasMute = true; // The mute button act as switch in this case
			}
			else {
				continue;
			}

			mixerIDs[mixerIdx] = mixerIdx; // -<- Remove this array again!!         // was: currentID;
			MixDevice* mdw =
			    new MixDevice( mixerIdx,
					   *vol,
					   canRecord,
					   hasMute,
					   snd_mixer_selem_id_get_name( sid ),
					   ct,
					   cc );
			m_mixDevices.append( mdw );
			//kdDebug(67100) << "ALSA create MDW, vol= " << *vol << endl;
			delete vol;
		}
		else
		{
			MixDevice* md = m_mixDevices.at( mixerIdx );
			if( !md )
			{
				return ERR_INCOMPATIBLESET;
			}
			writeVolumeToHW( mixerIdx, md->getVolume() );
		}

	}

	//return error for invalid devices
	if ( !validDevice )
	{
		return Mixer::ERR_NODEV;
	}

	// Copy the name of kmix mixer from card name
	// Real name of mixer is not too good
	m_mixerName = mixer_card_name;

	// return with success
	m_isOpen = true;

	return 0;
}


int
Mixer_ALSA::releaseMixer()
{
	int ret = snd_mixer_close( handle );
	return ret;
}


snd_mixer_elem_t* Mixer_ALSA::getMixerElem(int devnum) {
	snd_mixer_elem_t* elem = 0;
	if ( int( mixer_sid_list.count() ) > devnum ) {
		snd_mixer_selem_id_t * sid = mixer_sid_list[ devnum ];
		elem = snd_mixer_find_selem(handle, sid);

		if ( elem == 0 ) {
			kdDebug(67100) << "Error finding mixer element " << devnum << endl;
		}
	}
	return elem;
//	return mixer_elem_list[ devnum ];
}

bool Mixer_ALSA::prepareUpdate() {
    //kdDebug(67100) << "Mixer_ALSA::prepareUpdate() 1\n";
    bool updated = false;
    struct pollfd  *fds;
    unsigned short revents;
    int count, err;

/* setup for select on stdin and the mixer fd */
    if ((count = snd_mixer_poll_descriptors_count(handle)) < 0) {
	kdDebug(67100) << "Mixer_ALSA::poll() , snd_mixer_poll_descriptors_count() err=" <<  count << "\n";
	return false;
    }

    //kdDebug(67100) << "Mixer_ALSA::prepareUpdate() 2\n";
    
    fds = (struct pollfd*)calloc(count + 1, sizeof(struct pollfd));
    if (fds == NULL) {
	kdDebug(67100) << "Mixer_ALSA::poll() , calloc() = null" << "\n";
	return false;
    }

    fds->events = POLLIN;
    if ((err = snd_mixer_poll_descriptors(handle, fds, count)) < 0) {
	kdDebug(67100) << "Mixer_ALSA::poll() , snd_mixer_poll_descriptors_count() err=" <<  err << "\n";
	return false;
    }
    if (err != count) {
	kdDebug(67100) << "Mixer_ALSA::poll() , snd_mixer_poll_descriptors_count() err=" << err << " count=" <<  count << "\n";
	    return false;
    }

    // Poll on fds with 10ms timeout
    // Hint: alsamixer has an infinite timeout, but we cannot do this because we would block
    // the X11 event handling (Qt event loop) with this.
    //kdDebug(67100) << "Mixer_ALSA::prepareUpdate() 3\n";
    int finished = poll(fds, count, 10);
    //kdDebug(67100) << "Mixer_ALSA::prepareUpdate() 4\n";

    if (finished > 0) {
    //kdDebug(67100) << "Mixer_ALSA::prepareUpdate() 5\n";

	if (snd_mixer_poll_descriptors_revents(handle, fds, count, &revents) >= 0) {
    //kdDebug(67100) << "Mixer_ALSA::prepareUpdate() 6\n";


	    if (revents & POLLNVAL) {
		kdDebug(67100) << "Mixer_ALSA::poll() , Error: poll() returns POLLNVAL\n";
		return false;
	    }
	    if (revents & POLLERR) {
		kdDebug(67100) << "Mixer_ALSA::poll() , Error: poll() returns POLLERR\n";
		return false;
	    }
	    if (revents & POLLIN) {
    //kdDebug(67100) << "Mixer_ALSA::prepareUpdate() 7\n";

		snd_mixer_handle_events(handle);
                updated = true;
	    }
	}
    }

    //kdDebug(67100) << "Mixer_ALSA::prepareUpdate() 8\n";


    // !!! memory leak under any error condition
    free(fds);
    return updated;
}

bool
Mixer_ALSA::isRecsrcHW( int devnum )
{
	bool isCurrentlyRecSrc = false;
	snd_mixer_elem_t *elem = getMixerElem( devnum );

	if ( !elem ) {
		return false;
	}

	if ( snd_mixer_selem_has_capture_switch( elem ) ) {
		// Has a on-off switch
		// Yes, this element can be record source. But the user can switch it off, so lets see if it is switched on.
		int swLeft;
		int ret = snd_mixer_selem_get_capture_switch( elem, SND_MIXER_SCHN_FRONT_LEFT, &swLeft );
                if ( ret != 0 ) {
                        kdDebug(67100) << "snd_mixer_selem_get_capture_switch() failed 1\n";
                }

		if (snd_mixer_selem_has_capture_switch_joined( elem ) ) {
			isCurrentlyRecSrc = (swLeft != 0);
#ifdef ALSA_SWITCH_DEBUG
			kdDebug(67100) << "Mixer_ALSA::isRecsrcHW() has_switch joined: #" << devnum << " >>> " << swLeft << " : " << isCurrentlyRecSrc << endl;
#endif
		}
		else {
			int swRight;
			snd_mixer_selem_get_capture_switch( elem, SND_MIXER_SCHN_FRONT_RIGHT, &swRight );
			isCurrentlyRecSrc = ( (swLeft != 0) || (swRight != 0) );
#ifdef ALSA_SWITCH_DEBUG
			kdDebug(67100) << "Mixer_ALSA::isRecsrcHW() has_switch non-joined, state " << isCurrentlyRecSrc << endl;
#endif
		}
	}
	else {
		// Has no on-off switch
		if ( snd_mixer_selem_has_capture_volume( elem ) ) {
			// Has a volume, but has no OnOffSwitch => We assume that this is a fixed record source (always on). (esken)
			isCurrentlyRecSrc = true;
#ifdef ALSA_SWITCH_DEBUG
			kdDebug(67100) << "Mixer_ALSA::isRecsrcHW() has_no_switch, state " << isCurrentlyRecSrc << endl;
#endif
		}
	}

	return isCurrentlyRecSrc;
}

bool
Mixer_ALSA::setRecsrcHW( int devnum, bool on )
{
	int sw = 0;
	if (on)
		sw = !sw;

	snd_mixer_elem_t *elem = getMixerElem( devnum );
	if ( !elem )
	{
		return 0;
	}

	if (snd_mixer_selem_has_capture_switch_joined( elem ) )
	{
		int before, after;
		int ret = snd_mixer_selem_get_capture_switch( elem, SND_MIXER_SCHN_FRONT_LEFT, &before );
		if ( ret != 0 ) {
			kdDebug(67100) << "snd_mixer_selem_get_capture_switch() failed 1\n";
		}
	
		ret = snd_mixer_selem_set_capture_switch_all( elem, sw );
                if ( ret != 0 ) {
                        kdDebug(67100) << "snd_mixer_selem_set_capture_switch_all() failed 2: errno=" << ret << "\n";
                }
		ret = snd_mixer_selem_get_capture_switch( elem, SND_MIXER_SCHN_FRONT_LEFT, &after );
                if ( ret != 0 ) {
                        kdDebug(67100) << "snd_mixer_selem_get_capture_switch() failed 3: errno=" << ret << "\n";
                }

#ifdef ALSA_SWITCH_DEBUG
		kdDebug(67100) << "Mixer_ALSA::setRecsrcHW(" << devnum <<  "," << on << ")joined. Before=" << before << " Set=" << sw << " After=" << after <<"\n";
#endif
		
	}
	else
	{
		if ( snd_mixer_selem_has_capture_channel( elem, SND_MIXER_SCHN_FRONT_LEFT ) )
		{
#ifdef ALSA_SWITCH_DEBUG
			kdDebug(67100) << "Mixer_ALSA::setRecsrcHW LEFT\n";
#endif
			snd_mixer_selem_set_capture_switch( elem, SND_MIXER_SCHN_FRONT_LEFT, sw );
		}

		if ( snd_mixer_selem_has_capture_channel(elem, SND_MIXER_SCHN_FRONT_RIGHT ) )
		{
#ifdef ALSA_SWITCH_DEBUG
			kdDebug(67100) << "Mixer_ALSA::setRecsrcHW RIGHT\n";
#endif
			snd_mixer_selem_set_capture_switch(elem, SND_MIXER_SCHN_FRONT_RIGHT, sw);
		}
	}

#ifdef ALSA_SWITCH_DEBUG
	kdDebug(67100) << "EXIT Mixer_ALSA::setRecsrcHW(" << devnum << "," << on <<  ")\n";
#endif
	return false; // we should always return false, so that other devnum's get updated
}

int
Mixer_ALSA::readVolumeFromHW( int mixerIdx, Volume &volume )
{
	int elem_sw;
	bool hasVol = false;
	long left, right;

	snd_mixer_elem_t *elem = getMixerElem( mixerIdx );

	if ( !elem )
	{
		return 0;
	}
		

	hasVol = ( snd_mixer_selem_has_playback_volume ( elem ) ||
			snd_mixer_selem_has_capture_volume ( elem ) );

	if ( hasVol )
	{
                bool usePlaybackVolume = snd_mixer_selem_has_playback_volume ( elem );
		
		// Read value from LEFT playback/capture volume
		if ( usePlaybackVolume )
		{
			int ret = snd_mixer_selem_get_playback_volume( elem, SND_MIXER_SCHN_FRONT_LEFT, &left );
#ifdef KMIX_ALSA_VOLUME_DEBUG
			kdDebug(67100) << "readVolumeFromHW(" << mixerIdx << ") [has_playback_volume,L] = " << left << endl;
#endif

			if ( ret != 0 ) kdDebug(67100) << "readVolumeFromHW(" << mixerIdx << ") [has_playback_volume,L] failed, errno=" << ret << endl;
		}
		else
		{
			int ret = snd_mixer_selem_get_capture_volume ( elem, SND_MIXER_SCHN_FRONT_LEFT, &left );
#ifdef KMIX_ALSA_VOLUME_DEBUG
			kdDebug(67100) << "readVolumeFromHW(" << mixerIdx << ") [has_capture_volume,L] = " << left << endl;
#endif
			if ( ret != 0 ) kdDebug(67100) << "readVolumeFromHW(" << mixerIdx << ") [has_capture_volume,L] failed, errno=" << ret << endl;			
		}

		// Is Mono channel ?  !! What if we only have capture volume? Does snd_mixer_selem_is_PLAYBACK_mono() apply?
		// Trying the following temporary fix.
		// (the real solution is to support both playback and capture volumes with 0-2 sliders)
		if ( ( usePlaybackVolume && snd_mixer_selem_is_playback_mono ( elem )) ||
		     (!usePlaybackVolume && snd_mixer_selem_is_capture_mono  ( elem ))   )
		{
#ifdef KMIX_ALSA_VOLUME_DEBUG
			kdDebug(67100) << "Is mono volume: " << left << endl;
#endif
			volume.setAllVolumes( left );
		}
		else
		{
			// Read value from RIGHT playback/capture volume
			if ( usePlaybackVolume )
			{
				int ret = 	snd_mixer_selem_get_playback_volume( elem, SND_MIXER_SCHN_FRONT_RIGHT, &right );
#ifdef KMIX_ALSA_VOLUME_DEBUG
			kdDebug(67100) << "readVolumeFromHW(" << mixerIdx << ") [has_playback_volume,R] = " << right << endl;
#endif
				if ( ret != 0 ) kdDebug(67100) << "readVolumeFromHW(" << mixerIdx << ") [has_playback_volume,R] failed, errno=" << ret << endl;
			}
			else {
				int ret = snd_mixer_selem_get_capture_volume( elem, SND_MIXER_SCHN_FRONT_RIGHT, &right );
#ifdef KMIX_ALSA_VOLUME_DEBUG
			kdDebug(67100) << "readVolumeFromHW(" << mixerIdx << ") [has_capture_volume,R] = " << right << endl;
#endif
				if ( ret != 0 ) kdDebug(67100) << "readVolumeFromHW(" << mixerIdx << ") [has_capture_volume,R] failed, errno=" << ret << endl;			
			}

			volume.setVolume( Volume::RIGHT, right );
			volume.setVolume( Volume::LEFT, left );
		}
	}
	else {
		//kdDebug(67100) << "readVolumeFromHW(" << mixerIdx << ") has no volume\n";
	}

	if ( snd_mixer_selem_has_playback_switch( elem ) )
	{
		snd_mixer_selem_get_playback_switch( elem, SND_MIXER_SCHN_FRONT_LEFT, &elem_sw );
		if( elem_sw == 0 )
			volume.setMuted(true);
		else
			volume.setMuted(false);
	}
	else if (  snd_mixer_selem_has_capture_switch(  elem ) )
	{
		snd_mixer_selem_get_capture_switch( elem, SND_MIXER_SCHN_FRONT_LEFT, &elem_sw );
		if( elem_sw == 0 )
			volume.setMuted(true);
		else
			volume.setMuted(false);

	}
/* I will try this after KDE3.2 - esken !!
	// The next line is a nice workaround for channels that have no explicite "muted" switch (esken)
	else if ( (left == pmin ) &&
				( (right == pmin) || snd_mixer_selem_is_playback_mono ( elem ) ) )
	{
		volume.setMuted( ! elem_sw );
	}
*/

	//kdDebug(67100) << "EXIT Mixer_ALSA::readVolumeFromHW()\n";
	return 0;
}

int
Mixer_ALSA::writeVolumeToHW( int devnum, Volume& volume )
{
	int left, right;
	int elem_sw;

	Volume data = volume; // !! Variable called "data" is never used - cesken
	snd_mixer_elem_t *elem = getMixerElem( devnum );

	if ( !elem )
	{
		return 0;
	}

	left = volume[ Volume::LEFT ];
	right = volume[ Volume::RIGHT ];

	if (snd_mixer_selem_has_playback_volume( elem ) )
	{
		snd_mixer_selem_set_playback_volume ( elem, SND_MIXER_SCHN_FRONT_LEFT, left );
		if ( ! snd_mixer_selem_is_playback_mono ( elem ) )
			snd_mixer_selem_set_playback_volume ( elem, SND_MIXER_SCHN_FRONT_RIGHT, right );
	}
	else if ( snd_mixer_selem_has_capture_volume( elem ) )
	{
		snd_mixer_selem_set_capture_volume ( elem, SND_MIXER_SCHN_FRONT_LEFT, left );
		if ( ! snd_mixer_selem_is_playback_mono ( elem ) )
			snd_mixer_selem_set_capture_volume ( elem, SND_MIXER_SCHN_FRONT_RIGHT, right );
	}

/* !! will try to remove this for test later - esken */
	if ( snd_mixer_selem_has_playback_switch( elem ) )
	{
		snd_mixer_selem_get_playback_switch( elem, SND_MIXER_SCHN_FRONT_LEFT, &elem_sw );
		if( elem_sw == volume.isMuted() )
			snd_mixer_selem_set_playback_switch_all( elem, ! elem_sw );
	}
	else if (  snd_mixer_selem_has_capture_switch(  elem ) )
	{
		snd_mixer_selem_get_capture_switch( elem, SND_MIXER_SCHN_FRONT_LEFT, &elem_sw );
		if( elem_sw == volume.isMuted() )
			snd_mixer_selem_set_capture_switch_all( elem, ! elem_sw );
	}
	return 0;
}

QString
Mixer_ALSA::errorText( int mixer_error )
{
	QString l_s_errmsg;
	switch ( mixer_error )
	{
		case ERR_PERM:
			l_s_errmsg = i18n("You do not have permission to access the alsa mixer device.\n" \
					"Please verify if all alsa devices are properly created.");
      break;
		case ERR_OPEN:
			l_s_errmsg = i18n("Alsa mixer cannot be found.\n" \
					"Please check that the soundcard is installed and the\n" \
					"soundcard driver is loaded.\n" );
			break;
		default:
			l_s_errmsg = Mixer::errorText( mixer_error );
	}
	return l_s_errmsg;
}

bool Mixer_ALSA::hasBrokenRecSourceHandling() {
    // Only for the current Mixer_ALSA implementation.
    // This implementation does not see changes from the Mixer Hardware to the Record Sources.
    // So the workaround is to manually call md.setRecSrc(false) for all aother channels.
    // Fixed now :-)))
    return false;
}

QString
ALSA_getDriverName()
{
	return "ALSA";
}


