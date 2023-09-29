#pragma once

/****************************************************************************
Copyright (c) 2010 - 2022 Oscium, a Dechnia LLC

http://www.oscium.com

The above copyright notice shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

/** @file */

#include <string>
#include <vector>
#include <stdint.h>

#ifndef WiPryClarity_h
#define WiPryClarity_h

#ifdef _MSC_VER
	#pragma warning(disable : 4200)
	#pragma warning(disable : 4068)

	//#ifdef WIPRYCLARITY_EXPORTS
	//	#define WIPRYCLARITY_API __declspec(dllexport)
	//#else
	//	#define WIPRYCLARITY_API __declspec(dllimport)
	//#endif

	#define WIPRYCLARITY_API
	#define PACKED_STRUCT(name)  __pragma(pack(push, 1)) struct name __pragma(pack(pop))
	#define TYPEDEF_PACKED_STRUCT_START(name)  __pragma(pack(push, 1)) typedef struct 
	#define TYPEDEF_PACKED_STRUCT_END(name)  name __pragma(pack(pop))
	#define TYPEDEF_PACKED_UNION_START(name)  __pragma(pack(push, 1)) typedef union 
	#define TYPEDEF_PACKED_UNION_END(name)  name __pragma(pack(pop))

#elif defined(__GNUC__)
	#define WIPRYCLARITY_API
	#define PACKED_STRUCT(name) struct __attribute__((packed)) name
	#define TYPEDEF_PACKED_STRUCT_START(name)typedef struct __attribute__((packed))
	#define TYPEDEF_PACKED_STRUCT_END(name) name
	#define TYPEDEF_PACKED_UNION_START(name)typedef struct __attribute__((packed))
	#define TYPEDEF_PACKED_UNION_END(name) name

#endif

/**
* @brief Radio Tap struct.
*
* This struct contains the radio tap information for the captured beacon. This structure is present before
*   every captured beacon.
*/
TYPEDEF_PACKED_STRUCT_START(OsciumRadioTap) {
	uint32_t timestamp;					///< mactime (in 1us per count)
	uint16_t channel;					///< scan channel
	int8_t RSSI;						///< RSSI value in dBm
	uint8_t rfu[1];					///< Reserved for future use
}TYPEDEF_PACKED_STRUCT_END(OsciumRadioTap);

/**
 * @brief Struct containing beacon capture data and radio tap
 *
 * This struct contains the custom radio tap info and beacon capture data. The data field of  \link #oscium::OsciumBeaconCapture OsciumBeaconCapture \endlink is an array of OsciumBeaconCaptureData elements.
 */
TYPEDEF_PACKED_STRUCT_START(OsciumBeaconCaptureData) {

	OsciumRadioTap radiotap; ///< Radio tap info

	uint16_t dataLength; ///< Length of the beacon stored in bytes
	uint8_t rfu[2]; ///< unused byte. Reserved for future use

	uint8_t data[0]; ///< Start of the data buffer. 

}TYPEDEF_PACKED_STRUCT_END(OsciumBeaconCaptureData);

/// Header length of OsciumBeaconCaptureData struct
#define OSCIUMBEACONCAPTUREDATA_HEADER_LEN offsetof(OsciumBeaconCaptureData, data)

/**
 * @brief Captured beacons list.
 *
 * This is the struct returned from the Accessory after a beacon capture has completed. This
*   containing an array of OsciumBeaconCaptureData.
 */
TYPEDEF_PACKED_STRUCT_START(OsciumBeaconCapture) {
	uint32_t dataLength; ///< Length of data buffer in bytes
	uint8_t data[0]; ///< Start of the data buffer containing array of OsciumBeaconCaptureData
}TYPEDEF_PACKED_STRUCT_END(OsciumBeaconCapture);

/// Header length of OsciumBeaconCapture struct
#define OSCIUMBEACONCAPTURE_HEADER_LEN offsetof(OsciumBeaconCapture, data)

/**
* Oscium Namespace
*/
namespace oscium {
	class WIPRYCLARITY_API WiPryClarityDelegate;

	class WIPRYCLARITY_API WiPryClarity_pimpl;

	/**
	* This class handles communication to the WiPry Clarity Accessory.
	* @version 1.0.0
	*/
	class WIPRYCLARITY_API WiPryClarity {
	public:

