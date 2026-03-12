/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define PULSE_TIME_MS 150

static const struct gpio_dt_spec leds[] = {
	GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(led3), gpios),
};

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_NODELABEL(button0), gpios);

K_SEM_DEFINE(button_sem, 0, 1);

void pin_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	k_sem_give(&button_sem);
}

int main(void)
{
	int ret;
	static struct gpio_callback pin_cb_data;

	for (int i = 0; i < ARRAY_SIZE(leds); i++) {
		if (!gpio_is_ready_dt(&leds[i]))
			return 0;
		ret = gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_INACTIVE);
		if (ret < 0)
			return 0;
	}

	if (!gpio_is_ready_dt(&button))
		return 0;

	ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret < 0)
		return 0;

	ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret < 0)
		return 0;

	gpio_init_callback(&pin_cb_data, pin_isr, BIT(button.pin));
	gpio_add_callback(button.port, &pin_cb_data);

	while (1) {
		k_sem_take(&button_sem, K_FOREVER);

		for (int i = 0; i < ARRAY_SIZE(leds); i++) {
			gpio_pin_set_dt(&leds[i], 1);
			k_msleep(PULSE_TIME_MS);
			gpio_pin_set_dt(&leds[i], 0);
		}
	}

	return 0;
}
