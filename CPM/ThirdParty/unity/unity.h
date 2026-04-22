#ifndef UNITY_H
#define UNITY_H

#include <stddef.h>
#include <stdint.h>

typedef unsigned int UNITY_LINE_TYPE;
typedef void (*UnityTestFunction)(void);

int UnityBegin(const char *filename);
int UnityEnd(void);
void UnityDefaultTestRun(UnityTestFunction func,
                         const char *funcName,
                         UNITY_LINE_TYPE lineNum);

void UnityFail(const char *message, UNITY_LINE_TYPE line);
void UnityAssertTrue(int condition,
                     const char *message,
                     UNITY_LINE_TYPE line);
void UnityAssertEqualSigned(long long expected,
                            long long actual,
                            const char *message,
                            UNITY_LINE_TYPE line);
void UnityAssertEqualUnsigned(unsigned long long expected,
                              unsigned long long actual,
                              const char *message,
                              UNITY_LINE_TYPE line);
void UnityAssertEqualHex(unsigned long long expected,
                         unsigned long long actual,
                         unsigned int width,
                         const char *message,
                         UNITY_LINE_TYPE line);
void UnityAssertNotEqualSigned(long long expected,
                               long long actual,
                               const char *message,
                               UNITY_LINE_TYPE line);
void UnityAssertNull(const void *pointer,
                     const char *message,
                     UNITY_LINE_TYPE line);
void UnityAssertNotNull(const void *pointer,
                        const char *message,
                        UNITY_LINE_TYPE line);
void UnityAssertEqualMemory(const void *expected,
                            const void *actual,
                            size_t length,
                            const char *message,
                            UNITY_LINE_TYPE line);
void UnityAssertEqualString(const char *expected,
                            const char *actual,
                            const char *message,
                            UNITY_LINE_TYPE line);
void UnityAssertEqualUInt8Array(const uint8_t *expected,
                                const uint8_t *actual,
                                size_t length,
                                const char *message,
                                UNITY_LINE_TYPE line);
void UnityAssertEqualUInt32Array(const uint32_t *expected,
                                 const uint32_t *actual,
                                 size_t length,
                                 const char *message,
                                 UNITY_LINE_TYPE line);
void UnityAssertGreaterThanUnsigned(unsigned long long threshold,
                                    unsigned long long actual,
                                    const char *message,
                                    UNITY_LINE_TYPE line);
void UnityAssertLessThanUnsigned(unsigned long long threshold,
                                 unsigned long long actual,
                                 const char *message,
                                 UNITY_LINE_TYPE line);
void UnityAssertBitsHigh(unsigned long long mask,
                         unsigned long long actual,
                         const char *message,
                         UNITY_LINE_TYPE line);
void UnityAssertBitsLow(unsigned long long mask,
                        unsigned long long actual,
                        const char *message,
                        UNITY_LINE_TYPE line);

void setUp(void);
void tearDown(void);

#define UNITY_BEGIN() UnityBegin(__FILE__)
#define UNITY_END() UnityEnd()
#define RUN_TEST(func) UnityDefaultTestRun((func), #func, (UNITY_LINE_TYPE) __LINE__)

#define TEST_ASSERT_TRUE(condition) \
  UnityAssertTrue(((condition) != 0), NULL, (UNITY_LINE_TYPE) __LINE__)
#define TEST_ASSERT_FALSE(condition) \
  UnityAssertTrue(((condition) == 0), NULL, (UNITY_LINE_TYPE) __LINE__)
#define TEST_ASSERT_EQUAL(expected, actual) \
  UnityAssertEqualSigned((long long) (expected), \
                         (long long) (actual), \
                         NULL, \
                         (UNITY_LINE_TYPE) __LINE__)
#define TEST_ASSERT_EQUAL_MESSAGE(expected, actual, message) \
  UnityAssertEqualSigned((long long) (expected), \
                         (long long) (actual), \
                         (message), \
                         (UNITY_LINE_TYPE) __LINE__)
#define TEST_ASSERT_EQUAL_INT(expected, actual) \
  UnityAssertEqualSigned((long long) (expected), \
                         (long long) (actual), \
                         NULL, \
                         (UNITY_LINE_TYPE) __LINE__)
#define TEST_ASSERT_EQUAL_INT8(expected, actual) \
  UnityAssertEqualSigned((long long) (int8_t) (expected), \
                         (long long) (int8_t) (actual), \
                         NULL, \
                         (UNITY_LINE_TYPE) __LINE__)
#define TEST_ASSERT_EQUAL_INT16(expected, actual) \
  UnityAssertEqualSigned((long long) (int16_t) (expected), \
                         (long long) (int16_t) (actual), \
                         NULL, \
                         (UNITY_LINE_TYPE) __LINE__)
#define TEST_ASSERT_EQUAL_INT32(expected, actual) \
  UnityAssertEqualSigned((long long) (int32_t) (expected), \
                         (long long) (int32_t) (actual), \
                         NULL, \
                         (UNITY_LINE_TYPE) __LINE__)
#define TEST_ASSERT_EQUAL_UINT(expected, actual) \
  UnityAssertEqualUnsigned((unsigned long long) (expected), \
                           (unsigned long long) (actual), \
                           NULL, \
                           (UNITY_LINE_TYPE) __LINE__)
