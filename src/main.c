/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>

LOG_MODULE_REGISTER(blinky, LOG_LEVEL_NONE);

static const struct gpio_dt_spec leds[] = {
	GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(led3), gpios),
};

static const struct gpio_dt_spec buttons[] = {
	GPIO_DT_SPEC_GET(DT_NODELABEL(button0), gpios),
	GPIO_DT_SPEC_GET(DT_NODELABEL(button1), gpios),
	GPIO_DT_SPEC_GET(DT_NODELABEL(button2), gpios),
	GPIO_DT_SPEC_GET(DT_NODELABEL(button3), gpios),
};

void pin_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	for (int i = 0; i < ARRAY_SIZE(buttons); i++)
	{
		if (pins & BIT(buttons[i].pin))
			gpio_pin_set_dt(&leds[i], gpio_pin_get_dt(&buttons[i]));
	}
}

int main(void)
{
	int ret;
	static struct gpio_callback pin_cb_data[ARRAY_SIZE(buttons)];

	for (int i = 0; i < ARRAY_SIZE(leds); i++)
	{
		if (!gpio_is_ready_dt(&leds[i]))
			return 0;
		ret = gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_INACTIVE);
		if (ret < 0)
			return 0;
	}

	for (int i = 0; i < ARRAY_SIZE(buttons); i++)
	{
		if (!gpio_is_ready_dt(&buttons[i]))
			return 0;
		ret = gpio_pin_configure_dt(&buttons[i], GPIO_INPUT);
		if (ret < 0)
			return 0;
		ret = gpio_pin_interrupt_configure_dt(&buttons[i], GPIO_INT_EDGE_BOTH);
		if (ret < 0)
			return 0;
		gpio_init_callback(&pin_cb_data[i], pin_isr, BIT(buttons[i].pin));
		gpio_add_callback(buttons[i].port, &pin_cb_data[i]);
	}

	while (1)
		k_sleep(K_FOREVER);

	return 0;
}
