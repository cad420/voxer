#pragma once
#include "ParallelRenderer/GenerateSciVis.h"
#include <map>
#include <string>
#include <vector>

struct User {
  std::string id;
  std::map<std::string, gensv::LoadedVolume> volumes;
  User(std::string id) : id(id){};
  void load(std::string volume);
  gensv::LoadedVolume& get(std::string volume);
};
typedef struct User User;

struct UserManager {
  std::map<std::string, User> users;
  UserManager();
  User &get(std::string id);
};
typedef struct UserManager UserManager;
