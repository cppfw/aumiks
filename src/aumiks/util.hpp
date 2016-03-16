/**
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once



namespace aumiks{



/**
 * @brief Returns frame size for given sound output format.
 * Returns number of bytes per frame for given sound output format.
 * The sound frame is the sound data for a single sampling rate tick.
 * Each frame consists of number of channels samples (e.g. mono: 1 frame = 1 sample, stereo: 1 frame = 2 samples).
 * @param chans - number of channels. 1 = mono, 2 = stereo, etc.
 * @return the size of the frame in bytes.
 */
inline unsigned BytesPerOutputFrame(unsigned chans){
	return 2 * chans; //we only support 16 bits per sample
}



//TODO:



}//~namespace
