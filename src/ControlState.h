#ifndef CONTROL_STATE_H
#define CONTROL_STATE_H

struct ControlState
{
    int steering = 0;
    int throttle = 0;
    int reverse = 0;
    char traction[4] = "";
    bool lights = false;
};

#endif