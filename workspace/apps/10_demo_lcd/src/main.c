#include <zephyr/kernel.h>
#include <zephyr/drivers/display.h>
#include <lvgl.h>
#include <string.h>

// Settings
static const int32_t sleep_time_ms = 50;	// Target 20 FPS

int main(void)
{
	uint32_t count = 0;
	char buf[11] = {0};
	const struct device *display;
	lv_obj_t *label_hello;
	lv_obj_t *label_counter;
	lv_obj_t *rect;
	lv_obj_t *circle;

	// Initialize the display
	display = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	if (!device_is_ready(display)) {
		printk("Error: display not ready\r\n");
		return 0;
	}

	// Create static label widget
	label_hello = lv_label_create(lv_scr_act());
	lv_label_set_text(label_hello, "Hello, World!");
	lv_obj_align(label_hello, LV_ALIGN_TOP_MID, 0, 0);

	// Create dynamic label widget
	label_counter = lv_label_create(lv_scr_act());
	lv_obj_align(label_counter, LV_ALIGN_BOTTOM_MID, 0, 0);

	// Create a rectangle
	rect = lv_obj_create(lv_scr_act());
	lv_obj_set_size(rect, 50, 50);
	lv_obj_align(rect, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_bg_color(rect, lv_color_hex(0x00FF00), 0);  // Set background color to green

	// SRH: TODO - Draw a line

	// Create a line [NOT WORKING]
	lv_obj_t *line1 = lv_line_create(lv_scr_act());
	static lv_point_t line_points[] = { {0, 0}, {100, 0}, {100, 100}, {0, 100}, {0, 0} };
	lv_line_set_points(line1, line_points, 5);
	lv_obj_align(line1, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_bg_color(line1, lv_color_hex(0x0000FF), 0);  // Set background color to blue


	// SRH: TODO - Figure out how this works and make a circle

	// // Create style for the rectangle
	// lv_style_t style;
	// lv_style_init(&style);

	//  /*Set a background color and a radius*/
    // lv_style_set_radius(&style, 5);
    // lv_style_set_bg_opa(&style, LV_OPA_COVER);
    // lv_style_set_bg_color(&style, lv_palette_lighten(LV_PALETTE_GREY, 1));

    // /*Add outline*/
    // lv_style_set_outline_width(&style, 2);
    // lv_style_set_outline_color(&style, lv_palette_main(LV_PALETTE_BLUE));
    // lv_style_set_outline_pad(&style, 8);

    // /*Create an object with the new style*/
    // lv_obj_add_style(rect, &style, 0);

	// ---

	// // Create a circle
	// circle = lv_obj_create(lv_scr_act());
	// lv_obj_set_size(circle, 50, 50);
	
	
	// // TEST: create a style and apply it to the object to create a circle
	// lv_style_t style;
	// lv_style_init(&style);
	// lv_style_set_radius(&style, LV_RADIUS_CIRCLE);  // Set the radius to full circle
	// lv_style_set_bg_color(&style, lv_color_hex(0xFF0000));  // Set background color to red
	// lv_style_set_border_width(&style, 0);  // No border
	// lv_obj_add_style(circle, &style, 0);  // Apply the style to the circle object
	
	// lv_obj_align(circle, LV_ALIGN_CENTER, 0, 0);

	// lv_task_handler();
	display_blanking_off(display);

	// Do forever
	while (1) {

		// Update counter label every second
		if ((count % (1000 / sleep_time_ms)) == 0U) {
			sprintf(buf, "%d", count / (1000 / sleep_time_ms));
			lv_label_set_text(label_counter, buf);
		}
		
		++count;

		// Draw circle


		// Must be called periodically
		lv_task_handler();

		// Sleep
		k_msleep(sleep_time_ms);
	}
}
