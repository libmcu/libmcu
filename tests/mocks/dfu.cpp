#include "CppUTestExt/MockSupport.h"
#include "libmcu/dfu.h"

struct dfu *dfu_new(size_t data_block_size) {
	return (struct dfu *)mock().actualCall(__func__)
		.withParameter("data_block_size", data_block_size)
		.returnPointerValue();
}

void dfu_delete(struct dfu *dfu) {
	mock().actualCall(__func__);
}

dfu_error_t dfu_prepare(struct dfu *dfu, const struct dfu_image_header *header) {
	return (dfu_error_t)mock().actualCall(__func__)
		.withParameter("header", header)
		.returnIntValue();
}

dfu_error_t dfu_write(struct dfu *dfu, uint32_t offset,
		const void *data, size_t datasize) {
	return (dfu_error_t)mock().actualCall(__func__)
		.withParameter("offset", offset)
		.withParameter("data", data)
		.withParameter("datasize", datasize)
		.returnIntValue();
}

dfu_error_t dfu_finish(struct dfu *dfu) {
	return (dfu_error_t)mock().actualCall(__func__)
		.returnIntValue();
}

dfu_error_t dfu_abort(struct dfu *dfu) {
	return (dfu_error_t)mock().actualCall(__func__)
		.returnIntValue();
}

dfu_error_t dfu_commit(struct dfu *dfu) {
	return (dfu_error_t)mock().actualCall(__func__)
		.returnIntValue();
}

bool dfu_is_valid_header(const struct dfu_image_header *header) {
	return mock().actualCall(__func__)
		.withParameter("header", header)
		.returnBoolValue();
}

dfu_error_t dfu_invalidate(dfu_slot_t slot) {
	return (dfu_error_t)mock().actualCall(__func__)
		.withParameter("slot", slot)
		.returnIntValue();
}
