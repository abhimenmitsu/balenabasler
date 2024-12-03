# Base image for Balena
FROM balenalib/aarch64-ubuntu
LABEL io.balena.device-type="raspberrypicm4-ioboard"

# Set up environment variables for non-interactive installation
ENV DEBIAN_FRONTEND=noninteractive
ENV UDEV=1
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
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Copy the Pylon SDK tarball into the container
COPY pylon_6.1.3.20159_armhf_setup.tar.gz /usr/src/app/

# Install the Pylon SDK
RUN mkdir -p /opt/pylon && \
    tar -xzf /usr/src/app/pylon_6.1.3.20159_armhf_setup.tar.gz -C /opt/pylon --strip-components=1 && \
    yes | /opt/pylon/setup-usb.sh

# Set environment variables for Pylon SDK
ENV LD_LIBRARY_PATH=/opt/pylon/lib:$LD_LIBRARY_PATH
ENV PATH=/opt/pylon/bin:$PATH

# Create the application working directory
WORKDIR /usr/src/app

# Copy the source code into the container
COPY bsfast.cpp .


# Verify OpenCV installation
RUN pkg-config --modversion opencv || pkg-config --modversion opencv4

# Build the application
RUN g++ -o camera_app bsfast.cpp $(pkg-config --libs opencv) -I/opt/pylon/include -L/opt/pylon/lib -lpylonbase -lpylonutility

# Create the output directory for frames
RUN mkdir -p /usr/src/app/significant_changes_frames

# Expose the application port if required
EXPOSE 8080

# Set the container's entry point
CMD ["./camera_app"]
