/**
  Implement UnitTestLib

  Copyright (c) Microsoft Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <Library/UnitTestLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/UnitTestPersistenceLib.h>
#include <Library/UnitTestResultReportLib.h>

//
// Forward declaration of prototype
//
STATIC
VOID
UpdateTestFromSave (
  IN OUT UNIT_TEST              *Test,
  IN     UNIT_TEST_SAVE_HEADER  *SavedState
  );

/**
  This function will determine whether the short name violates any rules that would
  prevent it from being used as a reporting name or as a serialization name.

  Example: If the name cannot be serialized to a filesystem file name.

  @param[in]  ShortTitleString  A pointer to the short title string to be evaluated.

  @retval  TRUE   The string is acceptable.
  @retval  FALSE  The string should not be used.

**/
STATIC
BOOLEAN
IsFrameworkShortNameValid (
  IN CHAR8  *ShortTitleString
  )
{
  // TODO: Finish this function.
  return TRUE;
}

STATIC
CHAR8*
AllocateAndCopyString (
  IN CHAR8  *StringToCopy
  )
{
  CHAR8  *NewString;
  UINTN  NewStringLength;

  NewString = NULL;
  NewStringLength = AsciiStrnLenS (StringToCopy, UNIT_TEST_MAX_STRING_LENGTH) + 1;
  NewString = AllocatePool (NewStringLength * sizeof( CHAR8 ));
  if (NewString != NULL) {
    AsciiStrCpyS (NewString, NewStringLength, StringToCopy);
  }
  return NewString;
}

STATIC
VOID
SetFrameworkFingerprint (
  OUT UINT8                *Fingerprint,
  IN  UNIT_TEST_FRAMEWORK  *Framework
  )
{
  UINT32  NewFingerprint;

  // For this one we'll just use the title and version as the unique fingerprint.
  NewFingerprint = CalculateCrc32( Framework->Title, (AsciiStrLen( Framework->Title ) * sizeof( CHAR8 )) );
  NewFingerprint = (NewFingerprint >> 8) ^ CalculateCrc32( Framework->VersionString, (AsciiStrLen( Framework->VersionString ) * sizeof( CHAR8 )) );

  CopyMem( Fingerprint, &NewFingerprint, UNIT_TEST_FINGERPRINT_SIZE );
  return;
}

STATIC
VOID
SetSuiteFingerprint (
  OUT UINT8                *Fingerprint,
  IN  UNIT_TEST_FRAMEWORK  *Framework,
  IN  UNIT_TEST_SUITE      *Suite
  )
{
  UINT32  NewFingerprint;

  // For this one, we'll use the fingerprint from the framework, and the title of the suite.
  NewFingerprint = CalculateCrc32( &Framework->Fingerprint[0], UNIT_TEST_FINGERPRINT_SIZE );
  NewFingerprint = (NewFingerprint >> 8) ^ CalculateCrc32( Suite->Title, (AsciiStrLen( Suite->Title ) * sizeof( CHAR8 )) );
  NewFingerprint = (NewFingerprint >> 8) ^ CalculateCrc32( Suite->Package, (AsciiStrLen(Suite->Package) * sizeof(CHAR8)) );

  CopyMem( Fingerprint, &NewFingerprint, UNIT_TEST_FINGERPRINT_SIZE );
  return;
}

STATIC
VOID
SetTestFingerprint (
  OUT UINT8            *Fingerprint,
  IN  UNIT_TEST_SUITE  *Suite,
  IN  UNIT_TEST        *Test
  )
{
  UINT32  NewFingerprint;

  // For this one, we'll use the fingerprint from the suite, and the description and classname of the test.
  NewFingerprint = CalculateCrc32( &Suite->Fingerprint[0], UNIT_TEST_FINGERPRINT_SIZE );
  NewFingerprint = (NewFingerprint >> 8) ^ CalculateCrc32( Test->Description, (AsciiStrLen( Test->Description ) * sizeof( CHAR8 )) );
  NewFingerprint = (NewFingerprint >> 8) ^ CalculateCrc32( Test->ClassName, (AsciiStrLen(Test->ClassName) * sizeof(CHAR8)) );

  CopyMem( Fingerprint, &NewFingerprint, UNIT_TEST_FINGERPRINT_SIZE );
  return;
}

