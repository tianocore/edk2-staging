/** @file
  Provides the basic types and common elements of the unit test framework

  Copyright (c) 2016, Microsoft Corporation. All rights reserved.<BR>
  Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __UNIT_TEST_TYPES_H__
#define __UNIT_TEST_TYPES_H__

#include <Library/UnitTestLib.h>

///================================================================================================
///================================================================================================
///
/// HANDY DEFINITIONS
///
///================================================================================================
///================================================================================================
#define UNIT_TEST_MAX_STRING_LENGTH  (120)

#define UNIT_TEST_FINGERPRINT_SIZE       (sizeof(UINT32))    // Hardcoded to CRC32.
#define UNIT_TEST_TESTFAILUREMSG_LENGTH  (120)

typedef UINT32 FAILURE_TYPE;
#define FAILURETYPE_NOFAILURE          (0)
#define FAILURETYPE_OTHER              (1)
#define FAILURETYPE_ASSERTTRUE         (2)
#define FAILURETYPE_ASSERTFALSE        (3)
#define FAILURETYPE_ASSERTEQUAL        (4)
#define FAILURETYPE_ASSERTNOTEQUAL     (5)
#define FAILURETYPE_ASSERTNOTEFIERROR  (6)
#define FAILURETYPE_ASSERTSTATUSEQUAL  (7)
#define FAILURETYPE_ASSERTNOTNULL      (8)


///================================================================================================
///================================================================================================
///
/// UNIT TEST DATA STRUCTURE DEFINITIONS
///
///================================================================================================
///================================================================================================

typedef struct {
  CHAR8                     *Description;
  CHAR8                     *ClassName;  //can't have spaces and should be short
  CHAR8                     *Log;
  FAILURE_TYPE              FailureType;
  CHAR8                     FailureMessage[UNIT_TEST_TESTFAILUREMSG_LENGTH];
  UINT8                     Fingerprint[UNIT_TEST_FINGERPRINT_SIZE];
  UNIT_TEST_STATUS          Result;
  UNIT_TEST_FUNCTION        RunTest;
  UNIT_TEST_PREREQ          PreReq;
  UNIT_TEST_CLEANUP         CleanUp;
  UNIT_TEST_CONTEXT         Context;
  UNIT_TEST_SUITE_HANDLE    ParentSuite;
} UNIT_TEST;

typedef struct {
  LIST_ENTRY    Entry;
  UNIT_TEST     UT;
} UNIT_TEST_LIST_ENTRY;

typedef struct {
  UINTN                       NumTests;
  CHAR8                       *Title;
  CHAR8                       *Package;
  UINT8                       Fingerprint[UNIT_TEST_FINGERPRINT_SIZE];
  UNIT_TEST_SUITE_SETUP       Setup;
  UNIT_TEST_SUITE_TEARDOWN    Teardown;
  LIST_ENTRY                  TestCaseList;     // UNIT_TEST_LIST_ENTRY
  UNIT_TEST_FRAMEWORK_HANDLE  ParentFramework;
} UNIT_TEST_SUITE;

typedef struct {
  LIST_ENTRY        Entry;
  UNIT_TEST_SUITE   UTS;
} UNIT_TEST_SUITE_LIST_ENTRY;

typedef struct {
  CHAR8                     *Title;
  CHAR8                     *ShortTitle;      // This title should contain NO spaces or non-filename charatecters. Is used in reporting and serialization.
  CHAR8                     *VersionString;
  CHAR8                     *Log;
  UINT8                     Fingerprint[UNIT_TEST_FINGERPRINT_SIZE];
  LIST_ENTRY                TestSuiteList;    // UNIT_TEST_SUITE_LIST_ENTRY
  EFI_TIME                  StartTime;
  EFI_TIME                  EndTime;
  UNIT_TEST                 *CurrentTest;
  VOID                      *SavedState;      // This is an instance of UNIT_TEST_SAVE_HEADER*, if present.
} UNIT_TEST_FRAMEWORK;

//
//Structures for the framework to serializing unit test status
//

typedef struct
{
  UINT32            Size;                                         // Size of the UNIT_TEST_SAVE_TEST including Log[]
  UINT8             Fingerprint[UNIT_TEST_FINGERPRINT_SIZE];      // Fingerprint of the test itself.
  CHAR8             FailureMessage[UNIT_TEST_TESTFAILUREMSG_LENGTH];
  FAILURE_TYPE      FailureType;
  UNIT_TEST_STATUS  Result;
  CHAR8             Log[];
} UNIT_TEST_SAVE_TEST;

typedef struct
{
  UINT32            Size;                                         // Size of the UNIT_TEST_SAVE_CONTEXT including Data[]
  UINT8             Fingerprint[UNIT_TEST_FINGERPRINT_SIZE];      // Fingerprint of the corresponding test.
  UINT8             Data[];                                       // Actual data of the context.
} UNIT_TEST_SAVE_CONTEXT;

typedef struct
{
  UINT8             Version;
  UINT32            SaveStateSize;                                // Size of the entire serialized buffer.
  UINT8             Fingerprint[UNIT_TEST_FINGERPRINT_SIZE];      // Fingerprint of the framework that has been saved.
  EFI_TIME          StartTime;
  UINT32            TestCount;
  BOOLEAN           HasSavedContext;
  // UNIT_TEST_SAVE_TEST    Tests[];                              // Array of structures starts here.
  // UNIT_TEST_SAVE_CONTEXT SavedContext[];                       // Saved context for the currently running test.
  // CHAR8                  Log[];                                // NOTE: Not yet implemented!!
} UNIT_TEST_SAVE_HEADER;

/**
  This function is responsible for initializing the log buffer for a single test. It can
  be used internally, but may also be consumed by the test framework to add pre-existing
  data to a log before it's used.

  @param[in,out]  TestHandle    A handle to the test being initialized.
  @param[in]      Buffer        [Optional] A pointer to pre-existing log data that should
                                be used to initialize the log. Should include a NULL terminator.
  @param[in]      BufferSize    [Optional] The size of the pre-existing log data.

**/
VOID
EFIAPI
UnitTestLogInit (
  IN OUT UNIT_TEST_HANDLE   TestHandle,
  IN UINT8                  *Buffer     OPTIONAL,
  IN UINTN                  BufferSize  OPTIONAL
  );

#endif
