/**
 * MAKCU CLI Interface for Python Integration
 * ==========================================
 * 
 * Command-line interface for the MAKCU C++ library that enables
 * Python scripts to access high-performance mouse control.
 * 
 * This allows the Python BetterAifinal.py script to use the
 * ultra-fast C++ implementation (0.07ms movements) instead of
 * the slower Python alternatives.
 */

#include "makcu-cpp/include/makcu.h"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <chrono>

// Global device instance for persistent connection
static makcu::Device* g_device = nullptr;
static bool g_high_performance_mode = false;

/**
 * Parse command line arguments into command and parameters
 */
struct Command {
    std::string action;
    std::vector<std::string> params;
};

Command parseCommand(const std::string& cmdString) {
    Command cmd;
    std::istringstream iss(cmdString);
    std::string part;
    
    if (std::getline(iss, part, ':')) {
        cmd.action = part;
        
        std::string params_str;
        if (std::getline(iss, params_str)) {
            std::istringstream params_iss(params_str);
            std::string param;
            while (std::getline(params_iss, param, ',')) {
                cmd.params.push_back(param);
            }
        }
    }
    
    return cmd;
}

/**
 * Initialize device connection
 */
bool initializeDevice(const std::string& port = "") {
    if (g_device) {
        delete g_device;
    }
    
    g_device = new makcu::Device();
    
    if (g_device->connect(port)) {
        if (g_high_performance_mode) {
            g_device->enableHighPerformanceMode(true);
        }
        return true;
    }
    
    delete g_device;
    g_device = nullptr;
    return false;
}

/**
 * Clean up device connection
 */
void cleanupDevice() {
    if (g_device) {
        g_device->disconnect();
        delete g_device;
        g_device = nullptr;
    }
}

/**
 * Execute individual commands for maximum performance
 */
int executeCommand(const Command& cmd) {
    try {
        // Connection commands
        if (cmd.action == "connect") {
            std::string port = cmd.params.empty() ? "" : cmd.params[0];
            if (initializeDevice(port)) {
                std::cout << "connected:" << port << std::endl;
                return 0;
            } else {
                std::cout << "connection_failed" << std::endl;
                return 1;
            }
        }
        
        if (cmd.action == "disconnect") {
            cleanupDevice();
            std::cout << "disconnected" << std::endl;
            return 0;
        }
        
        if (cmd.action == "enable_high_performance") {
            g_high_performance_mode = (cmd.params.empty() || cmd.params[0] == "true");
            if (g_device && g_high_performance_mode) {
                g_device->enableHighPerformanceMode(true);
            }
            std::cout << "high_performance:" << (g_high_performance_mode ? "enabled" : "disabled") << std::endl;
            return 0;
        }
        
        // Ensure device is connected for mouse operations
        if (!g_device || !g_device->isConnected()) {
            std::cout << "device_not_connected" << std::endl;
            return 1;
        }
        
        // Mouse movement commands (fire-and-forget for performance)
        if (cmd.action == "move") {
            if (cmd.params.size() >= 2) {
                int x = std::stoi(cmd.params[0]);
                int y = std::stoi(cmd.params[1]);
                
                // Fire-and-forget for maximum performance
                g_device->mouseMove(x, y);
                
                // No output for fire-and-forget mode (gaming performance)
                return 0;
            }
        }
        
        if (cmd.action == "move_smooth") {
            if (cmd.params.size() >= 2) {
                int x = std::stoi(cmd.params[0]);
                int y = std::stoi(cmd.params[1]);
                int segments = cmd.params.size() > 2 ? std::stoi(cmd.params[2]) : 10;
                
                g_device->mouseMoveSmooth(x, y, segments);
                return 0;
            }
        }
        
        // Mouse button commands
        if (cmd.action == "click") {
            int button = cmd.params.empty() ? 0 : std::stoi(cmd.params[0]);
            makcu::MouseButton mb = static_cast<makcu::MouseButton>(button);
            g_device->click(mb);
            return 0;
        }
        
        if (cmd.action == "press") {
            int button = cmd.params.empty() ? 0 : std::stoi(cmd.params[0]);
            makcu::MouseButton mb = static_cast<makcu::MouseButton>(button);
            g_device->mouseDown(mb);
            return 0;
        }
        
        if (cmd.action == "release") {
            int button = cmd.params.empty() ? 0 : std::stoi(cmd.params[0]);
            makcu::MouseButton mb = static_cast<makcu::MouseButton>(button);
            g_device->mouseUp(mb);
            return 0;
        }
        
        // Mouse wheel command
        if (cmd.action == "scroll") {
            int delta = cmd.params.empty() ? 1 : std::stoi(cmd.params[0]);
            g_device->mouseWheel(delta);
            return 0;
        }
        
        // Mouse locking commands
        if (cmd.action == "lock_x") {
            bool lock = cmd.params.empty() || cmd.params[0] == "1";
            g_device->lockMouseX(lock);
            return 0;
        }
        
        if (cmd.action == "lock_y") {
            bool lock = cmd.params.empty() || cmd.params[0] == "1";
            g_device->lockMouseY(lock);
            return 0;
        }
        
        // Query commands
        if (cmd.action == "status") {
            if (g_device && g_device->isConnected()) {
                std::cout << "connected" << std::endl;
            } else {
                std::cout << "disconnected" << std::endl;
            }
            return 0;
        }
        
        if (cmd.action == "version") {
            if (g_device && g_device->isConnected()) {
                std::cout << g_device->getVersion() << std::endl;
            } else {
                std::cout << "device_not_connected" << std::endl;
            }
            return 0;
        }
        
        // Performance test command
        if (cmd.action == "performance_test") {
            if (!g_device || !g_device->isConnected()) {
                std::cout << "device_not_connected" << std::endl;
                return 1;
            }
            
            // Enable high-performance mode and profiling
            g_device->enableHighPerformanceMode(true);
            makcu::PerformanceProfiler::enableProfiling(true);
            
            auto start = std::chrono::high_resolution_clock::now();
            
            // Test rapid movements
            for (int i = 0; i < 100; ++i) {
                g_device->mouseMove(i % 5, i % 5);
            }
            
            auto end = std::chrono::high_resolution_clock::now();
            auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            
            std::cout << "performance_test_result:100_movements:" << duration_ms << "ms" << std::endl;
            return 0;
        }
        
        // Unknown command
        std::cout << "unknown_command:" << cmd.action << std::endl;
        return 1;
        
    } catch (const std::exception& e) {
        std::cout << "error:" << e.what() << std::endl;
        return 1;
    }
}

