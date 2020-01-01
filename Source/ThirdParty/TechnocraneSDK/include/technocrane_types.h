#pragma once

///////////////////////////////////////////////////////////////////
// Copyright (c) 2020 Technocrane s.r.o. 
//
// Technocrane SDK Types declaration
// Sergei <Neill3d> Solokhin 2018-2020
///////////////////////////////////////////////////////////////////

#include <vector>
#include <string>

#ifdef TECHNOCRANESDK_EXPORTS
#define TECHNOCRANESDK_API	__declspec(dllexport)
#define TECHNOCRANESDK_CAPI	extern "C" __declspec(dllexport)
#else
#define TECHNOCRANESDK_API  __declspec(dllimport)
#define TECHNOCRANESDK_CAPI  __declspec(dllimport)
#endif

namespace NTechnocrane
{

	typedef std::pair<std::string, unsigned int>	port_pair;

#define		TECHNOCRANE_HELP			"Please visit Technocrane homepage - http://www.supertechno.com/"
#define		TECHNOCRANE_ABOUT			"TECHNOCRANE s.r.o.\n Developed by Sergei <Neill3d> Solokhin 2018-2020"

#define		TECHNOCRANE_NO_ERROR			0
#define		TECHNOCRANE_NO_PORTS			1
#define		TECHNOCRANE_NO_PORTS_MSG		"Is Data Cabel Connected? Do you want to try again ?"
#define		TECHNOCRANE_SERIAL_FAILED		2
#define		TECHNOCRANE_SERIAL_FAILED_MSG	"Failed to Activate Serial Port"
#define		TECHNOCRANE_NETWORK_FAILED		3
#define		TECHNOCRANE_NETWORK_FAILED_MSG	"Failed to Start A Network Communication"

#define		default_port	1
#define		default_fps		25.0f	// PAL

	////////////////////////////////////////////////////////////////////////////////
	// SAvaliablePortList

	struct TECHNOCRANESDK_API SAvaliablePortList
	{
		SAvaliablePortList(const void* _impl);

		const int GetCount() const;

		const char* GetPortName(const int index) const;
		const int GetPort(const int index) const;

	protected:
		const void*	impl;
	};

	////////////////////////////////////////////////////////////////////////////////
	// SOptions
	struct SOptions
	{
		float		m_CameraFPS;
		bool		m_UseNetworkConnection;

		// network options
		bool			m_BindAnyAddress;
		unsigned long	m_NetworkAddress;
		int				m_NetworkPort;

		// serial options
		int			m_SerialPort;
		int			m_BaudRate;
	};

	void TECHNOCRANESDK_API SetDefaultOptions(SOptions& options);

	///////////////////////////////////////////////////////////////////////////////
	// SStatusInfo
	struct SStatusInfo
	{
		double	timecode_rate;	// rate extracted from received packets
		bool	has_timecode;
		bool	is_camera_on;
		bool	is_running;
		bool	is_zoom_calibrated;
		bool	is_focus_calibrated;
		bool	is_iris_calibrated;
	};

	// DONE:
	enum ETimeRatePreset
	{
		eTimeRateCustom,
		eNTSC_DROP,	//!<  29.97f
		eNTSC_FULL,	//!< -29.97f
		ePAL_25,		//!< -25.0f
		eMPAL_30,		//!< -29.971f Currently not supported : "1" is added just to differentiate from NTSC_FULL(-29.97f)
		eFILM_24,		//!< -24.0f
		eFILM_23976,	//!< -23.976f
		eFRAMES_30,	//!< -30.0f
		eFRAMES_5994	//!< -59.94f
	};

	float TECHNOCRANESDK_API ETimeRatePresetToFloat(const ETimeRatePreset preset);


	//////////////////////////////////////////////////////////////////////////////////////////
	//! Camera packet class.
	struct TECHNOCRANESDK_API STechnocrane_Packet
	{
		//! a constructor
		STechnocrane_Packet()
		{
			Pan = 0.0f;
			Tilt = 0.0f;
			Roll = 0.0f;

			memset(Position, 0, sizeof(float) * 3);
			memset(Rotation, 0, sizeof(float) * 3);

			Iris = 0.0f;
			TrackPos = 0.0f;
			PacketNumber = 0.0f;
		
			CameraOn = false;
			Running = false;
			IsZoomCalibrated = false;	// calibrated zoom, focus, iris ?!
			IsFocusCalibrated = false;
			IsIrisCalibrated = false;
			PacketHasTimeCode = false;
			
			hours = 0;
			minutes = 0;
			seconds = 0;
			frames = 0;
			field = 0;
		}


		float		Focus;			//!< <b>Property:</b> Focus encoder value (relative).
		float		Zoom;			//!< <b>Property:</b> Zoom encoder value.
									
		float		Position[3];		//!< <b>Property:</b> Position of camera.
		float		Rotation[3];		//!< <b>Property:</b> Orientation of camera.
		float		Offset[3];			//!< <b>Property:</b> Camera offset.
		float		FieldOfView[2];	//!< <b>Property:</b> Field of View (X,Y) of camera lens.
		float		OpticalCenter[2];	//!< <b>Property:</b> Optical Center of camera lens.

		float	Pan;
		float	Tilt;
		float	Roll;

		float	Iris;
		float	TrackPos;

		// manually computed FOV
		float		ComputedFOV;

		float	PacketNumber;
		
		unsigned int	hours;
		unsigned int	minutes;
		unsigned int	seconds;
		unsigned int	frames;
		unsigned int	field;

		// control over a camera

		bool	IsZoomCalibrated;	// calibrated zoom, focus, iris ?!
		bool	IsFocusCalibrated;
		bool	IsIrisCalibrated;
		bool	PacketHasTimeCode;

		bool	CameraOn;
		bool	Running;

		
		bool HasTimeCode() const
		{
			return (true == PacketHasTimeCode);
		}
		float ComputeLocalTime(const float framerate) const
		{
			return (3600.0f * hours + 60.0f * minutes + seconds + (1.0f / framerate * frames));
		}
	};

};

