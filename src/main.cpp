#include <string>
#include <chrono>
#include <thread>
#include <mutex>
#include <set>

#include "crow/crow_all.h"
#include "nlohmann/json.hpp"

#include "monitor.hpp"
using json = nlohmann::json;


std::set<crow::websocket::connection*> active_connections;
std::mutex conn_mutex;

int main() {
    crow::SimpleApp app;

    Monitor monitor;
    CROW_ROUTE(app, "/ws")
        .websocket(&app)
        .onopen([&](crow::websocket::connection& conn) {
            CROW_LOG_INFO << "Новое соединение";
            std::lock_guard<std::mutex> lock(conn_mutex);
            active_connections.insert(&conn);
        })
        .onclose([&](crow::websocket::connection& conn, const std::string& reason) {
            CROW_LOG_INFO << "Соединение закрыто: " << reason;
            std::lock_guard<std::mutex> lock(conn_mutex);
            active_connections.erase(&conn);
        });

    std::thread([&]() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            
            monitor.update();
            json response;
            response["total"] = monitor.getMemory().total_;
            response["free"] = monitor.getMemory().free_;
            response["available"] = monitor.getMemory().available_;
            response["cpu"] = monitor.getCpuUsage();

            response["processes"] = json::array();
            for (const auto& p : monitor.getProcesses()) {
                json p_json;
                p_json["pid"] = p.pid;
                p_json["name"] = p.name;
                p_json["mem"] = p.mem;
                p_json["status"] = std::string(1, p.status);
                p_json["threads"] = p.threads;
                p_json["proc_cpu"] = p.cpuUsage;
                p_json["user"] = p.user;
                response["processes"].push_back(p_json);
            }

            std::string msg = response.dump();
            std::lock_guard<std::mutex> lock(conn_mutex);
            for (auto conn : active_connections) {
                conn->send_text(msg);
            }
        }
    }).detach();

    app.port(8080).multithreaded().run();
    return 0;
}