# Base image for Balena
#FROM balenalib/raspberrypicm4-ioboard-ubuntu-python:latest
FROM balenalib/armv7hf-ubuntu:latest
# Set up environment variables for non-interactive installation
ENV DEBIAN_FRONTEND=noninteractive
ENV UDEV=1
ENV PYLON_ROOT=/opt/pylon
# ENV PKG_CONFIG_PATH=/usr/lib/x86_64-linux-gnu/pkgconfig:$PKG_CONFIG_PATH
ENV PKG_CONFIG_PATH=/usr/lib/pkgconfig:$PKG_CONFIG_PATH

# Install system dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    libopencv-dev \
    wget \
    git \
    pkg-config \
    python3 \
    python3-pip \
    file \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

RUN pkg-config --modversion opencv || pkg-config --modversion opencv4


# Copy the Pylon SDK tarball into the container
COPY pylon_6.1.3.20159_armhf_edited.tar.gz /usr/src/app/

# Install the Pylon SDK
RUN mkdir -p ${PYLON_ROOT} && \
    tar -xzf /usr/src/app/pylon_6.1.3.20159_armhf_edited.tar.gz -C ${PYLON_ROOT} --strip-components=1 && \
    sudo chmod 755 /opt/pylon && \
    yes | ${PYLON_ROOT}/share/pylon/setup-usb.sh

# Set environment variables for Pylon SDK
ENV LD_LIBRARY_PATH=${PYLON_ROOT}/lib:$LD_LIBRARY_PATH
ENV PATH=${PYLON_ROOT}/bin:$PATH

# Create the application working directory
WORKDIR /usr/src/app

# Copy the source code into the container
COPY bsfast.cpp .
COPY bs.cpp .


RUN file /opt/pylon/lib/libpylonbase-6.1.0.so

#RUN file /opt/pylon/lib/libpylonbase.so

# Build the application using the specified flags
# RUN g++ -o camera_app bsfast.cpp $(pkg-config --cflags --libs opencv4) -I/opt/pylon/include -L/opt/pylon/lib -lpylonbase -lpylonutility
# RUN g++ -o camera_app bs.cpp \
#     $(${PYLON_ROOT}/bin/pylon-config --cflags) \
#     -I/usr/include/opencv4 \
#     -g -O2 \
#     $(${PYLON_ROOT}/bin/pylon-config --libs-rpath) \
#     $(${PYLON_ROOT}/bin/pylon-config --libs) \
#     $(pkg-config --libs opencv4)

RUN g++ -std=c++11 -o camera_app bs.cpp \
    $(${PYLON_ROOT}/bin/pylon-config --cflags) \
    -I/usr/include/opencv4 -g -O2 \
    $(${PYLON_ROOT}/bin/pylon-config --libs-rpath) \
    $(${PYLON_ROOT}/bin/pylon-config --libs) \
    $(pkg-config --libs opencv4)


# Create the output directory for frames
RUN mkdir -p /usr/src/app/significant_changes_frames

# Expose the application port if required
EXPOSE 8080

# Set the container's entry point
CMD ["./camera_app"]
