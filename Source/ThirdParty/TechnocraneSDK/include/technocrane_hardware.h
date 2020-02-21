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
	bool TECHNOCRANESDK_API ComputeFocusf(float &value, const float src, const float rangeMin, const float rangeMax);
	bool TECHNOCRANESDK_API ComputeZoomf(float &value, const float src, const float rangeMin, const float rangeMax);
	bool TECHNOCRANESDK_API ComputeIrisf(float &value, const float src, const float rangeMin, const float rangeMax);

	float TECHNOCRANESDK_API ComputeHorizontalFOVf(const float height, const float aspect, const float focal_length);

	bool TECHNOCRANESDK_API ComputeFocusd(double &value, const double src, const double rangeMin, const double rangeMax);
	bool TECHNOCRANESDK_API ComputeZoomd(double &value, const double src, const double rangeMin, const double rangeMax);
	bool TECHNOCRANESDK_API ComputeIrisd(double &value, const double src, const double rangeMin, const double rangeMax);

	double TECHNOCRANESDK_API ComputeHorizontalFOVd(const double height, const double aspect, const double focal_length);

	bool TECHNOCRANESDK_API UnPackData(STechnocrane_Packet& packet, const double framerate, 
		const void* raw_data, const bool packed_data, const bool mobu_rotation);


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
		const dataReceptionStatus &GetDataReceptionStatus() const;

		//--- Opens and closes connection with data server. returns true if successful
		bool	Init(bool enable_comm_log, bool enable_packets_log, bool mobu_rotation);				//!< Initialize hardware.
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

		void SetHardwareRate(const float rate);
		float GetHardwareRate() const;

		void GetRawRotation(float& pan, float& tilt, float& roll);
		void	GetRotation(float* r, const ERotationOrder order, const float* mult);
		void	GetPosition(float* p, const float scale, const bool righthanded);

		bool GetFocus(float &value, const float rangeMin, const float rangeMax);
		bool GetZoom(float &value, const float rangeMin, const float rangeMax);
		bool GetIris(float &value, const float rangeMin, const float rangeMax);

		float GetTimeCodeRate() const;

		bool		HasTimeCode() const;
		bool		GetCameraOn() const;
		bool		GetRunning() const;
		bool		IsZoomCalibrated() const;
		bool		IsFocusCalibrated() const;
		bool		IsIrisCalibrated() const;

		//
		// track last error

		const int			GetLastError() const;
		void				ClearLastError();

		// IO

		bool StartDataRecording();
		bool StopDataRecording();

		bool SaveRecordedData(const char *filename);

		int GetNumberOfRecordedPackets() const;
		const void* GetRawPacketData(const int index) const;

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

