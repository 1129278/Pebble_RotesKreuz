#include <pebble.h>

#define REPEAT_INTERVAL_MS 50
#define REPEAT_INTERVAL_MS_LONG 700

// This is a custom defined key for saving our count field
#define NUM_ADRENALIN_PKEY 1
#define SECONDS_ADRENALIN_PKEY 1

// You can define defaults for values in persistent storage
#define NUM_ADRENALIN_DEFAULT 0
#define NUM_SCHOCK_DEFAULT 0
#define NUM_AMIODARON_DEFAULT 0
#define SECONDS_ADRENALIN_DEFAULT 0

static Window *s_main_window;
static ActionBarLayer *s_action_bar;
static TextLayer *s_time_layer, *s_adrenalin_layer, *s_schock_layer, *s_amiodaron_layer;
static AppTimer *timer;
static const uint16_t timer_interval_ms = 1000;
static int seconds_adrenalin = SECONDS_ADRENALIN_DEFAULT;

static int s_num_adrenalin = NUM_ADRENALIN_DEFAULT;
static int s_num_schock = NUM_SCHOCK_DEFAULT;
static int s_num_amiodaron = NUM_AMIODARON_DEFAULT;
static char s_time_adrenalin[6]; //Global is schirch, aber funktioniert

static void update_text_adrenalin() {
  
  static char s_body_text[43]; // Max. 2 Digits für Adrenalin-Anzahl
  
  int min = seconds_adrenalin/60;
  int sec = seconds_adrenalin%60;
  
  snprintf(s_body_text, sizeof(s_body_text), "Adrenalin: %u (Last: %s)\nNext in  %02d:%02d", s_num_adrenalin, s_time_adrenalin, min, sec);
  text_layer_set_text(s_adrenalin_layer, s_body_text);
  
    if (seconds_adrenalin == 0) {
      vibes_long_pulse();
    }
}

static void timer_callback(void *data) {
  seconds_adrenalin--;
  timer = app_timer_register(timer_interval_ms, timer_callback, NULL);
  update_text_adrenalin();
}



static void handle_second_tick(struct tm* tick_time, TimeUnits units_changed) {
  // Needs to be static because it's used by the system later.
  static char s_time_text[] = "00:00:00";
  
  strftime(s_time_text, sizeof(s_time_text), "%T", tick_time);
  text_layer_set_text(s_time_layer, s_time_text);
}



static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  
  vibes_short_pulse();
  s_num_adrenalin++;    
  seconds_adrenalin = 240; // Adrenalin alle 3-5 Minuten geben
  
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  strftime(s_time_adrenalin, sizeof(s_time_adrenalin), "%T", current_time);
  
  if (timer) {
    app_timer_cancel(timer);
    timer = NULL;
  }
  timer = app_timer_register(timer_interval_ms, timer_callback, NULL);
  
}

static void update_text_schock() {
  
  static char s_body_text[12]; // Max. 2 Digits für Schock-Anzahl
  snprintf(s_body_text, sizeof(s_body_text), "Schocks: %u", s_num_schock);
  text_layer_set_text(s_schock_layer, s_body_text);
  
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  
  vibes_short_pulse();
  s_num_schock++;
  update_text_schock();
  
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  
  vibes_short_pulse();
  text_layer_set_text(s_amiodaron_layer, "Amiodaron: 1 (Last: 18:23)\nNext in 5:21");
}

static void back_click_handler(ClickRecognizerRef recognizer, void *context) {
  
  vibes_short_pulse();
  
  if (timer) {
    app_timer_cancel(timer);
    timer = NULL;
  }
  
  s_num_adrenalin = 0;
  s_num_schock = 0;
  s_num_amiodaron = 0;
  
  update_text_adrenalin();
  update_text_schock();
    
}

static void click_config_provider(void *context) {
  window_single_repeating_click_subscribe(BUTTON_ID_UP, REPEAT_INTERVAL_MS, up_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_SELECT, REPEAT_INTERVAL_MS, select_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, REPEAT_INTERVAL_MS, down_click_handler);
  
  window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler); // Overrides default (long press to quit app)
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  s_action_bar = action_bar_layer_create();
  action_bar_layer_add_to_window(s_action_bar, window);
  action_bar_layer_set_click_config_provider(s_action_bar, click_config_provider);
  
  // Main time
  s_time_layer = text_layer_create(GRect(0, 0, bounds.size.w, 34));
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  // Adrenalin
  s_adrenalin_layer = text_layer_create(GRect(0, 35, bounds.size.w, 34));
  text_layer_set_text_color(s_adrenalin_layer, GColorWhite);
  text_layer_set_background_color(s_adrenalin_layer, GColorClear);
  text_layer_set_font(s_adrenalin_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_adrenalin_layer, GTextAlignmentRight);
  text_layer_set_text(s_adrenalin_layer, "Adrenalin: 0");
  
  // Schocks
  s_schock_layer = text_layer_create(GRect(0, bounds.size.w/2, bounds.size.w, 34));
  text_layer_set_text_color(s_schock_layer, GColorWhite);
  text_layer_set_background_color(s_schock_layer, GColorClear);
  text_layer_set_font(s_schock_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_schock_layer, GTextAlignmentRight);
  text_layer_set_text(s_schock_layer, "Schocks: 0");
  
  // Amiodaron
  s_amiodaron_layer = text_layer_create(GRect(0, (bounds.size.h/3.0)*2.0, bounds.size.w, 34));
  text_layer_set_text_color(s_amiodaron_layer, GColorWhite);
  text_layer_set_background_color(s_amiodaron_layer, GColorClear);
  text_layer_set_font(s_amiodaron_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_amiodaron_layer, GTextAlignmentRight);
  text_layer_set_text(s_amiodaron_layer, "Amiodaron: 0");
  
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_second_tick(current_time, SECOND_UNIT);

  tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
  
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_adrenalin_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_schock_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_amiodaron_layer));
}

static void main_window_unload(Window *window) {
  tick_timer_service_unsubscribe();
  
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_adrenalin_layer);
  text_layer_destroy(s_schock_layer);
  text_layer_destroy(s_amiodaron_layer);
}

static void init() {
  
  s_num_adrenalin = persist_exists(NUM_ADRENALIN_PKEY) ? persist_read_int(NUM_ADRENALIN_PKEY) : NUM_ADRENALIN_DEFAULT;
  seconds_adrenalin = persist_exists(SECONDS_ADRENALIN_PKEY) ? persist_read_int(SECONDS_ADRENALIN_PKEY) : SECONDS_ADRENALIN_DEFAULT;
  
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
}

static void deinit() {
  // Save into persistent storage on app exit
  persist_write_int(NUM_ADRENALIN_PKEY, s_num_adrenalin);
  
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
