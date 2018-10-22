#pragma once

//////////////////////////////////////////////////////////////////
// Copyright (c) 2018 Technocrane s.r.o. 
//
// Technocrane SDK hardware device, communication class
// Sergei <Neill3d> Solokhin 2018
//////////////////////////////////////////////////////////////////

#include "technocrane_types.h"

namespace NTechnocrane
{
	// helper function for a manual calibration
	bool TECHNOCRANESDK_API ComputeFocus(double &value, const double src, const double rangeMin, const double rangeMax);
	bool TECHNOCRANESDK_API ComputeZoom(double &value, const double src, const double rangeMin, const double rangeMax);
	bool TECHNOCRANESDK_API ComputeIris(double &value, const double src, const double rangeMin, const double rangeMax);

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
		int	FetchDataPacket(STechnocrane_Packet& packet, size_t localReadIndex, size_t &localReadCount);

		//
		// track last error

		const int			GetLastError() const;
		void				ClearLastError();

	protected:
		void*			impl;
	};

};
