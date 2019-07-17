#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>
#include <signal.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include "PhoXi.h"
#include "Server/server.h"

volatile sig_atomic_t sigint_received = 0;

// Control-C handler
void sigint_handler(int);

// Checks whether the requested device is available
bool isDeviceAvailable(pho::api::PhoXiFactory &Factory, const std::string &serialNumber);

// Returns a device object
pho::api::PPhoXi connectToDevice(pho::api::PhoXiFactory &Factory, const std::string &serialNumber);

// Configures the device's parameters
void configDevice(const pho::api::PPhoXi &PhoXiDevice, const std::string &resolution);

// Grabs a frame from a connected device
bool getFrame(const pho::api::PPhoXi &PhoXiDevice, pho::api::PFrame &Frame);

// Checks whether the received frame is corrupt
bool checkFrame(const pho::api::PFrame &Frame);

// Captures grayscale and depth images and sends them through TCP
// serial number: the hardware identification number, i.e. "2019-01-016-LC3"
// port number: 50000
// resolution: "low" or "high"
int main(int argc, char * argv[]) {
    if (argc != 4) {
        std::cout << "[!] Missing or invalid command line arguments." << std::endl;
        std::cout << "Usage: " << argv[0] << " <serial#> <port> <resolution>" << std::endl;
        return 0;
    }

    // parse command line arguments
    std::string serialNumber = argv[1];
    unsigned int portNumber = std::stoi(argv[2]);
    std::string resolution = argv[3];

    if (resolution != "high" && resolution != "low") {
        std::cout << "[!] Resolution must be one of [\"low\", \"high\"]." << std::endl;
        return 0;
    }

    // check if any connected device matches the requested serial number
    pho::api::PhoXiFactory factory;
    bool isFound = isDeviceAvailable(factory, serialNumber);
    if (!isFound) {
        std::cout << "[!] Requested device (serial number: " << serialNumber << ") not found!" << std::endl;
        return 0;
    }

    // connect to the device
    pho::api::PPhoXi cam = connectToDevice(factory, serialNumber);
    if (!cam->isConnected()) {
        std::cout << "[!] Could not connect to device." << std::endl;
    } else {
        std::cout << "[*] Successfully connected to device." << std::endl;
    }

    // configure the device
    configDevice(cam, resolution);

    // connect to the server
    Server phoxiServer(portNumber);
    phoxiServer.init_listener_thread();

    pho::api::PFrame frame;
    int frameID = 0;
    bool success;
    signal(SIGINT, sigint_handler);
    while (!sigint_received) {
        // wait for frame to arrive
        success = getFrame(cam, frame);

        // send frame to server
        if (checkFrame(frame) && success) {
            std::cout << "Frame ID: " << frameID << std::endl;

            // both gray and depth have the same size so grab from whichever
            unsigned long dataSize = frame->DepthMap.GetDataSize();
            int height = frame->DepthMap.Size.Height;
            int width = frame->DepthMap.Size.Width;

            // packet header
            phoxiServer.update_buffer((unsigned char*)&height, 0, 4);
            phoxiServer.update_buffer((unsigned char*)&width, 4, 4);
            phoxiServer.update_buffer((unsigned char*)&frameID, 8, 4);

            // packet payload
            phoxiServer.update_buffer((unsigned char*)frame->Texture.GetDataPtr(), 12, dataSize);
            phoxiServer.update_buffer((unsigned char*)frame->DepthMap.GetDataPtr(), 12 + dataSize, dataSize);

            frameID += 1;
        }

        // sleep for a few milliseconds
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    }

    // release resources
    std::cout << "[*] Disconnecting from device." << std::endl;
    cam->StopAcquisition();

    return 0;
}

void sigint_handler(int s) {
    sigint_received = 1;
}

bool isDeviceAvailable(pho::api::PhoXiFactory &factory, const std::string &serialNumber) {
    if (!factory.isPhoXiControlRunning()) {
        std::cout << "[!] PhoXi Control Software is not running." << std::endl;
        return false;
    }
    std::vector <pho::api::PhoXiDeviceInformation> deviceList = factory.GetDeviceList();
    if (deviceList.empty()) {
        std::cout << "[!] 0 devices found." << std::endl;
        return false;
    }
    bool isFound = false;
    for (std::size_t i = 0; i < deviceList.size(); ++i) {
        if (deviceList[i].HWIdentification == serialNumber) {
            isFound = true;
        }
    }
    return isFound;
}

pho::api::PPhoXi connectToDevice(pho::api::PhoXiFactory &factory, const std::string &serialNumber) {
    pho::api::PPhoXi PhoXiDevice = factory.CreateAndConnect(serialNumber);
    return PhoXiDevice;
}

void configDevice(const pho::api::PPhoXi &PhoXiDevice, const std::string &resolution) {
    // Set trigger to "freerun" mode
    if (PhoXiDevice->TriggerMode != pho::api::PhoXiTriggerMode::Freerun) {
        if (PhoXiDevice->isAcquiring()) {
            if (!PhoXiDevice->StopAcquisition()) {
                throw std::runtime_error("Error in StopAcquistion");
            }
        }
        PhoXiDevice->TriggerMode = pho::api::PhoXiTriggerMode::Freerun;
        if (!PhoXiDevice->TriggerMode.isLastOperationSuccessful()) {
            throw std::runtime_error(PhoXiDevice->TriggerMode.GetLastErrorMessage().c_str());
        }
    }

    // Just send Texture and DepthMap
    pho::api::FrameOutputSettings currentOutputSettings = PhoXiDevice->OutputSettings;
    pho::api::FrameOutputSettings newOutputSettings = currentOutputSettings;
    newOutputSettings.SendPointCloud = false;
    newOutputSettings.SendNormalMap = false;
    newOutputSettings.SendDepthMap = true;
    newOutputSettings.SendConfidenceMap = false;
    newOutputSettings.SendTexture = true;
    PhoXiDevice->OutputSettings = newOutputSettings;

    // Configure the device resolution
    pho::api::PhoXiCapturingMode mode = PhoXiDevice->CapturingMode;
    if (resolution == "low") {
        mode.Resolution.Width = 1032;
        mode.Resolution.Height = 772;
    } else {
        mode.Resolution.Width = 2064;
        mode.Resolution.Height = 1544;
    }
    PhoXiDevice->CapturingMode = mode;
}

bool getFrame(const pho::api::PPhoXi &PhoXiDevice, pho::api::PFrame &Frame) {
    // start device acquisition if necessary
    if (!PhoXiDevice->isAcquiring()) PhoXiDevice->StartAcquisition();

    // clear the current acquisition buffer
    PhoXiDevice->ClearBuffer();

    if (!PhoXiDevice->isAcquiring()) {
        std::cout << "[!] Your device could not start acquisition!" << std::endl;
        return false;
    }
    Frame = PhoXiDevice->GetFrame();
    if (!Frame) {
        std::cout << "[!] Failed to retrieve the frame!" << std::endl;
        return false;
    }
    return true;
}

bool checkFrame(const pho::api::PFrame& Frame) {
    if (Frame->Empty()) {
        std::cout << "Frame is empty.";
        return false;
    }
    if ((Frame->DepthMap.Empty()) || (Frame->Texture.Empty())) return false;
    return true;
}
