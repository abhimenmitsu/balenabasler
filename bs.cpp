#include <iostream>
#include <pylon/PylonIncludes.h>

int main() {
    try {
        // Initialize Pylon
        Pylon::PylonInitialize();
        std::cout << "Pylon initialized successfully." << std::endl;

        // Your camera code here...
        camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);

        Pylon::PylonTerminate();
    } catch (const Pylon::GenericException& e) {
        std::cerr << "Pylon exception occurred: " << e.GetDescription() << std::endl;
        return 1;
    }
    return 0;
}
