#include "unity.h"

#include <inttypes.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

typedef struct
{
  const char *fileName;
  const char *currentTestName;
  UNITY_LINE_TYPE currentTestLine;
  unsigned int testCount;
  unsigned int failCount;
  unsigned int currentTestFailed;
  jmp_buf abortFrame;
} UnityState_t;

static UnityState_t g_unityState;

static void UnityAbort(void)
{
  longjmp(g_unityState.abortFrame, 1);
}

static void UnityReportFailure(UNITY_LINE_TYPE line,
                               const char *message,
                               const char *format,
                               ...)
{
  char detail[256];
  va_list args;

  va_start(args, format);
  (void) vsnprintf(detail, sizeof(detail), format, args);
  va_end(args);

  if (g_unityState.currentTestFailed == 0U)
  {
    g_unityState.failCount++;
    g_unityState.currentTestFailed = 1U;
  }

  fprintf(stderr,
          "%s:%u:%s:FAIL: %s",
          (g_unityState.fileName != NULL) ? g_unityState.fileName : "<unknown>",
          line,
          (g_unityState.currentTestName != NULL) ? g_unityState.currentTestName
                                                 : "<unknown>",
          detail);

  if ((message != NULL) && (message[0] != '\0'))
  {
    fprintf(stderr, " (%s)", message);
  }

  fputc('\n', stderr);
  UnityAbort();
}

int UnityBegin(const char *filename)
{
  memset(&g_unityState, 0, sizeof(g_unityState));
  g_unityState.fileName = filename;
  return 0;
}

int UnityEnd(void)
{
  if (g_unityState.failCount == 0U)
  {
    fprintf(stdout, "OK (%u tests)\n", g_unityState.testCount);
    return 0;
  }

  fprintf(stderr,
          "FAILED (%u tests, %u failures)\n",
          g_unityState.testCount,
          g_unityState.failCount);
  return (int) g_unityState.failCount;
}

void UnityDefaultTestRun(UnityTestFunction func,
                         const char *funcName,
                         UNITY_LINE_TYPE lineNum)
{
  g_unityState.currentTestName = funcName;
  g_unityState.currentTestLine = lineNum;
  g_unityState.currentTestFailed = 0U;
  g_unityState.testCount++;

  if (setjmp(g_unityState.abortFrame) == 0)
  {
    setUp();
    func();
  }

  if (setjmp(g_unityState.abortFrame) == 0)
  {
    tearDown();
  }
}

void UnityFail(const char *message, UNITY_LINE_TYPE line)
{
  UnityReportFailure(line,
                     message,
                     "Explicit failure at line %u",
                     (unsigned int) line);
}

void UnityAssertTrue(int condition,
                     const char *message,
                     UNITY_LINE_TYPE line)
{
  if (condition == 0)
  {
    UnityReportFailure(line, message, "Expression evaluated false");
  }
}

void UnityAssertEqualSigned(long long expected,
                            long long actual,
                            const char *message,
                            UNITY_LINE_TYPE line)
{
  if (expected != actual)
  {
    UnityReportFailure(line,
                       message,
                       "Expected %lld Was %lld",
                       expected,
                       actual);
  }
}

void UnityAssertEqualUnsigned(unsigned long long expected,
                              unsigned long long actual,
                              const char *message,
                              UNITY_LINE_TYPE line)
{
  if (expected != actual)
  {
    UnityReportFailure(line,
                       message,
                       "Expected %llu Was %llu",
                       expected,
                       actual);
  }
}

void UnityAssertEqualHex(unsigned long long expected,
                         unsigned long long actual,
                         unsigned int width,
                         const char *message,
                         UNITY_LINE_TYPE line)
{
  if (expected != actual)
  {
    char expectedHex[32];
    char actualHex[32];

    (void) snprintf(expectedHex,
                    sizeof(expectedHex),
                    "0x%0*llX",
                    (int) width,
                    expected);
    (void) snprintf(actualHex,
                    sizeof(actualHex),
                    "0x%0*llX",
                    (int) width,
                    actual);
    UnityReportFailure(line,
                       message,
                       "Expected %s Was %s",
                       expectedHex,
                       actualHex);
  }
}

void UnityAssertNotEqualSigned(long long expected,
                               long long actual,
                               const char *message,
                               UNITY_LINE_TYPE line)
{
  if (expected == actual)
  {
    UnityReportFailure(line,
                       message,
                       "Values unexpectedly matched: %lld",
                       actual);
  }
}

