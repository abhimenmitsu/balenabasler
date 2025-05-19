Repo Structure:

- Dockerfile

   - Installation of relevant libraries
   - Main Component

       - Installation of Pylon over docker image balenalib/armv7hf-ubuntu:latest
   
       - Copy pylon_6.1.3.20159_armhf_edited.tar.gz in Docker, unzip and run relevant command to install pylon to run basler camera over Balena

- bs.cpp
  
    - Background subtraction in C++

- bsfast.cpp
  
    - Optimized version of background subtraction in C++

- client.cpp

    - Websocket connect to send over server written in C++

- pylon_6.1.3.20159_armhf_edited.tar.gz

  - Pylon sdk to install in Docker
