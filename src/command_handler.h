#pragma once

#include <stddef.h>

void handleCommands(const char *commands, size_t length);
void updateDriveTimeout();
void setRosConnected(bool connected);
bool rosHasControl();
char getCurrentDriveCommand();
