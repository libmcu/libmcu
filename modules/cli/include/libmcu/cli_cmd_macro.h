/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_CLI_CMD_MACRO_H
#define LIBMCU_CLI_CMD_MACRO_H

#if defined(__cplusplus)
extern "C" {
#endif

#define CLI_CPP_MAX_CMD		32 /*64 - 1*/

#define CLI_CPP_RSEQ_N() 62,61,60,59,58,57,56,55,54,53,52,51,50, \
	49,48,47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,31,30, \
	29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10, \
	9,8,7,6,5,4,3,2,1,0
#define CLI_CPP_ARG_N(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10, \
	_11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
	_21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
	_31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
	_41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
	_51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
	_61,_62,_63, N, ...) N

#define CLI_CPP_APPLY_0(macroname, a1)
#define CLI_CPP_APPLY_1(macroname, a1)	\
	CLI_CPP_EXPAND_NESTED(macroname, (a1))
#define CLI_CPP_APPLY_2(macroname, a1,a2) \
	CLI_CPP_EXPAND_NESTED(macroname, (a1)) CLI_CPP_COMMA \
	CLI_CPP_APPLY_1(macroname, a2)
#define CLI_CPP_APPLY_3(macroname, a1,a2,a3) \
	CLI_CPP_EXPAND_NESTED(macroname, (a1)) CLI_CPP_COMMA \
	CLI_CPP_APPLY_2(macroname, a2,a3)
#define CLI_CPP_APPLY_4(macroname, a1,a2,a3,a4) \
	CLI_CPP_EXPAND_NESTED(macroname, (a1)) CLI_CPP_COMMA \
	CLI_CPP_APPLY_3(macroname, a2,a3,a4)
#define CLI_CPP_APPLY_5(macroname, a1,a2,a3,a4,a5) \
	CLI_CPP_EXPAND_NESTED(macroname, (a1)) CLI_CPP_COMMA \
	CLI_CPP_APPLY_4(macroname, a2,a3,a4,a5)
#define CLI_CPP_APPLY_6(macroname, a1,a2,a3,a4,a5,a6) \
	CLI_CPP_EXPAND_NESTED(macroname, (a1)) CLI_CPP_COMMA \
	CLI_CPP_APPLY_5(macroname, a2,a3,a4,a5,a6)
#define CLI_CPP_APPLY_7(macroname, a1,a2,a3,a4,a5,a6,a7) \
	CLI_CPP_EXPAND_NESTED(macroname, (a1)) CLI_CPP_COMMA \
	CLI_CPP_APPLY_6(macroname, a2,a3,a4,a5,a6,a7)
#define CLI_CPP_APPLY_8(macroname, a1,a2,a3,a4,a5,a6,a7,a8) \
	CLI_CPP_EXPAND_NESTED(macroname, (a1)) CLI_CPP_COMMA \
	CLI_CPP_APPLY_7(macroname, a2,a3,a4,a5,a6,a7,a8)
#define CLI_CPP_APPLY_9(macroname, a1,a2,a3,a4,a5,a6,a7,a8,a9) \
	CLI_CPP_EXPAND_NESTED(macroname, (a1)) CLI_CPP_COMMA \
	CLI_CPP_APPLY_8(macroname, a2,a3,a4,a5,a6,a7,a8,a9)
#define CLI_CPP_APPLY_10(macroname, a1,a2,a3,a4,a5,a6,a7,a8,a9,a10) \
	CLI_CPP_EXPAND_NESTED(macroname, (a1)) CLI_CPP_COMMA \
	CLI_CPP_APPLY_9(macroname, a2,a3,a4,a5,a6,a7,a8,a9,a10)
#define CLI_CPP_APPLY_11(macroname, a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11) \
	CLI_CPP_EXPAND_NESTED(macroname, (a1)) CLI_CPP_COMMA \
	CLI_CPP_APPLY_10(macroname, a2,a3,a4,a5,a6,a7,a8,a9,a10,a11)
#define CLI_CPP_APPLY_12(macroname, a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12) \
	CLI_CPP_EXPAND_NESTED(macroname, (a1)) CLI_CPP_COMMA \
	CLI_CPP_APPLY_11(macroname, a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12)
