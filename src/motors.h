#pragma once

void setupMotors();
void driveForward();
void driveBackward();
void turnLeft();
void turnRight();
void stopMotors();
void driveWithSpeed(int forwardPercent, int turnPercent);
bool motorsAreRunning();
int getLeftSpeed();
int getRightSpeed();
