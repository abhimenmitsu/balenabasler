try {
    camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);
} catch (const GenericException& e) {
    cerr << "Pylon exception occurred: " << e.GetDescription() << endl;
    return 1;
}
