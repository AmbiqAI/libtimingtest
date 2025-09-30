#ifndef UNITY_H
#define UNITY_H

#ifdef __cplusplus
extern "C" {
#endif

extern volatile int __unity_failures;

/* Optional logging hook:
 *   void __unity_on_fail(const char* macro, const char* msg) {
 *     ns_lp_printf("[UNITY_FAIL] %s: %s\n", macro, msg ? msg : "");
 *   }
 *
 */
 #if defined(__GNUC__)
 __attribute__((weak))
 #endif
 void __unity_on_fail(const char* macro, const char* msg);
 
 #define __UNITY_CHECK(cond, MACRO, MSG) \
   do {                                   \
     if (!(cond)) {                       \
       __unity_failures++;                \
       __unity_on_fail(MACRO, MSG);       \
     }                                    \
   } while (0)
 
   #define TEST_ASSERT_FLOAT_WITHIN(delta, expected, actual)                             \
  do {                                                                                \
    float __ua_delta__    = (float)(delta);                                           \
    float __ua_expected__ = (float)(expected);                                        \
    float __ua_actual__   = (float)(actual);                                          \
    float __ua_diff__     = __ua_actual__ - __ua_expected__;                          \
    if (__ua_diff__ < 0.0f) __ua_diff__ = -__ua_diff__;                               \
    __UNITY_CHECK(__ua_diff__ <= __ua_delta__, "TEST_ASSERT_FLOAT_WITHIN", "");       \
  } while (0)

 #define TEST_ASSERT_TRUE(cond)                  __UNITY_CHECK((cond), "TEST_ASSERT_TRUE", "")
 #define TEST_ASSERT_TRUE_MESSAGE(cond, msg)     __UNITY_CHECK((cond), "TEST_ASSERT_TRUE_MESSAGE", (msg))
 
 #define TEST_ASSERT_EQUAL(exp, act)             __UNITY_CHECK(((exp) == (act)), "TEST_ASSERT_EQUAL", "")
 #define TEST_ASSERT_EQUAL_MESSAGE(exp, act, msg) __UNITY_CHECK(((exp) == (act)), "TEST_ASSERT_EQUAL_MESSAGE", (msg))
 
 #define TEST_ASSERT_EQUAL_INT8_MESSAGE(exp, act, msg)   __UNITY_CHECK(((int8_t)(exp) == (int8_t)(act)), "TEST_ASSERT_EQUAL_INT8_MESSAGE", (msg))
 #define TEST_ASSERT_EQUAL_INT8(exp, act)   TEST_ASSERT_EQUAL_INT8_MESSAGE((exp), (act), "")
 

 #define TEST_ASSERT_EQUAL_UINT8_MESSAGE(exp, act, msg)   __UNITY_CHECK(((uint8_t)(exp) == (uint8_t)(act)), "TEST_ASSERT_EQUAL_UINT8_MESSAGE", (msg))
 #define TEST_ASSERT_EQUAL_INT16_MESSAGE(exp, act, msg)   __UNITY_CHECK(((int16_t)(exp) == (int16_t)(act)), "TEST_ASSERT_EQUAL_INT16_MESSAGE", (msg))
 #define TEST_ASSERT_EQUAL_INT32_MESSAGE(exp, act, msg)   __UNITY_CHECK(((int32_t)(exp) == (int32_t)(act)), "TEST_ASSERT_EQUAL_INT32_MESSAGE", (msg))
 
#define TEST_ASSERT_EQUAL_INT8_ARRAY(expected, actual, num_elements) \
  do { \
    int __ua_i__; \
    for (__ua_i__ = 0; __ua_i__ < (num_elements); __ua_i__++) { \
      __UNITY_CHECK(((int8_t*)(expected))[__ua_i__] == ((int8_t*)(actual))[__ua_i__], "TEST_ASSERT_EQUAL_INT8_ARRAY", ""); \
    } \
  } while (0)

#define TEST_ASSERT_EQUAL_INT16_ARRAY(expected, actual, num_elements) \
  do { \
    int __ua_i__; \
    for (__ua_i__ = 0; __ua_i__ < (num_elements); __ua_i__++) { \
      __UNITY_CHECK(((int16_t*)(expected))[__ua_i__] == ((int16_t*)(actual))[__ua_i__], "TEST_ASSERT_EQUAL_INT16_ARRAY", ""); \
    } \
  } while (0)

#define TEST_ASSERT_EQUAL_INT32_ARRAY(expected, actual, num_elements) \
  do { \
    int __ua_i__; \
    for (__ua_i__ = 0; __ua_i__ < (num_elements); __ua_i__++) { \
      __UNITY_CHECK(((int32_t*)(expected))[__ua_i__] == ((int32_t*)(actual))[__ua_i__], "TEST_ASSERT_EQUAL_INT32_ARRAY", ""); \
    } \
  } while (0)

#define TEST_ASSERT(cond)                       TEST_ASSERT_TRUE(cond)
#define TEST_ASSERT_EQUAL_INT32(exp, act)       TEST_ASSERT_EQUAL_INT32_MESSAGE((exp), (act), "")

#define TEST_FAIL_MESSAGE(msg)                  __UNITY_CHECK(0, "TEST_FAIL_MESSAGE", (msg))
 

 

#ifdef __cplusplus
}
#endif

#endif // UNITY_H
