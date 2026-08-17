/*
 * Drop trackpoint/pointer relative events while Left Ctrl is applied.
 * Covers a held LCTRL key and implicit mods from LC(...) keycodes.
 */

#define DT_DRV_COMPAT zmk_input_processor_mute_if_lctrl

#include <zephyr/device.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <drivers/input_processor.h>
#include <dt-bindings/zmk/modifiers.h>

#if !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
#include <zmk/hid.h>
#endif

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

static bool is_rel_motion_or_scroll(const struct input_event *event) {
    if (event->type != INPUT_EV_REL) {
        return false;
    }

    switch (event->code) {
    case INPUT_REL_X:
    case INPUT_REL_Y:
    case INPUT_REL_WHEEL:
    case INPUT_REL_HWHEEL:
        return true;
    default:
        return false;
    }
}

static int mute_if_lctrl_handle_event(const struct device *dev, struct input_event *event,
                                      uint32_t param1, uint32_t param2,
                                      struct zmk_input_processor_state *state) {
    ARG_UNUSED(dev);
    ARG_UNUSED(param1);
    ARG_UNUSED(param2);
    ARG_UNUSED(state);

    if (!is_rel_motion_or_scroll(event)) {
        return ZMK_INPUT_PROC_CONTINUE;
    }

#if !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    const struct zmk_hid_keyboard_report *report = zmk_hid_get_keyboard_report();
    if (report == NULL) {
        return ZMK_INPUT_PROC_CONTINUE;
    }

    /* HID modifier byte includes explicit holds and LC() implicit mods. */
    if ((report->body.modifiers & MOD_LCTL) != 0) {
        return ZMK_INPUT_PROC_STOP;
    }
#endif

    return ZMK_INPUT_PROC_CONTINUE;
}

static struct zmk_input_processor_driver_api mute_if_lctrl_driver_api = {
    .handle_event = mute_if_lctrl_handle_event,
};

#define MUTE_IF_LCTRL_INST(n)                                                                      \
    DEVICE_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                                  \
                          CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &mute_if_lctrl_driver_api);

DT_INST_FOREACH_STATUS_OKAY(MUTE_IF_LCTRL_INST)
