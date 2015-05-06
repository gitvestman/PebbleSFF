#include <pebble.h>
#include "battery.h"
  
Layer *s_battery_layer;

static GPath *s_bolt_path_ptr = NULL;
static GPath *s_battery_path_ptr = NULL;

static const GPathInfo BOLT_PATH_INFO = {
  .num_points = 6,
  .points = (GPoint []) {{15, 0}, {10, 26}, {20, 26}, {5, 60}, {10, 34}, {0, 34}}
};

static const GPathInfo BATTERY_PATH_INFO = {
  .num_points = 9,
  .points = (GPoint []) {{0, 10}, {5, 10}, {5, 0}, {15, 0}, {15, 10}, {20, 10}, {20, 160}, {0, 160}, {0, 10}}
};


static BatteryChargeState charge_state = { .is_charging = false, .charge_percent = 100 };

void setup_my_paths(void) {
  s_bolt_path_ptr = gpath_create(&BOLT_PATH_INFO);
  gpath_move_to(s_bolt_path_ptr, GPoint(2, 20));
  s_battery_path_ptr = gpath_create(&BATTERY_PATH_INFO);
  gpath_move_to(s_battery_path_ptr, GPoint(3, 0));
}

void save_battery_state(BatteryChargeState state) {
  charge_state = state;
}

void battery_layer_update_callback(Layer *layer, GContext *ctx) {
  setup_my_paths();

  // Stroke the battery:
  #ifdef PBL_COLOR
    graphics_context_set_stroke_width(ctx, 2);
  #endif
  graphics_context_set_stroke_color(ctx, GColorWhite);
  gpath_draw_outline(ctx, s_battery_path_ptr);
  #ifndef PBL_COLOR
    gpath_move_to(s_battery_path_ptr, GPoint(2, 0));
    gpath_draw_outline(ctx, s_battery_path_ptr);
  #endif

  if (charge_state.is_charging) {
    // Fill the bolt:
    graphics_context_set_fill_color(ctx, GColorWhite);
    gpath_draw_filled(ctx, s_bolt_path_ptr);
    // Stroke the bolt:
    graphics_context_set_stroke_color(ctx, GColorBlack);
    gpath_draw_outline(ctx, s_bolt_path_ptr);
  } else {
    int y = 144 - (131 * charge_state.charge_percent / 100);
    graphics_context_set_fill_color(ctx, COLOR_FALLBACK(GColorCeleste, GColorWhite));
    //graphics_fill_rect(ctx, GRect(6, 20, 16, 40), 2, GCornersAll );    
    graphics_fill_rect(ctx, GRect(6, y, 15, 144), 2, GCornersAll );    
  }
}