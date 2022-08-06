#include "eunomia/http_server.h"

eunomia_server::eunomia_server() {
    port = 8080;
    start_tracking();
    end_trackting();
    server.listen("0.0.0.0", port);
}
eunomia_server::eunomia_server(int p):port(p) {
    start_tracking();
    end_trackting();
    server.listen("0.0.0.0", port);
}

void eunomia_server::start_tracking() {
    server.Get("/start", [](const httplib::Request &req, httplib::Response &res) {
        std::string req_str("Start ");
        eunomia_config_data core_config;
        core_config.enabled_trackers.clear();
        for (auto [_, tracker_name] : req.params)
        {
            req_str += tracker_name;
            core_config.enabled_trackers.push_back(tracker_config_data{ .name = tracker_name});
            req_str += ",";
        }
        eunomia_core core(core_config);
        core.start_eunomia();
        res.status = 200;
        res.set_content(req_str, "text/plain");
    });
}

void eunomia_server::end_trackting() {

}