#define CLI_CPP_APPLY_13(macroname, a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13) \
	CLI_CPP_EXPAND_NESTED(macroname, (a1)) CLI_CPP_COMMA \
	CLI_CPP_APPLY_12(macroname, a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13)
#define CLI_CPP_APPLY_14(macroname, a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14) \
	CLI_CPP_EXPAND_NESTED(macroname, (a1)) CLI_CPP_COMMA \
	CLI_CPP_APPLY_13(macroname, a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14)
#define CLI_CPP_APPLY_15(macroname, a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15) \
	CLI_CPP_EXPAND_NESTED(macroname, (a1)) CLI_CPP_COMMA \
	CLI_CPP_APPLY_14(macroname, a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15)
#define CLI_CPP_APPLY_16(macroname, a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16) \
	CLI_CPP_EXPAND_NESTED(macroname, (a1)) CLI_CPP_COMMA \
	CLI_CPP_APPLY_15(macroname, a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16)
#define CLI_CPP_APPLY_17(macroname, a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17) \
	CLI_CPP_EXPAND_NESTED(macroname, (a1)) CLI_CPP_COMMA \
	CLI_CPP_APPLY_16(macroname, a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17)
#define CLI_CPP_APPLY_18(macroname, a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18) \
	CLI_CPP_EXPAND_NESTED(macroname, (a1)) CLI_CPP_COMMA \
	CLI_CPP_APPLY_17(macroname, a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18)
#define CLI_CPP_APPLY_19(macroname, a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19) \
	CLI_CPP_EXPAND_NESTED(macroname, (a1)) CLI_CPP_COMMA \
	CLI_CPP_APPLY_18(macroname, a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19)
#define CLI_CPP_APPLY_20(macroname, a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20) \
	CLI_CPP_EXPAND_NESTED(macroname, (a1)) CLI_CPP_COMMA \
	CLI_CPP_APPLY_19(macroname, a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20)
#define CLI_CPP_APPLY_21(macroname, a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,a21) \
	CLI_CPP_EXPAND_NESTED(macroname, (a1)) CLI_CPP_COMMA \
	CLI_CPP_APPLY_20(macroname, a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,a21)
#define CLI_CPP_APPLY_22(macroname, a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,a21,a22) \
	CLI_CPP_EXPAND_NESTED(macroname, (a1)) CLI_CPP_COMMA \
	CLI_CPP_APPLY_21(macroname, a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,a21,a22)
#define CLI_CPP_APPLY_23(macroname, a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,a21,a22,a23) \
	CLI_CPP_EXPAND_NESTED(macroname, (a1)) CLI_CPP_COMMA \
	CLI_CPP_APPLY_22(macroname, a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,a21,a22,a23)
#define CLI_CPP_APPLY_24(macroname, a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,a21,a22,a23,a24) \
	CLI_CPP_EXPAND_NESTED(macroname, (a1)) CLI_CPP_COMMA \
	CLI_CPP_APPLY_23(macroname, a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,a21,a22,a23,a24)
#define CLI_CPP_APPLY_25(macroname, a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,a21,a22,a23,a24,a25) \
	CLI_CPP_EXPAND_NESTED(macroname, (a1)) CLI_CPP_COMMA \
	CLI_CPP_APPLY_24(macroname, a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,a21,a22,a23,a24,a25)
#define CLI_CPP_APPLY_26(macroname, a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,a21,a22,a23,a24,a25,a26) \
	CLI_CPP_EXPAND_NESTED(macroname, (a1)) CLI_CPP_COMMA \
	CLI_CPP_APPLY_25(macroname, a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,a21,a22,a23,a24,a25,a26)
#define CLI_CPP_APPLY_27(macroname, a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,a21,a22,a23,a24,a25,a26,a27) \
	CLI_CPP_EXPAND_NESTED(macroname, (a1)) CLI_CPP_COMMA \
	CLI_CPP_APPLY_26(macroname, a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,a21,a22,a23,a24,a25,a26,a27)
