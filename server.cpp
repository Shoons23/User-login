#include <iostream>
#include "cpp-httplib/httplib.h"
#include <nlohmann/json.hpp> 

using json = nlohmann::json;

void setup_cors(httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*"); 
    res.set_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
}

int main() {
    httplib::Server svr;

    // POST 
    svr.Post("/login", [](const httplib::Request& req, httplib::Response& res) {
        setup_cors(res);  // CORS header

        // json parsing
        auto j = json::parse(req.body);
        std::string username = j["username"];
        std::string password = j["password"];

        // 
        std::cout << "Received Username: " << username << std::endl;
        std::cout << "Received Password: " << password << std::endl;

        // 
        res.set_content("Login data received!", "text/plain");
    });

    // CORS  OPTIONS 
    svr.Options("/login", [](const httplib::Request& req, httplib::Response& res) {
        setup_cors(res);  // 
        res.status = 200; // 
    });

    // 
    std::cout << "Starting server on port 8080..." << std::endl;
    svr.listen("localhost", 8080);

    return 0;
}
