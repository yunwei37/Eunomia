#ifndef CONTAINER_INFO_H
#define CONTAINER_INFO_H

#include <string>

// statues of container
enum class container_status
{
  RUNNING,
  EXITED,
  INVALID,
};

static container_status container_status_from_str(const std::string &s)
{
  if (s == "running")
  {
    return container_status::RUNNING;
  }
  else if (s == "exited")
  {
    return container_status::EXITED;
  }
  else
  {
    return container_status::INVALID;
  }
}

//  container info
struct container_info
{
  std::string id;
  std::string name;
  container_status status;
};

#endif