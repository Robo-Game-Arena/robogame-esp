#pragma once

void setupArm();
void updateArm();
void moveShoulder(int angleStep);
void moveElbow(int angleStep);
void openGripper();
void closeGripper();
int getShoulderAngle();
int getElbowAngle();
bool gripperIsPowered();
