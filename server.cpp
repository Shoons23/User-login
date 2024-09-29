#include <iostream>
#include <string>
#include "cpp-httplib/httplib.h"
#include <nlohmann/json.hpp> 
#include <fstream>
#include <unordered_map> // dictionary 형태: 빠른 검색 속도

using namespace std;
using json = nlohmann::json;

void setup_cors(httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*"); 
    res.set_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
}

class SimpleDatabase{
private:   
    unordered_map<string, string> users;
    const string db_filename = "user_db.txt";

    // read user data in file
    void loadDatabase(){
        ifstream file(db_filename);
        if(file.is_open()){
            string id, password;
            while(file >> id >> password){
                users[id] = password;
            }
            file.close();
        }
    }

    // save user data in file
    void saveDatabase(){
        ofstream file(db_filename);
        if(file.is_open()){
            for(const auto& user: users){
                file << user.first << " " << user.second << endl;
            }
            file.close();
        }
        
    };

public:
    SimpleDatabase() {
        loadDatabase();  // if program start, load database
    }

    ~SimpleDatabase() {
        saveDatabase();  // if program end, save database 
    }

    void addUser(const string& id, const string& password){
        users[id]= password;
        saveDatabase(); 
    }

    bool authenticate(const string& id, const string& password){
        if(users.find(id) != users.end() && users[id] == password){
            return true;
        }
        return false;
    }

    bool userExists(const std::string& id) {
        return users.find(id) != users.end();
    }

};

int main() {
    httplib::Server svr;
    SimpleDatabase db;

    // POST 
    // db object not captured within lambda function
    svr.Post("/login", [&db](const httplib::Request& req, httplib::Response& res) {
        setup_cors(res);  // CORS header

        try {
             // json parsing
            auto j = json::parse(req.body);
            std::string username = j["username"];
            std::string password = j["password"];

            // // server receive 
            std::cout << "Received Username: " << username << std::endl;
            std::cout << "Received Password: " << password << std::endl;

            res.set_content("Login data received!", "text/plain");
            
            json response;
            if (db.authenticate(username, password)) {
                response["status"] = "success";
                response["message"] = "Login successful!";
            } else {
                response["status"] = "fail";
                response["message"] = "Invalid username or password";
            }
            res.set_content(response.dump(), "application/json");
        } catch (...) {
            res.status = 400;
            res.set_content("Invalid request format", "text/plain");
        }
    });


    svr.Post("/signup", [&db](const httplib::Request& req, httplib::Response& res) {
        setup_cors(res);

        try {
            auto j = json::parse(req.body);
            std::string username = j["username"];
            std::string password = j["password"];

            json response;
            if (db.userExists(username)) {
                response["status"] = "fail";
                response["message"] = "User already exists";
            } else {
                db.addUser(username, password);
                response["status"] = "success";
                response["message"] = "Signup successful";
            }
            res.set_content(response.dump(), "application/json");

        } catch (...) {
            res.status = 400;
            res.set_content("Invalid request format", "text/plain");
        }
    });


    // CORS  OPTIONS 
    svr.Options("/login", [](const httplib::Request& req, httplib::Response& res) {
        setup_cors(res);  
        res.status = 200;
    });
    svr.Options("/signup", [](const httplib::Request& req, httplib::Response& res) {
        setup_cors(res);
        res.status = 200;
    });


    std::cout << "Starting server on port 8080..." << std::endl;
    svr.listen("localhost", 8080);

    return 0;
}