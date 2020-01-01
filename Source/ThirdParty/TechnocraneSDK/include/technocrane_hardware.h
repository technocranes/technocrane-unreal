#pragma once

//////////////////////////////////////////////////////////////////
// Copyright (c) 2018-2020 Technocrane s.r.o. 
//
// Technocrane SDK hardware device, communication class
// Sergei <Neill3d> Solokhin 2018-2020
//////////////////////////////////////////////////////////////////

#include "technocrane_types.h"

namespace NTechnocrane
{
	// helper function for a manual calibration
	bool TECHNOCRANESDK_API ComputeFocus(float &value, const float src, const float rangeMin, const float rangeMax);
	bool TECHNOCRANESDK_API ComputeZoom(float &value, const float src, const float rangeMin, const float rangeMax);
	bool TECHNOCRANESDK_API ComputeIris(float &value, const float src, const float rangeMin, const float rangeMax);

	// Log output
	typedef void(*LOG_CALLBACK)(const char* message, const int level);
	void TECHNOCRANESDK_API SetLogCallback(LOG_CALLBACK	log_callback);

	/////////////////////////////////////////////////////////////////////////////////////////////
	//! Technocrane hardware.
	class TECHNOCRANESDK_API CTechnocrane_Hardware
	{
	public:
		//! Constructor.
		CTechnocrane_Hardware();

		//! Destructor.
		virtual ~CTechnocrane_Hardware();

		// set options

		const SAvaliablePortList		GetAvaliablePortList();
		const SOptions					GetOptions() const;

		// run-time status info
		const SStatusInfo GetStatusInfo() const;

		// communication info

		//--- Opens and closes connection with data server. returns true if successful
		bool	Init();				//!< Initialize hardware.
		bool	Open(const SOptions& options);				//!< Open the connection.
		bool	Close();			//!< Close connection.
		bool	Done();				//!< Close down hardware.

		// Streaming functions
		//	Once connection is established and information packets have been received
		//	these functions start and stops the data streaming
		bool	StartDataStream();					//!< Start data streaming.
		bool	StopDataStream();					//!< Stop data streaming.

		bool	IsReady() const;

		//--- Packet query

		// packet contains 
		int	FetchDataPacket(STechnocrane_Packet& packet, size_t localReadIndex, size_t &localReadCount, const bool packed_data);

		//
		// track last error

		const int			GetLastError() const;
		void				ClearLastError();

	protected:
		void*			impl;
	};

};

//
//

TECHNOCRANESDK_CAPI int32_t StartStream(const bool use_network_connection, const int32_t port_id);
TECHNOCRANESDK_CAPI int32_t StopStream();

TECHNOCRANESDK_CAPI void*  MapPacket();
TECHNOCRANESDK_CAPI void  UnMapPacket();
