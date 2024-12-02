# Base image for Balena
FROM balenalib/aarch64-ubuntu
LABEL io.balena.device-type="raspberrypicm4-ioboard"

# Set up environment variables for non-interactive installation
ENV DEBIAN_FRONTEND=noninteractive
ENV UDEV=1

# Install system dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    libopencv-dev \
    libopencv-highgui-dev \
    libopencv-imgcodecs-dev \
    libopencv-imgproc-dev \
    libopencv-videoio-dev \
    libopencv-video-dev \
    libopencv-core-dev \
    libopencv-calib3d-dev \
    libopencv-features2d-dev \
    libopencv-flann-dev \
    wget \
    git \
    pkg-config \
    python3 \
    python3-pip \
    libopencv-dev \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Install the Pylon SDK for ARM
RUN wget https://www.baslerweb.com/fp-1575732014/media/downloads/software/pylon-5.2.0.13457-x86.tar.gz && \
    tar -xzf pylon-5.2.0.13457-x86.tar.gz && \
    ./pylon-5.2.0.13457-x86/bin/pylon_install.sh -y && \
    rm -rf pylon-5.2.0.13457-x86.tar.gz

# Set environment variables for Pylon SDK
ENV LD_LIBRARY_PATH=/opt/pylon/lib:$LD_LIBRARY_PATH
ENV PATH=/opt/pylon/bin:$PATH

# Create application directory
WORKDIR /usr/src/app

# Copy source code to container
COPY . .

# Build the application
RUN g++ -o camera_app bsfast.cpp $(pkg-config --cflags --libs opencv4) -lpylonbase -lpylonutility

# Ensure the output directory for frames exists
RUN mkdir -p significant_changes_frames

# Expose a port if required (adjust based on your application)
EXPOSE 8080

# Entry point for the application
CMD ["./camera_app"]