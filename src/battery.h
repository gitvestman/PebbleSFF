#pragma once

extern Layer *s_battery_layer;

void save_battery_state(BatteryChargeState state);
void battery_layer_update_callback(Layer *layer, GContext *ctx);
