/* How a guest frame lands on a screen: the drawable is the screen's own pixels, the frame is
   aspect-fit inside it, and the filter is nearest only when every screen pixel is a whole
   number of frame pixels (1:1 or integer), linear otherwise. Expected values are worked by hand. */
#include <stdio.h>

#include "pvg_present.h"

static int checks, failures;

static void check(const char *what, long got, long want) {
    checks++;
    if (got != want) {
        printf("  FAIL %s: %ld, expected %ld\n", what, got, want);
        failures++;
    }
}

static void expect(const char *what, HvPresentPlan p, uint32_t dw, uint32_t dh, uint32_t x, uint32_t y, uint32_t w, uint32_t h, int nearest) {
    char buf[96];
    snprintf(buf, sizeof buf, "%s drawable", what); check(buf, (long)p.drawable_w << 16 | p.drawable_h, (long)dw << 16 | dh);
    snprintf(buf, sizeof buf, "%s viewport origin", what); check(buf, (long)p.x << 16 | p.y, (long)x << 16 | y);
    snprintf(buf, sizeof buf, "%s viewport size", what); check(buf, (long)p.w << 16 | p.h, (long)w << 16 | h);
    snprintf(buf, sizeof buf, "%s nearest", what); check(buf, p.nearest, nearest);
}

int main(void) {
    /* A 1080p frame on a MacBook's 3390 x 2160 AirPlay screen: the width limits, 3390 / 1920 = 1.7656,
       so 1080 * 1.7656 = 1906.9 -> 1907 tall, (2160 - 1907) / 2 = 126 down. Not a whole number: linear. */
    expect("1080p on 3390x2160", hv_present_plan(1920, 1080, 3390, 2160), 3390, 2160, 0, 126, 3390, 1907, 0);
    /* The frame is the screen's own size: every pixel lands on its own, nearest. */
    expect("3390x2160 on 3390x2160", hv_present_plan(3390, 2160, 3390, 2160), 3390, 2160, 0, 0, 3390, 2160, 1);
    /* Half a 4K screen (the budget's choice for 5K, or a 2x mode): each frame pixel is exactly 2 x 2 screen pixels, nearest. */
    expect("1920x1080 on 3840x2160", hv_present_plan(1920, 1080, 3840, 2160), 3840, 2160, 0, 0, 3840, 2160, 1);
    /* The phone itself (iPhone 17 Pro on its side, 2622 x 1206): the height limits, 1206 / 1080 = 1.11667,
       so 1920 * 1.11667 = 2144 wide, (2622 - 2144) / 2 = 239 in. Linear. */
    expect("1080p on 2622x1206", hv_present_plan(1920, 1080, 2622, 1206), 2622, 1206, 239, 0, 2144, 1206, 0);
    /* No screen size known (a layer before layout, the colour probe): the frame's own pixels, as before. */
    expect("1080p on an unknown screen", hv_present_plan(1920, 1080, 0, 0), 1920, 1080, 0, 0, 1920, 1080, 1);
    printf("%s: %d checks, %d failures\n", failures ? "Present plan: FAILED" : "Present plan: all checks passed", checks, failures);
    return failures != 0;
}
