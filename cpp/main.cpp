#include "httplib.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

int main() {
    httplib::Server server;

    server.Get("/metrics", [](const httplib::Request&, httplib::Response& res) {
        json data = {
            {"cpu", 42.5},
            {"ram", 68.2},
            {"status", "ok"}
        };

        res.set_content(data.dump(), "application/json");
    });

    server.listen("0.0.0.0", 8080);
}
