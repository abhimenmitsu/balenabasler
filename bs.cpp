#include <iostream>
#include <pylon/PylonIncludes.h>

int main() {
    try {
        // Initialize Pylon
        Pylon::PylonInitialize();
        std::cout << "Pylon initialized successfully." << std::endl;

        // Create an InstantCamera object for the first available camera
        Pylon::CInstantCamera camera(Pylon::CTlFactory::GetInstance().CreateFirstDevice());
        std::cout << "Using device: " << camera.GetDeviceInfo().GetModelName() << std::endl;

        // Open the camera
        camera.Open();
        std::cout << "Camera opened successfully." << std::endl;

        // Start grabbing images
        camera.StartGrabbing(Pylon::GrabStrategy_LatestImageOnly);

        // Retrieve a frame
        Pylon::CGrabResultPtr ptrGrabResult;
        camera.RetrieveResult(5000, ptrGrabResult, Pylon::TimeoutHandling_ThrowException);

        if (ptrGrabResult->GrabSucceeded()) {
            std::cout << "Image captured successfully." << std::endl;
            // Process the image here...
        } else {
            std::cerr << "Image capture failed." << std::endl;
        }

        // Close the camera
        camera.Close();
        std::cout << "Camera closed successfully." << std::endl;

        // Terminate Pylon
        Pylon::PylonTerminate();
        std::cout << "Pylon terminated successfully." << std::endl;

    } catch (const Pylon::GenericException& e) {
        std::cerr << "Pylon exception occurred: " << e.GetDescription() << std::endl;
        return 1;
    }
    return 0;
}
