/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "hal_gpio.h"
#include "hal_peci.h"
#include "power_status.h"
#include "util_sys.h"
#include "plat_gpio.h"
#include "plat_i2c_target.h"
#include "ipmi.h"
#include "pldm.h"
#include "plat_mctp.h"
#include "plat_class.h"
#include "plat_pldm_monitor.h"
#include "plat_led.h"

SCU_CFG scu_cfg[] = {
	//register    value
	{ 0x7e6e2618, 0x00FF0000 },
};

void pal_pre_init()
{
	/* init i2c target */
	for (int index = 0; index < MAX_TARGET_NUM; index++) {
		if (I2C_TARGET_ENABLE_TABLE[index])
			i2c_target_control(
				index, (struct _i2c_target_config *)&I2C_TARGET_CONFIG_TABLE[index],
				1);
	}
	scu_init(scu_cfg, sizeof(scu_cfg) / sizeof(SCU_CFG));

	init_platform_config();
}

void pal_post_init()
{
	plat_mctp_init();
	/* Send device presence log when the BIC is AC on */
	if (is_ac_lost()) {
		ssd_present_check();
		nic_present_check();
	}

	gpio_set(BIC_SYS_READY_N, GPIO_LOW);

	sys_led_init_and_check();
	plat_pldm_assign_gpio_effecter_id();
}

void pal_set_sys_status()
{
	set_DC_status(SYS_PWR_READY_N);
	set_DC_on_delayed_status();
}

#define DEF_PROJ_GPIO_PRIORITY 78

DEVICE_DEFINE(PRE_DEF_PROJ_GPIO, "PRE_DEF_PROJ_GPIO_NAME", &gpio_init, NULL, NULL, NULL,
	      POST_KERNEL, DEF_PROJ_GPIO_PRIORITY, NULL);