#define TEST_ASSERT_EQUAL_UINT8(expected, actual) \
  UnityAssertEqualUnsigned((unsigned long long) (uint8_t) (expected), \
                           (unsigned long long) (uint8_t) (actual), \
                           NULL, \
                           (UNITY_LINE_TYPE) __LINE__)
#define TEST_ASSERT_EQUAL_UINT16(expected, actual) \
  UnityAssertEqualUnsigned((unsigned long long) (uint16_t) (expected), \
                           (unsigned long long) (uint16_t) (actual), \
                           NULL, \
                           (UNITY_LINE_TYPE) __LINE__)
#define TEST_ASSERT_EQUAL_UINT32(expected, actual) \
  UnityAssertEqualUnsigned((unsigned long long) (uint32_t) (expected), \
                           (unsigned long long) (uint32_t) (actual), \
                           NULL, \
                           (UNITY_LINE_TYPE) __LINE__)
#define TEST_ASSERT_EQUAL_HEX16(expected, actual) \
  UnityAssertEqualHex((unsigned long long) (uint16_t) (expected), \
                      (unsigned long long) (uint16_t) (actual), \
                      4U, \
                      NULL, \
                      (UNITY_LINE_TYPE) __LINE__)
#define TEST_ASSERT_EQUAL_HEX32(expected, actual) \
  UnityAssertEqualHex((unsigned long long) (uint32_t) (expected), \
                      (unsigned long long) (uint32_t) (actual), \
                      8U, \
                      NULL, \
                      (UNITY_LINE_TYPE) __LINE__)
#define TEST_ASSERT_EQUAL_MEMORY(expected, actual, length) \
  UnityAssertEqualMemory((expected), \
                         (actual), \
                         (size_t) (length), \
                         NULL, \
                         (UNITY_LINE_TYPE) __LINE__)
#define TEST_ASSERT_EQUAL_STRING(expected, actual) \
  UnityAssertEqualString((expected), \
                         (actual), \
                         NULL, \
                         (UNITY_LINE_TYPE) __LINE__)
#define TEST_ASSERT_EQUAL_CHAR(expected, actual) \
  UnityAssertEqualSigned((long long) (char) (expected), \
                         (long long) (char) (actual), \
                         NULL, \
                         (UNITY_LINE_TYPE) __LINE__)
#define TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, actual, length) \
  UnityAssertEqualUInt8Array((expected), \
                             (actual), \
                             (size_t) (length), \
                             NULL, \
                             (UNITY_LINE_TYPE) __LINE__)
#define TEST_ASSERT_EQUAL_UINT32_ARRAY(expected, actual, length) \
  UnityAssertEqualUInt32Array((expected), \
                              (actual), \
                              (size_t) (length), \
                              NULL, \
                              (UNITY_LINE_TYPE) __LINE__)
#define TEST_ASSERT_NOT_EQUAL(expected, actual) \
  UnityAssertNotEqualSigned((long long) (expected), \
                            (long long) (actual), \
                            NULL, \
                            (UNITY_LINE_TYPE) __LINE__)
#define TEST_ASSERT_NOT_NULL(pointer) \
  UnityAssertNotNull((pointer), NULL, (UNITY_LINE_TYPE) __LINE__)
#define TEST_ASSERT_NULL(pointer) \
  UnityAssertNull((pointer), NULL, (UNITY_LINE_TYPE) __LINE__)
#define TEST_ASSERT_GREATER_THAN_UINT8(threshold, actual) \
  UnityAssertGreaterThanUnsigned((unsigned long long) (uint8_t) (threshold), \
                                 (unsigned long long) (uint8_t) (actual), \
                                 NULL, \
                                 (UNITY_LINE_TYPE) __LINE__)
#define TEST_ASSERT_GREATER_THAN_UINT16(threshold, actual) \
  UnityAssertGreaterThanUnsigned((unsigned long long) (uint16_t) (threshold), \
                                 (unsigned long long) (uint16_t) (actual), \
                                 NULL, \
                                 (UNITY_LINE_TYPE) __LINE__)
#define TEST_ASSERT_GREATER_THAN_UINT32(threshold, actual) \
  UnityAssertGreaterThanUnsigned((unsigned long long) (uint32_t) (threshold), \
                                 (unsigned long long) (uint32_t) (actual), \
                                 NULL, \
                                 (UNITY_LINE_TYPE) __LINE__)
#define TEST_ASSERT_LESS_THAN_UINT16(threshold, actual) \
  UnityAssertLessThanUnsigned((unsigned long long) (uint16_t) (threshold), \
                              (unsigned long long) (uint16_t) (actual), \
                              NULL, \
                              (UNITY_LINE_TYPE) __LINE__)
#define TEST_ASSERT_BITS_HIGH(mask, actual) \
  UnityAssertBitsHigh((unsigned long long) (mask), \
                      (unsigned long long) (actual), \
                      NULL, \
                      (UNITY_LINE_TYPE) __LINE__)
#define TEST_ASSERT_BITS_LOW(mask, actual) \
  UnityAssertBitsLow((unsigned long long) (mask), \
                     (unsigned long long) (actual), \
                     NULL, \
                     (UNITY_LINE_TYPE) __LINE__)
#define TEST_FAIL_MESSAGE(message) \
  UnityFail((message), (UNITY_LINE_TYPE) __LINE__)

#endif
