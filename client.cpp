#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <iostream>
#include <sstream>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

// Namespace and type definitions
typedef websocketpp::client<websocketpp::config::asio_client> client;
using websocketpp::connection_hdl;

// WebSocket client instance and state variables
client ws_client_instance;
connection_hdl ws_hdl;
bool ws_connected = false;

// WebSocket event handlers
void on_open(client* c, connection_hdl hdl) {
    std::cout << "[WebSocket] Connection opened." << std::endl;
    ws_hdl = hdl;
    ws_connected = true;
}

void on_fail(client* c, connection_hdl hdl) {
    std::cerr << "[WebSocket] Connection failed!" << std::endl;
    ws_connected = false;
}

void initialize_websocket(const std::string& uri) {
    try {
        ws_client_instance.init_asio();

        // Set WebSocket handlers
        ws_client_instance.set_open_handler(bind(&on_open, &ws_client_instance, std::placeholders::_1));
        ws_client_instance.set_fail_handler(bind(&on_fail, &ws_client_instance, std::placeholders::_1));

        websocketpp::lib::error_code ec;
        auto con = ws_client_instance.get_connection(uri, ec);
        if (ec) {
            throw std::runtime_error("[WebSocket] Connection error: " + ec.message());
        }

        ws_client_instance.connect(con);
        std::thread ws_thread([&]() { ws_client_instance.run(); });
        ws_thread.detach();

        // Wait for connection
        std::this_thread::sleep_for(std::chrono::seconds(2));
        if (!ws_connected) {
            throw std::runtime_error("[WebSocket] Unable to establish connection.");
        }
    } catch (const std::exception& e) {
        std::cerr << "[WebSocket] Exception: " << e.what() << std::endl;
        throw;
    }
}

void send_frame_over_websocket(const cv::Mat& frame) {
    if (!ws_connected) {
        std::cerr << "[WebSocket] Not connected, cannot send frame." << std::endl;
        return;
    }

    try {
        ws_client_instance.send(ws_hdl, buf.data(), buf.size(), websocketpp::frame::opcode::binary);
        std::cout << "[WebSocket] Frame sent." << std::endl;
    } catch (const websocketpp::exception& e) {
        std::cerr << "[WebSocket] Send error: " << e.what() << std::endl;
    }
}
