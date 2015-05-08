#include <pebble.h>
#include "graph.h"

#define BOX_WIDTH 115
#define BOX_HEIGHT 50
#define TITLE_BAR_SIZE 16

#define ANIM_DURATION 500
#define ANIM_DELAY 300

static Layer *s_box_layer;
static PropertyAnimation *s_box_animation;

static GPath *s_graph_path_ptr = NULL;

static const GPathInfo GRAPH_PATH_INFO = {
  .num_points = 8,
  .points = (GPoint []) {{15, 50}, {25, 31}, {35, 32}, {45, 10}, {55, 39}, {65, 35}, {75, 42}, {85, 50}}
};

static void anim_stopped_handler(Animation *animation, bool finished, void *context) {
  // Free the animation
  property_animation_destroy(s_box_animation);
}

static void anim_started_handler(Animation *animation, void *context) {
  layer_set_hidden(s_box_layer, false);
}

void graph_layer_update_callback(Layer *layer, GContext* ctx) {
  APP_LOG(APP_LOG_LEVEL_INFO, "graph_layer_update_callback");
  // Fill the path:
  graphics_context_set_fill_color(ctx, COLOR_FALLBACK(GColorYellow, GColorWhite));  
  graphics_fill_rect(ctx, GRect(0, 0, BOX_WIDTH, BOX_HEIGHT), 2, GCornersAll);
    
  // Staples
  gpath_draw_outline(ctx, s_graph_path_ptr);

  // Stroke the path:
  //graphics_context_set_stroke_color(ctx, GColorWhite);
  //graphics_draw_rect(ctx, GRect(0, 0, BOX_WIDTH-1, BOX_HEIGHT-1));
}

void init_graph(Window *window) {
  // Create Layer
  s_box_layer = layer_create(GRect(2, 60, BOX_WIDTH, BOX_HEIGHT));
  //layer_set_background_color(s_box_layer, GColorWhite);
  layer_add_child(window_get_root_layer(window), s_box_layer);
  layer_set_hidden(s_box_layer, true);
  
  s_graph_path_ptr = gpath_create(&GRAPH_PATH_INFO);
  
  layer_set_update_proc(s_box_layer, graph_layer_update_callback);
}

void destroy_graph() {
  // Destroy Layer
  layer_destroy(s_box_layer);
}

void schedule_animation(GRect start, GRect finish) {
  // Schedule the animation
  s_box_animation = property_animation_create_layer_frame(s_box_layer, &start, &finish);
  animation_set_duration((Animation*)s_box_animation, ANIM_DURATION);
  animation_set_delay((Animation*)s_box_animation, ANIM_DELAY);
  animation_set_curve((Animation*)s_box_animation, AnimationCurveEaseInOut);
  animation_set_handlers((Animation*)s_box_animation, (AnimationHandlers) {
    .started = anim_started_handler,
    .stopped = anim_stopped_handler
  }, NULL);
  animation_schedule((Animation*)s_box_animation);
}

// Start animation loop
void animate_graph() {
  APP_LOG(APP_LOG_LEVEL_INFO, "Show graph");
  layer_set_hidden(s_box_layer, true);
   // Determine start and finish positions
  //GRect start, finish;

  //start = GRect(BOX_WIDTH/2, 60, 0, BOX_HEIGHT);
  //finish = GRect(2, 60, BOX_WIDTH, BOX_HEIGHT);
  
  //schedule_animation(start, finish);
}

void hide_graph() {
  APP_LOG(APP_LOG_LEVEL_INFO, "Hide graph");
  layer_set_hidden(s_box_layer, true);
  // Determine start and finish positions
  //GRect start, finish;

  //finish = GRect(BOX_WIDTH/2, 60, 0, BOX_HEIGHT);
  //start = GRect(2, 60, BOX_WIDTH, BOX_HEIGHT);
  
  //schedule_animation(start, finish);
}
