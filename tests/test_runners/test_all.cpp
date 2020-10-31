#include "CppUTest/CommandLineTestRunner.h"

int main(int argc, char **argv)
{
	return RUN_ALL_TESTS(argc, argv);
	//return CommandLineTestRunner::RunAllTests(argc, argv);
}
