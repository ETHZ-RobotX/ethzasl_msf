/*

Copyright (c) 2012, Simon Lynen, ASL, ETH Zurich, Switzerland
You can contact the author at <slynen at ethz dot org>

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of ETHZ-ASL nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL ETHZ-ASL BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef SENSORMANAGERROS_H
#define SENSORMANAGERROS_H

#include <ros/ros.h>
#include <msf_core/MSF_CoreConfig.h>
#include <msf_core/msf_sensormanager.hpp>

#include <ros/ros.h>
#include <dynamic_reconfigure/server.h>

namespace msf_core{

typedef dynamic_reconfigure::Server<msf_core::MSF_CoreConfig> ReconfigureServer;

//abstract class defining user configurable calculations for the msf_core with ROS interfaces
struct MSF_SensorManagerROS:public msf_core::MSF_SensorManager{
protected:
	/// dynamic reconfigure config
	msf_core::MSF_CoreConfig config_;
private:
	// dynamic reconfigure
	ReconfigureServer *reconfServer_;
	typedef boost::function<void(msf_core::MSF_CoreConfig& config, uint32_t level)> CallbackType;
	std::vector<CallbackType> callbacks_;

public:

	/// registers dynamic reconfigure callbacks
	template<class T>
	void registerCallback(void(T::*cb_func)(msf_core::MSF_CoreConfig& config, uint32_t level), T* p_obj)
	{
		callbacks_.push_back(boost::bind(cb_func, p_obj, _1, _2));
	}

	MSF_SensorManagerROS(){
		reconfServer_ = new ReconfigureServer(ros::NodeHandle("~"));
		ReconfigureServer::CallbackType f = boost::bind(&MSF_SensorManagerROS::Config, this, _1, _2);
		reconfServer_->setCallback(f);
		//register dyn config list
		registerCallback(&MSF_SensorManagerROS::DynConfig, this);
	}

	~MSF_SensorManagerROS(){
		delete reconfServer_;
	}

	/// gets called by dynamic reconfigure and calls all registered callbacks in callbacks_
	void Config(msf_core::MSF_CoreConfig &config, uint32_t level)
	{
		config_ = config;
		for (std::vector<CallbackType>::iterator it = callbacks_.begin(); it != callbacks_.end(); it++)
			(*it)(config, level);
	}

	virtual void DynConfig(msf_core::MSF_CoreConfig &config, uint32_t level){
		if(level & msf_core::MSF_Core_INIT_FILTER)
		{
			init(config.scale_init);
			config.init_filter = false;
		}
	}

	virtual bool getParam_fixed_bias(){
		return config_.fixed_bias;
	}
	virtual double getParam_noise_acc(){
		return config_.noise_acc;
	}
	virtual double getParam_noise_accbias(){
		return config_.noise_accbias;
	}
	virtual double getParam_noise_gyr(){
		return config_.noise_gyr;
	}
	virtual double getParam_noise_gyrbias(){
		return config_.noise_gyrbias;
	}
	virtual double getParam_fuzzythres(){
		return 0.1;
	}

};

}
#endif /* SENSORMANAGERROS_H */