/**
 * @file door_sensor.h
 * @author Victor Oliveira (victor.rsoliveira@gmail.com)
 * @brief
 * @version 0.1
 * @date 2025-06-15
 *
 */

#ifndef BT_APP_DOOR_SENSOR_H
#define BT_APP_DOOR_SENSOR_H

enum door_sensor_status {
    DOOR_IS_OPEN,
    DOOR_IS_CLOSED,
    DOOR_IS_OPENING,
    DOOR_IS_CLOSING
};

int bt_app_door_sensor_init(void);

enum door_sensor_status bt_app_door_sensor_get_status(void);

char * bt_app_door_sensor_get_status_string(enum door_sensor_status status);

int bt_app_door_sensor_set_status(enum door_sensor_status status);

#endif  // BT_APP_DOOR_SENSOR_H