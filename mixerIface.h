#ifndef __MIXER_IFACE_H
#define __MIXER_IFACE_H

#include <dcopobject.h>

class MixerIface : virtual public DCOPObject
{
   K_DCOP

k_dcop:
   /**
    Sets the volume of the device with index deviceidx to the percentage
    specified in the second parameter. The deviceidx is got from the array
    at the start of mixer_oss.cpp .
    */
   virtual void setVolume( int deviceidx, int percentage )=0;
   /**
    A simpler way to access the master volume (which is deviceidx 0).
    */
   virtual void setMasterVolume( int percentage )=0;

   /**
    Increase the volume of the given device by a 5% .
    */
   virtual void increaseVolume( int deviceidx )=0;
   /**
    Decrease the volume of the given device by a 5% .
    */
   virtual void decreaseVolume( int deviceidx )=0;

   /**
    Returns the volume of the device (as a percentage, 0..100).
    */
   virtual int volume( int deviceidx )=0;
   /**
    Returns the volume of the master device (as a percentage, 0..100).
    */
   virtual int masterVolume()=0;

   /**
    Mutes or unmutes the specified device.
    */
   virtual void setMute( int deviceidx, bool on )=0;
   /**
    Returns if the given device is muted or not. If the device is not
    available in this mixer, it is reported as muted.
    */
   virtual bool mute( int deviceidx )=0;
   
   /**
    Makes the given device a record source.
    */
   virtual void setRecordSource( int deviceidx, bool on )=0;
   /**
    Returns if the given device is a record source.
    */
   virtual bool isRecordSource( int deviceidx )=0;
	       
   /**
    Returns true if the given device is available in the current mixer
    and false if it's not.
    */
   virtual bool isAvailableDevice( int deviceidx )=0;

   /**
    Returns the name of the mixer.
    */
   virtual QString mixerName()=0;

};

#endif
