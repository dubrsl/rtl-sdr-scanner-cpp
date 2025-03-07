#pragma once

#include <mosquitto.h>
#include <config.h>

#include <atomic>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

class Mqtt {
 public:
  Mqtt(const Config& config);
  ~Mqtt();

  void publish(const std::string& topic, const std::string& data);
  void publish(const std::string& topic, const std::vector<uint8_t>& data);
  void publish(const std::string& topic, const std::vector<uint8_t>&& data);
  void setMessageCallback(std::function<void(const std::string&, const std::string&)> callback);

 private:
  void onConnect();
  void onDisconnect();
  void onMessage(const mosquitto_message* message);

  mosquitto* m_client;
  std::atomic_bool m_isRunning;
  std::thread m_thread;
  std::mutex m_mutex;
  std::queue<std::tuple<std::string, std::vector<uint8_t>>> m_messages;
  std::vector<std::function<void(const std::string&, const std::string&)>> m_callbacks;
};
