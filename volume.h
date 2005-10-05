//-*-C++-*-
/*
 * KMix -- KDE's full featured mini mixer
 *
 * Copyright Christian Esken <esken@kde.org>
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
#ifndef VOLUME_H
#define VOLUME_H

#include <fstream>

#include <kdebug.h>

class Volume
{
 public:
  enum ChannelMask { MNONE     = 0,
                     MLEFT     = 1, MRIGHT     =   2, MCENTER =  4,
                     MMAIN     = 3, MFRONT    = 7,
                     MREARLEFT = 8, MREARRIGHT =  16, MWOOFER = 32,
                     MREAR     = 56,
                     MLEFTREC  = 64, MRIGHTREC = 128,
                     MREC      =192,
                     MCUSTOM1  =256, MCUSTOM2  = 512,
                     MALL=65535 };


 enum ChannelID { CHIDMIN  = 0,
                  LEFT     = 0, RIGHT     = 1, CENTER = 2,
                  REARLEFT = 3, REARRIGHT = 4, WOOFER = 5,
                  LEFTREC  = 6, RIGHTREC  = 7,
                  CUSTOM1  = 8, CUSTOM2   = 9, CHIDMAX  = 9 };


  Volume( ChannelMask chmask = MALL, long maxVolume = 100, long minVolume=0 );
  Volume( ChannelMask chmask, long maxVolume, long minVolume, long maxVolumeRec, long minVolumeRec );
  Volume( const Volume &v );
  Volume( int channels, long maxVolume );



  // Set all volumes as given by vol
  void setAllVolumes(long vol);
  // Set all volumes to the ones given in vol
  void setVolume(const Volume &vol );
  // Set volumes as specified by the channel mask
  void setVolume( const Volume &vol, ChannelMask chmask);
  void setVolume( ChannelID chid, long volume);

  long getVolume(ChannelID chid);
  long getAvgVolume(ChannelMask chmask);
  long getTopStereoVolume(ChannelMask chmask);
  long operator[](int);
  long maxVolume();
  long minVolume();
  int  count();

  void setMuted( bool val ) { _muted = val; };
  bool isMuted()      { return _muted; };

  friend std::ostream& operator<<(std::ostream& os, const Volume& vol);
  friend kdbgstream& operator<<(kdbgstream& os, const Volume& vol);

    // _channelMaskEnum[] and the following elements moved to public seection. operator<<() could not
    // access it, when private. Strange, as operator<<() is declared friend.
  static int _channelMaskEnum[10];
  bool          _muted;
  long          _chmask;
  long          _volumes[CHIDMAX+1];
  long          _maxVolume;
  long          _minVolume;
  long          _maxVolumeRec;
  long          _minVolumeRec;

private:
  void init( ChannelMask chmask, long, long, long, long );

  long volrange( int vol );
  long volrangeRec( int vol );
};

std::ostream& operator<<(std::ostream& os, const Volume& vol);
kdbgstream& operator<<(kdbgstream &os, const Volume& vol);

#endif // VOLUME

