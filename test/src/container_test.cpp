/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
 *
 * Copyright (c) 2022, 郑昱笙，濮雯旭，张典典（牛校牛子队）
 * All rights reserved.
 */

#include "eunomia/tracker_manager.h"
#include "eunomia/container.h"

using namespace std::chrono_literals;

int main(int argc, char **argv)
{ 
  container_manager manager;
  std::cout << "start ebpf...\n";

  manager.start_container_tracing("./logs/container_log.txt");

  std::this_thread::sleep_for(100s);
  return 0;
}


/*
TEST(TmpAddTest, CheckValues)
{
  ASSERT_EQ(tmp::add(1, 2), 3);
  EXPECT_TRUE(true);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
*/
