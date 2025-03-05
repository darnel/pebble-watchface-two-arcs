#include<pebble.h>

static Window *s_window;
static Layer *s_canvas;
static TextLayer *s_time_layer;
static struct tm *s_tick_time;

static int32_t get_angle_for_hour(int hour) {
    // Progress through 12 hours, out of 360 degrees
    return (hour%12 * 360) / 12;
}

static int32_t get_angle_for_minute(int minute) {
    // Progress through 60 minutes, out of 360 degrees
    return (minute * 360) / 60;
}

static void layer_update_proc(Layer *layer, GContext *ctx) {
    GRect bounds = layer_get_bounds(layer);
    int hour_angle = get_angle_for_hour(s_tick_time->tm_hour);
    int minute_angle = get_angle_for_minute(s_tick_time->tm_min);
    
    // minute arc
    GRect frame = grect_inset(bounds, GEdgeInsets(10));
    graphics_context_set_fill_color(ctx, GColorDarkGray);
    graphics_fill_radial(ctx, frame, GOvalScaleModeFitCircle, 15, 0, DEG_TO_TRIGANGLE(minute_angle));
    
    // hour arc
    frame = grect_inset(bounds, GEdgeInsets(25));
    graphics_context_set_fill_color(ctx, GColorLightGray);
    graphics_fill_radial(ctx, frame, GOvalScaleModeFitCircle, 15, 0, DEG_TO_TRIGANGLE(hour_angle));
    
    // Write the current hours and minutes into a buffer
    static char s_buffer[8];
    strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
        "%H:%M" : "%I:%M", s_tick_time);

    // Display this time on the TextLayer
    text_layer_set_text(s_time_layer, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    *s_tick_time = *tick_time;
    layer_mark_dirty(s_canvas);
}

static void window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    s_canvas = layer_create(bounds);
    s_time_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50));
    text_layer_set_background_color(s_time_layer, GColorClear);
    text_layer_set_text_color(s_time_layer, GColorWhite);
    text_layer_set_text(s_time_layer, "00:00");
    text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
    window_set_background_color(window, GColorBlack);
    layer_add_child(window_layer, s_canvas);
    layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
    
    layer_set_update_proc(s_canvas, layer_update_proc);
}

static void window_unload(Window *window) {
    text_layer_destroy(s_time_layer);
}

static void init(void) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
    const bool animated = true;
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    window_stack_push(s_window, animated);
    
    // Get a tm structure
    time_t temp = time(NULL);
    s_tick_time = localtime(&temp);
}

static void deinit(void) {
    window_destroy(s_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