		/**
		* Enumeration of error codes
		*/
		enum class ErrorCode : int32_t {
			UnknownError = -1, ///< Unknown Error
			UnableToCommunicateWithAccessory = -2, ///<Error communicating with Accessory
			UnableToAuthenticateWithAccessory = -3, ///< Error occurred while authenticating with accessory
			UnsupportedAccessory = -4 ///< Unsupported Accessory was used to communicate with the API
		};

		/// Enumeration of the types of data stream
		enum class DataType : uint8_t {
			/// rssi data in the 2.4 Ghz band
			RSSI_2_4GHZ = 0,
			/// rssi data in the 5 Ghz band
			RSSI_5GHZ = 1,
			/// rssi data in the 6E band
			RSSI_6E = 2,
			//// rssi data for both 2.4GHz and 5GHz
			RSSI_DUAL25 = 3,
		};

		/**
		* Gets the version of the API
		* @return the current api version
		*/
		static std::string getVersion();

		std::string getString();

		/**
		* Default Constructor
		*/
		WiPryClarity();

		/**
		* Destructor
		*/
		~WiPryClarity();

		/**
		* Sets the delegate
		* @param delegate WiPryClarity delegate object
		*/
		void setDelegate(WiPryClarityDelegate *delegate);

		/**
		* Get the current delegate
		* @return the current delegate object
		*/
		WiPryClarityDelegate *getDelegate();

		/**
		* Returns whether communication has started
		* @return true if the communication process has been started else false
		*/
		bool didStartCommunication();

		/**
		* Starts the communication process with the usb device and returns. If the delegate is set,
		*    upon successful connection the
		*    delegate method WiPryClarityDelegate::wipryClarityDidConnect() is called. If the connection fails,
		*    delegate method WiPryClarityDelegate::wipryClarityUnableToConnect() is called.
		* @return true if the connection has started else false. Also returns false if the
		*  communication had started prior to method being called.
		* @note Check the didStartCommunication() method to see if communication has started.
		* @note Call the endCommunication() to end the communication to the usb device even if the device is no longer present.
		*/
		bool startCommunication();

		/**
		* Ends the communication with the usb device.
		* @return true if the connection has stopped else false.
		* @note Calling the didStartCommunication() method after ending the connection will
		*  return false.
		*  @note Call this method even when the device is no longer connected to clean up the resources
		*    created in method startCommunication()
		*/
		bool endCommunication();

		/**
		* Checks if the current accessory supports the data stream
		* @param dataType data type
		* @return true if the connected accessory supports the data stream.
		* @note Only call this method if the communication has been succesfully established.
		*/
		bool doesAccessorySupportDataType(DataType dataType);

		/**
		* Starts the data stream for the specified data type and zoom settings. The data received in the
		*    stream is passed on to the delegate object set using the methods WiPryClarity::setDelegate().
		*    Enabling zoom mode allows faster scan times but the number of points received is still the same
		*    as the non zoomed points with only the points in the range [startIndex .. startIndex+width-1]
		*    being populated. Eg. If 1000 points are received in the non zoomed frame setting shouldZoom,
		*    startIndex & width to 0x01, 100, 200 resp. will still return a frame of 1000 points with points
		*    100 to 299 being populated with rssi values. The values at the other points are in valid.
		* @param dataType data type of the data stream.
		* @param shouldZoom flag to enable zoom mode.
		* @param startIndex 12 bit startIndex of the zoom. Valid if shouldZoom is non zero. Valid values are [0 - 0x0FFFFF]
		* @param width 8 bit width of the zoom. Valid if shouldZoom is non zero. Valid values are [0 - 255].
		* @return true if the data stream process has started, else false
		* @note Only call this method if the communication has been succesfully established.
		* @note Only one RSSI data stream or beacon capture can be active at a time. Starting a RSSI data stream
		*         automatically stops the currently open data stream and beacon capture
		*/
		bool startRssiData(DataType dataType, uint8_t shouldZoom, uint16_t startIndex, uint16_t width);

		/**
		* Stops the current rssi data stream
		* @return true if the data stream is stopped else false
		*/
		bool stopRssiData();

		/**
		* Gets the device serial number. Make sure the connection is established
		* @return the device's serial number
		*/
		std::string getSerialNumber();

