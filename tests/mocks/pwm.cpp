#include "CppUTestExt/MockSupport.h"
#include "libmcu/pwm.h"

struct lm_pwm_channel {
	int dummy;
};

struct lm_pwm {
	struct lm_pwm_channel dummy;
};

struct lm_pwm *lm_pwm_create(uint8_t timer) {
	return (struct lm_pwm *)mock().actualCall("lm_pwm_create")
		.withParameter("timer", timer)
		.returnPointerValue();
}

int lm_pwm_delete(struct lm_pwm *self) {
	return mock().actualCall("lm_pwm_delete")
		.withParameter("self", self)
		.returnIntValue();
}

struct lm_pwm_channel *lm_pwm_create_channel(struct lm_pwm *self, int ch, int pin) {
	return (struct lm_pwm_channel *)mock().actualCall("lm_pwm_create_channel")
		.withParameter("self", self)
		.withParameter("ch", ch)
		.withParameter("pin", pin)
		.returnPointerValue();
}

int lm_pwm_delete_channel(struct lm_pwm_channel *ch) {
	return mock().actualCall("lm_pwm_delete_channel")
		.withParameter("ch", ch)
		.returnIntValue();
}

int lm_pwm_enable(struct lm_pwm_channel *ch) {
	return mock().actualCall("lm_pwm_enable")
		.withParameter("ch", ch)
		.returnIntValue();
}

int lm_pwm_disable(struct lm_pwm_channel *ch) {
	return mock().actualCall("lm_pwm_disable")
		.withParameter("ch", ch)
		.returnIntValue();
}

int lm_pwm_start(struct lm_pwm_channel *ch, uint32_t freq_hz, uint32_t duty_millipercent) {
	return mock().actualCall("lm_pwm_start")
		.withParameter("ch", ch)
		.withParameter("freq_hz", freq_hz)
		.withParameter("duty_millipercent", duty_millipercent)
		.returnIntValue();
}

int lm_pwm_stop(struct lm_pwm_channel *ch) {
	return mock().actualCall("lm_pwm_stop")
		.withParameter("ch", ch)
		.returnIntValue();
}

int lm_pwm_update_frequency(struct lm_pwm_channel *ch, uint32_t hz) {
	return mock().actualCall("lm_pwm_update_frequency")
		.withParameter("ch", ch)
		.withParameter("hz", hz)
		.returnIntValue();
}

int lm_pwm_update_duty(struct lm_pwm_channel *ch, uint32_t millipercent) {
	return mock().actualCall("lm_pwm_update_duty")
		.withParameter("ch", ch)
		.withParameter("millipercent", millipercent)
		.returnIntValue();
}