STATIC
BOOLEAN
CompareFingerprints (
  IN UINT8  *FingerprintA,
  IN UINT8  *FingerprintB
  )
{
  return (CompareMem( FingerprintA, FingerprintB, UNIT_TEST_FINGERPRINT_SIZE ) == 0);
}

EFI_STATUS
EFIAPI
FreeUnitTestFramework (
  IN UNIT_TEST_FRAMEWORK_HANDLE  Framework
  )
{
  // TODO: Finish this function.
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
FreeUnitTestSuiteEntry (
  IN UNIT_TEST_SUITE_LIST_ENTRY  *SuiteEntry
  )
{
  // TODO: Finish this function.
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
FreeUnitTestTestEntry (
  IN UNIT_TEST_LIST_ENTRY  *TestEntry
  )
{
  // TODO: Finish this function.
  return EFI_SUCCESS;
}

/*
  Method to Initialize the Unit Test framework

  @retval  Success    - Unit Test init.
  @retval  EFI_ERROR  - Unit Tests init failed.
*/
EFI_STATUS
EFIAPI
InitUnitTestFramework (
  OUT UNIT_TEST_FRAMEWORK_HANDLE  *Framework,
  IN  CHAR8                       *Title,
  IN  CHAR8                       *ShortTitle,
  IN  CHAR8                       *VersionString
  )
{
  EFI_STATUS             Status;
  UNIT_TEST_FRAMEWORK    *NewFramework;
  UNIT_TEST_SAVE_HEADER  *SavedState;

  Status       = EFI_SUCCESS;
  NewFramework = NULL;

  //
  // First, check all pointers and make sure nothing's broked.
  //
  if (Framework == NULL || Title == NULL ||
      ShortTitle == NULL || VersionString == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Next, determine whether all of the strings are good to use.
  //
  if (!IsFrameworkShortNameValid (ShortTitle)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Next, set aside some space to start messing with the framework.
  //
  NewFramework = AllocateZeroPool (sizeof (UNIT_TEST_FRAMEWORK));
  if (NewFramework == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Next, set up all the test data.
  //
  NewFramework->Title         = AllocateAndCopyString (Title);
  NewFramework->ShortTitle    = AllocateAndCopyString (ShortTitle);
  NewFramework->VersionString = AllocateAndCopyString (VersionString);
  NewFramework->Log           = NULL;
  NewFramework->CurrentTest   = NULL;
  NewFramework->SavedState    = NULL;
  if (NewFramework->Title == NULL ||
      NewFramework->ShortTitle == NULL ||
      NewFramework->VersionString == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }
  InitializeListHead (&(NewFramework->TestSuiteList));

  //
  // Create the framework fingerprint.
  //
  SetFrameworkFingerprint (&NewFramework->Fingerprint[0], NewFramework);

  //
  // If there is a persisted context, load it now.
  //
  if (DoesCacheExist (NewFramework)) {
    SavedState = (UNIT_TEST_SAVE_HEADER *)NewFramework->SavedState;
    Status = LoadUnitTestCache (NewFramework, &SavedState);
    if (EFI_ERROR (Status)) {
      //
      // Don't actually report it as an error, but emit a warning.
      //
      DEBUG (( DEBUG_ERROR, "%a - Cache was detected, but failed to load.\n", __FUNCTION__ ));
      Status = EFI_SUCCESS;
    }
  }

Exit:
  //
  // If we're good, then let's copy the framework.
  //
  if (!EFI_ERROR (Status)) {
    *Framework = NewFramework;
  } else {
    //
    // Otherwise, we need to undo this horrible thing that we've done.
    //
    FreeUnitTestFramework ((UNIT_TEST_FRAMEWORK_HANDLE)NewFramework);
  }

  return Status;
}

EFI_STATUS
EFIAPI
CreateUnitTestSuite (
  OUT UNIT_TEST_SUITE_HANDLE      *Suite,
  IN  UNIT_TEST_FRAMEWORK_HANDLE  FrameworkHandle,
  IN  CHAR8                       *Title,
  IN  CHAR8                       *Package,
  IN  UNIT_TEST_SUITE_SETUP       Sup,              OPTIONAL
  IN  UNIT_TEST_SUITE_TEARDOWN    Tdn               OPTIONAL
  )
{
  EFI_STATUS                  Status;
  UNIT_TEST_SUITE_LIST_ENTRY  *NewSuiteEntry;
  UNIT_TEST_FRAMEWORK         *Framework;

  Status = EFI_SUCCESS;
  Framework = (UNIT_TEST_FRAMEWORK *)FrameworkHandle;

  //
  // First, let's check to make sure that our parameters look good.
  //
  if ((Framework == NULL) || (Title == NULL) || (Package == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Create the new entry.
  //
  NewSuiteEntry = AllocateZeroPool (sizeof (UNIT_TEST_SUITE_LIST_ENTRY));
  if (NewSuiteEntry == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Copy the fields we think we need.
  //
  NewSuiteEntry->UTS.NumTests         = 0;
  NewSuiteEntry->UTS.Title            = AllocateAndCopyString (Title);
  NewSuiteEntry->UTS.Package          = AllocateAndCopyString (Package);
  NewSuiteEntry->UTS.Setup            = Sup;
  NewSuiteEntry->UTS.Teardown         = Tdn;
  NewSuiteEntry->UTS.ParentFramework  = Framework;
  InitializeListHead (&(NewSuiteEntry->Entry));             // List entry for sibling suites.
  InitializeListHead (&(NewSuiteEntry->UTS.TestCaseList));  // List entry for child tests.
  if (NewSuiteEntry->UTS.Title == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  if (NewSuiteEntry->UTS.Package == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  //
  // Create the suite fingerprint.
  //
  SetSuiteFingerprint( &NewSuiteEntry->UTS.Fingerprint[0], Framework, &NewSuiteEntry->UTS );

Exit:
  //
  // If everything is going well, add the new suite to the tail list for the framework.
  //
  if (!EFI_ERROR( Status )) {
    InsertTailList (&(Framework->TestSuiteList), (LIST_ENTRY *)NewSuiteEntry);
    *Suite = &NewSuiteEntry->UTS;
  } else {
    //
    // Otherwise, make with the destruction.
    //
    FreeUnitTestSuiteEntry (NewSuiteEntry);
  }

  return Status;
}

EFI_STATUS
EFIAPI
AddTestCase (
  IN UNIT_TEST_SUITE_HANDLE  SuiteHandle,
  IN CHAR8                   *Description,
  IN CHAR8                   *ClassName,
  IN UNIT_TEST_FUNCTION      Func,
  IN UNIT_TEST_PREREQ        PreReq,       OPTIONAL
  IN UNIT_TEST_CLEANUP       CleanUp,      OPTIONAL
  IN UNIT_TEST_CONTEXT       Context       OPTIONAL
  )
{
  EFI_STATUS            Status;
  UNIT_TEST_LIST_ENTRY  *NewTestEntry;
  UNIT_TEST_FRAMEWORK   *ParentFramework;
  UNIT_TEST_SUITE       *Suite;

  Status          = EFI_SUCCESS;
  Suite           = (UNIT_TEST_SUITE *)SuiteHandle;
  ParentFramework = (UNIT_TEST_FRAMEWORK *)Suite->ParentFramework;

  //
  // First, let's check to make sure that our parameters look good.
  //
  if ((Suite == NULL) || (Description == NULL) || (ClassName == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Create the new entry.
  NewTestEntry = AllocateZeroPool (sizeof( UNIT_TEST_LIST_ENTRY ));
  if (NewTestEntry == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Copy the fields we think we need.
  NewTestEntry->UT.Description       = AllocateAndCopyString (Description);
  NewTestEntry->UT.ClassName         = AllocateAndCopyString (ClassName);
  NewTestEntry->UT.FailureType       = FAILURETYPE_NOFAILURE;
  NewTestEntry->UT.FailureMessage[0] = '\0';
  NewTestEntry->UT.Log               = NULL;
  NewTestEntry->UT.PreReq            = PreReq;
  NewTestEntry->UT.CleanUp           = CleanUp;
  NewTestEntry->UT.RunTest           = Func;
  NewTestEntry->UT.Context           = Context;
  NewTestEntry->UT.Result            = UNIT_TEST_PENDING;
  NewTestEntry->UT.ParentSuite       = Suite;
  InitializeListHead (&(NewTestEntry->Entry));  // List entry for sibling tests.
  if (NewTestEntry->UT.Description == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }

  //
  // Create the test fingerprint.
  //
  SetTestFingerprint (&NewTestEntry->UT.Fingerprint[0], Suite, &NewTestEntry->UT);

  // TODO: Make sure that duplicate fingerprints cannot be created.

  //
  // If there is saved test data, update this record.
  //
  if (ParentFramework->SavedState != NULL) {
    UpdateTestFromSave (&NewTestEntry->UT, ParentFramework->SavedState);
  }

Exit:
  //
  // If everything is going well, add the new suite to the tail list for the framework.
  //
  if (!EFI_ERROR (Status)) {
    InsertTailList (&(Suite->TestCaseList), (LIST_ENTRY*)NewTestEntry);
    Suite->NumTests++;
  } else {
    //
    // Otherwise, make with the destruction.
    //
    FreeUnitTestTestEntry (NewTestEntry);
  }

  return Status;
}

STATIC
VOID
UpdateTestFromSave (
  IN OUT UNIT_TEST              *Test,
  IN     UNIT_TEST_SAVE_HEADER  *SavedState
  )
{
  UNIT_TEST_SAVE_TEST     *CurrentTest;
  UNIT_TEST_SAVE_TEST     *MatchingTest;
  UINT8                   *FloatingPointer;
  UNIT_TEST_SAVE_CONTEXT  *SavedContext;
  UINTN                   Index;

  //
  // First, evaluate the inputs.
  //
  if (Test == NULL || SavedState == NULL) {
    return;
  }
  if (SavedState->TestCount == 0) {
    return;
  }

  //
  // Next, determine whether a matching test can be found.
  // Start at the beginning.
  //
  MatchingTest    = NULL;
  FloatingPointer = (UINT8 *)SavedState + sizeof (*SavedState);
  for (Index = 0; Index < SavedState->TestCount; Index++) {
    CurrentTest = (UNIT_TEST_SAVE_TEST *)FloatingPointer;
    if (CompareFingerprints (&Test->Fingerprint[0], &CurrentTest->Fingerprint[0])) {
      MatchingTest = CurrentTest;
      //
      // If there's a saved context, it's important that we iterate through the entire list.
      //
      if (!SavedState->HasSavedContext) {
        break;
      }
    }

    //
    // If we didn't find it, we have to increment to the next test.
    //
    FloatingPointer = (UINT8 *)CurrentTest + CurrentTest->Size;
  }

  //
  // If a matching test was found, copy the status.
  //
  if (MatchingTest) {
    //
    // Override the test status with the saved status.
    //
    Test->Result = MatchingTest->Result;

    Test->FailureType = MatchingTest->FailureType;
    AsciiStrnCpyS (
      &Test->FailureMessage[0],
      UNIT_TEST_TESTFAILUREMSG_LENGTH,
      &MatchingTest->FailureMessage[0],
      UNIT_TEST_TESTFAILUREMSG_LENGTH
      );

    //
    // If there is a log string associated, grab that.
    // We can tell that there's a log string because the "size" will be larger than
    // the structure size.
    // IMPORTANT NOTE: There are security implications here.
    //                 This data is user-supplied and we're about to play kinda
    //                 fast and loose with data buffers.
    //
    if (MatchingTest->Size > sizeof (UNIT_TEST_SAVE_TEST)) {
      UnitTestLogInit (Test, (UINT8 *)MatchingTest->Log, MatchingTest->Size - sizeof (UNIT_TEST_SAVE_TEST));
    }
  }

  //
  // If the saved context exists and matches this test, grab it, too.
  //
  if (SavedState->HasSavedContext) {
    //
    // If there was a saved context, the "matching test" loop will have placed the FloatingPointer
    // at the beginning of the context structure.
    //
    SavedContext = (UNIT_TEST_SAVE_CONTEXT *)FloatingPointer;
    if ((SavedContext->Size - sizeof (UNIT_TEST_SAVE_CONTEXT)) > 0 &&
        CompareFingerprints (&Test->Fingerprint[0], &SavedContext->Fingerprint[0])) {
      //
      // Override the test context with the saved context.
      //
      Test->Context = (VOID*)SavedContext->Data;
    }
  }
}

STATIC
UNIT_TEST_SAVE_HEADER*
SerializeState (
  IN UNIT_TEST_FRAMEWORK_HANDLE  FrameworkHandle,
  IN UNIT_TEST_CONTEXT           ContextToSave,      OPTIONAL
  IN UINTN                       ContextToSaveSize
  )
{
  UNIT_TEST_FRAMEWORK     *Framework;
  UNIT_TEST_SAVE_HEADER   *Header;
  LIST_ENTRY              *SuiteListHead;
  LIST_ENTRY              *Suite;
  LIST_ENTRY              *TestListHead;
  LIST_ENTRY              *Test;
  UINT32                  TestCount;
  UINT32                  TotalSize;
  UINTN                   LogSize;
  UNIT_TEST_SAVE_TEST     *TestSaveData;
  UNIT_TEST_SAVE_CONTEXT  *TestSaveContext;
  UNIT_TEST               *UnitTest;
  UINT8                   *FloatingPointer;

  Framework = (UNIT_TEST_FRAMEWORK *)FrameworkHandle;
  Header    = NULL;

  //
  // First, let's not make assumptions about the parameters.
  //
  if (Framework == NULL ||
      (ContextToSave != NULL && ContextToSaveSize == 0) ||
      ContextToSaveSize > MAX_UINT32) {
    return NULL;
  }

  //
  // Next, we've gotta figure out the resources that will be required to serialize the
  // the framework state so that we can persist it.
  // To start with, we're gonna need a header.
  //
  TotalSize = sizeof (UNIT_TEST_SAVE_HEADER);
  //
  // Now we need to figure out how many tests there are.
  //
  TestCount = 0;
  //
  // Iterate all suites.
  //
  SuiteListHead = &Framework->TestSuiteList;
  for (Suite = GetFirstNode (SuiteListHead); Suite != SuiteListHead; Suite = GetNextNode (SuiteListHead, Suite)) {
    //
    // Iterate all tests within the suite.
    //
    TestListHead = &((UNIT_TEST_SUITE_LIST_ENTRY *)Suite)->UTS.TestCaseList;
    for (Test = GetFirstNode (TestListHead); Test != TestListHead; Test = GetNextNode (TestListHead, Test)) {
      UnitTest = &((UNIT_TEST_LIST_ENTRY *)Test)->UT;
      //
      // Account for the size of a test structure.
      //
      TotalSize += sizeof( UNIT_TEST_SAVE_TEST );
      //
      // If there's a log, make sure to account for the log size.
      //
      if (UnitTest->Log != NULL)     {
        //
        // The +1 is for the NULL character. Can't forget the NULL character.
        //
        LogSize = (AsciiStrLen (UnitTest->Log) + 1) * sizeof (CHAR8);
        ASSERT (LogSize < MAX_UINT32);
        TotalSize += (UINT32)LogSize;
      }
      //
      // Increment the test count.
      //
      TestCount++;
    }
  }
  //
  // If there are no tests, we're done here.
  //
  if (TestCount == 0) {
    return NULL;
  }
  //
  // Add room for the context, if there is one.
  //
  if (ContextToSave != NULL) {
    TotalSize += sizeof (UNIT_TEST_SAVE_CONTEXT) + (UINT32)ContextToSaveSize;
  }

  //
  // Now that we know the size, we need to allocate space for the serialized output.
  //
  Header = AllocateZeroPool (TotalSize);
  if (Header == NULL) {
    return NULL;
  }

  //
  // Alright, let's start setting up some data.
  //
  Header->Version         = UNIT_TEST_PERSISTENCE_LIB_VERSION;
  Header->SaveStateSize   = TotalSize;
  CopyMem (&Header->Fingerprint[0], &Framework->Fingerprint[0], UNIT_TEST_FINGERPRINT_SIZE);
  CopyMem (&Header->StartTime, &Framework->StartTime, sizeof (EFI_TIME));
  Header->TestCount       = TestCount;
  Header->HasSavedContext = FALSE;

  //
  // Start adding all of the test cases.
  // Set the floating pointer to the start of the current test save buffer.
  //
  FloatingPointer = (UINT8*)Header + sizeof( UNIT_TEST_SAVE_HEADER );
  //
  // Iterate all suites.
  //
  SuiteListHead = &Framework->TestSuiteList;
  for (Suite = GetFirstNode (SuiteListHead); Suite != SuiteListHead; Suite = GetNextNode (SuiteListHead, Suite)) {
    //
    // Iterate all tests within the suite.
    //
    TestListHead = &((UNIT_TEST_SUITE_LIST_ENTRY *)Suite)->UTS.TestCaseList;
    for (Test = GetFirstNode (TestListHead); Test != TestListHead; Test = GetNextNode (TestListHead, Test)) {
      TestSaveData  = (UNIT_TEST_SAVE_TEST *)FloatingPointer;
      UnitTest      = &((UNIT_TEST_LIST_ENTRY *)Test)->UT;

      //
      // Save the fingerprint.
      //
      CopyMem (&TestSaveData->Fingerprint[0], &UnitTest->Fingerprint[0], UNIT_TEST_FINGERPRINT_SIZE);

      //
      // Save the result.
      //
      TestSaveData->Result = UnitTest->Result;
      TestSaveData->FailureType = UnitTest->FailureType;
      AsciiStrnCpyS (&TestSaveData->FailureMessage[0], UNIT_TEST_TESTFAILUREMSG_LENGTH, &UnitTest->FailureMessage[0], UNIT_TEST_TESTFAILUREMSG_LENGTH);


      //
      // If there is a log, save the log.
      //
      FloatingPointer += sizeof (UNIT_TEST_SAVE_TEST);
      if (UnitTest->Log != NULL) {
        //
        // The +1 is for the NULL character. Can't forget the NULL character.
        //
        LogSize = (AsciiStrLen (UnitTest->Log) + 1) * sizeof (CHAR8);
        CopyMem (FloatingPointer, UnitTest->Log, LogSize);
        FloatingPointer += LogSize;
      }

      //
      // Update the size once the structure is complete.
      // NOTE: Should thise be a straight cast without validation?
      //
      TestSaveData->Size = (UINT32)(FloatingPointer - (UINT8 *)TestSaveData);
    }
  }

  //
  // If there is a context to save, let's do that now.
  //
  if (ContextToSave != NULL && Framework->CurrentTest != NULL) {
    TestSaveContext         = (UNIT_TEST_SAVE_CONTEXT*)FloatingPointer;
    TestSaveContext->Size   = (UINT32)ContextToSaveSize + sizeof (UNIT_TEST_SAVE_CONTEXT);
    CopyMem (&TestSaveContext->Fingerprint[0], &Framework->CurrentTest->Fingerprint[0], UNIT_TEST_FINGERPRINT_SIZE);
    CopyMem (((UINT8 *)TestSaveContext + sizeof (UNIT_TEST_SAVE_CONTEXT)), ContextToSave, ContextToSaveSize);
    Header->HasSavedContext = TRUE;
  }

  return Header;
}

EFI_STATUS
EFIAPI
SaveFrameworkState (
  IN UNIT_TEST_FRAMEWORK_HANDLE  FrameworkHandle,
  IN UNIT_TEST_CONTEXT           ContextToSave,      OPTIONAL
  IN UINTN                       ContextToSaveSize
  )
{
  EFI_STATUS             Status;
  UNIT_TEST_SAVE_HEADER  *Header;

  Header = NULL;

  //
  // First, let's not make assumptions about the parameters.
  //
  if (FrameworkHandle == NULL ||
      (ContextToSave != NULL && ContextToSaveSize == 0) ||
      ContextToSaveSize > MAX_UINT32) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Now, let's package up all the data for saving.
  //
  Header = SerializeState (FrameworkHandle, ContextToSave, ContextToSaveSize);
  if (Header == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // All that should be left to do is save it using the associated persistence lib.
  //
  Status = SaveUnitTestCache (FrameworkHandle, Header);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a - Could not save state! %r\n", __FUNCTION__, Status));
    Status = EFI_DEVICE_ERROR;
  }

  //
  // Free data that was used.
  //
  FreePool (Header);

  return Status;
}
