#pragma once
#include <map>
#include <ospray/ospray_cpp.h>
#include <string>
#include <vector>

namespace voxer {
struct User {
  std::string id;
  std::map<std::string, ospray::cpp::Volume> volumes;
  User(std::string id) : id(id){};
  void load(std::string volume);
  ospray::cpp::Volume &get(std::string volume);
};

struct UserManager {
  std::map<std::string, User> users;
  UserManager();
  User &get(std::string id);
};
}
