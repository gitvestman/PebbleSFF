#include <pebble.h>
#include "main.h"
#include "battery.h"
#include "graph.h"

static Window *s_main_window;
static TextLayer *s_time_layer;
static GFont s_time_font;
static GFont s_date_font;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;
static TextLayer *s_weather_layer;
static TextLayer *s_date_layer;
static GFont s_weather_font;

// Store incoming information
static char temperature_buffer[8];
static char conditions_buffer[32];
static char weather_layer_buffer[32];

static void main_window_load(Window *window) {
  // Create GBitmap, then set to created BitmapLayer
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SFF);
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_background_color(s_background_layer, GColorBlack);
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  bitmap_layer_set_alignment(s_background_layer, GAlignTopLeft);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
  
  // Create time textlayer
  s_time_layer = text_layer_create(GRect(4,6,110,45));
  text_layer_set_background_color(s_time_layer, COLOR_FALLBACK(GColorVeryLightBlue , GColorBlack));
  text_layer_set_text_color(s_time_layer, GColorWhite);
  
  // Create GFont
  //s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_22));
  s_date_font = fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21);
  s_time_font = fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS);
  text_layer_set_font(s_time_layer, s_time_font);
                                       
  // Improve layout to be more like watchface
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  // Add is as a child to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));

  // Create date Layer
  s_date_layer = text_layer_create(GRect(2, 119, 140, 26));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentLeft);
  text_layer_set_font(s_date_layer, s_date_font);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));

  // Create temperature Layer
  s_weather_layer = text_layer_create(GRect(2, 142, 140, 25));
  text_layer_set_background_color(s_weather_layer, GColorClear);
  #ifdef PBL_COLOR
    text_layer_set_text_color(s_weather_layer, GColorBabyBlueEyes);
  #else
    text_layer_set_text_color(s_weather_layer, GColorWhite);
  #endif
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentLeft);
  //text_layer_set_text(s_weather_layer, "Loading...");
  
  // Create second custom font, apply it and add to Window
  //s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_20));
  s_weather_font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  text_layer_set_font(s_weather_layer, s_weather_font);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));

  init_battery(window);
  
  init_graph(window);
  
  APP_LOG(APP_LOG_LEVEL_INFO, "Main windows load!");
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  //fonts_unload_custom_font(s_time_font);
  gbitmap_destroy(s_background_bitmap);
  bitmap_layer_destroy(s_background_layer);
  destroy_battery_layer();
  destroy_graph();
  // Destroy weather elements
  text_layer_destroy(s_weather_layer);
  //fonts_unload_custom_font(s_weather_font);
}

