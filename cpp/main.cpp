#include "httplib.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

int main() {
    httplib::Server server;

    // CORS preflight
    server.Options("/metrics", [](const httplib::Request&, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "http://localhost");
        res.set_header("Access-Control-Allow-Methods", "GET, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.status = 204;
    });

    server.Get("/metrics", [](const httplib::Request&, httplib::Response& res) {
        json data = {
            {"cpu", 55},
            {"ram", 73}
        };

        res.set_header("Access-Control-Allow-Origin", "http://localhost");
        res.set_content(data.dump(), "application/json");
    });

    server.listen("0.0.0.0", 80);
}
