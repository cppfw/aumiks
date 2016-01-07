/**
 * @author Ivan Gagis <igagis@gmail.com>
 */



#pragma once


#include <list>


#include "Channel.hpp"



namespace aumiks{



class MixChannel : public aumiks::Channel{
	friend class aumiks::Channel;
	
	typedef std::list<std::shared_ptr<aumiks::Channel> > T_ChannelList;
	typedef T_ChannelList::iterator T_ChannelIter;
	T_ChannelList channels;//should be accessed from audio thread only
	
	std::vector<std::int32_t> smpBuf;
	
	bool isPersistent;
	
public:
	/**
	 * @brief Create a new MixChannel object.
     * @param isPersistent - true = the channel will continue to be in the playing state even
	 *                       after all the child channels have finished playing.
	 *                       flase = the channel will stop as fast as all the child channels has finished playing.
     * @return a reference to a newly creted object.
     */
	MixChannel(bool isPersistent = false) :
			isPersistent(isPersistent)
	{}
	
private:
	bool FillSmpBuf(utki::Buf<std::int32_t> buf)override;
	
	
	void MixSmpBufTo(utki::Buf<std::int32_t> buf);
	
	
	
	void RemoveChannel(const std::shared_ptr<Channel>& channel){
		for(T_ChannelIter i = this->channels.begin(); i != this->channels.end(); ++i){
			if((*i) == channel){
				this->channels.erase(i);
				return;
			}
		}
	}
	
	
public:
	virtual ~MixChannel()throw(){}
	
	//TODO: doxygen
	//playing same channel second time results in undefined behavior!
	void PlayChannel_ts(const std::shared_ptr<aumiks::Channel>& channel);
	


};


}//~namespace
