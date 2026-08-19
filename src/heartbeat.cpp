#include <iostream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <thread>
#include <chrono>

using json = nlohmann::json;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void sendHeartbeat(const std::string& nodeId, const std::string& status, double latency) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    curl = curl_easy_init();
    if(curl) {
        std::string url = "https://firestore.googleapis.com/v1/projects/proteus-mesh-core-dd2e1/databases/(default)/documents/nodes/" + nodeId + "?key=YOUR_API_KEY";
        json data;
        data["fields"]["id"]["stringValue"] = nodeId;
        data["fields"]["status"]["stringValue"] = status;
        data["fields"]["latency"]["doubleValue"] = latency;
        data["fields"]["lastHeartbeat"]["timestampValue"] = "2026-01-01T00:00:00Z";
        data["fields"]["ip"]["stringValue"] = "192.168.1.100";
        data["fields"]["port"]["integerValue"] = 8080;
        std::string jsonStr = data.dump();

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStr.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
}

int main() {
    while(true) {
        sendHeartbeat("node-00", "active", 12.5);
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    return 0;
}
