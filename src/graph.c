#include <pebble.h>
#include "graph.h"

#define BOX_WIDTH 114
#define BOX_HEIGHT 51
#define TITLE_BAR_SIZE 16

#define ANIM_DURATION 500
#define ANIM_DELAY 300

static Layer *s_graph_layer;
static Layer *s_status_layer;
static PropertyAnimation *s_graph_animation;
static PropertyAnimation *s_status_animation;
char statusbuffer[30];

static GPath *s_graph_path_ptr = NULL;

static const GPathInfo GRAPH_PATH_INFO = {
  .num_points = 14,
  .points = (GPoint []) {
    {2, 50}, {2, 31}, {12, 32}, {22, 10}, {32, 39}, {42, 35}, {52, 42}, {62, 38}, 
  {72, 40}, {82, 40}, {92, 40}, {102, 40}, {112, 40}, {112, 50}}
};

static void anim_stopped_handler(Animation *animation, bool finished, void *context) {
  // Free the animation
#ifdef PBL_PLATFORM_APLITE   
  property_animation_destroy((PropertyAnimation*)animation);
#endif
}

static void anim_started_handler(Animation *animation, void *context) {
  layer_set_hidden(s_graph_layer, false);
}

void graph_layer_update_callback(Layer *layer, GContext* ctx) {
  // Fill the background:
  graphics_context_set_fill_color(ctx, COLOR_FALLBACK(GColorYellow, GColorWhite));  
  graphics_fill_rect(ctx, GRect(0, 0, BOX_WIDTH, BOX_HEIGHT), 2, GCornersAll);
    
  // graph
  graphics_context_set_fill_color(ctx, GColorBlack);  
  gpath_draw_filled(ctx, s_graph_path_ptr);
}

void status_layer_update_callback(Layer *layer, GContext* ctx) {
  // Fill the background:
  graphics_context_set_fill_color(ctx, GColorBlack);  
  graphics_fill_rect(ctx, GRect(0, 0, BOX_WIDTH, BOX_HEIGHT), 2, GCornersAll);
    
  // Status
  graphics_context_set_fill_color(ctx, GColorBlack);  
  graphics_context_set_stroke_color(ctx, GColorWhite);  
  graphics_context_set_text_color(ctx, GColorWhite);
  graphics_draw_round_rect(ctx, layer_get_bounds(layer), 2);
  graphics_draw_text(ctx, statusbuffer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21), 
                    GRect(4, 0, 120, 40), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
}

void init_graph(Window *window) {
  // Create Graph Layer
  s_graph_layer = layer_create(GRect(2, 60, BOX_WIDTH, BOX_HEIGHT));
  layer_add_child(window_get_root_layer(window), s_graph_layer);
  layer_set_hidden(s_graph_layer, true);
  
  // Create Status Layer
  s_status_layer = layer_create(GRect(-120, 120, 120, 47));
  layer_add_child(window_get_root_layer(window), s_status_layer);
  snprintf(statusbuffer, 30, "Totalt: 0\nUser: 0"); 
  
  s_graph_path_ptr = gpath_create(&GRAPH_PATH_INFO);
  
  layer_set_update_proc(s_graph_layer, graph_layer_update_callback);
  layer_set_update_proc(s_status_layer, status_layer_update_callback);
}

void destroy_graph() {
  // Destroy Layer
  layer_destroy(s_graph_layer);
  layer_destroy(s_status_layer);
  gpath_destroy(s_graph_path_ptr);
}

void schedule_graph_animation(GRect start, GRect finish) {
  // Schedule the graph animation
  s_graph_animation = property_animation_create_layer_frame(s_graph_layer, &start, &finish);
  animation_set_duration((Animation*)s_graph_animation, ANIM_DURATION);
  animation_set_delay((Animation*)s_graph_animation, ANIM_DELAY);
  animation_set_curve((Animation*)s_graph_animation, AnimationCurveEaseInOut);
  animation_set_handlers((Animation*)s_graph_animation, (AnimationHandlers) {
    .started = anim_started_handler,
    .stopped = anim_stopped_handler
  }, NULL);
  animation_schedule((Animation*)s_graph_animation);
}

void schedule_status_animation(GRect start, GRect finish) {
  s_status_animation = property_animation_create_layer_frame(s_status_layer, &start, &finish);
  animation_set_duration((Animation*)s_status_animation, ANIM_DURATION);
  animation_set_delay((Animation*)s_status_animation, ANIM_DELAY);
  animation_set_curve((Animation*)s_status_animation, AnimationCurveEaseInOut);
  animation_set_handlers((Animation*)s_status_animation, (AnimationHandlers) {
    .stopped = anim_stopped_handler
  }, NULL);
  animation_schedule((Animation*)s_status_animation);

}

// Start animation loop
void animate_graph() {
  APP_LOG(APP_LOG_LEVEL_INFO, "Show graph");
  //layer_set_hidden(s_box_layer, false);
   // Determine start and finish positions
  GRect start, finish;

  start = GRect(50, 60, 0, BOX_HEIGHT);
  finish = GRect(3, 60, BOX_WIDTH, BOX_HEIGHT);
  
  schedule_graph_animation(start, finish);

  GRect starts, finishs;

  starts = GRect(-120, 120, 120, 47);
  finishs = GRect(0, 120, 120, 47);
  
  schedule_status_animation(starts, finishs);
}

void hide_graph() {
  APP_LOG(APP_LOG_LEVEL_INFO, "Hide graph");
  //layer_set_hidden(s_box_layer, true);
  // Determine start and finish positions
  GRect start, finish;

  finish = GRect(50, 60, 0, BOX_HEIGHT);
  start = GRect(4, 60, BOX_WIDTH, BOX_HEIGHT);
  
  GRect starts, finishs;

  schedule_graph_animation(start, finish);

  finishs = GRect(-120, 120, 120, 47);
  starts = GRect(0, 120, 120, 47);
  
  schedule_status_animation(starts, finishs);
}

void update_graph(int *data) {
  int i, y;
  for (i = 0; i < 12; i++) {
    y = 50 - data[i] / 5;    
    GRAPH_PATH_INFO.points[i+1].y = y < 0 ? 0 : y;
  }
  layer_mark_dirty(s_graph_layer);
}