#define CLI_CPP_APPLY_28(macroname, a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,a21,a22,a23,a24,a25,a26,a27,a28) \
	CLI_CPP_EXPAND_NESTED(macroname, (a1)) CLI_CPP_COMMA \
	CLI_CPP_APPLY_27(macroname, a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,a21,a22,a23,a24,a25,a26,a27,a28)
#define CLI_CPP_APPLY_29(macroname, a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,a21,a22,a23,a24,a25,a26,a27,a28,a29) \
	CLI_CPP_EXPAND_NESTED(macroname, (a1)) CLI_CPP_COMMA \
	CLI_CPP_APPLY_28(macroname, a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,a21,a22,a23,a24,a25,a26,a27,a28,a29)
#define CLI_CPP_APPLY_30(macroname, a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,a21,a22,a23,a24,a25,a26,a27,a28,a29,a30) \
	CLI_CPP_EXPAND_NESTED(macroname, (a1)) CLI_CPP_COMMA \
	CLI_CPP_APPLY_29(macroname, a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,a21,a22,a23,a24,a25,a26,a27,a28,a29,a30)
#define CLI_CPP_APPLY_31(macroname, a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,a21,a22,a23,a24,a25,a26,a27,a28,a29,a30,a31) \
	CLI_CPP_EXPAND_NESTED(macroname, (a1)) CLI_CPP_COMMA \
	CLI_CPP_APPLY_30(macroname, a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,a21,a22,a23,a24,a25,a26,a27,a28,a29,a30,a31)
#define CLI_CPP_APPLY_32(macroname, a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,a21,a22,a23,a24,a25,a26,a27,a28,a29,a30,a31,a32) \
	CLI_CPP_EXPAND_NESTED(macroname, (a1)) CLI_CPP_COMMA \
	CLI_CPP_APPLY_31(macroname, a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15,a16,a17,a18,a19,a20,a21,a22,a23,a24,a25,a26,a27,a28,a29,a30,a31,a32)

#define CLI_CPP_NARG(...)		\
	CLI_CPP_NARG_(_0, ## __VA_ARGS__, CLI_CPP_RSEQ_N())
#define CLI_CPP_NARG_(...)		CLI_CPP_ARG_N(__VA_ARGS__)

#define CLI_CPP_REF			&
#define CLI_CPP_COMMA			,
#define CLI_CPP_CMD_PREFIX(name)	cli_cmd_##name
#define CLI_CPP_CMD_PREFIX_REF(name)	CLI_CPP_REF cli_cmd_##name

#define CLI_CPP_CONCAT_(a, b)		a ## b
#define CLI_CPP_CONCAT(a, b)		CLI_CPP_CONCAT_(a, b)

#define CLI_CPP_EXPAND(macro, args)	macro args
#define CLI_CPP_EXPAND_NESTED(macro, args) macro args
#define CLI_CPP_EXPAND_N(macroname, ...) \
	CLI_CPP_EXPAND(CLI_CPP_CONCAT(CLI_CPP_APPLY_, CLI_CPP_NARG(__VA_ARGS__)), \
		(macroname, __VA_ARGS__))

#define CLI_CPP_PUT_PREFIX(prefix, ...)	\
	CLI_CPP_EXPAND_N(prefix, __VA_ARGS__)

#define DEFINE_CLI_CMD(name, desc)	\
	static cli_cmd_error_t cli_cmd_##name##_handler(int argc, \
			const char *argv[], void *env); \
	const struct cli_cmd cli_cmd_##name = { #name, \
			cli_cmd_##name##_handler, desc }; \
	static cli_cmd_error_t cli_cmd_##name##_handler(int argc, \
			const char *argv[], void *env)

#define DEFINE_CLI_CMD_LIST(name, ...)	\
	CLI_ASSERT(CLI_CPP_NARG(__VA_ARGS__) <= CLI_CPP_MAX_CMD); \
	extern const struct cli_cmd CLI_CPP_PUT_PREFIX(CLI_CPP_CMD_PREFIX, \
				__VA_ARGS__); \
	static const struct cli_cmd *name[] = { \
		CLI_CPP_PUT_PREFIX(CLI_CPP_CMD_PREFIX_REF, __VA_ARGS__), \
		0, \
	}

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_CLI_CMD_MACRO_H */
