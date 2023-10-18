#include "CppUTestExt/MockSupport.h"
#include "bytearray.h"

class ByteArrayComparator : public MockNamedValueComparator {
	public:
		virtual bool isEqual(const void *object1, const void *object2) {
			return true;
		}
		virtual SimpleString valueToString(const void *object) {
			return StringFrom(object);
		}
};