void UnityAssertNull(const void *pointer,
                     const char *message,
                     UNITY_LINE_TYPE line)
{
  if (pointer != NULL)
  {
    UnityReportFailure(line, message, "Pointer was not NULL");
  }
}

void UnityAssertNotNull(const void *pointer,
                        const char *message,
                        UNITY_LINE_TYPE line)
{
  if (pointer == NULL)
  {
    UnityReportFailure(line, message, "Pointer was NULL");
  }
}

void UnityAssertEqualMemory(const void *expected,
                            const void *actual,
                            size_t length,
                            const char *message,
                            UNITY_LINE_TYPE line)
{
  if ((expected == NULL) || (actual == NULL))
  {
    if (expected != actual)
    {
      UnityReportFailure(line, message, "Memory pointer mismatch");
    }
    return;
  }

  if (memcmp(expected, actual, length) != 0)
  {
    UnityReportFailure(line,
                       message,
                       "Memory blocks differed over %zu bytes",
                       length);
  }
}

void UnityAssertEqualString(const char *expected,
                            const char *actual,
                            const char *message,
                            UNITY_LINE_TYPE line)
{
  if ((expected == NULL) || (actual == NULL))
  {
    if (expected != actual)
    {
      UnityReportFailure(line, message, "String pointer mismatch");
    }
    return;
  }

  if (strcmp(expected, actual) != 0)
  {
    UnityReportFailure(line,
                       message,
                       "Expected \"%s\" Was \"%s\"",
                       expected,
                       actual);
  }
}

void UnityAssertEqualUInt8Array(const uint8_t *expected,
                                const uint8_t *actual,
                                size_t length,
                                const char *message,
                                UNITY_LINE_TYPE line)
{
  size_t index;

  if ((expected == NULL) || (actual == NULL))
  {
    if (expected != actual)
    {
      UnityReportFailure(line, message, "Array pointer mismatch");
    }
    return;
  }

  for (index = 0U; index < length; index++)
  {
    if (expected[index] != actual[index])
    {
      UnityReportFailure(line,
                         message,
                         "Array mismatch at index %zu: expected %" PRIu8
                         " was %" PRIu8,
                         index,
                         expected[index],
                         actual[index]);
    }
  }
}

void UnityAssertEqualUInt32Array(const uint32_t *expected,
                                 const uint32_t *actual,
                                 size_t length,
                                 const char *message,
                                 UNITY_LINE_TYPE line)
{
  size_t index;

  if ((expected == NULL) || (actual == NULL))
  {
    if (expected != actual)
    {
      UnityReportFailure(line, message, "Array pointer mismatch");
    }
    return;
  }

  for (index = 0U; index < length; index++)
  {
    if (expected[index] != actual[index])
    {
      UnityReportFailure(line,
                         message,
                         "Array mismatch at index %zu: expected %" PRIu32
                         " was %" PRIu32,
                         index,
                         expected[index],
                         actual[index]);
    }
  }
}

void UnityAssertGreaterThanUnsigned(unsigned long long threshold,
                                    unsigned long long actual,
                                    const char *message,
                                    UNITY_LINE_TYPE line)
{
  if (actual <= threshold)
  {
    UnityReportFailure(line,
                       message,
                       "Expected > %llu Was %llu",
                       threshold,
                       actual);
  }
}

void UnityAssertLessThanUnsigned(unsigned long long threshold,
                                 unsigned long long actual,
                                 const char *message,
                                 UNITY_LINE_TYPE line)
{
  if (actual >= threshold)
  {
    UnityReportFailure(line,
                       message,
                       "Expected < %llu Was %llu",
                       threshold,
                       actual);
  }
}

void UnityAssertBitsHigh(unsigned long long mask,
                         unsigned long long actual,
                         const char *message,
                         UNITY_LINE_TYPE line)
{
  if ((actual & mask) != mask)
  {
    UnityReportFailure(line,
                       message,
                       "Expected bits high mask=0x%llX actual=0x%llX",
                       mask,
                       actual);
  }
}

void UnityAssertBitsLow(unsigned long long mask,
                        unsigned long long actual,
                        const char *message,
                        UNITY_LINE_TYPE line)
{
  if ((actual & mask) != 0U)
  {
    UnityReportFailure(line,
                       message,
                       "Expected bits low mask=0x%llX actual=0x%llX",
                       mask,
                       actual);
  }
}
