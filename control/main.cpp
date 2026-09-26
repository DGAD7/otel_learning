#include "led/led.h"
#include "led_control.h"

#include "grpcpp/grpcpp.h"
#include "grpcpp/security/server_credentials.h"

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <thread>

namespace
{

std::unique_ptr<grpc::Server> server;

std::string GetEnvOrDefault(const char *env_name, const std::string &default_address)
{
    const char *value = std::getenv(env_name);
    return value != nullptr ? std::string(value) : default_address;
}

std::atomic<bool> grpc_shutdown_requested{false};

void handle_shutdown_request(int /*signal*/)
{
    grpc_shutdown_requested.store(true);
}

}

int main()
{
    std::cout << "Hello OTEL" << std::endl;

    const auto led = std::make_shared<led::led>();

    const auto led_controller_service = std::make_shared<led_control::led_control>(led);

    const std::string listen_address = GetEnvOrDefault("LED_CONTROL_LISTEN_ADDRESS", "0.0.0.0:50051");
    grpc::ServerBuilder builder;
    builder.AddListeningPort(listen_address, grpc::InsecureServerCredentials());
    builder.RegisterService(led_controller_service.get());
    server = builder.BuildAndStart();

    std::signal(SIGINT, handle_shutdown_request);
    std::signal(SIGTERM, handle_shutdown_request);

    std::thread shutdown_watcher([]()
                                 {
                                     while (!grpc_shutdown_requested.load())
                                     {
                                         std::this_thread::sleep_for(std::chrono::milliseconds(100));
                                     }
                                     server->Shutdown();
                                 });

    server->Wait();

    grpc_shutdown_requested.store(true);
    shutdown_watcher.join();

    return 0;
}