static char* getMonthName(int month) {
  switch(month) {
    case 0: return "Januari";
    case 1: return "Februari";
    case 2: return "Mars";
    case 3: return "April";
    case 4: return "Maj";
    case 5: return "Juni";
    case 6: return "Juli";
    case 7: return "Augusti";
    case 8: return "September";
    case 9: return "October";
    case 10: return "November";
    case 11: return "December";
    default: return "Trasigt";
  }  
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  // Create longed-lived buffers
  static char timebuffer[] = "00:00";
  static char datebuffer[] = "December 31";                              
  
  // Write the current hours and minutes into the buffer
  if (clock_is_24h_style()) {
    // use 24 hour format  
    strftime(timebuffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    strftime(timebuffer, sizeof(timebuffer), "%I, %M", tick_time);
  }  
  // Display the time on the TextLayer
  text_layer_set_text(s_time_layer, timebuffer);
  
  // Display the date
  //strftime(datebuffer, sizeof(datebuffer), "%B %d", tick_time);
  snprintf(datebuffer, sizeof(datebuffer), "%s %d", getMonthName(tick_time->tm_mon), tick_time->tm_mday);
  text_layer_set_text(s_date_layer, datebuffer);  
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  
  // Get weather update every 30 minutes
  if(tick_time->tm_min % 30 == 0) {  
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_uint8(iter, 1, 1);

    // Send the message!
    app_message_outbox_send();
  }
}

static uint32_t convert_bytes_to_int(uint8_t* bytes) {
  return (bytes[0] << 24) + (bytes[1] << 16) + (bytes[2] << 8) + bytes[3];
} 

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
    // Read first item
  Tuple *t = dict_read_first(iterator);
  static char bestUserBuffer[7];
  static char hours[12];
  static int data[12];
  static int bestUserCount;
  static int dailyTotal;
  bool weather = false;
  int i, j, length, start;
  
  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
    case KEY_TEMPERATURE:
      snprintf(temperature_buffer, sizeof(temperature_buffer), "%d°C", (int)t->value->int32);
      weather = true;
      break;
    case KEY_CONDITIONS:
      snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
      break;
    case KEY_BESTUSER:
      snprintf(bestUserBuffer, sizeof(bestUserBuffer), "%s", t->value->cstring);
      break;
    case KEY_BESTUSERCOUNT:
      bestUserCount = t->value->int32;
      break;
    case KEY_DAILYTOTAL:
      dailyTotal = t->value->int32;
      break;
    case KEY_HOURLYSTATS:
      length = t->length / 5;
      start = length > 12 ? length - 12 : 0;
      for (i = start, j = 0; i < length; i++, j++) {
        hours[j] = t->value->data[i*5];
        data[j] = convert_bytes_to_int(&t->value->data[i*5 + 1]);
      }
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }

    // Look for next item
    t = dict_read_next(iterator);
  }
  
  if (weather) {
    // Assemble full string and display
    snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
    text_layer_set_text(s_weather_layer, weather_layer_buffer);    
  } else {
    snprintf(statusbuffer, sizeof(statusbuffer), "Totalt: %d\n%s: %d", dailyTotal, bestUserBuffer, bestUserCount);
    update_graph(data);
    (void)hours;
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
  static bool graph = false;
  static long last_tap = 0;
  //static long first_tap = 0;

  time_t seconds;
  uint16_t milliseconds;
  time_ms(&seconds, &milliseconds);
  //APP_LOG(APP_LOG_LEVEL_INFO, "tap_handler: %u, %u", (uint16_t)seconds, milliseconds);  
  
  long time = seconds * 1000 + milliseconds;

  //APP_LOG(APP_LOG_LEVEL_INFO, "time - last_tap = %ld", time - last_tap);
  
  if (time - last_tap < 1000) {
    last_tap = time;
    return;
/*  } else if (time - first_tap < 100 || time - first_tap > 500) {
    first_tap = time;
    last_tap = time; 
    APP_LOG(APP_LOG_LEVEL_INFO, "first_tap = time - last_tap = %ld", time - last_tap);
    return;*/
  }

  last_tap = time;
  //last_tap = 0;
  //first_tap = 0;
  
  switch (axis) {    
  case ACCEL_AXIS_X:
    if (direction > 0) {            
      APP_LOG(APP_LOG_LEVEL_INFO, "X axis positive.");
    } else {
      APP_LOG(APP_LOG_LEVEL_INFO, "X axis negative.");
    }
    break;
  case ACCEL_AXIS_Y:
    if (direction > 0) {
      APP_LOG(APP_LOG_LEVEL_INFO, "Y axis positive.");
    } else {
      APP_LOG(APP_LOG_LEVEL_INFO, "Y axis negative.");
    }
    break;
  case ACCEL_AXIS_Z:
    if (direction > 0) {
      APP_LOG(APP_LOG_LEVEL_INFO, "Z axis positive.");
    } else {
      APP_LOG(APP_LOG_LEVEL_INFO, "Z axis negative.");
    }
    break;
  }
  
  graph = !graph;
  if (graph) {
    animate_graph();    
  } else {
    hide_graph();
  }
}

static void init() {
  // Create main Window element and assign to poiner
  s_main_window = window_create();
  
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  // Show the window on the watch, with animated = true;
  window_stack_push(s_main_window, true);
  
  APP_LOG(APP_LOG_LEVEL_INFO, "Show main window!");

  //Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Make sure the time is displayed from the start
  update_time();
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  // Subscribe to taps
  accel_tap_service_subscribe(tap_handler);
  
  APP_LOG(APP_LOG_LEVEL_INFO, "Init!");
}

static void deinit() {
  // Stop any animation in progress
  animation_unschedule_all();

  tick_timer_service_unsubscribe();
  accel_tap_service_unsubscribe();
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}