/**
 * Main entry point for CLI interface
 */
int main(int argc, char* argv[]) {
    // Set up cleanup handler
    std::atexit(cleanupDevice);
    
    if (argc < 3 || std::string(argv[1]) != "--command") {
        std::cout << "Usage: " << argv[0] << " --command <command_string>" << std::endl;
        std::cout << std::endl;
        std::cout << "Commands:" << std::endl;
        std::cout << "  connect[:port]                 - Connect to device" << std::endl;
        std::cout << "  disconnect                     - Disconnect device" << std::endl;
        std::cout << "  enable_high_performance:true   - Enable gaming mode" << std::endl;
        std::cout << "  move:x,y                       - Move mouse (fire-and-forget)" << std::endl;
        std::cout << "  move_smooth:x,y,segments       - Smooth mouse movement" << std::endl;
        std::cout << "  click:button                   - Click mouse button (0=left, 1=right)" << std::endl;
        std::cout << "  press:button                   - Press mouse button" << std::endl;
        std::cout << "  release:button                 - Release mouse button" << std::endl;
        std::cout << "  scroll:delta                   - Scroll mouse wheel" << std::endl;
        std::cout << "  lock_x:1                       - Lock X-axis movement" << std::endl;
        std::cout << "  lock_y:1                       - Lock Y-axis movement" << std::endl;
        std::cout << "  status                         - Get connection status" << std::endl;
        std::cout << "  version                        - Get firmware version" << std::endl;
        std::cout << "  performance_test               - Run performance test" << std::endl;
        std::cout << std::endl;
        std::cout << "Performance: 0.07ms movements, 0.16ms clicks (28x faster than Python)" << std::endl;
        return 1;
    }
    
    std::string commandString = argv[2];
    Command cmd = parseCommand(commandString);
    
    return executeCommand(cmd);
}