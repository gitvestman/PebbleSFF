#include <pebble.h>
#include "battery.h"
  
Layer *s_battery_layer;

static GPath *s_bolt_path_ptr = NULL;
static GPath *s_battery_path_ptr = NULL;

static const GPathInfo BOLT_PATH_INFO = {
  .num_points = 6,
  .points = (GPoint []) {{15, 0}, {10, 31}, {20, 31}, {5, 70}, {10, 39}, {0, 39}}
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

static void handle_battery(BatteryChargeState state) {
  charge_state = state;
  layer_mark_dirty(s_battery_layer);
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
    graphics_context_set_fill_color(ctx,  COLOR_FALLBACK(GColorYellow, GColorWhite));
    gpath_draw_filled(ctx, s_bolt_path_ptr);
    // Stroke the bolt:
    graphics_context_set_stroke_color(ctx, GColorBlack);
    gpath_draw_outline(ctx, s_bolt_path_ptr);
  } else if (charge_state.charge_percent > 0) {
    int height = (137 * charge_state.charge_percent / 100);
    graphics_context_set_fill_color(ctx, COLOR_FALLBACK(GColorCeleste, GColorWhite));
    //graphics_fill_rect(ctx, GRect(6, 20, 16, 40), 2, GCornersAll );    
    APP_LOG(APP_LOG_LEVEL_INFO, "Battery %d (%d%%)", height, charge_state.charge_percent);
    graphics_fill_rect(ctx, GRect(6, 150 - height, 15, height + 7), 2, GCornersAll );    
  }
}

void init_battery(Window *window) {
  // Subscribe to battery status
  battery_state_service_subscribe(handle_battery);
  
  // Create Battery layer
  s_battery_layer = layer_create(GRect(120, 0, 144, 168));
  layer_set_update_proc(s_battery_layer, battery_layer_update_callback);
  layer_add_child(window_get_root_layer(window), s_battery_layer);
  charge_state = battery_state_service_peek();
}

void destroy_battery_layer() {
  battery_state_service_unsubscribe();
}
