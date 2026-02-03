#include "httplib.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

float i1 = 0.0;
float i2 = 0.0;

float cpu_usage() {
    if (i1 > 100.0) {
        i1 = 0.0;
    }
    i1 += 0.1;
    return i1;
}

float ram_usage() {
    if (i2 > 100.0) {
        i2 = 0.0;
    }
    i2 += 0.1;
    return i2;
}

int main() {
    httplib::Server server;

    server.Get("/metrics/stream", [](const httplib::Request&, httplib::Response& res) {
    res.set_header("Content-Type", "text/event-stream");
    res.set_header("Cache-Control", "no-cache");
    res.set_header("Connection", "keep-alive");
    res.set_header("Access-Control-Allow-Origin", "http://localhost");

    res.set_chunked_content_provider(
        "text/event-stream",
        [](size_t, httplib::DataSink& sink) {
            json data = {
                {"cpu", cpu_usage()},
                {"ram", ram_usage()}
            };

            std::string msg = "data: " + data.dump() + "\n\n";
            sink.write(msg.data(), msg.size());

            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            return true;
        }
    );
});


    server.listen("0.0.0.0", 80);
}
