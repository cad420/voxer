#pragma once
#include <map>
#include <ospray/ospray_cpp.h>
#include <string>
#include <vector>

struct User {
  std::string id;
  std::map<std::string, ospray::cpp::Volume> volumes;
  User(std::string id) : id(id){};
  void load(std::string volume);
  ospray::cpp::Volume &get(std::string volume);
};
typedef struct User User;

struct UserManager {
  std::map<std::string, User> users;
  UserManager();
  User &get(std::string id);
};
typedef struct UserManager UserManager;
