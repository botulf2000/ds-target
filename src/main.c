#include <pebble.h>
#include <math.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
//static TextLayer *s_minute_layer;
static BitmapLayer *background_layer;
static GBitmap *bg_image;


static GFont s_time_font;
//static GFont s_date_font;

static Layer *s_canvas_layer;
static int minutes, hours;
static struct tm ts;
static int steps;
static int target_steps;

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  ts=*localtime(&temp);
 
  minutes=ts.tm_min;
  layer_mark_dirty(s_canvas_layer);

  hours=ts.tm_hour;  
  // Write the current hours and minutes into a buffer
  static char s_buffer[6];

  if(minutes<10)
  {
    snprintf(s_buffer,6, "%d:0%d", hours,minutes);
  }
  else
  {
    snprintf(s_buffer,6, "%d:%d", hours,minutes);
  }
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
  
  #if defined(PBL_HEALTH)
  // Use the step count metric
  HealthMetric metric = HealthMetricStepCount;

  // Create timestamps for midnight (the start time) and now (the end time)
  time_t start = time_start_of_today();
  time_t end = time(NULL);

  // Check step data is available
  HealthServiceAccessibilityMask mask = health_service_metric_accessible(metric, 
                                                                    start, end);
  bool any_data_available = mask & HealthServiceAccessibilityMaskAvailable;
  //steps= (int)health_service_sum_today(metric);
  steps+=600;
  printf("%d",steps);
  #else
  
  // Health data is not available here
  bool any_data_available = false;
  
  
  #endif
  
  /*
  minutes=(minutes+13)%60;
  layer_mark_dirty(s_canvas_layer);
  static char s_buffer[3];
    snprintf(s_buffer,3, "%d", minutes);
    // Display this time on the TextLayer
    text_layer_set_text(s_time_layer, s_buffer);
  */
   
  }

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}


static void canvas_update_proc(Layer *layer, GContext *ctx) {
  // Custom drawing happens here!
  
  Layer *window_layer = window_get_root_layer(s_main_window);
  GRect bounds = layer_get_bounds(window_layer);
  
  int x,y;
  int left=bounds.size.w/10+2;
  int right=bounds.size.w-left;
  int adjusted_steps=steps/(target_steps/5);
  uint16_t radius = bounds.size.w/8;
  
  if(adjusted_steps>4) adjusted_steps=4;
  
  printf("adj steps %d", adjusted_steps);
  
  y=bounds.size.h/2;
  x=left;
  GPoint center = GPoint(x, y);
  
  graphics_context_set_stroke_width(ctx, 2);
  graphics_context_set_stroke_color(ctx, GColorInchworm);  
  graphics_draw_circle(ctx, center, radius);
  // Draw the outline of a circle
  for(int i=0;i<adjusted_steps;i++)
  {
    x=x+((right-left)/6);
    center = GPoint(x, y);
    graphics_draw_circle(ctx, center, radius);
  }
  
  // Fill a circle
  GPoint p1 = GPoint(center.x-radius, center.y-radius+4);  
  GPoint p2 = GPoint(center.x+radius, center.y-radius+4);  
  graphics_context_set_fill_color(ctx, GColorInchworm);
  graphics_fill_circle(ctx, center, radius);
  
  graphics_context_set_stroke_color(ctx, GColorBlack);  
  graphics_context_set_stroke_width(ctx, 1);
  graphics_context_set_fill_color(ctx, GColorBlack);
  
  for(int i=0;i<2*radius;i+=4)
  {
    p1.y=center.y-radius+i;
    p2.y=center.y-radius+i;
    graphics_draw_line(ctx, p1, p2);
  }
  radius=radius/4;
  
  graphics_fill_circle(ctx, center, radius);

  
}


static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  
  bg_image=gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BG);
  background_layer=bitmap_layer_create(bounds);
  
  bitmap_layer_set_bitmap(background_layer, bg_image);
  layer_add_child(window_layer, bitmap_layer_get_layer(background_layer));  
  

  // Create canvas layer
  s_canvas_layer = layer_create(bounds);
  
  
  // Assign the custom drawing procedure
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);

  // Add to Window
  layer_add_child(window_get_root_layer(window), s_canvas_layer);

  // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(
      GRect(bounds.size.w/2, 2*bounds.size.h/3, bounds.size.w/2, bounds.size.h/2));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text(s_time_layer, "0");
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Create GFont
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_AW_18));
  
  // Create the TextLayer with specific bounds
//  s_minute_layer = text_layer_create(
//      GRect(0, 2*bounds.size.h/10, 3*bounds.size.w/4, bounds.size.h/3));

  // Improve the layout to be more like a watchface
 // text_layer_set_background_color(s_minute_layer, GColorClear);
 // text_layer_set_text_color(s_minute_layer, GColorWhite);
 // text_layer_set_text(s_minute_layer, "0");
  
  // Apply to TextLayer
  text_layer_set_font(s_time_layer, s_time_font);
  //text_layer_set_font(s_minute_layer, s_time_font);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
  // Create the TextLayer with specific bounds
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);

  // Unload GFont
  fonts_unload_custom_font(s_time_font);
  layer_destroy(s_canvas_layer);
  //window_destroy(s_main_window);
  
  // Destroy BitmapLayer
//  bitmap_layer_destroy(s_background_layer);
  gbitmap_destroy(bg_image);
  bitmap_layer_destroy(background_layer);
}


static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set the background color
  window_set_background_color(s_main_window, GColorOxfordBlue);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Make sure the time is displayed from the start
  steps=0;
  target_steps=10000;
  update_time();

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
}

static void deinit() {
  // Destroy Window
  
    window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}