#include <numeric>
#include <iostream>
#include <sstream>
#include <chrono>
#include <fstream> // For file operations
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <csignal> // For signal handling
#include <pylon/PylonIncludes.h>
#include <pylon/ImageFormatConverter.h>
#include <pylon/InstantCamera.h>
#include <sys/stat.h> // For mkdir
#include <cerrno>
#include <vector> 
#include <unordered_map>
#include "client.cpp" 
using namespace GenApi;
using namespace cv;
using namespace std;
using namespace Pylon; // Pylon namespace for Basler cameras




volatile bool stopProgram = false; // Global flag to handle interruption

// Signal handler
void signalHandler(int signal) {
    if (signal == SIGINT) {
        stopProgram = true;
        cout << "\nProgram interrupted. Finalizing logs..." << endl;
    }
}

int main(int argc, char* argv[]) {
    // Initialize Pylon runtime
    PylonInitialize();


    float f, Pwhite;
    float FPS[16] = {0}; // Initialize all FPS elements to 0
    size_t i, n;
    float Sum;
    int Xcnt, Tcnt, Fcnt = 0;
    float Tau = 0.05;
    float ttm = 0.0;
    int totalFrames = 0; // Counter for total frames processed
    float totalElapsedTime = 0.0;
    chrono::steady_clock::time_point Tbegin, Tend;
    vector<Mat> significantFrames;
    // Declare a map to store frame number and frames
    unordered_map<int, Mat> frameMemory;
    Mat frame, bw_frame, sum_frame, sub_frame, finalImage, fImage;

    f = 1.0 / Tau;
    Tcnt = static_cast<int>(f);
    Xcnt = static_cast<int>(f);

    // Open log file
    ofstream logFile("processing_logs.txt");
    if (!logFile.is_open()) {
        cerr << "Unable to open log file!" << endl;
        return 1;
    }
    // Initialize WebSocket
    const std::string ws_uri = "ws://3.131.203.190:8765";
    initialize_websocket(ws_uri);
    // Connecting to the camera
    cout << "[INFO] Connecting to the first available camera..." << endl;
    CInstantCamera camera(CTlFactory::GetInstance().CreateFirstDevice());
    cout << "[INFO] Connected to camera: " << camera.GetDeviceInfo().GetModelName() << endl;

    // Open the camera
    camera.Open();
    cout << "[INFO] Camera opened." << endl;

    // Access and set camera parameters
    INodeMap& nodeMap = camera.GetNodeMap();
    CEnumerationPtr(nodeMap.GetNode("ExposureAuto"))->SetIntValue(
        CEnumerationPtr(nodeMap.GetNode("ExposureAuto"))->GetEntryByName("Off")->GetValue());
    CEnumerationPtr(nodeMap.GetNode("GainAuto"))->SetIntValue(
        CEnumerationPtr(nodeMap.GetNode("GainAuto"))->GetEntryByName("Off")->GetValue());
    CEnumerationPtr(nodeMap.GetNode("BalanceWhiteAuto"))->SetIntValue(
        CEnumerationPtr(nodeMap.GetNode("BalanceWhiteAuto"))->GetEntryByName("Off")->GetValue());
    CFloatPtr(nodeMap.GetNode("ExposureTime"))->SetValue(4400);

    CIntegerPtr(nodeMap.GetNode("Width"))->SetValue(640);
    CIntegerPtr(nodeMap.GetNode("Height"))->SetValue(360);

    cout << "[INFO] Camera parameters configured." << endl;

    // Start grabbing images
    camera.StartGrabbing(GrabStrategy_LatestImageOnly);
    cout << "[INFO] Camera started grabbing images." << endl;

    CImageFormatConverter formatConverter;
    formatConverter.OutputPixelFormat = PixelType_BGR8packed;

    Mat openCvImage; // OpenCV matrix to hold the frame
    chrono::steady_clock::time_point ProgramStart = chrono::steady_clock::now(); // Track program start time

    // Register the signal handler
    signal(SIGINT, signalHandler);
    const string outputDirectory = "significant_changes_frames/";
    if (mkdir(outputDirectory.c_str(), 0777) == -1 && errno != EEXIST) {
    cerr << "Error creating output directory!" << endl;
    return 1;
    }

    while (!stopProgram && camera.IsGrabbing()) {
        CGrabResultPtr ptrGrabResult;
        camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);

        if (ptrGrabResult->GrabSucceeded()) {
            // Increment the total frames counter
            totalFrames++;

            CPylonImage pylonImage;
            formatConverter.Convert(pylonImage, ptrGrabResult);
            openCvImage = Mat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC3,
                              (uint8_t*)pylonImage.GetBuffer());

            if (openCvImage.empty()) {
                cerr << "Empty frame captured. Skipping..." << endl;
                continue;
            }

            Tbegin = chrono::steady_clock::now();

            // Convert to grayscale
            cv::cvtColor(openCvImage, bw_frame, COLOR_BGR2GRAY);
            bw_frame.convertTo(fImage, CV_32FC1);

            // Update background model
            f = static_cast<float>(Tcnt) / Xcnt;
            Tcnt--;
            if (Tcnt < 1) Tcnt = 1;

            if (sum_frame.empty()) {
                sum_frame = Mat::zeros(fImage.size(), fImage.type());
            }

            cv::addWeighted(fImage, f, sum_frame, (1.0f - f), 0.0, sum_frame);

            // Subtract background
            cv::subtract(sum_frame, fImage, sub_frame);
            cv::convertScaleAbs(sub_frame, finalImage, 1.5);

            // Count bright pixels
            n = finalImage.total();
            Sum = countNonZero(finalImage > 30);
            Pwhite = (Sum * 100.0f) / n;

            Tend = chrono::steady_clock::now();
            f = chrono::duration_cast<chrono::milliseconds>(Tend - Tbegin).count();
            FPS[(Fcnt++) & 0x0F] = f;

            ttm += std::accumulate(std::begin(FPS),std::end(FPS),0.0f) / 16;

            // Log results
            cout << Pwhite << "% - " << f / 16 << " ms" << endl;
            logFile << "Frame " << totalFrames
                    << ": Bright Pixels = " << Pwhite << "%, Avg Time = " << f / 16 << " ms" << endl;



        if (Pwhite > 0.01) {
            significantFrames.push_back(openCvImage.clone());
            frameMemory[totalFrames] = openCvImage.clone(); // Save the frame along with its number
            send_frame_over_websocket(openCvImage.clone());
            logFile << "Significant frame saved: " << totalFrames << endl;
        }
  
        }
        
    }

    camera.StopGrabbing();
    camera.Close();



    chrono::steady_clock::time_point ProgramEnd = chrono::steady_clock::now();
    totalElapsedTime = chrono::duration_cast<chrono::milliseconds>(ProgramEnd - ProgramStart).count() / 1000.0f;

    float finalFPS = totalFrames / totalElapsedTime;

    // Final logs
    cout << "Total background subtraction time: " << ttm << endl;
    cout << "Total number of frames processed: " << totalFrames << endl;
    cout << "Total time elapsed: " << totalElapsedTime << " seconds" << endl;
    cout << "Final FPS: " << finalFPS << endl;

    logFile << "Total background subtraction time: " << ttm << endl;
    logFile << "Total number of frames processed: " << totalFrames << endl;
    logFile << "Total time elapsed: " << totalElapsedTime << " seconds" << endl;
    logFile << "Final FPS: " << finalFPS << endl;

    logFile.close();
    PylonTerminate();

    return 0;
}

//c++ code on basler written signficant changes frames in -memory