		/**
		* Get the odd rssi limits in dBm. Make sure connection is established
		* @param min pointer to store min float value in dBm
		* @param max pointer to store max float value in dBm
		* @param noiseFloor pointer to store noise floor float value in dBm
		* @return true if the values is stored into the pointers else false
		*/
		bool getOddRssiLimts(float* min, float* max, float * noiseFloor);

		/**
		* Get the even rssi limits in dBm. Make sure connection is established
		* @param min pointer to store min float value in dBm
		* @param max pointer to store max float value in dBm
		* @param noiseFloor pointer to store noise floor float value in dBm
		* @return true if the values is stored into the pointers else false
		*/
		bool getEvenRssiLimts(float* min, float* max, float * noiseFloor);

		/**
		* Get the 2.4 GHz frequency boundary in MHz.
		* @param minMHz pointer to store the lower frequency of the boundary in MHz
		* @param maxMHz pointer to store the higher frequency of the boundary in MHz
		* @return true if the values is stored in the pointer else false
		*/
		bool get2_4GHzBoundary(float * minMHz, float * maxMHz);

		/**
		* Get the 5 GHz frequency in MHz.
		* @param minMHz pointer to store the lower frequency of the boundary in MHz
		* @param maxMHz pointer to store the higher frequency of the boundary in MHz
		* @return true if the values is stored in the pointer else false
		*/
		bool get5GHzBoundary(float * minMHz, float * maxMHz);

		/**
		* Get the 6E frequency in MHz.
		* @param minMHz pointer to store the lower frequency of the boundary in MHz
		* @param maxMHz pointer to store the higher frequency of the boundary in MHz
		* @return true if the values is stored in the pointer else false
		*/
		bool get6EBoundary(float * minMHz, float * maxMHz);

	private:
		WiPryClarity_pimpl * _pimpl;

		void _communicationThreadMain();

		static void *communicationThreadMain(void *threadObject);

		void _dataThreadMain();

		static void *dataThreadMain(void *threadObject);

		static void *processDataThreadMain(void *threadObject);
		void _processDataThreadMain();

		void _processData(uint8_t *data, uint32_t size);

	};

	/**
	* @brief Interface for the delegate to WiPryClarity.
	*
	* Class that serves as a delegate to WiPryClarity. Classes that want to receive messages must sublcass
	*   this class and implement/override the methods whose messages they want to receive and set an
	*   object of the class as a delegate of WiPryClarity using the method WiPryClarity::setDelegate()
	*/
	class WIPRYCLARITY_API WiPryClarityDelegate {
	public:
		/**
		* Method that is called by WiPryClarity when the communication has been successfuly started
		* @param wipryClarity WiPryClarity object calling this method
		*/
		virtual void wipryClarityDidConnect(WiPryClarity *wipryClarity) {}

		/**
		* This method is called by WiPryClarity when the communication process has failed.
		* @param wipryClarity WiPryClarity object calling this method
		* @param errorCode Error code
		*/
		virtual void wipryClarityUnableToConnect(WiPryClarity *wipryClarity, WiPryClarity::ErrorCode errorCode) {}

		/**
		* This method is called when there is 2.4Ghz or 5GHz rssi data from the usb device
		* @param wipryClarity WiPryClarity object calling this method
		* @param dataType datatype (Either WiPryClarity::DataType::RSSI_2_4GHZ or WiPryClarity::DataType::RSSI_5GHZ)
		* @param rssiData float vector containing the rssi data.
		* @note Make sure the implementation of this method is fast. If not, subsequent data will be missed.
		*/
		virtual void wipryClarityDidReceiveRSSIData(WiPryClarity *wipryClarity, WiPryClarity::DataType dataType,
			std::vector<float> rssiData) {}

		/**
		* This method is called when there is beacon capture data from the usb device
		* @param wipryClarity WiPryClarity object calling this method
		* @param beaconCaptures vector whose data contains an array of tightly packed OsciumBeaconCapture elements
		* @note Make sure the implementation of this method is fast. If not, subsequent data will be missed.
		*/
		virtual void wipryClarityDidReceiveBeaconCaptureData(WiPryClarity *wipryClarity,
			std::vector<uint8_t> beaconCaptures) {}

	};

}

#endif /* WiPryClarity_h */
