#include "WiPryClarity.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <signal.h>
#include <cstring>

/*

    wipry-lp

    Outputs Oscium WiPry spectrum analysis data in Influx Line Protocol format

    Copyright (c) 2023 Matt Lee and Bryan Ward

*/


std::string VERSION = "v1.1.0";


using namespace oscium;
WiPryClarity* wipryClarity = nullptr;

sig_atomic_t signaled = 0;
bool run = true;
unsigned int band;
std::string serial;

bool isConnected = false; // flag denoting that there is a successful connection
bool connectionProcessComplete = true; // flag denoting that the connection process has completed
unsigned int rssi2_4GHzFrameCount;
unsigned int rssi5GHzFrameCount;
unsigned int rssi6EFrameCount;


float even_min, even_max, even_noisefloor;
float odd_min, odd_max, odd_noisefloor;

float freqLow2, freqHigh2;
float freqLow5, freqHigh5;
float freqLow6, freqHigh6;


// This the delegate class that receives the events from the WiPryClarity object.
 class MyDelegate : public WiPryClarityDelegate {
public:
	void wipryClarityDidConnect(WiPryClarity *aWipryClarity) {
		std::cerr << "Connected to device." << std::endl;
		isConnected = true;
		connectionProcessComplete = true;

		float min, max, noisefloor;
		float freqLow, freqHigh;
		bool result;

		result = aWipryClarity->getEvenRssiLimts(&min, &max, &noisefloor);
		if (result) {
			std::cerr << "Even Min:" << min << "dBm Max:" << max << "dBm Noise Floor:" << noisefloor << "dBm" << std::endl;
			even_min = min;
			even_max = max;
			even_noisefloor = noisefloor;
		}
		else
			std::cerr << "Unable to get limits" << std::endl;

		result = aWipryClarity->getOddRssiLimts(&min, &max, &noisefloor);
		if (result) {
			std::cerr << " Odd Min:" << min << "dBm Max:" << max << "dBm Noise Floor:" << noisefloor << "dBm" << std::endl;
			odd_min = min;
			odd_max = max;
			odd_noisefloor = noisefloor;
		}
		else
			std::cerr << "Unable to get limits" << std::endl;

		result = aWipryClarity->get2_4GHzBoundary(&freqLow, &freqHigh);
		if (result) {
			std::cerr << "2.4GHz Low:" << freqLow << "MHz High:" << freqHigh << "MHz" << std::endl;
			freqLow2 = freqLow;
			freqHigh2 = freqHigh;
		}
		else
			std::cerr << "Unable to get 2.4GHz frequency limits" << std::endl;

		result = aWipryClarity->get5GHzBoundary(&freqLow, &freqHigh);
		if (result) {
			std::cerr << "  5GHz Low:" << freqLow << "MHz High:" << freqHigh << "MHz" << std::endl;
			freqLow5 = freqLow;
			freqHigh5 = freqHigh;
		}
		else
			std::cerr << "Unable to get 5GHz frequency limits" << std::endl;

		result = aWipryClarity->get6EBoundary(&freqLow, &freqHigh);
		if (result) {
			std::cerr << "    6E Low:" << freqLow << "MHz High:" << freqHigh << "MHz" << std::endl;
			freqLow6 = freqLow;
			freqHigh6 = freqHigh;
		}
		else
			std::cerr << "Unable to get 6E frequency limits" << std::endl;

		rssi2_4GHzFrameCount = 0;
		rssi5GHzFrameCount = 0;
		rssi6EFrameCount = 0;
	}

	void wipryClarityUnableToConnect(WiPryClarity *wipryClarity, WiPryClarity::ErrorCode errorCode) {
		std::cerr << "Disconnected from Accessory with Error Code : " << (int)errorCode << std::endl;
		// delete the global WiPry Clarity object
		if (wipryClarity != nullptr)
		{
			if (wipryClarity->didStartCommunication())
				wipryClarity->endCommunication();

			// NOTE: Do not delete the wipry2500 object here or it will cause a crash. Delete it somewhere else.
			// delete wipryClarity;
			// wipryClarity = nullptr;

			connectionProcessComplete = true;
			isConnected = false;
		}
	}

	//Not Implemented in library
	/*
	void wipryClarityDidReceiveBeaconCaptureData(WiPryClarity *wipryClarity, std::vector<uint8_t> beaconCaptures) {
		if (beaconCaptures.size() > 0) {
			std::cerr<<"Got " << beaconCaptures.size() << " beacons!" << std::endl;
		}
	}
	*/


	void wipryClarityDidReceiveRSSIData(WiPryClarity *aWipryClarity, WiPryClarity::DataType dataType, std::vector<float> rssiData) {
                long long timens = std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
		switch (dataType)
		{
			case oscium::WiPryClarity::DataType::RSSI_2_4GHZ:
			{
				std::cerr << "2.4 GHz rssi data with " << (int)rssiData.size() << " points" << std::endl;
				float stepsize = ( (freqHigh2 - freqLow2) / (int)rssiData.size() );
				std::cout << "wipry,serial=" << serial << ",band=2 ";
				for (int p=0; p < (int)rssiData.size(); p++) {
					//std::cerr << rssiData[p] << " ";
					//Truncate
					if (p < ((int)rssiData.size() - 1))
						std::cout << (freqLow2 + p * stepsize) << "=" <<(int)rssiData[p] << ",";
					else
						std::cout << (freqLow2 + p * stepsize) << "=" <<(int)rssiData[p];
				}
                                std::cout << " " << timens << std::endl;
			}
			break;
			case oscium::WiPryClarity::DataType::RSSI_5GHZ:
			{
				std::cerr << "5 GHz rssi data with " << (int)rssiData.size() << " points" << std::endl;
				float stepsize = ( (freqHigh5 - freqLow5) / (int)rssiData.size() );
				std::cout << "wipry,serial=" << serial << ",band=5 ";
				for (int p=0; p < (int)rssiData.size(); p++) {
					//std::cerr << rssiData[p] << " ";
					//Truncate
					if (p < ((int)rssiData.size() - 1))
						std::cout << (freqLow5 + p * stepsize) << "=" <<(int)rssiData[p] << ",";
					else
						std::cout << (freqLow5 + p * stepsize) << "=" <<(int)rssiData[p];
				}
                                std::cout << " " << timens << std::endl;
			}
			break;
/*
			case oscium::WiPryClarity::DataType::RSSI_DUAL25:
			{
				std::cerr << "Dual band rssi data with " << (int)rssiData.size() << " points" << std::endl;
				for (int p=0; p < (int)rssiData.size(); p++) {
					//std::cerr << rssiData[p] << " ";
					//Truncate
					std::cerr << (int)rssiData[p] << " ";
				}
                                std::cout << " " << timens << std::endl;
			}
			break;
*/
			case oscium::WiPryClarity::DataType::RSSI_6E:
			{
				std::cerr << "6 GHz rssi data with " << (int)rssiData.size() << " points" << std::endl;
				float stepsize = ( (freqHigh6 - freqLow6) / (int)rssiData.size() );
				std::cout << "wipry,serial=" << serial << ",band=6 ";
				for (int p=0; p < (int)rssiData.size(); p++) {
					//std::cerr << rssiData[p] << " ";
					//Truncate
					if (p < ((int)rssiData.size() - 1))
						std::cout << (freqLow6 + p * stepsize) << "=" <<(int)rssiData[p] << ",";
					else
						std::cout << (freqLow6 + p * stepsize) << "=" <<(int)rssiData[p];
				}
                                std::cout << " " << timens << std::endl;
			}
			break;

			default:
				break;
		}

	}
};


