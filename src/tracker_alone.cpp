#include "eunomia/model/tracker_alone.h"

#include <spdlog/spdlog.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void tracker_alone_base::start_child_process()
{
  int res = 0;
  // close unused pipe end
  res = close(pipe_fd[0]);
  if (res == -1)
  {
    spdlog::error("parent process close pipe failed");
    exit(1);
  }

  // redirect stdout to pipe
  res = dup2(pipe_fd[1], STDOUT_FILENO);
  if (res == -1)
  {
    spdlog::error("child process dup2 failed");
    exit(1);
  }
  const auto& env = current_config.env;

  std::vector<char*> argv;
  for (auto& arg : env.process_args)
  {
    argv.push_back(const_cast<char*>(arg.c_str()));
  }
  argv.push_back(nullptr);
  // start child process ebpf program
  res = env.main_func(env.process_args.size(), argv.data());
  // exit child process
  exit(res);
}

void tracker_alone_base::start_parent_process()
{
  int res = 0;
  // close unused pipe end
  res = close(pipe_fd[1]);
  if (res == -1)
  {
    spdlog::error("parent process close pipe failed");
    return;
  }
  // read from pipe
  while ((!exiting) && (res = read(pipe_fd[0], pipe_buf, MAX_PROCESS_MESSAGE_LENGTH)) > 0)
  {
    // send to event handler
    auto event = tracker_event<tracker_alone_event>{ std::string(pipe_buf, res) };
    if (current_config.handler)
    {
      current_config.handler->do_handle_event(event);
    }
    else
    {
      std::cout << "warn: no handler for tracker event" << std::endl;
      break;
    }
  }
  // close pipe
  res = close(pipe_fd[0]);
  if (res == -1)
  {
    spdlog::error("parent process close pipe failed");
    return;
  }
  res = waitpid(child_pid, nullptr, 0);
  if (res == -1)
  {
    spdlog::error("parent process waitpid failed");
    return;
  }
}

void tracker_alone_base::start_tracker()
{
  int res = 0;
  res = pipe(pipe_fd);
  if (res < 0)
  {
    spdlog::error("create pipe error");
    return;
  }

  res = fork();
  if (res == 0)
  {
    // child process
    start_child_process();
    assert(false && "The program should not be here");
  }
  else if (res > 0)
  {
    // parent process
    child_pid = res;
    start_parent_process();
  }
  else
  {
    // error
    spdlog::error("fork error");
    close(pipe_fd[0]);
    close(pipe_fd[1]);
  }
}

tracker_alone_base::tracker_alone_base(config_data config) : tracker_with_config(config)
{
  exiting = false;
  this->current_config.env.exiting = &exiting;
}

std::unique_ptr<tracker_alone_base> tracker_alone_base::create_tracker_with_default_env(tracker_event_handler handler)
{
  config_data config;
  config.handler = handler;
  config.name = "process_tracker";
  config.env = tracker_alone_env{ 0 };
  return std::make_unique<tracker_alone_base>(config);
}

tracker_alone_base::tracker_alone_base(tracker_alone_env env)
    : tracker_alone_base(config_data{
          .env = env,
      })
{
}
