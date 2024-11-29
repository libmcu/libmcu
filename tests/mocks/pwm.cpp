#include "CppUTestExt/MockSupport.h"
#include "libmcu/pwm.h"

struct pwm_channel {
	int dummy;
};

struct pwm {
	struct pwm_channel dummy;
};

struct pwm *pwm_create(uint8_t timer) {
	return (struct pwm *)mock().actualCall("pwm_create")
		.withParameter("timer", timer)
		.returnPointerValue();
}

int pwm_delete(struct pwm *self) {
	return mock().actualCall("pwm_delete")
		.withParameter("self", self)
		.returnIntValue();
}

struct pwm_channel *pwm_create_channel(struct pwm *self, int ch, int pin) {
	return (struct pwm_channel *)mock().actualCall("pwm_create_channel")
		.withParameter("self", self)
		.withParameter("ch", ch)
		.withParameter("pin", pin)
		.returnPointerValue();
}

int pwm_delete_channel(struct pwm_channel *ch) {
	return mock().actualCall("pwm_delete_channel")
		.withParameter("ch", ch)
		.returnIntValue();
}

int pwm_enable(struct pwm_channel *ch) {
	return mock().actualCall("pwm_enable")
		.withParameter("ch", ch)
		.returnIntValue();
}

int pwm_disable(struct pwm_channel *ch) {
	return mock().actualCall("pwm_disable")
		.withParameter("ch", ch)
		.returnIntValue();
}

int pwm_start(struct pwm_channel *ch, uint32_t freq_hz, uint32_t duty_millipercent) {
	return mock().actualCall("pwm_start")
		.withParameter("ch", ch)
		.withParameter("freq_hz", freq_hz)
		.withParameter("duty_millipercent", duty_millipercent)
		.returnIntValue();
}

int pwm_stop(struct pwm_channel *ch) {
	return mock().actualCall("pwm_stop")
		.withParameter("ch", ch)
		.returnIntValue();
}

int pwm_update_frequency(struct pwm_channel *ch, uint32_t hz) {
	return mock().actualCall("pwm_update_frequency")
		.withParameter("ch", ch)
		.withParameter("hz", hz)
		.returnIntValue();
}

int pwm_update_duty(struct pwm_channel *ch, uint32_t millipercent) {
	return mock().actualCall("pwm_update_duty")
		.withParameter("ch", ch)
		.withParameter("millipercent", millipercent)
		.returnIntValue();
}