void sig_handler (int param)
{
  run = false;
}


void helptext() {
        std::cout << "Outputs Oscium WiPry spectrum analysis data in Influx Line Protocol format" << std::endl;
        std::cout << std::endl;
	std::cout << "Usage:" << std::endl;
	std::cout << std::endl;
        std::cout << "    wipry-lp -[2|5|6]" << std::endl;
        std::cout << std::endl;
        std::cout << "Options:" << std::endl;
	std::cout << "	-2		Run on the 2.4GHz Band" << std::endl;
	std::cout << "	-5		Run on the 5GHz Band" << std::endl;
	std::cout << "	-6		Run on the 6GHz Band" << std::endl;
        //Not yet implemented.  Supported by libWiPryClarity, but requires special handling of non-contiguous spectrum data
	//std::cout << "	-D		Run on both the 2.4GHz and 5GHz Band" << std::endl;
        //Not yet implemented.  ToDo: Requires logic to manually switch between bands while running
	//std::cout << "	-T		Run on all three bands" << std::endl;
	std::cout << "	-h		Print this help text and exit." << std::endl;

        std::cout << std::endl;
        std::cout << std::endl;
        std::cout << "Built by Bryan Ward, based on sample code graciously provided by Matt Lee from Oscium" << std::endl;
        std::cout << "wipry-lp Version " << VERSION << std::endl;
        //std::cout << "Build Date " << __BUILDTIMESTAMP__ << std::endl;
        std::cout << "libWiPryClarity version " << oscium::WiPryClarity::getVersion() << std::endl;
        std::cout << "Copyright (c) 2023 Matt Lee and Bryan Ward" << std::endl;
}


MyDelegate delegate;

int main(int argc, char *argv[]) {

	if (argc <= 1) {
		std::cerr << "No band specified!" << std::endl;
		helptext();
		return 1;
	}

	if (argc > 2) {
		std::cerr << "Specify only one argument!" << std::endl;
		helptext();
		return 1;
	}

	if (argc == 2) {
		if (strcmp(argv[1], "-h") == 0) {
			helptext();
			return 0;
		}
		else if (strcmp(argv[1], "-2") == 0) {
			band = 2;
		}
		else if (strcmp(argv[1], "-5") == 0) {
			band = 5;
		}
		else if (strcmp(argv[1], "-6") == 0) {
			band = 6;
		}
		else if (strcmp(argv[1], "-D") == 0) {
			band = 25;
		}
		//else if (strcmp(argv[1], "-T") == 0) {
		//	band = 256;
		//	std::cerr << "Not Yet Implemented!" << std::endl;
		//	return 2;
		//}
		else {
			std::cerr << "Invalid Argument Specified!" << std::endl;
			helptext();
			return 1;
		}
	}

	wipryClarity = new WiPryClarity();
	wipryClarity->setDelegate(&delegate);


	// start the connection
	std::cerr<< "Starting connection process." << std::endl;
	connectionProcessComplete = false;
	isConnected =false;

	bool result = wipryClarity->startCommunication();
	if (result == false) {
		std::cerr<< "Error: Unable to connect to the WiPryClarity. Make sure that the device has been connected." << std::endl;
		delete wipryClarity;
		wipryClarity = nullptr;
		return -1;
	}

	// wait for the connection prcoess to complete
	while(!connectionProcessComplete) {
		std::chrono::milliseconds duration(100);
		std::this_thread::sleep_for(duration);
	}

	if ( isConnected )
	{
		std::cerr<< "Connection Success." << std::endl;
		serial = wipryClarity->getSerialNumber();
		std::cerr<< "Serial Number: " << serial;
	} else {
		// connection failed
		delete wipryClarity;
		return -1;
	}

	[[maybe_unused]] void (*sigint_handler)(int);
	sigint_handler = signal(SIGINT, sig_handler);
	[[maybe_unused]] void (*sigterm_handler)(int);
	sigterm_handler = signal(SIGTERM, sig_handler);
	[[maybe_unused]] void (*sigabrt_handler)(int);
	sigabrt_handler = signal(SIGABRT, sig_handler);


	if (band == 2) {
		// start 2.4 Ghz Rssi data
		std::cerr << "Starting 2.4 GHz rssi data stream." << std::endl;
		wipryClarity->startRssiData(oscium::WiPryClarity::DataType::RSSI_2_4GHZ, 0, 0, 0);
	}

	if (band == 5) {
		// start 5 Ghz Rssi data
		std::cerr << "Starting 5 GHz rssi data stream." << std::endl;
		wipryClarity->startRssiData(oscium::WiPryClarity::DataType::RSSI_5GHZ, 0, 0, 0);
	}

	if (band == 6) {
		// start 6 GHz Rssi data
		std::cerr << "Starting 6 GHz rssi data stream." << std::endl;
		wipryClarity->startRssiData(oscium::WiPryClarity::DataType::RSSI_6E, 0, 0, 0);
	}

	if (band == 25) {
		// start Dual band RSSI data
		std::cerr << "Starting Dual Band rssi data stream." << std::endl;
		wipryClarity->startRssiData(oscium::WiPryClarity::DataType::RSSI_DUAL25, 0, 0, 0);
	}

	while (run) {
	    std::this_thread::yield();
            //Prevent CPU spinlock
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
       }


	// stop the data
	std::cerr << "Stopping rssi data stream." << std::endl;
	wipryClarity->stopRssiData();

	// sleep for 500ms
	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	// closing connection
	std::cerr<< "Closing connection to WiPry Clarity." << std::endl;
	if (wipryClarity->didStartCommunication())
		wipryClarity->endCommunication();
	delete wipryClarity;
	wipryClarity = nullptr;


	return 0;
}
