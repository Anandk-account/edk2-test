/** @file
  Logo DXE Driver, install Edkii Platform Logo protocol.

Copyright (c) 2016 - 2017, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

/**

#include <Uefi.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/HiiImageEx.h>
#include <Protocol/PlatformLogo.h>
#include <Protocol/HiiPackageList.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>

typedef struct {
  EFI_IMAGE_ID                             ImageId;
  EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE    Attribute;
  INTN                                     OffsetX;
  INTN                                     OffsetY;
} LOGO_ENTRY;

EFI_HII_IMAGE_EX_PROTOCOL  *mHiiImageEx;
EFI_HII_HANDLE             mHiiHandle;
LOGO_ENTRY                 mLogos[] = {
  {
    IMAGE_TOKEN (IMG_LOGO),
    EdkiiPlatformLogoDisplayAttributeCenter,
    0,
    0
  }
};

**/

/**
  Load a platform logo image and return its data and attributes.

  @param This              The pointer to this protocol instance.
  @param Instance          The visible image instance is found.
  @param Image             Points to the image.
  @param Attribute         The display attributes of the image returned.
  @param OffsetX           The X offset of the image regarding the Attribute.
  @param OffsetY           The Y offset of the image regarding the Attribute.

  @retval EFI_SUCCESS      The image was fetched successfully.
  @retval EFI_NOT_FOUND    The specified image could not be found.
**/


/**

EFI_STATUS
EFIAPI
GetImage (
  IN     EDKII_PLATFORM_LOGO_PROTOCOL        *This,
  IN OUT UINT32                              *Instance,
  OUT EFI_IMAGE_INPUT                        *Image,
  OUT EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE  *Attribute,
  OUT INTN                                   *OffsetX,
  OUT INTN                                   *OffsetY
  )
{
  UINT32  Current;

  if ((Instance == NULL) || (Image == NULL) ||
      (Attribute == NULL) || (OffsetX == NULL) || (OffsetY == NULL))
  {
    return EFI_INVALID_PARAMETER;
  }

  Current = *Instance;
  if (Current >= ARRAY_SIZE (mLogos)) {
    return EFI_NOT_FOUND;
  }

  (*Instance)++;
  *Attribute = mLogos[Current].Attribute;
  *OffsetX   = mLogos[Current].OffsetX;
  *OffsetY   = mLogos[Current].OffsetY;
  return mHiiImageEx->GetImageEx (mHiiImageEx, mHiiHandle, mLogos[Current].ImageId, Image);
}

EDKII_PLATFORM_LOGO_PROTOCOL  mPlatformLogo = {
  GetImage
};


**/

/**
  Entrypoint of this module.

  This function is the entrypoint of this module. It installs the Edkii
  Platform Logo protocol.

  @param  ImageHandle       The firmware allocated handle for the EFI image.
  @param  SystemTable       A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.

**/

/**

EFI_STATUS
EFIAPI
InitializeLogo (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                   Status;
  EFI_HII_PACKAGE_LIST_HEADER  *PackageList;
  EFI_HII_DATABASE_PROTOCOL    *HiiDatabase;
  EFI_HANDLE                   Handle;

  Status = gBS->LocateProtocol (
                  &gEfiHiiDatabaseProtocolGuid,
                  NULL,
                  (VOID **)&HiiDatabase
                  );
  ASSERT_EFI_ERROR (Status);

  Status = gBS->LocateProtocol (
                  &gEfiHiiImageExProtocolGuid,
                  NULL,
                  (VOID **)&mHiiImageEx
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // Retrieve HII package list from ImageHandle
  //
  Status = gBS->OpenProtocol (
                  ImageHandle,
                  &gEfiHiiPackageListProtocolGuid,
                  (VOID **)&PackageList,
                  ImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "HII Image Package with logo not found in PE/COFF resource section\n"));
    return Status;
  }

  //
  // Publish HII package list to HII Database.
  //
  Status = HiiDatabase->NewPackageList (
                          HiiDatabase,
                          PackageList,
                          NULL,
                          &mHiiHandle
                          );
  if (!EFI_ERROR (Status)) {
    Handle = NULL;
    Status = gBS->InstallMultipleProtocolInterfaces (
                    &Handle,
                    &gEdkiiPlatformLogoProtocolGuid,
                    &mPlatformLogo,
                    NULL
                    );
  }

  return Status;
}


**/


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// message print behind the logo and no delay used.


/** @file
  Logo DXE Driver with F12 key shutdown functionality.
  Combines logo display with keyboard input detection.

Copyright (c) 2016 - 2017, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

/**

#include <Uefi.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/HiiImageEx.h>
#include <Protocol/PlatformLogo.h>
#include <Protocol/HiiPackageList.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/SimpleTextOut.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>

typedef struct {
  EFI_IMAGE_ID                             ImageId;
  EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE    Attribute;
  INTN                                     OffsetX;
  INTN                                     OffsetY;
} LOGO_ENTRY;

// Global variables
EFI_HII_IMAGE_EX_PROTOCOL  *mHiiImageEx;
EFI_HII_HANDLE             mHiiHandle;
EFI_EVENT                  mKeyMonitorEvent;
BOOLEAN                    mLogoDisplayed = FALSE;

LOGO_ENTRY                 mLogos[] = {
  {
    IMAGE_TOKEN (IMG_LOGO),
    EdkiiPlatformLogoDisplayAttributeCenter,
    0,
    0
  }
};

**/

/**
  Monitor keyboard input in background.
  This function runs periodically to check for key presses.

  @param Event      The event that is signaled.
  @param Context    The context passed to the event.
**/


/**
VOID
EFIAPI
KeyboardMonitor (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
 // UINTN          EventIndex;
  
  // Check if key is available
  Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
  
  if (!EFI_ERROR (Status)) {
    // Display key press information
    Print (L"\nKey press detected: %c (Code: %d, Scan: %d)\n", 
           Key.UnicodeChar, Key.UnicodeChar, Key.ScanCode);
    
    // Check for F12 key (ScanCode = 0x16) or 's' key for shutdown
    if (Key.ScanCode == SCAN_F12 || Key.UnicodeChar == L's' || Key.UnicodeChar == L'S') {
      Print (L"\nShutdown key detected! Shutting down system...\n");
      
      // Small delay to show message
      gBS->Stall (2000000); // 2 seconds
      
      // Shutdown system
      gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
    }
  }
}


**/
/**
  Setup keyboard monitoring after logo is displayed.
**/

/**
VOID
SetupKeyboardMonitoring (
  VOID
  )
{
  EFI_STATUS  Status;
  
  if (mLogoDisplayed) {
    return; // Already setup
  }
  
  // Clear screen first
  gST->ConOut->ClearScreen (gST->ConOut);
  
  // Set cursor position and display message
  gST->ConOut->SetCursorPosition (gST->ConOut, 0, 5);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
  
  Print (L"=== UEFI Logo Display Driver ===\n\n");
  Print (L"Logo displayed successfully!\n\n");
  Print (L"Press F12 key or 's' key to shutdown system\n");
  Print (L"Press any other key to see key information\n\n");
  Print (L"Waiting for key press...\n");
  
  // Reset console input
  gST->ConIn->Reset (gST->ConIn, FALSE);
  
  // Create periodic timer event for keyboard monitoring
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  KeyboardMonitor,
                  NULL,
                  &mKeyMonitorEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    // Set timer to fire every 100ms
    Status = gBS->SetTimer (
                    mKeyMonitorEvent,
                    TimerPeriodic,
                    1000000  // 100ms in 100ns units
                    );
    
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set keyboard monitor timer: %r\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to create keyboard monitor event: %r\n", Status));
  }
  
  mLogoDisplayed = TRUE;
}

**/

/**
  Load a platform logo image and return its data and attributes.

  @param This              The pointer to this protocol instance.
  @param Instance          The visible image instance is found.
  @param Image             Points to the image.
  @param Attribute         The display attributes of the image returned.
  @param OffsetX           The X offset of the image regarding the Attribute.
  @param OffsetY           The Y offset of the image regarding the Attribute.

  @retval EFI_SUCCESS      The image was fetched successfully.
  @retval EFI_NOT_FOUND    The specified image could not be found.
**/
/**

EFI_STATUS
EFIAPI
GetImage (
  IN     EDKII_PLATFORM_LOGO_PROTOCOL        *This,
  IN OUT UINT32                              *Instance,
  OUT EFI_IMAGE_INPUT                        *Image,
  OUT EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE  *Attribute,
  OUT INTN                                   *OffsetX,
  OUT INTN                                   *OffsetY
  )
{
  UINT32      Current;
  EFI_STATUS  Status;

  if ((Instance == NULL) || (Image == NULL) ||
      (Attribute == NULL) || (OffsetX == NULL) || (OffsetY == NULL))
  {
    return EFI_INVALID_PARAMETER;
  }

  Current = *Instance;
  if (Current >= ARRAY_SIZE (mLogos)) {
    return EFI_NOT_FOUND;
  }

  (*Instance)++;
  *Attribute = mLogos[Current].Attribute;
  *OffsetX   = mLogos[Current].OffsetX;
  *OffsetY   = mLogos[Current].OffsetY;
  
  // Get image from HII database
  Status = mHiiImageEx->GetImageEx (mHiiImageEx, mHiiHandle, mLogos[Current].ImageId, Image);
  
  // After first logo is displayed, setup keyboard monitoring
  if (Current == 0 && !EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Logo displayed, setting up keyboard monitoring\n"));
    
    // Add a small delay to ensure logo is visible
    gBS->Stall (1000000); // 1 second
    
    // Setup keyboard monitoring
    SetupKeyboardMonitoring ();
  }
  
  return Status;
}

EDKII_PLATFORM_LOGO_PROTOCOL  mPlatformLogo = {
  GetImage
};

**/


/**
  Cleanup function called when driver is unloaded.

  @param ImageHandle    The handle of the image being unloaded.

  @retval EFI_SUCCESS   The image was unloaded successfully.
**/

/**

EFI_STATUS
EFIAPI
LogoDriverUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
  // Close keyboard monitoring event if it exists
  if (mKeyMonitorEvent != NULL) {
    gBS->CloseEvent (mKeyMonitorEvent);
    mKeyMonitorEvent = NULL;
  }
  
  return EFI_SUCCESS;
}
**/

/**
  Entrypoint of this module.

  This function is the entrypoint of this module. It installs the Edkii
  Platform Logo protocol and sets up keyboard monitoring.

  @param  ImageHandle       The firmware allocated handle for the EFI image.
  @param  SystemTable       A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.

**/

/**
EFI_STATUS
EFIAPI
InitializeLogo (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                   Status;
  EFI_HII_PACKAGE_LIST_HEADER  *PackageList;
  EFI_HII_DATABASE_PROTOCOL    *HiiDatabase;
  EFI_HANDLE                   Handle;

  DEBUG ((DEBUG_INFO, "Logo Driver: Initializing...\n"));

  // Locate HII Database Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiDatabaseProtocolGuid,
                  NULL,
                  (VOID **)&HiiDatabase
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Database Protocol: %r\n", Status));
    return Status;
  }

  // Locate HII Image Ex Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiImageExProtocolGuid,
                  NULL,
                  (VOID **)&mHiiImageEx
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Image Ex Protocol: %r\n", Status));
    return Status;
  }

  // Retrieve HII package list from ImageHandle
  Status = gBS->OpenProtocol (
                  ImageHandle,
                  &gEfiHiiPackageListProtocolGuid,
                  (VOID **)&PackageList,
                  ImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "HII Image Package with logo not found in PE/COFF resource section\n"));
    return Status;
  }

  // Publish HII package list to HII Database
  Status = HiiDatabase->NewPackageList (
                          HiiDatabase,
                          PackageList,
                          NULL,
                          &mHiiHandle
                          );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to add package list to HII Database: %r\n", Status));
    return Status;
  }

  // Install Platform Logo Protocol
  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEdkiiPlatformLogoProtocolGuid,
                  &mPlatformLogo,
                  NULL
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to install Platform Logo Protocol: %r\n", Status));
    // Cleanup HII handle
    HiiDatabase->RemovePackageList (HiiDatabase, mHiiHandle);
    return Status;
  }

  DEBUG ((DEBUG_INFO, "Logo Driver: Initialized successfully\n"));
  
  return EFI_SUCCESS;
}

**/




//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// after logo press F12 key shows but showing blk related details.


/** @file
  Logo DXE Driver with F12 key shutdown functionality.
  Shows logo for 5 seconds, then message for 15 seconds with F12 detection.

Copyright (c) 2016 - 2017, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/
/**

#include <Uefi.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/HiiImageEx.h>
#include <Protocol/PlatformLogo.h>
#include <Protocol/HiiPackageList.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/SimpleTextOut.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>

typedef struct {
  EFI_IMAGE_ID                             ImageId;
  EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE    Attribute;
  INTN                                     OffsetX;
  INTN                                     OffsetY;
} LOGO_ENTRY;

// Global variables
EFI_HII_IMAGE_EX_PROTOCOL  *mHiiImageEx;
EFI_HII_HANDLE             mHiiHandle;
EFI_EVENT                  mKeyMonitorEvent;
EFI_EVENT                  mTimerEvent;
BOOLEAN                    mLogoDisplayed = FALSE;
BOOLEAN                    mMessageShown = FALSE;
UINTN                      mTimerCounter = 0;

LOGO_ENTRY                 mLogos[] = {
  {
    IMAGE_TOKEN (IMG_LOGO),
    EdkiiPlatformLogoDisplayAttributeCenter,
    0,
    0
  }
};

**/

/**
  Monitor keyboard input for F12 key press.

  @param Event      The event that is signaled.
  @param Context    The context passed to the event.
**/

/**
VOID
EFIAPI
KeyboardMonitor (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  
  // Only monitor after message is shown
  if (!mMessageShown) {
    return;
  }
  
  // Check if key is available
  Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
  
  if (!EFI_ERROR (Status)) {
    // Check for F12 key only
    if (Key.ScanCode == SCAN_F12) {
      DEBUG ((DEBUG_INFO, "F12 key pressed - shutting down\n"));
      
      // Clear screen and show shutdown message
      gST->ConOut->ClearScreen (gST->ConOut);
      gST->ConOut->SetCursorPosition (gST->ConOut, 30, 12);
      Print (L"System Shutting Down...");
      
      // Small delay
      gBS->Stall (1000000); // 1 second
      
      // Shutdown system
      gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
    }
  }
}


**/
/**
  Timer callback function to handle logo display timing and message.

  @param Event      The event that is signaled.
  @param Context    The context passed to the event.
**/

/**
VOID
EFIAPI
TimerCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  mTimerCounter++;
  
  // After 5 seconds, show the message below logo
  if (mTimerCounter == 50 && !mMessageShown) { // 50 * 100ms = 5 seconds
    // Position cursor below the logo area (assuming logo is centered)
    gST->ConOut->SetCursorPosition (gST->ConOut, 28, 18);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
    Print (L"Press F12 key");
    
    mMessageShown = TRUE;
    
    // Reset console input for clean key detection
    gST->ConIn->Reset (gST->ConIn, FALSE);
    
    DEBUG ((DEBUG_INFO, "Message displayed after 5 seconds\n"));
  }
  
  // After 20 seconds total (5 sec logo + 15 sec message), proceed to shell
  if (mTimerCounter >= 200) { // 200 * 100ms = 20 seconds
    DEBUG ((DEBUG_INFO, "Timer expired, proceeding to shell\n"));
    
    // Cancel timer events
    gBS->SetTimer (mTimerEvent, TimerCancel, 0);
    gBS->SetTimer (mKeyMonitorEvent, TimerCancel, 0);
    
    // Clear the message
    gST->ConOut->SetCursorPosition (gST->ConOut, 28, 18);
    Print (L"             "); // Clear the message
    
    // Let the system proceed to shell naturally
    return;
  }
}

**/

/**
  Setup timing and keyboard monitoring after logo is displayed.
**/

/**
VOID
SetupLogoTiming (
  VOID
  )
{
  EFI_STATUS  Status;
  
  if (mLogoDisplayed) {
    return; // Already setup
  }
  
  DEBUG ((DEBUG_INFO, "Setting up logo timing and keyboard monitoring\n"));
  
  // Create timer event for logo timing
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  TimerCallback,
                  NULL,
                  &mTimerEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    // Set timer to fire every 100ms
    Status = gBS->SetTimer (
                    mTimerEvent,
                    TimerPeriodic,
                    1000000  // 100ms in 100ns units
                    );
  }
  
  // Create keyboard monitoring event
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  KeyboardMonitor,
                  NULL,
                  &mKeyMonitorEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    // Set keyboard timer to fire every 50ms for responsive key detection
    Status = gBS->SetTimer (
                    mKeyMonitorEvent,
                    TimerPeriodic,
                    500000  // 50ms in 100ns units
                    );
  }
  
  mLogoDisplayed = TRUE;
}

**/

/**
  Load a platform logo image and return its data and attributes.

  @param This              The pointer to this protocol instance.
  @param Instance          The visible image instance is found.
  @param Image             Points to the image.
  @param Attribute         The display attributes of the image returned.
  @param OffsetX           The X offset of the image regarding the Attribute.
  @param OffsetY           The Y offset of the image regarding the Attribute.

  @retval EFI_SUCCESS      The image was fetched successfully.
  @retval EFI_NOT_FOUND    The specified image could not be found.
**/

/**
EFI_STATUS
EFIAPI
GetImage (
  IN     EDKII_PLATFORM_LOGO_PROTOCOL        *This,
  IN OUT UINT32                              *Instance,
  OUT EFI_IMAGE_INPUT                        *Image,
  OUT EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE  *Attribute,
  OUT INTN                                   *OffsetX,
  OUT INTN                                   *OffsetY
  )
{
  UINT32      Current;
  EFI_STATUS  Status;

  if ((Instance == NULL) || (Image == NULL) ||
      (Attribute == NULL) || (OffsetX == NULL) || (OffsetY == NULL))
  {
    return EFI_INVALID_PARAMETER;
  }

  Current = *Instance;
  if (Current >= ARRAY_SIZE (mLogos)) {
    return EFI_NOT_FOUND;
  }

  (*Instance)++;
  *Attribute = mLogos[Current].Attribute;
  *OffsetX   = mLogos[Current].OffsetX;
  *OffsetY   = mLogos[Current].OffsetY;
  
  // Get image from HII database
  Status = mHiiImageEx->GetImageEx (mHiiImageEx, mHiiHandle, mLogos[Current].ImageId, Image);
  
  // After first logo is displayed, setup timing
  if (Current == 0 && !EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Logo displayed, setting up timing\n"));
    SetupLogoTiming ();
  }
  
  return Status;
}

EDKII_PLATFORM_LOGO_PROTOCOL  mPlatformLogo = {
  GetImage
};


**/
/**
  Cleanup function called when driver is unloaded.

  @param ImageHandle    The handle of the image being unloaded.

  @retval EFI_SUCCESS   The image was unloaded successfully.
**/

/**
EFI_STATUS
EFIAPI
LogoDriverUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
  // Close events if they exist
  if (mTimerEvent != NULL) {
    gBS->CloseEvent (mTimerEvent);
    mTimerEvent = NULL;
  }
  
  if (mKeyMonitorEvent != NULL) {
    gBS->CloseEvent (mKeyMonitorEvent);
    mKeyMonitorEvent = NULL;
  }
  
  return EFI_SUCCESS;
}

**/

/**
  Entrypoint of this module.

  This function is the entrypoint of this module. It installs the Edkii
  Platform Logo protocol.

  @param  ImageHandle       The firmware allocated handle for the EFI image.
  @param  SystemTable       A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.

**/

/**

EFI_STATUS
EFIAPI
InitializeLogo (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                   Status;
  EFI_HII_PACKAGE_LIST_HEADER  *PackageList;
  EFI_HII_DATABASE_PROTOCOL    *HiiDatabase;
  EFI_HANDLE                   Handle;

  DEBUG ((DEBUG_INFO, "Logo Driver: Initializing...\n"));

  // Locate HII Database Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiDatabaseProtocolGuid,
                  NULL,
                  (VOID **)&HiiDatabase
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Database Protocol: %r\n", Status));
    return Status;
  }

  // Locate HII Image Ex Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiImageExProtocolGuid,
                  NULL,
                  (VOID **)&mHiiImageEx
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Image Ex Protocol: %r\n", Status));
    return Status;
  }

  // Retrieve HII package list from ImageHandle
  Status = gBS->OpenProtocol (
                  ImageHandle,
                  &gEfiHiiPackageListProtocolGuid,
                  (VOID **)&PackageList,
                  ImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "HII Image Package with logo not found in PE/COFF resource section\n"));
    return Status;
  }

  // Publish HII package list to HII Database
  Status = HiiDatabase->NewPackageList (
                          HiiDatabase,
                          PackageList,
                          NULL,
                          &mHiiHandle
                          );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to add package list to HII Database: %r\n", Status));
    return Status;
  }

  // Install Platform Logo Protocol
  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEdkiiPlatformLogoProtocolGuid,
                  &mPlatformLogo,
                  NULL
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to install Platform Logo Protocol: %r\n", Status));
    // Cleanup HII handle
    HiiDatabase->RemovePackageList (HiiDatabase, mHiiHandle);
    return Status;
  }

  DEBUG ((DEBUG_INFO, "Logo Driver: Initialized successfully\n"));
  
  return EFI_SUCCESS;
}


**/




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




/** @file
  Logo DXE Driver with F12 key shutdown functionality.
  Shows logo for 5 seconds, then message for 15 seconds with F12 detection.

Copyright (c) 2016 - 2017, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

/**
#include <Uefi.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/HiiImageEx.h>
#include <Protocol/PlatformLogo.h>
#include <Protocol/HiiPackageList.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/SimpleTextOut.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>

typedef struct {
  EFI_IMAGE_ID                             ImageId;
  EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE    Attribute;
  INTN                                     OffsetX;
  INTN                                     OffsetY;
} LOGO_ENTRY;

// Global variables
EFI_HII_IMAGE_EX_PROTOCOL  *mHiiImageEx;
EFI_HII_HANDLE             mHiiHandle;
EFI_EVENT                  mKeyMonitorEvent;
EFI_EVENT                  mTimerEvent;
BOOLEAN                    mLogoDisplayed = FALSE;
BOOLEAN                    mMessageShown = FALSE;
UINTN                      mTimerCounter = 0;

LOGO_ENTRY                 mLogos[] = {
  {
    IMAGE_TOKEN (IMG_LOGO),
    EdkiiPlatformLogoDisplayAttributeCenter,
    0,
    0
  }
};

**/



/**
  Monitor keyboard input for F12 key press.

  @param Event      The event that is signaled.
  @param Context    The context passed to the event.
**/
/**

VOID
EFIAPI
KeyboardMonitor (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  
  // Only monitor after message is shown
  if (!mMessageShown) {
    return;
  }
  
  // Check if key is available
  Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
  
  if (!EFI_ERROR (Status)) {
    // Check for F12 key only
    if (Key.ScanCode == SCAN_F12) {
      DEBUG ((DEBUG_INFO, "F12 key pressed - shutting down\n"));
      
      // Clear screen and show shutdown message
      gST->ConOut->ClearScreen (gST->ConOut);
      gST->ConOut->SetCursorPosition (gST->ConOut, 30, 12);
      Print (L"System Shutting Down...");
      
      // Small delay
      gBS->Stall (1000000); // 1 second
      
      // Shutdown system
      gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
    }
  }
}

**/


/**
  Timer callback function to handle logo display timing and message.

  @param Event      The event that is signaled.
  @param Context    The context passed to the event.
**/

/**
VOID
EFIAPI
TimerCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  mTimerCounter++;
  
  // After 5 seconds, clear screen and show only the message below logo
  if (mTimerCounter == 50 && !mMessageShown) { // 50 * 100ms = 5 seconds
    // Clear the entire screen first
    gST->ConOut->ClearScreen (gST->ConOut);
    
    // Position cursor below the logo area (assuming logo is centered)
    gST->ConOut->SetCursorPosition (gST->ConOut, 28, 18);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
    Print (L"Press F12");
    
    mMessageShown = TRUE;
    
    // Reset console input for clean key detection
    gST->ConIn->Reset (gST->ConIn, FALSE);
    
    DEBUG ((DEBUG_INFO, "Message displayed after 5 seconds\n"));
  }
  
  // After 20 seconds total (5 sec logo + 15 sec message), proceed to shell
  if (mTimerCounter >= 200) { // 200 * 100ms = 20 seconds
    DEBUG ((DEBUG_INFO, "Timer expired, proceeding to shell\n"));
    
    // Cancel timer events
    gBS->SetTimer (mTimerEvent, TimerCancel, 0);
    gBS->SetTimer (mKeyMonitorEvent, TimerCancel, 0);
    
    // Clear the entire screen before proceeding to shell
    gST->ConOut->ClearScreen (gST->ConOut);
    
    // Let the system proceed to shell naturally
    return;
  }
}

**/

/**
  Setup timing and keyboard monitoring after logo is displayed.
**/

/**
VOID
SetupLogoTiming (
  VOID
  )
{
  EFI_STATUS  Status;
  
  if (mLogoDisplayed) {
    return; // Already setup
  }
  
  DEBUG ((DEBUG_INFO, "Setting up logo timing and keyboard monitoring\n"));
  
  // Create timer event for logo timing
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  TimerCallback,
                  NULL,
                  &mTimerEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    // Set timer to fire every 100ms
    Status = gBS->SetTimer (
                    mTimerEvent,
                    TimerPeriodic,
                    1000000  // 100ms in 100ns units
                    );
  }
  
  // Create keyboard monitoring event
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  KeyboardMonitor,
                  NULL,
                  &mKeyMonitorEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    // Set keyboard timer to fire every 50ms for responsive key detection
    Status = gBS->SetTimer (
                    mKeyMonitorEvent,
                    TimerPeriodic,
                    500000  // 50ms in 100ns units
                    );
  }
  
  mLogoDisplayed = TRUE;
}

**/

/**
  Load a platform logo image and return its data and attributes.

  @param This              The pointer to this protocol instance.
  @param Instance          The visible image instance is found.
  @param Image             Points to the image.
  @param Attribute         The display attributes of the image returned.
  @param OffsetX           The X offset of the image regarding the Attribute.
  @param OffsetY           The Y offset of the image regarding the Attribute.

  @retval EFI_SUCCESS      The image was fetched successfully.
  @retval EFI_NOT_FOUND    The specified image could not be found.
**/

/**
EFI_STATUS
EFIAPI
GetImage (
  IN     EDKII_PLATFORM_LOGO_PROTOCOL        *This,
  IN OUT UINT32                              *Instance,
  OUT EFI_IMAGE_INPUT                        *Image,
  OUT EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE  *Attribute,
  OUT INTN                                   *OffsetX,
  OUT INTN                                   *OffsetY
  )
{
  UINT32      Current;
  EFI_STATUS  Status;

  if ((Instance == NULL) || (Image == NULL) ||
      (Attribute == NULL) || (OffsetX == NULL) || (OffsetY == NULL))
  {
    return EFI_INVALID_PARAMETER;
  }

  Current = *Instance;
  if (Current >= ARRAY_SIZE (mLogos)) {
    return EFI_NOT_FOUND;
  }

  (*Instance)++;
  *Attribute = mLogos[Current].Attribute;
  *OffsetX   = mLogos[Current].OffsetX;
  *OffsetY   = mLogos[Current].OffsetY;
  
  // Get image from HII database
  Status = mHiiImageEx->GetImageEx (mHiiImageEx, mHiiHandle, mLogos[Current].ImageId, Image);
  
  // After first logo is displayed, setup timing
  if (Current == 0 && !EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Logo displayed, setting up timing\n"));
    SetupLogoTiming ();
  }
  
  return Status;
}

EDKII_PLATFORM_LOGO_PROTOCOL  mPlatformLogo = {
  GetImage
};

**/

/**
  Cleanup function called when driver is unloaded.

  @param ImageHandle    The handle of the image being unloaded.

  @retval EFI_SUCCESS   The image was unloaded successfully.
**/

/**
EFI_STATUS
EFIAPI
LogoDriverUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
  // Close events if they exist
  if (mTimerEvent != NULL) {
    gBS->CloseEvent (mTimerEvent);
    mTimerEvent = NULL;
  }
  
  if (mKeyMonitorEvent != NULL) {
    gBS->CloseEvent (mKeyMonitorEvent);
    mKeyMonitorEvent = NULL;
  }
  
  return EFI_SUCCESS;
}

**/


/**
  Entrypoint of this module.

  This function is the entrypoint of this module. It installs the Edkii
  Platform Logo protocol.

  @param  ImageHandle       The firmware allocated handle for the EFI image.
  @param  SystemTable       A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.

**/

/**
EFI_STATUS
EFIAPI
InitializeLogo (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                   Status;
  EFI_HII_PACKAGE_LIST_HEADER  *PackageList;
  EFI_HII_DATABASE_PROTOCOL    *HiiDatabase;
  EFI_HANDLE                   Handle;

  DEBUG ((DEBUG_INFO, "Logo Driver: Initializing...\n"));

  // Locate HII Database Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiDatabaseProtocolGuid,
                  NULL,
                  (VOID **)&HiiDatabase
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Database Protocol: %r\n", Status));
    return Status;
  }

  // Locate HII Image Ex Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiImageExProtocolGuid,
                  NULL,
                  (VOID **)&mHiiImageEx
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Image Ex Protocol: %r\n", Status));
    return Status;
  }

  // Retrieve HII package list from ImageHandle
  Status = gBS->OpenProtocol (
                  ImageHandle,
                  &gEfiHiiPackageListProtocolGuid,
                  (VOID **)&PackageList,
                  ImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "HII Image Package with logo not found in PE/COFF resource section\n"));
    return Status;
  }

  // Publish HII package list to HII Database
  Status = HiiDatabase->NewPackageList (
                          HiiDatabase,
                          PackageList,
                          NULL,
                          &mHiiHandle
                          );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to add package list to HII Database: %r\n", Status));
    return Status;
  }

  // Install Platform Logo Protocol
  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEdkiiPlatformLogoProtocolGuid,
                  &mPlatformLogo,
                  NULL
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to install Platform Logo Protocol: %r\n", Status));
    // Cleanup HII handle
    HiiDatabase->RemovePackageList (HiiDatabase, mHiiHandle);
    return Status;
  }

  DEBUG ((DEBUG_INFO, "Logo Driver: Initialized successfully\n"));
  
  return EFI_SUCCESS;
}

**/





///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



/** @file
  Logo DXE Driver with F12 key shutdown functionality.
  Shows logo for 5 seconds, then message for 15 seconds with F12 detection.

Copyright (c) 2016 - 2017, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

/**
#include <Uefi.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/HiiImageEx.h>
#include <Protocol/PlatformLogo.h>
#include <Protocol/HiiPackageList.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/SimpleTextOut.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>

typedef struct {
  EFI_IMAGE_ID                             ImageId;
  EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE    Attribute;
  INTN                                     OffsetX;
  INTN                                     OffsetY;
} LOGO_ENTRY;

// Global variables
EFI_HII_IMAGE_EX_PROTOCOL  *mHiiImageEx;
EFI_HII_HANDLE             mHiiHandle;
EFI_EVENT                  mKeyMonitorEvent;
EFI_EVENT                  mTimerEvent;
BOOLEAN                    mLogoDisplayed = FALSE;
BOOLEAN                    mMessageShown = FALSE;
UINTN                      mTimerCounter = 0;

// Function declarations
VOID
DisplayBiosSetupForm (
  VOID
  );

LOGO_ENTRY                 mLogos[] = {
  {
    IMAGE_TOKEN (IMG_LOGO),
    EdkiiPlatformLogoDisplayAttributeCenter,
    0,
    0
  }
};

**/

/**
  Monitor keyboard input for F12 key press.

  @param Event      The event that is signaled.
  @param Context    The context passed to the event.
**/

/**
VOID
EFIAPI
KeyboardMonitor (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  
  // Only monitor after message is shown
  if (!mMessageShown) {
    return;
  }
  
  // Check if key is available
  Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
  
  if (!EFI_ERROR (Status)) {
    // Check for F12 key only
    if (Key.ScanCode == SCAN_F12) {
      DEBUG ((DEBUG_INFO, "F12 key pressed - showing BIOS setup form\n"));
      
      // Cancel timer events to stop normal flow
      gBS->SetTimer (mTimerEvent, TimerCancel, 0);
      gBS->SetTimer (mKeyMonitorEvent, TimerCancel, 0);
      
      // Display BIOS setup form
      DisplayBiosSetupForm ();
      
      // Stay in this form - don't proceed to shell
      // The system will remain in this state
      return;
    }
  }
}

**/

/**
  Timer callback function to handle logo display timing and message.

  @param Event      The event that is signaled.
  @param Context    The context passed to the event.
**/

/**
VOID
EFIAPI
TimerCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  mTimerCounter++;
  
  // After 15 seconds, clear screen and show only the message below logo
  if (mTimerCounter == 15 && !mMessageShown) { // 15 * 100ms = 15 seconds
    // Clear the entire screen first
    gST->ConOut->ClearScreen (gST->ConOut);
    
    // Position cursor below the logo area (assuming logo is centered)
    gST->ConOut->SetCursorPosition (gST->ConOut, 20, 18);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
    Print (L"Press key F12 for BIOS setup");
    
    mMessageShown = TRUE;
    
    // Reset console input for clean key detection
    gST->ConIn->Reset (gST->ConIn, FALSE);
    
    DEBUG ((DEBUG_INFO, "Message displayed after 15 seconds\n"));
  }
  
  // After 30 seconds total (15 sec logo + 15 sec message), proceed to shell
  if (mTimerCounter >= 300) { // 300 * 100ms = 30 seconds
    DEBUG ((DEBUG_INFO, "Timer expired, proceeding to shell\n"));
    
    // Cancel timer events
    gBS->SetTimer (mTimerEvent, TimerCancel, 0);
    gBS->SetTimer (mKeyMonitorEvent, TimerCancel, 0);
    
    // Clear the entire screen before proceeding to shell
    gST->ConOut->ClearScreen (gST->ConOut);
    
    // Let the system proceed to shell naturally
    return;
  }
}

**/

/**
  Display BIOS setup form with white background.
**/

/**
VOID
DisplayBiosSetupForm (
  VOID
  )
{
  UINTN  Index;
  
  // Clear screen
  gST->ConOut->ClearScreen (gST->ConOut);
  
  // Set white background with black text
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  
  // Fill entire screen with white background
  for (Index = 0; Index < 25; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Index);
    Print (L"                                                                                ");
  }
  
  // Display centered message
   gST->ConOut->SetCursorPosition (gST->ConOut, 35, 9);
  Print (L"CDAC Bangalore");
  gST->ConOut->SetCursorPosition (gST->ConOut, 35, 12);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"Hello Anand");
  
  // Add some decoration
  gST->ConOut->SetCursorPosition (gST->ConOut, 25, 10);
  Print (L"================================");
  gST->ConOut->SetCursorPosition (gST->ConOut, 30, 11);
  Print (L"BIOS SETUP UTILITY");
  gST->ConOut->SetCursorPosition (gST->ConOut, 25, 13);
  Print (L"================================");
  
  // Show some options
  // gST->ConOut->SetCursorPosition (gST->ConOut, 25, 15);
  // Print (L"Press ESC to return to boot menu");
  // gST->ConOut->SetCursorPosition (gST->ConOut, 25, 16);
  // Print (L"Press F10 to save and exit");
}
**/

/**
  Setup timing and keyboard monitoring after logo is displayed.
**/

/**
VOID
SetupLogoTiming (
  VOID
  )
{
  EFI_STATUS  Status;
  
  if (mLogoDisplayed) {
    return; // Already setup
  }
  
  DEBUG ((DEBUG_INFO, "Setting up logo timing and keyboard monitoring\n"));
  
  // Create timer event for logo timing
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  TimerCallback,
                  NULL,
                  &mTimerEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    // Set timer to fire every 100ms
    Status = gBS->SetTimer (
                    mTimerEvent,
                    TimerPeriodic,
                    1000000  // 100ms in 100ns units
                    );
  }
  
  // Create keyboard monitoring event
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  KeyboardMonitor,
                  NULL,
                  &mKeyMonitorEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    // Set keyboard timer to fire every 50ms for responsive key detection
    Status = gBS->SetTimer (
                    mKeyMonitorEvent,
                    TimerPeriodic,
                    500000  // 50ms in 100ns units
                    );
  }
  
  mLogoDisplayed = TRUE;
}

**/

/**
  Load a platform logo image and return its data and attributes.

  @param This              The pointer to this protocol instance.
  @param Instance          The visible image instance is found.
  @param Image             Points to the image.
  @param Attribute         The display attributes of the image returned.
  @param OffsetX           The X offset of the image regarding the Attribute.
  @param OffsetY           The Y offset of the image regarding the Attribute.

  @retval EFI_SUCCESS      The image was fetched successfully.
  @retval EFI_NOT_FOUND    The specified image could not be found.
**/

/**

EFI_STATUS
EFIAPI
GetImage (
  IN     EDKII_PLATFORM_LOGO_PROTOCOL        *This,
  IN OUT UINT32                              *Instance,
  OUT EFI_IMAGE_INPUT                        *Image,
  OUT EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE  *Attribute,
  OUT INTN                                   *OffsetX,
  OUT INTN                                   *OffsetY
  )
{
  UINT32      Current;
  EFI_STATUS  Status;

  if ((Instance == NULL) || (Image == NULL) ||
      (Attribute == NULL) || (OffsetX == NULL) || (OffsetY == NULL))
  {
    return EFI_INVALID_PARAMETER;
  }

  Current = *Instance;
  if (Current >= ARRAY_SIZE (mLogos)) {
    return EFI_NOT_FOUND;
  }

  (*Instance)++;
  *Attribute = mLogos[Current].Attribute;
  *OffsetX   = mLogos[Current].OffsetX;
  *OffsetY   = mLogos[Current].OffsetY;
  
  // Get image from HII database
  Status = mHiiImageEx->GetImageEx (mHiiImageEx, mHiiHandle, mLogos[Current].ImageId, Image);
  
  // After first logo is displayed, setup timing
  if (Current == 0 && !EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Logo displayed, setting up timing\n"));
    SetupLogoTiming ();
  }
  
  return Status;
}

EDKII_PLATFORM_LOGO_PROTOCOL  mPlatformLogo = {
  GetImage
};

**/

/**
  Cleanup function called when driver is unloaded.

  @param ImageHandle    The handle of the image being unloaded.

  @retval EFI_SUCCESS   The image was unloaded successfully.
**/

/**
EFI_STATUS
EFIAPI
LogoDriverUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
  // Close events if they exist
  if (mTimerEvent != NULL) {
    gBS->CloseEvent (mTimerEvent);
    mTimerEvent = NULL;
  }
  
  if (mKeyMonitorEvent != NULL) {
    gBS->CloseEvent (mKeyMonitorEvent);
    mKeyMonitorEvent = NULL;
  }
  
  return EFI_SUCCESS;
}

**/

/**
  Entrypoint of this module.

  This function is the entrypoint of this module. It installs the Edkii
  Platform Logo protocol.

  @param  ImageHandle       The firmware allocated handle for the EFI image.
  @param  SystemTable       A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.

**/

/**
EFI_STATUS
EFIAPI
InitializeLogo (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                   Status;
  EFI_HII_PACKAGE_LIST_HEADER  *PackageList;
  EFI_HII_DATABASE_PROTOCOL    *HiiDatabase;
  EFI_HANDLE                   Handle;

  DEBUG ((DEBUG_INFO, "Logo Driver: Initializing...\n"));

  // Locate HII Database Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiDatabaseProtocolGuid,
                  NULL,
                  (VOID **)&HiiDatabase
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Database Protocol: %r\n", Status));
    return Status;
  }

  // Locate HII Image Ex Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiImageExProtocolGuid,
                  NULL,
                  (VOID **)&mHiiImageEx
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Image Ex Protocol: %r\n", Status));
    return Status;
  }

  // Retrieve HII package list from ImageHandle
  Status = gBS->OpenProtocol (
                  ImageHandle,
                  &gEfiHiiPackageListProtocolGuid,
                  (VOID **)&PackageList,
                  ImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "HII Image Package with logo not found in PE/COFF resource section\n"));
    return Status;
  }

  // Publish HII package list to HII Database
  Status = HiiDatabase->NewPackageList (
                          HiiDatabase,
                          PackageList,
                          NULL,
                          &mHiiHandle
                          );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to add package list to HII Database: %r\n", Status));
    return Status;
  }

  // Install Platform Logo Protocol
  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEdkiiPlatformLogoProtocolGuid,
                  &mPlatformLogo,
                  NULL
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to install Platform Logo Protocol: %r\n", Status));
    // Cleanup HII handle
    HiiDatabase->RemovePackageList (HiiDatabase, mHiiHandle);
    return Status;
  }

  DEBUG ((DEBUG_INFO, "Logo Driver: Initialized successfully\n"));
  
  return EFI_SUCCESS;
}


**/


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// press F12 new hii form open and cdac bangalore and time(not correct) shows.


/** @file
  Logo DXE Driver with F12 key BIOS setup functionality.
  Shows logo, waits for F12 key, then displays BIOS setup form.

Copyright (c) 2016 - 2017, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

/**

#include <Uefi.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/HiiImageEx.h>
#include <Protocol/PlatformLogo.h>
#include <Protocol/HiiPackageList.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/SimpleTextOut.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>

typedef struct {
  EFI_IMAGE_ID                             ImageId;
  EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE    Attribute;
  INTN                                     OffsetX;
  INTN                                     OffsetY;
} LOGO_ENTRY;

// Global variables
EFI_HII_IMAGE_EX_PROTOCOL  *mHiiImageEx;
EFI_HII_HANDLE             mHiiHandle;
EFI_EVENT                  mKeyMonitorEvent;
EFI_EVENT                  mTimerEvent;
BOOLEAN                    mLogoDisplayed = FALSE;
BOOLEAN                    mMessageShown = FALSE;
UINTN                      mTimerCounter = 0;

// Function declarations
VOID
DisplayBiosSetupForm (
  VOID
  );

VOID
GetCurrentTime (
  OUT CHAR16  *TimeString
  );

LOGO_ENTRY                 mLogos[] = {
  {
    IMAGE_TOKEN (IMG_LOGO),
    EdkiiPlatformLogoDisplayAttributeCenter,
    0,
    0
  }
};

**/

/**
  Get current time and format it as string.

  @param TimeString   Output buffer for formatted time string.
**/

/**

VOID
GetCurrentTime (
  OUT CHAR16  *TimeString
  )
{
  EFI_TIME    Time;
  EFI_STATUS  Status;
  
  Status = gRT->GetTime (&Time, NULL);
  if (!EFI_ERROR (Status)) {
    UnicodeSPrint (
      TimeString, 
      50 * sizeof(CHAR16),
      L"%02d:%02d:%02d",
      Time.Hour,
      Time.Minute,
      Time.Second
    );
  } else {
    UnicodeSPrint (TimeString, 50 * sizeof(CHAR16), L"--:--:--");
  }
}

**/

/**
  Display BIOS setup form with white background and current time.
**/

/**

VOID
DisplayBiosSetupForm (
  VOID
  )
{
  UINTN   Index;
  CHAR16  TimeString[50];
  
  // Clear screen
  gST->ConOut->ClearScreen (gST->ConOut);
  
  // Set white background with black text
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  
  // Fill entire screen with white background
  for (Index = 0; Index < 25; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Index);
    Print (L"                                                                                ");
  }
  
  // Display "CDAC Bangalore" at top center
  gST->ConOut->SetCursorPosition (gST->ConOut, 30, 2);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"CDAC Bangalore");
  
  // Get and display current time at top right corner
  GetCurrentTime (TimeString);
  gST->ConOut->SetCursorPosition (gST->ConOut, 65, 2);
  Print (L"%s", TimeString);
  
  // Add main content area
  gST->ConOut->SetCursorPosition (gST->ConOut, 25, 8);
  Print (L"====================================");
  gST->ConOut->SetCursorPosition (gST->ConOut, 30, 9);
  Print (L"BIOS SETUP UTILITY");
  gST->ConOut->SetCursorPosition (gST->ConOut, 25, 10);
  Print (L"====================================");
  
  // Display main message
  gST->ConOut->SetCursorPosition (gST->ConOut, 32, 12);
  Print (L"Welcome to BIOS Setup");
  
  // Show navigation options
  gST->ConOut->SetCursorPosition (gST->ConOut, 25, 15);
  Print (L"Use arrow keys to navigate");
  gST->ConOut->SetCursorPosition (gST->ConOut, 25, 16);
  Print (L"Press ESC to exit");
  gST->ConOut->SetCursorPosition (gST->ConOut, 25, 17);
  Print (L"Press F10 to save and exit");
  
  // Add footer
  gST->ConOut->SetCursorPosition (gST->ConOut, 20, 22);
  Print (L"BIOS Setup - F12 Key Activated - CDAC Bangalore");
}

**/

/**
  Monitor keyboard input for F12 key press only.

  @param Event      The event that is signaled.
  @param Context    The context passed to the event.
**/

/**

VOID
EFIAPI
KeyboardMonitor (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  
  // Only monitor after message is shown
  if (!mMessageShown) {
    return;
  }
  
  // Check if key is available
  Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
  
  if (!EFI_ERROR (Status)) {
    // Check for F12 key only
    if (Key.ScanCode == SCAN_F12) {
      DEBUG ((DEBUG_INFO, "F12 key pressed - showing BIOS setup form\n"));
      
      // Cancel timer events to stop normal flow
      gBS->SetTimer (mKeyMonitorEvent, TimerCancel, 0);
      
      // Display BIOS setup form
      DisplayBiosSetupForm ();
      
      // Stay in this form indefinitely - no shell, no exit
      // System will remain in BIOS setup mode
      return;
    }
  }
}

**/

/**
  Timer callback function to handle logo display timing.

  @param Event      The event that is signaled.
  @param Context    The context passed to the event.
**/

/**
VOID
EFIAPI
TimerCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  mTimerCounter++;
  
  // After 5 seconds, show the message below logo and wait for F12
  if (mTimerCounter == 50 && !mMessageShown) { // 50 * 100ms = 5 seconds
    // Clear the entire screen first
    gST->ConOut->ClearScreen (gST->ConOut);
    
    // Position cursor below the logo area
    gST->ConOut->SetCursorPosition (gST->ConOut, 18, 18);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
    Print (L"Press F12 key for BIOS setup");
    
    mMessageShown = TRUE;
    
    // Reset console input for clean key detection
    gST->ConIn->Reset (gST->ConIn, FALSE);
    
    DEBUG ((DEBUG_INFO, "Message displayed, waiting for F12 key\n"));
  }
  
  // No automatic shell redirect - wait indefinitely for F12
  // Remove any timer expiry logic that goes to shell
}

**/

/**
  Setup timing and keyboard monitoring after logo is displayed.
**/

/**
VOID
SetupLogoTiming (
  VOID
  )
{
  EFI_STATUS  Status;
  
  if (mLogoDisplayed) {
    return; // Already setup
  }
  
  DEBUG ((DEBUG_INFO, "Setting up logo timing and keyboard monitoring\n"));
  
  // Create timer event for logo timing
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  TimerCallback,
                  NULL,
                  &mTimerEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    // Set timer to fire every 100ms
    Status = gBS->SetTimer (
                    mTimerEvent,
                    TimerPeriodic,
                    1000000  // 100ms in 100ns units
                    );
  }
  
  // Create keyboard monitoring event
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  KeyboardMonitor,
                  NULL,
                  &mKeyMonitorEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    // Set keyboard timer to fire every 50ms for responsive key detection
    Status = gBS->SetTimer (
                    mKeyMonitorEvent,
                    TimerPeriodic,
                    500000  // 50ms in 100ns units
                    );
  }
  
  mLogoDisplayed = TRUE;
}

**/

/**
  Load a platform logo image and return its data and attributes.

  @param This              The pointer to this protocol instance.
  @param Instance          The visible image instance is found.
  @param Image             Points to the image.
  @param Attribute         The display attributes of the image returned.
  @param OffsetX           The X offset of the image regarding the Attribute.
  @param OffsetY           The Y offset of the image regarding the Attribute.

  @retval EFI_SUCCESS      The image was fetched successfully.
  @retval EFI_NOT_FOUND    The specified image could not be found.
**/

/**

EFI_STATUS
EFIAPI
GetImage (
  IN     EDKII_PLATFORM_LOGO_PROTOCOL        *This,
  IN OUT UINT32                              *Instance,
  OUT EFI_IMAGE_INPUT                        *Image,
  OUT EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE  *Attribute,
  OUT INTN                                   *OffsetX,
  OUT INTN                                   *OffsetY
  )
{
  UINT32      Current;
  EFI_STATUS  Status;

  if ((Instance == NULL) || (Image == NULL) ||
      (Attribute == NULL) || (OffsetX == NULL) || (OffsetY == NULL))
  {
    return EFI_INVALID_PARAMETER;
  }

  Current = *Instance;
  if (Current >= ARRAY_SIZE (mLogos)) {
    return EFI_NOT_FOUND;
  }

  (*Instance)++;
  *Attribute = mLogos[Current].Attribute;
  *OffsetX   = mLogos[Current].OffsetX;
  *OffsetY   = mLogos[Current].OffsetY;
  
  // Get image from HII database
  Status = mHiiImageEx->GetImageEx (mHiiImageEx, mHiiHandle, mLogos[Current].ImageId, Image);
  
  // After first logo is displayed, setup timing
  if (Current == 0 && !EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Logo displayed, setting up timing\n"));
    SetupLogoTiming ();
  }
  
  return Status;
}

EDKII_PLATFORM_LOGO_PROTOCOL  mPlatformLogo = {
  GetImage
};

**/

/**
  Cleanup function called when driver is unloaded.

  @param ImageHandle    The handle of the image being unloaded.

  @retval EFI_SUCCESS   The image was unloaded successfully.
**/

/**
EFI_STATUS
EFIAPI
LogoDriverUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
  // Close events if they exist
  if (mTimerEvent != NULL) {
    gBS->CloseEvent (mTimerEvent);
    mTimerEvent = NULL;
  }
  
  if (mKeyMonitorEvent != NULL) {
    gBS->CloseEvent (mKeyMonitorEvent);
    mKeyMonitorEvent = NULL;
  }
  
  return EFI_SUCCESS;
}

**/

/**
  Entrypoint of this module.

  This function is the entrypoint of this module. It installs the Edkii
  Platform Logo protocol.

  @param  ImageHandle       The firmware allocated handle for the EFI image.
  @param  SystemTable       A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.

**/

/**

EFI_STATUS
EFIAPI
InitializeLogo (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                   Status;
  EFI_HII_PACKAGE_LIST_HEADER  *PackageList;
  EFI_HII_DATABASE_PROTOCOL    *HiiDatabase;
  EFI_HANDLE                   Handle;

  DEBUG ((DEBUG_INFO, "Logo Driver: Initializing...\n"));

  // Locate HII Database Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiDatabaseProtocolGuid,
                  NULL,
                  (VOID **)&HiiDatabase
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Database Protocol: %r\n", Status));
    return Status;
  }

  // Locate HII Image Ex Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiImageExProtocolGuid,
                  NULL,
                  (VOID **)&mHiiImageEx
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Image Ex Protocol: %r\n", Status));
    return Status;
  }

  // Retrieve HII package list from ImageHandle
  Status = gBS->OpenProtocol (
                  ImageHandle,
                  &gEfiHiiPackageListProtocolGuid,
                  (VOID **)&PackageList,
                  ImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "HII Image Package with logo not found in PE/COFF resource section\n"));
    return Status;
  }

  // Publish HII package list to HII Database
  Status = HiiDatabase->NewPackageList (
                          HiiDatabase,
                          PackageList,
                          NULL,
                          &mHiiHandle
                          );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to add package list to HII Database: %r\n", Status));
    return Status;
  }

  // Install Platform Logo Protocol
  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEdkiiPlatformLogoProtocolGuid,
                  &mPlatformLogo,
                  NULL
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to install Platform Logo Protocol: %r\n", Status));
    // Cleanup HII handle
    HiiDatabase->RemovePackageList (HiiDatabase, mHiiHandle);
    return Status;
  }

  DEBUG ((DEBUG_INFO, "Logo Driver: Initialized successfully\n"));
  
  return EFI_SUCCESS;
}


**/



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** @file
  Logo DXE Driver with F12 key BIOS setup functionality.
  Shows logo, waits for F12 key, then displays BIOS setup form.

Copyright (c) 2016 - 2017, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent
**/

/**

#include <Uefi.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/HiiImageEx.h>
#include <Protocol/PlatformLogo.h>
#include <Protocol/HiiPackageList.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/SimpleTextOut.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>

typedef struct {
  EFI_IMAGE_ID                             ImageId;
  EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE    Attribute;
  INTN                                     OffsetX;
  INTN                                     OffsetY;
} LOGO_ENTRY;

// Global variables
EFI_HII_IMAGE_EX_PROTOCOL  *mHiiImageEx;
EFI_HII_HANDLE             mHiiHandle;
EFI_EVENT                  mKeyMonitorEvent;
EFI_EVENT                  mTimerEvent;
BOOLEAN                    mLogoDisplayed = FALSE;
BOOLEAN                    mMessageShown = FALSE;
UINTN                      mTimerCounter = 0;

// Function declarations
VOID
DisplayBiosSetupForm (
  VOID
  );

VOID
GetCurrentTime (
  OUT CHAR16  *TimeString
  );

LOGO_ENTRY                 mLogos[] = {
  {
    IMAGE_TOKEN (IMG_LOGO),
    EdkiiPlatformLogoDisplayAttributeCenter,
    0,
    0
  }
};

**/

/**
  Get current time and format it as string.

  @param TimeString   Output buffer for formatted time string.
**/

/**

VOID
GetCurrentTime (
  OUT CHAR16  *TimeString
  )
{
  EFI_TIME    Time;
  EFI_STATUS  Status;
  
  // Attempt to get the system time
  Status = gRT->GetTime (&Time, NULL);
  if (!EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "GetCurrentTime: Successfully retrieved time - %02d:%02d:%02d\n", Time.Hour, Time.Minute, Time.Second));
    UnicodeSPrint (
      TimeString, 
      50 * sizeof(CHAR16),
      L"%02d:%02d:%02d",
      Time.Hour,
      Time.Minute,
      Time.Second
    );
  } else {
    DEBUG ((DEBUG_ERROR, "GetCurrentTime: Failed to get time, Status: %r\n", Status));
    // Fallback: Display a static message or placeholder
    UnicodeSPrint (TimeString, 50 * sizeof(CHAR16), L"Time Unavailable");
  }
}

**/


/**
  Display BIOS setup form with white background and current time.
**/
/**

VOID
DisplayBiosSetupForm (
  VOID
  )
{
  UINTN   Index;
  CHAR16  TimeString[50];
  
  // Clear screen
  gST->ConOut->ClearScreen (gST->ConOut);
  
  // Set white background with black text
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  
  // Fill entire screen with white background
  for (Index = 0; Index < 25; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Index);
    Print (L"                                                                                ");
  }
  
  // Display "CDAC Bangalore" at top center
  gST->ConOut->SetCursorPosition (gST->ConOut, 30, 2);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"CDAC Bangalore");
  
  // Get and display current time at top right corner
  GetCurrentTime (TimeString);
  gST->ConOut->SetCursorPosition (gST->ConOut, 65, 2);
  Print (L"%s", TimeString);
  
  // Add main content area
  gST->ConOut->SetCursorPosition (gST->ConOut, 25, 8);
  Print (L"====================================");
  gST->ConOut->SetCursorPosition (gST->ConOut, 30, 9);
  Print (L"BIOS SETUP UTILITY");
  gST->ConOut->SetCursorPosition (gST->ConOut, 25, 10);
  Print (L"====================================");
  
  // Display main message
  gST->ConOut->SetCursorPosition (gST->ConOut, 32, 12);
  Print (L"Welcome to BIOS Setup");
  
  // Show navigation options
  gST->ConOut->SetCursorPosition (gST->ConOut, 25, 15);
  Print (L"Use arrow keys to navigate");
  gST->ConOut->SetCursorPosition (gST->ConOut, 25, 16);
  Print (L"Press ESC to exit");
  gST->ConOut->SetCursorPosition (gST->ConOut, 25, 17);
  Print (L"Press F10 to save and exit");
  
  // Add footer
  gST->ConOut->SetCursorPosition (gST->ConOut, 20, 22);
  Print (L"BIOS Setup - F12 Key Activated - CDAC Bangalore");
}

**/

/**
  Monitor keyboard input for F12 key press only.

  @param Event      The event that is signaled.
  @param Context    The context passed to the event.
**/

/**

VOID
EFIAPI
KeyboardMonitor (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  
  // Only monitor after message is shown
  if (!mMessageShown) {
    return;
  }
  
  // Check if key is available
  Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
  
  if (!EFI_ERROR (Status)) {
    // Check for F12 key only
    if (Key.ScanCode == SCAN_F12) {
      DEBUG ((DEBUG_INFO, "F12 key pressed - showing BIOS setup form\n"));
      
      // Cancel timer and keyboard events to stop normal flow
      gBS->SetTimer (mKeyMonitorEvent, TimerCancel, 0);
      gBS->SetTimer (mTimerEvent, TimerCancel, 0);
      
      // Display BIOS setup form
      DisplayBiosSetupForm ();
      
      // Stay in this form indefinitely - no shell, no exit
      // System will remain in BIOS setup mode
      return;
    }
  }
}

**/


/**
  Timer callback function to handle logo display timing.

  @param Event      The event that is signaled.
  @param Context    The context passed to the event.
**/

/**

VOID
EFIAPI
TimerCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  mTimerCounter++;
  
  // After 5 seconds, show the message below logo and wait for F12
  if (mTimerCounter == 50 && !mMessageShown) { // 50 * 100ms = 5 seconds
    // Clear the entire screen first
    gST->ConOut->ClearScreen (gST->ConOut);
    
    // Position cursor below the logo area
    gST->ConOut->SetCursorPosition (gST->ConOut, 18, 18);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
    Print (L"Press F12 key for BIOS setup");
    
    mMessageShown = TRUE;
    
    // Reset console input for clean key detection
    gST->ConIn->Reset (gST->ConIn, FALSE);
    
    DEBUG ((DEBUG_INFO, "Message displayed, waiting for F12 key\n"));
  }
  
  // Explicitly do nothing after showing the message
  // Wait indefinitely for F12 key press, no shell redirect
}

**/


/**
  Setup timing and keyboard monitoring after logo is displayed.
**/

/**

VOID
SetupLogoTiming (
  VOID
  )
{
  EFI_STATUS  Status;
  
  if (mLogoDisplayed) {
    return; // Already setup
  }
  
  DEBUG ((DEBUG_INFO, "Setting up logo timing and keyboard monitoring\n"));
  
  // Create timer event for logo timing
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  TimerCallback,
                  NULL,
                  &mTimerEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    // Set timer to fire every 100ms
    Status = gBS->SetTimer (
                    mTimerEvent,
                    TimerPeriodic,
                    1000000  // 100ms in 100ns units
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set timer for mTimerEvent: %r\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to create mTimerEvent: %r\n", Status));
  }
  
  // Create keyboard monitoring event
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  KeyboardMonitor,
                  NULL,
                  &mKeyMonitorEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    // Set keyboard timer to fire every 50ms for responsive key detection
    Status = gBS->SetTimer (
                    mKeyMonitorEvent,
                    TimerPeriodic,
                    500000  // 50ms in 100ns units
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set timer for mKeyMonitorEvent: %r\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to create mKeyMonitorEvent: %r\n", Status));
  }
  
  mLogoDisplayed = TRUE;
}

**/


/**
  Load a platform logo image and return its data and attributes.

  @param This              The pointer to this protocol instance.
  @param Instance          The visible image instance is found.
  @param Image             Points to the image.
  @param Attribute         The display attributes of the image returned.
  @param OffsetX           The X offset of the image regarding the Attribute.
  @param OffsetY           The Y offset of the image regarding the Attribute.

  @retval EFI_SUCCESS      The image was fetched successfully.
  @retval EFI_NOT_FOUND    The specified image could not be found.
**/

/**

EFI_STATUS
EFIAPI
GetImage (
  IN     EDKII_PLATFORM_LOGO_PROTOCOL        *This,
  IN OUT UINT32                              *Instance,
  OUT EFI_IMAGE_INPUT                        *Image,
  OUT EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE  *Attribute,
  OUT INTN                                   *OffsetX,
  OUT INTN                                   *OffsetY
  )
{
  UINT32      Current;
  EFI_STATUS  Status;

  if ((Instance == NULL) || (Image == NULL) ||
      (Attribute == NULL) || (OffsetX == NULL) || (OffsetY == NULL))
  {
    return EFI_INVALID_PARAMETER;
  }

  Current = *Instance;
  if (Current >= ARRAY_SIZE (mLogos)) {
    return EFI_NOT_FOUND;
  }

  (*Instance)++;
  *Attribute = mLogos[Current].Attribute;
  *OffsetX   = mLogos[Current].OffsetX;
  *OffsetY   = mLogos[Current].OffsetY;
  
  // Get image from HII database
  Status = mHiiImageEx->GetImageEx (mHiiImageEx, mHiiHandle, mLogos[Current].ImageId, Image);
  
  // After first logo is displayed, setup timing
  if (Current == 0 && !EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Logo displayed, setting up timing\n"));
    SetupLogoTiming ();
  }
  
  return Status;
}

EDKII_PLATFORM_LOGO_PROTOCOL  mPlatformLogo = {
  GetImage
};

**/

/**
  Cleanup function called when driver is unloaded.

  @param ImageHandle    The handle of the image being unloaded.

  @retval EFI_SUCCESS   The image was unloaded successfully.
**/

/**
EFI_STATUS
EFIAPI
LogoDriverUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
  // Close events if they exist
  if (mTimerEvent != NULL) {
    gBS->CloseEvent (mTimerEvent);
    mTimerEvent = NULL;
  }
  
  if (mKeyMonitorEvent != NULL) {
    gBS->CloseEvent (mKeyMonitorEvent);
    mKeyMonitorEvent = NULL;
  }
  
  return EFI_SUCCESS;
}

**/

/**
  Entrypoint of this module.

  This function is the entrypoint of this module. It installs the Edkii
  Platform Logo protocol.

  @param  ImageHandle       The firmware allocated handle for the EFI image.
  @param  SystemTable       A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
**/

/**

EFI_STATUS
EFIAPI
InitializeLogo (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                   Status;
  EFI_HII_PACKAGE_LIST_HEADER  *PackageList;
  EFI_HII_DATABASE_PROTOCOL    *HiiDatabase;
  EFI_HANDLE                   Handle;

  DEBUG ((DEBUG_INFO, "Logo Driver: Initializing...\n"));

  // Locate HII Database Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiDatabaseProtocolGuid,
                  NULL,
                  (VOID **)&HiiDatabase
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Database Protocol: %r\n", Status));
    return Status;
  }

  // Locate HII Image Ex Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiImageExProtocolGuid,
                  NULL,
                  (VOID **)&mHiiImageEx
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Image Ex Protocol: %r\n", Status));
    return Status;
  }

  // Retrieve HII package list from ImageHandle
  Status = gBS->OpenProtocol (
                  ImageHandle,
                  &gEfiHiiPackageListProtocolGuid,
                  (VOID **)&PackageList,
                  ImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "HII Image Package with logo not found in PE/COFF resource section\n"));
    return Status;
  }

  // Publish HII package list to HII Database
  Status = HiiDatabase->NewPackageList (
                          HiiDatabase,
                          PackageList,
                          NULL,
                          &mHiiHandle
                          );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to add package list to HII Database: %r\n", Status));
    return Status;
  }

  // Install Platform Logo Protocol
  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEdkiiPlatformLogoProtocolGuid,
                  &mPlatformLogo,
                  NULL
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to install Platform Logo Protocol: %r\n", Status));
    // Cleanup HII handle
    HiiDatabase->RemovePackageList (HiiDatabase, mHiiHandle);
    return Status;
  }

  DEBUG ((DEBUG_INFO, "Logo Driver: Initialized successfully\n"));
  
  return EFI_SUCCESS;
}


**/


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



/** @file
  Logo DXE Driver with F12 key BIOS setup functionality.
  Shows logo, waits for F12 key, then displays BIOS setup form with navigation.

Copyright (c) 2016 - 2017, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent
**/

/**

#include <Uefi.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/HiiImageEx.h>
#include <Protocol/PlatformLogo.h>
#include <Protocol/HiiPackageList.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/SimpleTextOut.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>

typedef struct {
  EFI_IMAGE_ID                             ImageId;
  EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE    Attribute;
  INTN                                     OffsetX;
  INTN                                     OffsetY;
} LOGO_ENTRY;

// Menu item structure
typedef struct {
  CHAR16  *MenuText;
  CHAR16  *Description;
} MENU_ITEM;

// Global variables
EFI_HII_IMAGE_EX_PROTOCOL  *mHiiImageEx;
EFI_HII_HANDLE             mHiiHandle;
EFI_EVENT                  mKeyMonitorEvent;
EFI_EVENT                  mTimerEvent;
EFI_EVENT                  mBiosSetupKeyEvent;
BOOLEAN                    mLogoDisplayed = FALSE;
BOOLEAN                    mMessageShown = FALSE;
BOOLEAN                    mInBiosSetup = FALSE;
UINTN                      mTimerCounter = 0;
UINTN                      mSelectedMenuItem = 0;
UINTN                      mTotalMenuItems = 0;

// Menu items for BIOS setup
MENU_ITEM mBiosSetupMenu[] = {
  //{ L"Main", L"Main system configuration" },
   {  L"Boot Manager" },
  // { L"Advanced", L"Advanced system settings" },
   {  L"Device manager" },
  // { L"Chipset", L"Chipset configuration" },
  {  L"Boot maintenance manager" },
 // { L"Security", L"Security settings" },
  //{ L"Boot", L"Boot configuration" },
  { L"Save & Exit", L"Save changes and exit" },
  { L"Exit Without Saving", L"Exit without saving changes" }
};

// Function declarations
VOID
DisplayBiosSetupForm (
  VOID
  );

VOID
GetCurrentTime (
  OUT CHAR16  *TimeString
  );

VOID
HandleBiosSetupNavigation (
  VOID
  );

VOID
DisplaySelectedMenuInfo (
  VOID
  );

LOGO_ENTRY                 mLogos[] = {
  {
    IMAGE_TOKEN (IMG_LOGO),
    EdkiiPlatformLogoDisplayAttributeCenter,
    0,
    0
  }
};

**/

/**
  Get current time and format it as string.

  @param TimeString   Output buffer for formatted time string.
**/
/**

VOID
GetCurrentTime (
  OUT CHAR16  *TimeString
  )
{
  EFI_TIME    Time;
  EFI_STATUS  Status;
  
  // Attempt to get the system time
  Status = gRT->GetTime (&Time, NULL);
  if (!EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "GetCurrentTime: Successfully retrieved time - %02d:%02d:%02d\n", Time.Hour, Time.Minute, Time.Second));
    UnicodeSPrint (
      TimeString, 
      50 * sizeof(CHAR16),
      L"%02d:%02d:%02d",
      Time.Hour,
      Time.Minute,
      Time.Second
    );
  } else {
    DEBUG ((DEBUG_ERROR, "GetCurrentTime: Failed to get time, Status: %r\n", Status));
    // Fallback: Display a static message or placeholder
    UnicodeSPrint (TimeString, 50 * sizeof(CHAR16), L"Time Unavailable");
  }
}

**/

/**
  Display information about the selected menu item.
**/
/**

VOID
DisplaySelectedMenuInfo (
  VOID
  )
{
  // Clear the info area
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 12);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"                                    ");
  
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 13);
  Print (L"                                    ");
  
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 14);
  Print (L"                                    ");
  
  // Display selected item info
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 12);
  Print (L"Selected: %s", mBiosSetupMenu[mSelectedMenuItem].MenuText);
  
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 14);
  Print (L"%s", mBiosSetupMenu[mSelectedMenuItem].Description);
}

**/

/**
  Display BIOS setup form with white background, menu items and navigation.
**/
/**

VOID
DisplayBiosSetupForm (
  VOID
  )
{
  UINTN   Index;
  CHAR16  TimeString[50];
  
  mTotalMenuItems = ARRAY_SIZE (mBiosSetupMenu);
  mSelectedMenuItem = 0;
  mInBiosSetup = TRUE;
  
  // Clear screen
  gST->ConOut->ClearScreen (gST->ConOut);
  
  // Set white background with black text
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  
  // Fill entire screen with white background
  for (Index = 0; Index < 25; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Index);
    Print (L"                                                                                ");
  }
  
  // Display "CDAC Bangalore" at top center
  gST->ConOut->SetCursorPosition (gST->ConOut, 30, 2);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"CDAC Bangalore");
  
  // Get and display current time at top right corner
  GetCurrentTime (TimeString);
  gST->ConOut->SetCursorPosition (gST->ConOut, 65, 2);
  Print (L"%s", TimeString);
  
  // Add main content area
  gST->ConOut->SetCursorPosition (gST->ConOut, 25, 5);
  Print (L"====================================");
  gST->ConOut->SetCursorPosition (gST->ConOut, 30, 6);
  Print (L"BIOS SETUP UTILITY");
  gST->ConOut->SetCursorPosition (gST->ConOut, 25, 7);
  Print (L"====================================");
  
  // Display menu items
  for (Index = 0; Index < mTotalMenuItems; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 10, 10 + Index);
    
    if (Index == mSelectedMenuItem) {
      // Highlight selected item with reverse colors
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
      Print (L"-> %s", mBiosSetupMenu[Index].MenuText);
    } else {
      // Normal menu item
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"   %s", mBiosSetupMenu[Index].MenuText);
    }
  }
  
  // Display selected item information
  DisplaySelectedMenuInfo ();
  
  // Show navigation instructions
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  gST->ConOut->SetCursorPosition (gST->ConOut, 10, 20);
  Print (L"Navigation Instructions:");
  gST->ConOut->SetCursorPosition (gST->ConOut, 10, 21);
  Print (L"Use UP/DOWN arrow keys to navigate");
  gST->ConOut->SetCursorPosition (gST->ConOut, 10, 22);
  Print (L"Press ENTER to select");
  gST->ConOut->SetCursorPosition (gST->ConOut, 10, 23);
  Print (L"Press ESC to exit setup");
  
  // Add footer
  gST->ConOut->SetCursorPosition (gST->ConOut, 20, 24);
  Print (L"BIOS Setup - F12 Key Activated - CDAC Bangalore");
  
  // Reset console input for clean key detection in setup
  gST->ConIn->Reset (gST->ConIn, FALSE);
  
  // Start navigation handling
  HandleBiosSetupNavigation ();
}

**/

/**
  Handle menu selection based on current selected item.
**/
/**

VOID
HandleMenuSelection (
  VOID
  )
{
  switch (mSelectedMenuItem) {
    case 0: // Main
      gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
      // Print (L"Main menu selected!");
       Print (L"Boot Manager!");
      break;
      
    case 1: // Advanced
      gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
     // Print (L"Advanced menu selected!");
      Print (L"Device Manager!");
      break;
      
    case 2: // Chipset
      gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
      // Print (L"Chipset menu selected!");
       Print (L"Boot Maintenance Manager!");
      break;
   
   **/
   
    /**
    
    case 3: // Security
      gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
      Print (L"Security menu selected!");
      break;
      
    case 4: // Boot
      gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
      Print (L"Boot menu selected!");
      break;
      **/
      
      /**
      
    case 5: // Save & Exit
      gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_GREEN | EFI_BACKGROUND_BLUE);
      Print (L"Saving and exiting...");
      // Add save logic here
      gBS->Stall (2000000); // Wait 2 seconds
      mInBiosSetup = FALSE;
      break;
      
    case 6: // Exit Without Saving
      gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_RED | EFI_BACKGROUND_BLUE);
      Print (L"Exiting without saving...");
      gBS->Stall (2000000); // Wait 2 seconds
      mInBiosSetup = FALSE;
      break;
      
    default:
      break;
  }
  
  // If not exiting, continue showing the menu
  if (mInBiosSetup) {
    gBS->Stall (1000000); // Wait 1 second to show selection message
    DisplayBiosSetupForm (); // Refresh the form
  }
}

**/

/**
  Handle navigation within BIOS setup using arrow keys.
**/
/**

VOID
HandleBiosSetupNavigation (
  VOID
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  UINTN          PreviousSelection;
  
  while (mInBiosSetup) {
    // Wait for key input
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
    
    if (!EFI_ERROR (Status)) {
      PreviousSelection = mSelectedMenuItem;
      
      switch (Key.ScanCode) {
        case SCAN_UP:
          // Move up in menu
          if (mSelectedMenuItem > 0) {
            mSelectedMenuItem--;
          } else {
            mSelectedMenuItem = mTotalMenuItems - 1; // Wrap to bottom
          }
          break;
          
        case SCAN_DOWN:
          // Move down in menu
          if (mSelectedMenuItem < mTotalMenuItems - 1) {
            mSelectedMenuItem++;
          } else {
            mSelectedMenuItem = 0; // Wrap to top
          }
          break;
          
        case SCAN_ESC:
          // Exit BIOS setup
          mInBiosSetup = FALSE;
          gST->ConOut->ClearScreen (gST->ConOut);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
          gST->ConOut->SetCursorPosition (gST->ConOut, 30, 12);
          Print (L"Exiting BIOS Setup...");
          gBS->Stall (1000000); // Wait 1 second
          return;
          
        default:
          // Check for ENTER key (Unicode character)
          if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
            HandleMenuSelection ();
            if (!mInBiosSetup) {
              return; // Exit if user chose to exit
            }
          }
          break;
      }
      
      // Update display if selection changed
      if (mInBiosSetup && (PreviousSelection != mSelectedMenuItem)) {
        // Update previous item display (remove highlight)
        gST->ConOut->SetCursorPosition (gST->ConOut, 10, 10 + PreviousSelection);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"   %s", mBiosSetupMenu[PreviousSelection].MenuText);
        
        // Update current item display (add highlight)
        gST->ConOut->SetCursorPosition (gST->ConOut, 10, 10 + mSelectedMenuItem);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
        Print (L"-> %s", mBiosSetupMenu[mSelectedMenuItem].MenuText);
        
        // Update selected item information
        DisplaySelectedMenuInfo ();
      }
    } else {
      // Small delay to prevent excessive CPU usage
      gBS->Stall (50000); // 50ms
    }
  }
}

**/

/**
  Monitor keyboard input for F12 key press only.

  @param Event      The event that is signaled.
  @param Context    The context passed to the event.
**/
/**

VOID
EFIAPI
KeyboardMonitor (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  
  // Only monitor after message is shown and not in BIOS setup
  if (!mMessageShown || mInBiosSetup) {
    return;
  }
  
  // Check if key is available
  Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
  
  if (!EFI_ERROR (Status)) {
    // Check for F12 key only
    if (Key.ScanCode == SCAN_F12) {
      DEBUG ((DEBUG_INFO, "F12 key pressed - showing BIOS setup form\n"));
      
      // Cancel timer and keyboard events to stop normal flow
      gBS->SetTimer (mKeyMonitorEvent, TimerCancel, 0);
      gBS->SetTimer (mTimerEvent, TimerCancel, 0);
      
      // Display BIOS setup form
      DisplayBiosSetupForm ();
      
      // After exiting BIOS setup, show exit message
      gST->ConOut->ClearScreen (gST->ConOut);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
      gST->ConOut->SetCursorPosition (gST->ConOut, 25, 12);
      Print (L"BIOS Setup completed. System ready.");
      
      return;
    }
  }
}

**/

/**
  Timer callback function to handle logo display timing.

  @param Event      The event that is signaled.
  @param Context    The context passed to the event.
**/
/**

VOID
EFIAPI
TimerCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  mTimerCounter++;
  
  // After 5 seconds, show the message below logo and wait for F12
  if (mTimerCounter == 50 && !mMessageShown) { // 50 * 100ms = 5 seconds
    // Clear the entire screen first
    gST->ConOut->ClearScreen (gST->ConOut);
    
    // Position cursor below the logo area
    gST->ConOut->SetCursorPosition (gST->ConOut, 18, 18);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
    Print (L"Press F12 key for BIOS setup");
    
    mMessageShown = TRUE;
    
    // Reset console input for clean key detection
    gST->ConIn->Reset (gST->ConIn, FALSE);
    
    DEBUG ((DEBUG_INFO, "Message displayed, waiting for F12 key\n"));
  }
  
  // Explicitly do nothing after showing the message
  // Wait indefinitely for F12 key press, no shell redirect
}

**/

/**
  Setup timing and keyboard monitoring after logo is displayed.
**/

/**
VOID
SetupLogoTiming (
  VOID
  )
{
  EFI_STATUS  Status;
  
  if (mLogoDisplayed) {
    return; // Already setup
  }
  
  DEBUG ((DEBUG_INFO, "Setting up logo timing and keyboard monitoring\n"));
  
  // Create timer event for logo timing
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  TimerCallback,
                  NULL,
                  &mTimerEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    // Set timer to fire every 100ms
    Status = gBS->SetTimer (
                    mTimerEvent,
                    TimerPeriodic,
                    1000000  // 100ms in 100ns units
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set timer for mTimerEvent: %r\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to create mTimerEvent: %r\n", Status));
  }
  
  // Create keyboard monitoring event
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  KeyboardMonitor,
                  NULL,
                  &mKeyMonitorEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    // Set keyboard timer to fire every 50ms for responsive key detection
    Status = gBS->SetTimer (
                    mKeyMonitorEvent,
                    TimerPeriodic,
                    500000  // 50ms in 100ns units
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set timer for mKeyMonitorEvent: %r\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to create mKeyMonitorEvent: %r\n", Status));
  }
  
  mLogoDisplayed = TRUE;
}
**/

/**
  Load a platform logo image and return its data and attributes.

  @param This              The pointer to this protocol instance.
  @param Instance          The visible image instance is found.
  @param Image             Points to the image.
  @param Attribute         The display attributes of the image returned.
  @param OffsetX           The X offset of the image regarding the Attribute.
  @param OffsetY           The Y offset of the image regarding the Attribute.

  @retval EFI_SUCCESS      The image was fetched successfully.
  @retval EFI_NOT_FOUND    The specified image could not be found.
**/

/**
EFI_STATUS
EFIAPI
GetImage (
  IN     EDKII_PLATFORM_LOGO_PROTOCOL        *This,
  IN OUT UINT32                              *Instance,
  OUT EFI_IMAGE_INPUT                        *Image,
  OUT EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE  *Attribute,
  OUT INTN                                   *OffsetX,
  OUT INTN                                   *OffsetY
  )
{
  UINT32      Current;
  EFI_STATUS  Status;

  if ((Instance == NULL) || (Image == NULL) ||
      (Attribute == NULL) || (OffsetX == NULL) || (OffsetY == NULL))
  {
    return EFI_INVALID_PARAMETER;
  }

  Current = *Instance;
  if (Current >= ARRAY_SIZE (mLogos)) {
    return EFI_NOT_FOUND;
  }

  (*Instance)++;
  *Attribute = mLogos[Current].Attribute;
  *OffsetX   = mLogos[Current].OffsetX;
  *OffsetY   = mLogos[Current].OffsetY;
  
  // Get image from HII database
  Status = mHiiImageEx->GetImageEx (mHiiImageEx, mHiiHandle, mLogos[Current].ImageId, Image);
  
  // After first logo is displayed, setup timing
  if (Current == 0 && !EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Logo displayed, setting up timing\n"));
    SetupLogoTiming ();
  }
  
  return Status;
}

EDKII_PLATFORM_LOGO_PROTOCOL  mPlatformLogo = {
  GetImage
};

**/

/**
  Cleanup function called when driver is unloaded.

  @param ImageHandle    The handle of the image being unloaded.

  @retval EFI_SUCCESS   The image was unloaded successfully.
**/
/**

EFI_STATUS
EFIAPI
LogoDriverUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
  // Close events if they exist
  if (mTimerEvent != NULL) {
    gBS->CloseEvent (mTimerEvent);
    mTimerEvent = NULL;
  }
  
  if (mKeyMonitorEvent != NULL) {
    gBS->CloseEvent (mKeyMonitorEvent);
    mKeyMonitorEvent = NULL;
  }
  
  if (mBiosSetupKeyEvent != NULL) {
    gBS->CloseEvent (mBiosSetupKeyEvent);
    mBiosSetupKeyEvent = NULL;
  }
  
  return EFI_SUCCESS;
}

**/

/**
  Entrypoint of this module.

  This function is the entrypoint of this module. It installs the Edkii
  Platform Logo protocol.

  @param  ImageHandle       The firmware allocated handle for the EFI image.
  @param  SystemTable       A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
**/
/**

EFI_STATUS
EFIAPI
InitializeLogo (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                   Status;
  EFI_HII_PACKAGE_LIST_HEADER  *PackageList;
  EFI_HII_DATABASE_PROTOCOL    *HiiDatabase;
  EFI_HANDLE                   Handle;

  DEBUG ((DEBUG_INFO, "Logo Driver: Initializing...\n"));

  // Initialize global variables
  mInBiosSetup = FALSE;
  mSelectedMenuItem = 0;
  mTotalMenuItems = ARRAY_SIZE (mBiosSetupMenu);

  // Locate HII Database Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiDatabaseProtocolGuid,
                  NULL,
                  (VOID **)&HiiDatabase
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Database Protocol: %r\n", Status));
    return Status;
  }

  // Locate HII Image Ex Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiImageExProtocolGuid,
                  NULL,
                  (VOID **)&mHiiImageEx
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Image Ex Protocol: %r\n", Status));
    return Status;
  }

  // Retrieve HII package list from ImageHandle
  Status = gBS->OpenProtocol (
                  ImageHandle,
                  &gEfiHiiPackageListProtocolGuid,
                  (VOID **)&PackageList,
                  ImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "HII Image Package with logo not found in PE/COFF resource section\n"));
    return Status;
  }

  // Publish HII package list to HII Database
  Status = HiiDatabase->NewPackageList (
                          HiiDatabase,
                          PackageList,
                          NULL,
                          &mHiiHandle
                          );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to add package list to HII Database: %r\n", Status));
    return Status;
  }

  // Install Platform Logo Protocol
  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEdkiiPlatformLogoProtocolGuid,
                  &mPlatformLogo,
                  NULL
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to install Platform Logo Protocol: %r\n", Status));
    // Cleanup HII handle
    HiiDatabase->RemovePackageList (HiiDatabase, mHiiHandle);
    return Status;
  }

  DEBUG ((DEBUG_INFO, "Logo Driver: Initialized successfully\n"));
  
  return EFI_SUCCESS;
}

**/


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// after pressed esc button system will be shut down.

/** @file
  Logo DXE Driver with F12 key BIOS setup functionality.
  Shows logo, waits for F12 key, then displays BIOS setup form with navigation.

Copyright (c) 2016 - 2017, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent
**/

/**

#include <Uefi.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/HiiImageEx.h>
#include <Protocol/PlatformLogo.h>
#include <Protocol/HiiPackageList.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/SimpleTextOut.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>

typedef struct {
  EFI_IMAGE_ID                             ImageId;
  EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE    Attribute;
  INTN                                     OffsetX;
  INTN                                     OffsetY;
} LOGO_ENTRY;

// Menu item structure
typedef struct {
  CHAR16  *MenuText;
  CHAR16  *Description;
} MENU_ITEM;

// Global variables
EFI_HII_IMAGE_EX_PROTOCOL  *mHiiImageEx;
EFI_HII_HANDLE             mHiiHandle;
EFI_EVENT                  mKeyMonitorEvent;
EFI_EVENT                  mTimerEvent;
EFI_EVENT                  mBiosSetupKeyEvent;
BOOLEAN                    mLogoDisplayed = FALSE;
BOOLEAN                    mMessageShown = FALSE;
BOOLEAN                    mInBiosSetup = FALSE;
UINTN                      mTimerCounter = 0;
UINTN                      mSelectedMenuItem = 0;
UINTN                      mTotalMenuItems = 0;

// Menu items for BIOS setup - Fixed structure
MENU_ITEM mBiosSetupMenu[] = {
  { L"Boot Manager", L"Configure boot device priority" },
  { L"Device Manager", L"Configure hardware devices" },
  { L"Boot Maintenance Manager", L"Advanced boot options" },
  { L"Save & Exit", L"Save changes and exit" },
  { L"Exit Without Saving", L"Exit without saving changes" }
};

// Function declarations
VOID
DisplayBiosSetupForm (
  VOID
  );

VOID
GetCurrentTime (
  OUT CHAR16  *TimeString
  );

VOID
HandleBiosSetupNavigation (
  VOID
  );

VOID
DisplaySelectedMenuInfo (
  VOID
  );

LOGO_ENTRY                 mLogos[] = {
  {
    IMAGE_TOKEN (IMG_LOGO),
    EdkiiPlatformLogoDisplayAttributeCenter,
    0,
    0
  }
};

**/

/**
  Get current time and format it as string.

  @param TimeString   Output buffer for formatted time string.
**/
/**

VOID
GetCurrentTime (
  OUT CHAR16  *TimeString
  )
{
  EFI_TIME    Time;
  EFI_STATUS  Status;
  
  // Attempt to get the system time
  Status = gRT->GetTime (&Time, NULL);
  if (!EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "GetCurrentTime: Successfully retrieved time - %02d:%02d:%02d\n", Time.Hour, Time.Minute, Time.Second));
    UnicodeSPrint (
      TimeString, 
      50 * sizeof(CHAR16),
      L"%02d:%02d:%02d",
      Time.Hour,
      Time.Minute,
      Time.Second
    );
  } else {
    DEBUG ((DEBUG_ERROR, "GetCurrentTime: Failed to get time, Status: %r\n", Status));
    // Fallback: Display a static message or placeholder
    UnicodeSPrint (TimeString, 50 * sizeof(CHAR16), L"Time Unavailable");
  }
}

**/

/**
  Display information about the selected menu item.
**/

/**
VOID
DisplaySelectedMenuInfo (
  VOID
  )
{
  // Clear the info area
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 12);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"                                    ");
  
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 13);
  Print (L"                                    ");
  
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 14);
  Print (L"                                    ");
  
  // Display selected item info
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 12);
  Print (L"Selected: %s", mBiosSetupMenu[mSelectedMenuItem].MenuText);
  
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 14);
  Print (L"%s", mBiosSetupMenu[mSelectedMenuItem].Description);
}

**/

/**
  Display BIOS setup form with white background, menu items and navigation.
**/

/**
VOID
DisplayBiosSetupForm (
  VOID
  )
{
  UINTN   Index;
  CHAR16  TimeString[50];
  
  mTotalMenuItems = ARRAY_SIZE (mBiosSetupMenu);
  mSelectedMenuItem = 0;
  mInBiosSetup = TRUE;
  
  // Clear screen
  gST->ConOut->ClearScreen (gST->ConOut);
  
  // Set white background with black text
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  
  // Fill entire screen with white background
  for (Index = 0; Index < 25; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Index);
    Print (L"                                                                                ");
  }
  
  // Display "CDAC Bangalore" at top center
  gST->ConOut->SetCursorPosition (gST->ConOut, 30, 2);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"CDAC Bangalore");
  
  // Get and display current time at top right corner
  GetCurrentTime (TimeString);
  gST->ConOut->SetCursorPosition (gST->ConOut, 65, 2);
  Print (L"%s", TimeString);
  
  // Add main content area
  gST->ConOut->SetCursorPosition (gST->ConOut, 25, 5);
  Print (L"====================================");
  gST->ConOut->SetCursorPosition (gST->ConOut, 30, 6);
  Print (L"BIOS SETUP UTILITY");
  gST->ConOut->SetCursorPosition (gST->ConOut, 25, 7);
  Print (L"====================================");
  
  // Display menu items
  for (Index = 0; Index < mTotalMenuItems; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 10, 10 + Index);
    
    if (Index == mSelectedMenuItem) {
      // Highlight selected item with reverse colors
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
      Print (L"-> %s", mBiosSetupMenu[Index].MenuText);
    } else {
      // Normal menu item
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"   %s", mBiosSetupMenu[Index].MenuText);
    }
  }
  
  // Display selected item information
  DisplaySelectedMenuInfo ();
  
  // Show navigation instructions
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  gST->ConOut->SetCursorPosition (gST->ConOut, 10, 20);
  Print (L"Navigation Instructions:");
  gST->ConOut->SetCursorPosition (gST->ConOut, 10, 21);
  Print (L"Use UP/DOWN arrow keys to navigate");
  gST->ConOut->SetCursorPosition (gST->ConOut, 10, 22);
  Print (L"Press ENTER to select");
  gST->ConOut->SetCursorPosition (gST->ConOut, 10, 23);
  Print (L"Press ESC to exit setup");
  
  // Add footer
  gST->ConOut->SetCursorPosition (gST->ConOut, 20, 24);
  Print (L"BIOS Setup - F12 Key Activated - CDAC Bangalore");
  
  // Reset console input for clean key detection in setup
  gST->ConIn->Reset (gST->ConIn, FALSE);
  
  // Start navigation handling
  HandleBiosSetupNavigation ();
}

**/


/**
  Handle menu selection based on current selected item.
  Modified to shutdown system instead of trying to continue boot.
**/

/**
VOID
HandleMenuSelection (
  VOID
  )
{
  switch (mSelectedMenuItem) {
    case 0: // Boot Manager
      gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
      Print (L"Boot Manager!");
      break;
      
    case 1: // Device Manager  
      gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
      Print (L"Device Manager!");
      break;
      
    case 2: // Boot Maintenance Manager
      gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
      Print (L"Boot Maintenance Manager!");
      break;
    
    case 3: // Save & Exit
      gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_GREEN | EFI_BACKGROUND_BLUE);
      Print (L"Saving and exiting...");
      gBS->Stall (2000000); // Wait 2 seconds
      
      // Show shutdown message
      gST->ConOut->ClearScreen (gST->ConOut);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
      gST->ConOut->SetCursorPosition (gST->ConOut, 25, 12);
      Print (L"Settings saved. Shutting down system...");
      gBS->Stall (2000000); // Wait 2 seconds
      
      // Shutdown the system instead of continuing boot
      gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
      
      mInBiosSetup = FALSE;
      break;
      
    case 4: // Exit Without Saving
      gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_RED | EFI_BACKGROUND_BLUE);
      Print (L"Exiting without saving...");
      gBS->Stall (2000000); // Wait 2 seconds
      
      // Show shutdown message
      gST->ConOut->ClearScreen (gST->ConOut);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
      gST->ConOut->SetCursorPosition (gST->ConOut, 25, 12);
      Print (L"Exiting setup. Shutting down system...");
      gBS->Stall (2000000); // Wait 2 seconds
      
      // Shutdown the system instead of continuing boot  
      gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
      
      mInBiosSetup = FALSE;
      break;
      
    default:
      break;
  }
  
  // If not exiting, continue showing the menu
  if (mInBiosSetup) {
    gBS->Stall (1000000); // Wait 1 second to show selection message
    DisplayBiosSetupForm (); // Refresh the form
  }
}

**/


/**
  Handle navigation within BIOS setup using arrow keys.
  Modified to shutdown on ESC instead of continuing boot.
**/

/**
VOID
HandleBiosSetupNavigation (
  VOID
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  UINTN          PreviousSelection;
  
  while (mInBiosSetup) {
    // Wait for key input
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
    
    if (!EFI_ERROR (Status)) {
      PreviousSelection = mSelectedMenuItem;
      
      switch (Key.ScanCode) {
        case SCAN_UP:
          // Move up in menu
          if (mSelectedMenuItem > 0) {
            mSelectedMenuItem--;
          } else {
            mSelectedMenuItem = mTotalMenuItems - 1; // Wrap to bottom
          }
          break;
          
        case SCAN_DOWN:
          // Move down in menu
          if (mSelectedMenuItem < mTotalMenuItems - 1) {
            mSelectedMenuItem++;
          } else {
            mSelectedMenuItem = 0; // Wrap to top
          }
          break;
          
        case SCAN_ESC:
          // Exit BIOS setup with shutdown
          mInBiosSetup = FALSE;
          gST->ConOut->ClearScreen (gST->ConOut);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
          gST->ConOut->SetCursorPosition (gST->ConOut, 25, 12);
          Print (L"Exiting BIOS Setup. Shutting down system...");
          gBS->Stall (2000000); // Wait 2 seconds
          
          // Shutdown instead of returning to boot process
          gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
          return;
          
        default:
          // Check for ENTER key (Unicode character)
          if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
            HandleMenuSelection ();
            if (!mInBiosSetup) {
              return; // Exit if user chose to exit (will shutdown in HandleMenuSelection)
            }
          }
          break;
      }
      
      // Update display if selection changed
      if (mInBiosSetup && (PreviousSelection != mSelectedMenuItem)) {
        // Update previous item display (remove highlight)
        gST->ConOut->SetCursorPosition (gST->ConOut, 10, 10 + PreviousSelection);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"   %s", mBiosSetupMenu[PreviousSelection].MenuText);
        
        // Update current item display (add highlight)
        gST->ConOut->SetCursorPosition (gST->ConOut, 10, 10 + mSelectedMenuItem);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
        Print (L"-> %s", mBiosSetupMenu[mSelectedMenuItem].MenuText);
        
        // Update selected item information
        DisplaySelectedMenuInfo ();
      }
    } else {
      // Small delay to prevent excessive CPU usage
      gBS->Stall (50000); // 50ms
    }
  }
}


**/

/**
  Monitor keyboard input for F12 key press only.
  Modified to shutdown after BIOS setup completion.
**/

/**
VOID
EFIAPI
KeyboardMonitor (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  
  // Only monitor after message is shown and not in BIOS setup
  if (!mMessageShown || mInBiosSetup) {
    return;
  }
  
  // Check if key is available
  Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
  
  if (!EFI_ERROR (Status)) {
    // Check for F12 key only
    if (Key.ScanCode == SCAN_F12) {
      DEBUG ((DEBUG_INFO, "F12 key pressed - showing BIOS setup form\n"));
      
      // Cancel timer and keyboard events to stop normal flow
      gBS->SetTimer (mKeyMonitorEvent, TimerCancel, 0);
      gBS->SetTimer (mTimerEvent, TimerCancel, 0);
      
      // Display BIOS setup form
      DisplayBiosSetupForm ();
      
      // After exiting BIOS setup, shutdown (this code won't reach normally 
      // because HandleBiosSetupNavigation() handles shutdown)
      gST->ConOut->ClearScreen (gST->ConOut);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
      gST->ConOut->SetCursorPosition (gST->ConOut, 25, 12);
      Print (L"BIOS Setup completed. Shutting down system...");
      gBS->Stall (2000000); // Wait 2 seconds
      
      // Shutdown the system
      gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
      
      return;
    }
  }
}

**/

/**
  Timer callback function to handle logo display timing.

  @param Event      The event that is signaled.
  @param Context    The context passed to the event.
**/

/**
VOID
EFIAPI
TimerCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  mTimerCounter++;
  
  // After 5 seconds, show the message below logo and wait for F12
  if (mTimerCounter == 50 && !mMessageShown) { // 50 * 100ms = 5 seconds
    // Clear the entire screen first
    gST->ConOut->ClearScreen (gST->ConOut);
    
    // Position cursor below the logo area
    gST->ConOut->SetCursorPosition (gST->ConOut, 18, 18);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
    Print (L"Press F12 key for BIOS setup");
    
    mMessageShown = TRUE;
    
    // Reset console input for clean key detection
    gST->ConIn->Reset (gST->ConIn, FALSE);
    
    DEBUG ((DEBUG_INFO, "Message displayed, waiting for F12 key\n"));
  }
  
  // Explicitly do nothing after showing the message
  // Wait indefinitely for F12 key press, no shell redirect
}


**/
/**
  Setup timing and keyboard monitoring after logo is displayed.
**/

/**
VOID
SetupLogoTiming (
  VOID
  )
{
  EFI_STATUS  Status;
  
  if (mLogoDisplayed) {
    return; // Already setup
  }
  
  DEBUG ((DEBUG_INFO, "Setting up logo timing and keyboard monitoring\n"));
  
  // Create timer event for logo timing
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  TimerCallback,
                  NULL,
                  &mTimerEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    // Set timer to fire every 100ms
    Status = gBS->SetTimer (
                    mTimerEvent,
                    TimerPeriodic,
                    1000000  // 100ms in 100ns units
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set timer for mTimerEvent: %r\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to create mTimerEvent: %r\n", Status));
  }
  
  // Create keyboard monitoring event
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  KeyboardMonitor,
                  NULL,
                  &mKeyMonitorEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    // Set keyboard timer to fire every 50ms for responsive key detection
    Status = gBS->SetTimer (
                    mKeyMonitorEvent,
                    TimerPeriodic,
                    500000  // 50ms in 100ns units
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set timer for mKeyMonitorEvent: %r\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to create mKeyMonitorEvent: %r\n", Status));
  }
  
  mLogoDisplayed = TRUE;
}

**/

/**
  Load a platform logo image and return its data and attributes.

  @param This              The pointer to this protocol instance.
  @param Instance          The visible image instance is found.
  @param Image             Points to the image.
  @param Attribute         The display attributes of the image returned.
  @param OffsetX           The X offset of the image regarding the Attribute.
  @param OffsetY           The Y offset of the image regarding the Attribute.

  @retval EFI_SUCCESS      The image was fetched successfully.
  @retval EFI_NOT_FOUND    The specified image could not be found.
**/

/**

EFI_STATUS
EFIAPI
GetImage (
  IN     EDKII_PLATFORM_LOGO_PROTOCOL        *This,
  IN OUT UINT32                              *Instance,
  OUT EFI_IMAGE_INPUT                        *Image,
  OUT EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE  *Attribute,
  OUT INTN                                   *OffsetX,
  OUT INTN                                   *OffsetY
  )
{
  UINT32      Current;
  EFI_STATUS  Status;

  if ((Instance == NULL) || (Image == NULL) ||
      (Attribute == NULL) || (OffsetX == NULL) || (OffsetY == NULL))
  {
    return EFI_INVALID_PARAMETER;
  }

  Current = *Instance;
  if (Current >= ARRAY_SIZE (mLogos)) {
    return EFI_NOT_FOUND;
  }

  (*Instance)++;
  *Attribute = mLogos[Current].Attribute;
  *OffsetX   = mLogos[Current].OffsetX;
  *OffsetY   = mLogos[Current].OffsetY;
  
  // Get image from HII database
  Status = mHiiImageEx->GetImageEx (mHiiImageEx, mHiiHandle, mLogos[Current].ImageId, Image);
  
  // After first logo is displayed, setup timing
  if (Current == 0 && !EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Logo displayed, setting up timing\n"));
    SetupLogoTiming ();
  }
  
  return Status;
}

EDKII_PLATFORM_LOGO_PROTOCOL  mPlatformLogo = {
  GetImage
};


**/

/**
  Cleanup function called when driver is unloaded.

  @param ImageHandle    The handle of the image being unloaded.

  @retval EFI_SUCCESS   The image was unloaded successfully.
**/

/**
EFI_STATUS
EFIAPI
LogoDriverUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
  // Close events if they exist
  if (mTimerEvent != NULL) {
    gBS->CloseEvent (mTimerEvent);
    mTimerEvent = NULL;
  }
  
  if (mKeyMonitorEvent != NULL) {
    gBS->CloseEvent (mKeyMonitorEvent);
    mKeyMonitorEvent = NULL;
  }
  
  if (mBiosSetupKeyEvent != NULL) {
    gBS->CloseEvent (mBiosSetupKeyEvent);
    mBiosSetupKeyEvent = NULL;
  }
  
  return EFI_SUCCESS;
}

**/

/**
  Entrypoint of this module.

  This function is the entrypoint of this module. It installs the Edkii
  Platform Logo protocol.

  @param  ImageHandle       The firmware allocated handle for the EFI image.
  @param  SystemTable       A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
**/

/**

EFI_STATUS
EFIAPI
InitializeLogo (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                   Status;
  EFI_HII_PACKAGE_LIST_HEADER  *PackageList;
  EFI_HII_DATABASE_PROTOCOL    *HiiDatabase;
  EFI_HANDLE                   Handle;

  DEBUG ((DEBUG_INFO, "Logo Driver: Initializing...\n"));

  // Initialize global variables
  mInBiosSetup = FALSE;
  mSelectedMenuItem = 0;
  mTotalMenuItems = ARRAY_SIZE (mBiosSetupMenu);

  // Locate HII Database Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiDatabaseProtocolGuid,
                  NULL,
                  (VOID **)&HiiDatabase
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Database Protocol: %r\n", Status));
    return Status;
  }

  // Locate HII Image Ex Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiImageExProtocolGuid,
                  NULL,
                  (VOID **)&mHiiImageEx
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Image Ex Protocol: %r\n", Status));
    return Status;
  }

  // Retrieve HII package list from ImageHandle
  Status = gBS->OpenProtocol (
                  ImageHandle,
                  &gEfiHiiPackageListProtocolGuid,
                  (VOID **)&PackageList,
                  ImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "HII Image Package with logo not found in PE/COFF resource section\n"));
    return Status;
  }

  // Publish HII package list to HII Database
  Status = HiiDatabase->NewPackageList (
                          HiiDatabase,
                          PackageList,
                          NULL,
                          &mHiiHandle
                          );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to add package list to HII Database: %r\n", Status));
    return Status;
  }

  // Install Platform Logo Protocol
  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEdkiiPlatformLogoProtocolGuid,
                  &mPlatformLogo,
                  NULL
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to install Platform Logo Protocol: %r\n", Status));
    // Cleanup HII handle
    HiiDatabase->RemovePackageList (HiiDatabase, mHiiHandle);
    return Status;
  }

  DEBUG ((DEBUG_INFO, "Logo Driver: Initialized successfully\n"));
  
  return EFI_SUCCESS;
}


**/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



// BIOS Setup Utility with Mouse Support
// Modified to include mouse navigation and double-click functionality

/** @file
  Logo DXE Driver with F12 key BIOS setup functionality and Mouse Support.
  Shows logo, waits for F12 key, then displays BIOS setup form with navigation using both keyboard and mouse.

Copyright (c) 2016 - 2017, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent
**/

/**

#include <Uefi.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/HiiImageEx.h>
#include <Protocol/PlatformLogo.h>
#include <Protocol/HiiPackageList.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/SimpleTextOut.h>
#include <Protocol/SimplePointer.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>

typedef struct {
  EFI_IMAGE_ID                             ImageId;
  EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE    Attribute;
  INTN                                     OffsetX;
  INTN                                     OffsetY;
} LOGO_ENTRY;

// Menu item structure
typedef struct {
  CHAR16  *MenuText;
  CHAR16  *Description;
  UINTN   StartRow;
  UINTN   EndRow;
  UINTN   StartCol;
  UINTN   EndCol;
} MENU_ITEM;

// Mouse state structure
typedef struct {
  INT32   X;
  INT32   Y;
  BOOLEAN LeftButton;
  BOOLEAN RightButton;
  BOOLEAN PreviousLeftButton;
  UINT64  LastClickTime;
  UINTN   ClickCount;
} MOUSE_STATE;

// Global variables
EFI_HII_IMAGE_EX_PROTOCOL    *mHiiImageEx;
EFI_HII_HANDLE               mHiiHandle;
EFI_EVENT                    mKeyMonitorEvent;
EFI_EVENT                    mTimerEvent;
EFI_EVENT                    mBiosSetupKeyEvent;
EFI_EVENT                    mMouseEvent;
EFI_SIMPLE_POINTER_PROTOCOL  *mSimplePointer;
MOUSE_STATE                  mMouseState;
BOOLEAN                      mLogoDisplayed = FALSE;
BOOLEAN                      mMessageShown = FALSE;
BOOLEAN                      mInBiosSetup = FALSE;
BOOLEAN                      mMouseAvailable = FALSE;
UINTN                        mTimerCounter = 0;
UINTN                        mSelectedMenuItem = 0;
UINTN                        mTotalMenuItems = 0;
UINTN                        mConsoleWidth = 80;
UINTN                        mConsoleHeight = 25;

// Menu items for BIOS setup - Updated with mouse coordinates
MENU_ITEM mBiosSetupMenu[] = {
  { L"Boot Manager", L"Configure boot device priority", 10, 10, 10, 40 },
  { L"Device Manager", L"Configure hardware devices", 11, 11, 10, 40 },
  { L"Boot Maintenance Manager", L"Advanced boot options", 12, 12, 10, 40 },
  { L"Save & Exit", L"Save changes and exit", 13, 13, 10, 40 },
  { L"Exit Without Saving", L"Exit without saving changes", 14, 14, 10, 40 }
};

// Function declarations
VOID DisplayBiosSetupForm (VOID);
VOID GetCurrentTime (OUT CHAR16 *TimeString);
VOID HandleBiosSetupNavigation (VOID);
VOID DisplaySelectedMenuInfo (VOID);
VOID HandleMenuSelection (VOID);
EFI_STATUS InitializeMouse (VOID);
VOID HandleMouseInput (VOID);
UINTN GetMenuItemFromMousePosition (INT32 X, INT32 Y);
VOID UpdateMenuSelection (UINTN NewSelection);

LOGO_ENTRY mLogos[] = {
  {
    IMAGE_TOKEN (IMG_LOGO),
    EdkiiPlatformLogoDisplayAttributeCenter,
    0,
    0
  }
};

**/

/**
  Initialize mouse support.
  
  @retval EFI_SUCCESS   Mouse initialized successfully
  @retval Others        Error occurred
**/
/**

EFI_STATUS
InitializeMouse (
  VOID
  )
{
  EFI_STATUS Status;
  
  // Locate Simple Pointer Protocol (Mouse)
  Status = gBS->LocateProtocol (
                  &gEfiSimplePointerProtocolGuid,
                  NULL,
                  (VOID **)&mSimplePointer
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Mouse not available: %r\n", Status));
    mMouseAvailable = FALSE;
    return Status;
  }
  
  // Reset mouse
  Status = mSimplePointer->Reset (mSimplePointer, FALSE);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "Failed to reset mouse: %r\n", Status));
  }
  
  // Initialize mouse state
  mMouseState.X = mConsoleWidth / 2;
  mMouseState.Y = mConsoleHeight / 2;
  mMouseState.LeftButton = FALSE;
  mMouseState.RightButton = FALSE;
  mMouseState.PreviousLeftButton = FALSE;
  mMouseState.LastClickTime = 0;
  mMouseState.ClickCount = 0;
  
  mMouseAvailable = TRUE;
  DEBUG ((DEBUG_INFO, "Mouse initialized successfully\n"));
  
  return EFI_SUCCESS;
}

**/

/**
  Get menu item index from mouse position.
  
  @param X    Mouse X coordinate
  @param Y    Mouse Y coordinate
  
  @return     Menu item index, or MAX_UINTN if not over any menu item
**/
/**

UINTN
GetMenuItemFromMousePosition (
  INT32 X,
  INT32 Y
  )
{
  UINTN Index;
  
  // Convert pixel coordinates to character coordinates (approximate)
  UINTN CharX = (UINTN)(X / 8);  // Assuming 8 pixels per character width
  UINTN CharY = (UINTN)(Y / 16); // Assuming 16 pixels per character height
  
  // Check each menu item
  for (Index = 0; Index < mTotalMenuItems; Index++) {
    if (CharY == mBiosSetupMenu[Index].StartRow &&
        CharX >= mBiosSetupMenu[Index].StartCol &&
        CharX <= mBiosSetupMenu[Index].EndCol) {
      return Index;
    }
  }
  
  return MAX_UINTN; // Not over any menu item
}


**/

/**
  Update menu selection and refresh display.
  
  @param NewSelection   New menu item to select
**/
/**

VOID
UpdateMenuSelection (
  UINTN NewSelection
  )
{
  UINTN PreviousSelection;
  
  if (NewSelection >= mTotalMenuItems) {
    return;
  }
  
  PreviousSelection = mSelectedMenuItem;
  mSelectedMenuItem = NewSelection;
  
  if (PreviousSelection != mSelectedMenuItem) {
    // Update previous item display (remove highlight)
    gST->ConOut->SetCursorPosition (gST->ConOut, 10, 10 + PreviousSelection);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    Print (L"   %s", mBiosSetupMenu[PreviousSelection].MenuText);
    
    // Update current item display (add highlight)
    gST->ConOut->SetCursorPosition (gST->ConOut, 10, 10 + mSelectedMenuItem);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
    Print (L"-> %s", mBiosSetupMenu[mSelectedMenuItem].MenuText);
    
    // Update selected item information
    DisplaySelectedMenuInfo ();
  }
}

**/

/**
  Handle mouse input and update selection.
**/

/**
VOID
HandleMouseInput (
  VOID
  )
{
  EFI_STATUS                     Status;
  EFI_SIMPLE_POINTER_STATE       MouseState;
  UINTN                          MenuIndex;
  UINT64                         CurrentTime;
  static UINT64                  DoubleClickTimeout = 5000000; // 500ms in 100ns units
  
  if (!mMouseAvailable || !mInBiosSetup) {
    return;
  }
  
  Status = mSimplePointer->GetState (mSimplePointer, &MouseState);
  if (EFI_ERROR (Status)) {
    return;
  }
  
  // Update mouse position (relative movement)
  mMouseState.X += MouseState.RelativeMovementX / 1000; // Scale down movement
  mMouseState.Y += MouseState.RelativeMovementY / 1000;
  
  // Clamp to screen boundaries
  if (mMouseState.X < 0) mMouseState.X = 0;
  if (mMouseState.X >= (INT32)mConsoleWidth * 8) mMouseState.X = (INT32)mConsoleWidth * 8 - 1;
  if (mMouseState.Y < 0) mMouseState.Y = 0;
  if (mMouseState.Y >= (INT32)mConsoleHeight * 16) mMouseState.Y = (INT32)mConsoleHeight * 16 - 1;
  
  // Store previous button state
  mMouseState.PreviousLeftButton = mMouseState.LeftButton;
  mMouseState.LeftButton = MouseState.LeftButton;
  mMouseState.RightButton = MouseState.RightButton;
  
  // Handle mouse movement - highlight menu item under cursor
  MenuIndex = GetMenuItemFromMousePosition (mMouseState.X, mMouseState.Y);
  if (MenuIndex != MAX_UINTN) {
    UpdateMenuSelection (MenuIndex);
  }
  
  // Handle left button click
  if (mMouseState.LeftButton && !mMouseState.PreviousLeftButton) {
    // Button just pressed
    Status = gRT->GetTime (NULL, NULL);
    CurrentTime = 0; // Simplified - in real implementation, use proper time
    
    if (MenuIndex != MAX_UINTN) {
      // Check for double-click
      if ((CurrentTime - mMouseState.LastClickTime) < DoubleClickTimeout) {
        mMouseState.ClickCount++;
        if (mMouseState.ClickCount >= 2) {
          // Double-click detected - execute menu selection
          DEBUG ((DEBUG_INFO, "Mouse double-click detected on menu item %d\n", MenuIndex));
          UpdateMenuSelection (MenuIndex);
          HandleMenuSelection ();
          mMouseState.ClickCount = 0;
        }
      } else {
        // Single click - just select the item
        mMouseState.ClickCount = 1;
        UpdateMenuSelection (MenuIndex);
      }
      
      mMouseState.LastClickTime = CurrentTime;
    }
  }
}


**/
/**
  Get current time and format it as string.

  @param TimeString   Output buffer for formatted time string.
**/
/**

VOID
GetCurrentTime (
  OUT CHAR16  *TimeString
  )
{
  EFI_TIME    Time;
  EFI_STATUS  Status;
  
  Status = gRT->GetTime (&Time, NULL);
  if (!EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "GetCurrentTime: Successfully retrieved time - %02d:%02d:%02d\n", Time.Hour, Time.Minute, Time.Second));
    UnicodeSPrint (
      TimeString, 
      50 * sizeof(CHAR16),
      L"%02d:%02d:%02d",
      Time.Hour,
      Time.Minute,
      Time.Second
    );
  } else {
    DEBUG ((DEBUG_ERROR, "GetCurrentTime: Failed to get time, Status: %r\n", Status));
    UnicodeSPrint (TimeString, 50 * sizeof(CHAR16), L"Time Unavailable");
  }
}

**/

/**
  Display information about the selected menu item.
**/
/**

VOID
DisplaySelectedMenuInfo (
  VOID
  )
{
  // Clear the info area
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 12);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"                                    ");
  
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 13);
  Print (L"                                    ");
  
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 14);
  Print (L"                                    ");
  
  // Display selected item info
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 12);
  Print (L"Selected: %s", mBiosSetupMenu[mSelectedMenuItem].MenuText);
  
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 14);
  Print (L"%s", mBiosSetupMenu[mSelectedMenuItem].Description);
}

**/

/**
  Display BIOS setup form with white background, menu items and navigation.
**/

/**
VOID
DisplayBiosSetupForm (
  VOID
  )
{
  UINTN   Index;
  CHAR16  TimeString[50];
  
  mTotalMenuItems = ARRAY_SIZE (mBiosSetupMenu);
  mSelectedMenuItem = 0;
  mInBiosSetup = TRUE;
  
  // Initialize mouse support
  InitializeMouse ();
  
  // Clear screen
  gST->ConOut->ClearScreen (gST->ConOut);
  
  // Set white background with black text
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  
  // Fill entire screen with white background
  for (Index = 0; Index < 25; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Index);
    Print (L"                                                                                ");
  }
  
  // Display "CDAC Bangalore" at top center
  gST->ConOut->SetCursorPosition (gST->ConOut, 30, 2);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"CDAC Bangalore");
  
  // Get and display current time at top right corner
  GetCurrentTime (TimeString);
  gST->ConOut->SetCursorPosition (gST->ConOut, 65, 2);
  Print (L"%s", TimeString);
  
  // Add main content area
  gST->ConOut->SetCursorPosition (gST->ConOut, 25, 5);
  Print (L"====================================");
  gST->ConOut->SetCursorPosition (gST->ConOut, 30, 6);
  Print (L"BIOS SETUP UTILITY");
  gST->ConOut->SetCursorPosition (gST->ConOut, 25, 7);
  Print (L"====================================");
  
  // Display menu items
  for (Index = 0; Index < mTotalMenuItems; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 10, 10 + Index);
    
    if (Index == mSelectedMenuItem) {
      // Highlight selected item with reverse colors
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
      Print (L"-> %s", mBiosSetupMenu[Index].MenuText);
    } else {
      // Normal menu item
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"   %s", mBiosSetupMenu[Index].MenuText);
    }
  }
  
  // Display selected item information
  DisplaySelectedMenuInfo ();
  
  // Show navigation instructions (updated for mouse)
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  gST->ConOut->SetCursorPosition (gST->ConOut, 10, 20);
  Print (L"Navigation Instructions:");
  gST->ConOut->SetCursorPosition (gST->ConOut, 10, 21);
  Print (L"Use UP/DOWN arrow keys or MOUSE to navigate");
  gST->ConOut->SetCursorPosition (gST->ConOut, 10, 22);
  Print (L"Press ENTER or DOUBLE-CLICK to select");
  gST->ConOut->SetCursorPosition (gST->ConOut, 10, 23);
  Print (L"Press ESC to exit setup");
  
  // Add footer
  gST->ConOut->SetCursorPosition (gST->ConOut, 20, 24);
  Print (L"BIOS Setup - F12 Key Activated - Mouse Enabled - CDAC Bangalore");
  
  // Reset console input for clean key detection in setup
  gST->ConIn->Reset (gST->ConIn, FALSE);
  
  // Start navigation handling
  HandleBiosSetupNavigation ();
}


**/
/**
  Handle menu selection based on current selected item.
  Modified to shutdown system instead of trying to continue boot.
**/

/**
VOID
HandleMenuSelection (
  VOID
  )
{
  switch (mSelectedMenuItem) {
    case 0: // Boot Manager
      gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
      Print (L"Boot Manager!");
      break;
      
    case 1: // Device Manager  
      gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
      Print (L"Device Manager!");
      break;
      
    case 2: // Boot Maintenance Manager
      gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
      Print (L"Boot Maintenance Manager!");
      break;
    
    case 3: // Save & Exit
      gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_GREEN | EFI_BACKGROUND_BLUE);
      Print (L"Saving and exiting...");
      gBS->Stall (2000000); // Wait 2 seconds
      
      // Show shutdown message
      gST->ConOut->ClearScreen (gST->ConOut);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
      gST->ConOut->SetCursorPosition (gST->ConOut, 25, 12);
      Print (L"Settings saved. Shutting down system...");
      gBS->Stall (2000000); // Wait 2 seconds
      
      // Shutdown the system instead of continuing boot
      gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
      
      mInBiosSetup = FALSE;
      break;
      
    case 4: // Exit Without Saving
      gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_RED | EFI_BACKGROUND_BLUE);
      Print (L"Exiting without saving...");
      gBS->Stall (2000000); // Wait 2 seconds
      
      // Show shutdown message
      gST->ConOut->ClearScreen (gST->ConOut);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
      gST->ConOut->SetCursorPosition (gST->ConOut, 25, 12);
      Print (L"Exiting setup. Shutting down system...");
      gBS->Stall (2000000); // Wait 2 seconds
      
      // Shutdown the system instead of continuing boot  
      gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
      
      mInBiosSetup = FALSE;
      break;
      
    default:
      break;
  }
  
  // If not exiting, continue showing the menu
  if (mInBiosSetup) {
    gBS->Stall (1000000); // Wait 1 second to show selection message
    DisplayBiosSetupForm (); // Refresh the form
  }
}

**/

/**
  Handle navigation within BIOS setup using arrow keys and mouse.
  Modified to shutdown on ESC instead of continuing boot.
**/

/**
VOID
HandleBiosSetupNavigation (
  VOID
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  UINTN          PreviousSelection;
  
  while (mInBiosSetup) {
    // Handle mouse input first
    HandleMouseInput ();
    
    // Wait for key input
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
    
    if (!EFI_ERROR (Status)) {
      PreviousSelection = mSelectedMenuItem;
      
      switch (Key.ScanCode) {
        case SCAN_UP:
          // Move up in menu
          if (mSelectedMenuItem > 0) {
            mSelectedMenuItem--;
          } else {
            mSelectedMenuItem = mTotalMenuItems - 1; // Wrap to bottom
          }
          break;
          
        case SCAN_DOWN:
          // Move down in menu
          if (mSelectedMenuItem < mTotalMenuItems - 1) {
            mSelectedMenuItem++;
          } else {
            mSelectedMenuItem = 0; // Wrap to top
          }
          break;
          
        case SCAN_ESC:
          // Exit BIOS setup with shutdown
          mInBiosSetup = FALSE;
          gST->ConOut->ClearScreen (gST->ConOut);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
          gST->ConOut->SetCursorPosition (gST->ConOut, 25, 12);
          Print (L"Exiting BIOS Setup. Shutting down system...");
          gBS->Stall (2000000); // Wait 2 seconds
          
          // Shutdown instead of returning to boot process
          gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
          return;
          
        default:
          // Check for ENTER key (Unicode character)
          if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
            HandleMenuSelection ();
            if (!mInBiosSetup) {
              return; // Exit if user chose to exit (will shutdown in HandleMenuSelection)
            }
          }
          break;
      }
      
      // Update display if selection changed via keyboard
      if (mInBiosSetup && (PreviousSelection != mSelectedMenuItem)) {
        UpdateMenuSelection (mSelectedMenuItem);
      }
    } else {
      // Small delay to prevent excessive CPU usage
      gBS->Stall (50000); // 50ms
    }
  }
}

**/
// Rest of the functions remain the same as in original code...
// (KeyboardMonitor, TimerCallback, SetupLogoTiming, GetImage, LogoDriverUnload, InitializeLogo)

/**
  Monitor keyboard input for F12 key press only.
  Modified to shutdown after BIOS setup completion.
**/

/**
VOID
EFIAPI
KeyboardMonitor (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  
  // Only monitor after message is shown and not in BIOS setup
  if (!mMessageShown || mInBiosSetup) {
    return;
  }
  
  // Check if key is available
  Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
  
  if (!EFI_ERROR (Status)) {
    // Check for F12 key only
    if (Key.ScanCode == SCAN_F12) {
      DEBUG ((DEBUG_INFO, "F12 key pressed - showing BIOS setup form\n"));
      
      // Cancel timer and keyboard events to stop normal flow
      gBS->SetTimer (mKeyMonitorEvent, TimerCancel, 0);
      gBS->SetTimer (mTimerEvent, TimerCancel, 0);
      
      // Display BIOS setup form
      DisplayBiosSetupForm ();
      
      // After exiting BIOS setup, shutdown
      gST->ConOut->ClearScreen (gST->ConOut);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
      gST->ConOut->SetCursorPosition (gST->ConOut, 25, 12);
      Print (L"BIOS Setup completed. Shutting down system...");
      gBS->Stall (2000000); // Wait 2 seconds
      
      // Shutdown the system
      gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
      
      return;
    }
  }
}

**/

/**
  Timer callback function to handle logo display timing.
**/

/**
VOID
EFIAPI
TimerCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  mTimerCounter++;
  
  // After 5 seconds, show the message below logo and wait for F12
  if (mTimerCounter == 50 && !mMessageShown) { // 50 * 100ms = 5 seconds
    // Clear the entire screen first
    gST->ConOut->ClearScreen (gST->ConOut);
    
    // Position cursor below the logo area
    gST->ConOut->SetCursorPosition (gST->ConOut, 18, 18);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
    Print (L"Press F12 key for BIOS setup");
    
    mMessageShown = TRUE;
    
    // Reset console input for clean key detection
    gST->ConIn->Reset (gST->ConIn, FALSE);
    
    DEBUG ((DEBUG_INFO, "Message displayed, waiting for F12 key\n"));
  }
}

**/

/**
  Setup timing and keyboard monitoring after logo is displayed.
**/

/**
VOID
SetupLogoTiming (
  VOID
  )
{
  EFI_STATUS  Status;
  
  if (mLogoDisplayed) {
    return; // Already setup
  }
  
  DEBUG ((DEBUG_INFO, "Setting up logo timing and keyboard monitoring\n"));
  
  // Create timer event for logo timing
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  TimerCallback,
                  NULL,
                  &mTimerEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    Status = gBS->SetTimer (
                    mTimerEvent,
                    TimerPeriodic,
                    1000000  // 100ms in 100ns units
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set timer for mTimerEvent: %r\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to create mTimerEvent: %r\n", Status));
  }
  
  // Create keyboard monitoring event
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  KeyboardMonitor,
                  NULL,
                  &mKeyMonitorEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    Status = gBS->SetTimer (
                    mKeyMonitorEvent,
                    TimerPeriodic,
                    500000  // 50ms in 100ns units
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set timer for mKeyMonitorEvent: %r\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to create mKeyMonitorEvent: %r\n", Status));
  }
  
  mLogoDisplayed = TRUE;
}

**/

/**
  Load a platform logo image and return its data and attributes.
**/

/**
EFI_STATUS
EFIAPI
GetImage (
  IN     EDKII_PLATFORM_LOGO_PROTOCOL        *This,
  IN OUT UINT32                              *Instance,
  OUT EFI_IMAGE_INPUT                        *Image,
  OUT EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE  *Attribute,
  OUT INTN                                   *OffsetX,
  OUT INTN                                   *OffsetY
  )
{
  UINT32      Current;
  EFI_STATUS  Status;

  if ((Instance == NULL) || (Image == NULL) ||
      (Attribute == NULL) || (OffsetX == NULL) || (OffsetY == NULL))
  {
    return EFI_INVALID_PARAMETER;
  }

  Current = *Instance;
  if (Current >= ARRAY_SIZE (mLogos)) {
    return EFI_NOT_FOUND;
  }

  (*Instance)++;
  *Attribute = mLogos[Current].Attribute;
  *OffsetX   = mLogos[Current].OffsetX;
  *OffsetY   = mLogos[Current].OffsetY;
  
  // Get image from HII database
  Status = mHiiImageEx->GetImageEx (mHiiImageEx, mHiiHandle, mLogos[Current].ImageId, Image);
  
  // After first logo is displayed, setup timing
  if (Current == 0 && !EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Logo displayed, setting up timing\n"));
    SetupLogoTiming ();
  }
  
  return Status;
}

EDKII_PLATFORM_LOGO_PROTOCOL  mPlatformLogo = {
  GetImage
};

**/

/**
  Cleanup function called when driver is unloaded.

  @param ImageHandle    The handle of the image being unloaded.

  @retval EFI_SUCCESS   The image was unloaded successfully.
**/

/**

EFI_STATUS
EFIAPI
LogoDriverUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
  // Close events if they exist
  if (mTimerEvent != NULL) {
    gBS->CloseEvent (mTimerEvent);
    mTimerEvent = NULL;
  }
  
  if (mKeyMonitorEvent != NULL) {
    gBS->CloseEvent (mKeyMonitorEvent);
    mKeyMonitorEvent = NULL;
  }
  
  if (mBiosSetupKeyEvent != NULL) {
    gBS->CloseEvent (mBiosSetupKeyEvent);
    mBiosSetupKeyEvent = NULL;
  }
  
  if (mMouseEvent != NULL) {
    gBS->CloseEvent (mMouseEvent);
    mMouseEvent = NULL;
  }
  
  return EFI_SUCCESS;
}

**/

/**
  Entrypoint of this module.

  This function is the entrypoint of this module. It installs the Edkii
  Platform Logo protocol.

  @param  ImageHandle       The firmware allocated handle for the EFI image.
  @param  SystemTable       A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
**/

/**

EFI_STATUS
EFIAPI
InitializeLogo (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                   Status;
  EFI_HII_PACKAGE_LIST_HEADER  *PackageList;
  EFI_HII_DATABASE_PROTOCOL    *HiiDatabase;
  EFI_HANDLE                   Handle;

  DEBUG ((DEBUG_INFO, "Logo Driver: Initializing with Mouse Support...\n"));

  // Initialize global variables
  mInBiosSetup = FALSE;
  mSelectedMenuItem = 0;
  mTotalMenuItems = ARRAY_SIZE (mBiosSetupMenu);
  mMouseAvailable = FALSE;

  // Locate HII Database Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiDatabaseProtocolGuid,
                  NULL,
                  (VOID **)&HiiDatabase
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Database Protocol: %r\n", Status));
    return Status;
  }

  // Locate HII Image Ex Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiImageExProtocolGuid,
                  NULL,
                  (VOID **)&mHiiImageEx
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Image Ex Protocol: %r\n", Status));
    return Status;
  }

  // Retrieve HII package list from ImageHandle
  Status = gBS->OpenProtocol (
                  ImageHandle,
                  &gEfiHiiPackageListProtocolGuid,
                  (VOID **)&PackageList,
                  ImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "HII Image Package with logo not found in PE/COFF resource section\n"));
    return Status;
  }

  // Publish HII package list to HII Database
  Status = HiiDatabase->NewPackageList (
                          HiiDatabase,
                          PackageList,
                          NULL,
                          &mHiiHandle
                          );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to add package list to HII Database: %r\n", Status));
    return Status;
  }

  // Install Platform Logo Protocol
  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEdkiiPlatformLogoProtocolGuid,
                  &mPlatformLogo,
                  NULL
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to install Platform Logo Protocol: %r\n", Status));
    // Cleanup HII handle
    HiiDatabase->RemovePackageList (HiiDatabase, mHiiHandle);
    return Status;
  }

  DEBUG ((DEBUG_INFO, "Logo Driver: Initialized successfully with Mouse Support\n"));
  
  return EFI_SUCCESS;
}

**/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// BIOS Setup Utility with  Mouse and Keyboard Support


/** @file
  Logo DXE Driver with F12 key BIOS setup functionality and Mouse Support.
  Shows logo, waits for F12 key, then displays BIOS setup form with navigation using both keyboard and mouse.

Copyright (c) 2016 - 2017, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent
**/

/**

#include <Uefi.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/HiiImageEx.h>
#include <Protocol/PlatformLogo.h>
#include <Protocol/HiiPackageList.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/SimpleTextOut.h>
#include <Protocol/SimplePointer.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>

typedef struct {
  EFI_IMAGE_ID                             ImageId;
  EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE    Attribute;
  INTN                                     OffsetX;
  INTN                                     OffsetY;
} LOGO_ENTRY;

// Menu item structure
typedef struct {
  CHAR16  *MenuText;
  CHAR16  *Description;
  UINTN   MenuRow;      // Row where menu item is displayed
  UINTN   MenuCol;      // Column where menu item starts
  UINTN   MenuLength;   // Length of menu text
} MENU_ITEM;

// Mouse state structure
typedef struct {
  BOOLEAN LeftButton;
  BOOLEAN RightButton;
  BOOLEAN PreviousLeftButton;
  UINT64  LastClickTime;
  UINTN   ClickCount;
  INT32   CurrentX;
  INT32   CurrentY;
} MOUSE_STATE;

// Global variables
EFI_HII_IMAGE_EX_PROTOCOL    *mHiiImageEx;
EFI_HII_HANDLE               mHiiHandle;
EFI_EVENT                    mKeyMonitorEvent;
EFI_EVENT                    mTimerEvent;
EFI_EVENT                    mBiosSetupKeyEvent;
EFI_SIMPLE_POINTER_PROTOCOL  *mSimplePointer;
MOUSE_STATE                  mMouseState;
BOOLEAN                      mLogoDisplayed = FALSE;
BOOLEAN                      mMessageShown = FALSE;
BOOLEAN                      mInBiosSetup = FALSE;
BOOLEAN                      mMouseAvailable = FALSE;
UINTN                        mTimerCounter = 0;
UINTN                        mSelectedMenuItem = 0;
UINTN                        mTotalMenuItems = 0;

// Menu items for BIOS setup - Fixed structure with proper coordinates
MENU_ITEM mBiosSetupMenu[] = {
  { L"Boot Manager", L"Configure boot device priority", 10, 13, 12 },
  { L"Device Manager", L"Configure hardware devices", 11, 13, 14 },
  { L"Boot Maintenance Manager", L"Advanced boot options", 12, 13, 25 },
  { L"Save & Exit", L"Save changes and exit", 13, 13, 11 },
  { L"Exit Without Saving", L"Exit without saving changes", 14, 13, 19 }
};

LOGO_ENTRY mLogos[] = {
  {
    IMAGE_TOKEN (IMG_LOGO),
    EdkiiPlatformLogoDisplayAttributeCenter,
    0,
    0
  }
};

// Function declarations
VOID DisplayBiosSetupForm (VOID);
VOID GetCurrentTime (OUT CHAR16 *TimeString);
VOID HandleBiosSetupNavigation (VOID);
VOID DisplaySelectedMenuInfo (VOID);
VOID HandleMenuSelection (VOID);
EFI_STATUS InitializeMouse (VOID);
UINTN GetMenuItemFromMousePosition (UINTN Row, UINTN Col);
VOID UpdateMenuHighlight (VOID);

**/

/**
  Initialize mouse support.
  
  @retval EFI_SUCCESS   Mouse initialized successfully
  @retval Others        Error occurred
**/

/**
EFI_STATUS
InitializeMouse (
  VOID
  )
{
  EFI_STATUS Status;
  
  // Try to locate Simple Pointer Protocol (Mouse)
  Status = gBS->LocateProtocol (
                  &gEfiSimplePointerProtocolGuid,
                  NULL,
                  (VOID **)&mSimplePointer
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Mouse not available: %r\n", Status));
    mMouseAvailable = FALSE;
    return Status;
  }
  
  // Reset mouse to known state
  Status = mSimplePointer->Reset (mSimplePointer, FALSE);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "Failed to reset mouse: %r\n", Status));
  }
  
  // Initialize mouse state
  mMouseState.LeftButton = FALSE;
  mMouseState.RightButton = FALSE;
  mMouseState.PreviousLeftButton = FALSE;
  mMouseState.LastClickTime = 0;
  mMouseState.ClickCount = 0;
  mMouseState.CurrentX = 40;  // Center of screen
  mMouseState.CurrentY = 12;  // Middle of menu area
  
  mMouseAvailable = TRUE;
  DEBUG ((DEBUG_INFO, "Mouse initialized successfully\n"));
  
  return EFI_SUCCESS;
}

**/

/**
  Get menu item index from mouse position (row/column based).
  
  @param Row    Current cursor row
  @param Col    Current cursor column
  
  @return       Menu item index, or MAX_UINTN if not over any menu item
**/

/**
UINTN
GetMenuItemFromMousePosition (
  UINTN Row,
  UINTN Col
  )
{
  UINTN Index;
  
  // Check each menu item
  for (Index = 0; Index < mTotalMenuItems; Index++) {
    // Check if cursor is on the menu item row and within its text area
    if (Row == mBiosSetupMenu[Index].MenuRow &&
        Col >= mBiosSetupMenu[Index].MenuCol &&
        Col <= (mBiosSetupMenu[Index].MenuCol + mBiosSetupMenu[Index].MenuLength)) {
      return Index;
    }
  }
  
  return MAX_UINTN; // Not over any menu item
}

**/
/**
  Update menu item highlighting based on current selection.
**/

/**
VOID
UpdateMenuHighlight (
  VOID
  )
{
  UINTN Index;
  
  // Redraw all menu items with proper highlighting
  for (Index = 0; Index < mTotalMenuItems; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 10, mBiosSetupMenu[Index].MenuRow);
    
    if (Index == mSelectedMenuItem) {
      // Highlight selected item
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
      Print (L"-> %s                    ", mBiosSetupMenu[Index].MenuText);
    } else {
      // Normal menu item
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"   %s                    ", mBiosSetupMenu[Index].MenuText);
    }
  }
  
  // Update selected item information
  DisplaySelectedMenuInfo ();
}

**/

/**
  Get current time and format it as string.

  @param TimeString   Output buffer for formatted time string.
**/

/**
VOID
GetCurrentTime (
  OUT CHAR16  *TimeString
  )
{
  EFI_TIME    Time;
  EFI_STATUS  Status;
  
  Status = gRT->GetTime (&Time, NULL);
  if (!EFI_ERROR (Status)) {
    UnicodeSPrint (
      TimeString, 
      50 * sizeof(CHAR16),
      L"%02d:%02d:%02d",
      Time.Hour,
      Time.Minute,
      Time.Second
    );
  } else {
    UnicodeSPrint (TimeString, 50 * sizeof(CHAR16), L"--:--:--");
  }
}


**/
/**
  Display information about the selected menu item.
**/

/**
VOID
DisplaySelectedMenuInfo (
  VOID
  )
{
  // Clear the info area
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 12);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"                                    ");
  
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 13);
  Print (L"                                    ");
  
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 14);
  Print (L"                                    ");
  
  // Display selected item info
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 12);
  Print (L"Selected: %s", mBiosSetupMenu[mSelectedMenuItem].MenuText);
  
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 14);
  Print (L"%s", mBiosSetupMenu[mSelectedMenuItem].Description);
}

**/

/**
  Display BIOS setup form with white background, menu items and navigation.
**/
/**

VOID
DisplayBiosSetupForm (
  VOID
  )
{
  UINTN   Index;
  CHAR16  TimeString[50];
  
  mTotalMenuItems = ARRAY_SIZE (mBiosSetupMenu);
  mSelectedMenuItem = 0;
  mInBiosSetup = TRUE;
  
  // Initialize mouse support
  InitializeMouse ();
  
  // Clear screen
  gST->ConOut->ClearScreen (gST->ConOut);
  
  // Set white background with black text
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  
  // Fill entire screen with white background
  for (Index = 0; Index < 25; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Index);
    Print (L"                                                                                ");
  }
  
  // Display "CDAC Bangalore" at top center
  gST->ConOut->SetCursorPosition (gST->ConOut, 30, 2);
  Print (L"CDAC Bangalore");
  
  // Get and display current time at top right corner
  GetCurrentTime (TimeString);
  gST->ConOut->SetCursorPosition (gST->ConOut, 65, 2);
  Print (L"%s", TimeString);
  
  // Add main content area
  gST->ConOut->SetCursorPosition (gST->ConOut, 25, 5);
  Print (L"====================================");
  gST->ConOut->SetCursorPosition (gST->ConOut, 30, 6);
  Print (L"BIOS SETUP UTILITY");
  gST->ConOut->SetCursorPosition (gST->ConOut, 25, 7);
  Print (L"====================================");
  
  // Display menu items using UpdateMenuHighlight
  UpdateMenuHighlight ();
  
  // Show navigation instructions
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  gST->ConOut->SetCursorPosition (gST->ConOut, 10, 17);
  Print (L"Navigation Instructions:");
  gST->ConOut->SetCursorPosition (gST->ConOut, 10, 18);
  Print (L"Use UP/DOWN arrow keys to navigate");
  gST->ConOut->SetCursorPosition (gST->ConOut, 10, 19);
  if (mMouseAvailable) {
    Print (L"Use MOUSE to hover and DOUBLE-CLICK to select");
  } else {
    Print (L"Mouse not available - Use keyboard only");
  }
  gST->ConOut->SetCursorPosition (gST->ConOut, 10, 20);
  Print (L"Press ENTER to select");
  gST->ConOut->SetCursorPosition (gST->ConOut, 10, 21);
  Print (L"Press ESC to exit setup");
  
  // Add footer
  gST->ConOut->SetCursorPosition (gST->ConOut, 15, 23);
  if (mMouseAvailable) {
    Print (L"BIOS Setup - F12 Activated - Mouse Enabled - CDAC Bangalore");
  } else {
    Print (L"BIOS Setup - F12 Activated - Keyboard Only - CDAC Bangalore");    
  }
  
  // Start navigation handling
  HandleBiosSetupNavigation ();
}


**/
/**
  Handle menu selection based on current selected item.
**/

/**
VOID
HandleMenuSelection (
  VOID
  )
{
  // Clear any previous selection message
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"                                    ");
  
  switch (mSelectedMenuItem) {
    case 0: // Boot Manager
      gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
      Print (L"Opening Boot Manager...");
      break;
      
    case 1: // Device Manager  
      gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
      Print (L"Opening Device Manager...");
      break;
      
    case 2: // Boot Maintenance Manager
      gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
      Print (L"Opening Boot Maintenance...");
      break;
    
    case 3: // Save & Exit
      gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_GREEN | EFI_BACKGROUND_BLUE);
      Print (L"Saving and exiting...");
      gBS->Stall (2000000); // Wait 2 seconds
      
      // Show shutdown message
      gST->ConOut->ClearScreen (gST->ConOut);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
      gST->ConOut->SetCursorPosition (gST->ConOut, 25, 12);
      Print (L"Settings saved. Shutting down system...");
      gBS->Stall (2000000); // Wait 2 seconds
      
      // Shutdown the system
      gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
      mInBiosSetup = FALSE;
      break;
      
    case 4: // Exit Without Saving
      gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_RED | EFI_BACKGROUND_BLUE);
      Print (L"Exiting without saving...");
      gBS->Stall (2000000); // Wait 2 seconds
      
      // Show shutdown message
      gST->ConOut->ClearScreen (gST->ConOut);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
      gST->ConOut->SetCursorPosition (gST->ConOut, 25, 12);
      Print (L"Exiting setup. Shutting down system...");
      gBS->Stall (2000000); // Wait 2 seconds
      
      // Shutdown the system
      gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
      mInBiosSetup = FALSE;
      break;
      
    default:
      break;
  }
  
  // If not exiting, wait a moment and refresh
  if (mInBiosSetup) {
    gBS->Stall (1500000); // Wait 1.5 seconds to show selection message
    // Clear the selection message
    gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    Print (L"                                    ");
  }
}


**/
/**
  Handle navigation within BIOS setup using arrow keys and mouse.
**/

/**
VOID
HandleBiosSetupNavigation (
  VOID
  )
{
  EFI_STATUS                Status;
  EFI_INPUT_KEY             Key;
  EFI_SIMPLE_POINTER_STATE  MouseState;
  UINTN                     PreviousSelection;
 // UINTN                     MouseMenuIndex;
  UINT64                    CurrentTime;
  static UINT64             DoubleClickTime = 5000000; // 500ms in 100ns units
  
  while (mInBiosSetup) {
    PreviousSelection = mSelectedMenuItem;
    
    // Handle mouse input if available
    if (mMouseAvailable) {
      Status = mSimplePointer->GetState (mSimplePointer, &MouseState);
      if (!EFI_ERROR (Status)) {
        // Store previous button state
        mMouseState.PreviousLeftButton = mMouseState.LeftButton;
        mMouseState.LeftButton = MouseState.LeftButton;
        mMouseState.RightButton = MouseState.RightButton;
        
        // Update cursor position based on relative movement
        // Convert mouse movement to console cursor movement
        if (MouseState.RelativeMovementY < -1000) {
          // Mouse moved up
          if (mSelectedMenuItem > 0) {
            mSelectedMenuItem--;
          } else {
            mSelectedMenuItem = mTotalMenuItems - 1; // Wrap to bottom
          }
        } else if (MouseState.RelativeMovementY > 1000) {
          // Mouse moved down
          if (mSelectedMenuItem < mTotalMenuItems - 1) {
            mSelectedMenuItem++;
          } else {
            mSelectedMenuItem = 0; // Wrap to top
          }
        }
        
        // Handle mouse click
        if (mMouseState.LeftButton && !mMouseState.PreviousLeftButton) {
          // Left button just pressed
          gRT->GetTime (NULL, NULL);
          CurrentTime = 0; // Simplified time handling
          
          // Check for double-click
          if ((CurrentTime - mMouseState.LastClickTime) < DoubleClickTime) {
            mMouseState.ClickCount++;
            if (mMouseState.ClickCount >= 2) {
              // Double-click detected - execute menu selection
              DEBUG ((DEBUG_INFO, "Mouse double-click detected\n"));
              HandleMenuSelection ();
              mMouseState.ClickCount = 0;
            }
          } else {
            // Single click
            mMouseState.ClickCount = 1;
          }
          
          mMouseState.LastClickTime = CurrentTime;
        }
      }
    }
    
    // Handle keyboard input
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
    
    if (!EFI_ERROR (Status)) {
      switch (Key.ScanCode) {
        case SCAN_UP:
          // Move up in menu
          if (mSelectedMenuItem > 0) {
            mSelectedMenuItem--;
          } else {
            mSelectedMenuItem = mTotalMenuItems - 1; // Wrap to bottom
          }
          DEBUG ((DEBUG_INFO, "UP key pressed, selected item: %d\n", mSelectedMenuItem));
          break;
          
        case SCAN_DOWN:
          // Move down in menu
          if (mSelectedMenuItem < mTotalMenuItems - 1) {
            mSelectedMenuItem++;
          } else {
            mSelectedMenuItem = 0; // Wrap to top
          }
          DEBUG ((DEBUG_INFO, "DOWN key pressed, selected item: %d\n", mSelectedMenuItem));
          break;
          
        case SCAN_ESC:
          // Exit BIOS setup with shutdown
          mInBiosSetup = FALSE;
          gST->ConOut->ClearScreen (gST->ConOut);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
          gST->ConOut->SetCursorPosition (gST->ConOut, 25, 12);
          Print (L"Exiting BIOS Setup. Shutting down system...");
          gBS->Stall (2000000); // Wait 2 seconds
          
          // Shutdown instead of returning to boot process
          gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
          return;
          
        default:
          // Check for ENTER key (Unicode character)
          if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
            DEBUG ((DEBUG_INFO, "ENTER key pressed\n"));
            HandleMenuSelection ();
            if (!mInBiosSetup) {
              return; // Exit if shutdown was triggered
            }
          }
          break;
      }
      
      // Update display if selection changed
      if (PreviousSelection != mSelectedMenuItem) {
        UpdateMenuHighlight ();
      }
    } else {
      // Small delay to prevent excessive CPU usage
      gBS->Stall (50000); // 50ms
    }
  }
}

**/

/**
  Monitor keyboard input for F12 key press only.
**/

/**
VOID
EFIAPI
KeyboardMonitor (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  
  // Only monitor after message is shown and not in BIOS setup
  if (!mMessageShown || mInBiosSetup) {
    return;
  }
  
  // Check if key is available
  Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
  
  if (!EFI_ERROR (Status)) {
    // Check for F12 key only
    if (Key.ScanCode == SCAN_F12) {
      DEBUG ((DEBUG_INFO, "F12 key pressed - showing BIOS setup form\n"));
      
      // Cancel timer and keyboard events to stop normal flow
      gBS->SetTimer (mKeyMonitorEvent, TimerCancel, 0);
      gBS->SetTimer (mTimerEvent, TimerCancel, 0);
      
      // Display BIOS setup form
      DisplayBiosSetupForm ();
      
      // After exiting BIOS setup, shutdown
      gST->ConOut->ClearScreen (gST->ConOut);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
      gST->ConOut->SetCursorPosition (gST->ConOut, 25, 12);
      Print (L"BIOS Setup completed. Shutting down system...");
      gBS->Stall (2000000); // Wait 2 seconds
      
      // Shutdown the system
      gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
      
      return;
    }
  }
}

**/

/**
  Timer callback function to handle logo display timing.
**/

/**
VOID
EFIAPI
TimerCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  mTimerCounter++;
  
  // After 5 seconds, show the message below logo and wait for F12
  if (mTimerCounter == 50 && !mMessageShown) { // 50 * 100ms = 5 seconds
    // Clear the entire screen first
    gST->ConOut->ClearScreen (gST->ConOut);
    
    // Position cursor below the logo area
    gST->ConOut->SetCursorPosition (gST->ConOut, 18, 18);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
    Print (L"Press F12 key for BIOS setup");
    
    mMessageShown = TRUE;
    
    // Reset console input for clean key detection
    gST->ConIn->Reset (gST->ConIn, FALSE);
    
    DEBUG ((DEBUG_INFO, "Message displayed, waiting for F12 key\n"));
  }
}

**/

/**
  Setup timing and keyboard monitoring after logo is displayed.
**/

/**
VOID
SetupLogoTiming (
  VOID
  )
{
  EFI_STATUS  Status;
  
  if (mLogoDisplayed) {
    return; // Already setup
  }
  
  DEBUG ((DEBUG_INFO, "Setting up logo timing and keyboard monitoring\n"));
  
  // Create timer event for logo timing
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  TimerCallback,
                  NULL,
                  &mTimerEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    Status = gBS->SetTimer (
                    mTimerEvent,
                    TimerPeriodic,
                    1000000  // 100ms in 100ns units
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set timer for mTimerEvent: %r\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to create mTimerEvent: %r\n", Status));
  }
  
  // Create keyboard monitoring event
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  KeyboardMonitor,
                  NULL,
                  &mKeyMonitorEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    Status = gBS->SetTimer (
                    mKeyMonitorEvent,
                    TimerPeriodic,
                    500000  // 50ms in 100ns units
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set timer for mKeyMonitorEvent: %r\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to create mKeyMonitorEvent: %r\n", Status));
  }
  
  mLogoDisplayed = TRUE;
}


**/
/**
  Load a platform logo image and return its data and attributes.
**/

/**
EFI_STATUS
EFIAPI
GetImage (
  IN     EDKII_PLATFORM_LOGO_PROTOCOL        *This,
  IN OUT UINT32                              *Instance,
  OUT EFI_IMAGE_INPUT                        *Image,
  OUT EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE  *Attribute,
  OUT INTN                                   *OffsetX,
  OUT INTN                                   *OffsetY
  )
{
  UINT32      Current;
  EFI_STATUS  Status;

  if ((Instance == NULL) || (Image == NULL) ||
      (Attribute == NULL) || (OffsetX == NULL) || (OffsetY == NULL))
  {
    return EFI_INVALID_PARAMETER;
  }

  Current = *Instance;
  if (Current >= ARRAY_SIZE (mLogos)) {
    return EFI_NOT_FOUND;
  }

  (*Instance)++;
  *Attribute = mLogos[Current].Attribute;
  *OffsetX   = mLogos[Current].OffsetX;
  *OffsetY   = mLogos[Current].OffsetY;
  
  // Get image from HII database
  Status = mHiiImageEx->GetImageEx (mHiiImageEx, mHiiHandle, mLogos[Current].ImageId, Image);
  
  // After first logo is displayed, setup timing
  if (Current == 0 && !EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Logo displayed, setting up timing\n"));
    SetupLogoTiming ();
  }
  
  return Status;
}

EDKII_PLATFORM_LOGO_PROTOCOL  mPlatformLogo = {
  GetImage
};


**/
/**
  Cleanup function called when driver is unloaded.
**/

/**
EFI_STATUS
EFIAPI
LogoDriverUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
  // Close events if they exist
  if (mTimerEvent != NULL) {
    gBS->CloseEvent (mTimerEvent);
    mTimerEvent = NULL;
  }
  
  if (mKeyMonitorEvent != NULL) {
    gBS->CloseEvent (mKeyMonitorEvent);
    mKeyMonitorEvent = NULL;
  }
  
  if (mBiosSetupKeyEvent != NULL) {
    gBS->CloseEvent (mBiosSetupKeyEvent);
    mBiosSetupKeyEvent = NULL;
  }
  
  return EFI_SUCCESS;
}


**/
/**
  Entrypoint of this module.
**/

/**
EFI_STATUS
EFIAPI
InitializeLogo (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                   Status;
  EFI_HII_PACKAGE_LIST_HEADER  *PackageList;
  EFI_HII_DATABASE_PROTOCOL    *HiiDatabase;
  EFI_HANDLE                   Handle;

  DEBUG ((DEBUG_INFO, "Logo Driver: Initializing with Mouse and Keyboard Support...\n"));

  // Initialize global variables
  mInBiosSetup = FALSE;
  mSelectedMenuItem = 0;
  mTotalMenuItems = ARRAY_SIZE (mBiosSetupMenu);
  mMouseAvailable = FALSE;

  // Locate HII Database Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiDatabaseProtocolGuid,
                  NULL,
                  (VOID **)&HiiDatabase
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Database Protocol: %r\n", Status));
    return Status;
  }

  // Locate HII Image Ex Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiImageExProtocolGuid,
                  NULL,
                  (VOID **)&mHiiImageEx
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Image Ex Protocol: %r\n", Status));
    return Status;
  }

  // Retrieve HII package list from ImageHandle
  Status = gBS->OpenProtocol (
                  ImageHandle,
                  &gEfiHiiPackageListProtocolGuid,
                  (VOID **)&PackageList,
                  ImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "HII Image Package with logo not found in PE/COFF resource section\n"));
    return Status;
  }

  // Publish HII package list to HII Database
  Status = HiiDatabase->NewPackageList (
                          HiiDatabase,
                          PackageList,
                          NULL,
                          &mHiiHandle
                          );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to add package list to HII Database: %r\n", Status));
    return Status;
  }

  // Install Platform Logo Protocol
  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEdkiiPlatformLogoProtocolGuid,
                  &mPlatformLogo,
                  NULL
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to install Platform Logo Protocol: %r\n", Status));
    // Cleanup HII handle
    HiiDatabase->RemovePackageList (HiiDatabase, mHiiHandle);
    return Status;
  }

  DEBUG ((DEBUG_INFO, "Logo Driver: Initialized successfully with Mouse and Keyboard Support\n"));
  
  return EFI_SUCCESS;
}

**/

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// remaining process work but mouse is not properly working.



/** @file
  Logo DXE Driver with F12 key BIOS setup functionality.
  Shows logo, waits for F12 key, then displays BIOS setup form with navigation.
  Enhanced with mouse support for menu selection.

Copyright (c) 2016 - 2017, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent
**/

/**

#include <Uefi.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/HiiImageEx.h>
#include <Protocol/PlatformLogo.h>
#include <Protocol/HiiPackageList.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/SimpleTextOut.h>
#include <Protocol/SimplePointer.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>

typedef struct {
  EFI_IMAGE_ID                             ImageId;
  EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE    Attribute;
  INTN                                     OffsetX;
  INTN                                     OffsetY;
} LOGO_ENTRY;

// Menu item structure
typedef struct {
  CHAR16  *MenuText;
  CHAR16  *Description;
  UINT32  StartRow;    // Row where menu item starts
  UINT32  StartCol;    // Column where menu item starts  
  UINT32  EndRow;      // Row where menu item ends
  UINT32  EndCol;      // Column where menu item ends
} MENU_ITEM;

// Mouse state structure
typedef struct {
  BOOLEAN  LeftButtonPressed;
  BOOLEAN  LeftButtonReleased;
  BOOLEAN  WasLeftButtonPressed;
  UINT32   ClickCount;
  UINT64   LastClickTime;
  INT32    LastX;
  INT32    LastY;
} MOUSE_STATE;

// Global variables
EFI_HII_IMAGE_EX_PROTOCOL  *mHiiImageEx;
EFI_HII_HANDLE             mHiiHandle;
EFI_EVENT                  mKeyMonitorEvent;
EFI_EVENT                  mTimerEvent;
EFI_EVENT                  mBiosSetupKeyEvent;
EFI_EVENT                  mMouseEvent;
EFI_SIMPLE_POINTER_PROTOCOL *mSimplePointer = NULL;
BOOLEAN                    mLogoDisplayed = FALSE;
BOOLEAN                    mMessageShown = FALSE;
BOOLEAN                    mInBiosSetup = FALSE;
UINTN                      mTimerCounter = 0;
UINTN                      mSelectedMenuItem = 0;
UINTN                      mTotalMenuItems = 0;
MOUSE_STATE                mMouseState;

// Menu items for BIOS setup - Enhanced with position information
MENU_ITEM mBiosSetupMenu[] = {
  { L"Boot Manager", L"Configure boot device priority", 10, 10, 10, 35, },
  { L"Device Manager", L"Configure hardware devices", 11, 10, 11, 35 },
  { L"Boot Maintenance Manager", L"Advanced boot options", 12, 10, 12, 45 },
  { L"Save & Exit", L"Save changes and exit", 13, 10, 13, 30 },
  { L"Exit Without Saving", L"Exit without saving changes", 14, 10, 14, 40 }
};

// Function declarations
VOID
DisplayBiosSetupForm (
  VOID
  );

VOID
GetCurrentTime (
  OUT CHAR16  *TimeString
  );

VOID
HandleBiosSetupNavigation (
  VOID
  );

VOID
DisplaySelectedMenuInfo (
  VOID
  );

EFI_STATUS
InitializeMouse (
  VOID
  );

VOID
UpdateMouseState (
  VOID
  );

UINTN
GetMenuItemFromMousePosition (
  IN INT32 MouseX,
  IN INT32 MouseY
  );

LOGO_ENTRY                 mLogos[] = {
  {
    IMAGE_TOKEN (IMG_LOGO),
    EdkiiPlatformLogoDisplayAttributeCenter,
    0,
    0
  }
};

**/

/**
  Initialize mouse support by locating Simple Pointer Protocol.

  @retval EFI_SUCCESS   Mouse initialized successfully.
  @retval Others        Failed to initialize mouse.
**/

/**
EFI_STATUS
InitializeMouse (
  VOID
  )
{
  EFI_STATUS Status;
  
  // Initialize mouse state
  mMouseState.LeftButtonPressed = FALSE;
  mMouseState.LeftButtonReleased = FALSE;
  mMouseState.WasLeftButtonPressed = FALSE;
  mMouseState.ClickCount = 0;
  mMouseState.LastClickTime = 0;
  mMouseState.LastX = 0;
  mMouseState.LastY = 0;
  
  // Locate Simple Pointer Protocol
  Status = gBS->LocateProtocol (
                  &gEfiSimplePointerProtocolGuid,
                  NULL,
                  (VOID **)&mSimplePointer
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "Mouse not available: %r\n", Status));
    mSimplePointer = NULL;
    return Status;
  }
  
  // Reset mouse
  Status = mSimplePointer->Reset (mSimplePointer, FALSE);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "Failed to reset mouse: %r\n", Status));
  }
  
  DEBUG ((DEBUG_INFO, "Mouse initialized successfully\n"));
  return EFI_SUCCESS;
}


**/

/**
  Update mouse state by reading current mouse input.
**/

/**
VOID
UpdateMouseState (
  VOID
  )
{
  EFI_STATUS                Status;
  EFI_SIMPLE_POINTER_STATE  MouseState;
  
  if (mSimplePointer == NULL) {
    return;
  }
  
  Status = mSimplePointer->GetState (mSimplePointer, &MouseState);
  if (EFI_ERROR (Status)) {
    return;
  }
  
  // Update mouse position (convert relative to screen coordinates)
  mMouseState.LastX += MouseState.RelativeMovementX / 1000; // Adjust scaling as needed
  mMouseState.LastY += MouseState.RelativeMovementY / 1000;
  
  // Boundary check
  if (mMouseState.LastX < 0) mMouseState.LastX = 0;
  if (mMouseState.LastY < 0) mMouseState.LastY = 0;
  if (mMouseState.LastX > 79) mMouseState.LastX = 79; // Console width
  if (mMouseState.LastY > 24) mMouseState.LastY = 24; // Console height
  
  // Update button state
  mMouseState.WasLeftButtonPressed = mMouseState.LeftButtonPressed;
  mMouseState.LeftButtonPressed = MouseState.LeftButton;
  
  // Detect button release (click)
  if (mMouseState.WasLeftButtonPressed && !mMouseState.LeftButtonPressed) {
    mMouseState.LeftButtonReleased = TRUE;
    
    // Handle double-click detection (within 500ms)
    UINT64 CurrentTime = 0; // You might need to implement time tracking
    if (CurrentTime - mMouseState.LastClickTime < 5000000) { // 500ms in 100ns units
      mMouseState.ClickCount++;
    } else {
      mMouseState.ClickCount = 1;
    }
    mMouseState.LastClickTime = CurrentTime;
  } else {
    mMouseState.LeftButtonReleased = FALSE;
  }
}


**/
/**
  Get menu item index based on mouse position.

  @param MouseX   Mouse X coordinate.
  @param MouseY   Mouse Y coordinate.

  @retval Index   Menu item index, or -1 if not over any menu item.
**/

/**

UINTN
GetMenuItemFromMousePosition (
  IN INT32 MouseX,
  IN INT32 MouseY
  )
{
  UINTN Index;
  
  for (Index = 0; Index < mTotalMenuItems; Index++) {
    if (MouseY == (mBiosSetupMenu[Index].StartRow) &&
        MouseX >= mBiosSetupMenu[Index].StartCol &&
        MouseX <= mBiosSetupMenu[Index].EndCol) {
      return Index;
    }
  }
  
  return (UINTN)-1; // Not over any menu item
}

**/

/**
  Get current time and format it as string.

  @param TimeString   Output buffer for formatted time string.
**/

/**
VOID
GetCurrentTime (
  OUT CHAR16  *TimeString
  )
{
  EFI_TIME    Time;
  EFI_STATUS  Status;
  
  // Attempt to get the system time
  Status = gRT->GetTime (&Time, NULL);
  if (!EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "GetCurrentTime: Successfully retrieved time - %02d:%02d:%02d\n", Time.Hour, Time.Minute, Time.Second));
    UnicodeSPrint (
      TimeString, 
      50 * sizeof(CHAR16),
      L"%02d:%02d:%02d",
      Time.Hour,
      Time.Minute,
      Time.Second
    );
  } else {
    DEBUG ((DEBUG_ERROR, "GetCurrentTime: Failed to get time, Status: %r\n", Status));
    // Fallback: Display a static message or placeholder
    UnicodeSPrint (TimeString, 50 * sizeof(CHAR16), L"Time Unavailable");
  }
}


**/
/**
  Display information about the selected menu item.
**/

/**
VOID
DisplaySelectedMenuInfo (
  VOID
  )
{
  // Clear the info area
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 12);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"                                    ");
  
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 13);
  Print (L"                                    ");
  
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 14);
  Print (L"                                    ");
  
  // Display selected item info
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 12);
  Print (L"Selected: %s", mBiosSetupMenu[mSelectedMenuItem].MenuText);
  
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 14);
  Print (L"%s", mBiosSetupMenu[mSelectedMenuItem].Description);
}

**/

/**
  Display BIOS setup form with white background, menu items and navigation.
**/

/**
VOID
DisplayBiosSetupForm (
  VOID
  )
{
  UINTN   Index;
  CHAR16  TimeString[50];
  
  mTotalMenuItems = ARRAY_SIZE (mBiosSetupMenu);
  mSelectedMenuItem = 0;
  mInBiosSetup = TRUE;
  
  // Initialize mouse support
  InitializeMouse ();
  
  // Clear screen
  gST->ConOut->ClearScreen (gST->ConOut);
  
  // Set white background with black text
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  
  // Fill entire screen with white background
  for (Index = 0; Index < 25; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Index);
    Print (L"                                                                                ");
  }
  
  // Display "CDAC Bangalore" at top center
  gST->ConOut->SetCursorPosition (gST->ConOut, 30, 2);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"CDAC Bangalore");
  
  // Get and display current time at top right corner
  GetCurrentTime (TimeString);
  gST->ConOut->SetCursorPosition (gST->ConOut, 65, 2);
  Print (L"%s", TimeString);
  
  // Add main content area
  gST->ConOut->SetCursorPosition (gST->ConOut, 25, 5);
  Print (L"====================================");
  gST->ConOut->SetCursorPosition (gST->ConOut, 30, 6);
  Print (L"BIOS SETUP UTILITY");
  gST->ConOut->SetCursorPosition (gST->ConOut, 25, 7);
  Print (L"====================================");
  
  // Update menu item positions and display menu items
  for (Index = 0; Index < mTotalMenuItems; Index++) {
    // Update position information
    mBiosSetupMenu[Index].StartRow = 10 + Index;
    mBiosSetupMenu[Index].StartCol = 10;
    mBiosSetupMenu[Index].EndRow = 10 + Index;
    mBiosSetupMenu[Index].EndCol = 10 + StrLen(mBiosSetupMenu[Index].MenuText) + 3;
    
    gST->ConOut->SetCursorPosition (gST->ConOut, 10, 10 + Index);
    
    if (Index == mSelectedMenuItem) {
      // Highlight selected item with reverse colors
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
      Print (L"-> %s", mBiosSetupMenu[Index].MenuText);
    } else {
      // Normal menu item
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"   %s", mBiosSetupMenu[Index].MenuText);
    }
  }
  
  // Display selected item information
  DisplaySelectedMenuInfo ();
  
  // Show navigation instructions
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  gST->ConOut->SetCursorPosition (gST->ConOut, 10, 20);
  Print (L"Navigation Instructions:");
  gST->ConOut->SetCursorPosition (gST->ConOut, 10, 21);
  Print (L"Use UP/DOWN arrow keys or mouse to navigate");
  gST->ConOut->SetCursorPosition (gST->ConOut, 10, 22);
  Print (L"Press ENTER or double-click to select");
  gST->ConOut->SetCursorPosition (gST->ConOut, 10, 23);
  Print (L"Press ESC to exit setup");
  
  // Add footer
  gST->ConOut->SetCursorPosition (gST->ConOut, 20, 24);
  Print (L"BIOS Setup - F12 Key Activated - CDAC Bangalore - Mouse Enabled");
  
  // Reset console input for clean key detection in setup
  gST->ConIn->Reset (gST->ConIn, FALSE);
  
  // Start navigation handling
  HandleBiosSetupNavigation ();
}

**/

/**
  Handle menu selection based on current selected item.
  Modified to shutdown system instead of trying to continue boot.
**/
/**

VOID
HandleMenuSelection (
  VOID
  )
{
  switch (mSelectedMenuItem) {
    case 0: // Boot Manager
      gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
      Print (L"Boot Manager!");
      break;
      
    case 1: // Device Manager  
      gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
      Print (L"Device Manager!");
      break;
      
    case 2: // Boot Maintenance Manager
      gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
      Print (L"Boot Maintenance Manager!");
      break;
    
    case 3: // Save & Exit
      gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_GREEN | EFI_BACKGROUND_BLUE);
      Print (L"Saving and exiting...");
      gBS->Stall (2000000); // Wait 2 seconds
      
      // Show shutdown message
      gST->ConOut->ClearScreen (gST->ConOut);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
      gST->ConOut->SetCursorPosition (gST->ConOut, 25, 12);
      Print (L"Settings saved. Shutting down system...");
      gBS->Stall (2000000); // Wait 2 seconds
      
      // Shutdown the system instead of continuing boot
      gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
      
      mInBiosSetup = FALSE;
      break;
      
    case 4: // Exit Without Saving
      gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_RED | EFI_BACKGROUND_BLUE);
      Print (L"Exiting without saving...");
      gBS->Stall (2000000); // Wait 2 seconds
      
      // Show shutdown message
      gST->ConOut->ClearScreen (gST->ConOut);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
      gST->ConOut->SetCursorPosition (gST->ConOut, 25, 12);
      Print (L"Exiting setup. Shutting down system...");
      gBS->Stall (2000000); // Wait 2 seconds
      
      // Shutdown the system instead of continuing boot  
      gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
      
      mInBiosSetup = FALSE;
      break;
      
    default:
      break;
  }
  
  // If not exiting, continue showing the menu
  if (mInBiosSetup) {
    gBS->Stall (1000000); // Wait 1 second to show selection message
    DisplayBiosSetupForm (); // Refresh the form
  }
}

**/

/**
  Handle navigation within BIOS setup using arrow keys and mouse.
  Modified to shutdown on ESC instead of continuing boot.
**/

/**
VOID
HandleBiosSetupNavigation (
  VOID
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  UINTN          PreviousSelection;
  UINTN          MouseMenuIndex;
  
  while (mInBiosSetup) {
    // Update mouse state first
    UpdateMouseState ();
    
    // Handle mouse input
    if (mSimplePointer != NULL) {
      MouseMenuIndex = GetMenuItemFromMousePosition (mMouseState.LastX, mMouseState.LastY);
      
      // Single click - select menu item
      if (mMouseState.LeftButtonReleased && mMouseState.ClickCount == 1) {
        if (MouseMenuIndex != (UINTN)-1 && MouseMenuIndex < mTotalMenuItems) {
          PreviousSelection = mSelectedMenuItem;
          mSelectedMenuItem = MouseMenuIndex;
          
          // Update display if selection changed
          if (PreviousSelection != mSelectedMenuItem) {
            // Update previous item display (remove highlight)
            gST->ConOut->SetCursorPosition (gST->ConOut, 10, 10 + PreviousSelection);
            gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
            Print (L"   %s", mBiosSetupMenu[PreviousSelection].MenuText);
            
            // Update current item display (add highlight)
            gST->ConOut->SetCursorPosition (gST->ConOut, 10, 10 + mSelectedMenuItem);
            gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
            Print (L"-> %s", mBiosSetupMenu[mSelectedMenuItem].MenuText);
            
            // Update selected item information
            DisplaySelectedMenuInfo ();
          }
        }
        
        // Reset mouse click state
        mMouseState.LeftButtonReleased = FALSE;
      }
      
      // Double click - execute menu selection
      if (mMouseState.LeftButtonReleased && mMouseState.ClickCount >= 2) {
        if (MouseMenuIndex != (UINTN)-1 && MouseMenuIndex < mTotalMenuItems) {
          mSelectedMenuItem = MouseMenuIndex;
          HandleMenuSelection ();
          if (!mInBiosSetup) {
            return; // Exit if user chose to exit
          }
        }
        
        // Reset mouse click state
        mMouseState.LeftButtonReleased = FALSE;
        mMouseState.ClickCount = 0;
      }
    }
    
    // Handle keyboard input
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
    
    if (!EFI_ERROR (Status)) {
      PreviousSelection = mSelectedMenuItem;
      
      switch (Key.ScanCode) {
        case SCAN_UP:
          // Move up in menu
          if (mSelectedMenuItem > 0) {
            mSelectedMenuItem--;
          } else {
            mSelectedMenuItem = mTotalMenuItems - 1; // Wrap to bottom
          }
          break;
          
        case SCAN_DOWN:
          // Move down in menu
          if (mSelectedMenuItem < mTotalMenuItems - 1) {
            mSelectedMenuItem++;
          } else {
            mSelectedMenuItem = 0; // Wrap to top
          }
          break;
          
        case SCAN_ESC:
          // Exit BIOS setup with shutdown
          mInBiosSetup = FALSE;
          gST->ConOut->ClearScreen (gST->ConOut);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
          gST->ConOut->SetCursorPosition (gST->ConOut, 25, 12);
          Print (L"Exiting BIOS Setup. Shutting down system...");
          gBS->Stall (2000000); // Wait 2 seconds
          
          // Shutdown instead of returning to boot process
          gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
          return;
          
        default:
          // Check for ENTER key (Unicode character)
          if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
            HandleMenuSelection ();
            if (!mInBiosSetup) {
              return; // Exit if user chose to exit (will shutdown in HandleMenuSelection)
            }
          }
          break;
      }
      
      // Update display if selection changed
      if (mInBiosSetup && (PreviousSelection != mSelectedMenuItem)) {
        // Update previous item display (remove highlight)
        gST->ConOut->SetCursorPosition (gST->ConOut, 10, 10 + PreviousSelection);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"   %s", mBiosSetupMenu[PreviousSelection].MenuText);
        
        // Update current item display (add highlight)
        gST->ConOut->SetCursorPosition (gST->ConOut, 10, 10 + mSelectedMenuItem);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
        Print (L"-> %s", mBiosSetupMenu[mSelectedMenuItem].MenuText);
        
        // Update selected item information
        DisplaySelectedMenuInfo ();
      }
    } else {
      // Small delay to prevent excessive CPU usage
      gBS->Stall (50000); // 50ms
    }
  }
}


**/
/**
  Monitor keyboard input for F12 key press only.
  Modified to shutdown after BIOS setup completion.
**/

/**
VOID
EFIAPI
KeyboardMonitor (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  
  // Only monitor after message is shown and not in BIOS setup
  if (!mMessageShown || mInBiosSetup) {
    return;
  }
  
  // Check if key is available
  Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
  
  if (!EFI_ERROR (Status)) {
    // Check for F12 key only
    if (Key.ScanCode == SCAN_F12) {
      DEBUG ((DEBUG_INFO, "F12 key pressed - showing BIOS setup form\n"));
      
      // Cancel timer and keyboard events to stop normal flow
      gBS->SetTimer (mKeyMonitorEvent, TimerCancel, 0);
      gBS->SetTimer (mTimerEvent, TimerCancel, 0);
      
      // Display BIOS setup form
      DisplayBiosSetupForm ();
      
      // After exiting BIOS setup, shutdown (this code won't reach normally 
      // because HandleBiosSetupNavigation() handles shutdown)
      gST->ConOut->ClearScreen (gST->ConOut);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
      gST->ConOut->SetCursorPosition (gST->ConOut, 25, 12);
      Print (L"BIOS Setup completed. Shutting down system...");
      gBS->Stall (2000000); // Wait 2 seconds
      
      // Shutdown the system
      gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
      
      return;
    }
  }
}

**/

/**
  Timer callback function to handle logo display timing.

  @param Event      The event that is signaled.
  @param Context    The context passed to the event.
**/

/**
VOID
EFIAPI
TimerCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  mTimerCounter++;
  
  // After 5 seconds, show the message below logo and wait for F12
  if (mTimerCounter == 50 && !mMessageShown) { // 50 * 100ms = 5 seconds
    // Clear the entire screen first
    gST->ConOut->ClearScreen (gST->ConOut);
    
    // Position cursor below the logo area
    gST->ConOut->SetCursorPosition (gST->ConOut, 18, 18);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
    Print (L"Press F12 key for BIOS setup");
    
    mMessageShown = TRUE;
    
    // Reset console input for clean key detection
    gST->ConIn->Reset (gST->ConIn, FALSE);
    
    DEBUG ((DEBUG_INFO, "Message displayed, waiting for F12 key\n"));
  }
  
  // Explicitly do nothing after showing the message
  // Wait indefinitely for F12 key press, no shell redirect
}


**/
/**
  Setup timing and keyboard monitoring after logo is displayed.
**/

/**
VOID
SetupLogoTiming (
  VOID
  )
{
  EFI_STATUS  Status;
  
  if (mLogoDisplayed) {
    return; // Already setup
  }
  
  DEBUG ((DEBUG_INFO, "Setting up logo timing and keyboard monitoring\n"));
  
  // Create timer event for logo timing
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  TimerCallback,
                  NULL,
                  &mTimerEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    // Set timer to fire every 100ms
    Status = gBS->SetTimer (
                    mTimerEvent,
                    TimerPeriodic,
                    1000000  // 100ms in 100ns units
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set timer for mTimerEvent: %r\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to create mTimerEvent: %r\n", Status));
  }
  
  // Create keyboard monitoring event
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  KeyboardMonitor,
                  NULL,
                  &mKeyMonitorEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    // Set keyboard timer to fire every 50ms for responsive key detection
    Status = gBS->SetTimer (
                    mKeyMonitorEvent,
                    TimerPeriodic,
                    500000  // 50ms in 100ns units
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set timer for mKeyMonitorEvent: %r\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to create mKeyMonitorEvent: %r\n", Status));
  }
  
  mLogoDisplayed = TRUE;
}

**/

/**
  Load a platform logo image and return its data and attributes.

  @param This              The pointer to this protocol instance.
  @param Instance          The visible image instance is found.
  @param Image             Points to the image.
  @param Attribute         The display attributes of the image returned.
  @param OffsetX           The X offset of the image regarding the Attribute.
  @param OffsetY           The Y offset of the image regarding the Attribute.

  @retval EFI_SUCCESS      The image was fetched successfully.
  @retval EFI_NOT_FOUND    The specified image could not be found.
**/

/**
EFI_STATUS
EFIAPI
GetImage (
  IN     EDKII_PLATFORM_LOGO_PROTOCOL        *This,
  IN OUT UINT32                              *Instance,
  OUT EFI_IMAGE_INPUT                        *Image,
  OUT EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE  *Attribute,
  OUT INTN                                   *OffsetX,
  OUT INTN                                   *OffsetY
  )
{
  UINT32      Current;
  EFI_STATUS  Status;

  if ((Instance == NULL) || (Image == NULL) ||
      (Attribute == NULL) || (OffsetX == NULL) || (OffsetY == NULL))
  {
    return EFI_INVALID_PARAMETER;
  }

  Current = *Instance;
  if (Current >= ARRAY_SIZE (mLogos)) {
    return EFI_NOT_FOUND;
  }

  (*Instance)++;
  *Attribute = mLogos[Current].Attribute;
  *OffsetX   = mLogos[Current].OffsetX;
  *OffsetY   = mLogos[Current].OffsetY;
  
  // Get image from HII database
  Status = mHiiImageEx->GetImageEx (mHiiImageEx, mHiiHandle, mLogos[Current].ImageId, Image);
  
  // After first logo is displayed, setup timing
  if (Current == 0 && !EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Logo displayed, setting up timing\n"));
    SetupLogoTiming ();
  }
  
  return Status;
}

EDKII_PLATFORM_LOGO_PROTOCOL  mPlatformLogo = {
  GetImage
};

**/

/**
  Cleanup function called when driver is unloaded.

  @param ImageHandle    The handle of the image being unloaded.

  @retval EFI_SUCCESS   The image was unloaded successfully.
**/

/**
EFI_STATUS
EFIAPI
LogoDriverUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
  // Close events if they exist
  if (mTimerEvent != NULL) {
    gBS->CloseEvent (mTimerEvent);
    mTimerEvent = NULL;
  }
  
  if (mKeyMonitorEvent != NULL) {
    gBS->CloseEvent (mKeyMonitorEvent);
    mKeyMonitorEvent = NULL;
  }
  
  if (mBiosSetupKeyEvent != NULL) {
    gBS->CloseEvent (mBiosSetupKeyEvent);
    mBiosSetupKeyEvent = NULL;
  }
  
  if (mMouseEvent != NULL) {
    gBS->CloseEvent (mMouseEvent);
    mMouseEvent = NULL;
  }
  
  return EFI_SUCCESS;
}

**/

/**
  Entrypoint of this module.

  This function is the entrypoint of this module. It installs the Edkii
  Platform Logo protocol.

  @param  ImageHandle       The firmware allocated handle for the EFI image.
  @param  SystemTable       A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
**/

/**
EFI_STATUS
EFIAPI
InitializeLogo (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                   Status;
  EFI_HII_PACKAGE_LIST_HEADER  *PackageList;
  EFI_HII_DATABASE_PROTOCOL    *HiiDatabase;
  EFI_HANDLE                   Handle;

  DEBUG ((DEBUG_INFO, "Logo Driver: Initializing with mouse support...\n"));

  // Initialize global variables
  mInBiosSetup = FALSE;
  mSelectedMenuItem = 0;
  mTotalMenuItems = ARRAY_SIZE (mBiosSetupMenu);

  // Locate HII Database Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiDatabaseProtocolGuid,
                  NULL,
                  (VOID **)&HiiDatabase
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Database Protocol: %r\n", Status));
    return Status;
  }

  // Locate HII Image Ex Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiImageExProtocolGuid,
                  NULL,
                  (VOID **)&mHiiImageEx
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Image Ex Protocol: %r\n", Status));
    return Status;
  }

  // Retrieve HII package list from ImageHandle
  Status = gBS->OpenProtocol (
                  ImageHandle,
                  &gEfiHiiPackageListProtocolGuid,
                  (VOID **)&PackageList,
                  ImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "HII Image Package with logo not found in PE/COFF resource section\n"));
    return Status;
  }

  // Publish HII package list to HII Database
  Status = HiiDatabase->NewPackageList (
                          HiiDatabase,
                          PackageList,
                          NULL,
                          &mHiiHandle
                          );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to add package list to HII Database: %r\n", Status));
    return Status;
  }

  // Install Platform Logo Protocol
  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEdkiiPlatformLogoProtocolGuid,
                  &mPlatformLogo,
                  NULL
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to install Platform Logo Protocol: %r\n", Status));
    // Cleanup HII handle
    HiiDatabase->RemovePackageList (HiiDatabase, mHiiHandle);
    return Status;
  }

  DEBUG ((DEBUG_INFO, "Logo Driver: Initialized successfully with mouse support\n"));
  
  return EFI_SUCCESS;
}


**/


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//mouse and keyboard bios setup utility.but mouse is not properly work. 



/** @file
  Logo DXE Driver with F12 key BIOS setup functionality.
  Shows logo, waits for F12 key, then displays BIOS setup form with navigation.
  Enhanced with mouse support for menu selection.

Copyright (c) 2016 - 2017, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent
**/

/**

#include <Uefi.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/HiiImageEx.h>
#include <Protocol/PlatformLogo.h>
#include <Protocol/HiiPackageList.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/SimpleTextOut.h>
#include <Protocol/SimplePointer.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>

typedef struct {
  EFI_IMAGE_ID                             ImageId;
  EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE    Attribute;
  INTN                                     OffsetX;
  INTN                                     OffsetY;
} LOGO_ENTRY;

// Menu item structure
typedef struct {
  CHAR16  *MenuText;
  CHAR16  *Description;
  UINT32  StartRow;    // Row where menu item starts
  UINT32  StartCol;    // Column where menu item starts  
  UINT32  EndRow;      // Row where menu item ends
  UINT32  EndCol;      // Column where menu item ends
} MENU_ITEM;

// Mouse state structure
typedef struct {
  BOOLEAN  LeftButtonPressed;
  BOOLEAN  LeftButtonReleased;
  BOOLEAN  WasLeftButtonPressed;
  UINT32   ClickCount;
  UINT64   LastClickTime;
  INT32    LastX;
  INT32    LastY;
} MOUSE_STATE;

// Global variables
EFI_HII_IMAGE_EX_PROTOCOL  *mHiiImageEx;
EFI_HII_HANDLE             mHiiHandle;
EFI_EVENT                  mKeyMonitorEvent;
EFI_EVENT                  mTimerEvent;
EFI_EVENT                  mBiosSetupKeyEvent;
EFI_EVENT                  mMouseEvent;
EFI_SIMPLE_POINTER_PROTOCOL *mSimplePointer = NULL;
BOOLEAN                    mLogoDisplayed = FALSE;
BOOLEAN                    mMessageShown = FALSE;
BOOLEAN                    mInBiosSetup = FALSE;
UINTN                      mTimerCounter = 0;
UINTN                      mSelectedMenuItem = 0;
UINTN                      mTotalMenuItems = 0;
MOUSE_STATE                mMouseState;

// Menu items for BIOS setup - Enhanced with position information
MENU_ITEM mBiosSetupMenu[] = {
  { L"Boot Manager", L"Configure boot device priority", 10, 10, 10, 35, },
  { L"Device Manager", L"Configure hardware devices", 11, 10, 11, 35 },
  { L"Boot Maintenance Manager", L"Advanced boot options", 12, 10, 12, 45 },
  { L"Save & Exit", L"Save changes and exit", 13, 10, 13, 30 },
  { L"Exit Without Saving", L"Exit without saving changes", 14, 10, 14, 40 }
};

// Function declarations
VOID
DisplayBiosSetupForm (
  VOID
  );

VOID
GetCurrentTime (
  OUT CHAR16  *TimeString
  );

VOID
HandleBiosSetupNavigation (
  VOID
  );

VOID
DisplaySelectedMenuInfo (
  VOID
  );

EFI_STATUS
InitializeMouse (
  VOID
  );

VOID
UpdateMouseState (
  VOID
  );

UINTN
GetMenuItemFromMousePosition (
  IN INT32 MouseX,
  IN INT32 MouseY
  );

LOGO_ENTRY                 mLogos[] = {
  {
    IMAGE_TOKEN (IMG_LOGO),
    EdkiiPlatformLogoDisplayAttributeCenter,
    0,
    0
  }
};

**/

/**
  Initialize mouse support by locating Simple Pointer Protocol.

  @retval EFI_SUCCESS   Mouse initialized successfully.
  @retval Others        Failed to initialize mouse.
**/

/**
EFI_STATUS
InitializeMouse (
  VOID
  )
{
  EFI_STATUS Status;
  
  // Initialize mouse state with center screen position
  mMouseState.LeftButtonPressed = FALSE;
  mMouseState.LeftButtonReleased = FALSE;
  mMouseState.WasLeftButtonPressed = FALSE;
  mMouseState.ClickCount = 0;
  mMouseState.LastClickTime = 0;
  mMouseState.LastX = 40; // Center of 80-column screen
  mMouseState.LastY = 12; // Center of 25-row screen
  
  // Try to locate Simple Pointer Protocol
  Status = gBS->LocateProtocol (
                  &gEfiSimplePointerProtocolGuid,
                  NULL,
                  (VOID **)&mSimplePointer
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "Simple Pointer Protocol not available: %r\n", Status));
    DEBUG ((DEBUG_WARN, "Mouse support disabled - using keyboard only\n"));
    mSimplePointer = NULL;
    return Status;
  }
  
  // Reset mouse with extended reset
  Status = mSimplePointer->Reset (mSimplePointer, TRUE);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "Failed to reset mouse: %r\n", Status));
    // Try without extended verification
    Status = mSimplePointer->Reset (mSimplePointer, FALSE);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Mouse reset failed completely: %r\n", Status));
      mSimplePointer = NULL;
      return Status;
    }
  }
  
  DEBUG ((DEBUG_INFO, "Mouse initialized successfully\n"));
  DEBUG ((DEBUG_INFO, "Mouse Resolution: X=%d, Y=%d, Z=%d\n", 
          mSimplePointer->Mode->ResolutionX,
          mSimplePointer->Mode->ResolutionY, 
          mSimplePointer->Mode->ResolutionZ));
  
  return EFI_SUCCESS;
}

**/

/**
  Update mouse state by reading current mouse input.
**/

/**
VOID
UpdateMouseState (
  VOID
  )
{
  EFI_STATUS                Status;
  EFI_SIMPLE_POINTER_STATE  MouseState;
 // STATIC BOOLEAN            FirstRun = TRUE;
  
  if (mSimplePointer == NULL) {
    return;
  }
  
  Status = mSimplePointer->GetState (mSimplePointer, &MouseState);
  if (EFI_ERROR (Status)) {
    // No mouse input available right now
    return;
  }
  
  // Handle relative movement with better scaling
  // Scale down the movement for better control in console mode
  INT32 DeltaX = MouseState.RelativeMovementX / 65536; // Adjusted scaling
  INT32 DeltaY = MouseState.RelativeMovementY / 65536; // Adjusted scaling
  
  // Apply movement if significant enough
  if (DeltaX != 0 || DeltaY != 0) {
    mMouseState.LastX += DeltaX;
    mMouseState.LastY += DeltaY;
    
    // Boundary check for 80x25 console
    if (mMouseState.LastX < 0) mMouseState.LastX = 0;
    if (mMouseState.LastY < 0) mMouseState.LastY = 0;
    if (mMouseState.LastX > 79) mMouseState.LastX = 79;
    if (mMouseState.LastY > 24) mMouseState.LastY = 24;
    
    DEBUG ((DEBUG_VERBOSE, "Mouse moved to: %d, %d (Delta: %d, %d)\n", 
            mMouseState.LastX, mMouseState.LastY, DeltaX, DeltaY));
  }
  
  // Update button state
  mMouseState.WasLeftButtonPressed = mMouseState.LeftButtonPressed;
  mMouseState.LeftButtonPressed = MouseState.LeftButton;
  
  // Detect button release (click completion)
  if (mMouseState.WasLeftButtonPressed && !mMouseState.LeftButtonPressed) {
    mMouseState.LeftButtonReleased = TRUE;
    
    // Simple click counting without time dependency (UEFI time services may not be reliable)
    mMouseState.ClickCount++;
    
    DEBUG ((DEBUG_INFO, "Mouse click detected at: %d, %d (Click count: %d)\n", 
            mMouseState.LastX, mMouseState.LastY, mMouseState.ClickCount));
            
    // Reset click count after a reasonable number to prevent overflow
    if (mMouseState.ClickCount > 10) {
      mMouseState.ClickCount = 1;
    }
  } else {
    mMouseState.LeftButtonReleased = FALSE;
  }
  
  // Reset click count when button is pressed (start of new click sequence)
  if (!mMouseState.WasLeftButtonPressed && mMouseState.LeftButtonPressed) {
    // Button just pressed - reset for new click sequence
    if (mMouseState.ClickCount > 2) {
      mMouseState.ClickCount = 0;
    }
  }
}

**/

/**
  Get menu item index based on mouse position.

  @param MouseX   Mouse X coordinate.
  @param MouseY   Mouse Y coordinate.

  @retval Index   Menu item index, or -1 if not over any menu item.
**/

/**
UINTN
GetMenuItemFromMousePosition (
  IN INT32 MouseX,
  IN INT32 MouseY
  )
{
  UINTN Index;
  
  for (Index = 0; Index < mTotalMenuItems; Index++) {
    if (MouseY == (mBiosSetupMenu[Index].StartRow) &&
        MouseX >= mBiosSetupMenu[Index].StartCol &&
        MouseX <= mBiosSetupMenu[Index].EndCol) {
      return Index;
    }
  }
  
  return (UINTN)-1; // Not over any menu item
}


**/
/**
  Get current time and format it as string.

  @param TimeString   Output buffer for formatted time string.
**/

/**
VOID
GetCurrentTime (
  OUT CHAR16  *TimeString
  )
{
  EFI_TIME    Time;
  EFI_STATUS  Status;
  
  // Attempt to get the system time
  Status = gRT->GetTime (&Time, NULL);
  if (!EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "GetCurrentTime: Successfully retrieved time - %02d:%02d:%02d\n", Time.Hour, Time.Minute, Time.Second));
    UnicodeSPrint (
      TimeString, 
      50 * sizeof(CHAR16),
      L"%02d:%02d:%02d",
      Time.Hour,
      Time.Minute,
      Time.Second
    );
  } else {
    DEBUG ((DEBUG_ERROR, "GetCurrentTime: Failed to get time, Status: %r\n", Status));
    // Fallback: Display a static message or placeholder
    UnicodeSPrint (TimeString, 50 * sizeof(CHAR16), L"Time Unavailable");
  }
}

**/

/**
  Display information about the selected menu item.
**/

/**
VOID
DisplaySelectedMenuInfo (
  VOID
  )
{
  // Clear the info area
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 12);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"                                    ");
  
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 13);
  Print (L"                                    ");
  
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 14);
  Print (L"                                    ");
  
  // Display selected item info
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 12);
  Print (L"Selected: %s", mBiosSetupMenu[mSelectedMenuItem].MenuText);
  
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 14);
  Print (L"%s", mBiosSetupMenu[mSelectedMenuItem].Description);
}


**/
/**
  Display BIOS setup form with white background, menu items and navigation.
**/

/**
VOID
DisplayBiosSetupForm (
  VOID
  )
{
  UINTN   Index;
  CHAR16  TimeString[50];
  
  mTotalMenuItems = ARRAY_SIZE (mBiosSetupMenu);
  mSelectedMenuItem = 0;
  mInBiosSetup = TRUE;
  
  // Initialize mouse support
  InitializeMouse ();
  
  // Clear screen
  gST->ConOut->ClearScreen (gST->ConOut);
  
  // Set white background with black text
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  
  // Fill entire screen with white background
  for (Index = 0; Index < 25; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Index);
    Print (L"                                                                                ");
  }
  
  // Display "CDAC Bangalore" at top center
  gST->ConOut->SetCursorPosition (gST->ConOut, 30, 2);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"CDAC Bangalore");
  
  // Get and display current time at top right corner
  GetCurrentTime (TimeString);
  gST->ConOut->SetCursorPosition (gST->ConOut, 65, 2);
  Print (L"%s", TimeString);
  
  // Add main content area
  gST->ConOut->SetCursorPosition (gST->ConOut, 25, 5);
  Print (L"====================================");
  gST->ConOut->SetCursorPosition (gST->ConOut, 30, 6);
  Print (L"BIOS SETUP UTILITY");
  gST->ConOut->SetCursorPosition (gST->ConOut, 25, 7);
  Print (L"====================================");
  
  // Update menu item positions and display menu items
  for (Index = 0; Index < mTotalMenuItems; Index++) {
    // Update position information
    mBiosSetupMenu[Index].StartRow = 10 + Index;
    mBiosSetupMenu[Index].StartCol = 10;
    mBiosSetupMenu[Index].EndRow = 10 + Index;
    mBiosSetupMenu[Index].EndCol = 10 + StrLen(mBiosSetupMenu[Index].MenuText) + 3;
    
    gST->ConOut->SetCursorPosition (gST->ConOut, 10, 10 + Index);
    
    if (Index == mSelectedMenuItem) {
      // Highlight selected item with reverse colors
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
      Print (L"-> %s", mBiosSetupMenu[Index].MenuText);
    } else {
      // Normal menu item
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"   %s", mBiosSetupMenu[Index].MenuText);
    }
  }
  
  // Display selected item information
  DisplaySelectedMenuInfo ();
  
  // Show navigation instructions
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  gST->ConOut->SetCursorPosition (gST->ConOut, 10, 20);
  Print (L"Navigation Instructions:");
  gST->ConOut->SetCursorPosition (gST->ConOut, 10, 21);
  Print (L"Use UP/DOWN arrow keys or mouse to navigate");
  gST->ConOut->SetCursorPosition (gST->ConOut, 10, 22);
  Print (L"Press ENTER or double-click to select");
  gST->ConOut->SetCursorPosition (gST->ConOut, 10, 23);
  Print (L"Press ESC to exit setup");
  
  // Add footer
  gST->ConOut->SetCursorPosition (gST->ConOut, 20, 24);
  Print (L"BIOS Setup - F12 Key Activated - CDAC Bangalore - Mouse Enabled");
  
  // Reset console input for clean key detection in setup
  gST->ConIn->Reset (gST->ConIn, FALSE);
  
  // Start navigation handling
  HandleBiosSetupNavigation ();
}

**/

/**
  Handle menu selection based on current selected item.
  Modified to shutdown system instead of trying to continue boot.
**/

/**
VOID
HandleMenuSelection (
  VOID
  )
{
  switch (mSelectedMenuItem) {
    case 0: // Boot Manager
      gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
      Print (L"Boot Manager!");
      break;
      
    case 1: // Device Manager  
      gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
      Print (L"Device Manager!");
      break;
      
    case 2: // Boot Maintenance Manager
      gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
      Print (L"Boot Maintenance Manager!");
      break;
    
    case 3: // Save & Exit
      gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_GREEN | EFI_BACKGROUND_BLUE);
      Print (L"Saving and exiting...");
      gBS->Stall (2000000); // Wait 2 seconds
      
      // Show shutdown message
      gST->ConOut->ClearScreen (gST->ConOut);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
      gST->ConOut->SetCursorPosition (gST->ConOut, 25, 12);
      Print (L"Settings saved. Shutting down system...");
      gBS->Stall (2000000); // Wait 2 seconds
      
      // Shutdown the system instead of continuing boot
      gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
      
      mInBiosSetup = FALSE;
      break;
      
    case 4: // Exit Without Saving
      gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_RED | EFI_BACKGROUND_BLUE);
      Print (L"Exiting without saving...");
      gBS->Stall (2000000); // Wait 2 seconds
      
      // Show shutdown message
      gST->ConOut->ClearScreen (gST->ConOut);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
      gST->ConOut->SetCursorPosition (gST->ConOut, 25, 12);
      Print (L"Exiting setup. Shutting down system...");
      gBS->Stall (2000000); // Wait 2 seconds
      
      // Shutdown the system instead of continuing boot  
      gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
      
      mInBiosSetup = FALSE;
      break;
      
    default:
      break;
  }
  
  // If not exiting, continue showing the menu
  if (mInBiosSetup) {
    gBS->Stall (1000000); // Wait 1 second to show selection message
    DisplayBiosSetupForm (); // Refresh the form
  }
}

**/

/**
  Handle navigation within BIOS setup using arrow keys and mouse.
  Modified to shutdown on ESC instead of continuing boot.
  Enhanced with better mouse support for QEMU/virtualized environments.
**/

/**
VOID
HandleBiosSetupNavigation (
  VOID
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  UINTN          PreviousSelection;
  UINTN          MouseMenuIndex;
  STATIC UINTN   ConsecutiveClicks = 0;
  STATIC UINTN   LastClickedItem = (UINTN)-1;
  
  // Display mouse status
  if (mSimplePointer != NULL) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 45, 18);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    Print (L"Mouse: Active at (%d,%d)", mMouseState.LastX, mMouseState.LastY);
  } else {
    gST->ConOut->SetCursorPosition (gST->ConOut, 45, 18);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    Print (L"Mouse: Not Available");
  }
  
  while (mInBiosSetup) {
    // Update mouse state first
    UpdateMouseState ();
    
    // Handle mouse input - improved logic for virtualized environments
    if (mSimplePointer != NULL) {
      MouseMenuIndex = GetMenuItemFromMousePosition (mMouseState.LastX, mMouseState.LastY);
      
      // Update mouse position display for debugging
      gST->ConOut->SetCursorPosition (gST->ConOut, 45, 18);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"Mouse: (%02d,%02d) Item:%d", mMouseState.LastX, mMouseState.LastY, 
             (MouseMenuIndex == (UINTN)-1) ? 99 : MouseMenuIndex);
      
      // Handle mouse clicks with improved detection
      if (mMouseState.LeftButtonReleased) {
        DEBUG ((DEBUG_INFO, "Mouse click at (%d,%d), Menu item: %d\n", 
                mMouseState.LastX, mMouseState.LastY, MouseMenuIndex));
        
        if (MouseMenuIndex != (UINTN)-1 && MouseMenuIndex < mTotalMenuItems) {
          // Check for consecutive clicks on same item
          if (LastClickedItem == MouseMenuIndex) {
            ConsecutiveClicks++;
          } else {
            ConsecutiveClicks = 1;
            LastClickedItem = MouseMenuIndex;
          }
          
          // First click - select menu item
          if (ConsecutiveClicks == 1) {
            PreviousSelection = mSelectedMenuItem;
            mSelectedMenuItem = MouseMenuIndex;
            
            // Update display if selection changed
            if (PreviousSelection != mSelectedMenuItem) {
              // Update previous item display (remove highlight)
              gST->ConOut->SetCursorPosition (gST->ConOut, 10, 10 + PreviousSelection);
              gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
              Print (L"   %s", mBiosSetupMenu[PreviousSelection].MenuText);
              
              // Update current item display (add highlight)
              gST->ConOut->SetCursorPosition (gST->ConOut, 10, 10 + mSelectedMenuItem);
              gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
              Print (L"-> %s", mBiosSetupMenu[mSelectedMenuItem].MenuText);
              
              // Update selected item information
              DisplaySelectedMenuInfo ();
            }
            
            // Show click feedback
            gST->ConOut->SetCursorPosition (gST->ConOut, 45, 19);
            gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
            Print (L"Click 1: Selected");
          }
          
          // Second click - execute selection (double-click simulation)
          else if (ConsecutiveClicks >= 2) {
            mSelectedMenuItem = MouseMenuIndex;
            
            // Show execution feedback
            gST->ConOut->SetCursorPosition (gST->ConOut, 45, 19);
            gST->ConOut->SetAttribute (gST->ConOut, EFI_GREEN | EFI_BACKGROUND_BLUE);
            Print (L"Click 2: Executing...");
            gBS->Stall (500000); // Brief pause to show feedback
            
            HandleMenuSelection ();
            ConsecutiveClicks = 0;
            LastClickedItem = (UINTN)-1;
            
            if (!mInBiosSetup) {
              return; // Exit if user chose to exit
            }
          }
        } else {
          // Click outside menu area - reset click counter
          ConsecutiveClicks = 0;
          LastClickedItem = (UINTN)-1;
          
          gST->ConOut->SetCursorPosition (gST->ConOut, 45, 19);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_RED | EFI_BACKGROUND_LIGHTGRAY);
          Print (L"Click: Outside menu");
        }
        
        // Reset mouse click state
        mMouseState.LeftButtonReleased = FALSE;
        mMouseState.ClickCount = 0;
      }
    }
    
    // Handle keyboard input (unchanged)
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
    
    if (!EFI_ERROR (Status)) {
      PreviousSelection = mSelectedMenuItem;
      
      switch (Key.ScanCode) {
        case SCAN_UP:
          // Move up in menu
          if (mSelectedMenuItem > 0) {
            mSelectedMenuItem--;
          } else {
            mSelectedMenuItem = mTotalMenuItems - 1; // Wrap to bottom
          }
          ConsecutiveClicks = 0; // Reset mouse click counter
          LastClickedItem = (UINTN)-1;
          break;
          
        case SCAN_DOWN:
          // Move down in menu
          if (mSelectedMenuItem < mTotalMenuItems - 1) {
            mSelectedMenuItem++;
          } else {
            mSelectedMenuItem = 0; // Wrap to top
          }
          ConsecutiveClicks = 0; // Reset mouse click counter
          LastClickedItem = (UINTN)-1;
          break;
          
        case SCAN_ESC:
          // Exit BIOS setup with shutdown
          mInBiosSetup = FALSE;
          gST->ConOut->ClearScreen (gST->ConOut);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
          gST->ConOut->SetCursorPosition (gST->ConOut, 25, 12);
          Print (L"Exiting BIOS Setup. Shutting down system...");
          gBS->Stall (2000000); // Wait 2 seconds
          
          // Shutdown instead of returning to boot process
          gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
          return;
          
        default:
          // Check for ENTER key (Unicode character)
          if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
            HandleMenuSelection ();
            if (!mInBiosSetup) {
              return; // Exit if user chose to exit (will shutdown in HandleMenuSelection)
            }
          }
          break;
      }
      
      // Update display if selection changed via keyboard
      if (mInBiosSetup && (PreviousSelection != mSelectedMenuItem)) {
        // Update previous item display (remove highlight)
        gST->ConOut->SetCursorPosition (gST->ConOut, 10, 10 + PreviousSelection);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"   %s", mBiosSetupMenu[PreviousSelection].MenuText);
        
        // Update current item display (add highlight)
        gST->ConOut->SetCursorPosition (gST->ConOut, 10, 10 + mSelectedMenuItem);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
        Print (L"-> %s", mBiosSetupMenu[mSelectedMenuItem].MenuText);
        
        // Update selected item information
        DisplaySelectedMenuInfo ();
      }
    } else {
      // Small delay to prevent excessive CPU usage
      gBS->Stall (50000); // 50ms
    }
  }
}

**/

/**
  Monitor keyboard input for F12 key press only.
  Modified to shutdown after BIOS setup completion.
**/

/**
VOID
EFIAPI
KeyboardMonitor (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  
  // Only monitor after message is shown and not in BIOS setup
  if (!mMessageShown || mInBiosSetup) {
    return;
  }
  
  // Check if key is available
  Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
  
  if (!EFI_ERROR (Status)) {
    // Check for F12 key only
    if (Key.ScanCode == SCAN_F12) {
      DEBUG ((DEBUG_INFO, "F12 key pressed - showing BIOS setup form\n"));
      
      // Cancel timer and keyboard events to stop normal flow
      gBS->SetTimer (mKeyMonitorEvent, TimerCancel, 0);
      gBS->SetTimer (mTimerEvent, TimerCancel, 0);
      
      // Display BIOS setup form
      DisplayBiosSetupForm ();
      
      // After exiting BIOS setup, shutdown (this code won't reach normally 
      // because HandleBiosSetupNavigation() handles shutdown)
      gST->ConOut->ClearScreen (gST->ConOut);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
      gST->ConOut->SetCursorPosition (gST->ConOut, 25, 12);
      Print (L"BIOS Setup completed. Shutting down system...");
      gBS->Stall (2000000); // Wait 2 seconds
      
      // Shutdown the system
      gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
      
      return;
    }
  }
}

**/

/**
  Timer callback function to handle logo display timing.

  @param Event      The event that is signaled.
  @param Context    The context passed to the event.
**/

/**
VOID
EFIAPI
TimerCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  mTimerCounter++;
  
  // After 5 seconds, show the message below logo and wait for F12
  if (mTimerCounter == 50 && !mMessageShown) { // 50 * 100ms = 5 seconds
    // Clear the entire screen first
    gST->ConOut->ClearScreen (gST->ConOut);
    
    // Position cursor below the logo area
    gST->ConOut->SetCursorPosition (gST->ConOut, 18, 18);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
    Print (L"Press F12 key for BIOS setup");
    
    mMessageShown = TRUE;
    
    // Reset console input for clean key detection
    gST->ConIn->Reset (gST->ConIn, FALSE);
    
    DEBUG ((DEBUG_INFO, "Message displayed, waiting for F12 key\n"));
  }
  
  // Explicitly do nothing after showing the message
  // Wait indefinitely for F12 key press, no shell redirect
}


**/
/**
  Setup timing and keyboard monitoring after logo is displayed.
**/

/**
VOID
SetupLogoTiming (
  VOID
  )
{
  EFI_STATUS  Status;
  
  if (mLogoDisplayed) {
    return; // Already setup
  }
  
  DEBUG ((DEBUG_INFO, "Setting up logo timing and keyboard monitoring\n"));
  
  // Create timer event for logo timing
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  TimerCallback,
                  NULL,
                  &mTimerEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    // Set timer to fire every 100ms
    Status = gBS->SetTimer (
                    mTimerEvent,
                    TimerPeriodic,
                    1000000  // 100ms in 100ns units
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set timer for mTimerEvent: %r\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to create mTimerEvent: %r\n", Status));
  }
  
  // Create keyboard monitoring event
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  KeyboardMonitor,
                  NULL,
                  &mKeyMonitorEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    // Set keyboard timer to fire every 50ms for responsive key detection
    Status = gBS->SetTimer (
                    mKeyMonitorEvent,
                    TimerPeriodic,
                    500000  // 50ms in 100ns units
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set timer for mKeyMonitorEvent: %r\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to create mKeyMonitorEvent: %r\n", Status));
  }
  
  mLogoDisplayed = TRUE;
}

**/

/**
  Load a platform logo image and return its data and attributes.

  @param This              The pointer to this protocol instance.
  @param Instance          The visible image instance is found.
  @param Image             Points to the image.
  @param Attribute         The display attributes of the image returned.
  @param OffsetX           The X offset of the image regarding the Attribute.
  @param OffsetY           The Y offset of the image regarding the Attribute.

  @retval EFI_SUCCESS      The image was fetched successfully.
  @retval EFI_NOT_FOUND    The specified image could not be found.
**/

/**
EFI_STATUS
EFIAPI
GetImage (
  IN     EDKII_PLATFORM_LOGO_PROTOCOL        *This,
  IN OUT UINT32                              *Instance,
  OUT EFI_IMAGE_INPUT                        *Image,
  OUT EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE  *Attribute,
  OUT INTN                                   *OffsetX,
  OUT INTN                                   *OffsetY
  )
{
  UINT32      Current;
  EFI_STATUS  Status;

  if ((Instance == NULL) || (Image == NULL) ||
      (Attribute == NULL) || (OffsetX == NULL) || (OffsetY == NULL))
  {
    return EFI_INVALID_PARAMETER;
  }

  Current = *Instance;
  if (Current >= ARRAY_SIZE (mLogos)) {
    return EFI_NOT_FOUND;
  }

  (*Instance)++;
  *Attribute = mLogos[Current].Attribute;
  *OffsetX   = mLogos[Current].OffsetX;
  *OffsetY   = mLogos[Current].OffsetY;
  
  // Get image from HII database
  Status = mHiiImageEx->GetImageEx (mHiiImageEx, mHiiHandle, mLogos[Current].ImageId, Image);
  
  // After first logo is displayed, setup timing
  if (Current == 0 && !EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Logo displayed, setting up timing\n"));
    SetupLogoTiming ();
  }
  
  return Status;
}

EDKII_PLATFORM_LOGO_PROTOCOL  mPlatformLogo = {
  GetImage
};


**/
/**
  Cleanup function called when driver is unloaded.

  @param ImageHandle    The handle of the image being unloaded.

  @retval EFI_SUCCESS   The image was unloaded successfully.
**/

/**
EFI_STATUS
EFIAPI
LogoDriverUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
  // Close events if they exist
  if (mTimerEvent != NULL) {
    gBS->CloseEvent (mTimerEvent);
    mTimerEvent = NULL;
  }
  
  if (mKeyMonitorEvent != NULL) {
    gBS->CloseEvent (mKeyMonitorEvent);
    mKeyMonitorEvent = NULL;
  }
  
  if (mBiosSetupKeyEvent != NULL) {
    gBS->CloseEvent (mBiosSetupKeyEvent);
    mBiosSetupKeyEvent = NULL;
  }
  
  if (mMouseEvent != NULL) {
    gBS->CloseEvent (mMouseEvent);
    mMouseEvent = NULL;
  }
  
  return EFI_SUCCESS;
}

**/

/**
  Entrypoint of this module.

  This function is the entrypoint of this module. It installs the Edkii
  Platform Logo protocol.

  @param  ImageHandle       The firmware allocated handle for the EFI image.
  @param  SystemTable       A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
**/

/**
EFI_STATUS
EFIAPI
InitializeLogo (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                   Status;
  EFI_HII_PACKAGE_LIST_HEADER  *PackageList;
  EFI_HII_DATABASE_PROTOCOL    *HiiDatabase;
  EFI_HANDLE                   Handle;

  DEBUG ((DEBUG_INFO, "Logo Driver: Initializing with mouse support...\n"));

  // Initialize global variables
  mInBiosSetup = FALSE;
  mSelectedMenuItem = 0;
  mTotalMenuItems = ARRAY_SIZE (mBiosSetupMenu);

  // Locate HII Database Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiDatabaseProtocolGuid,
                  NULL,
                  (VOID **)&HiiDatabase
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Database Protocol: %r\n", Status));
    return Status;
  }

  // Locate HII Image Ex Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiImageExProtocolGuid,
                  NULL,
                  (VOID **)&mHiiImageEx
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Image Ex Protocol: %r\n", Status));
    return Status;
  }

  // Retrieve HII package list from ImageHandle
  Status = gBS->OpenProtocol (
                  ImageHandle,
                  &gEfiHiiPackageListProtocolGuid,
                  (VOID **)&PackageList,
                  ImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "HII Image Package with logo not found in PE/COFF resource section\n"));
    return Status;
  }

  // Publish HII package list to HII Database
  Status = HiiDatabase->NewPackageList (
                          HiiDatabase,
                          PackageList,
                          NULL,
                          &mHiiHandle
                          );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to add package list to HII Database: %r\n", Status));
    return Status;
  }

  // Install Platform Logo Protocol
  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEdkiiPlatformLogoProtocolGuid,
                  &mPlatformLogo,
                  NULL
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to install Platform Logo Protocol: %r\n", Status));
    // Cleanup HII handle
    HiiDatabase->RemovePackageList (HiiDatabase, mHiiHandle);
    return Status;
  }

  DEBUG ((DEBUG_INFO, "Logo Driver: Initialized successfully with mouse support\n"));
  
  return EFI_SUCCESS;
}

**/




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



//Enhanced BIOS setup utility with nested submenus - mouse and keyboard support

/** @file
  Logo DXE Driver with F12 key BIOS setup functionality.
  Shows logo, waits for F12 key, then displays BIOS setup form with navigation.
  Enhanced with mouse support and nested submenus for Main/Security/Advanced/UEFI Drivers.

Copyright (c) 2016 - 2017, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent
**/

/**

#include <Uefi.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/HiiImageEx.h>
#include <Protocol/PlatformLogo.h>
#include <Protocol/HiiPackageList.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/SimpleTextOut.h>
#include <Protocol/SimplePointer.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>

typedef struct {
  EFI_IMAGE_ID                             ImageId;
  EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE    Attribute;
  INTN                                     OffsetX;
  INTN                                     OffsetY;
} LOGO_ENTRY;

// Menu item structure
typedef struct {
  CHAR16  *MenuText;
  CHAR16  *Description;
  UINT32  StartRow;    // Row where menu item starts
  UINT32  StartCol;    // Column where menu item starts  
  UINT32  EndRow;      // Row where menu item ends
  UINT32  EndCol;      // Column where menu item ends
} MENU_ITEM;

// Mouse state structure
typedef struct {
  BOOLEAN  LeftButtonPressed;
  BOOLEAN  LeftButtonReleased;
  BOOLEAN  WasLeftButtonPressed;
  UINT32   ClickCount;
  UINT64   LastClickTime;
  INT32    LastX;
  INT32    LastY;
} MOUSE_STATE;

// Menu level enumeration
typedef enum {
  MENU_LEVEL_MAIN_MENU = 0,
  MENU_LEVEL_MAIN_SUBMENU,
  MENU_LEVEL_SECURITY_SUBMENU,
  MENU_LEVEL_ADVANCED_SUBMENU,
  MENU_LEVEL_UEFI_SUBMENU
} MENU_LEVEL;

// Global variables
EFI_HII_IMAGE_EX_PROTOCOL  *mHiiImageEx;
EFI_HII_HANDLE             mHiiHandle;
EFI_EVENT                  mKeyMonitorEvent;
EFI_EVENT                  mTimerEvent;
EFI_EVENT                  mBiosSetupKeyEvent;
EFI_EVENT                  mMouseEvent;
EFI_SIMPLE_POINTER_PROTOCOL *mSimplePointer = NULL;
BOOLEAN                    mLogoDisplayed = FALSE;
BOOLEAN                    mMessageShown = FALSE;
BOOLEAN                    mInBiosSetup = FALSE;
UINTN                      mTimerCounter = 0;
UINTN                      mSelectedMenuItem = 0;
UINTN                      mTotalMenuItems = 0;
MENU_LEVEL                 mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;
MOUSE_STATE                mMouseState;

// Main menu items
MENU_ITEM mMainBiosMenu[] = {
  { L"Main", L"System Information and Basic Settings", 10, 10, 10, 25, },
  { L"Security", L"Security and Password Settings", 11, 10, 11, 30 },
  { L"Advanced", L"Advanced Configuration Options", 12, 10, 12, 35 },
  { L"UEFI Drivers", L"Manage UEFI Driver Configuration", 13, 10, 13, 40 },
  { L"Save & Exit", L"Save changes and exit setup", 14, 10, 14, 35 },
  { L"Exit Without Saving", L"Exit without saving changes", 15, 10, 15, 45 }
};

// Main submenu items
MENU_ITEM mMainSubmenu[] = {
  { L"System Information", L"View system hardware information", 10, 12, 10, 42 },
  { L"BIOS Event Log", L"View and manage BIOS event logs", 11, 12, 11, 40 },
  { L"Update System BIOS", L"Update system BIOS firmware", 12, 12, 12, 42 },
  { L"Change Date and Time", L"Modify system date and time", 13, 12, 13, 45 },
  { L"Save Changes and Exit", L"Save settings and return to main menu", 14, 12, 14, 50 }
};

// Security submenu items
MENU_ITEM mSecuritySubmenu[] = {
  { L"Administrator Tools", L"Administrative security tools", 10, 12, 10, 42 },
  { L"Create BIOS Administrator Password", L"Set BIOS administrator password", 11, 12, 11, 55 },
  { L"TPM Embedded Security", L"Configure TPM security settings", 12, 12, 12, 48 },
  { L"Return to Main Menu", L"Go back to main BIOS menu", 13, 12, 13, 42 }
};

// Advanced submenu items
MENU_ITEM mAdvancedSubmenu[] = {
  { L"Boot Options", L"Configure boot device settings", 10, 12, 10, 35 },
  { L"Port Options", L"Configure I/O port settings", 11, 12, 11, 35 },
  { L"System Options", L"Configure system-level options", 12, 12, 12, 38 },
  { L"Return to Main Menu", L"Go back to main BIOS menu", 13, 12, 13, 42 }
};

// UEFI Drivers submenu items
MENU_ITEM mUefiSubmenu[] = {
  { L"3rd Party Option ROM Management", L"Manage third-party option ROMs", 10, 12, 10, 60 },
  { L"Return to Main Menu", L"Go back to main BIOS menu", 11, 12, 11, 42 }
};

// Function declarations
VOID DisplayBiosSetupForm (VOID);
VOID GetCurrentTime (OUT CHAR16 *TimeString);
VOID HandleBiosSetupNavigation (VOID);
VOID DisplaySelectedMenuInfo (VOID);
EFI_STATUS InitializeMouse (VOID);
VOID UpdateMouseState (VOID);
UINTN GetMenuItemFromMousePosition (IN INT32 MouseX, IN INT32 MouseY);
VOID HandleMenuSelection (VOID);
VOID DisplayCurrentMenu (VOID);
MENU_ITEM* GetCurrentMenuArray (OUT UINTN *ItemCount);

LOGO_ENTRY mLogos[] = {
  {
    IMAGE_TOKEN (IMG_LOGO),
    EdkiiPlatformLogoDisplayAttributeCenter,
    0,
    0
  }
};


**/
/**
  Get current menu array based on menu level.

  @param ItemCount   Output parameter for number of items in menu.

  @retval Pointer to current menu array.
**/

/**
MENU_ITEM*
GetCurrentMenuArray (
  OUT UINTN *ItemCount
  )
{
  switch (mCurrentMenuLevel) {
    case MENU_LEVEL_MAIN_SUBMENU:
      *ItemCount = ARRAY_SIZE (mMainSubmenu);
      return mMainSubmenu;
      
    case MENU_LEVEL_SECURITY_SUBMENU:
      *ItemCount = ARRAY_SIZE (mSecuritySubmenu);
      return mSecuritySubmenu;
      
    case MENU_LEVEL_ADVANCED_SUBMENU:
      *ItemCount = ARRAY_SIZE (mAdvancedSubmenu);
      return mAdvancedSubmenu;
      
    case MENU_LEVEL_UEFI_SUBMENU:
      *ItemCount = ARRAY_SIZE (mUefiSubmenu);
      return mUefiSubmenu;
      
    default: // MENU_LEVEL_MAIN_MENU
      *ItemCount = ARRAY_SIZE (mMainBiosMenu);
      return mMainBiosMenu;
  }
}


**/
/**
  Initialize mouse support by locating Simple Pointer Protocol.

  @retval EFI_SUCCESS   Mouse initialized successfully.
  @retval Others        Failed to initialize mouse.
**/

/**
EFI_STATUS
InitializeMouse (
  VOID
  )
{
  EFI_STATUS Status;
  
  // Initialize mouse state with center screen position
  mMouseState.LeftButtonPressed = FALSE;
  mMouseState.LeftButtonReleased = FALSE;
  mMouseState.WasLeftButtonPressed = FALSE;
  mMouseState.ClickCount = 0;
  mMouseState.LastClickTime = 0;
  mMouseState.LastX = 40; // Center of 80-column screen
  mMouseState.LastY = 12; // Center of 25-row screen
  
  // Try to locate Simple Pointer Protocol
  Status = gBS->LocateProtocol (
                  &gEfiSimplePointerProtocolGuid,
                  NULL,
                  (VOID **)&mSimplePointer
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "Simple Pointer Protocol not available: %r\n", Status));
    DEBUG ((DEBUG_WARN, "Mouse support disabled - using keyboard only\n"));
    mSimplePointer = NULL;
    return Status;
  }
  
  // Reset mouse with extended reset
  Status = mSimplePointer->Reset (mSimplePointer, TRUE);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "Failed to reset mouse: %r\n", Status));
    // Try without extended verification
    Status = mSimplePointer->Reset (mSimplePointer, FALSE);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Mouse reset failed completely: %r\n", Status));
      mSimplePointer = NULL;
      return Status;
    }
  }
  
  DEBUG ((DEBUG_INFO, "Mouse initialized successfully\n"));
  DEBUG ((DEBUG_INFO, "Mouse Resolution: X=%d, Y=%d, Z=%d\n", 
          mSimplePointer->Mode->ResolutionX,
          mSimplePointer->Mode->ResolutionY, 
          mSimplePointer->Mode->ResolutionZ));
  
  return EFI_SUCCESS;
}

**/

/**
  Update mouse state by reading current mouse input.
**/

/**
VOID
UpdateMouseState (
  VOID
  )
{
  EFI_STATUS                Status;
  EFI_SIMPLE_POINTER_STATE  MouseState;
  
  if (mSimplePointer == NULL) {
    return;
  }
  
  Status = mSimplePointer->GetState (mSimplePointer, &MouseState);
  if (EFI_ERROR (Status)) {
    // No mouse input available right now
    return;
  }
  
  // Handle relative movement with better scaling
  INT32 DeltaX = MouseState.RelativeMovementX / 65536; // Adjusted scaling
  INT32 DeltaY = MouseState.RelativeMovementY / 65536; // Adjusted scaling
  
  // Apply movement if significant enough
  if (DeltaX != 0 || DeltaY != 0) {
    mMouseState.LastX += DeltaX;
    mMouseState.LastY += DeltaY;
    
    // Boundary check for 80x25 console
    if (mMouseState.LastX < 0) mMouseState.LastX = 0;
    if (mMouseState.LastY < 0) mMouseState.LastY = 0;
    if (mMouseState.LastX > 79) mMouseState.LastX = 79;
    if (mMouseState.LastY > 24) mMouseState.LastY = 24;
    
    DEBUG ((DEBUG_VERBOSE, "Mouse moved to: %d, %d (Delta: %d, %d)\n", 
            mMouseState.LastX, mMouseState.LastY, DeltaX, DeltaY));
  }
  
  // Update button state
  mMouseState.WasLeftButtonPressed = mMouseState.LeftButtonPressed;
  mMouseState.LeftButtonPressed = MouseState.LeftButton;
  
  // Detect button release (click completion)
  if (mMouseState.WasLeftButtonPressed && !mMouseState.LeftButtonPressed) {
    mMouseState.LeftButtonReleased = TRUE;
    
    // Simple click counting without time dependency
    mMouseState.ClickCount++;
    
    DEBUG ((DEBUG_INFO, "Mouse click detected at: %d, %d (Click count: %d)\n", 
            mMouseState.LastX, mMouseState.LastY, mMouseState.ClickCount));
            
    // Reset click count after a reasonable number to prevent overflow
    if (mMouseState.ClickCount > 10) {
      mMouseState.ClickCount = 1;
    }
  } else {
    mMouseState.LeftButtonReleased = FALSE;
  }
  
  // Reset click count when button is pressed (start of new click sequence)
  if (!mMouseState.WasLeftButtonPressed && mMouseState.LeftButtonPressed) {
    // Button just pressed - reset for new click sequence
    if (mMouseState.ClickCount > 2) {
      mMouseState.ClickCount = 0;
    }
  }
}


**/
/**
  Get menu item index based on mouse position.

  @param MouseX   Mouse X coordinate.
  @param MouseY   Mouse Y coordinate.

  @retval Index   Menu item index, or -1 if not over any menu item.
**/

/**
UINTN
GetMenuItemFromMousePosition (
  IN INT32 MouseX,
  IN INT32 MouseY
  )
{
  UINTN Index;
  MENU_ITEM *CurrentMenu;
  UINTN ItemCount;
  
  CurrentMenu = GetCurrentMenuArray (&ItemCount);
  
  for (Index = 0; Index < ItemCount; Index++) {
    if (MouseY == (CurrentMenu[Index].StartRow) &&
        MouseX >= CurrentMenu[Index].StartCol &&
        MouseX <= CurrentMenu[Index].EndCol) {
      return Index;
    }
  }
  
  return (UINTN)-1; // Not over any menu item
}


**/
/**
  Get current time and format it as string.

  @param TimeString   Output buffer for formatted time string.
**/

/**
VOID
GetCurrentTime (
  OUT CHAR16  *TimeString
  )
{
  EFI_TIME    Time;
  EFI_STATUS  Status;
  
  // Attempt to get the system time
  Status = gRT->GetTime (&Time, NULL);
  if (!EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "GetCurrentTime: Successfully retrieved time - %02d:%02d:%02d\n", Time.Hour, Time.Minute, Time.Second));
    UnicodeSPrint (
      TimeString, 
      50 * sizeof(CHAR16),
      L"%02d:%02d:%02d",
      Time.Hour,
      Time.Minute,
      Time.Second
    );
  } else {
    DEBUG ((DEBUG_ERROR, "GetCurrentTime: Failed to get time, Status: %r\n", Status));
    // Fallback: Display a static message or placeholder
    UnicodeSPrint (TimeString, 50 * sizeof(CHAR16), L"Time Unavailable");
  }
}


**/
/**
  Display information about the selected menu item.
**/

/**
VOID
DisplaySelectedMenuInfo (
  VOID
  )
{
  MENU_ITEM *CurrentMenu;
  UINTN ItemCount;
  
  CurrentMenu = GetCurrentMenuArray (&ItemCount);
  
  // Clear the info area
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 12);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"                                    ");
  
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 13);
  Print (L"                                    ");
  
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 14);
  Print (L"                                    ");
  
  // Display selected item info
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 12);
  Print (L"Selected: %s", CurrentMenu[mSelectedMenuItem].MenuText);
  
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 14);
  Print (L"%s", CurrentMenu[mSelectedMenuItem].Description);
}
**/
/**
  Display current menu based on menu level.
**/

/**
VOID
DisplayCurrentMenu (
  VOID
  )
{
  UINTN   Index;
  CHAR16  TimeString[50];
  MENU_ITEM *CurrentMenu;
  UINTN ItemCount;
  CHAR16 *MenuTitle;
  
  CurrentMenu = GetCurrentMenuArray (&ItemCount);
  mTotalMenuItems = ItemCount;
  
  // Determine menu title based on current level
  switch (mCurrentMenuLevel) {
    case MENU_LEVEL_MAIN_SUBMENU:
      MenuTitle = L"MAIN MENU OPTIONS";
      break;
    case MENU_LEVEL_SECURITY_SUBMENU:
      MenuTitle = L"SECURITY OPTIONS";
      break;
    case MENU_LEVEL_ADVANCED_SUBMENU:
      MenuTitle = L"ADVANCED OPTIONS";
      break;
    case MENU_LEVEL_UEFI_SUBMENU:
      MenuTitle = L"UEFI DRIVERS OPTIONS";
      break;
    default:
      MenuTitle = L"BIOS SETUP UTILITY";
      break;
  }
  
  // Clear screen
  gST->ConOut->ClearScreen (gST->ConOut);
  
  // Set white background with black text
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  //gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_CYAN);
  
  // Fill entire screen with white background
  for (Index = 0; Index < 25; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Index);
    Print (L"                                                                                ");
  }
  
  // Display "CDAC Bangalore" at top center
  gST->ConOut->SetCursorPosition (gST->ConOut, 30, 2);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"CDAC Bangalore");
  
  // Get and display current time at top right corner
  GetCurrentTime (TimeString);
  gST->ConOut->SetCursorPosition (gST->ConOut, 65, 2);
  Print (L"%s", TimeString);
  
  // Add main content area with dynamic title
  gST->ConOut->SetCursorPosition (gST->ConOut, 25, 5);
  Print (L"====================================");
  gST->ConOut->SetCursorPosition (gST->ConOut, 25 + (34 - StrLen(MenuTitle))/2, 6);
  Print (L"%s", MenuTitle);
  gST->ConOut->SetCursorPosition (gST->ConOut, 25, 7);
  Print (L"====================================");
  
  // Update menu item positions and display menu items
  for (Index = 0; Index < mTotalMenuItems; Index++) {
    // Update position information
    CurrentMenu[Index].StartRow = 10 + Index;
    if (mCurrentMenuLevel == MENU_LEVEL_MAIN_MENU) {
      CurrentMenu[Index].StartCol = 10;
    } else {
      CurrentMenu[Index].StartCol = 12; // Indent submenus
    }
    CurrentMenu[Index].EndRow = 10 + Index;
    CurrentMenu[Index].EndCol = CurrentMenu[Index].StartCol + StrLen(CurrentMenu[Index].MenuText) + 3;
    
    gST->ConOut->SetCursorPosition (gST->ConOut, CurrentMenu[Index].StartCol, 10 + Index);
    
    if (Index == mSelectedMenuItem) {
      // Highlight selected item with reverse colors
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
      Print (L"-> %s", CurrentMenu[Index].MenuText);
    } else {
      // Normal menu item
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"   %s", CurrentMenu[Index].MenuText);
    }
  }
  
  // Display selected item information
  DisplaySelectedMenuInfo ();
  
  // Show navigation instructions with breadcrumb
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  // gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_CYAN);
  gST->ConOut->SetCursorPosition (gST->ConOut, 10, 20);
  Print (L"Navigation Instructions:");
 // gST->ConOut->SetCursorPosition (gST->ConOut, 10, 21);
  //Print (L"Use UP/DOWN arrow keys or mouse to navigate");
  //gST->ConOut->SetCursorPosition (gST->ConOut, 10, 22);
  //Print (L"Press ENTER or double-click to select");
  gST->ConOut->SetCursorPosition (gST->ConOut, 10, 23);
  Print (L"Press ESC to %s", (mCurrentMenuLevel == MENU_LEVEL_MAIN_MENU) ? L"exit setup" : L"go back");
  
  // Add footer with current menu level indicator
  gST->ConOut->SetCursorPosition (gST->ConOut, 15, 24);
 // Print (L"BIOS Setup - F12 Activated - CDAC Bangalore - Mouse Enabled - Level: %d", mCurrentMenuLevel);
   Print (L"BIOS Setup - F12 Activated  - Mouse Enabled - Level: %d", mCurrentMenuLevel);
}

**/

/**
  Display BIOS setup form with white background, menu items and navigation.
**/

/**
VOID
DisplayBiosSetupForm (
  VOID
  )
{
  mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;
  mSelectedMenuItem = 0;
  mInBiosSetup = TRUE;
  
  // Initialize mouse support
  InitializeMouse ();
  
  // Display the current menu
  DisplayCurrentMenu ();
  
  // Reset console input for clean key detection in setup
  gST->ConIn->Reset (gST->ConIn, FALSE);
  
  // Start navigation handling
  HandleBiosSetupNavigation ();
}


**/
/**
  Handle menu selection based on current selected item and menu level.
**/

/**
VOID
HandleMenuSelection (
  VOID
  )
{
  switch (mCurrentMenuLevel) {
    case MENU_LEVEL_MAIN_MENU:
      switch (mSelectedMenuItem) {
        case 0: // Main
          mCurrentMenuLevel = MENU_LEVEL_MAIN_SUBMENU;
          mSelectedMenuItem = 0;
          DisplayCurrentMenu ();
          break;
          
        case 1: // Security
          mCurrentMenuLevel = MENU_LEVEL_SECURITY_SUBMENU;
          mSelectedMenuItem = 0;
          DisplayCurrentMenu ();
          break;
          
        case 2: // Advanced
          mCurrentMenuLevel = MENU_LEVEL_ADVANCED_SUBMENU;
          mSelectedMenuItem = 0;
          DisplayCurrentMenu ();
          break;
          
        case 3: // UEFI Drivers
          mCurrentMenuLevel = MENU_LEVEL_UEFI_SUBMENU;
          mSelectedMenuItem = 0;
          DisplayCurrentMenu ();
          break;
        
        case 4: // Save & Exit
          gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_GREEN | EFI_BACKGROUND_BLUE);
          Print (L"Saving and exiting...");
          gBS->Stall (2000000); // Wait 2 seconds
          
          // Show shutdown message
          gST->ConOut->ClearScreen (gST->ConOut);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
          gST->ConOut->SetCursorPosition (gST->ConOut, 25, 12);
          Print (L"Settings saved. Shutting down system...");
          gBS->Stall (2000000); // Wait 2 seconds
          
          // Shutdown the system
          gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
          mInBiosSetup = FALSE;
          break;
          
        case 5: // Exit Without Saving
          gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_RED | EFI_BACKGROUND_BLUE);
          Print (L"Exiting without saving...");
          gBS->Stall (2000000); // Wait 2 seconds
          
          // Show shutdown message
          gST->ConOut->ClearScreen (gST->ConOut);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
          gST->ConOut->SetCursorPosition (gST->ConOut, 25, 12);
          Print (L"Exiting setup. Shutting down system...");
          gBS->Stall (2000000); // Wait 2 seconds
          
          // Shutdown the system
          gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
          mInBiosSetup = FALSE;
          break;
      }
      break;
      
    case MENU_LEVEL_MAIN_SUBMENU:
      switch (mSelectedMenuItem) {
        case 0: // System Information
          gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
          Print (L"Displaying System Info...");
          gBS->Stall (1500000);
          break;
          
        case 1: // BIOS Event Log
          gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
          Print (L"Opening Event Log...");
          gBS->Stall (1500000);
          break;
          
        case 2: // Update System BIOS
          gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
          Print (L"BIOS Update Utility...");
          gBS->Stall (1500000);
          break;
          
        case 3: // Change Date and Time
          gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
          Print (L"Date/Time Settings...");
          gBS->Stall (1500000);
          break;
          
        case 4: // Save Changes and Exit
          mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;
          mSelectedMenuItem = 0;
          DisplayCurrentMenu ();
          break;
      }
      break;
      
    case MENU_LEVEL_SECURITY_SUBMENU:
      switch (mSelectedMenuItem) {
        case 0: // Administrator Tools
          gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
          Print (L"Admin Tools Loading...");
          gBS->Stall (1500000);
          break;
          
        case 1: // Create BIOS Administrator Password
          gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
          Print (L"Password Setup...");
          gBS->Stall (1500000);
          break;
          
        case 2: // TPM Embedded Security
          gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
          Print (L"TPM Security Config...");
          gBS->Stall (1500000);
          break;
          
        case 3: // Return to Main Menu
          mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;
          mSelectedMenuItem = 1; // Return to Security item
          DisplayCurrentMenu ();
          break;
      }
      break;
      
    case MENU_LEVEL_ADVANCED_SUBMENU:
      switch (mSelectedMenuItem) {
        case 0: // Boot Options
          gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
          Print (L"Boot Options Config...");
          gBS->Stall (1500000);
          break;
          
        case 1: // Port Options
          gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
          Print (L"I/O Port Configuration...");
          gBS->Stall (1500000);
          break;
          
        case 2: // System Options
          gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
          Print (L"System Options Config...");
          gBS->Stall (1500000);
          break;
          
        case 3: // Return to Main Menu
          mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;
          mSelectedMenuItem = 2; // Return to Advanced item
          DisplayCurrentMenu ();
          break;
      }
      break;
      
    case MENU_LEVEL_UEFI_SUBMENU:
      switch (mSelectedMenuItem) {
        case 0: // 3rd Party Option ROM Management
          gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
          Print (L"Option ROM Manager...");
          gBS->Stall (1500000);
          break;
          
        case 1: // Return to Main Menu
          mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;
          mSelectedMenuItem = 3; // Return to UEFI Drivers item
          DisplayCurrentMenu ();
          break;
      }
      break;
      
    default:
      break;
  }
  
  // If still in BIOS setup and not changing menus, show feedback and return
  if (mInBiosSetup && mCurrentMenuLevel != MENU_LEVEL_MAIN_MENU) {
    gBS->Stall (1000000); // Wait 1 second to show selection message
    // Clear the feedback message
    gST->ConOut->SetCursorPosition (gST->ConOut, 45, 16);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    Print (L"                           ");
  }
}

**/

/**
  Handle navigation within BIOS setup using arrow keys and mouse.
  Enhanced with submenu support and better mouse handling.
**/

/**
VOID
HandleBiosSetupNavigation (
  VOID
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  UINTN          PreviousSelection;
  UINTN          MouseMenuIndex;
  STATIC UINTN   ConsecutiveClicks = 0;
  STATIC UINTN   LastClickedItem = (UINTN)-1;
  MENU_ITEM      *CurrentMenu;
  UINTN          ItemCount;
  
  while (mInBiosSetup) {
    // Get current menu info
    CurrentMenu = GetCurrentMenuArray (&ItemCount);
    
    // Display mouse status
    if (mSimplePointer != NULL) {
      gST->ConOut->SetCursorPosition (gST->ConOut, 45, 18);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"Mouse: Active at (%d,%d)", mMouseState.LastX, mMouseState.LastY);
    } else {
      gST->ConOut->SetCursorPosition (gST->ConOut, 45, 18);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"Mouse: Not Available     ");
    }
    
    // Update mouse state first
    UpdateMouseState ();
    
    // Handle mouse input
    if (mSimplePointer != NULL) {
      MouseMenuIndex = GetMenuItemFromMousePosition (mMouseState.LastX, mMouseState.LastY);
      
      // Update mouse position display for debugging
      gST->ConOut->SetCursorPosition (gST->ConOut, 45, 18);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"Mouse: (%02d,%02d) Item:%d", mMouseState.LastX, mMouseState.LastY, 
             (MouseMenuIndex == (UINTN)-1) ? 99 : MouseMenuIndex);
      
      // Handle mouse clicks
      if (mMouseState.LeftButtonReleased) {
        DEBUG ((DEBUG_INFO, "Mouse click at (%d,%d), Menu item: %d\n", 
                mMouseState.LastX, mMouseState.LastY, MouseMenuIndex));
        
        if (MouseMenuIndex != (UINTN)-1 && MouseMenuIndex < mTotalMenuItems) {
          // Check for consecutive clicks on same item
          if (LastClickedItem == MouseMenuIndex) {
            ConsecutiveClicks++;
          } else {
            ConsecutiveClicks = 1;
            LastClickedItem = MouseMenuIndex;
          }
          
          // First click - select menu item
          if (ConsecutiveClicks == 1) {
            PreviousSelection = mSelectedMenuItem;
            mSelectedMenuItem = MouseMenuIndex;
            
            // Update display if selection changed
            if (PreviousSelection != mSelectedMenuItem) {
              // Update previous item display (remove highlight)
              gST->ConOut->SetCursorPosition (gST->ConOut, CurrentMenu[PreviousSelection].StartCol, 10 + PreviousSelection);
              gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
              Print (L"   %s", CurrentMenu[PreviousSelection].MenuText);
              
              // Update current item display (add highlight)
              gST->ConOut->SetCursorPosition (gST->ConOut, CurrentMenu[mSelectedMenuItem].StartCol, 10 + mSelectedMenuItem);
              gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
              Print (L"-> %s", CurrentMenu[mSelectedMenuItem].MenuText);
              
              // Update selected item information
              DisplaySelectedMenuInfo ();
            }
            
            // Show click feedback
            gST->ConOut->SetCursorPosition (gST->ConOut, 45, 19);
            gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
            Print (L"Click 1: Selected       ");
          }
          
          // Second click - execute selection (double-click simulation)
          else if (ConsecutiveClicks >= 2) {
            mSelectedMenuItem = MouseMenuIndex;
            
            // Show execution feedback
            gST->ConOut->SetCursorPosition (gST->ConOut, 45, 19);
            gST->ConOut->SetAttribute (gST->ConOut, EFI_GREEN | EFI_BACKGROUND_BLUE);
            Print (L"Click 2: Executing...   ");
            gBS->Stall (500000); // Brief pause to show feedback
            
            HandleMenuSelection ();
            ConsecutiveClicks = 0;
            LastClickedItem = (UINTN)-1;
            
            if (!mInBiosSetup) {
              return; // Exit if user chose to exit
            }
          }
        } else {
          // Click outside menu area - reset click counter
          ConsecutiveClicks = 0;
          LastClickedItem = (UINTN)-1;
          
          gST->ConOut->SetCursorPosition (gST->ConOut, 45, 19);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_RED | EFI_BACKGROUND_LIGHTGRAY);
          Print (L"Click: Outside menu     ");
        }
        
        // Reset mouse click state
        mMouseState.LeftButtonReleased = FALSE;
        mMouseState.ClickCount = 0;
      }
    }
    
    // Handle keyboard input
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
    
    if (!EFI_ERROR (Status)) {
      PreviousSelection = mSelectedMenuItem;
      
      switch (Key.ScanCode) {
        case SCAN_UP:
          // Move up in menu
          if (mSelectedMenuItem > 0) {
            mSelectedMenuItem--;
          } else {
            mSelectedMenuItem = mTotalMenuItems - 1; // Wrap to bottom
          }
          ConsecutiveClicks = 0; // Reset mouse click counter
          LastClickedItem = (UINTN)-1;
          break;
          
        case SCAN_DOWN:
          // Move down in menu
          if (mSelectedMenuItem < mTotalMenuItems - 1) {
            mSelectedMenuItem++;
          } else {
            mSelectedMenuItem = 0; // Wrap to top
          }
          ConsecutiveClicks = 0; // Reset mouse click counter
          LastClickedItem = (UINTN)-1;
          break;
          
        case SCAN_ESC:
          // Handle ESC based on menu level
          if (mCurrentMenuLevel == MENU_LEVEL_MAIN_MENU) {
            // Exit BIOS setup with shutdown
            mInBiosSetup = FALSE;
            gST->ConOut->ClearScreen (gST->ConOut);
            gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
            gST->ConOut->SetCursorPosition (gST->ConOut, 25, 12);
            Print (L"Exiting BIOS Setup. Shutting down system...");
            gBS->Stall (2000000); // Wait 2 seconds
            
            // Shutdown instead of returning to boot process
            gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
            return;
          } else {
            // Go back to main menu from submenu
            UINTN PrevLevel = mCurrentMenuLevel;
            mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;
            
            // Set selection based on which submenu we're coming from
            switch (PrevLevel) {
              case MENU_LEVEL_MAIN_SUBMENU:
                mSelectedMenuItem = 0;
                break;
              case MENU_LEVEL_SECURITY_SUBMENU:
                mSelectedMenuItem = 1;
                break;
              case MENU_LEVEL_ADVANCED_SUBMENU:
                mSelectedMenuItem = 2;
                break;
              case MENU_LEVEL_UEFI_SUBMENU:
                mSelectedMenuItem = 3;
                break;
              default:
                mSelectedMenuItem = 0;
                break;
            }
            
            DisplayCurrentMenu ();
            ConsecutiveClicks = 0;
            LastClickedItem = (UINTN)-1;
          }
          break;
          
        default:
          // Check for ENTER key (Unicode character)
          if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
            HandleMenuSelection ();
            if (!mInBiosSetup) {
              return; // Exit if user chose to exit (will shutdown in HandleMenuSelection)
            }
          }
          break;
      }
      
      // Update display if selection changed via keyboard
      if (mInBiosSetup && (PreviousSelection != mSelectedMenuItem)) {
        // Get current menu again in case it changed
        CurrentMenu = GetCurrentMenuArray (&ItemCount);
        
        // Update previous item display (remove highlight)
        gST->ConOut->SetCursorPosition (gST->ConOut, CurrentMenu[PreviousSelection].StartCol, 10 + PreviousSelection);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"   %s", CurrentMenu[PreviousSelection].MenuText);
        
        // Update current item display (add highlight)
        gST->ConOut->SetCursorPosition (gST->ConOut, CurrentMenu[mSelectedMenuItem].StartCol, 10 + mSelectedMenuItem);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
        Print (L"-> %s", CurrentMenu[mSelectedMenuItem].MenuText);
        
        // Update selected item information
        DisplaySelectedMenuInfo ();
      }
    } else {
      // Small delay to prevent excessive CPU usage
      gBS->Stall (50000); // 50ms
    }
  }
}


**/
/**
  Monitor keyboard input for F12 key press only.
  Modified to shutdown after BIOS setup completion.
**/

/**
VOID
EFIAPI
KeyboardMonitor (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  
  // Only monitor after message is shown and not in BIOS setup
  if (!mMessageShown || mInBiosSetup) {
    return;
  }
  
  // Check if key is available
  Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
  
  if (!EFI_ERROR (Status)) {
    // Check for F12 key only
    if (Key.ScanCode == SCAN_F12) {
      DEBUG ((DEBUG_INFO, "F12 key pressed - showing BIOS setup form\n"));
      
      // Cancel timer and keyboard events to stop normal flow
      gBS->SetTimer (mKeyMonitorEvent, TimerCancel, 0);
      gBS->SetTimer (mTimerEvent, TimerCancel, 0);
      
      // Display BIOS setup form
      DisplayBiosSetupForm ();
      
      // After exiting BIOS setup, shutdown
      gST->ConOut->ClearScreen (gST->ConOut);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
      gST->ConOut->SetCursorPosition (gST->ConOut, 25, 12);
      Print (L"BIOS Setup completed. Shutting down system...");
      gBS->Stall (2000000); // Wait 2 seconds
      
      // Shutdown the system
      gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
      
      return;
    }
  }
}


**/
/**
  Timer callback function to handle logo display timing.

  @param Event      The event that is signaled.
  @param Context    The context passed to the event.
**/

/**
VOID
EFIAPI
TimerCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  mTimerCounter++;
  
  // After 5 seconds, show the message below logo and wait for F12
  if (mTimerCounter == 50 && !mMessageShown) { // 50 * 100ms = 5 seconds
    // Clear the entire screen first
    gST->ConOut->ClearScreen (gST->ConOut);
    
    // Position cursor below the logo area
    gST->ConOut->SetCursorPosition (gST->ConOut, 18, 18);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
    Print (L"Press F12 key for BIOS setup");
    
    mMessageShown = TRUE;
    
    // Reset console input for clean key detection
    gST->ConIn->Reset (gST->ConIn, FALSE);
    
    DEBUG ((DEBUG_INFO, "Message displayed, waiting for F12 key\n"));
  }
  
  // Wait indefinitely for F12 key press, no shell redirect
}

**/

/**
  Setup timing and keyboard monitoring after logo is displayed.
**/

/**
VOID
SetupLogoTiming (
  VOID
  )
{
  EFI_STATUS  Status;
  
  if (mLogoDisplayed) {
    return; // Already setup
  }
  
  DEBUG ((DEBUG_INFO, "Setting up logo timing and keyboard monitoring\n"));
  
  // Create timer event for logo timing
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  TimerCallback,
                  NULL,
                  &mTimerEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    // Set timer to fire every 100ms
    Status = gBS->SetTimer (
                    mTimerEvent,
                    TimerPeriodic,
                    1000000  // 100ms in 100ns units
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set timer for mTimerEvent: %r\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to create mTimerEvent: %r\n", Status));
  }
  
  // Create keyboard monitoring event
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  KeyboardMonitor,
                  NULL,
                  &mKeyMonitorEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    // Set keyboard timer to fire every 50ms for responsive key detection
    Status = gBS->SetTimer (
                    mKeyMonitorEvent,
                    TimerPeriodic,
                    500000  // 50ms in 100ns units
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set timer for mKeyMonitorEvent: %r\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to create mKeyMonitorEvent: %r\n", Status));
  }
  
  mLogoDisplayed = TRUE;
}

**/

/**
  Load a platform logo image and return its data and attributes.

  @param This              The pointer to this protocol instance.
  @param Instance          The visible image instance is found.
  @param Image             Points to the image.
  @param Attribute         The display attributes of the image returned.
  @param OffsetX           The X offset of the image regarding the Attribute.
  @param OffsetY           The Y offset of the image regarding the Attribute.

  @retval EFI_SUCCESS      The image was fetched successfully.
  @retval EFI_NOT_FOUND    The specified image could not be found.
**/

/**
EFI_STATUS
EFIAPI
GetImage (
  IN     EDKII_PLATFORM_LOGO_PROTOCOL        *This,
  IN OUT UINT32                              *Instance,
  OUT EFI_IMAGE_INPUT                        *Image,
  OUT EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE  *Attribute,
  OUT INTN                                   *OffsetX,
  OUT INTN                                   *OffsetY
  )
{
  UINT32      Current;
  EFI_STATUS  Status;

  if ((Instance == NULL) || (Image == NULL) ||
      (Attribute == NULL) || (OffsetX == NULL) || (OffsetY == NULL))
  {
    return EFI_INVALID_PARAMETER;
  }

  Current = *Instance;
  if (Current >= ARRAY_SIZE (mLogos)) {
    return EFI_NOT_FOUND;
  }

  (*Instance)++;
  *Attribute = mLogos[Current].Attribute;
  *OffsetX   = mLogos[Current].OffsetX;
  *OffsetY   = mLogos[Current].OffsetY;
  
  // Get image from HII database
  Status = mHiiImageEx->GetImageEx (mHiiImageEx, mHiiHandle, mLogos[Current].ImageId, Image);
  
  // After first logo is displayed, setup timing
  if (Current == 0 && !EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Logo displayed, setting up timing\n"));
    SetupLogoTiming ();
  }
  
  return Status;
}

EDKII_PLATFORM_LOGO_PROTOCOL  mPlatformLogo = {
  GetImage
};

**/

/**
  Cleanup function called when driver is unloaded.

  @param ImageHandle    The handle of the image being unloaded.

  @retval EFI_SUCCESS   The image was unloaded successfully.
**/

/**
EFI_STATUS
EFIAPI
LogoDriverUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
  // Close events if they exist
  if (mTimerEvent != NULL) {
    gBS->CloseEvent (mTimerEvent);
    mTimerEvent = NULL;
  }
  
  if (mKeyMonitorEvent != NULL) {
    gBS->CloseEvent (mKeyMonitorEvent);
    mKeyMonitorEvent = NULL;
  }
  
  if (mBiosSetupKeyEvent != NULL) {
    gBS->CloseEvent (mBiosSetupKeyEvent);
    mBiosSetupKeyEvent = NULL;
  }
  
  if (mMouseEvent != NULL) {
    gBS->CloseEvent (mMouseEvent);
    mMouseEvent = NULL;
  }
  
  return EFI_SUCCESS;
}


**/
/**
  Entrypoint of this module.

  This function is the entrypoint of this module. It installs the Edkii
  Platform Logo protocol.

  @param  ImageHandle       The firmware allocated handle for the EFI image.
  @param  SystemTable       A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
**/

/**
EFI_STATUS
EFIAPI
InitializeLogo (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                   Status;
  EFI_HII_PACKAGE_LIST_HEADER  *PackageList;
  EFI_HII_DATABASE_PROTOCOL    *HiiDatabase;
  EFI_HANDLE                   Handle;

  DEBUG ((DEBUG_INFO, "Logo Driver: Initializing with nested menu support...\n"));

  // Initialize global variables
  mInBiosSetup = FALSE;
  mSelectedMenuItem = 0;
  mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;
  mTotalMenuItems = ARRAY_SIZE (mMainBiosMenu);

  // Locate HII Database Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiDatabaseProtocolGuid,
                  NULL,
                  (VOID **)&HiiDatabase
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Database Protocol: %r\n", Status));
    return Status;
  }

  // Locate HII Image Ex Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiImageExProtocolGuid,
                  NULL,
                  (VOID **)&mHiiImageEx
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Image Ex Protocol: %r\n", Status));
    return Status;
  }

  // Retrieve HII package list from ImageHandle
  Status = gBS->OpenProtocol (
                  ImageHandle,
                  &gEfiHiiPackageListProtocolGuid,
                  (VOID **)&PackageList,
                  ImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "HII Image Package with logo not found in PE/COFF resource section\n"));
    return Status;
  }

  // Publish HII package list to HII Database
  Status = HiiDatabase->NewPackageList (
                          HiiDatabase,
                          PackageList,
                          NULL,
                          &mHiiHandle
                          );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to add package list to HII Database: %r\n", Status));
    return Status;
  }

  // Install Platform Logo Protocol
  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEdkiiPlatformLogoProtocolGuid,
                  &mPlatformLogo,
                  NULL
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to install Platform Logo Protocol: %r\n", Status));
    // Cleanup HII handle
    HiiDatabase->RemovePackageList (HiiDatabase, mHiiHandle);
    return Status;
  }

  DEBUG ((DEBUG_INFO, "Logo Driver: Initialized successfully with nested menu support\n"));
  
  return EFI_SUCCESS;
}

**/




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// This is working perfectly but in this menu shows in vertically.

//Enhanced BIOS setup utility with nested submenus - mouse and keyboard support

/** @file
  Logo DXE Driver with F12 key BIOS setup functionality.
  Shows logo, waits for F12 key, then displays BIOS setup form with navigation.
  Enhanced with mouse support and nested submenus for Main/Security/Advanced/UEFI Drivers.

Copyright (c) 2016 - 2017, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent
**/

/**

#include <Uefi.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/HiiImageEx.h>
#include <Protocol/PlatformLogo.h>
#include <Protocol/HiiPackageList.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/SimpleTextOut.h>
#include <Protocol/SimplePointer.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>

typedef struct {
  EFI_IMAGE_ID                             ImageId;
  EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE    Attribute;
  INTN                                     OffsetX;
  INTN                                     OffsetY;
} LOGO_ENTRY;

// Menu item structure
typedef struct {
  CHAR16  *MenuText;
  CHAR16  *Description;
  UINT32  StartRow;    // Row where menu item starts
  UINT32  StartCol;    // Column where menu item starts  
  UINT32  EndRow;      // Row where menu item ends
  UINT32  EndCol;      // Column where menu item ends
} MENU_ITEM;

// Mouse state structure
typedef struct {
  BOOLEAN  LeftButtonPressed;
  BOOLEAN  LeftButtonReleased;
  BOOLEAN  WasLeftButtonPressed;
  UINT32   ClickCount;
  UINT64   LastClickTime;
  INT32    LastX;
  INT32    LastY;
} MOUSE_STATE;

// Menu level enumeration
typedef enum {
  MENU_LEVEL_MAIN_MENU = 0,
  MENU_LEVEL_MAIN_SUBMENU,
  MENU_LEVEL_SECURITY_SUBMENU,
  MENU_LEVEL_ADVANCED_SUBMENU,
  MENU_LEVEL_UEFI_SUBMENU
} MENU_LEVEL;

// Global variables
EFI_HII_IMAGE_EX_PROTOCOL  *mHiiImageEx;
EFI_HII_HANDLE             mHiiHandle;
EFI_EVENT                  mKeyMonitorEvent;
EFI_EVENT                  mTimerEvent;
EFI_EVENT                  mBiosSetupKeyEvent;
EFI_EVENT                  mMouseEvent;
EFI_SIMPLE_POINTER_PROTOCOL *mSimplePointer = NULL;
BOOLEAN                    mLogoDisplayed = FALSE;
BOOLEAN                    mMessageShown = FALSE;
BOOLEAN                    mInBiosSetup = FALSE;
UINTN                      mTimerCounter = 0;
UINTN                      mSelectedMenuItem = 0;
UINTN                      mTotalMenuItems = 0;
MENU_LEVEL                 mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;
MOUSE_STATE                mMouseState;

// Main menu items
MENU_ITEM mMainBiosMenu[] = {
  { L"Main", L"System Information and Basic Settings", 10, 10, 10, 25, },
  { L"Security", L"Security and Password Settings", 11, 10, 11, 30 },
  { L"Advanced", L"Advanced Configuration Options", 12, 10, 12, 35 },
  { L"UEFI Drivers", L"Manage UEFI Driver Configuration", 13, 10, 13, 40 },
  { L"Save & Exit", L"Save changes and exit setup", 14, 10, 14, 35 },
  { L"Exit Without Saving", L"Exit without saving changes", 15, 10, 15, 45 }
};

// Main submenu items
MENU_ITEM mMainSubmenu[] = {
  { L"System Information", L"View system hardware information", 10, 12, 10, 42 },
  { L"BIOS Event Log", L"View and manage BIOS event logs", 11, 12, 11, 40 },
  { L"Update System BIOS", L"Update system BIOS firmware", 12, 12, 12, 42 },
  { L"Change Date and Time", L"Modify system date and time", 13, 12, 13, 45 },
  { L"Save Changes and Exit", L"Save settings and return to main menu", 14, 12, 14, 50 }
};

// Security submenu items
MENU_ITEM mSecuritySubmenu[] = {
  { L"Administrator Tools", L"Administrative security tools", 10, 12, 10, 42 },
  { L"Create BIOS Administrator Password", L"Set BIOS administrator password", 11, 12, 11, 55 },
  { L"TPM Embedded Security", L"Configure TPM security settings", 12, 12, 12, 48 },
  { L"Return to Main Menu", L"Go back to main BIOS menu", 13, 12, 13, 42 }
};

// Advanced submenu items
MENU_ITEM mAdvancedSubmenu[] = {
  { L"Boot Options", L"Configure boot device settings", 10, 12, 10, 35 },
  { L"Port Options", L"Configure I/O port settings", 11, 12, 11, 35 },
  { L"System Options", L"Configure system-level options", 12, 12, 12, 38 },
  { L"Return to Main Menu", L"Go back to main BIOS menu", 13, 12, 13, 42 }
};

// UEFI Drivers submenu items
MENU_ITEM mUefiSubmenu[] = {
  { L"3rd Party Option ROM Management", L"Manage third-party option ROMs", 10, 12, 10, 60 },
  { L"Return to Main Menu", L"Go back to main BIOS menu", 11, 12, 11, 42 }
};

// Function declarations
VOID DisplayBiosSetupForm (VOID);
VOID GetCurrentTime (OUT CHAR16 *TimeString);
VOID HandleBiosSetupNavigation (VOID);
VOID DisplaySelectedMenuInfo (VOID);
EFI_STATUS InitializeMouse (VOID);
VOID UpdateMouseState (VOID);
UINTN GetMenuItemFromMousePosition (IN INT32 MouseX, IN INT32 MouseY);
VOID HandleMenuSelection (VOID);
VOID DisplayCurrentMenu (VOID);
VOID DisplaySubmenu (VOID);
MENU_ITEM* GetCurrentMenuArray (OUT UINTN *ItemCount);

LOGO_ENTRY mLogos[] = {
  {
    IMAGE_TOKEN (IMG_LOGO),
    EdkiiPlatformLogoDisplayAttributeCenter,
    0,
    0
  }
};

**/

/**
  Get current menu array based on menu level.

  @param ItemCount   Output parameter for number of items in menu.

  @retval Pointer to current menu array.
**/

/**
MENU_ITEM*
GetCurrentMenuArray (
  OUT UINTN *ItemCount
  )
{
  switch (mCurrentMenuLevel) {
    case MENU_LEVEL_MAIN_SUBMENU:
      *ItemCount = ARRAY_SIZE (mMainSubmenu);
      return mMainSubmenu;
      
    case MENU_LEVEL_SECURITY_SUBMENU:
      *ItemCount = ARRAY_SIZE (mSecuritySubmenu);
      return mSecuritySubmenu;
      
    case MENU_LEVEL_ADVANCED_SUBMENU:
      *ItemCount = ARRAY_SIZE (mAdvancedSubmenu);
      return mAdvancedSubmenu;
      
    case MENU_LEVEL_UEFI_SUBMENU:
      *ItemCount = ARRAY_SIZE (mUefiSubmenu);
      return mUefiSubmenu;
      
    default: // MENU_LEVEL_MAIN_MENU
      *ItemCount = ARRAY_SIZE (mMainBiosMenu);
      return mMainBiosMenu;
  }
}

**/
/**
  Initialize mouse support by locating Simple Pointer Protocol.

  @retval EFI_SUCCESS   Mouse initialized successfully.
  @retval Others        Failed to initialize mouse.
**/

/**
EFI_STATUS
InitializeMouse (
  VOID
  )
{
  EFI_STATUS Status;
  
  // Initialize mouse state with center screen position
  mMouseState.LeftButtonPressed = FALSE;
  mMouseState.LeftButtonReleased = FALSE;
  mMouseState.WasLeftButtonPressed = FALSE;
  mMouseState.ClickCount = 0;
  mMouseState.LastClickTime = 0;
  mMouseState.LastX = 40; // Center of 80-column screen
  mMouseState.LastY = 12; // Center of 25-row screen
  
  // Try to locate Simple Pointer Protocol
  Status = gBS->LocateProtocol (
                  &gEfiSimplePointerProtocolGuid,
                  NULL,
                  (VOID **)&mSimplePointer
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "Simple Pointer Protocol not available: %r\n", Status));
    DEBUG ((DEBUG_WARN, "Mouse support disabled - using keyboard only\n"));
    mSimplePointer = NULL;
    return Status;
  }
  
  // Reset mouse with extended reset
  Status = mSimplePointer->Reset (mSimplePointer, TRUE);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "Failed to reset mouse: %r\n", Status));
    // Try without extended verification
    Status = mSimplePointer->Reset (mSimplePointer, FALSE);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Mouse reset failed completely: %r\n", Status));
      mSimplePointer = NULL;
      return Status;
    }
  }
  
  DEBUG ((DEBUG_INFO, "Mouse initialized successfully\n"));
  DEBUG ((DEBUG_INFO, "Mouse Resolution: X=%d, Y=%d, Z=%d\n", 
          mSimplePointer->Mode->ResolutionX,
          mSimplePointer->Mode->ResolutionY, 
          mSimplePointer->Mode->ResolutionZ));
  
  return EFI_SUCCESS;
}

**/

/**
  Update mouse state by reading current mouse input.
**/

/**
VOID
UpdateMouseState (
  VOID
  )
{
  EFI_STATUS                Status;
  EFI_SIMPLE_POINTER_STATE  MouseState;
  
  if (mSimplePointer == NULL) {
    return;
  }
  
  Status = mSimplePointer->GetState (mSimplePointer, &MouseState);
  if (EFI_ERROR (Status)) {
    // No mouse input available right now
    return;
  }
  
  // Handle relative movement with better scaling
  INT32 DeltaX = MouseState.RelativeMovementX / 65536; // Adjusted scaling
  INT32 DeltaY = MouseState.RelativeMovementY / 65536; // Adjusted scaling
  
  // Apply movement if significant enough
  if (DeltaX != 0 || DeltaY != 0) {
    mMouseState.LastX += DeltaX;
    mMouseState.LastY += DeltaY;
    
    // Boundary check for 80x25 console
    if (mMouseState.LastX < 0) mMouseState.LastX = 0;
    if (mMouseState.LastY < 0) mMouseState.LastY = 0;
    if (mMouseState.LastX > 79) mMouseState.LastX = 79;
    if (mMouseState.LastY > 24) mMouseState.LastY = 24;
    
    DEBUG ((DEBUG_VERBOSE, "Mouse moved to: %d, %d (Delta: %d, %d)\n", 
            mMouseState.LastX, mMouseState.LastY, DeltaX, DeltaY));
  }
  
  // Update button state
  mMouseState.WasLeftButtonPressed = mMouseState.LeftButtonPressed;
  mMouseState.LeftButtonPressed = MouseState.LeftButton;
  
  // Detect button release (click completion)
  if (mMouseState.WasLeftButtonPressed && !mMouseState.LeftButtonPressed) {
    mMouseState.LeftButtonReleased = TRUE;
    
    // Simple click counting without time dependency
    mMouseState.ClickCount++;
    
    DEBUG ((DEBUG_INFO, "Mouse click detected at: %d, %d (Click count: %d)\n", 
            mMouseState.LastX, mMouseState.LastY, mMouseState.ClickCount));
            
    // Reset click count after a reasonable number to prevent overflow
    if (mMouseState.ClickCount > 10) {
      mMouseState.ClickCount = 1;
    }
  } else {
    mMouseState.LeftButtonReleased = FALSE;
  }
  
  // Reset click count when button is pressed (start of new click sequence)
  if (!mMouseState.WasLeftButtonPressed && mMouseState.LeftButtonPressed) {
    // Button just pressed - reset for new click sequence
    if (mMouseState.ClickCount > 2) {
      mMouseState.ClickCount = 0;
    }
  }
}

**/

/**
  Get menu item index based on mouse position.

  @param MouseX   Mouse X coordinate.
  @param MouseY   Mouse Y coordinate.

  @retval Index   Menu item index, or -1 if not over any menu item.
**/

/**
UINTN
GetMenuItemFromMousePosition (
  IN INT32 MouseX,
  IN INT32 MouseY
  )
{
  UINTN Index;
  MENU_ITEM *CurrentMenu;
  UINTN ItemCount;
  
  CurrentMenu = GetCurrentMenuArray (&ItemCount);
  
  for (Index = 0; Index < ItemCount; Index++) {
    if (MouseY == (CurrentMenu[Index].StartRow) &&
        MouseX >= CurrentMenu[Index].StartCol &&
        MouseX <= CurrentMenu[Index].EndCol) {
      return Index;
    }
  }
  
  return (UINTN)-1; // Not over any menu item
}
**/
/**
  Get current time and format it as string.

  @param TimeString   Output buffer for formatted time string.
**/

/**
VOID
GetCurrentTime (
  OUT CHAR16  *TimeString
  )
{
  EFI_TIME    Time;
  EFI_STATUS  Status;
  
  // Attempt to get the system time
  Status = gRT->GetTime (&Time, NULL);
  if (!EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "GetCurrentTime: Successfully retrieved time - %02d:%02d:%02d\n", Time.Hour, Time.Minute, Time.Second));
    UnicodeSPrint (
      TimeString, 
      50 * sizeof(CHAR16),
      L"%02d:%02d:%02d",
      Time.Hour,
      Time.Minute,
      Time.Second
    );
  } else {
    DEBUG ((DEBUG_ERROR, "GetCurrentTime: Failed to get time, Status: %r\n", Status));
    // Fallback: Display a static message or placeholder
    UnicodeSPrint (TimeString, 50 * sizeof(CHAR16), L"Time Unavailable");
  }
}

**/

/**
  Display information about the selected menu item.
**/

/**
VOID
DisplaySelectedMenuInfo (
  VOID
  )
{
  MENU_ITEM *CurrentMenu;
  UINTN ItemCount;
  
  // Only show info for main menu, not submenus
  if (mCurrentMenuLevel != MENU_LEVEL_MAIN_MENU) {
    return;
  }
  
  CurrentMenu = GetCurrentMenuArray (&ItemCount);
  
  // Clear the info area
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 12);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"                                    ");
  
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 13);
  Print (L"                                    ");
  
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 14);
  Print (L"                                    ");
  
  // Display selected item info
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 12);
  Print (L"Selected: %s", CurrentMenu[mSelectedMenuItem].MenuText);
  
  gST->ConOut->SetCursorPosition (gST->ConOut, 45, 14);
  Print (L"%s", CurrentMenu[mSelectedMenuItem].Description);
}

**/
/**
  Display submenu in clean full screen format without navigation instructions.
**/

/**
VOID
DisplaySubmenu (
  VOID
  )
{
  UINTN   Index;
  CHAR16  TimeString[50];
  MENU_ITEM *CurrentMenu;
  UINTN ItemCount;
  CHAR16 *MenuTitle;
  
  CurrentMenu = GetCurrentMenuArray (&ItemCount);
  mTotalMenuItems = ItemCount;
  
  // Determine menu title based on current level
  switch (mCurrentMenuLevel) {
    case MENU_LEVEL_MAIN_SUBMENU:
      MenuTitle = L"MAIN MENU OPTIONS";
      break;
    case MENU_LEVEL_SECURITY_SUBMENU:
      MenuTitle = L"SECURITY OPTIONS";
      break;
    case MENU_LEVEL_ADVANCED_SUBMENU:
      MenuTitle = L"ADVANCED OPTIONS";
      break;
    case MENU_LEVEL_UEFI_SUBMENU:
      MenuTitle = L"UEFI DRIVERS OPTIONS";
      break;
    default:
      MenuTitle = L"SUBMENU";
      break;
  }
  
  // Clear screen
  gST->ConOut->ClearScreen (gST->ConOut);
  
  // Set white background with black text
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  
  // Fill entire screen with white background
  for (Index = 0; Index < 25; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Index);
    Print (L"                                                                                ");
  }
  
  // Display "CDAC Bangalore" at top center
  gST->ConOut->SetCursorPosition (gST->ConOut, 30, 2);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"CDAC Bangalore");
  
  // Get and display current time at top right corner
  GetCurrentTime (TimeString);
  gST->ConOut->SetCursorPosition (gST->ConOut, 65, 2);
  Print (L"%s", TimeString);
  
  // Add main content area with dynamic title
  gST->ConOut->SetCursorPosition (gST->ConOut, 25, 5);
  Print (L"====================================");
  gST->ConOut->SetCursorPosition (gST->ConOut, 25 + (34 - StrLen(MenuTitle))/2, 6);
  Print (L"%s", MenuTitle);
  gST->ConOut->SetCursorPosition (gST->ConOut, 25, 7);
  Print (L"====================================");
  
  // Display menu items starting from center of screen
  UINTN StartRow = 10;
  for (Index = 0; Index < mTotalMenuItems; Index++) {
    // Center the menu items
    UINTN MenuCol = 20;
    
    // Update position information for mouse detection
    CurrentMenu[Index].StartRow = StartRow + Index;
    CurrentMenu[Index].StartCol = MenuCol;
    CurrentMenu[Index].EndRow = StartRow + Index;
    CurrentMenu[Index].EndCol = MenuCol + StrLen(CurrentMenu[Index].MenuText) + 3;
    
    gST->ConOut->SetCursorPosition (gST->ConOut, MenuCol, StartRow + Index);
    
    if (Index == mSelectedMenuItem) {
      // Highlight selected item with reverse colors
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
      Print (L"► %s", CurrentMenu[Index].MenuText);
    } else {
      // Normal menu item
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"  %s", CurrentMenu[Index].MenuText);
    }
  }
  
  // Show minimal navigation help at bottom
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  gST->ConOut->SetCursorPosition (gST->ConOut, 20, 20);
  Print (L"↑↓ Navigate    ↵ Select    ESC Back");
  
  // Add footer 
  gST->ConOut->SetCursorPosition (gST->ConOut, 25, 24);
  Print (L"BIOS Setup - Submenu Level");
}

**/

/**
  Display current menu based on menu level.
**/

/**
VOID
DisplayCurrentMenu (
  VOID
  )
{
  // For submenus, use clean display
  if (mCurrentMenuLevel != MENU_LEVEL_MAIN_MENU) {
    DisplaySubmenu ();
    return;
  }
  
  // Main menu display (existing code)
  UINTN   Index;
  CHAR16  TimeString[50];
  MENU_ITEM *CurrentMenu;
  UINTN ItemCount;
  CHAR16 *MenuTitle;
  
  CurrentMenu = GetCurrentMenuArray (&ItemCount);
  mTotalMenuItems = ItemCount;
  MenuTitle = L"BIOS SETUP UTILITY";
  
  // Clear screen
  gST->ConOut->ClearScreen (gST->ConOut);
  
  // Set white background with black text
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  
  // Fill entire screen with white background
  for (Index = 0; Index < 25; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Index);
    Print (L"                                                                                ");
  }
  
  // Display "CDAC Bangalore" at top center
  gST->ConOut->SetCursorPosition (gST->ConOut, 30, 2);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"CDAC Bangalore");
  
  // Get and display current time at top right corner
  GetCurrentTime (TimeString);
  gST->ConOut->SetCursorPosition (gST->ConOut, 65, 2);
  Print (L"%s", TimeString);
  
  // Add main content area with dynamic title
  gST->ConOut->SetCursorPosition (gST->ConOut, 25, 5);
  Print (L"====================================");
  gST->ConOut->SetCursorPosition (gST->ConOut, 25 + (34 - StrLen(MenuTitle))/2, 6);
  Print (L"%s", MenuTitle);
  gST->ConOut->SetCursorPosition (gST->ConOut, 25, 7);
  Print (L"====================================");
  
  // Update menu item positions and display menu items
  for (Index = 0; Index < mTotalMenuItems; Index++) {
    // Update position information
    CurrentMenu[Index].StartRow = 10 + Index;
    CurrentMenu[Index].StartCol = 10;
    CurrentMenu[Index].EndRow = 10 + Index;
    CurrentMenu[Index].EndCol = CurrentMenu[Index].StartCol + StrLen(CurrentMenu[Index].MenuText) + 3;
    
    gST->ConOut->SetCursorPosition (gST->ConOut, CurrentMenu[Index].StartCol, 10 + Index);
    
    if (Index == mSelectedMenuItem) {
      // Highlight selected item with reverse colors and arrow symbol
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
      Print (L"► %s", CurrentMenu[Index].MenuText);
    } else {
      // Normal menu item
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"  %s", CurrentMenu[Index].MenuText);
    }
  }
  
  // Display selected item information
  DisplaySelectedMenuInfo ();
  
  // Show navigation instructions with symbols
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  gST->ConOut->SetCursorPosition (gST->ConOut, 10, 20);
  Print (L"Navigation: ↑↓ Arrow Keys    ↵ Enter    ESC Exit");
  
  gST->ConOut->SetCursorPosition (gST->ConOut, 10, 21);
  Print (L"Mouse: Move to navigate, Click to select");
  
  // Add footer with current menu level indicator
  gST->ConOut->SetCursorPosition (gST->ConOut, 15, 24);
  Print (L"BIOS Setup - F12 Activated - Mouse Enabled");
}

**/

/**
  Display BIOS setup form with white background, menu items and navigation.
**/
/**

VOID
DisplayBiosSetupForm (
  VOID
  )
{
  mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;
  mSelectedMenuItem = 0;
  mInBiosSetup = TRUE;
  
  // Initialize mouse support
  InitializeMouse ();
  
  // Display the current menu
  DisplayCurrentMenu ();
  
  // Reset console input for clean key detection in setup
  gST->ConIn->Reset (gST->ConIn, FALSE);
  
  // Start navigation handling
  HandleBiosSetupNavigation ();
}

**/

/**
  Handle menu selection based on current selected item and menu level.
**/
/**

VOID
HandleMenuSelection (
  VOID
  )
{
  switch (mCurrentMenuLevel) {
    case MENU_LEVEL_MAIN_MENU:
      switch (mSelectedMenuItem) {
        case 0: // Main
          mCurrentMenuLevel = MENU_LEVEL_MAIN_SUBMENU;
          mSelectedMenuItem = 0;
          DisplayCurrentMenu ();
          break;
          
        case 1: // Security
          mCurrentMenuLevel = MENU_LEVEL_SECURITY_SUBMENU;
          mSelectedMenuItem = 0;
          DisplayCurrentMenu ();
          break;
          
        case 2: // Advanced
          mCurrentMenuLevel = MENU_LEVEL_ADVANCED_SUBMENU;
          mSelectedMenuItem = 0;
          DisplayCurrentMenu ();
          break;
          
        case 3: // UEFI Drivers
          mCurrentMenuLevel = MENU_LEVEL_UEFI_SUBMENU;
          mSelectedMenuItem = 0;
          DisplayCurrentMenu ();
          break;
        
        case 4: // Save & Exit
          // Show saving message at center of screen
          gST->ConOut->ClearScreen (gST->ConOut);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
          gST->ConOut->SetCursorPosition (gST->ConOut, 25, 12);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_GREEN | EFI_BACKGROUND_BLACK);
          Print (L"✓ Settings saved. Shutting down system...");
          gBS->Stall (2000000); // Wait 2 seconds
          
          // Shutdown the system
          gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
          mInBiosSetup = FALSE;
          break;
          
        case 5: // Exit Without Saving
          // Show exit message at center of screen
          gST->ConOut->ClearScreen (gST->ConOut);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
          gST->ConOut->SetCursorPosition (gST->ConOut, 25, 12);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_RED | EFI_BACKGROUND_BLACK);
          Print (L"✗ Exiting without saving. Shutting down...");
          gBS->Stall (2000000); // Wait 2 seconds
          
          // Shutdown the system
          gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
          mInBiosSetup = FALSE;
          break;
      }
      break;
      
    case MENU_LEVEL_MAIN_SUBMENU:
      switch (mSelectedMenuItem) {
        case 0: // System Information
          // Show centered message
          gST->ConOut->SetCursorPosition (gST->ConOut, 25, 18);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
          Print (L"📊 Displaying System Info...");
          gBS->Stall (1500000);
          // Clear message
          gST->ConOut->SetCursorPosition (gST->ConOut, 25, 18);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
          Print (L"                             ");
          break;
          
        case 1: // BIOS Event Log
          gST->ConOut->SetCursorPosition (gST->ConOut, 25, 18);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
          Print (L"📋 Opening Event Log...");
          gBS->Stall (1500000);
          // Clear message
          gST->ConOut->SetCursorPosition (gST->ConOut, 25, 18);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
          Print (L"                        ");
          break;
          
        case 2: // Update System BIOS
          gST->ConOut->SetCursorPosition (gST->ConOut, 25, 18);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
          Print (L"🔧 BIOS Update Utility...");
          gBS->Stall (1500000);
          // Clear message
          gST->ConOut->SetCursorPosition (gST->ConOut, 25, 18);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
          Print (L"                         ");
          break;
          
        case 3: // Change Date and Time
          gST->ConOut->SetCursorPosition (gST->ConOut, 25, 18);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
          Print (L"🕐 Date/Time Settings...");
          gBS->Stall (1500000);
          // Clear message
          gST->ConOut->SetCursorPosition (gST->ConOut, 25, 18);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
          Print (L"                        ");
          break;
          
        case 4: // Save Changes and Exit
          mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;
          mSelectedMenuItem = 0;
          DisplayCurrentMenu ();
          break;
      }
      break;
      
    case MENU_LEVEL_SECURITY_SUBMENU:
      switch (mSelectedMenuItem) {
        case 0: // Administrator Tools
          gST->ConOut->SetCursorPosition (gST->ConOut, 25, 18);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
          Print (L"🔐 Admin Tools Loading...");
          gBS->Stall (1500000);
          // Clear message
          gST->ConOut->SetCursorPosition (gST->ConOut, 25, 18);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
          Print (L"                         ");
          break;
          
        case 1: // Create BIOS Administrator Password
          gST->ConOut->SetCursorPosition (gST->ConOut, 25, 18);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
          Print (L"🔑 Password Setup...");
          gBS->Stall (1500000);
          // Clear message
          gST->ConOut->SetCursorPosition (gST->ConOut, 25, 18);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
          Print (L"                    ");
          break;
          
        case 2: // TPM Embedded Security
          gST->ConOut->SetCursorPosition (gST->ConOut, 25, 18);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
          Print (L"🛡️ TPM Security Config...");
          gBS->Stall (1500000);
          // Clear message
          gST->ConOut->SetCursorPosition (gST->ConOut, 25, 18);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
          Print (L"                         ");
          break;
          
        case 3: // Return to Main Menu
          mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;
          mSelectedMenuItem = 1; // Return to Security item
          DisplayCurrentMenu ();
          break;
      }
      break;
      
    case MENU_LEVEL_ADVANCED_SUBMENU:
      switch (mSelectedMenuItem) {
        case 0: // Boot Options
          gST->ConOut->SetCursorPosition (gST->ConOut, 25, 18);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
          Print (L"🚀 Boot Options Config...");
          gBS->Stall (1500000);
          // Clear message
          gST->ConOut->SetCursorPosition (gST->ConOut, 25, 18);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
          Print (L"                         ");
          break;
          
        case 1: // Port Options
          gST->ConOut->SetCursorPosition (gST->ConOut, 25, 18);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
          Print (L"🔌 I/O Port Configuration...");
          gBS->Stall (1500000);
          // Clear message
          gST->ConOut->SetCursorPosition (gST->ConOut, 25, 18);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
          Print (L"                            ");
          break;
          
        case 2: // System Options
          gST->ConOut->SetCursorPosition (gST->ConOut, 25, 18);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
          Print (L"⚙️ System Options Config...");
          gBS->Stall (1500000);
          // Clear message
          gST->ConOut->SetCursorPosition (gST->ConOut, 25, 18);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
          Print (L"                           ");
          break;
          
        case 3: // Return to Main Menu
          mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;
          mSelectedMenuItem = 2; // Return to Advanced item
          DisplayCurrentMenu ();
          break;
      }
      break;
      
    case MENU_LEVEL_UEFI_SUBMENU:
      switch (mSelectedMenuItem) {
        case 0: // 3rd Party Option ROM Management
          gST->ConOut->SetCursorPosition (gST->ConOut, 25, 18);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
          Print (L"💾 Option ROM Manager...");
          gBS->Stall (1500000);
          // Clear message
          gST->ConOut->SetCursorPosition (gST->ConOut, 25, 18);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
          Print (L"                        ");
          break;
          
        case 1: // Return to Main Menu
          mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;
          mSelectedMenuItem = 3; // Return to UEFI Drivers item
          DisplayCurrentMenu ();
          break;
      }
      break;
      
    default:
      break;
  }
}

**/

/**
  Handle navigation within BIOS setup using arrow keys and mouse.
  Enhanced with submenu support and better mouse handling.
**/

/**
VOID
HandleBiosSetupNavigation (
  VOID
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  UINTN          PreviousSelection;
  UINTN          MouseMenuIndex;
  STATIC UINTN   ConsecutiveClicks = 0;
  STATIC UINTN   LastClickedItem = (UINTN)-1;
  MENU_ITEM      *CurrentMenu;
  UINTN          ItemCount;
  
  while (mInBiosSetup) {
    // Get current menu info
    CurrentMenu = GetCurrentMenuArray (&ItemCount);
    
    // Only show mouse status for main menu
    if (mCurrentMenuLevel == MENU_LEVEL_MAIN_MENU) {
      if (mSimplePointer != NULL) {
        gST->ConOut->SetCursorPosition (gST->ConOut, 45, 18);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"Mouse: Active at (%d,%d)", mMouseState.LastX, mMouseState.LastY);
      } else {
        gST->ConOut->SetCursorPosition (gST->ConOut, 45, 18);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"Mouse: Not Available     ");
      }
    }
    
    // Update mouse state first
    UpdateMouseState ();
    
    // Handle mouse input
    if (mSimplePointer != NULL) {
      MouseMenuIndex = GetMenuItemFromMousePosition (mMouseState.LastX, mMouseState.LastY);
      
      // Handle mouse clicks
      if (mMouseState.LeftButtonReleased) {
        DEBUG ((DEBUG_INFO, "Mouse click at (%d,%d), Menu item: %d\n", 
                mMouseState.LastX, mMouseState.LastY, MouseMenuIndex));
        
        if (MouseMenuIndex != (UINTN)-1 && MouseMenuIndex < mTotalMenuItems) {
          // Check for consecutive clicks on same item
          if (LastClickedItem == MouseMenuIndex) {
            ConsecutiveClicks++;
          } else {
            ConsecutiveClicks = 1;
            LastClickedItem = MouseMenuIndex;
          }
          
          // First click - select menu item
          if (ConsecutiveClicks == 1) {
            PreviousSelection = mSelectedMenuItem;
            mSelectedMenuItem = MouseMenuIndex;
            
            // Update display if selection changed
            if (PreviousSelection != mSelectedMenuItem) {
              // Update previous item display (remove highlight)
              gST->ConOut->SetCursorPosition (gST->ConOut, CurrentMenu[PreviousSelection].StartCol, CurrentMenu[PreviousSelection].StartRow);
              gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
              if (mCurrentMenuLevel == MENU_LEVEL_MAIN_MENU) {
                Print (L"  %s", CurrentMenu[PreviousSelection].MenuText);
              } else {
                Print (L"  %s", CurrentMenu[PreviousSelection].MenuText);
              }
              
              // Update current item display (add highlight)
              gST->ConOut->SetCursorPosition (gST->ConOut, CurrentMenu[mSelectedMenuItem].StartCol, CurrentMenu[mSelectedMenuItem].StartRow);
              gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
              Print (L"► %s", CurrentMenu[mSelectedMenuItem].MenuText);
              
              // Update selected item information (only for main menu)
              if (mCurrentMenuLevel == MENU_LEVEL_MAIN_MENU) {
                DisplaySelectedMenuInfo ();
              }
            }
          }
          
          // Second click - execute selection (double-click simulation)
          else if (ConsecutiveClicks >= 2) {
            mSelectedMenuItem = MouseMenuIndex;
            HandleMenuSelection ();
            ConsecutiveClicks = 0;
            LastClickedItem = (UINTN)-1;
            
            if (!mInBiosSetup) {
              return; // Exit if user chose to exit
            }
          }
        } else {
          // Click outside menu area - reset click counter
          ConsecutiveClicks = 0;
          LastClickedItem = (UINTN)-1;
        }
        
        // Reset mouse click state
        mMouseState.LeftButtonReleased = FALSE;
        mMouseState.ClickCount = 0;
      }
    }
    
    // Handle keyboard input
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
    
    if (!EFI_ERROR (Status)) {
      PreviousSelection = mSelectedMenuItem;
      
      switch (Key.ScanCode) {
        case SCAN_UP:
          // Move up in menu
          if (mSelectedMenuItem > 0) {
            mSelectedMenuItem--;
          } else {
            mSelectedMenuItem = mTotalMenuItems - 1; // Wrap to bottom
          }
          ConsecutiveClicks = 0; // Reset mouse click counter
          LastClickedItem = (UINTN)-1;
          break;
          
        case SCAN_DOWN:
          // Move down in menu
          if (mSelectedMenuItem < mTotalMenuItems - 1) {
            mSelectedMenuItem++;
          } else {
            mSelectedMenuItem = 0; // Wrap to top
          }
          ConsecutiveClicks = 0; // Reset mouse click counter
          LastClickedItem = (UINTN)-1;
          break;
          
        case SCAN_ESC:
          // Handle ESC based on menu level
          if (mCurrentMenuLevel == MENU_LEVEL_MAIN_MENU) {
            // Exit BIOS setup with shutdown
            mInBiosSetup = FALSE;
            gST->ConOut->ClearScreen (gST->ConOut);
            gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
            gST->ConOut->SetCursorPosition (gST->ConOut, 25, 12);
            Print (L"🚪 Exiting BIOS Setup. Shutting down system...");
            gBS->Stall (2000000); // Wait 2 seconds
            
            // Shutdown instead of returning to boot process
            gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
            return;
          } else {
            // Go back to main menu from submenu
            UINTN PrevLevel = mCurrentMenuLevel;
            mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;
            
            // Set selection based on which submenu we're coming from
            switch (PrevLevel) {
              case MENU_LEVEL_MAIN_SUBMENU:
                mSelectedMenuItem = 0;
                break;
              case MENU_LEVEL_SECURITY_SUBMENU:
                mSelectedMenuItem = 1;
                break;
              case MENU_LEVEL_ADVANCED_SUBMENU:
                mSelectedMenuItem = 2;
                break;
              case MENU_LEVEL_UEFI_SUBMENU:
                mSelectedMenuItem = 3;
                break;
              default:
                mSelectedMenuItem = 0;
                break;
            }
            
            DisplayCurrentMenu ();
            ConsecutiveClicks = 0;
            LastClickedItem = (UINTN)-1;
          }
          break;
          
        default:
          // Check for ENTER key (Unicode character)
          if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
            HandleMenuSelection ();
            if (!mInBiosSetup) {
              return; // Exit if user chose to exit (will shutdown in HandleMenuSelection)
            }
          }
          break;
      }
      
      // Update display if selection changed via keyboard
      if (mInBiosSetup && (PreviousSelection != mSelectedMenuItem)) {
        // Get current menu again in case it changed
        CurrentMenu = GetCurrentMenuArray (&ItemCount);
        
        // Update previous item display (remove highlight)
        gST->ConOut->SetCursorPosition (gST->ConOut, CurrentMenu[PreviousSelection].StartCol, CurrentMenu[PreviousSelection].StartRow);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
        if (mCurrentMenuLevel == MENU_LEVEL_MAIN_MENU) {
          Print (L"  %s", CurrentMenu[PreviousSelection].MenuText);
        } else {
          Print (L"  %s", CurrentMenu[PreviousSelection].MenuText);
        }
        
        // Update current item display (add highlight)
        gST->ConOut->SetCursorPosition (gST->ConOut, CurrentMenu[mSelectedMenuItem].StartCol, CurrentMenu[mSelectedMenuItem].StartRow);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
        Print (L"► %s", CurrentMenu[mSelectedMenuItem].MenuText);
        
        // Update selected item information (only for main menu)
        if (mCurrentMenuLevel == MENU_LEVEL_MAIN_MENU) {
          DisplaySelectedMenuInfo ();
        }
      }
    } else {
      // Small delay to prevent excessive CPU usage
      gBS->Stall (50000); // 50ms
    }
  }
}

**/
/**
  Monitor keyboard input for F12 key press only.
  Modified to shutdown after BIOS setup completion.
**/
/**
VOID
EFIAPI
KeyboardMonitor (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  
  // Only monitor after message is shown and not in BIOS setup
  if (!mMessageShown || mInBiosSetup) {
    return;
  }
  
  // Check if key is available
  Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
  
  if (!EFI_ERROR (Status)) {
    // Check for F12 key only
    if (Key.ScanCode == SCAN_F12) {
      DEBUG ((DEBUG_INFO, "F12 key pressed - showing BIOS setup form\n"));
      
      // Cancel timer and keyboard events to stop normal flow
      gBS->SetTimer (mKeyMonitorEvent, TimerCancel, 0);
      gBS->SetTimer (mTimerEvent, TimerCancel, 0);
      
      // Display BIOS setup form
      DisplayBiosSetupForm ();
      
      // After exiting BIOS setup, shutdown
      gST->ConOut->ClearScreen (gST->ConOut);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
      gST->ConOut->SetCursorPosition (gST->ConOut, 25, 12);
      Print (L"✅ BIOS Setup completed. Shutting down system...");
      gBS->Stall (2000000); // Wait 2 seconds
      
      // Shutdown the system
      gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
      
      return;
    }
  }
}

**/
/**
  Timer callback function to handle logo display timing.

  @param Event      The event that is signaled.
  @param Context    The context passed to the event.
**/

/**
VOID
EFIAPI
TimerCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  mTimerCounter++;
  
  // After 5 seconds, show the message below logo and wait for F12
  if (mTimerCounter == 50 && !mMessageShown) { // 50 * 100ms = 5 seconds
    // Clear the entire screen first
    gST->ConOut->ClearScreen (gST->ConOut);
    
    // Position cursor below the logo area
    gST->ConOut->SetCursorPosition (gST->ConOut, 18, 18);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
    Print (L"Press F12 key for BIOS setup");
    
    mMessageShown = TRUE;
    
    // Reset console input for clean key detection
    gST->ConIn->Reset (gST->ConIn, FALSE);
    
    DEBUG ((DEBUG_INFO, "Message displayed, waiting for F12 key\n"));
  }
  
  // Wait indefinitely for F12 key press, no shell redirect
}

**/

/**
  Setup timing and keyboard monitoring after logo is displayed.
**/

/**
VOID
SetupLogoTiming (
  VOID
  )
{
  EFI_STATUS  Status;
  
  if (mLogoDisplayed) {
    return; // Already setup
  }
  
  DEBUG ((DEBUG_INFO, "Setting up logo timing and keyboard monitoring\n"));
  
  // Create timer event for logo timing
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  TimerCallback,
                  NULL,
                  &mTimerEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    // Set timer to fire every 100ms
    Status = gBS->SetTimer (
                    mTimerEvent,
                    TimerPeriodic,
                    1000000  // 100ms in 100ns units
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set timer for mTimerEvent: %r\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to create mTimerEvent: %r\n", Status));
  }
  
  // Create keyboard monitoring event
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  KeyboardMonitor,
                  NULL,
                  &mKeyMonitorEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    // Set keyboard timer to fire every 50ms for responsive key detection
    Status = gBS->SetTimer (
                    mKeyMonitorEvent,
                    TimerPeriodic,
                    500000  // 50ms in 100ns units
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set timer for mKeyMonitorEvent: %r\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to create mKeyMonitorEvent: %r\n", Status));
  }
  
  mLogoDisplayed = TRUE;
}

**/

/**
  Load a platform logo image and return its data and attributes.

  @param This              The pointer to this protocol instance.
  @param Instance          The visible image instance is found.
  @param Image             Points to the image.
  @param Attribute         The display attributes of the image returned.
  @param OffsetX           The X offset of the image regarding the Attribute.
  @param OffsetY           The Y offset of the image regarding the Attribute.

  @retval EFI_SUCCESS      The image was fetched successfully.
  @retval EFI_NOT_FOUND    The specified image could not be found.
**/

/**
EFI_STATUS
EFIAPI
GetImage (
  IN     EDKII_PLATFORM_LOGO_PROTOCOL        *This,
  IN OUT UINT32                              *Instance,
  OUT EFI_IMAGE_INPUT                        *Image,
  OUT EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE  *Attribute,
  OUT INTN                                   *OffsetX,
  OUT INTN                                   *OffsetY
  )
{
  UINT32      Current;
  EFI_STATUS  Status;

  if ((Instance == NULL) || (Image == NULL) ||
      (Attribute == NULL) || (OffsetX == NULL) || (OffsetY == NULL))
  {
    return EFI_INVALID_PARAMETER;
  }

  Current = *Instance;
  if (Current >= ARRAY_SIZE (mLogos)) {
    return EFI_NOT_FOUND;
  }

  (*Instance)++;
  *Attribute = mLogos[Current].Attribute;
  *OffsetX   = mLogos[Current].OffsetX;
  *OffsetY   = mLogos[Current].OffsetY;
  
  // Get image from HII database
  Status = mHiiImageEx->GetImageEx (mHiiImageEx, mHiiHandle, mLogos[Current].ImageId, Image);
  
  // After first logo is displayed, setup timing
  if (Current == 0 && !EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Logo displayed, setting up timing\n"));
    SetupLogoTiming ();
  }
  
  return Status;
}

EDKII_PLATFORM_LOGO_PROTOCOL  mPlatformLogo = {
  GetImage
};

**/

/**
  Cleanup function called when driver is unloaded.

  @param ImageHandle    The handle of the image being unloaded.

  @retval EFI_SUCCESS   The image was unloaded successfully.
**/

/**
EFI_STATUS
EFIAPI
LogoDriverUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
  // Close events if they exist
  if (mTimerEvent != NULL) {
    gBS->CloseEvent (mTimerEvent);
    mTimerEvent = NULL;
  }
  
  if (mKeyMonitorEvent != NULL) {
    gBS->CloseEvent (mKeyMonitorEvent);
    mKeyMonitorEvent = NULL;
  }
  
  if (mBiosSetupKeyEvent != NULL) {
    gBS->CloseEvent (mBiosSetupKeyEvent);
    mBiosSetupKeyEvent = NULL;
  }
  
  if (mMouseEvent != NULL) {
    gBS->CloseEvent (mMouseEvent);
    mMouseEvent = NULL;
  }
  
  return EFI_SUCCESS;
}

**/

/**
  Entrypoint of this module.

  This function is the entrypoint of this module. It installs the Edkii
  Platform Logo protocol.

  @param  ImageHandle       The firmware allocated handle for the EFI image.
  @param  SystemTable       A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
**/

/**
EFI_STATUS
EFIAPI
InitializeLogo (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                   Status;
  EFI_HII_PACKAGE_LIST_HEADER  *PackageList;
  EFI_HII_DATABASE_PROTOCOL    *HiiDatabase;
  EFI_HANDLE                   Handle;

  DEBUG ((DEBUG_INFO, "Logo Driver: Initializing with nested menu support...\n"));

  // Initialize global variables
  mInBiosSetup = FALSE;
  mSelectedMenuItem = 0;
  mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;
  mTotalMenuItems = ARRAY_SIZE (mMainBiosMenu);

  // Locate HII Database Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiDatabaseProtocolGuid,
                  NULL,
                  (VOID **)&HiiDatabase
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Database Protocol: %r\n", Status));
    return Status;
  }

  // Locate HII Image Ex Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiImageExProtocolGuid,
                  NULL,
                  (VOID **)&mHiiImageEx
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Image Ex Protocol: %r\n", Status));
    return Status;
  }

  // Retrieve HII package list from ImageHandle
  Status = gBS->OpenProtocol (
                  ImageHandle,
                  &gEfiHiiPackageListProtocolGuid,
                  (VOID **)&PackageList,
                  ImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "HII Image Package with logo not found in PE/COFF resource section\n"));
    return Status;
  }

  // Publish HII package list to HII Database
  Status = HiiDatabase->NewPackageList (
                          HiiDatabase,
                          PackageList,
                          NULL,
                          &mHiiHandle
                          );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to add package list to HII Database: %r\n", Status));
    return Status;
  }

  // Install Platform Logo Protocol
  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEdkiiPlatformLogoProtocolGuid,
                  &mPlatformLogo,
                  NULL
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to install Platform Logo Protocol: %r\n", Status));
    // Cleanup HII handle
    HiiDatabase->RemovePackageList (HiiDatabase, mHiiHandle);
    return Status;
  }

  DEBUG ((DEBUG_INFO, "Logo Driver: Initialized successfully with nested menu support\n"));
  
  return EFI_SUCCESS;
}

**/

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



//Enhanced BIOS setup utility with blue theme and horizontal menu layout

/** @file
  Logo DXE Driver with F12 key BIOS setup functionality.
  Shows logo, waits for F12 key, then displays BIOS setup form with navigation.
  Enhanced with blue theme, horizontal menu tabs and three-section layout.

Copyright (c) 2016 - 2017, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent
**/

/**
#include <Uefi.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/HiiImageEx.h>
#include <Protocol/PlatformLogo.h>
#include <Protocol/HiiPackageList.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/SimpleTextOut.h>
#include <Protocol/SimplePointer.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>

typedef struct {
  EFI_IMAGE_ID                             ImageId;
  EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE    Attribute;
  INTN                                     OffsetX;
  INTN                                     OffsetY;
} LOGO_ENTRY;

// Menu item structure for horizontal tabs
typedef struct {
  CHAR16  *MenuText;
  CHAR16  *Description;
  UINT32  StartCol;    // Column where menu tab starts
  UINT32  EndCol;      // Column where menu tab ends
  UINT32  TabWidth;    // Width of the tab
} MENU_TAB;

// Submenu item structure
typedef struct {
  CHAR16  *MenuText;
  CHAR16  *Description;
  UINT32  StartRow;    // Row where menu item starts
  UINT32  StartCol;    // Column where menu item starts  
  UINT32  EndRow;      // Row where menu item ends
  UINT32  EndCol;      // Column where menu item ends
} SUBMENU_ITEM;

// Mouse state structure
typedef struct {
  BOOLEAN  LeftButtonPressed;
  BOOLEAN  LeftButtonReleased;
  BOOLEAN  WasLeftButtonPressed;
  UINT32   ClickCount;
  UINT64   LastClickTime;
  INT32    LastX;
  INT32    LastY;
} MOUSE_STATE;

// Menu level enumeration
typedef enum {
  MENU_LEVEL_MAIN_MENU = 0,
  MENU_LEVEL_MAIN_SUBMENU,
  MENU_LEVEL_SECURITY_SUBMENU,
  MENU_LEVEL_ADVANCED_SUBMENU,
  MENU_LEVEL_UEFI_SUBMENU
} MENU_LEVEL;

// Global variables
EFI_HII_IMAGE_EX_PROTOCOL  *mHiiImageEx;
EFI_HII_HANDLE             mHiiHandle;
EFI_EVENT                  mKeyMonitorEvent;
EFI_EVENT                  mTimerEvent;
EFI_EVENT                  mBiosSetupKeyEvent;
EFI_EVENT                  mMouseEvent;
EFI_SIMPLE_POINTER_PROTOCOL *mSimplePointer = NULL;
BOOLEAN                    mLogoDisplayed = FALSE;
BOOLEAN                    mMessageShown = FALSE;
BOOLEAN                    mInBiosSetup = FALSE;
UINTN                      mTimerCounter = 0;
UINTN                      mSelectedMainTab = 0;
UINTN                      mSelectedSubItem = 0;
UINTN                      mTotalMainTabs = 0;
UINTN                      mTotalSubItems = 0;
MENU_LEVEL                 mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;
MOUSE_STATE                mMouseState;

// Main horizontal tabs (like screenshot)
MENU_TAB mMainTabs[] = {
  { L"Main", L"System Information and Basic Settings", 1, 14, 14 },
  { L"Advanced", L"Advanced Configuration Options", 15, 28, 14 },
  { L"Boot", L"Boot Device and Priority Settings", 29, 42, 14 },
  { L"Security", L"Security and Password Settings", 43, 56, 14 },
  { L"Save & Exit", L"Save changes and exit setup", 57, 79, 23 }
};

// Main submenu items (for Main tab)
SUBMENU_ITEM mMainSubmenu[] = {
  { L"BIOS Information", L"View BIOS version and system information", 5, 5, 5, 35 },
  { L"BIOS Vendor", L"Display BIOS vendor information", 6, 5, 6, 35 },
  { L"Version", L"Show BIOS version details", 7, 5, 7, 35 },
  { L"Core Version", L"Display core firmware version", 8, 5, 8, 35 },
  { L"EC Version", L"Show embedded controller version", 9, 5, 9, 35 },
  { L"Processor Information", L"View processor specifications", 11, 5, 11, 35 },
  { L"Memory Information", L"Display memory configuration", 13, 5, 13, 35 },
  { L"Total Memory", L"Show total system memory", 14, 5, 14, 35 },
  { L"System Information", L"General system information", 16, 5, 16, 35 },
  { L"Serial Number", L"Display system serial number", 17, 5, 17, 35 },
  { L"System Time", L"Set system date and time", 19, 5, 19, 35 },
  { L"Access Level", L"Current user access level", 21, 5, 21, 35 }
};

// Advanced submenu items
SUBMENU_ITEM mAdvancedSubmenu[] = {
  { L"CPU Configuration", L"Configure processor settings", 5, 5, 5, 35 },
  { L"Memory Configuration", L"Configure memory settings", 6, 5, 6, 35 },
  { L"PCIe Configuration", L"Configure PCIe settings", 7, 5, 7, 35 },
  { L"Power Management", L"Configure power options", 8, 5, 8, 35 },
  { L"Advanced Options", L"Other advanced settings", 9, 5, 9, 35 }
};

// Boot submenu items
SUBMENU_ITEM mBootSubmenu[] = {
  { L"Boot Device Priority", L"Set boot device order", 5, 5, 5, 35 },
  { L"Boot Options", L"Configure boot settings", 6, 5, 6, 35 },
  { L"UEFI Boot Options", L"Configure UEFI boot", 7, 5, 7, 35 },
  { L"Legacy Boot Options", L"Configure legacy boot", 8, 5, 8, 35 }
};

// Security submenu items
SUBMENU_ITEM mSecuritySubmenu[] = {
  { L"Administrator Tools", L"Administrative security tools", 5, 5, 5, 35 },
  { L"Create BIOS Password", L"Set BIOS administrator password", 6, 5, 6, 35 },
  { L"TPM Security", L"Configure TPM security settings", 7, 5, 7, 35 },
  { L"Secure Boot", L"Configure secure boot options", 8, 5, 8, 35 }
};

// Save & Exit submenu items
SUBMENU_ITEM mSaveExitSubmenu[] = {
  { L"Save Changes and Exit", L"Save settings and exit setup", 5, 5, 5, 35 },
  { L"Discard Changes and Exit", L"Exit without saving changes", 6, 5, 6, 35 },
  { L"Save Changes", L"Save current settings", 7, 5, 7, 35 },
  { L"Discard Changes", L"Reset to previous settings", 8, 5, 8, 35 },
  { L"Load Setup Defaults", L"Load factory default settings", 9, 5, 9, 35 }
};

// Function declarations
VOID DisplayBiosSetupForm (VOID);
VOID GetCurrentTime (OUT CHAR16 *TimeString);
VOID HandleBiosSetupNavigation (VOID);
VOID DrawBlueBackground (VOID);
VOID DrawHorizontalTabs (VOID);
VOID DrawThreeSectionLayout (VOID);
VOID DisplayCurrentSubmenu (VOID);
VOID DisplaySelectedTabInfo (VOID);
VOID DisplayNavigationHelp (VOID);
EFI_STATUS InitializeMouse (VOID);
VOID UpdateMouseState (VOID);
UINTN GetTabFromMousePosition (IN INT32 MouseX, IN INT32 MouseY);
UINTN GetSubItemFromMousePosition (IN INT32 MouseX, IN INT32 MouseY);
VOID HandleTabSelection (VOID);
VOID HandleSubItemSelection (VOID);
SUBMENU_ITEM* GetCurrentSubmenuArray (OUT UINTN *ItemCount);

LOGO_ENTRY mLogos[] = {
  {
    IMAGE_TOKEN (IMG_LOGO),
    EdkiiPlatformLogoDisplayAttributeCenter,
    0,
    0
  }
};

**/

/**
  Get current submenu array based on selected main tab.

  @param ItemCount   Output parameter for number of items in submenu.

  @retval Pointer to current submenu array.
**/

/**
SUBMENU_ITEM*
GetCurrentSubmenuArray (
  OUT UINTN *ItemCount
  )
{
  switch (mSelectedMainTab) {
    case 0: // Main
      *ItemCount = ARRAY_SIZE (mMainSubmenu);
      return mMainSubmenu;
      
    case 1: // Advanced
      *ItemCount = ARRAY_SIZE (mAdvancedSubmenu);
      return mAdvancedSubmenu;
      
    case 2: // Boot
      *ItemCount = ARRAY_SIZE (mBootSubmenu);
      return mBootSubmenu;
      
    case 3: // Security
      *ItemCount = ARRAY_SIZE (mSecuritySubmenu);
      return mSecuritySubmenu;
      
    case 4: // Save & Exit
      *ItemCount = ARRAY_SIZE (mSaveExitSubmenu);
      return mSaveExitSubmenu;
      
    default:
      *ItemCount = ARRAY_SIZE (mMainSubmenu);
      return mMainSubmenu;
  }
}

**/

/**
  Initialize mouse support by locating Simple Pointer Protocol.

  @retval EFI_SUCCESS   Mouse initialized successfully.
  @retval Others        Failed to initialize mouse.
**/

/**
EFI_STATUS
InitializeMouse (
  VOID
  )
{
  EFI_STATUS Status;
  
  // Initialize mouse state
  mMouseState.LeftButtonPressed = FALSE;
  mMouseState.LeftButtonReleased = FALSE;
  mMouseState.WasLeftButtonPressed = FALSE;
  mMouseState.ClickCount = 0;
  mMouseState.LastClickTime = 0;
  mMouseState.LastX = 40; // Center of screen
  mMouseState.LastY = 12;
  
  // Try to locate Simple Pointer Protocol
  Status = gBS->LocateProtocol (
                  &gEfiSimplePointerProtocolGuid,
                  NULL,
                  (VOID **)&mSimplePointer
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "Simple Pointer Protocol not available: %r\n", Status));
    mSimplePointer = NULL;
    return Status;
  }
  
  // Reset mouse
  Status = mSimplePointer->Reset (mSimplePointer, TRUE);
  if (EFI_ERROR (Status)) {
    Status = mSimplePointer->Reset (mSimplePointer, FALSE);
    if (EFI_ERROR (Status)) {
      mSimplePointer = NULL;
      return Status;
    }
  }
  
  DEBUG ((DEBUG_INFO, "Mouse initialized successfully\n"));
  return EFI_SUCCESS;
}
**/
/**
  Update mouse state by reading current mouse input.
**/

/**
VOID
UpdateMouseState (
  VOID
  )
{
  EFI_STATUS                Status;
  EFI_SIMPLE_POINTER_STATE  MouseState;
  
  if (mSimplePointer == NULL) {
    return;
  }
  
  Status = mSimplePointer->GetState (mSimplePointer, &MouseState);
  if (EFI_ERROR (Status)) {
    return;
  }
  
  // Handle relative movement
  INT32 DeltaX = MouseState.RelativeMovementX / 65536;
  INT32 DeltaY = MouseState.RelativeMovementY / 65536;
  
  if (DeltaX != 0 || DeltaY != 0) {
    mMouseState.LastX += DeltaX;
    mMouseState.LastY += DeltaY;
    
    // Boundary check
    if (mMouseState.LastX < 0) mMouseState.LastX = 0;
    if (mMouseState.LastY < 0) mMouseState.LastY = 0;
    if (mMouseState.LastX > 79) mMouseState.LastX = 79;
    if (mMouseState.LastY > 24) mMouseState.LastY = 24;
  }
  
  // Update button state
  mMouseState.WasLeftButtonPressed = mMouseState.LeftButtonPressed;
  mMouseState.LeftButtonPressed = MouseState.LeftButton;
  
  // Detect button release (click completion)
  if (mMouseState.WasLeftButtonPressed && !mMouseState.LeftButtonPressed) {
    mMouseState.LeftButtonReleased = TRUE;
    mMouseState.ClickCount++;
  } else {
    mMouseState.LeftButtonReleased = FALSE;
  }
}

**/

/**
  Get tab index based on mouse position.

  @param MouseX   Mouse X coordinate.
  @param MouseY   Mouse Y coordinate.

  @retval Index   Tab index, or -1 if not over any tab.
**/

/**
UINTN
GetTabFromMousePosition (
  IN INT32 MouseX,
  IN INT32 MouseY
  )
{
  UINTN Index;
  
  // Check if mouse is on tab row (row 1)
  if (MouseY != 1) {
    return (UINTN)-1;
  }
  
  for (Index = 0; Index < mTotalMainTabs; Index++) {
    if (MouseX >= mMainTabs[Index].StartCol && MouseX <= mMainTabs[Index].EndCol) {
      return Index;
    }
  }
  
  return (UINTN)-1;
}

**/
/**
  Get submenu item index based on mouse position.

  @param MouseX   Mouse X coordinate.
  @param MouseY   Mouse Y coordinate.

  @retval Index   Submenu item index, or -1 if not over any item.
**/

/**
UINTN
GetSubItemFromMousePosition (
  IN INT32 MouseX,
  IN INT32 MouseY
  )
{
  UINTN Index;
  SUBMENU_ITEM *CurrentSubmenu;
  UINTN ItemCount;
  
  CurrentSubmenu = GetCurrentSubmenuArray (&ItemCount);
  
  for (Index = 0; Index < ItemCount; Index++) {
    if (MouseY == CurrentSubmenu[Index].StartRow &&
        MouseX >= CurrentSubmenu[Index].StartCol &&
        MouseX <= CurrentSubmenu[Index].EndCol) {
      return Index;
    }
  }
  
  return (UINTN)-1;
}

**/

/**
  Get current time and format it as string.

  @param TimeString   Output buffer for formatted time string.
**/

/**
VOID
GetCurrentTime (
  OUT CHAR16  *TimeString
  )
{
  EFI_TIME    Time;
  EFI_STATUS  Status;
  
  Status = gRT->GetTime (&Time, NULL);
  if (!EFI_ERROR (Status)) {
    UnicodeSPrint (
      TimeString, 
      50 * sizeof(CHAR16),
      L"%02d/%02d/%d",
      Time.Month,
      Time.Day,
      Time.Year
    );
  } else {
    UnicodeSPrint (TimeString, 50 * sizeof(CHAR16), L"[Date]");
  }
}
**/
/**
  Draw blue background similar to screenshot.
**/

/**
VOID
DrawBlueBackground (
  VOID
  )
{
  UINTN Index;
  
  // Clear screen and set blue background
  gST->ConOut->ClearScreen (gST->ConOut);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  
  // Fill entire screen with blue background
  for (Index = 0; Index < 25; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Index);
    Print (L"                                                                                ");
  }
  
  // Draw top title bar
  gST->ConOut->SetCursorPosition (gST->ConOut, 0, 0);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
 // Print (L" Aptio Setup Utility - Copyright (C) 2012 American Megatrends, Inc.");
  Print (L" CDAC BANGALORE"); 
  // Fill rest of top line
  gST->ConOut->SetCursorPosition (gST->ConOut, 69, 0);
  Print (L"           ");
}
**/
/**
  Draw horizontal tabs at top.
**/

/**
VOID
DrawHorizontalTabs (
  VOID
  )
{
  UINTN Index;
//  UINTN Col = 1;
  
  // Draw tab row background
  gST->ConOut->SetCursorPosition (gST->ConOut, 0, 1);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  Print (L"                                                                                ");
  
  // Draw each tab
  for (Index = 0; Index < mTotalMainTabs; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, mMainTabs[Index].StartCol, 1);
    
    if (Index == mSelectedMainTab) {
      // Selected tab - white background with black text
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
      Print (L" %s ", mMainTabs[Index].MenuText);
      
      // Add padding to fill tab width
      UINTN TextLen = StrLen(mMainTabs[Index].MenuText) + 2;
      UINTN PadLen = mMainTabs[Index].TabWidth - TextLen;
      while (PadLen > 0) {
        Print (L" ");
        PadLen--;
      }
    } else {
      // Unselected tab - blue background with white text
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
      Print (L" %s ", mMainTabs[Index].MenuText);
      
      // Add padding
      UINTN TextLen = StrLen(mMainTabs[Index].MenuText) + 2;
      UINTN PadLen = mMainTabs[Index].TabWidth - TextLen;
      while (PadLen > 0) {
        Print (L" ");
        PadLen--;
      }
    }
  }
}

**/

/**
  Draw the three-section layout.
**/

/**
VOID
DrawThreeSectionLayout (
  VOID
  )
{
  UINTN Row;
  
  // Fill the main content area with light gray background
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  
  for (Row = 2; Row < 23; Row++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Row);
    Print (L"                                                                                ");
  }
  
  // Draw vertical separators for three sections
  for (Row = 2; Row < 23; Row++) {
    // Separator between left submenu and right description (at column 50)
    gST->ConOut->SetCursorPosition (gST->ConOut, 50, Row);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
    Print (L"│");
  }
}
**/
/**
  Display current submenu in left section.
**/

/**
VOID
DisplayCurrentSubmenu (
  VOID
  )
{
  UINTN Index;
  SUBMENU_ITEM *CurrentSubmenu;
  UINTN ItemCount;
  
  CurrentSubmenu = GetCurrentSubmenuArray (&ItemCount);
  mTotalSubItems = ItemCount;
  
  // Clear left section
  for (Index = 3; Index < 22; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 1, Index);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    Print (L"                                                 ");
  }
  
  // Display submenu items
  for (Index = 0; Index < ItemCount && Index < 18; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, CurrentSubmenu[Index].StartCol, CurrentSubmenu[Index].StartRow);
    
    if (Index == mSelectedSubItem) {
      // Highlight selected item
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
    } else {
      // Normal item
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    }
    
    Print (L"%s", CurrentSubmenu[Index].MenuText);
  }
}
**/
/**
  Display selected item information in right section.
**/

/**
VOID
DisplaySelectedTabInfo (
  VOID
  )
{
  SUBMENU_ITEM *CurrentSubmenu;
  UINTN ItemCount;
  CHAR16 TimeString[50];
  
  CurrentSubmenu = GetCurrentSubmenuArray (&ItemCount);
  
  // Clear right section
  for (UINTN Index = 3; Index < 22; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 52, Index);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    Print (L"                           ");
  }
  
  // Display tab-specific information based on selection
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
  
  if (mSelectedMainTab == 0 && mSelectedSubItem < ItemCount) {
    // Main tab - show system information format
    gST->ConOut->SetCursorPosition (gST->ConOut, 52, 5);
    Print (L"Set the Date.");
    gST->ConOut->SetCursorPosition (gST->ConOut, 52, 6);
    Print (L"Switch between Date elements.");
    
    // Show current time
    GetCurrentTime (TimeString);
    gST->ConOut->SetCursorPosition (gST->ConOut, 52, 8);
    Print (L"Current: %s", TimeString);
    
    gST->ConOut->SetCursorPosition (gST->ConOut, 52, 10);
    Print (L"[Mon 11/11/]");
    gST->ConOut->SetCursorPosition (gST->ConOut, 52, 11);
    Print (L"[00:00:20 ]");
  } else {
    // Other tabs - show description
    if (mSelectedSubItem < ItemCount) {
      gST->ConOut->SetCursorPosition (gST->ConOut, 52, 5);
      Print (L"Selected Item:");
      gST->ConOut->SetCursorPosition (gST->ConOut, 52, 7);
      Print (L"%s", CurrentSubmenu[mSelectedSubItem].MenuText);
      gST->ConOut->SetCursorPosition (gST->ConOut, 52, 9);
      Print (L"%s", CurrentSubmenu[mSelectedSubItem].Description);
    }
  }
}

**/

/**
  Display navigation help in bottom section.
**/

/**
VOID
DisplayNavigationHelp (
  VOID
  )
{
  // Bottom blue bar with navigation help
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  
  gST->ConOut->SetCursorPosition (gST->ConOut, 0, 23);
  Print (L"                                                                                ");
  
  gST->ConOut->SetCursorPosition (gST->ConOut, 0, 24);
  Print (L" ←→: Select Screen  ↑↓: Select Item  Enter: Select  +/-: Change Opt");
  gST->ConOut->SetCursorPosition (gST->ConOut, 67, 24);
  Print (L"  F1: General Help");
  
  // Show additional help info
  gST->ConOut->SetCursorPosition (gST->ConOut, 52, 20);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"←→: Select Screen");
  gST->ConOut->SetCursorPosition (gST->ConOut, 52, 21);
  Print (L"↑↓: Select Item");
  gST->ConOut->SetCursorPosition (gST->ConOut, 52, 22);
  Print (L"Enter: Select");
  gST->ConOut->SetCursorPosition (gST->ConOut, 52, 23);
  Print (L"+/-: Change Opt");
}

**/

/**
  Display BIOS setup form with blue theme and horizontal layout.
**/

/**
VOID
DisplayBiosSetupForm (
  VOID
  )
{
  mSelectedMainTab = 0;
  mSelectedSubItem = 0;
  mTotalMainTabs = ARRAY_SIZE (mMainTabs);
  mInBiosSetup = TRUE;
  
  // Initialize mouse support
  InitializeMouse ();
  
  // Draw the complete BIOS setup interface
  DrawBlueBackground ();
  DrawHorizontalTabs ();
  DrawThreeSectionLayout ();
  DisplayCurrentSubmenu ();
  DisplaySelectedTabInfo ();
  DisplayNavigationHelp ();
  
  // Reset console input for clean key detection
  gST->ConIn->Reset (gST->ConIn, FALSE);
  
  // Start navigation handling
  HandleBiosSetupNavigation ();
}

**/
/**
  Handle tab selection based on current selected tab.
**/
/**
VOID
HandleTabSelection (
  VOID
  )
{
  switch (mSelectedMainTab) {
    case 4: // Save & Exit
      if (mSelectedSubItem == 0) { // Save Changes and Exit
        gST->ConOut->ClearScreen (gST->ConOut);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
        gST->ConOut->SetCursorPosition (gST->ConOut, 25, 12);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_GREEN | EFI_BACKGROUND_BLACK);
        Print (L"✓ Settings saved. Shutting down system...");
        gBS->Stall (2000000);
        gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
        mInBiosSetup = FALSE;
      } else if (mSelectedSubItem == 1) { // Discard Changes and Exit
        gST->ConOut->ClearScreen (gST->ConOut);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
        gST->ConOut->SetCursorPosition (gST->ConOut, 25, 12);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_RED | EFI_BACKGROUND_BLACK);
        Print (L"✗ Exiting without saving. Shutting down...");
        gBS->Stall (2000000);
        gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
        mInBiosSetup = FALSE;
      }
      break;
      
    default:
      // For other tabs, just show a brief action message
      gST->ConOut->SetCursorPosition (gST->ConOut, 52, 15);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"⚡ Action performed...    ");
      gBS->Stall (800000);
      // Clear the message
      gST->ConOut->SetCursorPosition (gST->ConOut, 52, 15);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"                         ");
      break;
  }
}

**/

/**
  Handle navigation within BIOS setup using arrow keys and mouse.
**/

/**
VOID
HandleBiosSetupNavigation (
  VOID
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  UINTN          MouseTabIndex, MouseSubIndex;
  
  while (mInBiosSetup) {
    // Update mouse state
    UpdateMouseState ();
    
    // Handle mouse input
    if (mSimplePointer != NULL) {
      MouseTabIndex = GetTabFromMousePosition (mMouseState.LastX, mMouseState.LastY);
      MouseSubIndex = GetSubItemFromMousePosition (mMouseState.LastX, mMouseState.LastY);
      
      // Handle mouse clicks on tabs
      if (mMouseState.LeftButtonReleased && MouseTabIndex != (UINTN)-1) {
        if (MouseTabIndex != mSelectedMainTab) {
          mSelectedMainTab = MouseTabIndex;
          mSelectedSubItem = 0; // Reset submenu selection
          
          // Redraw tabs and submenu
          DrawHorizontalTabs ();
          DisplayCurrentSubmenu ();
          DisplaySelectedTabInfo ();
        }
        mMouseState.LeftButtonReleased = FALSE;
      }
      
      // Handle mouse clicks on submenu items
      if (mMouseState.LeftButtonReleased && MouseSubIndex != (UINTN)-1) {
        if (MouseSubIndex != mSelectedSubItem) {
          mSelectedSubItem = MouseSubIndex;
          
          // Redraw submenu and info
          DisplayCurrentSubmenu ();
          DisplaySelectedTabInfo ();
        } else {
          // Double-click simulation - execute selection
          HandleTabSelection ();
          if (!mInBiosSetup) return;
        }
        mMouseState.LeftButtonReleased = FALSE;
      }
    }
    
    // Handle keyboard input
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
    
    if (!EFI_ERROR (Status)) {
      switch (Key.ScanCode) {
        case SCAN_LEFT:
          // Move left in main tabs
          if (mSelectedMainTab > 0) {
            mSelectedMainTab--;
          } else {
            mSelectedMainTab = mTotalMainTabs - 1; // Wrap to rightmost
          }
          mSelectedSubItem = 0; // Reset submenu selection
          
          // Redraw interface
          DrawHorizontalTabs ();
          DisplayCurrentSubmenu ();
          DisplaySelectedTabInfo ();
          break;
          
        case SCAN_RIGHT:
          // Move right in main tabs
          if (mSelectedMainTab < mTotalMainTabs - 1) {
            mSelectedMainTab++;
          } else {
            mSelectedMainTab = 0; // Wrap to leftmost
          }
          mSelectedSubItem = 0; // Reset submenu selection
          
          // Redraw interface
          DrawHorizontalTabs ();
          DisplayCurrentSubmenu ();
          DisplaySelectedTabInfo ();
          break;
          
        case SCAN_UP:
          // Move up in submenu
          if (mSelectedSubItem > 0) {
            mSelectedSubItem--;
          } else {
            mSelectedSubItem = mTotalSubItems - 1; // Wrap to bottom
          }
          
          // Update submenu and info
          DisplayCurrentSubmenu ();
          DisplaySelectedTabInfo ();
          break;
          
        case SCAN_DOWN:
          // Move down in submenu
          if (mSelectedSubItem < mTotalSubItems - 1) {
            mSelectedSubItem++;
          } else {
            mSelectedSubItem = 0; // Wrap to top
          }
          
          // Update submenu and info
          DisplayCurrentSubmenu ();
          DisplaySelectedTabInfo ();
          break;
          
        case SCAN_ESC:
          // Exit BIOS setup
          mInBiosSetup = FALSE;
          gST->ConOut->ClearScreen (gST->ConOut);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
          gST->ConOut->SetCursorPosition (gST->ConOut, 25, 12);
          Print (L"🚪 Exiting BIOS Setup. Shutting down system...");
          gBS->Stall (2000000);
          gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
          return;
          
        default:
          // Check for ENTER key
          if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
            HandleTabSelection ();
            if (!mInBiosSetup) {
              return;
            }
          }
          break;
      }
    } else {
      // Small delay to prevent excessive CPU usage
      gBS->Stall (50000); // 50ms
    }
  }
}


**/
/**
  Monitor keyboard input for F12 key press only.
**/

/**
VOID
EFIAPI
KeyboardMonitor (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  
  // Only monitor after message is shown and not in BIOS setup
  if (!mMessageShown || mInBiosSetup) {
    return;
  }
  
  // Check if key is available
  Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
  
  if (!EFI_ERROR (Status)) {
    // Check for F12 key only
    if (Key.ScanCode == SCAN_F12) {
      DEBUG ((DEBUG_INFO, "F12 key pressed - showing BIOS setup form\n"));
      
      // Cancel timer and keyboard events
      gBS->SetTimer (mKeyMonitorEvent, TimerCancel, 0);
      gBS->SetTimer (mTimerEvent, TimerCancel, 0);
      
      // Display BIOS setup form
      DisplayBiosSetupForm ();
      
      // After exiting BIOS setup, shutdown
      gST->ConOut->ClearScreen (gST->ConOut);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
      gST->ConOut->SetCursorPosition (gST->ConOut, 25, 12);
      Print (L"✅ BIOS Setup completed. Shutting down system...");
      gBS->Stall (2000000);
      gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
      return;
    }
  }
}
**/

/**
  Timer callback function to handle logo display timing.

  @param Event      The event that is signaled.
  @param Context    The context passed to the event.
**/

/**
VOID
EFIAPI
TimerCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  mTimerCounter++;
  
  // After 5 seconds, show the message below logo and wait for F12
  if (mTimerCounter == 50 && !mMessageShown) { // 50 * 100ms = 5 seconds
    // Clear the entire screen first
    gST->ConOut->ClearScreen (gST->ConOut);
    
    // Position cursor below the logo area
    gST->ConOut->SetCursorPosition (gST->ConOut, 18, 18);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
    Print (L"Press F12 key for BIOS setup");
    
    mMessageShown = TRUE;
    
    // Reset console input for clean key detection
    gST->ConIn->Reset (gST->ConIn, FALSE);
    
    DEBUG ((DEBUG_INFO, "Message displayed, waiting for F12 key\n"));
  }
}

**/
/**
  Setup timing and keyboard monitoring after logo is displayed.
**/
/**
VOID
SetupLogoTiming (
  VOID
  )
{
  EFI_STATUS  Status;
  
  if (mLogoDisplayed) {
    return;
  }
  
  DEBUG ((DEBUG_INFO, "Setting up logo timing and keyboard monitoring\n"));
  
  // Create timer event for logo timing
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  TimerCallback,
                  NULL,
                  &mTimerEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    Status = gBS->SetTimer (
                    mTimerEvent,
                    TimerPeriodic,
                    1000000  // 100ms in 100ns units
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set timer for mTimerEvent: %r\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to create mTimerEvent: %r\n", Status));
  }
  
  // Create keyboard monitoring event
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  KeyboardMonitor,
                  NULL,
                  &mKeyMonitorEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    Status = gBS->SetTimer (
                    mKeyMonitorEvent,
                    TimerPeriodic,
                    500000  // 50ms in 100ns units
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set timer for mKeyMonitorEvent: %r\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to create mKeyMonitorEvent: %r\n", Status));
  }
  
  mLogoDisplayed = TRUE;
}

**/
/**
  Load a platform logo image and return its data and attributes.
**/
/**
EFI_STATUS
EFIAPI
GetImage (
  IN     EDKII_PLATFORM_LOGO_PROTOCOL        *This,
  IN OUT UINT32                              *Instance,
  OUT EFI_IMAGE_INPUT                        *Image,
  OUT EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE  *Attribute,
  OUT INTN                                   *OffsetX,
  OUT INTN                                   *OffsetY
  )
{
  UINT32      Current;
  EFI_STATUS  Status;

  if ((Instance == NULL) || (Image == NULL) ||
      (Attribute == NULL) || (OffsetX == NULL) || (OffsetY == NULL))
  {
    return EFI_INVALID_PARAMETER;
  }

  Current = *Instance;
  if (Current >= ARRAY_SIZE (mLogos)) {
    return EFI_NOT_FOUND;
  }

  (*Instance)++;
  *Attribute = mLogos[Current].Attribute;
  *OffsetX   = mLogos[Current].OffsetX;
  *OffsetY   = mLogos[Current].OffsetY;
  
  // Get image from HII database
  Status = mHiiImageEx->GetImageEx (mHiiImageEx, mHiiHandle, mLogos[Current].ImageId, Image);
  
  // After first logo is displayed, setup timing
  if (Current == 0 && !EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Logo displayed, setting up timing\n"));
    SetupLogoTiming ();
  }
  
  return Status;
}

EDKII_PLATFORM_LOGO_PROTOCOL  mPlatformLogo = {
  GetImage
};

**/

/**
  Cleanup function called when driver is unloaded.
**/

/**
EFI_STATUS
EFIAPI
LogoDriverUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
  // Close events if they exist
  if (mTimerEvent != NULL) {
    gBS->CloseEvent (mTimerEvent);
    mTimerEvent = NULL;
  }
  
  if (mKeyMonitorEvent != NULL) {
    gBS->CloseEvent (mKeyMonitorEvent);
    mKeyMonitorEvent = NULL;
  }
  
  if (mBiosSetupKeyEvent != NULL) {
    gBS->CloseEvent (mBiosSetupKeyEvent);
    mBiosSetupKeyEvent = NULL;
  }
  
  if (mMouseEvent != NULL) {
    gBS->CloseEvent (mMouseEvent);
    mMouseEvent = NULL;
  }
  
  return EFI_SUCCESS;
}

**/
/**
  Entrypoint of this module.
**/

/**
EFI_STATUS
EFIAPI
InitializeLogo (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                   Status;
  EFI_HII_PACKAGE_LIST_HEADER  *PackageList;
  EFI_HII_DATABASE_PROTOCOL    *HiiDatabase;
  EFI_HANDLE                   Handle;

  DEBUG ((DEBUG_INFO, "Logo Driver: Initializing with blue theme and horizontal tabs...\n"));

  // Initialize global variables
  mInBiosSetup = FALSE;
  mSelectedMainTab = 0;
  mSelectedSubItem = 0;
  mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;
  mTotalMainTabs = ARRAY_SIZE (mMainTabs);

  // Locate HII Database Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiDatabaseProtocolGuid,
                  NULL,
                  (VOID **)&HiiDatabase
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Database Protocol: %r\n", Status));
    return Status;
  }

  // Locate HII Image Ex Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiImageExProtocolGuid,
                  NULL,
                  (VOID **)&mHiiImageEx
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Image Ex Protocol: %r\n", Status));
    return Status;
  }

  // Retrieve HII package list from ImageHandle
  Status = gBS->OpenProtocol (
                  ImageHandle,
                  &gEfiHiiPackageListProtocolGuid,
                  (VOID **)&PackageList,
                  ImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "HII Image Package with logo not found in PE/COFF resource section\n"));
    return Status;
  }

  // Publish HII package list to HII Database
  Status = HiiDatabase->NewPackageList (
                          HiiDatabase,
                          PackageList,
                          NULL,
                          &mHiiHandle
                          );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to add package list to HII Database: %r\n", Status));
    return Status;
  }

  // Install Platform Logo Protocol
  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEdkiiPlatformLogoProtocolGuid,
                  &mPlatformLogo,
                  NULL
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to install Platform Logo Protocol: %r\n", Status));
    HiiDatabase->RemovePackageList (HiiDatabase, mHiiHandle);
    return Status;
  }

  DEBUG ((DEBUG_INFO, "Logo Driver: Initialized successfully with blue theme layout\n"));
  
  return EFI_SUCCESS;
}

**/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// This is working but display size is small and not in proper dimension and without save and exit is not working.



//Enhanced BIOS setup utility with blue theme and horizontal menu layout

/** @file
  Logo DXE Driver with F12 key BIOS setup functionality.
  Shows logo, waits for F12 key, then displays BIOS setup form with navigation.
  Enhanced with blue theme, horizontal menu tabs and three-section layout.

Copyright (c) 2016 - 2017, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent
**/
/**

#include <Uefi.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/HiiImageEx.h>
#include <Protocol/PlatformLogo.h>
#include <Protocol/HiiPackageList.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/SimpleTextOut.h>
#include <Protocol/SimplePointer.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>

typedef struct {
  EFI_IMAGE_ID                             ImageId;
  EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE    Attribute;
  INTN                                     OffsetX;
  INTN                                     OffsetY;
} LOGO_ENTRY;

// Menu item structure for horizontal tabs
typedef struct {
  CHAR16  *MenuText;
  CHAR16  *Description;
  UINT32  StartCol;    // Column where menu tab starts
  UINT32  EndCol;      // Column where menu tab ends
  UINT32  TabWidth;    // Width of the tab
} MENU_TAB;

// Submenu item structure
typedef struct {
  CHAR16  *MenuText;
  CHAR16  *Description;
  UINT32  StartRow;    // Row where menu item starts
  UINT32  StartCol;    // Column where menu item starts  
  UINT32  EndRow;      // Row where menu item ends
  UINT32  EndCol;      // Column where menu item ends
} SUBMENU_ITEM;

// Mouse state structure
typedef struct {
  BOOLEAN  LeftButtonPressed;
  BOOLEAN  LeftButtonReleased;
  BOOLEAN  WasLeftButtonPressed;
  UINT32   ClickCount;
  UINT64   LastClickTime;
  INT32    LastX;
  INT32    LastY;
} MOUSE_STATE;

// Menu level enumeration
typedef enum {
  MENU_LEVEL_MAIN_MENU = 0,
  MENU_LEVEL_MAIN_SUBMENU,
  MENU_LEVEL_SECURITY_SUBMENU,
  MENU_LEVEL_ADVANCED_SUBMENU,
  MENU_LEVEL_UEFI_SUBMENU
} MENU_LEVEL;

// Global variables
EFI_HII_IMAGE_EX_PROTOCOL  *mHiiImageEx;
EFI_HII_HANDLE             mHiiHandle;
EFI_EVENT                  mKeyMonitorEvent;
EFI_EVENT                  mTimerEvent;
EFI_EVENT                  mBiosSetupKeyEvent;
EFI_EVENT                  mMouseEvent;
EFI_SIMPLE_POINTER_PROTOCOL *mSimplePointer = NULL;
BOOLEAN                    mLogoDisplayed = FALSE;
BOOLEAN                    mMessageShown = FALSE;
BOOLEAN                    mInBiosSetup = FALSE;
UINTN                      mTimerCounter = 0;
UINTN                      mSelectedMainTab = 0;
UINTN                      mSelectedSubItem = 0;
UINTN                      mTotalMainTabs = 0;
UINTN                      mTotalSubItems = 0;
MENU_LEVEL                 mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;
MOUSE_STATE                mMouseState;

// Main horizontal tabs (like screenshot)
MENU_TAB mMainTabs[] = {
  { L"Main", L"System Information and Basic Settings", 1, 14, 14 },
  { L"Advanced", L"Advanced Configuration Options", 15, 28, 14 },
 // { L"Boot", L"Boot Device and Priority Settings", 29, 42, 14 },
 { L"Security", L"Security and password Settings", 29, 42, 14 },
 // { L"Security", L"Security and Password Settings", 43, 56, 14 },
 
  { L"Save & Exit", L"Save changes and exit setup", 57, 79, 23 }
};

// Main submenu items (for Main tab)
SUBMENU_ITEM mMainSubmenu[] = {
 // { L"BIOS Information", L"View BIOS version and system information", 5, 5, 5, 35 },
   { L"BIOS Information", L"View BIOS version and system information", 5, 5, 5, 35 },
//  { L"BIOS Vendor", L"Display BIOS vendor information", 6, 5, 6, 35 },
{ L"BIOS Eventlog", L"Display BIOS Eventlog", 6, 5, 6, 35 },
 // { L"Version", L"Show BIOS version details", 7, 5, 7, 35 },
 { L"Update system BIOS", L"Show Update system BIOS information", 7, 5, 7, 35 },
 // { L"Core Version", L"Display core firmware version", 8, 5, 8, 35 },
  { L"Change Date & Time", L"Display Date & Time", 8, 5, 8, 35 },
 
 // { L"EC Version", L"Show embedded controller version", 9, 5, 9, 35 },
 // { L"Processor Information", L"View processor specifications", 11, 5, 11, 35 },
 // { L"Memory Information", L"Display memory configuration", 13, 5, 13, 35 },
 // { L"Total Memory", L"Show total system memory", 14, 5, 14, 35 },
 // { L"System Information", L"General system information", 16, 5, 16, 35 },
 // { L"Serial Number", L"Display system serial number", 17, 5, 17, 35 },
 // { L"System Time", L"Set system date and time", 19, 5, 19, 35 },
 // { L"Access Level", L"Current user access level", 21, 5, 21, 35 }
};

// Advanced submenu items
SUBMENU_ITEM mAdvancedSubmenu[] = {
 // { L"CPU Configuration", L"Configure processor settings", 5, 5, 5, 35 },
  { L"Boot Options", L"Configure boot settings", 5, 5, 5, 35 },
 // { L"Memory Configuration", L"Configure memory settings", 6, 5, 6, 35 },
   { L"Port Options", L"Configure Port options settings", 6, 5, 6, 35 },
//  { L"PCIe Configuration", L"Configure PCIe settings", 7, 5, 7, 35 },
  { L"System Options", L"Configure System options settings", 7, 5, 7, 35 },
//  { L"Power Management", L"Configure power options", 8, 5, 8, 35 },
//  { L"Advanced Options", L"Other advanced settings", 9, 5, 9, 35 }
};

**/

/**
// Boot submenu items
SUBMENU_ITEM mBootSubmenu[] = {
  { L"Boot Device Priority", L"Set boot device order", 5, 5, 5, 35 },
  { L"Boot Options", L"Configure boot settings", 6, 5, 6, 35 },
  { L"UEFI Boot Options", L"Configure UEFI boot", 7, 5, 7, 35 },
  { L"Legacy Boot Options", L"Configure legacy boot", 8, 5, 8, 35 }
};
**/

/**
// Security submenu items
SUBMENU_ITEM mSecuritySubmenu[] = {
  { L"Administrator Tools", L"Administrative security tools", 5, 5, 5, 35 },
  { L"Create BIOS Password", L"Set BIOS administrator password", 6, 5, 6, 35 },
  { L"TPM Security", L"Configure TPM security settings", 7, 5, 7, 35 },
//  { L"Secure Boot", L"Configure secure boot options", 8, 5, 8, 35 }
};

// Save & Exit submenu items
SUBMENU_ITEM mSaveExitSubmenu[] = {
  { L"Save Changes and Exit", L"Save settings and exit setup", 5, 5, 5, 35 },
  { L"Discard Changes and Exit", L"Exit without saving changes", 6, 5, 6, 35 },
  { L"Save Changes", L"Save current settings", 7, 5, 7, 35 },
  { L"Discard Changes", L"Reset to previous settings", 8, 5, 8, 35 },
  { L"Load Setup Defaults", L"Load factory default settings", 9, 5, 9, 35 }
};

// Function declarations
VOID DisplayBiosSetupForm (VOID);
VOID GetCurrentTime (OUT CHAR16 *TimeString);
VOID HandleBiosSetupNavigation (VOID);
VOID DrawBlueBackground (VOID);
VOID DrawHorizontalTabs (VOID);
VOID DrawThreeSectionLayout (VOID);
VOID DisplayCurrentSubmenu (VOID);
VOID DisplaySelectedTabInfo (VOID);
VOID DisplayNavigationHelp (VOID);
EFI_STATUS InitializeMouse (VOID);
VOID UpdateMouseState (VOID);
UINTN GetTabFromMousePosition (IN INT32 MouseX, IN INT32 MouseY);
UINTN GetSubItemFromMousePosition (IN INT32 MouseX, IN INT32 MouseY);
VOID HandleTabSelection (VOID);
VOID HandleSubItemSelection (VOID);
SUBMENU_ITEM* GetCurrentSubmenuArray (OUT UINTN *ItemCount);

LOGO_ENTRY mLogos[] = {
  {
    IMAGE_TOKEN (IMG_LOGO),
    EdkiiPlatformLogoDisplayAttributeCenter,
    0,
    0
  }
};

**/
/**
  Get current submenu array based on selected main tab.

  @param ItemCount   Output parameter for number of items in submenu.

  @retval Pointer to current submenu array.
**/

/**
SUBMENU_ITEM*
GetCurrentSubmenuArray (
  OUT UINTN *ItemCount
  )
{
  switch (mSelectedMainTab) {
    case 0: // Main
      *ItemCount = ARRAY_SIZE (mMainSubmenu);
      return mMainSubmenu;
      
    case 1: // Advanced
      *ItemCount = ARRAY_SIZE (mAdvancedSubmenu);
      return mAdvancedSubmenu;
      
   // case 2: // Boot
   //   *ItemCount = ARRAY_SIZE (mBootSubmenu);
   //   return mBootSubmenu;
      
    case 2: // Security
      *ItemCount = ARRAY_SIZE (mSecuritySubmenu);
      return mSecuritySubmenu;
      
    case 3: // Save & Exit
      *ItemCount = ARRAY_SIZE (mSaveExitSubmenu);
      return mSaveExitSubmenu;
      
    default:
      *ItemCount = ARRAY_SIZE (mMainSubmenu);
      return mMainSubmenu;
  }
}

**/
/**
  Initialize mouse support by locating Simple Pointer Protocol.

  @retval EFI_SUCCESS   Mouse initialized successfully.
  @retval Others        Failed to initialize mouse.
**/

/**
EFI_STATUS
InitializeMouse (
  VOID
  )
{
  EFI_STATUS Status;
  
  // Initialize mouse state
  mMouseState.LeftButtonPressed = FALSE;
  mMouseState.LeftButtonReleased = FALSE;
  mMouseState.WasLeftButtonPressed = FALSE;
  mMouseState.ClickCount = 0;
  mMouseState.LastClickTime = 0;
  mMouseState.LastX = 40; // Center of screen
  mMouseState.LastY = 12;
  
  // Try to locate Simple Pointer Protocol
  Status = gBS->LocateProtocol (
                  &gEfiSimplePointerProtocolGuid,
                  NULL,
                  (VOID **)&mSimplePointer
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "Simple Pointer Protocol not available: %r\n", Status));
    mSimplePointer = NULL;
    return Status;
  }
  
  // Reset mouse
  Status = mSimplePointer->Reset (mSimplePointer, TRUE);
  if (EFI_ERROR (Status)) {
    Status = mSimplePointer->Reset (mSimplePointer, FALSE);
    if (EFI_ERROR (Status)) {
      mSimplePointer = NULL;
      return Status;
    }
  }
  
  DEBUG ((DEBUG_INFO, "Mouse initialized successfully\n"));
  return EFI_SUCCESS;
}

**/

/**
  Update mouse state by reading current mouse input.
**/

/**
VOID
UpdateMouseState (
  VOID
  )
{
  EFI_STATUS                Status;
  EFI_SIMPLE_POINTER_STATE  MouseState;
  
  if (mSimplePointer == NULL) {
    return;
  }
  
  Status = mSimplePointer->GetState (mSimplePointer, &MouseState);
  if (EFI_ERROR (Status)) {
    return;
  }
  
  // Handle relative movement
  INT32 DeltaX = MouseState.RelativeMovementX / 65536;
  INT32 DeltaY = MouseState.RelativeMovementY / 65536;
  
  if (DeltaX != 0 || DeltaY != 0) {
    mMouseState.LastX += DeltaX;
    mMouseState.LastY += DeltaY;
    
    // Boundary check
    if (mMouseState.LastX < 0) mMouseState.LastX = 0;
    if (mMouseState.LastY < 0) mMouseState.LastY = 0;
    if (mMouseState.LastX > 79) mMouseState.LastX = 79;
    if (mMouseState.LastY > 24) mMouseState.LastY = 24;
  }
  
  // Update button state
  mMouseState.WasLeftButtonPressed = mMouseState.LeftButtonPressed;
  mMouseState.LeftButtonPressed = MouseState.LeftButton;
  
  // Detect button release (click completion)
  if (mMouseState.WasLeftButtonPressed && !mMouseState.LeftButtonPressed) {
    mMouseState.LeftButtonReleased = TRUE;
    mMouseState.ClickCount++;
  } else {
    mMouseState.LeftButtonReleased = FALSE;
  }
}

**/

/**
  Get tab index based on mouse position.

  @param MouseX   Mouse X coordinate.
  @param MouseY   Mouse Y coordinate.

  @retval Index   Tab index, or -1 if not over any tab.
**/

/**
UINTN
GetTabFromMousePosition (
  IN INT32 MouseX,
  IN INT32 MouseY
  )
{
  UINTN Index;
  
  // Check if mouse is on tab row (row 1)
  if (MouseY != 1) {
    return (UINTN)-1;
  }
  
  for (Index = 0; Index < mTotalMainTabs; Index++) {
    if (MouseX >= mMainTabs[Index].StartCol && MouseX <= mMainTabs[Index].EndCol) {
      return Index;
    }
  }
  
  return (UINTN)-1;
}

**/

/**
  Get submenu item index based on mouse position.

  @param MouseX   Mouse X coordinate.
  @param MouseY   Mouse Y coordinate.

  @retval Index   Submenu item index, or -1 if not over any item.
**/

/**
UINTN
GetSubItemFromMousePosition (
  IN INT32 MouseX,
  IN INT32 MouseY
  )
{
  UINTN Index;
  SUBMENU_ITEM *CurrentSubmenu;
  UINTN ItemCount;
  
  CurrentSubmenu = GetCurrentSubmenuArray (&ItemCount);
  
  for (Index = 0; Index < ItemCount; Index++) {
    if (MouseY == CurrentSubmenu[Index].StartRow &&
        MouseX >= CurrentSubmenu[Index].StartCol &&
        MouseX <= CurrentSubmenu[Index].EndCol) {
      return Index;
    }
  }
  
  return (UINTN)-1;
}

**/
/**
  Get current time and format it as string.

  @param TimeString   Output buffer for formatted time string.
**/
/**

VOID
GetCurrentTime (
  OUT CHAR16  *TimeString
  )
{
  EFI_TIME    Time;
  EFI_STATUS  Status;
  
  Status = gRT->GetTime (&Time, NULL);
  if (!EFI_ERROR (Status)) {
    UnicodeSPrint (
      TimeString, 
      50 * sizeof(CHAR16),
      L"%02d/%02d/%d",
      Time.Month,
      Time.Day,
      Time.Year
    );
  } else {
    UnicodeSPrint (TimeString, 50 * sizeof(CHAR16), L"[Date]");
  }
}

**/

/**
  Draw blue background similar to screenshot.
**/

/**
VOID
DrawBlueBackground (
  VOID
  )
{
  UINTN Index;
  
  // Clear screen and set blue background
  gST->ConOut->ClearScreen (gST->ConOut);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  
  // Fill entire screen with blue background
  for (Index = 0; Index < 25; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Index);
    Print (L"                                                                                ");
  }
  
  // Draw top title bar
  gST->ConOut->SetCursorPosition (gST->ConOut, 0, 0);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
 // Print (L" Aptio Setup Utility - Copyright (C) 2012 American Megatrends, Inc.");
  Print (L" CDAC BANGALORE"); 
   
  // Fill rest of top line
  gST->ConOut->SetCursorPosition (gST->ConOut, 69, 0);
  Print (L"           ");
}

**/
/**
  Draw horizontal tabs at top.
**/
/**

VOID
DrawHorizontalTabs (
  VOID
  )
{
  UINTN Index;
//  UINTN Col = 1;
  
  // Draw tab row background
  gST->ConOut->SetCursorPosition (gST->ConOut, 0, 1);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  Print (L"                                                                                ");
  
  // Draw each tab
  for (Index = 0; Index < mTotalMainTabs; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, mMainTabs[Index].StartCol, 1);
    
    if (Index == mSelectedMainTab) {
      // Selected tab - white background with black text
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
      Print (L" %s ", mMainTabs[Index].MenuText);
      
      // Add padding to fill tab width
      UINTN TextLen = StrLen(mMainTabs[Index].MenuText) + 2;
      UINTN PadLen = mMainTabs[Index].TabWidth - TextLen;
      while (PadLen > 0) {
        Print (L" ");
        PadLen--;
      }
    } else {
      // Unselected tab - blue background with white text
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
      Print (L" %s ", mMainTabs[Index].MenuText);
      
      // Add padding
      UINTN TextLen = StrLen(mMainTabs[Index].MenuText) + 2;
      UINTN PadLen = mMainTabs[Index].TabWidth - TextLen;
      while (PadLen > 0) {
        Print (L" ");
        PadLen--;
      }
    }
  }
}

**/
/**
  Draw the three-section layout.
**/

/**
VOID
DrawThreeSectionLayout (
  VOID
  )
{
  UINTN Row;
  
  // Fill the main content area with light gray background
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  
  for (Row = 2; Row < 23; Row++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Row);
    Print (L"                                                                                ");
  }
  
  // Draw vertical separators for three sections
  for (Row = 2; Row < 23; Row++) {
    // Separator between left submenu and right description (at column 50)
    gST->ConOut->SetCursorPosition (gST->ConOut, 50, Row);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
    Print (L"│");
  }
}

**/

/**
  Display current submenu in left section.
**/

/**
VOID
DisplayCurrentSubmenu (
  VOID
  )
{
  UINTN Index;
  SUBMENU_ITEM *CurrentSubmenu;
  UINTN ItemCount;
  
  CurrentSubmenu = GetCurrentSubmenuArray (&ItemCount);
  mTotalSubItems = ItemCount;
  
  // Clear left section
  for (Index = 3; Index < 22; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 1, Index);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    Print (L"                                                 ");
  }
  
  // Display submenu items
  for (Index = 0; Index < ItemCount && Index < 18; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, CurrentSubmenu[Index].StartCol, CurrentSubmenu[Index].StartRow);
    
    if (Index == mSelectedSubItem) {
      // Highlight selected item
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
    } else {
      // Normal item
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    }
    
    Print (L"%s", CurrentSubmenu[Index].MenuText);
  }
}

**/

/**
  Display selected item information in right section.
**/

/**
VOID
DisplaySelectedTabInfo (
  VOID
  )
{
  SUBMENU_ITEM *CurrentSubmenu;
  UINTN ItemCount;
  CHAR16 TimeString[50];
  
  CurrentSubmenu = GetCurrentSubmenuArray (&ItemCount);
  
  // Clear right section
  for (UINTN Index = 3; Index < 22; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 52, Index);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    Print (L"                           ");
  }
  
  // Display tab-specific information based on selection
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
  
  if (mSelectedMainTab == 0 && mSelectedSubItem < ItemCount) {
    // Main tab - show system information format
    gST->ConOut->SetCursorPosition (gST->ConOut, 52, 5);
    Print (L"Set the Date.");
    gST->ConOut->SetCursorPosition (gST->ConOut, 52, 6);
    Print (L"Switch between Date elements.");
    
    // Show current time
    GetCurrentTime (TimeString);
    gST->ConOut->SetCursorPosition (gST->ConOut, 52, 8);
    Print (L"Current: %s", TimeString);
    
    gST->ConOut->SetCursorPosition (gST->ConOut, 52, 10);
    Print (L"[Mon 18/08/]");
    gST->ConOut->SetCursorPosition (gST->ConOut, 52, 11);
    Print (L"[00:00:20 ]");
  } else {
    // Other tabs - show description
    if (mSelectedSubItem < ItemCount) {
      gST->ConOut->SetCursorPosition (gST->ConOut, 52, 5);
      Print (L"Selected Item:");
      gST->ConOut->SetCursorPosition (gST->ConOut, 52, 7);
      Print (L"%s", CurrentSubmenu[mSelectedSubItem].MenuText);
      gST->ConOut->SetCursorPosition (gST->ConOut, 52, 9);
      Print (L"%s", CurrentSubmenu[mSelectedSubItem].Description);
    }
  }
}

**/

/**
  Display navigation help in bottom section.
**/

/**
VOID
DisplayNavigationHelp (
  VOID
  )
{
  // Bottom blue bar with navigation help
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  
  gST->ConOut->SetCursorPosition (gST->ConOut, 0, 23);
  Print (L"                                                                                ");
  
  gST->ConOut->SetCursorPosition (gST->ConOut, 0, 24);
  Print (L" ←→: Select Screen  ↑↓: Select Item  Enter: Select  +/-: Change Opt");
  gST->ConOut->SetCursorPosition (gST->ConOut, 67, 24);
  Print (L"  F1: General Help");
  
  // Show additional help info
  gST->ConOut->SetCursorPosition (gST->ConOut, 52, 20);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"←→: Select Screen");
  gST->ConOut->SetCursorPosition (gST->ConOut, 52, 21);
  Print (L"↑↓: Select Item");
  gST->ConOut->SetCursorPosition (gST->ConOut, 52, 22);
  Print (L"Enter: Select");
 // gST->ConOut->SetCursorPosition (gST->ConOut, 52, 23);
 // Print (L"+/-: Change Opt");
}

**/

/**
  Display BIOS setup form with blue theme and horizontal layout.
**/

/**
VOID
DisplayBiosSetupForm (
  VOID
  )
{
  mSelectedMainTab = 0;
  mSelectedSubItem = 0;
  mTotalMainTabs = ARRAY_SIZE (mMainTabs);
  mInBiosSetup = TRUE;
  
  // Initialize mouse support
  InitializeMouse ();
  
  // Draw the complete BIOS setup interface
  DrawBlueBackground ();
  DrawHorizontalTabs ();
  DrawThreeSectionLayout ();
  DisplayCurrentSubmenu ();
  DisplaySelectedTabInfo ();
  DisplayNavigationHelp ();
  
  // Reset console input for clean key detection
  gST->ConIn->Reset (gST->ConIn, FALSE);
  
  // Start navigation handling
  HandleBiosSetupNavigation ();
}

**/

/**
  Handle tab selection based on current selected tab.
**/

/**
VOID
HandleTabSelection (
  VOID
  )
{
  switch (mSelectedMainTab) {
    case 4: // Save & Exit
      if (mSelectedSubItem == 0) { // Save Changes and Exit
        gST->ConOut->ClearScreen (gST->ConOut);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
        gST->ConOut->SetCursorPosition (gST->ConOut, 25, 12);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_GREEN | EFI_BACKGROUND_BLACK);
        Print (L"✓ Settings saved. Shutting down system...");
        gBS->Stall (2000000);
        gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
        mInBiosSetup = FALSE;
      } else if (mSelectedSubItem == 1) { // Discard Changes and Exit
        gST->ConOut->ClearScreen (gST->ConOut);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
        gST->ConOut->SetCursorPosition (gST->ConOut, 25, 12);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_RED | EFI_BACKGROUND_BLACK);
        Print (L"✗ Exiting without saving. Shutting down...");
        gBS->Stall (2000000);
        gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
        mInBiosSetup = FALSE;
      }
      break;
      
    default:
      // For other tabs, just show a brief action message
      gST->ConOut->SetCursorPosition (gST->ConOut, 52, 15);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"⚡ Action performed...    ");
      gBS->Stall (800000);
      // Clear the message
      gST->ConOut->SetCursorPosition (gST->ConOut, 52, 15);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"                         ");
      break;
  }
}

**/

/**
  Handle navigation within BIOS setup using arrow keys and mouse.
**/
/**
VOID
HandleBiosSetupNavigation (
  VOID
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  UINTN          MouseTabIndex, MouseSubIndex;
  
  while (mInBiosSetup) {
    // Update mouse state
    UpdateMouseState ();
    
    // Handle mouse input
    if (mSimplePointer != NULL) {
      MouseTabIndex = GetTabFromMousePosition (mMouseState.LastX, mMouseState.LastY);
      MouseSubIndex = GetSubItemFromMousePosition (mMouseState.LastX, mMouseState.LastY);
      
      // Handle mouse clicks on tabs
      if (mMouseState.LeftButtonReleased && MouseTabIndex != (UINTN)-1) {
        if (MouseTabIndex != mSelectedMainTab) {
          mSelectedMainTab = MouseTabIndex;
          mSelectedSubItem = 0; // Reset submenu selection
          
          // Redraw tabs and submenu
          DrawHorizontalTabs ();
          DisplayCurrentSubmenu ();
          DisplaySelectedTabInfo ();
        }
        mMouseState.LeftButtonReleased = FALSE;
      }
      
      // Handle mouse clicks on submenu items
      if (mMouseState.LeftButtonReleased && MouseSubIndex != (UINTN)-1) {
        if (MouseSubIndex != mSelectedSubItem) {
          mSelectedSubItem = MouseSubIndex;
          
          // Redraw submenu and info
          DisplayCurrentSubmenu ();
          DisplaySelectedTabInfo ();
        } else {
          // Double-click simulation - execute selection
          HandleTabSelection ();
          if (!mInBiosSetup) return;
        }
        mMouseState.LeftButtonReleased = FALSE;
      }
    }
    
    // Handle keyboard input
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
    
    if (!EFI_ERROR (Status)) {
      switch (Key.ScanCode) {
        case SCAN_LEFT:
          // Move left in main tabs
          if (mSelectedMainTab > 0) {
            mSelectedMainTab--;
          } else {
            mSelectedMainTab = mTotalMainTabs - 1; // Wrap to rightmost
          }
          mSelectedSubItem = 0; // Reset submenu selection
          
          // Redraw interface
          DrawHorizontalTabs ();
          DisplayCurrentSubmenu ();
          DisplaySelectedTabInfo ();
          break;
          
        case SCAN_RIGHT:
          // Move right in main tabs
          if (mSelectedMainTab < mTotalMainTabs - 1) {
            mSelectedMainTab++;
          } else {
            mSelectedMainTab = 0; // Wrap to leftmost
          }
          mSelectedSubItem = 0; // Reset submenu selection
          
          // Redraw interface
          DrawHorizontalTabs ();
          DisplayCurrentSubmenu ();
          DisplaySelectedTabInfo ();
          break;
          
        case SCAN_UP:
          // Move up in submenu
          if (mSelectedSubItem > 0) {
            mSelectedSubItem--;
          } else {
            mSelectedSubItem = mTotalSubItems - 1; // Wrap to bottom
          }
          
          // Update submenu and info
          DisplayCurrentSubmenu ();
          DisplaySelectedTabInfo ();
          break;
          
        case SCAN_DOWN:
          // Move down in submenu
          if (mSelectedSubItem < mTotalSubItems - 1) {
            mSelectedSubItem++;
          } else {
            mSelectedSubItem = 0; // Wrap to top
          }
          
          // Update submenu and info
          DisplayCurrentSubmenu ();
          DisplaySelectedTabInfo ();
          break;
          
        case SCAN_ESC:
          // Exit BIOS setup
          mInBiosSetup = FALSE;
          gST->ConOut->ClearScreen (gST->ConOut);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
          gST->ConOut->SetCursorPosition (gST->ConOut, 25, 12);
          Print (L"🚪 Exiting BIOS Setup. Shutting down system...");
          gBS->Stall (2000000);
          gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
          return;
          
        default:
          // Check for ENTER key
          if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
            HandleTabSelection ();
            if (!mInBiosSetup) {
              return;
            }
          }
          break;
      }
    } else {
      // Small delay to prevent excessive CPU usage
      gBS->Stall (50000); // 50ms
    }
  }
}

**/

/**
  Monitor keyboard input for F12 key press only.
**/

/**
VOID
EFIAPI
KeyboardMonitor (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  
  // Only monitor after message is shown and not in BIOS setup
  if (!mMessageShown || mInBiosSetup) {
    return;
  }
  
  // Check if key is available
  Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
  
  if (!EFI_ERROR (Status)) {
    // Check for F12 key only
    if (Key.ScanCode == SCAN_F12) {
      DEBUG ((DEBUG_INFO, "F12 key pressed - showing BIOS setup form\n"));
      
      // Cancel timer and keyboard events
      gBS->SetTimer (mKeyMonitorEvent, TimerCancel, 0);
      gBS->SetTimer (mTimerEvent, TimerCancel, 0);
      
      // Display BIOS setup form
      DisplayBiosSetupForm ();
      
      // After exiting BIOS setup, shutdown
      gST->ConOut->ClearScreen (gST->ConOut);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
      gST->ConOut->SetCursorPosition (gST->ConOut, 25, 12);
      Print (L"✅ BIOS Setup completed. Shutting down system...");
      gBS->Stall (2000000);
      gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
      return;
    }
  }
}

**/

/**
  Timer callback function to handle logo display timing.

  @param Event      The event that is signaled.
  @param Context    The context passed to the event.
**/

/**
VOID
EFIAPI
TimerCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  mTimerCounter++;
  
  // After 5 seconds, show the message below logo and wait for F12
  if (mTimerCounter == 50 && !mMessageShown) { // 50 * 100ms = 5 seconds
    // Clear the entire screen first
    gST->ConOut->ClearScreen (gST->ConOut);
    
    // Position cursor below the logo area
    gST->ConOut->SetCursorPosition (gST->ConOut, 18, 18);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
    Print (L"Press F12 key for BIOS setup");
    
    mMessageShown = TRUE;
    
    // Reset console input for clean key detection
    gST->ConIn->Reset (gST->ConIn, FALSE);
    
    DEBUG ((DEBUG_INFO, "Message displayed, waiting for F12 key\n"));
  }
}

**/

/**
  Setup timing and keyboard monitoring after logo is displayed.
**/

/**
VOID
SetupLogoTiming (
  VOID
  )
{
  EFI_STATUS  Status;
  
  if (mLogoDisplayed) {
    return;
  }
  
  DEBUG ((DEBUG_INFO, "Setting up logo timing and keyboard monitoring\n"));
  
  // Create timer event for logo timing
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  TimerCallback,
                  NULL,
                  &mTimerEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    Status = gBS->SetTimer (
                    mTimerEvent,
                    TimerPeriodic,
                    1000000  // 100ms in 100ns units
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set timer for mTimerEvent: %r\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to create mTimerEvent: %r\n", Status));
  }
  
  // Create keyboard monitoring event
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  KeyboardMonitor,
                  NULL,
                  &mKeyMonitorEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    Status = gBS->SetTimer (
                    mKeyMonitorEvent,
                    TimerPeriodic,
                    500000  // 50ms in 100ns units
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set timer for mKeyMonitorEvent: %r\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to create mKeyMonitorEvent: %r\n", Status));
  }
  
  mLogoDisplayed = TRUE;
}

**/

/**
  Load a platform logo image and return its data and attributes.
**/

/**
EFI_STATUS
EFIAPI
GetImage (
  IN     EDKII_PLATFORM_LOGO_PROTOCOL        *This,
  IN OUT UINT32                              *Instance,
  OUT EFI_IMAGE_INPUT                        *Image,
  OUT EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE  *Attribute,
  OUT INTN                                   *OffsetX,
  OUT INTN                                   *OffsetY
  )
{
  UINT32      Current;
  EFI_STATUS  Status;

  if ((Instance == NULL) || (Image == NULL) ||
      (Attribute == NULL) || (OffsetX == NULL) || (OffsetY == NULL))
  {
    return EFI_INVALID_PARAMETER;
  }

  Current = *Instance;
  if (Current >= ARRAY_SIZE (mLogos)) {
    return EFI_NOT_FOUND;
  }

  (*Instance)++;
  *Attribute = mLogos[Current].Attribute;
  *OffsetX   = mLogos[Current].OffsetX;
  *OffsetY   = mLogos[Current].OffsetY;
  
  // Get image from HII database
  Status = mHiiImageEx->GetImageEx (mHiiImageEx, mHiiHandle, mLogos[Current].ImageId, Image);
  
  // After first logo is displayed, setup timing
  if (Current == 0 && !EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Logo displayed, setting up timing\n"));
    SetupLogoTiming ();
  }
  
  return Status;
}

EDKII_PLATFORM_LOGO_PROTOCOL  mPlatformLogo = {
  GetImage
};

**/

/**
  Cleanup function called when driver is unloaded.
**/

/**
EFI_STATUS
EFIAPI
LogoDriverUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
  // Close events if they exist
  if (mTimerEvent != NULL) {
    gBS->CloseEvent (mTimerEvent);
    mTimerEvent = NULL;
  }
  
  if (mKeyMonitorEvent != NULL) {
    gBS->CloseEvent (mKeyMonitorEvent);
    mKeyMonitorEvent = NULL;
  }
  
  if (mBiosSetupKeyEvent != NULL) {
    gBS->CloseEvent (mBiosSetupKeyEvent);
    mBiosSetupKeyEvent = NULL;
  }
  
  if (mMouseEvent != NULL) {
    gBS->CloseEvent (mMouseEvent);
    mMouseEvent = NULL;
  }
  
  return EFI_SUCCESS;
}

**/

/**
  Entrypoint of this module.
**/

/**
EFI_STATUS
EFIAPI
InitializeLogo (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                   Status;
  EFI_HII_PACKAGE_LIST_HEADER  *PackageList;
  EFI_HII_DATABASE_PROTOCOL    *HiiDatabase;
  EFI_HANDLE                   Handle;

  DEBUG ((DEBUG_INFO, "Logo Driver: Initializing with blue theme and horizontal tabs...\n"));

  // Initialize global variables
  mInBiosSetup = FALSE;
  mSelectedMainTab = 0;
  mSelectedSubItem = 0;
  mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;
  mTotalMainTabs = ARRAY_SIZE (mMainTabs);

  // Locate HII Database Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiDatabaseProtocolGuid,
                  NULL,
                  (VOID **)&HiiDatabase
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Database Protocol: %r\n", Status));
    return Status;
  }

  // Locate HII Image Ex Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiImageExProtocolGuid,
                  NULL,
                  (VOID **)&mHiiImageEx
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Image Ex Protocol: %r\n", Status));
    return Status;
  }

  // Retrieve HII package list from ImageHandle
  Status = gBS->OpenProtocol (
                  ImageHandle,
                  &gEfiHiiPackageListProtocolGuid,
                  (VOID **)&PackageList,
                  ImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "HII Image Package with logo not found in PE/COFF resource section\n"));
    return Status;
  }

  // Publish HII package list to HII Database
  Status = HiiDatabase->NewPackageList (
                          HiiDatabase,
                          PackageList,
                          NULL,
                          &mHiiHandle
                          );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to add package list to HII Database: %r\n", Status));
    return Status;
  }

  // Install Platform Logo Protocol
  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEdkiiPlatformLogoProtocolGuid,
                  &mPlatformLogo,
                  NULL
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to install Platform Logo Protocol: %r\n", Status));
    HiiDatabase->RemovePackageList (HiiDatabase, mHiiHandle);
    return Status;
  }

  DEBUG ((DEBUG_INFO, "Logo Driver: Initialized successfully with blue theme layout\n"));
  
  return EFI_SUCCESS;
}

**/


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// In this main,advance,security and save &next shows two times.save &next is working.cdac bangalore is not showing.

// BIOS setup utility with blue theme and horizontal menu layout 

/** @file
  Logo DXE Driver with F12 key BIOS setup functionality.
  Shows logo, waits for F12 key, then displays BIOS setup form with navigation.
  Enhanced with blue theme, horizontal menu tabs and three-section layout.

Copyright (c) 2016 - 2017, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent
**/
/**

#include <Uefi.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/HiiImageEx.h>
#include <Protocol/PlatformLogo.h>
#include <Protocol/HiiPackageList.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/SimpleTextOut.h>
#include <Protocol/SimplePointer.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>
#include <Library/MemoryAllocationLib.h>

typedef struct {
  EFI_IMAGE_ID                             ImageId;
  EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE    Attribute;
  INTN                                     OffsetX;
  INTN                                     OffsetY;
} LOGO_ENTRY;

// Menu item structure for horizontal tabs
typedef struct {
  CHAR16  *MenuText;
  CHAR16  *Description;
  UINT32  StartCol;    // Column where menu tab starts
  UINT32  EndCol;      // Column where menu tab ends
  UINT32  TabWidth;    // Width of the tab
} MENU_TAB;

// Submenu item structure
typedef struct {
  CHAR16  *MenuText;
  CHAR16  *Description;
  UINT32  StartRow;    // Row where menu item starts
  UINT32  StartCol;    // Column where menu item starts  
  UINT32  EndRow;      // Row where menu item ends
  UINT32  EndCol;      // Column where menu item ends
} SUBMENU_ITEM;

// Mouse state structure
typedef struct {
  BOOLEAN  LeftButtonPressed;
  BOOLEAN  LeftButtonReleased;
  BOOLEAN  WasLeftButtonPressed;
  UINT32   ClickCount;
  UINT64   LastClickTime;
  INT32    LastX;
  INT32    LastY;
  BOOLEAN  Visible;
} MOUSE_STATE;

// Menu level enumeration
typedef enum {
  MENU_LEVEL_MAIN_MENU = 0,
  MENU_LEVEL_MAIN_SUBMENU,
  MENU_LEVEL_SECURITY_SUBMENU,
  MENU_LEVEL_ADVANCED_SUBMENU,
  MENU_LEVEL_UEFI_SUBMENU
} MENU_LEVEL;

// Global variables
EFI_HII_IMAGE_EX_PROTOCOL  *mHiiImageEx;
EFI_HII_HANDLE             mHiiHandle;
EFI_EVENT                  mKeyMonitorEvent;
EFI_EVENT                  mTimerEvent;
EFI_EVENT                  mBiosSetupKeyEvent;
EFI_EVENT                  mMouseEvent;
EFI_SIMPLE_POINTER_PROTOCOL *mSimplePointer = NULL;
BOOLEAN                    mLogoDisplayed = FALSE;
BOOLEAN                    mMessageShown = FALSE;
BOOLEAN                    mInBiosSetup = FALSE;
UINTN                      mTimerCounter = 0;
UINTN                      mSelectedMainTab = 0;
UINTN                      mSelectedSubItem = 0;
UINTN                      mTotalMainTabs = 0;
UINTN                      mTotalSubItems = 0;
MENU_LEVEL                 mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;
MOUSE_STATE                mMouseState;
UINTN                      mConsoleColumns = 100;  // Increased default size
UINTN                      mConsoleRows = 40;      // Increased default size

// Main horizontal tabs (fixed spacing and positioning)
MENU_TAB mMainTabs[] = {
  { L"Main", L"System Information and Basic Settings", 2, 17, 16 },
  { L"Advanced", L"Advanced Configuration Options", 18, 35, 18 },
  { L"Security", L"Security and password Settings", 36, 53, 18 },
  { L"Save & Exit", L"Save changes and exit setup", 54, 75, 22 }
};

// Main submenu items (for Main tab)
SUBMENU_ITEM mMainSubmenu[] = {
  { L"BIOS Information", L"View BIOS version and system information", 5, 5, 5, 35 },
  { L"BIOS Eventlog", L"Display BIOS event log entries", 6, 5, 6, 35 },
  { L"Update system BIOS", L"Update system BIOS firmware", 7, 5, 7, 35 },
  { L"Change Date & Time", L"Configure system date and time", 8, 5, 8, 35 },
};

// Advanced submenu items
SUBMENU_ITEM mAdvancedSubmenu[] = {
  { L"Boot Options", L"Configure boot device priority and settings", 5, 5, 5, 35 },
  { L"Port Options", L"Configure USB, SATA, and other port settings", 6, 5, 6, 35 },
  { L"System Options", L"Configure CPU, memory, and system settings", 7, 5, 7, 35 },
  { L"Power Management", L"Configure power and thermal settings", 8, 5, 8, 35 },
};

// Security submenu items
SUBMENU_ITEM mSecuritySubmenu[] = {
  { L"Administrator Tools", L"Administrative security management tools", 5, 5, 5, 35 },
  { L"Create BIOS Password", L"Set BIOS administrator password protection", 6, 5, 6, 35 },
  { L"TPM Security", L"Configure Trusted Platform Module settings", 7, 5, 7, 35 },
  { L"Secure Boot", L"Enable or disable UEFI Secure Boot", 8, 5, 8, 35 },
};

// Save & Exit submenu items
SUBMENU_ITEM mSaveExitSubmenu[] = {
  { L"Save Changes and Exit", L"Save current settings and exit setup", 5, 5, 5, 35 },
  { L"Discard Changes and Exit", L"Exit setup without saving any changes", 6, 5, 6, 35 },
  { L"Save Changes", L"Save current settings and continue", 7, 5, 7, 35 },
  { L"Discard Changes", L"Reset to previous saved settings", 8, 5, 8, 35 },
  { L"Load Setup Defaults", L"Load factory default settings", 9, 5, 9, 35 }
};

// Function declarations
VOID DisplayBiosSetupForm (VOID);
VOID GetCurrentTime (OUT CHAR16 *TimeString);
VOID HandleBiosSetupNavigation (VOID);
VOID DrawBlueBackground (VOID);
VOID DrawHorizontalTabs (VOID);
VOID DrawThreeSectionLayout (VOID);
VOID DisplayCurrentSubmenu (VOID);
VOID DisplaySelectedTabInfo (VOID);
VOID DisplayNavigationHelp (VOID);
EFI_STATUS InitializeMouse (VOID);
VOID UpdateMouseState (VOID);
UINTN GetTabFromMousePosition (IN INT32 MouseX, IN INT32 MouseY);
UINTN GetSubItemFromMousePosition (IN INT32 MouseX, IN INT32 MouseY);
VOID HandleTabSelection (VOID);
VOID HandleSubItemSelection (VOID);
SUBMENU_ITEM* GetCurrentSubmenuArray (OUT UINTN *ItemCount);
VOID DrawMouseCursor (VOID);
VOID GetConsoleSize (VOID);

LOGO_ENTRY mLogos[] = {
  {
    IMAGE_TOKEN (IMG_LOGO),
    EdkiiPlatformLogoDisplayAttributeCenter,
    0,
    0
  }
};

**/

/**
  Get console size and set to larger size for full screen display.
**/

/**
VOID
GetConsoleSize (
  VOID
  )
{
  EFI_STATUS Status;
  UINTN MaxMode, MaxColumns, MaxRows;
  UINTN BestMode = 0;
  UINTN BestColumns = 80;
  UINTN BestRows = 25;
  
  // Try to find the largest available mode
  MaxMode = gST->ConOut->Mode->MaxMode;
  
  for (UINTN Mode = 0; Mode < MaxMode; Mode++) {
    Status = gST->ConOut->QueryMode (
                            gST->ConOut,
                            Mode,
                            &MaxColumns,
                            &MaxRows
                            );
    
    if (!EFI_ERROR (Status)) {
      // Look for a larger mode that would give us better screen coverage
      if ((MaxColumns >= BestColumns && MaxRows >= BestRows) && 
          (MaxColumns > BestColumns || MaxRows > BestRows)) {
        BestMode = Mode;
        BestColumns = MaxColumns;
        BestRows = MaxRows;
      }
    }
  }
  
  // Set the best mode we found
  if (BestMode != gST->ConOut->Mode->Mode) {
    Status = gST->ConOut->SetMode (gST->ConOut, BestMode);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_WARN, "Failed to set console mode %d: %r\n", BestMode, Status));
    }
  }
  
  // Query the actual current mode
  Status = gST->ConOut->QueryMode (
                          gST->ConOut,
                          gST->ConOut->Mode->Mode,
                          &mConsoleColumns,
                          &mConsoleRows
                          );
  
  if (EFI_ERROR (Status)) {
    // Use larger default values
    mConsoleColumns = 100;
    mConsoleRows = 40;
  }
  
  DEBUG ((DEBUG_INFO, "Console size: %dx%d\n", mConsoleColumns, mConsoleRows));
}

**/

/**
  Get current submenu array based on selected main tab.

  @param ItemCount   Output parameter for number of items in submenu.

  @retval Pointer to current submenu array.
**/
/**

SUBMENU_ITEM*
GetCurrentSubmenuArray (
  OUT UINTN *ItemCount
  )
{
  switch (mSelectedMainTab) {
    case 0: // Main
      *ItemCount = ARRAY_SIZE (mMainSubmenu);
      return mMainSubmenu;
      
    case 1: // Advanced
      *ItemCount = ARRAY_SIZE (mAdvancedSubmenu);
      return mAdvancedSubmenu;
      
    case 2: // Security
      *ItemCount = ARRAY_SIZE (mSecuritySubmenu);
      return mSecuritySubmenu;
      
    case 3: // Save & Exit
      *ItemCount = ARRAY_SIZE (mSaveExitSubmenu);
      return mSaveExitSubmenu;
      
    default:
      *ItemCount = ARRAY_SIZE (mMainSubmenu);
      return mMainSubmenu;
  }
}

**/

/**
  Initialize mouse support by locating Simple Pointer Protocol.

  @retval EFI_SUCCESS   Mouse initialized successfully.
  @retval Others        Failed to initialize mouse.
**/

/**
EFI_STATUS
InitializeMouse (
  VOID
  )
{
  EFI_STATUS Status;
  
  // Initialize mouse state
  mMouseState.LeftButtonPressed = FALSE;
  mMouseState.LeftButtonReleased = FALSE;
  mMouseState.WasLeftButtonPressed = FALSE;
  mMouseState.ClickCount = 0;
  mMouseState.LastClickTime = 0;
  mMouseState.LastX = (INT32)(mConsoleColumns / 2); // Center of screen
  mMouseState.LastY = (INT32)(mConsoleRows / 2);
  mMouseState.Visible = TRUE;
  
  // Try to locate Simple Pointer Protocol
  Status = gBS->LocateProtocol (
                  &gEfiSimplePointerProtocolGuid,
                  NULL,
                  (VOID **)&mSimplePointer
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "Simple Pointer Protocol not available: %r\n", Status));
    mSimplePointer = NULL;
    return Status;
  }
  
  // Reset mouse
  Status = mSimplePointer->Reset (mSimplePointer, TRUE);
  if (EFI_ERROR (Status)) {
    Status = mSimplePointer->Reset (mSimplePointer, FALSE);
    if (EFI_ERROR (Status)) {
      mSimplePointer = NULL;
      return Status;
    }
  }
  
  DEBUG ((DEBUG_INFO, "Mouse initialized successfully\n"));
  return EFI_SUCCESS;
}

**/

/**
  Update mouse state by reading current mouse input.
**/

/**
VOID
UpdateMouseState (
  VOID
  )
{
  EFI_STATUS                Status;
  EFI_SIMPLE_POINTER_STATE  MouseState;
  
  if (mSimplePointer == NULL) {
    return;
  }
  
  Status = mSimplePointer->GetState (mSimplePointer, &MouseState);
  if (EFI_ERROR (Status)) {
    return;
  }
  
  // Handle relative movement
  INT32 DeltaX = MouseState.RelativeMovementX / 65536;
  INT32 DeltaY = MouseState.RelativeMovementY / 65536;
  
  if (DeltaX != 0 || DeltaY != 0) {
    mMouseState.LastX += DeltaX;
    mMouseState.LastY += DeltaY;
    
    // Boundary check
    if (mMouseState.LastX < 0) mMouseState.LastX = 0;
    if (mMouseState.LastY < 0) mMouseState.LastY = 0;
    if (mMouseState.LastX >= (INT32)mConsoleColumns) mMouseState.LastX = (INT32)mConsoleColumns - 1;
    if (mMouseState.LastY >= (INT32)mConsoleRows) mMouseState.LastY = (INT32)mConsoleRows - 1;
  }
  
  // Update button state
  mMouseState.WasLeftButtonPressed = mMouseState.LeftButtonPressed;
  mMouseState.LeftButtonPressed = MouseState.LeftButton;
  
  // Detect button release (click completion)
  if (mMouseState.WasLeftButtonPressed && !mMouseState.LeftButtonPressed) {
    mMouseState.LeftButtonReleased = TRUE;
    mMouseState.ClickCount++;
  } else {
    mMouseState.LeftButtonReleased = FALSE;
  }
}

**/

/**
  Draw mouse cursor at current position.
**/

/**
VOID
DrawMouseCursor (
  VOID
  )
{
  if (!mMouseState.Visible || mSimplePointer == NULL) {
    return;
  }
  
  gST->ConOut->SetCursorPosition (gST->ConOut, mMouseState.LastX, mMouseState.LastY);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
  Print (L"▓");
}

**/

/**
  Get tab index based on mouse position.

  @param MouseX   Mouse X coordinate.
  @param MouseY   Mouse Y coordinate.

  @retval Index   Tab index, or -1 if not over any tab.
**/

/**
UINTN
GetTabFromMousePosition (
  IN INT32 MouseX,
  IN INT32 MouseY
  )
{
  UINTN Index;
  
  // Check if mouse is on tab row (row 2)
  if (MouseY != 2) {
    return (UINTN)-1;
  }
  
  for (Index = 0; Index < mTotalMainTabs; Index++) {
    if (MouseX >= mMainTabs[Index].StartCol && MouseX <= mMainTabs[Index].EndCol) {
      return Index;
    }
  }
  
  return (UINTN)-1;
}
**/

/**
  Get submenu item index based on mouse position.

  @param MouseX   Mouse X coordinate.
  @param MouseY   Mouse Y coordinate.

  @retval Index   Submenu item index, or -1 if not over any item.
**/

/**
UINTN
GetSubItemFromMousePosition (
  IN INT32 MouseX,
  IN INT32 MouseY
  )
{
  UINTN Index;
  SUBMENU_ITEM *CurrentSubmenu;
  UINTN ItemCount;
  
  CurrentSubmenu = GetCurrentSubmenuArray (&ItemCount);
  
  for (Index = 0; Index < ItemCount; Index++) {
    if (MouseY == CurrentSubmenu[Index].StartRow + 1 && // Add offset for header rows
        MouseX >= CurrentSubmenu[Index].StartCol &&
        MouseX <= CurrentSubmenu[Index].EndCol) {
      return Index;
    }
  }
  
  return (UINTN)-1;
}

**/

/**
  Get current time and format it as string.

  @param TimeString   Output buffer for formatted time string.
**/

/**
VOID
GetCurrentTime (
  OUT CHAR16  *TimeString
  )
{
  EFI_TIME    Time;
  EFI_STATUS  Status;
  
  Status = gRT->GetTime (&Time, NULL);
  if (!EFI_ERROR (Status)) {
    UnicodeSPrint (
      TimeString, 
      50 * sizeof(CHAR16),
      L"%02d/%02d/%d  %02d:%02d:%02d",
      Time.Day,
      Time.Month,
      Time.Year,
      Time.Hour,
      Time.Minute,
      Time.Second
    );
  } else {
    UnicodeSPrint (TimeString, 50 * sizeof(CHAR16), L"[Date/Time Not Available]");
  }
}

**/

/**
  Draw blue background with proper full screen coverage.
**/

/**
VOID
DrawBlueBackground (
  VOID
  )
{
  UINTN Index, Col;
  
  // Get actual console size and try to maximize it
  GetConsoleSize ();
  
  // Clear screen and set blue background
  gST->ConOut->ClearScreen (gST->ConOut);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  
  // Fill entire screen with blue background
  for (Index = 0; Index < mConsoleRows; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Index);
    for (Col = 0; Col < mConsoleColumns; Col++) {
      Print (L" ");
    }
  }
  
  // Draw centered title bar - CDAC BANGALORE
  gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 13) / 2, 0);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
  Print (L"CDAC BANGALORE");
  
  // Draw BIOS Setup Utility title below
  gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 18) / 2, 1);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  Print (L"BIOS Setup Utility");
}

**/
/**
  Draw horizontal tabs at top with proper spacing and no duplication.
**/

/**
VOID
DrawHorizontalTabs (
  VOID
  )
{
  UINTN Index, Col;
  
  // Clear tab row completely
  gST->ConOut->SetCursorPosition (gST->ConOut, 0, 2);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  for (Col = 0; Col < mConsoleColumns; Col++) {
    Print (L" ");
  }
  
  // Draw each tab once only
  for (Index = 0; Index < mTotalMainTabs; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, mMainTabs[Index].StartCol, 2);
    
    if (Index == mSelectedMainTab) {
      // Selected tab - light gray background with black text
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    } else {
      // Unselected tab - blue background with white text
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
    }
    
    // Print tab text with padding
    Print (L" %s ", mMainTabs[Index].MenuText);
    
    // Fill remaining tab width with spaces
    UINTN TextLen = StrLen(mMainTabs[Index].MenuText) + 2; // +2 for the spaces around text
    UINTN CurrentPos = mMainTabs[Index].StartCol + TextLen;
    
    while (CurrentPos <= mMainTabs[Index].EndCol) {
      Print (L" ");
      CurrentPos++;
    }
  }
}

**/
/**
  Draw the three-section layout with proper boundaries.
**/

/**
VOID
DrawThreeSectionLayout (
  VOID
  )
{
  UINTN Row, Col;
  UINTN SeparatorCol = mConsoleColumns / 2; // Dynamic separator position
  
  // Fill the main content area
  for (Row = 3; Row < mConsoleRows - 2; Row++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Row);
    
    // Print left section (submenu area) - light gray background
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    for (Col = 0; Col < SeparatorCol; Col++) {
      Print (L" ");
    }
    
    // Print vertical separator
    if (SeparatorCol < mConsoleColumns) {
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"│");
    }
    
    // Print right section (info area) - light gray background
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    for (Col = SeparatorCol + 1; Col < mConsoleColumns; Col++) {
      Print (L" ");
    }
  }
}

**/

/**
  Display current submenu in left section.
**/

/**
VOID
DisplayCurrentSubmenu (
  VOID
  )
{
  UINTN Index, Col;
  SUBMENU_ITEM *CurrentSubmenu;
  UINTN ItemCount;
  UINTN SeparatorCol = mConsoleColumns / 2;
  
  CurrentSubmenu = GetCurrentSubmenuArray (&ItemCount);
  mTotalSubItems = ItemCount;
  
  // Clear left section properly
  for (Index = 4; Index < mConsoleRows - 3; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 1, Index);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    for (Col = 1; Col < SeparatorCol - 1; Col++) {
      Print (L" ");
    }
  }
  
  // Display submenu items
  for (Index = 0; Index < ItemCount && Index < 15; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 3, Index + 5); // Fixed positioning
    
    if (Index == mSelectedSubItem) {
      // Highlight selected item
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
    } else {
      // Normal item
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    }
    
    Print (L"%s", CurrentSubmenu[Index].MenuText);
  }
}

**/

/**
  Display selected item information in right section.
**/

/**
VOID
DisplaySelectedTabInfo (
  VOID
  )
{
  SUBMENU_ITEM *CurrentSubmenu;
  UINTN ItemCount;
  CHAR16 TimeString[100];
  UINTN SeparatorCol = mConsoleColumns / 2;
  UINTN RightStart = SeparatorCol + 2;
  UINTN Index, Col;
  
  CurrentSubmenu = GetCurrentSubmenuArray (&ItemCount);
  
  // Clear right section properly
  for (Index = 4; Index < mConsoleRows - 3; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, Index);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    for (Col = RightStart; Col < mConsoleColumns - 1; Col++) {
      Print (L" ");
    }
  }
  
  // Display information header
  gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 5);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"Selected Item:");
  
  // Display based on current tab and selected submenu item
  if (mSelectedSubItem < ItemCount) {
    // Show selected item name
    gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 7);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_DARKGRAY | EFI_BACKGROUND_LIGHTGRAY);
    Print (L"%s", CurrentSubmenu[mSelectedSubItem].MenuText);
    
    // Show description
    gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 9);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    Print (L"%s", CurrentSubmenu[mSelectedSubItem].Description);
    
    // For Main tab's Change Date & Time item, show additional time info
    if (mSelectedMainTab == 0 && mSelectedSubItem == 3) {
      GetCurrentTime (TimeString);
      gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 12);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"Current: %s", TimeString);
      
      gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 14);
      Print (L"Use +/- to change values");
    }
  }
  
  // Show current tab name at bottom of right section
  gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, mConsoleRows - 5);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"Current Tab: %s", mMainTabs[mSelectedMainTab].MenuText);
}

**/

/**
  Display navigation help in bottom section.
**/

/**
VOID
DisplayNavigationHelp (
  VOID
  )
{
  UINTN Col;
  
  // Bottom blue bars with navigation help
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  
  // Fill bottom two rows with blue background
  for (UINTN Row = mConsoleRows - 2; Row < mConsoleRows; Row++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Row);
    for (Col = 0; Col < mConsoleColumns; Col++) {
      Print (L" ");
    }
  }
  
  // Display navigation instructions on bottom row
  gST->ConOut->SetCursorPosition (gST->ConOut, 2, mConsoleRows - 1);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
 // Print (L"←→: Select Screen  ↑↓: Select Item  Enter: Select  +/-: Change Opt  F1: Help  ESC: Exit");
  Print (L"←→: Select Screen  ↑↓: Select Item  Enter: Select  ESC: Exit");
}

**/

/**
  Display BIOS setup form with blue theme and horizontal layout.
**/

/**
VOID
DisplayBiosSetupForm (
  VOID
  )
{
  mSelectedMainTab = 0;
  mSelectedSubItem = 0;
  mTotalMainTabs = ARRAY_SIZE (mMainTabs);
  mInBiosSetup = TRUE;
  
  // Initialize mouse support
  InitializeMouse ();
  
  // Draw the complete BIOS setup interface
  DrawBlueBackground ();
  DrawHorizontalTabs ();
  DrawThreeSectionLayout ();
  DisplayCurrentSubmenu ();
  DisplaySelectedTabInfo ();
  DisplayNavigationHelp ();
  
  // Draw mouse cursor
  DrawMouseCursor ();
  
  // Reset console input for clean key detection
  gST->ConIn->Reset (gST->ConIn, FALSE);
  
  // Start navigation handling
  HandleBiosSetupNavigation ();
}
**/

/**
  Handle tab selection based on current selected tab.
**/
/**
VOID
HandleTabSelection (
  VOID
  )
{
  switch (mSelectedMainTab) {
    case 3: // Save & Exit
      if (mSelectedSubItem == 0) { // Save Changes and Exit
        gST->ConOut->ClearScreen (gST->ConOut);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
        gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 35) / 2, mConsoleRows / 2);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_GREEN | EFI_BACKGROUND_BLACK);
        Print (L"✓ Settings saved. Shutting down system...");
        gBS->Stall (2000000);
        gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
        mInBiosSetup = FALSE;
      } else if (mSelectedSubItem == 1) { // Discard Changes and Exit
        gST->ConOut->ClearScreen (gST->ConOut);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
        gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 40) / 2, mConsoleRows / 2);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_RED | EFI_BACKGROUND_BLACK);
        Print (L"✗ Exiting without saving. Shutting down...");
        gBS->Stall (2000000);
        gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
        mInBiosSetup = FALSE;
      } else if (mSelectedSubItem == 2) { // Save Changes
        // Show save confirmation
        gST->ConOut->SetCursorPosition (gST->ConOut, mConsoleColumns / 2 + 2, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_GREEN | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"✓ Settings saved successfully");
        gBS->Stall (1500000);
        // Clear the message
        gST->ConOut->SetCursorPosition (gST->ConOut, mConsoleColumns / 2 + 2, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"                            ");
      } else if (mSelectedSubItem == 3) { // Discard Changes
        // Show discard confirmation
        gST->ConOut->SetCursorPosition (gST->ConOut, mConsoleColumns / 2 + 2, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_RED | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"✗ Changes discarded");
        gBS->Stall (1500000);
        // Clear the message
        gST->ConOut->SetCursorPosition (gST->ConOut, mConsoleColumns / 2 + 2, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"                   ");
      } else if (mSelectedSubItem == 4) { // Load Setup Defaults
        // Show defaults loaded confirmation
        gST->ConOut->SetCursorPosition (gST->ConOut, mConsoleColumns / 2 + 2, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"⚡ Default settings loaded");
        gBS->Stall (1500000);
        // Clear the message
        gST->ConOut->SetCursorPosition (gST->ConOut, mConsoleColumns / 2 + 2, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"                         ");
      }
      break;
      
    default:
      // For other tabs, show a brief action message
      gST->ConOut->SetCursorPosition (gST->ConOut, mConsoleColumns / 2 + 2, 15);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"⚡ Action performed...");
      gBS->Stall (800000);
      // Clear the message
      gST->ConOut->SetCursorPosition (gST->ConOut, mConsoleColumns / 2 + 2, 15);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"                     ");
      break;
  }
}

**/

/**
  Handle navigation within BIOS setup using arrow keys and mouse.
**/

/**
VOID
HandleBiosSetupNavigation (
  VOID
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  UINTN          MouseTabIndex, MouseSubIndex;
  
  while (mInBiosSetup) {
    // Update mouse state
    UpdateMouseState ();
    
    // Redraw mouse cursor
    DrawMouseCursor ();
    
    // Handle mouse input
    if (mSimplePointer != NULL) {
      MouseTabIndex = GetTabFromMousePosition (mMouseState.LastX, mMouseState.LastY);
      MouseSubIndex = GetSubItemFromMousePosition (mMouseState.LastX, mMouseState.LastY);
      
      // Handle mouse clicks on tabs
      if (mMouseState.LeftButtonReleased && MouseTabIndex != (UINTN)-1) {
        if (MouseTabIndex != mSelectedMainTab) {
          mSelectedMainTab = MouseTabIndex;
          mSelectedSubItem = 0; // Reset submenu selection
          
          // Redraw tabs and submenu
          DrawHorizontalTabs ();
          DisplayCurrentSubmenu ();
          DisplaySelectedTabInfo ();
        }
        mMouseState.LeftButtonReleased = FALSE;
      }
      
      // Handle mouse clicks on submenu items
      if (mMouseState.LeftButtonReleased && MouseSubIndex != (UINTN)-1) {
        if (MouseSubIndex != mSelectedSubItem) {
          mSelectedSubItem = MouseSubIndex;
          
          // Redraw submenu and info
          DisplayCurrentSubmenu ();
          DisplaySelectedTabInfo ();
        } else {
          // Double-click simulation - execute selection
          HandleTabSelection ();
          if (!mInBiosSetup) return;
        }
        mMouseState.LeftButtonReleased = FALSE;
      }
    }
    
    // Handle keyboard input
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
    
    if (!EFI_ERROR (Status)) {
      switch (Key.ScanCode) {
        case SCAN_LEFT:
          // Move left in main tabs
          if (mSelectedMainTab > 0) {
            mSelectedMainTab--;
          } else {
            mSelectedMainTab = mTotalMainTabs - 1; // Wrap to rightmost
          }
          mSelectedSubItem = 0; // Reset submenu selection
          
          // Redraw interface
          DrawHorizontalTabs ();
          DisplayCurrentSubmenu ();
          DisplaySelectedTabInfo ();
          break;
          
        case SCAN_RIGHT:
          // Move right in main tabs
          if (mSelectedMainTab < mTotalMainTabs - 1) {
            mSelectedMainTab++;
          } else {
            mSelectedMainTab = 0; // Wrap to leftmost
          }
          mSelectedSubItem = 0; // Reset submenu selection
          
          // Redraw interface
          DrawHorizontalTabs ();
          DisplayCurrentSubmenu ();
          DisplaySelectedTabInfo ();
          break;
          
        case SCAN_UP:
          // Move up in submenu
          if (mSelectedSubItem > 0) {
            mSelectedSubItem--;
          } else {
            mSelectedSubItem = mTotalSubItems - 1; // Wrap to bottom
          }
          
          // Update submenu and info
          DisplayCurrentSubmenu ();
          DisplaySelectedTabInfo ();
          break;
          
        case SCAN_DOWN:
          // Move down in submenu
          if (mSelectedSubItem < mTotalSubItems - 1) {
            mSelectedSubItem++;
          } else {
            mSelectedSubItem = 0; // Wrap to top
          }
          
          // Update submenu and info
          DisplayCurrentSubmenu ();
          DisplaySelectedTabInfo ();
          break;
          
        case SCAN_ESC:
          // Exit BIOS setup
          mInBiosSetup = FALSE;
          gST->ConOut->ClearScreen (gST->ConOut);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
          gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 40) / 2, mConsoleRows / 2);
          Print (L"🚪 Exiting BIOS Setup. Shutting down system...");
          gBS->Stall (2000000);
          gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
          return;
          
        default:
          // Check for ENTER key
          if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
            HandleTabSelection ();
            if (!mInBiosSetup) {
              return;
            }
          }
          break;
      }
    } else {
      // Small delay to prevent excessive CPU usage
      gBS->Stall (50000); // 50ms
    }
  }
}

**/

/**
  Monitor keyboard input for F12 key press only.
**/

/**
VOID
EFIAPI
KeyboardMonitor (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  
  // Only monitor after message is shown and not in BIOS setup
  if (!mMessageShown || mInBiosSetup) {
    return;
  }
  
  // Check if key is available
  Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
  
  if (!EFI_ERROR (Status)) {
    // Check for F12 key only
    if (Key.ScanCode == SCAN_F12) {
      DEBUG ((DEBUG_INFO, "F12 key pressed - showing BIOS setup form\n"));
      
      // Cancel timer and keyboard events
      gBS->SetTimer (mKeyMonitorEvent, TimerCancel, 0);
      gBS->SetTimer (mTimerEvent, TimerCancel, 0);
      
      // Display BIOS setup form
      DisplayBiosSetupForm ();
      
      // After exiting BIOS setup, shutdown
      gST->ConOut->ClearScreen (gST->ConOut);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
      gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 40) / 2, mConsoleRows / 2);
      Print (L"✅ BIOS Setup completed. Shutting down system...");
      gBS->Stall (2000000);
      gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
      return;
    }
  }
}

**/

/**
  Timer callback function to handle logo display timing.

  @param Event      The event that is signaled.
  @param Context    The context passed to the event.
**/

/**
VOID
EFIAPI
TimerCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  mTimerCounter++;
  
  // After 5 seconds, show the message below logo and wait for F12
  if (mTimerCounter == 50 && !mMessageShown) { // 50 * 100ms = 5 seconds
    // Clear the entire screen first
    gST->ConOut->ClearScreen (gST->ConOut);
    
    // Get console size for proper centering
    GetConsoleSize ();
    
    // Position cursor below the logo area (center of screen)
    gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 28) / 2, mConsoleRows / 2 + 5);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
    Print (L"Press F12 key for BIOS setup");
    
    mMessageShown = TRUE;
    
    // Reset console input for clean key detection
    gST->ConIn->Reset (gST->ConIn, FALSE);
    
    DEBUG ((DEBUG_INFO, "Message displayed, waiting for F12 key\n"));
  }
}

**/

/**
  Setup timing and keyboard monitoring after logo is displayed.
**/

/**
VOID
SetupLogoTiming (
  VOID
  )
{
  EFI_STATUS  Status;
  
  if (mLogoDisplayed) {
    return;
  }
  
  DEBUG ((DEBUG_INFO, "Setting up logo timing and keyboard monitoring\n"));
  
  // Create timer event for logo timing
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  TimerCallback,
                  NULL,
                  &mTimerEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    Status = gBS->SetTimer (
                    mTimerEvent,
                    TimerPeriodic,
                    1000000  // 100ms in 100ns units
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set timer for mTimerEvent: %r\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to create mTimerEvent: %r\n", Status));
  }
  
  // Create keyboard monitoring event
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  KeyboardMonitor,
                  NULL,
                  &mKeyMonitorEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    Status = gBS->SetTimer (
                    mKeyMonitorEvent,
                    TimerPeriodic,
                    500000  // 50ms in 100ns units
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set timer for mKeyMonitorEvent: %r\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to create mKeyMonitorEvent: %r\n", Status));
  }
  
  mLogoDisplayed = TRUE;
}

**/
/**
  Load a platform logo image and return its data and attributes.
**/

/**
EFI_STATUS
EFIAPI
GetImage (
  IN     EDKII_PLATFORM_LOGO_PROTOCOL        *This,
  IN OUT UINT32                              *Instance,
  OUT EFI_IMAGE_INPUT                        *Image,
  OUT EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE  *Attribute,
  OUT INTN                                   *OffsetX,
  OUT INTN                                   *OffsetY
  )
{
  UINT32      Current;
  EFI_STATUS  Status;

  if ((Instance == NULL) || (Image == NULL) ||
      (Attribute == NULL) || (OffsetX == NULL) || (OffsetY == NULL))
  {
    return EFI_INVALID_PARAMETER;
  }

  Current = *Instance;
  if (Current >= ARRAY_SIZE (mLogos)) {
    return EFI_NOT_FOUND;
  }

  (*Instance)++;
  *Attribute = mLogos[Current].Attribute;
  *OffsetX   = mLogos[Current].OffsetX;
  *OffsetY   = mLogos[Current].OffsetY;
  
  // Get image from HII database
  Status = mHiiImageEx->GetImageEx (mHiiImageEx, mHiiHandle, mLogos[Current].ImageId, Image);
  
  // After first logo is displayed, setup timing
  if (Current == 0 && !EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Logo displayed, setting up timing\n"));
    SetupLogoTiming ();
  }
  
  return Status;
}

EDKII_PLATFORM_LOGO_PROTOCOL  mPlatformLogo = {
  GetImage
};

**/
/**
  Cleanup function called when driver is unloaded.
**/
/**
EFI_STATUS
EFIAPI
LogoDriverUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
  // Close events if they exist
  if (mTimerEvent != NULL) {
    gBS->CloseEvent (mTimerEvent);
    mTimerEvent = NULL;
  }
  
  if (mKeyMonitorEvent != NULL) {
    gBS->CloseEvent (mKeyMonitorEvent);
    mKeyMonitorEvent = NULL;
  }
  
  if (mBiosSetupKeyEvent != NULL) {
    gBS->CloseEvent (mBiosSetupKeyEvent);
    mBiosSetupKeyEvent = NULL;
  }
  
  if (mMouseEvent != NULL) {
    gBS->CloseEvent (mMouseEvent);
    mMouseEvent = NULL;
  }
  
  return EFI_SUCCESS;
}
**/

/**
  Entrypoint of this module.
**/

/**
EFI_STATUS
EFIAPI
InitializeLogo (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                   Status;
  EFI_HII_PACKAGE_LIST_HEADER  *PackageList;
  EFI_HII_DATABASE_PROTOCOL    *HiiDatabase;
  EFI_HANDLE                   Handle;

  DEBUG ((DEBUG_INFO, "Logo Driver: Initializing with blue theme and horizontal tabs...\n"));

  // Initialize global variables
  mInBiosSetup = FALSE;
  mSelectedMainTab = 0;
  mSelectedSubItem = 0;
  mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;
  mTotalMainTabs = ARRAY_SIZE (mMainTabs);

  // Locate HII Database Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiDatabaseProtocolGuid,
                  NULL,
                  (VOID **)&HiiDatabase
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Database Protocol: %r\n", Status));
    return Status;
  }

  // Locate HII Image Ex Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiImageExProtocolGuid,
                  NULL,
                  (VOID **)&mHiiImageEx
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Image Ex Protocol: %r\n", Status));
    return Status;
  }

  // Retrieve HII package list from ImageHandle
  Status = gBS->OpenProtocol (
                  ImageHandle,
                  &gEfiHiiPackageListProtocolGuid,
                  (VOID **)&PackageList,
                  ImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "HII Image Package with logo not found in PE/COFF resource section\n"));
    return Status;
  }

  // Publish HII package list to HII Database
  Status = HiiDatabase->NewPackageList (
                          HiiDatabase,
                          PackageList,
                          NULL,
                          &mHiiHandle
                          );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to add package list to HII Database: %r\n", Status));
    return Status;
  }

  // Install Platform Logo Protocol
  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEdkiiPlatformLogoProtocolGuid,
                  &mPlatformLogo,
                  NULL
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to install Platform Logo Protocol: %r\n", Status));
    HiiDatabase->RemovePackageList (HiiDatabase, mHiiHandle);
    return Status;
  }

  DEBUG ((DEBUG_INFO, "Logo Driver: Initialized successfully with blue theme layout\n"));
  
  return EFI_SUCCESS;
}


**/


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// This is not working properly.

// BIOS setup utility with blue theme and horizontal menu layout 

/** @file
  Logo DXE Driver with F12 key BIOS setup functionality.
  Shows logo, waits for F12 key, then displays BIOS setup form with navigation.
  Enhanced with blue theme, horizontal menu tabs and three-section layout.

Copyright (c) 2016 - 2017, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent
**/
/**

#include <Uefi.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/HiiImageEx.h>
#include <Protocol/PlatformLogo.h>
#include <Protocol/HiiPackageList.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/SimpleTextOut.h>
#include <Protocol/SimplePointer.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>
#include <Library/MemoryAllocationLib.h>

typedef struct {
  EFI_IMAGE_ID                             ImageId;
  EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE    Attribute;
  INTN                                     OffsetX;
  INTN                                     OffsetY;
} LOGO_ENTRY;

// Menu item structure for horizontal tabs
typedef struct {
  CHAR16  *MenuText;
  CHAR16  *Description;
  UINT32  StartCol;    // Column where menu tab starts
  UINT32  EndCol;      // Column where menu tab ends
  UINT32  TabWidth;    // Width of the tab
} MENU_TAB;

// Submenu item structure
typedef struct {
  CHAR16  *MenuText;
  CHAR16  *Description;
  UINT32  StartRow;    // Row where menu item starts
  UINT32  StartCol;    // Column where menu item starts  
  UINT32  EndRow;      // Row where menu item ends
  UINT32  EndCol;      // Column where menu item ends
} SUBMENU_ITEM;

// Mouse state structure
typedef struct {
  BOOLEAN  LeftButtonPressed;
  BOOLEAN  LeftButtonReleased;
  BOOLEAN  WasLeftButtonPressed;
  UINT32   ClickCount;
  UINT64   LastClickTime;
  INT32    LastX;
  INT32    LastY;
  BOOLEAN  Visible;
} MOUSE_STATE;

// Menu level enumeration
typedef enum {
  MENU_LEVEL_MAIN_MENU = 0,
  MENU_LEVEL_MAIN_SUBMENU,
  MENU_LEVEL_SECURITY_SUBMENU,
  MENU_LEVEL_ADVANCED_SUBMENU,
  MENU_LEVEL_UEFI_SUBMENU
} MENU_LEVEL;

// Global variables
EFI_HII_IMAGE_EX_PROTOCOL  *mHiiImageEx;
EFI_HII_HANDLE             mHiiHandle;
EFI_EVENT                  mKeyMonitorEvent;
EFI_EVENT                  mTimerEvent;
EFI_EVENT                  mBiosSetupKeyEvent;
EFI_EVENT                  mMouseEvent;
EFI_SIMPLE_POINTER_PROTOCOL *mSimplePointer = NULL;
BOOLEAN                    mLogoDisplayed = FALSE;
BOOLEAN                    mMessageShown = FALSE;
BOOLEAN                    mInBiosSetup = FALSE;
UINTN                      mTimerCounter = 0;
UINTN                      mSelectedMainTab = 0;
UINTN                      mSelectedSubItem = 0;
UINTN                      mTotalMainTabs = 0;
UINTN                      mTotalSubItems = 0;
MENU_LEVEL                 mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;
MOUSE_STATE                mMouseState;
UINTN                      mConsoleColumns = 120;  // Larger default size for full screen
UINTN                      mConsoleRows = 50;      // Larger default size for full screen

// Main horizontal tabs (fixed positioning to prevent overlap)
MENU_TAB mMainTabs[] = {
  { L"Main", L"System Information and Basic Settings", 1, 20, 20 },
  { L"Advanced", L"Advanced Configuration Options", 21, 45, 25 },
  { L"Security", L"Security and password Settings", 46, 70, 25 },
  { L"Save & Exit", L"Save changes and exit setup", 71, 100, 30 }
};

// Main submenu items (for Main tab)
SUBMENU_ITEM mMainSubmenu[] = {
  { L"BIOS Information", L"View BIOS version and system information", 5, 5, 5, 35 },
  { L"BIOS Eventlog", L"Display BIOS event log entries", 6, 5, 6, 35 },
  { L"Update system BIOS", L"Update system BIOS firmware", 7, 5, 7, 35 },
  { L"Change Date & Time", L"Configure system date and time", 8, 5, 8, 35 },
};

// Advanced submenu items
SUBMENU_ITEM mAdvancedSubmenu[] = {
  { L"Boot Options", L"Configure boot device priority and settings", 5, 5, 5, 35 },
  { L"Port Options", L"Configure USB, SATA, and other port settings", 6, 5, 6, 35 },
  { L"System Options", L"Configure CPU, memory, and system settings", 7, 5, 7, 35 },
  { L"Power Management", L"Configure power and thermal settings", 8, 5, 8, 35 },
};

// Security submenu items
SUBMENU_ITEM mSecuritySubmenu[] = {
  { L"Administrator Tools", L"Administrative security management tools", 5, 5, 5, 35 },
  { L"Create BIOS Password", L"Set BIOS administrator password protection", 6, 5, 6, 35 },
  { L"TPM Security", L"Configure Trusted Platform Module settings", 7, 5, 7, 35 },
  { L"Secure Boot", L"Enable or disable UEFI Secure Boot", 8, 5, 8, 35 },
};

// Save & Exit submenu items
SUBMENU_ITEM mSaveExitSubmenu[] = {
  { L"Save Changes and Exit", L"Save current settings and exit setup", 5, 5, 5, 35 },
  { L"Discard Changes and Exit", L"Exit setup without saving any changes", 6, 5, 6, 35 },
  { L"Save Changes", L"Save current settings and continue", 7, 5, 7, 35 },
  { L"Discard Changes", L"Reset to previous saved settings", 8, 5, 8, 35 },
  { L"Load Setup Defaults", L"Load factory default settings", 9, 5, 9, 35 }
};

// Function declarations
VOID DisplayBiosSetupForm (VOID);
VOID GetCurrentTime (OUT CHAR16 *TimeString);
VOID HandleBiosSetupNavigation (VOID);
VOID DrawBlueBackground (VOID);
VOID DrawHorizontalTabs (VOID);
VOID DrawThreeSectionLayout (VOID);
VOID DisplayCurrentSubmenu (VOID);
VOID DisplaySelectedTabInfo (VOID);
VOID DisplayNavigationHelp (VOID);
EFI_STATUS InitializeMouse (VOID);
VOID UpdateMouseState (VOID);
UINTN GetTabFromMousePosition (IN INT32 MouseX, IN INT32 MouseY);
UINTN GetSubItemFromMousePosition (IN INT32 MouseX, IN INT32 MouseY);
VOID HandleTabSelection (VOID);
VOID HandleSubItemSelection (VOID);
SUBMENU_ITEM* GetCurrentSubmenuArray (OUT UINTN *ItemCount);
VOID DrawMouseCursor (VOID);
VOID GetConsoleSize (VOID);

LOGO_ENTRY mLogos[] = {
  {
    IMAGE_TOKEN (IMG_LOGO),
    EdkiiPlatformLogoDisplayAttributeCenter,
    0,
    0
  }
};

**/

/**
  Get console size and set to maximum available size for full screen display.
**/

/**
VOID
GetConsoleSize (
  VOID
  )
{
  EFI_STATUS Status;
  UINTN MaxMode, TestColumns, TestRows;
  UINTN BestMode = 0;
  UINTN BestColumns = 80;
  UINTN BestRows = 25;
  
  // Try to find the largest available console mode
  MaxMode = gST->ConOut->Mode->MaxMode;
  
  for (UINTN Mode = 0; Mode < MaxMode; Mode++) {
    Status = gST->ConOut->QueryMode (
                            gST->ConOut,
                            Mode,
                            &TestColumns,
                            &TestRows
                            );
    
    if (!EFI_ERROR (Status)) {
      // Look for largest mode (prioritize width first, then height)
      if ((TestColumns * TestRows) > (BestColumns * BestRows)) {
        BestMode = Mode;
        BestColumns = TestColumns;
        BestRows = TestRows;
      }
    }
  }
  
  // Set the largest mode we found
  Status = gST->ConOut->SetMode (gST->ConOut, BestMode);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "Failed to set console mode %d: %r\n", BestMode, Status));
  }
  
  // Get the actual current mode settings
  Status = gST->ConOut->QueryMode (
                          gST->ConOut,
                          gST->ConOut->Mode->Mode,
                          &mConsoleColumns,
                          &mConsoleRows
                          );
  
  if (EFI_ERROR (Status)) {
    // Force larger default values if query fails
    mConsoleColumns = 120;
    mConsoleRows = 50;
  }
  
  // Ensure minimum size for proper display
  if (mConsoleColumns < 100) mConsoleColumns = 100;
  if (mConsoleRows < 40) mConsoleRows = 40;
  
  DEBUG ((DEBUG_INFO, "Console size set to: %dx%d\n", mConsoleColumns, mConsoleRows));
}


**/
/**
  Get current submenu array based on selected main tab.

  @param ItemCount   Output parameter for number of items in submenu.

  @retval Pointer to current submenu array.
**/

/**
SUBMENU_ITEM*
GetCurrentSubmenuArray (
  OUT UINTN *ItemCount
  )
{
  switch (mSelectedMainTab) {
    case 0: // Main
      *ItemCount = ARRAY_SIZE (mMainSubmenu);
      return mMainSubmenu;
      
    case 1: // Advanced
      *ItemCount = ARRAY_SIZE (mAdvancedSubmenu);
      return mAdvancedSubmenu;
      
    case 2: // Security
      *ItemCount = ARRAY_SIZE (mSecuritySubmenu);
      return mSecuritySubmenu;
      
    case 3: // Save & Exit
      *ItemCount = ARRAY_SIZE (mSaveExitSubmenu);
      return mSaveExitSubmenu;
      
    default:
      *ItemCount = ARRAY_SIZE (mMainSubmenu);
      return mMainSubmenu;
  }
}

**/

/**
  Initialize mouse support by locating Simple Pointer Protocol.

  @retval EFI_SUCCESS   Mouse initialized successfully.
  @retval Others        Failed to initialize mouse.
**/
/**
EFI_STATUS
InitializeMouse (
  VOID
  )
{
  EFI_STATUS Status;
  
  // Initialize mouse state
  mMouseState.LeftButtonPressed = FALSE;
  mMouseState.LeftButtonReleased = FALSE;
  mMouseState.WasLeftButtonPressed = FALSE;
  mMouseState.ClickCount = 0;
  mMouseState.LastClickTime = 0;
  mMouseState.LastX = (INT32)(mConsoleColumns / 2); // Center of screen
  mMouseState.LastY = (INT32)(mConsoleRows / 2);
  mMouseState.Visible = TRUE;
  
  // Try to locate Simple Pointer Protocol
  Status = gBS->LocateProtocol (
                  &gEfiSimplePointerProtocolGuid,
                  NULL,
                  (VOID **)&mSimplePointer
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "Simple Pointer Protocol not available: %r\n", Status));
    mSimplePointer = NULL;
    return Status;
  }
  
  // Reset mouse
  Status = mSimplePointer->Reset (mSimplePointer, TRUE);
  if (EFI_ERROR (Status)) {
    Status = mSimplePointer->Reset (mSimplePointer, FALSE);
    if (EFI_ERROR (Status)) {
      mSimplePointer = NULL;
      return Status;
    }
  }
  
  DEBUG ((DEBUG_INFO, "Mouse initialized successfully\n"));
  return EFI_SUCCESS;
}

**/

/**
  Update mouse state by reading current mouse input.
**/

/**
VOID
UpdateMouseState (
  VOID
  )
{
  EFI_STATUS                Status;
  EFI_SIMPLE_POINTER_STATE  MouseState;
  
  if (mSimplePointer == NULL) {
    return;
  }
  
  Status = mSimplePointer->GetState (mSimplePointer, &MouseState);
  if (EFI_ERROR (Status)) {
    return;
  }
  
  // Handle relative movement
  INT32 DeltaX = MouseState.RelativeMovementX / 65536;
  INT32 DeltaY = MouseState.RelativeMovementY / 65536;
  
  if (DeltaX != 0 || DeltaY != 0) {
    mMouseState.LastX += DeltaX;
    mMouseState.LastY += DeltaY;
    
    // Boundary check
    if (mMouseState.LastX < 0) mMouseState.LastX = 0;
    if (mMouseState.LastY < 0) mMouseState.LastY = 0;
    if (mMouseState.LastX >= (INT32)mConsoleColumns) mMouseState.LastX = (INT32)mConsoleColumns - 1;
    if (mMouseState.LastY >= (INT32)mConsoleRows) mMouseState.LastY = (INT32)mConsoleRows - 1;
  }
  
  // Update button state
  mMouseState.WasLeftButtonPressed = mMouseState.LeftButtonPressed;
  mMouseState.LeftButtonPressed = MouseState.LeftButton;
  
  // Detect button release (click completion)
  if (mMouseState.WasLeftButtonPressed && !mMouseState.LeftButtonPressed) {
    mMouseState.LeftButtonReleased = TRUE;
    mMouseState.ClickCount++;
  } else {
    mMouseState.LeftButtonReleased = FALSE;
  }
}

**/
/**
  Draw mouse cursor at current position.
**/

/**
VOID
DrawMouseCursor (
  VOID
  )
{
  if (!mMouseState.Visible || mSimplePointer == NULL) {
    return;
  }
  
  gST->ConOut->SetCursorPosition (gST->ConOut, mMouseState.LastX, mMouseState.LastY);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
  Print (L"▓");
}

**/
/**
  Get tab index based on mouse position.

  @param MouseX   Mouse X coordinate.
  @param MouseY   Mouse Y coordinate.

  @retval Index   Tab index, or -1 if not over any tab.
**/

/**
UINTN
GetTabFromMousePosition (
  IN INT32 MouseX,
  IN INT32 MouseY
  )
{
  UINTN Index;
  
  // Check if mouse is on tab row (row 2)
  if (MouseY != 2) {
    return (UINTN)-1;
  }
  
  for (Index = 0; Index < mTotalMainTabs; Index++) {
    if (MouseX >= mMainTabs[Index].StartCol && MouseX <= mMainTabs[Index].EndCol) {
      return Index;
    }
  }
  
  return (UINTN)-1;
}

**/

/**
  Get submenu item index based on mouse position.

  @param MouseX   Mouse X coordinate.
  @param MouseY   Mouse Y coordinate.

  @retval Index   Submenu item index, or -1 if not over any item.
**/

/**
UINTN
GetSubItemFromMousePosition (
  IN INT32 MouseX,
  IN INT32 MouseY
  )
{
  UINTN Index;
  SUBMENU_ITEM *CurrentSubmenu;
  UINTN ItemCount;
  
  CurrentSubmenu = GetCurrentSubmenuArray (&ItemCount);
  
  for (Index = 0; Index < ItemCount; Index++) {
    if (MouseY == CurrentSubmenu[Index].StartRow + 1 && // Add offset for header rows
        MouseX >= CurrentSubmenu[Index].StartCol &&
        MouseX <= CurrentSubmenu[Index].EndCol) {
      return Index;
    }
  }
  
  return (UINTN)-1;
}

**/

/**
  Get current time and format it as string.

  @param TimeString   Output buffer for formatted time string.
**/

/**
VOID
GetCurrentTime (
  OUT CHAR16  *TimeString
  )
{
  EFI_TIME    Time;
  EFI_STATUS  Status;
  
  Status = gRT->GetTime (&Time, NULL);
  if (!EFI_ERROR (Status)) {
    UnicodeSPrint (
      TimeString, 
      50 * sizeof(CHAR16),
      L"%02d/%02d/%d  %02d:%02d:%02d",
      Time.Day,
      Time.Month,
      Time.Year,
      Time.Hour,
      Time.Minute,
      Time.Second
    );
  } else {
    UnicodeSPrint (TimeString, 50 * sizeof(CHAR16), L"[Date/Time Not Available]");
  }
}

**/

/**
  Draw blue background with proper full screen coverage and persistent CDAC title.
**/

/**
VOID
DrawBlueBackground (
  VOID
  )
{
  UINTN Index, Col;
  
  // Get actual console size and try to maximize it first
  GetConsoleSize ();
  
  // Clear screen completely
  gST->ConOut->ClearScreen (gST->ConOut);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  
  // Fill entire screen with blue background - every pixel
  for (Index = 0; Index < mConsoleRows; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Index);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
    for (Col = 0; Col < mConsoleColumns; Col++) {
      Print (L" ");
    }
  }
  
  // Draw CDAC BANGALORE title at top - this will stay visible
  gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 13) / 2, 0);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
  Print (L"CDAC BANGALORE");
  
  // Draw BIOS Setup Utility title 
  gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 18) / 2, 1);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  Print (L"BIOS Setup Utility");
}

**/

/**
  Draw horizontal tabs at top with proper spacing and no duplication.
**/

/**
VOID
DrawHorizontalTabs (
  VOID
  )
{
  UINTN Index, Col;
  
  // Ensure CDAC BANGALORE title stays visible
  gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 13) / 2, 0);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
  Print (L"CDAC BANGALORE");
  
  // Ensure BIOS Setup Utility title stays visible
  gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 18) / 2, 1);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  Print (L"BIOS Setup Utility");
  
  // Clear tab row completely with blue background
  gST->ConOut->SetCursorPosition (gST->ConOut, 0, 2);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  for (Col = 0; Col < mConsoleColumns; Col++) {
    Print (L" ");
  }
  
  // Calculate proper tab positions based on screen width
  UINTN TabWidth = mConsoleColumns / 4;  // Divide screen into 4 equal parts
  
  // Draw each tab exactly once with proper spacing
  for (Index = 0; Index < mTotalMainTabs; Index++) {
    UINTN TabStartCol = Index * TabWidth + 1;
    
    gST->ConOut->SetCursorPosition (gST->ConOut, TabStartCol, 2);
    
    if (Index == mSelectedMainTab) {
      // Selected tab - light gray background with black text
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    } else {
      // Unselected tab - blue background with white text
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
    }
    
    // Print tab text with proper centering
    UINTN TextLen = StrLen(mMainTabs[Index].MenuText);
    UINTN PadLeft = (TabWidth - TextLen - 2) / 2;
    
    // Print left padding
    for (UINTN Pad = 0; Pad < PadLeft; Pad++) {
      Print (L" ");
    }
    
    // Print tab text
    Print (L"%s", mMainTabs[Index].MenuText);
    
    // Print right padding to fill tab width
    UINTN Remaining = TabWidth - PadLeft - TextLen - 1;
    for (UINTN Pad = 0; Pad < Remaining; Pad++) {
      Print (L" ");
    }
  }
}


**/
/**
  Draw the three-section layout with proper boundaries.
**/

/**
VOID
DrawThreeSectionLayout (
  VOID
  )
{
  UINTN Row, Col;
  UINTN SeparatorCol = mConsoleColumns / 2; // Dynamic separator position
  
  // Fill the main content area
  for (Row = 3; Row < mConsoleRows - 2; Row++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Row);
    
    // Print left section (submenu area) - light gray background
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    for (Col = 0; Col < SeparatorCol; Col++) {
      Print (L" ");
    }
    
    // Print vertical separator
    if (SeparatorCol < mConsoleColumns) {
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"│");
    }
    
    // Print right section (info area) - light gray background
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    for (Col = SeparatorCol + 1; Col < mConsoleColumns; Col++) {
      Print (L" ");
    }
  }
}

**/

/**
  Display current submenu in left section.
**/

/**
VOID
DisplayCurrentSubmenu (
  VOID
  )
{
  UINTN Index, Col;
  SUBMENU_ITEM *CurrentSubmenu;
  UINTN ItemCount;
  UINTN SeparatorCol = mConsoleColumns / 2;
  
  CurrentSubmenu = GetCurrentSubmenuArray (&ItemCount);
  mTotalSubItems = ItemCount;
  
  // Clear left section properly
  for (Index = 4; Index < mConsoleRows - 3; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 1, Index);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    for (Col = 1; Col < SeparatorCol - 1; Col++) {
      Print (L" ");
    }
  }
  
  // Display submenu items
  for (Index = 0; Index < ItemCount && Index < 15; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 3, Index + 5); // Fixed positioning
    
    if (Index == mSelectedSubItem) {
      // Highlight selected item
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
    } else {
      // Normal item
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    }
    
    Print (L"%s", CurrentSubmenu[Index].MenuText);
  }
}


**/
/**
  Display selected item information in right section.
**/

/**
VOID
DisplaySelectedTabInfo (
  VOID
  )
{
  SUBMENU_ITEM *CurrentSubmenu;
  UINTN ItemCount;
  CHAR16 TimeString[100];
  UINTN SeparatorCol = mConsoleColumns / 2;
  UINTN RightStart = SeparatorCol + 2;
  UINTN Index, Col;
  
  CurrentSubmenu = GetCurrentSubmenuArray (&ItemCount);
  
  // Clear right section properly
  for (Index = 4; Index < mConsoleRows - 3; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, Index);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    for (Col = RightStart; Col < mConsoleColumns - 1; Col++) {
      Print (L" ");
    }
  }
  
  // Display information header
  gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 5);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"Selected Item:");
  
  // Display based on current tab and selected submenu item
  if (mSelectedSubItem < ItemCount) {
    // Show selected item name
    gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 7);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_DARKGRAY | EFI_BACKGROUND_LIGHTGRAY);
    Print (L"%s", CurrentSubmenu[mSelectedSubItem].MenuText);
    
    // Show description
    gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 9);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    Print (L"%s", CurrentSubmenu[mSelectedSubItem].Description);
    
    // For Main tab's Change Date & Time item, show additional time info
    if (mSelectedMainTab == 0 && mSelectedSubItem == 3) {
      GetCurrentTime (TimeString);
      gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 12);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"Current: %s", TimeString);
      
      gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 14);
      Print (L"Use +/- to change values");
    }
  }
  
  // Show current tab name at bottom of right section
  gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, mConsoleRows - 5);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"Current Tab: %s", mMainTabs[mSelectedMainTab].MenuText);
}

**/

/**
  Display navigation help in bottom section.
**/

/**
VOID
DisplayNavigationHelp (
  VOID
  )
{
  UINTN Col;
  
  // Bottom blue bars with navigation help
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  
  // Fill bottom two rows with blue background
  for (UINTN Row = mConsoleRows - 2; Row < mConsoleRows; Row++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Row);
    for (Col = 0; Col < mConsoleColumns; Col++) {
      Print (L" ");
    }
  }
  
  // Display navigation instructions on bottom row
  gST->ConOut->SetCursorPosition (gST->ConOut, 2, mConsoleRows - 1);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  Print (L"←→: Select Screen  ↑↓: Select Item  Enter: Select  +/-: Change Opt  F1: Help  ESC: Exit");
}


**/
/**
  Display BIOS setup form with blue theme and horizontal layout.
**/

/**
VOID
DisplayBiosSetupForm (
  VOID
  )
{
  // Initialize console to maximum size first
  GetConsoleSize ();
  
  mSelectedMainTab = 0;
  mSelectedSubItem = 0;
  mTotalMainTabs = ARRAY_SIZE (mMainTabs);
  mInBiosSetup = TRUE;
  
  // Initialize mouse support
  InitializeMouse ();
  
  // Draw the complete BIOS setup interface step by step
  DrawBlueBackground ();        // This includes CDAC BANGALORE title
  DrawHorizontalTabs ();        // This preserves the title
  DrawThreeSectionLayout ();    // Draw the layout sections
  DisplayCurrentSubmenu ();     // Show submenu items
  DisplaySelectedTabInfo ();    // Show selected item info
  DisplayNavigationHelp ();     // Show navigation help
  
  // Draw mouse cursor
  DrawMouseCursor ();
  
  // Reset console input for clean key detection
  gST->ConIn->Reset (gST->ConIn, FALSE);
  
  // Start navigation handling
  HandleBiosSetupNavigation ();
}

**/
/**
  Handle tab selection based on current selected tab.
**/
/**
VOID
HandleTabSelection (
  VOID
  )
{
  switch (mSelectedMainTab) {
    case 3: // Save & Exit
      if (mSelectedSubItem == 0) { // Save Changes and Exit
        gST->ConOut->ClearScreen (gST->ConOut);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
        gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 35) / 2, mConsoleRows / 2);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_GREEN | EFI_BACKGROUND_BLACK);
        Print (L"✓ Settings saved. Shutting down system...");
        gBS->Stall (2000000);
        gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
        mInBiosSetup = FALSE;
      } else if (mSelectedSubItem == 1) { // Discard Changes and Exit
        gST->ConOut->ClearScreen (gST->ConOut);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
        gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 40) / 2, mConsoleRows / 2);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_RED | EFI_BACKGROUND_BLACK);
        Print (L"✗ Exiting without saving. Shutting down...");
        gBS->Stall (2000000);
        gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
        mInBiosSetup = FALSE;
      } else if (mSelectedSubItem == 2) { // Save Changes
        // Show save confirmation
        gST->ConOut->SetCursorPosition (gST->ConOut, mConsoleColumns / 2 + 2, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_GREEN | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"✓ Settings saved successfully");
        gBS->Stall (1500000);
        // Clear the message
        gST->ConOut->SetCursorPosition (gST->ConOut, mConsoleColumns / 2 + 2, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"                            ");
      } else if (mSelectedSubItem == 3) { // Discard Changes
        // Show discard confirmation
        gST->ConOut->SetCursorPosition (gST->ConOut, mConsoleColumns / 2 + 2, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_RED | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"✗ Changes discarded");
        gBS->Stall (1500000);
        // Clear the message
        gST->ConOut->SetCursorPosition (gST->ConOut, mConsoleColumns / 2 + 2, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"                   ");
      } else if (mSelectedSubItem == 4) { // Load Setup Defaults
        // Show defaults loaded confirmation
        gST->ConOut->SetCursorPosition (gST->ConOut, mConsoleColumns / 2 + 2, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"⚡ Default settings loaded");
        gBS->Stall (1500000);
        // Clear the message
        gST->ConOut->SetCursorPosition (gST->ConOut, mConsoleColumns / 2 + 2, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"                         ");
      }
      break;
      
    default:
      // For other tabs, show a brief action message
      gST->ConOut->SetCursorPosition (gST->ConOut, mConsoleColumns / 2 + 2, 15);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"⚡ Action performed...");
      gBS->Stall (800000);
      // Clear the message
      gST->ConOut->SetCursorPosition (gST->ConOut, mConsoleColumns / 2 + 2, 15);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"                     ");
      break;
  }
}

**/
/**
  Handle navigation within BIOS setup using arrow keys and mouse.
**/
/**
VOID
HandleBiosSetupNavigation (
  VOID
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  UINTN          MouseTabIndex, MouseSubIndex;
  
  while (mInBiosSetup) {
    // Update mouse state
    UpdateMouseState ();
    
    // Redraw mouse cursor
    DrawMouseCursor ();
    
    // Handle mouse input
    if (mSimplePointer != NULL) {
      MouseTabIndex = GetTabFromMousePosition (mMouseState.LastX, mMouseState.LastY);
      MouseSubIndex = GetSubItemFromMousePosition (mMouseState.LastX, mMouseState.LastY);
      
      // Handle mouse clicks on tabs
      if (mMouseState.LeftButtonReleased && MouseTabIndex != (UINTN)-1) {
        if (MouseTabIndex != mSelectedMainTab) {
          mSelectedMainTab = MouseTabIndex;
          mSelectedSubItem = 0; // Reset submenu selection
          
          // Redraw tabs and submenu
          DrawHorizontalTabs ();
          DisplayCurrentSubmenu ();
          DisplaySelectedTabInfo ();
        }
        mMouseState.LeftButtonReleased = FALSE;
      }
      
      // Handle mouse clicks on submenu items
      if (mMouseState.LeftButtonReleased && MouseSubIndex != (UINTN)-1) {
        if (MouseSubIndex != mSelectedSubItem) {
          mSelectedSubItem = MouseSubIndex;
          
          // Redraw submenu and info
          DisplayCurrentSubmenu ();
          DisplaySelectedTabInfo ();
        } else {
          // Double-click simulation - execute selection
          HandleTabSelection ();
          if (!mInBiosSetup) return;
        }
        mMouseState.LeftButtonReleased = FALSE;
      }
    }
    
    // Handle keyboard input
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
    
    if (!EFI_ERROR (Status)) {
      switch (Key.ScanCode) {
        case SCAN_LEFT:
          // Move left in main tabs
          if (mSelectedMainTab > 0) {
            mSelectedMainTab--;
          } else {
            mSelectedMainTab = mTotalMainTabs - 1; // Wrap to rightmost
          }
          mSelectedSubItem = 0; // Reset submenu selection
          
          // Redraw interface
          DrawHorizontalTabs ();
          DisplayCurrentSubmenu ();
          DisplaySelectedTabInfo ();
          break;
          
        case SCAN_RIGHT:
          // Move right in main tabs
          if (mSelectedMainTab < mTotalMainTabs - 1) {
            mSelectedMainTab++;
          } else {
            mSelectedMainTab = 0; // Wrap to leftmost
          }
          mSelectedSubItem = 0; // Reset submenu selection
          
          // Redraw interface
          DrawHorizontalTabs ();
          DisplayCurrentSubmenu ();
          DisplaySelectedTabInfo ();
          break;
          
        case SCAN_UP:
          // Move up in submenu
          if (mSelectedSubItem > 0) {
            mSelectedSubItem--;
          } else {
            mSelectedSubItem = mTotalSubItems - 1; // Wrap to bottom
          }
          
          // Update submenu and info
          DisplayCurrentSubmenu ();
          DisplaySelectedTabInfo ();
          break;
          
        case SCAN_DOWN:
          // Move down in submenu
          if (mSelectedSubItem < mTotalSubItems - 1) {
            mSelectedSubItem++;
          } else {
            mSelectedSubItem = 0; // Wrap to top
          }
          
          // Update submenu and info
          DisplayCurrentSubmenu ();
          DisplaySelectedTabInfo ();
          break;
          
        case SCAN_ESC:
          // Exit BIOS setup
          mInBiosSetup = FALSE;
          gST->ConOut->ClearScreen (gST->ConOut);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
          gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 40) / 2, mConsoleRows / 2);
          Print (L"🚪 Exiting BIOS Setup. Shutting down system...");
          gBS->Stall (2000000);
          gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
          return;
          
        default:
          // Check for ENTER key
          if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
            HandleTabSelection ();
            if (!mInBiosSetup) {
              return;
            }
          }
          break;
      }
    } else {
      // Small delay to prevent excessive CPU usage
      gBS->Stall (50000); // 50ms
    }
  }
}

**/

/**
  Monitor keyboard input for F12 key press only.
**/

/**
VOID
EFIAPI
KeyboardMonitor (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  
  // Only monitor after message is shown and not in BIOS setup
  if (!mMessageShown || mInBiosSetup) {
    return;
  }
  
  // Check if key is available
  Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
  
  if (!EFI_ERROR (Status)) {
    // Check for F12 key only
    if (Key.ScanCode == SCAN_F12) {
      DEBUG ((DEBUG_INFO, "F12 key pressed - showing BIOS setup form\n"));
      
      // Cancel timer and keyboard events
      gBS->SetTimer (mKeyMonitorEvent, TimerCancel, 0);
      gBS->SetTimer (mTimerEvent, TimerCancel, 0);
      
      // Display BIOS setup form
      DisplayBiosSetupForm ();
      
      // After exiting BIOS setup, shutdown
      gST->ConOut->ClearScreen (gST->ConOut);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
      gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 40) / 2, mConsoleRows / 2);
      Print (L"✅ BIOS Setup completed. Shutting down system...");
      gBS->Stall (2000000);
      gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
      return;
    }
  }
}

**/

/**
  Timer callback function to handle logo display timing.

  @param Event      The event that is signaled.
  @param Context    The context passed to the event.
**/

/**
VOID
EFIAPI
TimerCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  mTimerCounter++;
  
  // After 5 seconds, show the message below logo and wait for F12
  if (mTimerCounter == 50 && !mMessageShown) { // 50 * 100ms = 5 seconds
    // Clear the entire screen first
    gST->ConOut->ClearScreen (gST->ConOut);
    
    // Get console size for proper centering
    GetConsoleSize ();
    
    // Position cursor below the logo area (center of screen)
    gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 28) / 2, mConsoleRows / 2 + 5);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
    Print (L"Press F12 key for BIOS setup");
    
    mMessageShown = TRUE;
    
    // Reset console input for clean key detection
    gST->ConIn->Reset (gST->ConIn, FALSE);
    
    DEBUG ((DEBUG_INFO, "Message displayed, waiting for F12 key\n"));
  }
}

**/

/**
  Setup timing and keyboard monitoring after logo is displayed.
**/

/**
VOID
SetupLogoTiming (
  VOID
  )
{
  EFI_STATUS  Status;
  
  if (mLogoDisplayed) {
    return;
  }
  
  DEBUG ((DEBUG_INFO, "Setting up logo timing and keyboard monitoring\n"));
  
  // Create timer event for logo timing
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  TimerCallback,
                  NULL,
                  &mTimerEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    Status = gBS->SetTimer (
                    mTimerEvent,
                    TimerPeriodic,
                    1000000  // 100ms in 100ns units
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set timer for mTimerEvent: %r\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to create mTimerEvent: %r\n", Status));
  }
  
  // Create keyboard monitoring event
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  KeyboardMonitor,
                  NULL,
                  &mKeyMonitorEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    Status = gBS->SetTimer (
                    mKeyMonitorEvent,
                    TimerPeriodic,
                    500000  // 50ms in 100ns units
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set timer for mKeyMonitorEvent: %r\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to create mKeyMonitorEvent: %r\n", Status));
  }
  
  mLogoDisplayed = TRUE;
}

**/

/**
  Load a platform logo image and return its data and attributes.
**/

/**
EFI_STATUS
EFIAPI
GetImage (
  IN     EDKII_PLATFORM_LOGO_PROTOCOL        *This,
  IN OUT UINT32                              *Instance,
  OUT EFI_IMAGE_INPUT                        *Image,
  OUT EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE  *Attribute,
  OUT INTN                                   *OffsetX,
  OUT INTN                                   *OffsetY
  )
{
  UINT32      Current;
  EFI_STATUS  Status;

  if ((Instance == NULL) || (Image == NULL) ||
      (Attribute == NULL) || (OffsetX == NULL) || (OffsetY == NULL))
  {
    return EFI_INVALID_PARAMETER;
  }

  Current = *Instance;
  if (Current >= ARRAY_SIZE (mLogos)) {
    return EFI_NOT_FOUND;
  }

  (*Instance)++;
  *Attribute = mLogos[Current].Attribute;
  *OffsetX   = mLogos[Current].OffsetX;
  *OffsetY   = mLogos[Current].OffsetY;
  
  // Get image from HII database
  Status = mHiiImageEx->GetImageEx (mHiiImageEx, mHiiHandle, mLogos[Current].ImageId, Image);
  
  // After first logo is displayed, setup timing
  if (Current == 0 && !EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Logo displayed, setting up timing\n"));
    SetupLogoTiming ();
  }
  
  return Status;
}

EDKII_PLATFORM_LOGO_PROTOCOL  mPlatformLogo = {
  GetImage
};

**/

/**
  Cleanup function called when driver is unloaded.
**/

/**
EFI_STATUS
EFIAPI
LogoDriverUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
  // Close events if they exist
  if (mTimerEvent != NULL) {
    gBS->CloseEvent (mTimerEvent);
    mTimerEvent = NULL;
  }
  
  if (mKeyMonitorEvent != NULL) {
    gBS->CloseEvent (mKeyMonitorEvent);
    mKeyMonitorEvent = NULL;
  }
  
  if (mBiosSetupKeyEvent != NULL) {
    gBS->CloseEvent (mBiosSetupKeyEvent);
    mBiosSetupKeyEvent = NULL;
  }
  
  if (mMouseEvent != NULL) {
    gBS->CloseEvent (mMouseEvent);
    mMouseEvent = NULL;
  }
  
  return EFI_SUCCESS;
}

**/
/**
  Entrypoint of this module.
**/

/**
EFI_STATUS
EFIAPI
InitializeLogo (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                   Status;
  EFI_HII_PACKAGE_LIST_HEADER  *PackageList;
  EFI_HII_DATABASE_PROTOCOL    *HiiDatabase;
  EFI_HANDLE                   Handle;

  DEBUG ((DEBUG_INFO, "Logo Driver: Initializing with blue theme and horizontal tabs...\n"));

  // Initialize global variables
  mInBiosSetup = FALSE;
  mSelectedMainTab = 0;
  mSelectedSubItem = 0;
  mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;
  mTotalMainTabs = ARRAY_SIZE (mMainTabs);

  // Locate HII Database Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiDatabaseProtocolGuid,
                  NULL,
                  (VOID **)&HiiDatabase
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Database Protocol: %r\n", Status));
    return Status;
  }

  // Locate HII Image Ex Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiImageExProtocolGuid,
                  NULL,
                  (VOID **)&mHiiImageEx
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Image Ex Protocol: %r\n", Status));
    return Status;
  }

  // Retrieve HII package list from ImageHandle
  Status = gBS->OpenProtocol (
                  ImageHandle,
                  &gEfiHiiPackageListProtocolGuid,
                  (VOID **)&PackageList,
                  ImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "HII Image Package with logo not found in PE/COFF resource section\n"));
    return Status;
  }

  // Publish HII package list to HII Database
  Status = HiiDatabase->NewPackageList (
                          HiiDatabase,
                          PackageList,
                          NULL,
                          &mHiiHandle
                          );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to add package list to HII Database: %r\n", Status));
    return Status;
  }

  // Install Platform Logo Protocol
  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEdkiiPlatformLogoProtocolGuid,
                  &mPlatformLogo,
                  NULL
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to install Platform Logo Protocol: %r\n", Status));
    HiiDatabase->RemovePackageList (HiiDatabase, mHiiHandle);
    return Status;
  }

  DEBUG ((DEBUG_INFO, "Logo Driver: Initialized successfully with blue theme layout\n"));
  
  return EFI_SUCCESS;
}

**/


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




// Fixed BIOS setup utility with proper CDAC BANGALORE display and no menu duplication

/** @file
  Logo DXE Driver with F12 key BIOS setup functionality.
  Shows logo, waits for F12 key, then displays BIOS setup form with navigation.
  Enhanced with blue theme, horizontal menu tabs and three-section layout.

Copyright (c) 2016 - 2017, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent
**/
/**

#include <Uefi.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/HiiImageEx.h>
#include <Protocol/PlatformLogo.h>
#include <Protocol/HiiPackageList.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/SimpleTextOut.h>
#include <Protocol/SimplePointer.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>
#include <Library/MemoryAllocationLib.h>

typedef struct {
  EFI_IMAGE_ID                             ImageId;
  EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE    Attribute;
  INTN                                     OffsetX;
  INTN                                     OffsetY;
} LOGO_ENTRY;

// Menu item structure for horizontal tabs
typedef struct {
  CHAR16  *MenuText;
  CHAR16  *Description;
  UINT32  StartCol;    // Column where menu tab starts
  UINT32  EndCol;      // Column where menu tab ends
  UINT32  TabWidth;    // Width of the tab
} MENU_TAB;

// Submenu item structure
typedef struct {
  CHAR16  *MenuText;
  CHAR16  *Description;
  UINT32  StartRow;    // Row where menu item starts
  UINT32  StartCol;    // Column where menu item starts  
  UINT32  EndRow;      // Row where menu item ends
  UINT32  EndCol;      // Column where menu item ends
} SUBMENU_ITEM;

// Mouse state structure
typedef struct {
  BOOLEAN  LeftButtonPressed;
  BOOLEAN  LeftButtonReleased;
  BOOLEAN  WasLeftButtonPressed;
  UINT32   ClickCount;
  UINT64   LastClickTime;
  INT32    LastX;
  INT32    LastY;
  BOOLEAN  Visible;
} MOUSE_STATE;

// Menu level enumeration
typedef enum {
  MENU_LEVEL_MAIN_MENU = 0,
  MENU_LEVEL_MAIN_SUBMENU,
  MENU_LEVEL_SECURITY_SUBMENU,
  MENU_LEVEL_ADVANCED_SUBMENU,
  MENU_LEVEL_UEFI_SUBMENU
} MENU_LEVEL;

// Global variables
EFI_HII_IMAGE_EX_PROTOCOL  *mHiiImageEx;
EFI_HII_HANDLE             mHiiHandle;
EFI_EVENT                  mKeyMonitorEvent;
EFI_EVENT                  mTimerEvent;
EFI_EVENT                  mBiosSetupKeyEvent;
EFI_EVENT                  mMouseEvent;
EFI_SIMPLE_POINTER_PROTOCOL *mSimplePointer = NULL;
BOOLEAN                    mLogoDisplayed = FALSE;
BOOLEAN                    mMessageShown = FALSE;
BOOLEAN                    mInBiosSetup = FALSE;
UINTN                      mTimerCounter = 0;
UINTN                      mSelectedMainTab = 0;
UINTN                      mSelectedSubItem = 0;
UINTN                      mTotalMainTabs = 0;
UINTN                      mTotalSubItems = 0;
MENU_LEVEL                 mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;
MOUSE_STATE                mMouseState;
UINTN                      mConsoleColumns = 100;  // Increased default size
UINTN                      mConsoleRows = 40;      // Increased default size

// Main horizontal tabs (fixed spacing and positioning)
MENU_TAB mMainTabs[] = {
  { L"Main", L"System Information and Basic Settings", 2, 17, 16 },
  { L"Advanced", L"Advanced Configuration Options", 18, 35, 18 },
  { L"Security", L"Security and password Settings", 36, 53, 18 },
  { L"Save & Exit", L"Save changes and exit setup", 54, 75, 22 }
};

// Main submenu items (for Main tab)
SUBMENU_ITEM mMainSubmenu[] = {
  { L"BIOS Information", L"View BIOS version and system information", 5, 5, 5, 35 },
  { L"BIOS Eventlog", L"Display BIOS event log entries", 6, 5, 6, 35 },
  { L"Update system BIOS", L"Update system BIOS firmware", 7, 5, 7, 35 },
  { L"Change Date & Time", L"Configure system date and time", 8, 5, 8, 35 },
};

// Advanced submenu items
SUBMENU_ITEM mAdvancedSubmenu[] = {
  { L"Boot Options", L"Configure boot device priority and settings", 5, 5, 5, 35 },
  { L"Port Options", L"Configure USB, SATA, and other port settings", 6, 5, 6, 35 },
  { L"System Options", L"Configure CPU, memory, and system settings", 7, 5, 7, 35 },
  { L"Power Management", L"Configure power and thermal settings", 8, 5, 8, 35 },
};

// Security submenu items
SUBMENU_ITEM mSecuritySubmenu[] = {
  { L"Administrator Tools", L"Administrative security management tools", 5, 5, 5, 35 },
  { L"Create BIOS Password", L"Set BIOS administrator password protection", 6, 5, 6, 35 },
  { L"TPM Security", L"Configure Trusted Platform Module settings", 7, 5, 7, 35 },
  { L"Secure Boot", L"Enable or disable UEFI Secure Boot", 8, 5, 8, 35 },
};

// Save & Exit submenu items
SUBMENU_ITEM mSaveExitSubmenu[] = {
  { L"Save Changes and Exit", L"Save current settings and exit setup", 5, 5, 5, 35 },
  { L"Discard Changes and Exit", L"Exit setup without saving any changes", 6, 5, 6, 35 },
  { L"Save Changes", L"Save current settings and continue", 7, 5, 7, 35 },
  { L"Discard Changes", L"Reset to previous saved settings", 8, 5, 8, 35 },
  { L"Load Setup Defaults", L"Load factory default settings", 9, 5, 9, 35 }
};

// Function declarations
VOID DisplayBiosSetupForm (VOID);
VOID GetCurrentTime (OUT CHAR16 *TimeString);
VOID HandleBiosSetupNavigation (VOID);
VOID DrawBlueBackground (VOID);
VOID DrawHorizontalTabs (VOID);
VOID DrawThreeSectionLayout (VOID);
VOID DisplayCurrentSubmenu (VOID);
VOID DisplaySelectedTabInfo (VOID);
VOID DisplayNavigationHelp (VOID);
EFI_STATUS InitializeMouse (VOID);
VOID UpdateMouseState (VOID);
UINTN GetTabFromMousePosition (IN INT32 MouseX, IN INT32 MouseY);
UINTN GetSubItemFromMousePosition (IN INT32 MouseX, IN INT32 MouseY);
VOID HandleTabSelection (VOID);
VOID HandleSubItemSelection (VOID);
SUBMENU_ITEM* GetCurrentSubmenuArray (OUT UINTN *ItemCount);
VOID DrawMouseCursor (VOID);
VOID GetConsoleSize (VOID);

LOGO_ENTRY mLogos[] = {
  {
    IMAGE_TOKEN (IMG_LOGO),
    EdkiiPlatformLogoDisplayAttributeCenter,
    0,
    0
  }
};

**/

/**
  Get console size and set to larger size for full screen display.
**/

/**
VOID
GetConsoleSize (
  VOID
  )
{
  EFI_STATUS Status;
  UINTN MaxMode, MaxColumns, MaxRows;
  UINTN BestMode = 0;
  UINTN BestColumns = 80;
  UINTN BestRows = 25;
  
  // Try to find the largest available mode
  MaxMode = gST->ConOut->Mode->MaxMode;
  
  for (UINTN Mode = 0; Mode < MaxMode; Mode++) {
    Status = gST->ConOut->QueryMode (
                            gST->ConOut,
                            Mode,
                            &MaxColumns,
                            &MaxRows
                            );
    
    if (!EFI_ERROR (Status)) {
      // Look for a larger mode that would give us better screen coverage
      if ((MaxColumns >= BestColumns && MaxRows >= BestRows) && 
          (MaxColumns > BestColumns || MaxRows > BestRows)) {
        BestMode = Mode;
        BestColumns = MaxColumns;
        BestRows = MaxRows;
      }
    }
  }
  
  // Set the best mode we found
  if (BestMode != gST->ConOut->Mode->Mode) {
    Status = gST->ConOut->SetMode (gST->ConOut, BestMode);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_WARN, "Failed to set console mode %d: %r\n", BestMode, Status));
    }
  }
  
  // Query the actual current mode
  Status = gST->ConOut->QueryMode (
                          gST->ConOut,
                          gST->ConOut->Mode->Mode,
                          &mConsoleColumns,
                          &mConsoleRows
                          );
  
  if (EFI_ERROR (Status)) {
    // Use larger default values
    mConsoleColumns = 100;
    mConsoleRows = 40;
  }
  
  DEBUG ((DEBUG_INFO, "Console size: %dx%d\n", mConsoleColumns, mConsoleRows));
}

**/

/**
  Get current submenu array based on selected main tab.

  @param ItemCount   Output parameter for number of items in submenu.

  @retval Pointer to current submenu array.
**/

/**
SUBMENU_ITEM*
GetCurrentSubmenuArray (
  OUT UINTN *ItemCount
  )
{
  switch (mSelectedMainTab) {
    case 0: // Main
      *ItemCount = ARRAY_SIZE (mMainSubmenu);
      return mMainSubmenu;
      
    case 1: // Advanced
      *ItemCount = ARRAY_SIZE (mAdvancedSubmenu);
      return mAdvancedSubmenu;
      
    case 2: // Security
      *ItemCount = ARRAY_SIZE (mSecuritySubmenu);
      return mSecuritySubmenu;
      
    case 3: // Save & Exit
      *ItemCount = ARRAY_SIZE (mSaveExitSubmenu);
      return mSaveExitSubmenu;
      
    default:
      *ItemCount = ARRAY_SIZE (mMainSubmenu);
      return mMainSubmenu;
  }
}

**/

/**
  Initialize mouse support by locating Simple Pointer Protocol.

  @retval EFI_SUCCESS   Mouse initialized successfully.
  @retval Others        Failed to initialize mouse.
**/

/**
EFI_STATUS
InitializeMouse (
  VOID
  )
{
  EFI_STATUS Status;
  
  // Initialize mouse state
  mMouseState.LeftButtonPressed = FALSE;
  mMouseState.LeftButtonReleased = FALSE;
  mMouseState.WasLeftButtonPressed = FALSE;
  mMouseState.ClickCount = 0;
  mMouseState.LastClickTime = 0;
  mMouseState.LastX = (INT32)(mConsoleColumns / 2); // Center of screen
  mMouseState.LastY = (INT32)(mConsoleRows / 2);
  mMouseState.Visible = TRUE;
  
  // Try to locate Simple Pointer Protocol
  Status = gBS->LocateProtocol (
                  &gEfiSimplePointerProtocolGuid,
                  NULL,
                  (VOID **)&mSimplePointer
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "Simple Pointer Protocol not available: %r\n", Status));
    mSimplePointer = NULL;
    return Status;
  }
  
  // Reset mouse
  Status = mSimplePointer->Reset (mSimplePointer, TRUE);
  if (EFI_ERROR (Status)) {
    Status = mSimplePointer->Reset (mSimplePointer, FALSE);
    if (EFI_ERROR (Status)) {
      mSimplePointer = NULL;
      return Status;
    }
  }
  
  DEBUG ((DEBUG_INFO, "Mouse initialized successfully\n"));
  return EFI_SUCCESS;
}

**/

/**
  Update mouse state by reading current mouse input.
**/

/**
VOID
UpdateMouseState (
  VOID
  )
{
  EFI_STATUS                Status;
  EFI_SIMPLE_POINTER_STATE  MouseState;
  
  if (mSimplePointer == NULL) {
    return;
  }
  
  Status = mSimplePointer->GetState (mSimplePointer, &MouseState);
  if (EFI_ERROR (Status)) {
    return;
  }
  
  // Handle relative movement
  INT32 DeltaX = MouseState.RelativeMovementX / 65536;
  INT32 DeltaY = MouseState.RelativeMovementY / 65536;
  
  if (DeltaX != 0 || DeltaY != 0) {
    mMouseState.LastX += DeltaX;
    mMouseState.LastY += DeltaY;
    
    // Boundary check
    if (mMouseState.LastX < 0) mMouseState.LastX = 0;
    if (mMouseState.LastY < 0) mMouseState.LastY = 0;
    if (mMouseState.LastX >= (INT32)mConsoleColumns) mMouseState.LastX = (INT32)mConsoleColumns - 1;
    if (mMouseState.LastY >= (INT32)mConsoleRows) mMouseState.LastY = (INT32)mConsoleRows - 1;
  }
  
  // Update button state
  mMouseState.WasLeftButtonPressed = mMouseState.LeftButtonPressed;
  mMouseState.LeftButtonPressed = MouseState.LeftButton;
  
  // Detect button release (click completion)
  if (mMouseState.WasLeftButtonPressed && !mMouseState.LeftButtonPressed) {
    mMouseState.LeftButtonReleased = TRUE;
    mMouseState.ClickCount++;
  } else {
    mMouseState.LeftButtonReleased = FALSE;
  }
}

**/

/**
  Draw mouse cursor at current position.
**/
/**

VOID
DrawMouseCursor (
  VOID
  )
{
  if (!mMouseState.Visible || mSimplePointer == NULL) {
    return;
  }
  
  gST->ConOut->SetCursorPosition (gST->ConOut, mMouseState.LastX, mMouseState.LastY);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
  Print (L"▓");
}

**/

/**
  Get tab index based on mouse position.

  @param MouseX   Mouse X coordinate.
  @param MouseY   Mouse Y coordinate.

  @retval Index   Tab index, or -1 if not over any tab.
**/
/**

UINTN
GetTabFromMousePosition (
  IN INT32 MouseX,
  IN INT32 MouseY
  )
{
  UINTN Index;
  
  // Check if mouse is on tab row (row 2)
  if (MouseY != 2) {
    return (UINTN)-1;
  }
  
  for (Index = 0; Index < mTotalMainTabs; Index++) {
    if (MouseX >= mMainTabs[Index].StartCol && MouseX <= mMainTabs[Index].EndCol) {
      return Index;
    }
  }
  
  return (UINTN)-1;
}

**/
/**
  Get submenu item index based on mouse position.

  @param MouseX   Mouse X coordinate.
  @param MouseY   Mouse Y coordinate.

  @retval Index   Submenu item index, or -1 if not over any item.
**/

/**
UINTN
GetSubItemFromMousePosition (
  IN INT32 MouseX,
  IN INT32 MouseY
  )
{
  UINTN Index;
  SUBMENU_ITEM *CurrentSubmenu;
  UINTN ItemCount;
  
  CurrentSubmenu = GetCurrentSubmenuArray (&ItemCount);
  
  for (Index = 0; Index < ItemCount; Index++) {
    if (MouseY == CurrentSubmenu[Index].StartRow + 1 && // Add offset for header rows
        MouseX >= CurrentSubmenu[Index].StartCol &&
        MouseX <= CurrentSubmenu[Index].EndCol) {
      return Index;
    }
  }
  
  return (UINTN)-1;
}
**/


/**
  Get current time and format it as string.

  @param TimeString   Output buffer for formatted time string.
**/

/**
VOID
GetCurrentTime (
  OUT CHAR16  *TimeString
  )
{
  EFI_TIME    Time;
  EFI_STATUS  Status;
  
  Status = gRT->GetTime (&Time, NULL);
  if (!EFI_ERROR (Status)) {
    UnicodeSPrint (
      TimeString, 
      50 * sizeof(CHAR16),
      L"%02d/%02d/%d  %02d:%02d:%02d",
      Time.Day,
      Time.Month,
      Time.Year,
      Time.Hour,
      Time.Minute,
      Time.Second
    );
  } else {
    UnicodeSPrint (TimeString, 50 * sizeof(CHAR16), L"[Date/Time Not Available]");
  }
}

**/

/**
  Draw blue background with CDAC BANGALORE in top-left yellow corner.
**/

/**
VOID
DrawBlueBackground (
  VOID
  )
{
  UINTN Index, Col;
  
  // Get actual console size and try to maximize it
  GetConsoleSize ();
  
  // Clear screen and set blue background
  gST->ConOut->ClearScreen (gST->ConOut);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  
  // Fill entire screen with blue background
  for (Index = 0; Index < mConsoleRows; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Index);
    for (Col = 0; Col < mConsoleColumns; Col++) {
      Print (L" ");
    }
  }
  
  // Draw CDAC BANGALORE in top-left corner in YELLOW
  gST->ConOut->SetCursorPosition (gST->ConOut, 1, 0);
//  gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
   gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
 // Print (L"CDAC BANGALORE");
  Print (L"BIOS Setup Utility");
  
  // Draw BIOS Setup Utility title below center
  gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 18) / 2, 1);
 // gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
 // Print (L"BIOS Setup Utility");
   Print (L"CDAC BANGALORE");
}
**/

/**
  Draw horizontal tabs at top with proper spacing and no duplication.
**/
/**

VOID
DrawHorizontalTabs (
  VOID
  )
{
  UINTN Index, Col;
  
  // First, completely clear the tab row with blue background
  gST->ConOut->SetCursorPosition (gST->ConOut, 0, 2);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  for (Col = 0; Col < mConsoleColumns; Col++) {
    Print (L" ");
  }
  
  // Now draw each tab exactly once with proper positioning
  for (Index = 0; Index < mTotalMainTabs; Index++) {
    // Position cursor at the start of this tab
    gST->ConOut->SetCursorPosition (gST->ConOut, mMainTabs[Index].StartCol, 2);
    
    // Set colors based on selection
    if (Index == mSelectedMainTab) {
      // Selected tab - light gray background with black text
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    } else {
      // Unselected tab - blue background with white text
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
    }
    
    // Print the tab text with proper padding
    Print (L" %s ", mMainTabs[Index].MenuText);
  }
}

**/
/**
  Draw the three-section layout with proper boundaries and clear previous content.
**/

/**
VOID
DrawThreeSectionLayout (
  VOID
  )
{
  UINTN Row, Col;
  UINTN SeparatorCol = mConsoleColumns / 2; // Dynamic separator position
  
  // Clear and fill the main content area (rows 3 to second-to-last)
  for (Row = 3; Row < mConsoleRows - 2; Row++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Row);
    
    // Left section (submenu area) - light gray background
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    for (Col = 0; Col < SeparatorCol; Col++) {
      Print (L" ");
    }
    
    // Vertical separator
    if (SeparatorCol < mConsoleColumns) {
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"│");
    }
    
    // Right section (info area) - light gray background  
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    for (Col = SeparatorCol + 1; Col < mConsoleColumns; Col++) {
      Print (L" ");
    }
  }
}

**/

/**
  Display current submenu in left section without duplication.
**/
/**

VOID
DisplayCurrentSubmenu (
  VOID
  )
{
  UINTN Index, Col;
  SUBMENU_ITEM *CurrentSubmenu;
  UINTN ItemCount;
  UINTN SeparatorCol = mConsoleColumns / 2;
  
  CurrentSubmenu = GetCurrentSubmenuArray (&ItemCount);
  mTotalSubItems = ItemCount;
  
  // First, completely clear the left section area to remove any previous content
  for (Index = 4; Index < mConsoleRows - 3; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 1, Index);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    // Clear the entire left section width
    for (Col = 1; Col < SeparatorCol - 1; Col++) {
      Print (L" ");
    }
  }
  
  // Now display submenu items exactly once
  for (Index = 0; Index < ItemCount && Index < 15; Index++) {
    // Position cursor for this menu item
    gST->ConOut->SetCursorPosition (gST->ConOut, 3, Index + 5);
    
    // Set colors based on selection
    if (Index == mSelectedSubItem) {
      // Highlight selected item with blue background
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
    } else {
      // Normal item with light gray background
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    }
    
    // Print menu text once only
    Print (L"%s", CurrentSubmenu[Index].MenuText);
  }
}

**/

/**
  Display selected item information in right section.
**/

/**
VOID
DisplaySelectedTabInfo (
  VOID
  )
{
  SUBMENU_ITEM *CurrentSubmenu;
  UINTN ItemCount;
  CHAR16 TimeString[100];
  UINTN SeparatorCol = mConsoleColumns / 2;
  UINTN RightStart = SeparatorCol + 2;
  UINTN Index, Col;
  
  CurrentSubmenu = GetCurrentSubmenuArray (&ItemCount);
  
  // Clear right section completely first
  for (Index = 4; Index < mConsoleRows - 3; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, Index);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    for (Col = RightStart; Col < mConsoleColumns - 1; Col++) {
      Print (L" ");
    }
  }
  
  // Display information header
  gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 5);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"Selected Item:");
  
  // Display based on current tab and selected submenu item
  if (mSelectedSubItem < ItemCount) {
    // Show selected item name
    gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 7);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_DARKGRAY | EFI_BACKGROUND_LIGHTGRAY);
    Print (L"%s", CurrentSubmenu[mSelectedSubItem].MenuText);
    
    // Show description
    gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 9);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    Print (L"%s", CurrentSubmenu[mSelectedSubItem].Description);
    
    // For Main tab's Change Date & Time item, show additional time info
    if (mSelectedMainTab == 0 && mSelectedSubItem == 3) {
      GetCurrentTime (TimeString);
      gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 12);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"Current: %s", TimeString);
      
      gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 14);
      Print (L"Use +/- to change values");
    }
  }
  
  // Show current tab name at bottom of right section
  gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, mConsoleRows - 5);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"Current Tab: %s", mMainTabs[mSelectedMainTab].MenuText);
}

**/

/**
  Display navigation help in bottom section.
**/

/**
VOID
DisplayNavigationHelp (
  VOID
  )
{
  UINTN Col;
  
  // Bottom blue bars with navigation help
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  
  // Fill bottom two rows with blue background
  for (UINTN Row = mConsoleRows - 2; Row < mConsoleRows; Row++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Row);
    for (Col = 0; Col < mConsoleColumns; Col++) {
      Print (L" ");
    }
  }
  
  // Display navigation instructions on bottom row
  gST->ConOut->SetCursorPosition (gST->ConOut, 2, mConsoleRows - 1);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  Print (L"←→: Select Screen  ↑↓: Select Item  Enter: Select  ESC: Exit");
}

**/

/**
  Display BIOS setup form with blue theme and horizontal layout.
**/

/**
VOID
DisplayBiosSetupForm (
  VOID
  )
{
  mSelectedMainTab = 0;
  mSelectedSubItem = 0;
  mTotalMainTabs = ARRAY_SIZE (mMainTabs);
  mInBiosSetup = TRUE;
  
  // Initialize mouse support
  InitializeMouse ();
  
  // Draw the complete BIOS setup interface step by step
  DrawBlueBackground ();       // Includes CDAC BANGALORE in top-left yellow
  DrawHorizontalTabs ();       // Draw tabs without duplication
  DrawThreeSectionLayout ();   // Clear and draw the three sections
  DisplayCurrentSubmenu ();    // Display submenu items once only
  DisplaySelectedTabInfo ();   // Display item info in right panel
  DisplayNavigationHelp ();    // Bottom navigation help
  
  // Draw mouse cursor
  DrawMouseCursor ();
  
  // Reset console input for clean key detection
  gST->ConIn->Reset (gST->ConIn, FALSE);
  
  // Start navigation handling
  HandleBiosSetupNavigation ();
}

**/
/**
  Handle tab selection based on current selected tab.
**/
/**
VOID
HandleTabSelection (
  VOID
  )
{
  switch (mSelectedMainTab) {
    case 3: // Save & Exit
      if (mSelectedSubItem == 0) { // Save Changes and Exit
        gST->ConOut->ClearScreen (gST->ConOut);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
        gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 35) / 2, mConsoleRows / 2);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_GREEN | EFI_BACKGROUND_BLACK);
        Print (L"✓ Settings saved. Shutting down system...");
        gBS->Stall (2000000);
        gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
        mInBiosSetup = FALSE;
      } else if (mSelectedSubItem == 1) { // Discard Changes and Exit
        gST->ConOut->ClearScreen (gST->ConOut);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
        gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 40) / 2, mConsoleRows / 2);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_RED | EFI_BACKGROUND_BLACK);
        Print (L"✗ Exiting without saving. Shutting down...");
        gBS->Stall (2000000);
        gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
        mInBiosSetup = FALSE;
      } else if (mSelectedSubItem == 2) { // Save Changes
        // Show save confirmation
        gST->ConOut->SetCursorPosition (gST->ConOut, mConsoleColumns / 2 + 2, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_GREEN | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"✓ Settings saved successfully");
        gBS->Stall (1500000);
        // Clear the message
        gST->ConOut->SetCursorPosition (gST->ConOut, mConsoleColumns / 2 + 2, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"                            ");
      } else if (mSelectedSubItem == 3) { // Discard Changes
        // Show discard confirmation
        gST->ConOut->SetCursorPosition (gST->ConOut, mConsoleColumns / 2 + 2, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_RED | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"✗ Changes discarded");
        gBS->Stall (1500000);
        // Clear the message
        gST->ConOut->SetCursorPosition (gST->ConOut, mConsoleColumns / 2 + 2, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"                   ");
      } else if (mSelectedSubItem == 4) { // Load Setup Defaults
        // Show defaults loaded confirmation
        gST->ConOut->SetCursorPosition (gST->ConOut, mConsoleColumns / 2 + 2, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"⚡ Default settings loaded");
        gBS->Stall (1500000);
        // Clear the message
        gST->ConOut->SetCursorPosition (gST->ConOut, mConsoleColumns / 2 + 2, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"                         ");
      }
      break;
      
    default:
      // For other tabs, show a brief action message
      gST->ConOut->SetCursorPosition (gST->ConOut, mConsoleColumns / 2 + 2, 15);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"⚡ Action performed...");
      gBS->Stall (800000);
      // Clear the message
      gST->ConOut->SetCursorPosition (gST->ConOut, mConsoleColumns / 2 + 2, 15);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"                     ");
      break;
  }
}

**/

/**
  Handle navigation within BIOS setup using arrow keys and mouse.
**/
/**

VOID
HandleBiosSetupNavigation (
  VOID
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  UINTN          MouseTabIndex, MouseSubIndex;
  
  while (mInBiosSetup) {
    // Update mouse state
    UpdateMouseState ();
    
    // Redraw mouse cursor
    DrawMouseCursor ();
    
    // Handle mouse input
    if (mSimplePointer != NULL) {
      MouseTabIndex = GetTabFromMousePosition (mMouseState.LastX, mMouseState.LastY);
      MouseSubIndex = GetSubItemFromMousePosition (mMouseState.LastX, mMouseState.LastY);
      
      // Handle mouse clicks on tabs
      if (mMouseState.LeftButtonReleased && MouseTabIndex != (UINTN)-1) {
        if (MouseTabIndex != mSelectedMainTab) {
          mSelectedMainTab = MouseTabIndex;
          mSelectedSubItem = 0; // Reset submenu selection
          
          // Redraw interface completely to avoid duplication
          DrawBlueBackground ();
          DrawHorizontalTabs ();
          DrawThreeSectionLayout ();
          DisplayCurrentSubmenu ();
          DisplaySelectedTabInfo ();
          DisplayNavigationHelp ();
        }
        mMouseState.LeftButtonReleased = FALSE;
      }
      
      // Handle mouse clicks on submenu items
      if (mMouseState.LeftButtonReleased && MouseSubIndex != (UINTN)-1) {
        if (MouseSubIndex != mSelectedSubItem) {
          mSelectedSubItem = MouseSubIndex;
          
          // Redraw submenu and info only
          DisplayCurrentSubmenu ();
          DisplaySelectedTabInfo ();
        } else {
          // Double-click simulation - execute selection
          HandleTabSelection ();
          if (!mInBiosSetup) return;
        }
        mMouseState.LeftButtonReleased = FALSE;
      }
    }
    
    // Handle keyboard input
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
    
    if (!EFI_ERROR (Status)) {
      switch (Key.ScanCode) {
        case SCAN_LEFT:
          // Move left in main tabs
          if (mSelectedMainTab > 0) {
            mSelectedMainTab--;
          } else {
            mSelectedMainTab = mTotalMainTabs - 1; // Wrap to rightmost
          }
          mSelectedSubItem = 0; // Reset submenu selection
          
          // Redraw interface completely to avoid duplication
          DrawBlueBackground ();
          DrawHorizontalTabs ();
          DrawThreeSectionLayout ();
          DisplayCurrentSubmenu ();
          DisplaySelectedTabInfo ();
          DisplayNavigationHelp ();
          break;
          
        case SCAN_RIGHT:
          // Move right in main tabs
          if (mSelectedMainTab < mTotalMainTabs - 1) {
            mSelectedMainTab++;
          } else {
            mSelectedMainTab = 0; // Wrap to leftmost
          }
          mSelectedSubItem = 0; // Reset submenu selection
          
          // Redraw interface completely to avoid duplication
          DrawBlueBackground ();
          DrawHorizontalTabs ();
          DrawThreeSectionLayout ();
          DisplayCurrentSubmenu ();
          DisplaySelectedTabInfo ();
          DisplayNavigationHelp ();
          break;
          
        case SCAN_UP:
          // Move up in submenu
          if (mSelectedSubItem > 0) {
            mSelectedSubItem--;
          } else {
            mSelectedSubItem = mTotalSubItems - 1; // Wrap to bottom
          }
          
          // Update submenu and info only
          DisplayCurrentSubmenu ();
          DisplaySelectedTabInfo ();
          break;
          
        case SCAN_DOWN:
          // Move down in submenu
          if (mSelectedSubItem < mTotalSubItems - 1) {
            mSelectedSubItem++;
          } else {
            mSelectedSubItem = 0; // Wrap to top
          }
          
          // Update submenu and info only
          DisplayCurrentSubmenu ();
          DisplaySelectedTabInfo ();
          break;
          
        case SCAN_ESC:
          // Exit BIOS setup
          mInBiosSetup = FALSE;
          gST->ConOut->ClearScreen (gST->ConOut);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
          gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 40) / 2, mConsoleRows / 2);
          Print (L"🚪 Exiting BIOS Setup. Shutting down system...");
          gBS->Stall (2000000);
          gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
          return;
          
        default:
          // Check for ENTER key
          if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
            HandleTabSelection ();
            if (!mInBiosSetup) {
              return;
            }
          }
          break;
      }
    } else {
      // Small delay to prevent excessive CPU usage
      gBS->Stall (50000); // 50ms
    }
  }
}

**/

/**
  Monitor keyboard input for F12 key press only.
**/

/**
VOID
EFIAPI
KeyboardMonitor (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  
  // Only monitor after message is shown and not in BIOS setup
  if (!mMessageShown || mInBiosSetup) {
    return;
  }
  
  // Check if key is available
  Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
  
  if (!EFI_ERROR (Status)) {
    // Check for F12 key only
    if (Key.ScanCode == SCAN_F12) {
      DEBUG ((DEBUG_INFO, "F12 key pressed - showing BIOS setup form\n"));
      
      // Cancel timer and keyboard events
      gBS->SetTimer (mKeyMonitorEvent, TimerCancel, 0);
      gBS->SetTimer (mTimerEvent, TimerCancel, 0);
      
      // Display BIOS setup form
      DisplayBiosSetupForm ();
      
      // After exiting BIOS setup, shutdown
      gST->ConOut->ClearScreen (gST->ConOut);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
      gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 40) / 2, mConsoleRows / 2);
      Print (L"✅ BIOS Setup completed. Shutting down system...");
      gBS->Stall (2000000);
      gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
      return;
    }
  }
}

**/

/**
  Timer callback function to handle logo display timing.

  @param Event      The event that is signaled.
  @param Context    The context passed to the event.
**/

/**
VOID
EFIAPI
TimerCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  mTimerCounter++;
  
  // After 5 seconds, show the message below logo and wait for F12
  if (mTimerCounter == 50 && !mMessageShown) { // 50 * 100ms = 5 seconds
    // Clear the entire screen first
    gST->ConOut->ClearScreen (gST->ConOut);
    
    // Get console size for proper centering
    GetConsoleSize ();
    
    // Position cursor below the logo area (center of screen)
    gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 28) / 2, mConsoleRows / 2 + 5);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
    Print (L"Press F12 key for BIOS setup");
    
    mMessageShown = TRUE;
    
    // Reset console input for clean key detection
    gST->ConIn->Reset (gST->ConIn, FALSE);
    
    DEBUG ((DEBUG_INFO, "Message displayed, waiting for F12 key\n"));
  }
}

**/

/**
  Setup timing and keyboard monitoring after logo is displayed.
**/
/**

VOID
SetupLogoTiming (
  VOID
  )
{
  EFI_STATUS  Status;
  
  if (mLogoDisplayed) {
    return;
  }
  
  DEBUG ((DEBUG_INFO, "Setting up logo timing and keyboard monitoring\n"));
  
  // Create timer event for logo timing
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  TimerCallback,
                  NULL,
                  &mTimerEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    Status = gBS->SetTimer (
                    mTimerEvent,
                    TimerPeriodic,
                    1000000  // 100ms in 100ns units
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set timer for mTimerEvent: %r\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to create mTimerEvent: %r\n", Status));
  }
  
  // Create keyboard monitoring event
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  KeyboardMonitor,
                  NULL,
                  &mKeyMonitorEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    Status = gBS->SetTimer (
                    mKeyMonitorEvent,
                    TimerPeriodic,
                    500000  // 50ms in 100ns units
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set timer for mKeyMonitorEvent: %r\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to create mKeyMonitorEvent: %r\n", Status));
  }
  
  mLogoDisplayed = TRUE;
}
**/

/**
  Load a platform logo image and return its data and attributes.
**/

/**
EFI_STATUS
EFIAPI
GetImage (
  IN     EDKII_PLATFORM_LOGO_PROTOCOL        *This,
  IN OUT UINT32                              *Instance,
  OUT EFI_IMAGE_INPUT                        *Image,
  OUT EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE  *Attribute,
  OUT INTN                                   *OffsetX,
  OUT INTN                                   *OffsetY
  )
{
  UINT32      Current;
  EFI_STATUS  Status;

  if ((Instance == NULL) || (Image == NULL) ||
      (Attribute == NULL) || (OffsetX == NULL) || (OffsetY == NULL))
  {
    return EFI_INVALID_PARAMETER;
  }

  Current = *Instance;
  if (Current >= ARRAY_SIZE (mLogos)) {
    return EFI_NOT_FOUND;
  }

  (*Instance)++;
  *Attribute = mLogos[Current].Attribute;
  *OffsetX   = mLogos[Current].OffsetX;
  *OffsetY   = mLogos[Current].OffsetY;
  
  // Get image from HII database
  Status = mHiiImageEx->GetImageEx (mHiiImageEx, mHiiHandle, mLogos[Current].ImageId, Image);
  
  // After first logo is displayed, setup timing
  if (Current == 0 && !EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Logo displayed, setting up timing\n"));
    SetupLogoTiming ();
  }
  
  return Status;
}

EDKII_PLATFORM_LOGO_PROTOCOL  mPlatformLogo = {
  GetImage
};
**/

/**
  Cleanup function called when driver is unloaded.
**/
/**
EFI_STATUS
EFIAPI
LogoDriverUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
  // Close events if they exist
  if (mTimerEvent != NULL) {
    gBS->CloseEvent (mTimerEvent);
    mTimerEvent = NULL;
  }
  
  if (mKeyMonitorEvent != NULL) {
    gBS->CloseEvent (mKeyMonitorEvent);
    mKeyMonitorEvent = NULL;
  }
  
  if (mBiosSetupKeyEvent != NULL) {
    gBS->CloseEvent (mBiosSetupKeyEvent);
    mBiosSetupKeyEvent = NULL;
  }
  
  if (mMouseEvent != NULL) {
    gBS->CloseEvent (mMouseEvent);
    mMouseEvent = NULL;
  }
  
  return EFI_SUCCESS;
}

**/
/**
  Entrypoint of this module.
**/
/**

EFI_STATUS
EFIAPI
InitializeLogo (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                   Status;
  EFI_HII_PACKAGE_LIST_HEADER  *PackageList;
  EFI_HII_DATABASE_PROTOCOL    *HiiDatabase;
  EFI_HANDLE                   Handle;

  DEBUG ((DEBUG_INFO, "Logo Driver: Initializing with blue theme and horizontal tabs...\n"));

  // Initialize global variables
  mInBiosSetup = FALSE;
  mSelectedMainTab = 0;
  mSelectedSubItem = 0;
  mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;
  mTotalMainTabs = ARRAY_SIZE (mMainTabs);

  // Locate HII Database Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiDatabaseProtocolGuid,
                  NULL,
                  (VOID **)&HiiDatabase
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Database Protocol: %r\n", Status));
    return Status;
  }

  // Locate HII Image Ex Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiImageExProtocolGuid,
                  NULL,
                  (VOID **)&mHiiImageEx
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Image Ex Protocol: %r\n", Status));
    return Status;
  }

  // Retrieve HII package list from ImageHandle
  Status = gBS->OpenProtocol (
                  ImageHandle,
                  &gEfiHiiPackageListProtocolGuid,
                  (VOID **)&PackageList,
                  ImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "HII Image Package with logo not found in PE/COFF resource section\n"));
    return Status;
  }

  // Publish HII package list to HII Database
  Status = HiiDatabase->NewPackageList (
                          HiiDatabase,
                          PackageList,
                          NULL,
                          &mHiiHandle
                          );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to add package list to HII Database: %r\n", Status));
    return Status;
  }

  // Install Platform Logo Protocol
  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEdkiiPlatformLogoProtocolGuid,
                  &mPlatformLogo,
                  NULL
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to install Platform Logo Protocol: %r\n", Status));
    HiiDatabase->RemovePackageList (HiiDatabase, mHiiHandle);
    return Status;
  }

  DEBUG ((DEBUG_INFO, "Logo Driver: Initialized successfully with blue theme layout\n"));
  
  return EFI_SUCCESS;
}

**/



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




// Fixed BIOS setup utility with static form and 3/4-1/4 layout but menu shows 2 times. this is not good.

/** @file
  Logo DXE Driver with F12 key BIOS setup functionality.
  Shows logo, waits for F12 key, then displays BIOS setup form with navigation.
  Enhanced with blue theme, horizontal menu tabs and static three-section layout.

Copyright (c) 2016 - 2017, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent
**/

/**
#include <Uefi.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/HiiImageEx.h>
#include <Protocol/PlatformLogo.h>
#include <Protocol/HiiPackageList.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/SimpleTextOut.h>
#include <Protocol/SimplePointer.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>
#include <Library/MemoryAllocationLib.h>

typedef struct {
  EFI_IMAGE_ID                             ImageId;
  EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE    Attribute;
  INTN                                     OffsetX;
  INTN                                     OffsetY;
} LOGO_ENTRY;

// Menu item structure for horizontal tabs
typedef struct {
  CHAR16  *MenuText;
  CHAR16  *Description;
  UINT32  StartCol;    // Column where menu tab starts
  UINT32  EndCol;      // Column where menu tab ends
  UINT32  TabWidth;    // Width of the tab
} MENU_TAB;

// Submenu item structure
typedef struct {
  CHAR16  *MenuText;
  CHAR16  *Description;
  UINT32  StartRow;    // Row where menu item starts
  UINT32  StartCol;    // Column where menu item starts  
  UINT32  EndRow;      // Row where menu item ends
  UINT32  EndCol;      // Column where menu item ends
} SUBMENU_ITEM;

// Mouse state structure
typedef struct {
  BOOLEAN  LeftButtonPressed;
  BOOLEAN  LeftButtonReleased;
  BOOLEAN  WasLeftButtonPressed;
  UINT32   ClickCount;
  UINT64   LastClickTime;
  INT32    LastX;
  INT32    LastY;
  BOOLEAN  Visible;
} MOUSE_STATE;

// Menu level enumeration
typedef enum {
  MENU_LEVEL_MAIN_MENU = 0,
  MENU_LEVEL_MAIN_SUBMENU,
  MENU_LEVEL_SECURITY_SUBMENU,
  MENU_LEVEL_ADVANCED_SUBMENU,
  MENU_LEVEL_UEFI_SUBMENU
} MENU_LEVEL;

// Global variables
EFI_HII_IMAGE_EX_PROTOCOL  *mHiiImageEx;
EFI_HII_HANDLE             mHiiHandle;
EFI_EVENT                  mKeyMonitorEvent;
EFI_EVENT                  mTimerEvent;
EFI_EVENT                  mBiosSetupKeyEvent;
EFI_EVENT                  mMouseEvent;
EFI_SIMPLE_POINTER_PROTOCOL *mSimplePointer = NULL;
BOOLEAN                    mLogoDisplayed = FALSE;
BOOLEAN                    mMessageShown = FALSE;
BOOLEAN                    mInBiosSetup = FALSE;
UINTN                      mTimerCounter = 0;
UINTN                      mSelectedMainTab = 0;
UINTN                      mSelectedSubItem = 0;
UINTN                      mTotalMainTabs = 0;
UINTN                      mTotalSubItems = 0;
MENU_LEVEL                 mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;
MOUSE_STATE                mMouseState;
UINTN                      mConsoleColumns = 100;  // Increased default size
UINTN                      mConsoleRows = 40;      // Increased default size

// Main horizontal tabs (fixed spacing and positioning)
MENU_TAB mMainTabs[] = {
  { L"Main", L"System Information and Basic Settings", 2, 17, 16 },
  { L"Advanced", L"Advanced Configuration Options", 18, 35, 18 },
  { L"Security", L"Security and password Settings", 36, 53, 18 },
  { L"Save & Exit", L"Save changes and exit setup", 54, 75, 22 }
};

// Main submenu items (for Main tab)
SUBMENU_ITEM mMainSubmenu[] = {
  { L"BIOS Information", L"View BIOS version and system information", 5, 5, 5, 35 },
  { L"BIOS Eventlog", L"Display BIOS event log entries", 6, 5, 6, 35 },
  { L"Update system BIOS", L"Update system BIOS firmware", 7, 5, 7, 35 },
  { L"Change Date & Time", L"Configure system date and time", 8, 5, 8, 35 },
};

// Advanced submenu items
SUBMENU_ITEM mAdvancedSubmenu[] = {
  { L"Boot Options", L"Configure boot device priority and settings", 5, 5, 5, 35 },
  { L"Port Options", L"Configure USB, SATA, and other port settings", 6, 5, 6, 35 },
  { L"System Options", L"Configure CPU, memory, and system settings", 7, 5, 7, 35 },
  { L"Power Management", L"Configure power and thermal settings", 8, 5, 8, 35 },
};

// Security submenu items
SUBMENU_ITEM mSecuritySubmenu[] = {
  { L"Administrator Tools", L"Administrative security management tools", 5, 5, 5, 35 },
  { L"Create BIOS Password", L"Set BIOS administrator password protection", 6, 5, 6, 35 },
  { L"TPM Security", L"Configure Trusted Platform Module settings", 7, 5, 7, 35 },
  { L"Secure Boot", L"Enable or disable UEFI Secure Boot", 8, 5, 8, 35 },
};

// Save & Exit submenu items
SUBMENU_ITEM mSaveExitSubmenu[] = {
  { L"Save Changes and Exit", L"Save current settings and exit setup", 5, 5, 5, 35 },
  { L"Discard Changes and Exit", L"Exit setup without saving any changes", 6, 5, 6, 35 },
  { L"Save Changes", L"Save current settings and continue", 7, 5, 7, 35 },
  { L"Discard Changes", L"Reset to previous saved settings", 8, 5, 8, 35 },
  { L"Load Setup Defaults", L"Load factory default settings", 9, 5, 9, 35 }
};

// Function declarations
VOID DisplayBiosSetupForm (VOID);
VOID GetCurrentTime (OUT CHAR16 *TimeString);
VOID HandleBiosSetupNavigation (VOID);
VOID DrawBlueBackground (VOID);
VOID DrawHorizontalTabs (VOID);
VOID DrawStaticLayout (VOID);  // Changed from DrawThreeSectionLayout
VOID DisplayCurrentSubmenu (VOID);
VOID DisplaySelectedTabInfo (VOID);
VOID DisplayNavigationHelp (VOID);
EFI_STATUS InitializeMouse (VOID);
VOID UpdateMouseState (VOID);
UINTN GetTabFromMousePosition (IN INT32 MouseX, IN INT32 MouseY);
UINTN GetSubItemFromMousePosition (IN INT32 MouseX, IN INT32 MouseY);
VOID HandleTabSelection (VOID);
VOID HandleSubItemSelection (VOID);
SUBMENU_ITEM* GetCurrentSubmenuArray (OUT UINTN *ItemCount);
VOID DrawMouseCursor (VOID);
VOID GetConsoleSize (VOID);

LOGO_ENTRY mLogos[] = {
  {
    IMAGE_TOKEN (IMG_LOGO),
    EdkiiPlatformLogoDisplayAttributeCenter,
    0,
    0
  }
};

**/

/**
  Get console size and set to larger size for full screen display.
**/

/**
VOID
GetConsoleSize (
  VOID
  )
{
  EFI_STATUS Status;
  UINTN MaxMode, MaxColumns, MaxRows;
  UINTN BestMode = 0;
  UINTN BestColumns = 80;
  UINTN BestRows = 25;
  
  // Try to find the largest available mode
  MaxMode = gST->ConOut->Mode->MaxMode;
  
  for (UINTN Mode = 0; Mode < MaxMode; Mode++) {
    Status = gST->ConOut->QueryMode (
                            gST->ConOut,
                            Mode,
                            &MaxColumns,
                            &MaxRows
                            );
    
    if (!EFI_ERROR (Status)) {
      // Look for a larger mode that would give us better screen coverage
      if ((MaxColumns >= BestColumns && MaxRows >= BestRows) && 
          (MaxColumns > BestColumns || MaxRows > BestRows)) {
        BestMode = Mode;
        BestColumns = MaxColumns;
        BestRows = MaxRows;
      }
    }
  }
  
  // Set the best mode we found
  if (BestMode != gST->ConOut->Mode->Mode) {
    Status = gST->ConOut->SetMode (gST->ConOut, BestMode);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_WARN, "Failed to set console mode %d: %r\n", BestMode, Status));
    }
  }
  
  // Query the actual current mode
  Status = gST->ConOut->QueryMode (
                          gST->ConOut,
                          gST->ConOut->Mode->Mode,
                          &mConsoleColumns,
                          &mConsoleRows
                          );
  
  if (EFI_ERROR (Status)) {
    // Use larger default values
    mConsoleColumns = 100;
    mConsoleRows = 40;
  }
  
  DEBUG ((DEBUG_INFO, "Console size: %dx%d\n", mConsoleColumns, mConsoleRows));
}

**/

/**
  Get current submenu array based on selected main tab.

  @param ItemCount   Output parameter for number of items in submenu.

  @retval Pointer to current submenu array.
**/

/**
SUBMENU_ITEM*
GetCurrentSubmenuArray (
  OUT UINTN *ItemCount
  )
{
  switch (mSelectedMainTab) {
    case 0: // Main
      *ItemCount = ARRAY_SIZE (mMainSubmenu);
      return mMainSubmenu;
      
    case 1: // Advanced
      *ItemCount = ARRAY_SIZE (mAdvancedSubmenu);
      return mAdvancedSubmenu;
      
    case 2: // Security
      *ItemCount = ARRAY_SIZE (mSecuritySubmenu);
      return mSecuritySubmenu;
      
    case 3: // Save & Exit
      *ItemCount = ARRAY_SIZE (mSaveExitSubmenu);
      return mSaveExitSubmenu;
      
    default:
      *ItemCount = ARRAY_SIZE (mMainSubmenu);
      return mMainSubmenu;
  }
}

**/

/**
  Initialize mouse support by locating Simple Pointer Protocol.

  @retval EFI_SUCCESS   Mouse initialized successfully.
  @retval Others        Failed to initialize mouse.
**/

/**
EFI_STATUS
InitializeMouse (
  VOID
  )
{
  EFI_STATUS Status;
  
  // Initialize mouse state
  mMouseState.LeftButtonPressed = FALSE;
  mMouseState.LeftButtonReleased = FALSE;
  mMouseState.WasLeftButtonPressed = FALSE;
  mMouseState.ClickCount = 0;
  mMouseState.LastClickTime = 0;
  mMouseState.LastX = (INT32)(mConsoleColumns / 2); // Center of screen
  mMouseState.LastY = (INT32)(mConsoleRows / 2);
  mMouseState.Visible = TRUE;
  
  // Try to locate Simple Pointer Protocol
  Status = gBS->LocateProtocol (
                  &gEfiSimplePointerProtocolGuid,
                  NULL,
                  (VOID **)&mSimplePointer
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "Simple Pointer Protocol not available: %r\n", Status));
    mSimplePointer = NULL;
    return Status;
  }
  
  // Reset mouse
  Status = mSimplePointer->Reset (mSimplePointer, TRUE);
  if (EFI_ERROR (Status)) {
    Status = mSimplePointer->Reset (mSimplePointer, FALSE);
    if (EFI_ERROR (Status)) {
      mSimplePointer = NULL;
      return Status;
    }
  }
  
  DEBUG ((DEBUG_INFO, "Mouse initialized successfully\n"));
  return EFI_SUCCESS;
}

**/

/**
  Update mouse state by reading current mouse input.
**/

/**
VOID
UpdateMouseState (
  VOID
  )
{
  EFI_STATUS                Status;
  EFI_SIMPLE_POINTER_STATE  MouseState;
  
  if (mSimplePointer == NULL) {
    return;
  }
  
  Status = mSimplePointer->GetState (mSimplePointer, &MouseState);
  if (EFI_ERROR (Status)) {
    return;
  }
  
  // Handle relative movement
  INT32 DeltaX = MouseState.RelativeMovementX / 65536;
  INT32 DeltaY = MouseState.RelativeMovementY / 65536;
  
  if (DeltaX != 0 || DeltaY != 0) {
    mMouseState.LastX += DeltaX;
    mMouseState.LastY += DeltaY;
    
    // Boundary check
    if (mMouseState.LastX < 0) mMouseState.LastX = 0;
    if (mMouseState.LastY < 0) mMouseState.LastY = 0;
    if (mMouseState.LastX >= (INT32)mConsoleColumns) mMouseState.LastX = (INT32)mConsoleColumns - 1;
    if (mMouseState.LastY >= (INT32)mConsoleRows) mMouseState.LastY = (INT32)mConsoleRows - 1;
  }
  
  // Update button state
  mMouseState.WasLeftButtonPressed = mMouseState.LeftButtonPressed;
  mMouseState.LeftButtonPressed = MouseState.LeftButton;
  
  // Detect button release (click completion)
  if (mMouseState.WasLeftButtonPressed && !mMouseState.LeftButtonPressed) {
    mMouseState.LeftButtonReleased = TRUE;
    mMouseState.ClickCount++;
  } else {
    mMouseState.LeftButtonReleased = FALSE;
  }
}

**/

/**
  Draw mouse cursor at current position.
**/
/**

VOID
DrawMouseCursor (
  VOID
  )
{
  if (!mMouseState.Visible || mSimplePointer == NULL) {
    return;
  }
  
  gST->ConOut->SetCursorPosition (gST->ConOut, mMouseState.LastX, mMouseState.LastY);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
  Print (L"▓");
}

**/

/**
  Get tab index based on mouse position.

  @param MouseX   Mouse X coordinate.
  @param MouseY   Mouse Y coordinate.

  @retval Index   Tab index, or -1 if not over any tab.
**/

/**
UINTN
GetTabFromMousePosition (
  IN INT32 MouseX,
  IN INT32 MouseY
  )
{
  UINTN Index;
  
  // Check if mouse is on tab row (row 2)
  if (MouseY != 2) {
    return (UINTN)-1;
  }
  
  for (Index = 0; Index < mTotalMainTabs; Index++) {
    if (MouseX >= mMainTabs[Index].StartCol && MouseX <= mMainTabs[Index].EndCol) {
      return Index;
    }
  }
  
  return (UINTN)-1;
}
**/

/**
  Get submenu item index based on mouse position.

  @param MouseX   Mouse X coordinate.
  @param MouseY   Mouse Y coordinate.

  @retval Index   Submenu item index, or -1 if not over any item.
**/

/**
UINTN
GetSubItemFromMousePosition (
  IN INT32 MouseX,
  IN INT32 MouseY
  )
{
  UINTN Index;
  SUBMENU_ITEM *CurrentSubmenu;
  UINTN ItemCount;
  
  CurrentSubmenu = GetCurrentSubmenuArray (&ItemCount);
  
  for (Index = 0; Index < ItemCount; Index++) {
    if (MouseY == CurrentSubmenu[Index].StartRow + 1 && // Add offset for header rows
        MouseX >= CurrentSubmenu[Index].StartCol &&
        MouseX <= CurrentSubmenu[Index].EndCol) {
      return Index;
    }
  }
  
  return (UINTN)-1;
}


**/
/**
  Get current time and format it as string.

  @param TimeString   Output buffer for formatted time string.
**/

/**
VOID
GetCurrentTime (
  OUT CHAR16  *TimeString
  )
{
  EFI_TIME    Time;
  EFI_STATUS  Status;
  
  Status = gRT->GetTime (&Time, NULL);
  if (!EFI_ERROR (Status)) {
    UnicodeSPrint (
      TimeString, 
      50 * sizeof(CHAR16),
      L"%02d/%02d/%d  %02d:%02d:%02d",
      Time.Day,
      Time.Month,
      Time.Year,
      Time.Hour,
      Time.Minute,
      Time.Second
    );
  } else {
    UnicodeSPrint (TimeString, 50 * sizeof(CHAR16), L"[Date/Time Not Available]");
  }
}

**/

/**
  Draw blue background with BIOS Setup Utility in top-left corner.
**/
/**

VOID
DrawBlueBackground (
  VOID
  )
{
  UINTN Index, Col;
  
  // Get actual console size and try to maximize it
  GetConsoleSize ();
  
  // Clear screen and set blue background
  gST->ConOut->ClearScreen (gST->ConOut);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  
  // Fill entire screen with blue background
  for (Index = 0; Index < mConsoleRows; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Index);
    for (Col = 0; Col < mConsoleColumns; Col++) {
      Print (L" ");
    }
  }
  
  // Draw BIOS Setup Utility in top-left corner in WHITE
  gST->ConOut->SetCursorPosition (gST->ConOut, 1, 0);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  Print (L"BIOS Setup Utility");
  
  // Draw CDAC BANGALORE title below center
  gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 14) / 2, 1);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
  Print (L"CDAC BANGALORE");
}

**/
/**
  Draw horizontal tabs at top without duplication.
**/

/**
VOID
DrawHorizontalTabs (
  VOID
  )
{
  UINTN Index, Col;
  
  // First, completely clear the tab row with blue background
  gST->ConOut->SetCursorPosition (gST->ConOut, 0, 2);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  for (Col = 0; Col < mConsoleColumns; Col++) {
    Print (L" ");
  }
  
  // Now draw each tab exactly once with proper positioning
  for (Index = 0; Index < mTotalMainTabs; Index++) {
    // Position cursor at the start of this tab
    gST->ConOut->SetCursorPosition (gST->ConOut, mMainTabs[Index].StartCol, 2);
    
    // Set colors based on selection
    if (Index == mSelectedMainTab) {
      // Selected tab - light gray background with black text
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    } else {
      // Unselected tab - blue background with white text
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
    }
    
    // Print the tab text with proper padding
    Print (L" %s ", mMainTabs[Index].MenuText);
  }
}

**/
/**
  Draw the static layout with 3/4 left and 1/4 right sections.
**/
/**

VOID
DrawStaticLayout (
  VOID
  )
{
  UINTN Row, Col;
  // Changed: Now left section is 3/4 of screen, right is 1/4
  UINTN SeparatorCol = (mConsoleColumns * 3) / 4; // 3/4 position for separator
  
  // Clear and fill the main content area (rows 3 to second-to-last)
  for (Row = 3; Row < mConsoleRows - 2; Row++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Row);
    
    // Left section (submenu area) - light gray background - 3/4 of screen
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    for (Col = 0; Col < SeparatorCol; Col++) {
      Print (L" ");
    }
    
    // Vertical separator
    if (SeparatorCol < mConsoleColumns) {
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"│");
    }
    
    // Right section (info area) - light gray background - 1/4 of screen
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    for (Col = SeparatorCol + 1; Col < mConsoleColumns; Col++) {
      Print (L" ");
    }
  }
}

**/

/**
  Display current submenu in left section (3/4) without any sliding effect.
**/
/**

VOID
DisplayCurrentSubmenu (
  VOID
  )
{
  UINTN Index, Col;
  SUBMENU_ITEM *CurrentSubmenu;
  UINTN ItemCount;
  // Changed: Left section is now 3/4 of screen
  UINTN SeparatorCol = (mConsoleColumns * 3) / 4;
  
  CurrentSubmenu = GetCurrentSubmenuArray (&ItemCount);
  mTotalSubItems = ItemCount;
  
  // First, completely clear the left section area (3/4 of screen)
  for (Index = 4; Index < mConsoleRows - 3; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 1, Index);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    // Clear the entire left section width (3/4 of screen)
    for (Col = 1; Col < SeparatorCol - 1; Col++) {
      Print (L" ");
    }
  }
  
  // Display submenu items statically (no animation/sliding)
  for (Index = 0; Index < ItemCount && Index < 15; Index++) {
    // Position cursor for this menu item
    gST->ConOut->SetCursorPosition (gST->ConOut, 3, Index + 5);
    
    // Set colors based on selection
    if (Index == mSelectedSubItem) {
      // Highlight selected item with blue background
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
    } else {
      // Normal item with light gray background
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    }
    
    // Print menu text once only
    Print (L"%s", CurrentSubmenu[Index].MenuText);
  }
}

**/

/**
  Display selected item information in right section (1/4).
**/
/**

VOID
DisplaySelectedTabInfo (
  VOID
  )
{
  SUBMENU_ITEM *CurrentSubmenu;
  UINTN ItemCount;
  CHAR16 TimeString[100];
  // Changed: Right section starts at 3/4 position
  UINTN SeparatorCol = (mConsoleColumns * 3) / 4;
  UINTN RightStart = SeparatorCol + 2;
  UINTN Index, Col;
  
  CurrentSubmenu = GetCurrentSubmenuArray (&ItemCount);
  
  // Clear right section completely first (1/4 of screen)
  for (Index = 4; Index < mConsoleRows - 3; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, Index);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    for (Col = RightStart; Col < mConsoleColumns - 1; Col++) {
      Print (L" ");
    }
  }
  
  // Display information header
  gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 5);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"Selected Item:");
  
  // Display based on current tab and selected submenu item
  if (mSelectedSubItem < ItemCount) {
    // Show selected item name
    gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 7);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_DARKGRAY | EFI_BACKGROUND_LIGHTGRAY);
    Print (L"%s", CurrentSubmenu[mSelectedSubItem].MenuText);
    
    // Show description (wrapped for smaller right section)
    gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 9);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    
    // For right section (1/4), we need to handle text wrapping
    UINTN RightWidth = mConsoleColumns - RightStart - 1;
    CHAR16 *Description = CurrentSubmenu[mSelectedSubItem].Description;
    UINTN DescLen = StrLen(Description);
    
    if (DescLen <= RightWidth) {
      Print (L"%s", Description);
    } else {
      // Simple word wrapping for narrow right section
      Print (L"%.20s...", Description);
    }
    
    // For Main tab's Change Date & Time item, show additional time info
    if (mSelectedMainTab == 0 && mSelectedSubItem == 3) {
      GetCurrentTime (TimeString);
      gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 12);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"Current:");
      gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 13);
      Print (L"%.15s", TimeString);
      
      gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 15);
      Print (L"Use +/- to");
      gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 16);
      Print (L"change values");
    }
  }
  
  // Show current tab name at bottom of right section
  gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, mConsoleRows - 5);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"Tab: %s", mMainTabs[mSelectedMainTab].MenuText);
}

**/

/**
  Display navigation help in bottom section.
**/
/**

VOID
DisplayNavigationHelp (
  VOID
  )
{
  UINTN Col;
  
  // Bottom blue bars with navigation help
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  
  // Fill bottom two rows with blue background
  for (UINTN Row = mConsoleRows - 2; Row < mConsoleRows; Row++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Row);
    for (Col = 0; Col < mConsoleColumns; Col++) {
      Print (L" ");
    }
  }
  
  // Display navigation instructions on bottom row
  gST->ConOut->SetCursorPosition (gST->ConOut, 2, mConsoleRows - 1);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  Print (L"←→: Select Screen  ↑↓: Select Item  Enter: Select  ESC: Exit");
}

**/

/**
  Display BIOS setup form with blue theme and static horizontal layout.
**/
/**

VOID
DisplayBiosSetupForm (
  VOID
  )
{
  mSelectedMainTab = 0;
  mSelectedSubItem = 0;
  mTotalMainTabs = ARRAY_SIZE (mMainTabs);
  mInBiosSetup = TRUE;
  
  // Initialize mouse support
  InitializeMouse ();
  
  // Draw the complete BIOS setup interface step by step (STATIC - no slide effects)
  DrawBlueBackground ();       // Includes title headers
  DrawHorizontalTabs ();       // Draw tabs without duplication
  DrawStaticLayout ();         // Draw the static 3/4-1/4 layout
  DisplayCurrentSubmenu ();    // Display submenu items statically
  DisplaySelectedTabInfo ();   // Display item info in right panel (1/4)
  DisplayNavigationHelp ();    // Bottom navigation help
  
  // Draw mouse cursor
  DrawMouseCursor ();
  
  // Reset console input for clean key detection
  gST->ConIn->Reset (gST->ConIn, FALSE);
  
  // Start navigation handling
  HandleBiosSetupNavigation ();
}
**/

/**
  Handle tab selection based on current selected tab.
**/
/**
VOID
HandleTabSelection (
  VOID
  )
{
  // Changed: Right section starts at 3/4 position
  UINTN SeparatorCol = (mConsoleColumns * 3) / 4;
  UINTN RightStart = SeparatorCol + 2;
  
  switch (mSelectedMainTab) {
    case 3: // Save & Exit
      if (mSelectedSubItem == 0) { // Save Changes and Exit
        gST->ConOut->ClearScreen (gST->ConOut);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
        gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 35) / 2, mConsoleRows / 2);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_GREEN | EFI_BACKGROUND_BLACK);
        Print (L"✓ Settings saved. Shutting down system...");
        gBS->Stall (2000000);
        gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
        mInBiosSetup = FALSE;
      } else if (mSelectedSubItem == 1) { // Discard Changes and Exit
        gST->ConOut->ClearScreen (gST->ConOut);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
        gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 40) / 2, mConsoleRows / 2);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_RED | EFI_BACKGROUND_BLACK);
        Print (L"✗ Exiting without saving. Shutting down...");
        gBS->Stall (2000000);
        gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
        mInBiosSetup = FALSE;
      } else if (mSelectedSubItem == 2) { // Save Changes
        // Show save confirmation in right panel
        gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_GREEN | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"✓ Saved");
        gBS->Stall (1500000);
        // Clear the message
        gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"        ");
      } else if (mSelectedSubItem == 3) { // Discard Changes
        // Show discard confirmation in right panel
        gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_RED | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"✗ Discarded");
        gBS->Stall (1500000);
        // Clear the message
        gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"           ");
      } else if (mSelectedSubItem == 4) { // Load Setup Defaults
        // Show defaults loaded confirmation in right panel
        gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"⚡ Defaults");
        gBS->Stall (1500000);
        // Clear the message
        gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"          ");
      }
      break;
      
    default:
      // For other tabs, show a brief action message in right panel
      gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 15);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"⚡ Action...");
      gBS->Stall (800000);
      // Clear the message
      gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 15);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"           ");
      break;
  }
}

**/

/**
  Handle navigation within BIOS setup using arrow keys and mouse (STATIC - No sliding effects).
**/

/**
VOID
HandleBiosSetupNavigation (
  VOID
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  UINTN          MouseTabIndex, MouseSubIndex;
  
  while (mInBiosSetup) {
    // Update mouse state
    UpdateMouseState ();
    
    // Redraw mouse cursor
    DrawMouseCursor ();
    
    // Handle mouse input
    if (mSimplePointer != NULL) {
      MouseTabIndex = GetTabFromMousePosition (mMouseState.LastX, mMouseState.LastY);
      MouseSubIndex = GetSubItemFromMousePosition (mMouseState.LastX, mMouseState.LastY);
      
      // Handle mouse clicks on tabs
      if (mMouseState.LeftButtonReleased && MouseTabIndex != (UINTN)-1) {
        if (MouseTabIndex != mSelectedMainTab) {
          mSelectedMainTab = MouseTabIndex;
          mSelectedSubItem = 0; // Reset submenu selection
          
          // Redraw interface WITHOUT sliding effects - just update display
          DrawHorizontalTabs ();    // Update tab selection
          DisplayCurrentSubmenu (); // Update submenu content (static)
          DisplaySelectedTabInfo (); // Update info panel
        }
        mMouseState.LeftButtonReleased = FALSE;
      }
      
      // Handle mouse clicks on submenu items
      if (mMouseState.LeftButtonReleased && MouseSubIndex != (UINTN)-1) {
        if (MouseSubIndex != mSelectedSubItem) {
          mSelectedSubItem = MouseSubIndex;
          
          // Redraw submenu and info only (static update)
          DisplayCurrentSubmenu ();
          DisplaySelectedTabInfo ();
        } else {
          // Double-click simulation - execute selection
          HandleTabSelection ();
          if (!mInBiosSetup) return;
        }
        mMouseState.LeftButtonReleased = FALSE;
      }
    }
    
    // Handle keyboard input
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
    
    if (!EFI_ERROR (Status)) {
      switch (Key.ScanCode) {
        case SCAN_LEFT:
          // Move left in main tabs
          if (mSelectedMainTab > 0) {
            mSelectedMainTab--;
          } else {
            mSelectedMainTab = mTotalMainTabs - 1; // Wrap to rightmost
          }
          mSelectedSubItem = 0; // Reset submenu selection
          
          // Update interface statically (no slide effects)
          DrawHorizontalTabs ();    // Update tab selection
          DisplayCurrentSubmenu (); // Update submenu content statically
          DisplaySelectedTabInfo (); // Update info panel
          break;
          
        case SCAN_RIGHT:
          // Move right in main tabs
          if (mSelectedMainTab < mTotalMainTabs - 1) {
            mSelectedMainTab++;
          } else {
            mSelectedMainTab = 0; // Wrap to leftmost
          }
          mSelectedSubItem = 0; // Reset submenu selection
          
          // Update interface statically (no slide effects)
          DrawHorizontalTabs ();    // Update tab selection
          DisplayCurrentSubmenu (); // Update submenu content statically
          DisplaySelectedTabInfo (); // Update info panel
          break;
          
        case SCAN_UP:
          // Move up in submenu
          if (mSelectedSubItem > 0) {
            mSelectedSubItem--;
          } else {
            mSelectedSubItem = mTotalSubItems - 1; // Wrap to bottom
          }
          
          // Update submenu and info only (static)
          DisplayCurrentSubmenu ();
          DisplaySelectedTabInfo ();
          break;
          
        case SCAN_DOWN:
          // Move down in submenu
          if (mSelectedSubItem < mTotalSubItems - 1) {
            mSelectedSubItem++;
          } else {
            mSelectedSubItem = 0; // Wrap to top
          }
          
          // Update submenu and info only (static)
          DisplayCurrentSubmenu ();
          DisplaySelectedTabInfo ();
          break;
          
        case SCAN_ESC:
          // Exit BIOS setup
          mInBiosSetup = FALSE;
          gST->ConOut->ClearScreen (gST->ConOut);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
          gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 40) / 2, mConsoleRows / 2);
          Print (L"🚪 Exiting BIOS Setup. Shutting down system...");
          gBS->Stall (2000000);
          gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
          return;
          
        default:
          // Check for ENTER key
          if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
            HandleTabSelection ();
            if (!mInBiosSetup) {
              return;
            }
          }
          break;
      }
    } else {
      // Small delay to prevent excessive CPU usage
      gBS->Stall (50000); // 50ms
    }
  }
}

**/

/**
  Monitor keyboard input for F12 key press only.
**/
/**

VOID
EFIAPI
KeyboardMonitor (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  
  // Only monitor after message is shown and not in BIOS setup
  if (!mMessageShown || mInBiosSetup) {
    return;
  }
  
  // Check if key is available
  Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
  
  if (!EFI_ERROR (Status)) {
    // Check for F12 key only
    if (Key.ScanCode == SCAN_F12) {
      DEBUG ((DEBUG_INFO, "F12 key pressed - showing BIOS setup form\n"));
      
      // Cancel timer and keyboard events
      gBS->SetTimer (mKeyMonitorEvent, TimerCancel, 0);
      gBS->SetTimer (mTimerEvent, TimerCancel, 0);
      
      // Display BIOS setup form
      DisplayBiosSetupForm ();
      
      // After exiting BIOS setup, shutdown
      gST->ConOut->ClearScreen (gST->ConOut);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
      gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 40) / 2, mConsoleRows / 2);
      Print (L"✅ BIOS Setup completed. Shutting down system...");
      gBS->Stall (2000000);
      gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
      return;
    }
  }
}

**/

/**
  Timer callback function to handle logo display timing.

  @param Event      The event that is signaled.
  @param Context    The context passed to the event.
**/

/**
VOID
EFIAPI
TimerCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  mTimerCounter++;
  
  // After 5 seconds, show the message below logo and wait for F12
  if (mTimerCounter == 50 && !mMessageShown) { // 50 * 100ms = 5 seconds
    // Clear the entire screen first
    gST->ConOut->ClearScreen (gST->ConOut);
    
    // Get console size for proper centering
    GetConsoleSize ();
    
    // Position cursor below the logo area (center of screen)
    gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 28) / 2, mConsoleRows / 2 + 5);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
    Print (L"Press F12 key for BIOS setup");
    
    mMessageShown = TRUE;
    
    // Reset console input for clean key detection
    gST->ConIn->Reset (gST->ConIn, FALSE);
    
    DEBUG ((DEBUG_INFO, "Message displayed, waiting for F12 key\n"));
  }
}

**/

/**
  Setup timing and keyboard monitoring after logo is displayed.
**/

/**
VOID
SetupLogoTiming (
  VOID
  )
{
  EFI_STATUS  Status;
  
  if (mLogoDisplayed) {
    return;
  }
  
  DEBUG ((DEBUG_INFO, "Setting up logo timing and keyboard monitoring\n"));
  
  // Create timer event for logo timing
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  TimerCallback,
                  NULL,
                  &mTimerEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    Status = gBS->SetTimer (
                    mTimerEvent,
                    TimerPeriodic,
                    1000000  // 100ms in 100ns units
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set timer for mTimerEvent: %r\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to create mTimerEvent: %r\n", Status));
  }
  
  // Create keyboard monitoring event
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  KeyboardMonitor,
                  NULL,
                  &mKeyMonitorEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    Status = gBS->SetTimer (
                    mKeyMonitorEvent,
                    TimerPeriodic,
                    500000  // 50ms in 100ns units
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set timer for mKeyMonitorEvent: %r\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to create mKeyMonitorEvent: %r\n", Status));
  }
  
  mLogoDisplayed = TRUE;
}
**/

/**
  Load a platform logo image and return its data and attributes.
**/
/**

EFI_STATUS
EFIAPI
GetImage (
  IN     EDKII_PLATFORM_LOGO_PROTOCOL        *This,
  IN OUT UINT32                              *Instance,
  OUT EFI_IMAGE_INPUT                        *Image,
  OUT EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE  *Attribute,
  OUT INTN                                   *OffsetX,
  OUT INTN                                   *OffsetY
  )
{
  UINT32      Current;
  EFI_STATUS  Status;

  if ((Instance == NULL) || (Image == NULL) ||
      (Attribute == NULL) || (OffsetX == NULL) || (OffsetY == NULL))
  {
    return EFI_INVALID_PARAMETER;
  }

  Current = *Instance;
  if (Current >= ARRAY_SIZE (mLogos)) {
    return EFI_NOT_FOUND;
  }

  (*Instance)++;
  *Attribute = mLogos[Current].Attribute;
  *OffsetX   = mLogos[Current].OffsetX;
  *OffsetY   = mLogos[Current].OffsetY;
  
  // Get image from HII database
  Status = mHiiImageEx->GetImageEx (mHiiImageEx, mHiiHandle, mLogos[Current].ImageId, Image);
  
  // After first logo is displayed, setup timing
  if (Current == 0 && !EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Logo displayed, setting up timing\n"));
    SetupLogoTiming ();
  }
  
  return Status;
}

EDKII_PLATFORM_LOGO_PROTOCOL  mPlatformLogo = {
  GetImage
};

**/
/**
  Cleanup function called when driver is unloaded.
**/
/**
EFI_STATUS
EFIAPI
LogoDriverUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
  // Close events if they exist
  if (mTimerEvent != NULL) {
    gBS->CloseEvent (mTimerEvent);
    mTimerEvent = NULL;
  }
  
  if (mKeyMonitorEvent != NULL) {
    gBS->CloseEvent (mKeyMonitorEvent);
    mKeyMonitorEvent = NULL;
  }
  
  if (mBiosSetupKeyEvent != NULL) {
    gBS->CloseEvent (mBiosSetupKeyEvent);
    mBiosSetupKeyEvent = NULL;
  }
  
  if (mMouseEvent != NULL) {
    gBS->CloseEvent (mMouseEvent);
    mMouseEvent = NULL;
  }
  
  return EFI_SUCCESS;
}

**/
/**
  Entrypoint of this module.
**/
/**

EFI_STATUS
EFIAPI
InitializeLogo (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                   Status;
  EFI_HII_PACKAGE_LIST_HEADER  *PackageList;
  EFI_HII_DATABASE_PROTOCOL    *HiiDatabase;
  EFI_HANDLE                   Handle;

  DEBUG ((DEBUG_INFO, "Logo Driver: Initializing with blue theme and static horizontal tabs...\n"));

  // Initialize global variables
  mInBiosSetup = FALSE;
  mSelectedMainTab = 0;
  mSelectedSubItem = 0;
  mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;
  mTotalMainTabs = ARRAY_SIZE (mMainTabs);

  // Locate HII Database Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiDatabaseProtocolGuid,
                  NULL,
                  (VOID **)&HiiDatabase
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Database Protocol: %r\n", Status));
    return Status;
  }

  // Locate HII Image Ex Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiImageExProtocolGuid,
                  NULL,
                  (VOID **)&mHiiImageEx
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Image Ex Protocol: %r\n", Status));
    return Status;
  }

  // Retrieve HII package list from ImageHandle
  Status = gBS->OpenProtocol (
                  ImageHandle,
                  &gEfiHiiPackageListProtocolGuid,
                  (VOID **)&PackageList,
                  ImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "HII Image Package with logo not found in PE/COFF resource section\n"));
    return Status;
  }

  // Publish HII package list to HII Database
  Status = HiiDatabase->NewPackageList (
                          HiiDatabase,
                          PackageList,
                          NULL,
                          &mHiiHandle
                          );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to add package list to HII Database: %r\n", Status));
    return Status;
  }

  // Install Platform Logo Protocol
  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEdkiiPlatformLogoProtocolGuid,
                  &mPlatformLogo,
                  NULL
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to install Platform Logo Protocol: %r\n", Status));
    HiiDatabase->RemovePackageList (HiiDatabase, mHiiHandle);
    return Status;
  }

  DEBUG ((DEBUG_INFO, "Logo Driver: Initialized successfully with blue theme static layout\n"));
  
  return EFI_SUCCESS;
}

**/



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





// Fixed BIOS setup utility with static form and 3/4-1/4 layout but menu duplicate is problem.

/** @file
  Logo DXE Driver with F12 key BIOS setup functionality.
  Shows logo, waits for F12 key, then displays BIOS setup form with navigation.
  Enhanced with blue theme, horizontal menu tabs and static three-section layout.

Copyright (c) 2016 - 2017, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent
**/
/**

#include <Uefi.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/HiiImageEx.h>
#include <Protocol/PlatformLogo.h>
#include <Protocol/HiiPackageList.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/SimpleTextOut.h>
#include <Protocol/SimplePointer.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>
#include <Library/MemoryAllocationLib.h>

typedef struct {
  EFI_IMAGE_ID                             ImageId;
  EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE    Attribute;
  INTN                                     OffsetX;
  INTN                                     OffsetY;
} LOGO_ENTRY;

// Menu item structure for horizontal tabs
typedef struct {
  CHAR16  *MenuText;
  CHAR16  *Description;
  UINT32  StartCol;    // Column where menu tab starts
  UINT32  EndCol;      // Column where menu tab ends
  UINT32  TabWidth;    // Width of the tab
} MENU_TAB;

// Submenu item structure
typedef struct {
  CHAR16  *MenuText;
  CHAR16  *Description;
  UINT32  StartRow;    // Row where menu item starts
  UINT32  StartCol;    // Column where menu item starts  
  UINT32  EndRow;      // Row where menu item ends
  UINT32  EndCol;      // Column where menu item ends
} SUBMENU_ITEM;

// Mouse state structure
typedef struct {
  BOOLEAN  LeftButtonPressed;
  BOOLEAN  LeftButtonReleased;
  BOOLEAN  WasLeftButtonPressed;
  UINT32   ClickCount;
  UINT64   LastClickTime;
  INT32    LastX;
  INT32    LastY;
  BOOLEAN  Visible;
} MOUSE_STATE;

// Menu level enumeration
typedef enum {
  MENU_LEVEL_MAIN_MENU = 0,
  MENU_LEVEL_MAIN_SUBMENU,
  MENU_LEVEL_SECURITY_SUBMENU,
  MENU_LEVEL_ADVANCED_SUBMENU,
  MENU_LEVEL_UEFI_SUBMENU
} MENU_LEVEL;

// Global variables
EFI_HII_IMAGE_EX_PROTOCOL  *mHiiImageEx;
EFI_HII_HANDLE             mHiiHandle;
EFI_EVENT                  mKeyMonitorEvent;
EFI_EVENT                  mTimerEvent;
EFI_EVENT                  mBiosSetupKeyEvent;
EFI_EVENT                  mMouseEvent;
EFI_SIMPLE_POINTER_PROTOCOL *mSimplePointer = NULL;
BOOLEAN                    mLogoDisplayed = FALSE;
BOOLEAN                    mMessageShown = FALSE;
BOOLEAN                    mInBiosSetup = FALSE;
UINTN                      mTimerCounter = 0;
UINTN                      mSelectedMainTab = 0;
UINTN                      mSelectedSubItem = 0;
UINTN                      mTotalMainTabs = 0;
UINTN                      mTotalSubItems = 0;
MENU_LEVEL                 mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;
MOUSE_STATE                mMouseState;
UINTN                      mConsoleColumns = 100;  // Increased default size
UINTN                      mConsoleRows = 40;      // Increased default size

// Main horizontal tabs (fixed spacing and positioning)
MENU_TAB mMainTabs[] = {
  { L"Main", L"System Information and Basic Settings", 2, 17, 16 },
  { L"Advanced", L"Advanced Configuration Options", 18, 35, 18 },
  { L"Security", L"Security and password Settings", 36, 53, 18 },
  { L"Save & Exit", L"Save changes and exit setup", 54, 75, 22 }
};

// Main submenu items (for Main tab)
SUBMENU_ITEM mMainSubmenu[] = {
  { L"System Information", L"View BIOS version and system information", 5, 5, 5, 35 },
  { L"BIOS Eventlog", L"Display BIOS event log entries", 6, 5, 6, 35 },
  { L"Update system BIOS", L"Update system BIOS firmware", 7, 5, 7, 35 },
  { L"Change Date & Time", L"Configure system date and time", 8, 5, 8, 35 },
};

// Advanced submenu items
SUBMENU_ITEM mAdvancedSubmenu[] = {
  { L"Boot Options", L"Configure boot device priority and settings", 5, 5, 5, 35 },
  { L"Port Options", L"Configure USB, SATA, and other port settings", 6, 5, 6, 35 },
  { L"System Options", L"Configure CPU, memory, and system settings", 7, 5, 7, 35 },
  { L"Power Management", L"Configure power and thermal settings", 8, 5, 8, 35 },
};

// Security submenu items
SUBMENU_ITEM mSecuritySubmenu[] = {
  { L"Administrator Tools", L"Administrative security management tools", 5, 5, 5, 35 },
  { L"Create BIOS Password", L"Set BIOS administrator password protection", 6, 5, 6, 35 },
  { L"TPM Security", L"Configure Trusted Platform Module settings", 7, 5, 7, 35 },
  { L"Secure Boot", L"Enable or disable UEFI Secure Boot", 8, 5, 8, 35 },
};

// Save & Exit submenu items
SUBMENU_ITEM mSaveExitSubmenu[] = {
  { L"Save Changes and Exit", L"Save current settings and exit setup", 5, 5, 5, 35 },
  { L"Discard Changes and Exit", L"Exit setup without saving any changes", 6, 5, 6, 35 },
  { L"Save Changes", L"Save current settings and continue", 7, 5, 7, 35 },
  { L"Discard Changes", L"Reset to previous saved settings", 8, 5, 8, 35 },
  { L"Load Setup Defaults", L"Load factory default settings", 9, 5, 9, 35 }
};

// Function declarations
VOID DisplayBiosSetupForm (VOID);
VOID GetCurrentTime (OUT CHAR16 *TimeString);
VOID HandleBiosSetupNavigation (VOID);
VOID DrawBlueBackground (VOID);
VOID DrawHorizontalTabs (VOID);
VOID DrawStaticLayout (VOID);  // Changed from DrawThreeSectionLayout
VOID DisplayCurrentSubmenu (VOID);
VOID DisplaySelectedTabInfo (VOID);
VOID DisplayNavigationHelp (VOID);
EFI_STATUS InitializeMouse (VOID);
VOID UpdateMouseState (VOID);
UINTN GetTabFromMousePosition (IN INT32 MouseX, IN INT32 MouseY);
UINTN GetSubItemFromMousePosition (IN INT32 MouseX, IN INT32 MouseY);
VOID HandleTabSelection (VOID);
VOID HandleSubItemSelection (VOID);
SUBMENU_ITEM* GetCurrentSubmenuArray (OUT UINTN *ItemCount);
VOID DrawMouseCursor (VOID);
VOID GetConsoleSize (VOID);
VOID ClearTabRow (VOID);  // New function to properly clear tab row


LOGO_ENTRY mLogos[] = {
  {
    IMAGE_TOKEN (IMG_LOGO),
    EdkiiPlatformLogoDisplayAttributeCenter,
    0,
    0
  }
};
**/


/**
  Get console size and set to larger size for full screen display.
**/
/**

VOID
GetConsoleSize (
  VOID
  )
{
  EFI_STATUS Status;
  UINTN MaxMode, MaxColumns, MaxRows;
  UINTN BestMode = 0;
  UINTN BestColumns = 80;
  UINTN BestRows = 25;
  
  // Try to find the largest available mode
  MaxMode = gST->ConOut->Mode->MaxMode;
  
  for (UINTN Mode = 0; Mode < MaxMode; Mode++) {
    Status = gST->ConOut->QueryMode (
                            gST->ConOut,
                            Mode,
                            &MaxColumns,
                            &MaxRows
                            );
    
    if (!EFI_ERROR (Status)) {
      // Look for a larger mode that would give us better screen coverage
      if ((MaxColumns >= BestColumns && MaxRows >= BestRows) && 
          (MaxColumns > BestColumns || MaxRows > BestRows)) {
        BestMode = Mode;
        BestColumns = MaxColumns;
        BestRows = MaxRows;
      }
    }
  }
  
  // Set the best mode we found
  if (BestMode != gST->ConOut->Mode->Mode) {
    Status = gST->ConOut->SetMode (gST->ConOut, BestMode);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_WARN, "Failed to set console mode %d: %r\n", BestMode, Status));
    }
  }
  
  // Query the actual current mode
  Status = gST->ConOut->QueryMode (
                          gST->ConOut,
                          gST->ConOut->Mode->Mode,
                          &mConsoleColumns,
                          &mConsoleRows
                          );
  
  if (EFI_ERROR (Status)) {
    // Use larger default values
    mConsoleColumns = 100;
    mConsoleRows = 40;
  }
  
  DEBUG ((DEBUG_INFO, "Console size: %dx%d\n", mConsoleColumns, mConsoleRows));
}

**/

/**
  Get current submenu array based on selected main tab.

  @param ItemCount   Output parameter for number of items in submenu.

  @retval Pointer to current submenu array.
**/

/**
SUBMENU_ITEM*
GetCurrentSubmenuArray (
  OUT UINTN *ItemCount
  )
{
  switch (mSelectedMainTab) {
    case 0: // Main
      *ItemCount = ARRAY_SIZE (mMainSubmenu);
      return mMainSubmenu;
      
    case 1: // Advanced
      *ItemCount = ARRAY_SIZE (mAdvancedSubmenu);
      return mAdvancedSubmenu;
      
    case 2: // Security
      *ItemCount = ARRAY_SIZE (mSecuritySubmenu);
      return mSecuritySubmenu;
      
    case 3: // Save & Exit
      *ItemCount = ARRAY_SIZE (mSaveExitSubmenu);
      return mSaveExitSubmenu;
      
    default:
      *ItemCount = ARRAY_SIZE (mMainSubmenu);
      return mMainSubmenu;
  }
}

**/

/**
  Initialize mouse support by locating Simple Pointer Protocol.

  @retval EFI_SUCCESS   Mouse initialized successfully.
  @retval Others        Failed to initialize mouse.
**/

/**
EFI_STATUS
InitializeMouse (
  VOID
  )
{
  EFI_STATUS Status;
  
  // Initialize mouse state
  mMouseState.LeftButtonPressed = FALSE;
  mMouseState.LeftButtonReleased = FALSE;
  mMouseState.WasLeftButtonPressed = FALSE;
  mMouseState.ClickCount = 0;
  mMouseState.LastClickTime = 0;
  mMouseState.LastX = (INT32)(mConsoleColumns / 2); // Center of screen
  mMouseState.LastY = (INT32)(mConsoleRows / 2);
  mMouseState.Visible = TRUE;
  
  // Try to locate Simple Pointer Protocol
  Status = gBS->LocateProtocol (
                  &gEfiSimplePointerProtocolGuid,
                  NULL,
                  (VOID **)&mSimplePointer
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "Simple Pointer Protocol not available: %r\n", Status));
    mSimplePointer = NULL;
    return Status;
  }
  
  // Reset mouse
  Status = mSimplePointer->Reset (mSimplePointer, TRUE);
  if (EFI_ERROR (Status)) {
    Status = mSimplePointer->Reset (mSimplePointer, FALSE);
    if (EFI_ERROR (Status)) {
      mSimplePointer = NULL;
      return Status;
    }
  }
  
  DEBUG ((DEBUG_INFO, "Mouse initialized successfully\n"));
  return EFI_SUCCESS;
}

**/

/**
  Update mouse state by reading current mouse input.
**/

/**
VOID
UpdateMouseState (
  VOID
  )
{
  EFI_STATUS                Status;
  EFI_SIMPLE_POINTER_STATE  MouseState;
  
  if (mSimplePointer == NULL) {
    return;
  }
  
  Status = mSimplePointer->GetState (mSimplePointer, &MouseState);
  if (EFI_ERROR (Status)) {
    return;
  }
  
  // Handle relative movement
  INT32 DeltaX = MouseState.RelativeMovementX / 65536;
  INT32 DeltaY = MouseState.RelativeMovementY / 65536;
  
  if (DeltaX != 0 || DeltaY != 0) {
    mMouseState.LastX += DeltaX;
    mMouseState.LastY += DeltaY;
    
    // Boundary check
    if (mMouseState.LastX < 0) mMouseState.LastX = 0;
    if (mMouseState.LastY < 0) mMouseState.LastY = 0;
    if (mMouseState.LastX >= (INT32)mConsoleColumns) mMouseState.LastX = (INT32)mConsoleColumns - 1;
    if (mMouseState.LastY >= (INT32)mConsoleRows) mMouseState.LastY = (INT32)mConsoleRows - 1;
  }
  
  // Update button state
  mMouseState.WasLeftButtonPressed = mMouseState.LeftButtonPressed;
  mMouseState.LeftButtonPressed = MouseState.LeftButton;
  
  // Detect button release (click completion)
  if (mMouseState.WasLeftButtonPressed && !mMouseState.LeftButtonPressed) {
    mMouseState.LeftButtonReleased = TRUE;
    mMouseState.ClickCount++;
  } else {
    mMouseState.LeftButtonReleased = FALSE;
  }
}

**/

/**
  Draw mouse cursor at current position.
**/
/**

VOID
DrawMouseCursor (
  VOID
  )
{
  if (!mMouseState.Visible || mSimplePointer == NULL) {
    return;
  }
  
  gST->ConOut->SetCursorPosition (gST->ConOut, mMouseState.LastX, mMouseState.LastY);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
  Print (L"▓");
}

**/

/**
  Get tab index based on mouse position.

  @param MouseX   Mouse X coordinate.
  @param MouseY   Mouse Y coordinate.

  @retval Index   Tab index, or -1 if not over any tab.
**/

/**
UINTN
GetTabFromMousePosition (
  IN INT32 MouseX,
  IN INT32 MouseY
  )
{
  UINTN Index;
  
  // Check if mouse is on tab row (row 2)
  if (MouseY != 2) {
    return (UINTN)-1;
  }
  
  for (Index = 0; Index < mTotalMainTabs; Index++) {
    if (MouseX >= mMainTabs[Index].StartCol && MouseX <= mMainTabs[Index].EndCol) {
      return Index;
    }
  }
  
  return (UINTN)-1;
}
**/

/**
  Get submenu item index based on mouse position.

  @param MouseX   Mouse X coordinate.
  @param MouseY   Mouse Y coordinate.

  @retval Index   Submenu item index, or -1 if not over any item.
**/

/**
UINTN
GetSubItemFromMousePosition (
  IN INT32 MouseX,
  IN INT32 MouseY
  )
{
  UINTN Index;
  SUBMENU_ITEM *CurrentSubmenu;
  UINTN ItemCount;
  
  CurrentSubmenu = GetCurrentSubmenuArray (&ItemCount);
  
  for (Index = 0; Index < ItemCount; Index++) {
    if (MouseY == CurrentSubmenu[Index].StartRow + 1 && // Add offset for header rows
        MouseX >= CurrentSubmenu[Index].StartCol &&
        MouseX <= CurrentSubmenu[Index].EndCol) {
      return Index;
    }
  }
  
  return (UINTN)-1;
}

**/

/**
  Get current time and format it as string.

  @param TimeString   Output buffer for formatted time string.
**/
/**

VOID
GetCurrentTime (
  OUT CHAR16  *TimeString
  )
{
  EFI_TIME    Time;
  EFI_STATUS  Status;
  
  Status = gRT->GetTime (&Time, NULL);
  if (!EFI_ERROR (Status)) {
    UnicodeSPrint (
      TimeString, 
      50 * sizeof(CHAR16),
      L"%02d/%02d/%d  %02d:%02d:%02d",
      Time.Day,
      Time.Month,
      Time.Year,
      Time.Hour,
      Time.Minute,
      Time.Second
    );
  } else {
    UnicodeSPrint (TimeString, 50 * sizeof(CHAR16), L"[Date/Time Not Available]");
  }
}

**/

/**
  Draw blue background with BIOS Setup Utility in top-left corner.
**/
/**

VOID
DrawBlueBackground (
  VOID
  )
{
  UINTN Index, Col;
  
  // Get actual console size and try to maximize it
  GetConsoleSize ();
  
  // Clear screen and set blue background
  gST->ConOut->ClearScreen (gST->ConOut);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  
  // Fill entire screen with blue background
  for (Index = 0; Index < mConsoleRows; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Index);
    for (Col = 0; Col < mConsoleColumns; Col++) {
      Print (L" ");
    }
  }
  
  // Draw BIOS Setup Utility in top-left corner in WHITE
  gST->ConOut->SetCursorPosition (gST->ConOut, 1, 0);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  Print (L"BIOS Setup Utility");
  
  // Draw CDAC BANGALORE title below center
  gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 14) / 2, 1);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
  Print (L"BHARAT BIOS CDAC");
}

**/
/**
  Clear tab row completely to prevent duplication.
**/

/**
VOID
ClearTabRow (
  VOID
  )
{
  UINTN Col;
  
  // Completely clear the tab row (row 2) with blue background
  gST->ConOut->SetCursorPosition (gST->ConOut, 0, 2);
 
  
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  for (Col = 0; Col < mConsoleColumns; Col++) {
    Print (L" ");
  }
}


**/


/**
  Draw horizontal tabs at top without duplication.
**/
/**

VOID
DrawHorizontalTabs (
  VOID
  )
{
//  UINTN Index, Col;
   UINTN Index;
  
  // First, completely clear the tab row with blue background
//  gST->ConOut->SetCursorPosition (gST->ConOut, 0, 2);
//  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
//  for (Col = 0; Col < mConsoleColumns; Col++) {
//    Print (L" ");
//  }
  
  
  ClearTabRow ();
  
  // Now draw each tab exactly once with proper positioning
  for (Index = 0; Index < mTotalMainTabs; Index++) {
    // Position cursor at the start of this tab
   gST->ConOut->SetCursorPosition (gST->ConOut, mMainTabs[Index].StartCol, 2);
    
    
    // Set colors based on selection
    if (Index == mSelectedMainTab) {
      // Selected tab - light gray background with black text
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    } else {
      // Unselected tab - blue background with white text
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
    }
    
    // Print the tab text with proper padding
    Print (L" %s ", mMainTabs[Index].MenuText);
  }
}

**/
/**
  Draw the static layout with 3/4 left and 1/4 right sections.
**/
/**

VOID
DrawStaticLayout (
  VOID
  )
{
  UINTN Row, Col;
  // Changed: Now left section is 3/4 of screen, right is 1/4
  UINTN SeparatorCol = (mConsoleColumns * 3) / 4; // 3/4 position for separator
  
  // Clear and fill the main content area (rows 3 to second-to-last)
  for (Row = 3; Row < mConsoleRows - 2; Row++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Row);
    
    // Left section (submenu area) - light gray background - 3/4 of screen
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    for (Col = 0; Col < SeparatorCol; Col++) {
      Print (L" ");
    }
    
    // Vertical separator
    if (SeparatorCol < mConsoleColumns) {
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"│");
    }
    
    // Right section (info area) - light gray background - 1/4 of screen
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    for (Col = SeparatorCol + 1; Col < mConsoleColumns; Col++) {
      Print (L" ");
    }
  }
}

**/

/**
  Display current submenu in left section (3/4) without any sliding effect.
**/

/**
VOID
DisplayCurrentSubmenu (
  VOID
  )
{
  UINTN Index, Col;
  SUBMENU_ITEM *CurrentSubmenu;
  UINTN ItemCount;
  // Changed: Left section is now 3/4 of screen
  UINTN SeparatorCol = (mConsoleColumns * 3) / 4;
  
  CurrentSubmenu = GetCurrentSubmenuArray (&ItemCount);
  mTotalSubItems = ItemCount;
  
  // First, completely clear the left section area (3/4 of screen)
  for (Index = 4; Index < mConsoleRows - 3; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 1, Index);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    // Clear the entire left section width (3/4 of screen)
    for (Col = 1; Col < SeparatorCol - 1; Col++) {
      Print (L" ");
    }
  }
  
  // Display submenu items statically (no animation/sliding) - ONLY ONCE
  for (Index = 0; Index < ItemCount && Index < 15; Index++) {
    // Position cursor for this menu item - each item gets its own row
    gST->ConOut->SetCursorPosition (gST->ConOut, 3, Index + 5);
    
    // Set colors based on selection
    if (Index == mSelectedSubItem) {
      // Highlight selected item with blue background
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
    } else {
      // Normal item with light gray background
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    }
    
    // Print menu text EXACTLY ONCE - no duplication
    Print (L"%s", CurrentSubmenu[Index].MenuText);
  }
}

**/

/**
  Display selected item information in right section (1/4) with proper multi-line description.
**/

/**
VOID
DisplaySelectedTabInfo (
  VOID
  )
{
  SUBMENU_ITEM *CurrentSubmenu;
  UINTN ItemCount;
  CHAR16 TimeString[100];
  // Changed: Right section starts at 3/4 position
  UINTN SeparatorCol = (mConsoleColumns * 3) / 4;
  UINTN RightStart = SeparatorCol + 2;
  UINTN Index, Col;
  
  CurrentSubmenu = GetCurrentSubmenuArray (&ItemCount);
  
  // Clear right section completely first (1/4 of screen)
  for (Index = 4; Index < mConsoleRows - 3; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, Index);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    for (Col = RightStart; Col < mConsoleColumns - 1; Col++) {
      Print (L" ");
    }
  }
  
  // Display information header
  gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 5);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"Selected Item:");
  
  // Display based on current tab and selected submenu item
  if (mSelectedSubItem < ItemCount) {
    // Show selected item name
    gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 7);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_DARKGRAY | EFI_BACKGROUND_LIGHTGRAY);
    Print (L"%s", CurrentSubmenu[mSelectedSubItem].MenuText);
    
    // Show description with proper multi-line wrapping
    gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 9);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    
    // Calculate available width for right section (1/4)
    UINTN RightWidth = mConsoleColumns - RightStart - 2; // Leave some margin
    CHAR16 *Description = CurrentSubmenu[mSelectedSubItem].Description;
    UINTN DescLen = StrLen(Description);
    
    // Multi-line description display
    if (DescLen <= RightWidth) {
      // Single line - fits completely
      Print (L"%s", Description);
    } else {
      // Multi-line - need to wrap text properly
      UINTN CurrentPos = 0;
      UINTN CurrentLine = 9; // Starting line for description
      
      while (CurrentPos < DescLen && CurrentLine < mConsoleRows - 6) {
        gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, CurrentLine);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
        
        UINTN CharsToShow = RightWidth;
        if (CurrentPos + CharsToShow > DescLen) {
          CharsToShow = DescLen - CurrentPos;
        }
        
        // Find last space for proper word wrapping
        if (CurrentPos + CharsToShow < DescLen) {
          UINTN LastSpace = CharsToShow;
          for (UINTN i = CharsToShow; i > 0; i--) {
            if (Description[CurrentPos + i - 1] == L' ') {
              LastSpace = i;
              break;
            }
          }
          if (LastSpace < CharsToShow) {
            CharsToShow = LastSpace;
          }
        }
        
        // Print this line portion
        for (UINTN i = 0; i < CharsToShow && CurrentPos < DescLen; i++) {
          Print (L"%c", Description[CurrentPos]);
          CurrentPos++;
        }
        
        // Skip space if we broke at a space
        if (CurrentPos < DescLen && Description[CurrentPos] == L' ') {
          CurrentPos++;
        }
        
        CurrentLine++;
      }
    }
    
    // For Main tab's Change Date & Time item, show additional time info
    if (mSelectedMainTab == 0 && mSelectedSubItem == 3) {
      GetCurrentTime (TimeString);
      gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 12);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"Current:");
      gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 13);
      
      // Time display with wrapping if needed
      UINTN TimeLen = StrLen(TimeString);
      if (TimeLen <= RightWidth) {
        Print (L"%s", TimeString);
      } else {
        Print (L"%.15s", TimeString);
        gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 14);
        Print (L"%s", &TimeString[15]);
      }
      
      gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 16);
      Print (L"Use +/- to");
      gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 17);
      Print (L"change values");
    }
  }
  
  // Show current tab name at bottom of right section
  gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, mConsoleRows - 5);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"Tab: %s", mMainTabs[mSelectedMainTab].MenuText);
}

**/

/**
  Display navigation help in bottom section.
**/

/**
VOID
DisplayNavigationHelp (
  VOID
  )
{
  UINTN Col;
  
  // Bottom blue bars with navigation help
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  
  // Fill bottom two rows with blue background
  for (UINTN Row = mConsoleRows - 2; Row < mConsoleRows; Row++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Row);
    for (Col = 0; Col < mConsoleColumns; Col++) {
      Print (L" ");
    }
  }
  
  // Display navigation instructions on bottom row
  gST->ConOut->SetCursorPosition (gST->ConOut, 2, mConsoleRows - 1);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  Print (L"←→: Select Screen  ↑↓: Select Item  Enter: Select  ESC: Exit");
}

**/

/**
  Display BIOS setup form with blue theme and static horizontal layout.
**/
/**

VOID
DisplayBiosSetupForm (
  VOID
  )
{
  mSelectedMainTab = 0;
  mSelectedSubItem = 0;
  mTotalMainTabs = ARRAY_SIZE (mMainTabs);
  mInBiosSetup = TRUE;
  
  // Initialize mouse support
  InitializeMouse ();
  
  // Draw the complete BIOS setup interface step by step (STATIC - no slide effects)
  DrawBlueBackground ();       // Includes title headers
  DrawHorizontalTabs ();       // Draw tabs without duplication
  DrawStaticLayout ();         // Draw the static 3/4-1/4 layout
  DisplayCurrentSubmenu ();    // Display submenu items statically
  DisplaySelectedTabInfo ();   // Display item info in right panel (1/4)
  DisplayNavigationHelp ();    // Bottom navigation help
  
  // Draw mouse cursor
  DrawMouseCursor ();
  
  // Reset console input for clean key detection
  gST->ConIn->Reset (gST->ConIn, FALSE);
  
  // Start navigation handling
  HandleBiosSetupNavigation ();
}
**/

/**
  Handle tab selection based on current selected tab.
**/
/**
VOID
HandleTabSelection (
  VOID
  )
{
  // Changed: Right section starts at 3/4 position
  UINTN SeparatorCol = (mConsoleColumns * 3) / 4;
  UINTN RightStart = SeparatorCol + 2;
  
  switch (mSelectedMainTab) {
    case 3: // Save & Exit
      if (mSelectedSubItem == 0) { // Save Changes and Exit
        gST->ConOut->ClearScreen (gST->ConOut);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
        gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 35) / 2, mConsoleRows / 2);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_GREEN | EFI_BACKGROUND_BLACK);
        Print (L"✓ Settings saved. Shutting down system...");
        gBS->Stall (2000000);
        gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
        mInBiosSetup = FALSE;
      } else if (mSelectedSubItem == 1) { // Discard Changes and Exit
        gST->ConOut->ClearScreen (gST->ConOut);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
        gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 40) / 2, mConsoleRows / 2);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_RED | EFI_BACKGROUND_BLACK);
        Print (L"✗ Exiting without saving. Shutting down...");
        gBS->Stall (2000000);
        gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
        mInBiosSetup = FALSE;
      } else if (mSelectedSubItem == 2) { // Save Changes
        // Show save confirmation in right panel
        gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_GREEN | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"✓ Saved");
        gBS->Stall (1500000);
        // Clear the message
        gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"        ");
      } else if (mSelectedSubItem == 3) { // Discard Changes
        // Show discard confirmation in right panel
        gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_RED | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"✗ Discarded");
        gBS->Stall (1500000);
        // Clear the message
        gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"           ");
      } else if (mSelectedSubItem == 4) { // Load Setup Defaults
        // Show defaults loaded confirmation in right panel
        gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"⚡ Defaults");
        gBS->Stall (1500000);
        // Clear the message
        gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"          ");
      }
      break;
      
    default:
      // For other tabs, show a brief action message in right panel
      gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 15);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"⚡ Action...");
      gBS->Stall (800000);
      // Clear the message
      gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 15);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"           ");
      break;
  }
}

**/

/**
  Handle navigation within BIOS setup using arrow keys and mouse (STATIC - No sliding effects).
**/

/**
VOID
HandleBiosSetupNavigation (
  VOID
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  UINTN          MouseTabIndex, MouseSubIndex;
  
  while (mInBiosSetup) {
    // Update mouse state
    UpdateMouseState ();
    
    // Redraw mouse cursor
    DrawMouseCursor ();
    
    // Handle mouse input
    if (mSimplePointer != NULL) {
      MouseTabIndex = GetTabFromMousePosition (mMouseState.LastX, mMouseState.LastY);
      MouseSubIndex = GetSubItemFromMousePosition (mMouseState.LastX, mMouseState.LastY);
      
      // Handle mouse clicks on tabs
      if (mMouseState.LeftButtonReleased && MouseTabIndex != (UINTN)-1) {
        if (MouseTabIndex != mSelectedMainTab) {
          mSelectedMainTab = MouseTabIndex;
          mSelectedSubItem = 0; // Reset submenu selection
          
          // Redraw interface WITHOUT sliding effects - just update display
          DrawHorizontalTabs ();    // Update tab selection
          DisplayCurrentSubmenu (); // Update submenu content (static)
          DisplaySelectedTabInfo (); // Update info panel
        }
        mMouseState.LeftButtonReleased = FALSE;
      }
      
      // Handle mouse clicks on submenu items
      if (mMouseState.LeftButtonReleased && MouseSubIndex != (UINTN)-1) {
        if (MouseSubIndex != mSelectedSubItem) {
          mSelectedSubItem = MouseSubIndex;
          
          // Redraw submenu and info only (static update)
          DisplayCurrentSubmenu ();
          DisplaySelectedTabInfo ();
        } else {
          // Double-click simulation - execute selection
          HandleTabSelection ();
          if (!mInBiosSetup) return;
        }
        mMouseState.LeftButtonReleased = FALSE;
      }
    }
    
    // Handle keyboard input
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
    
    if (!EFI_ERROR (Status)) {
      switch (Key.ScanCode) {
        case SCAN_LEFT:
          // Move left in main tabs
          if (mSelectedMainTab > 0) {
            mSelectedMainTab--;
          } else {
            mSelectedMainTab = mTotalMainTabs - 1; // Wrap to rightmost
          }
          mSelectedSubItem = 0; // Reset submenu selection
          
          // Update interface statically (no slide effects)
          DrawHorizontalTabs ();    // Update tab selection
          DisplayCurrentSubmenu (); // Update submenu content statically
          DisplaySelectedTabInfo (); // Update info panel
          break;
          
        case SCAN_RIGHT:
          // Move right in main tabs
          if (mSelectedMainTab < mTotalMainTabs - 1) {
            mSelectedMainTab++;
          } else {
            mSelectedMainTab = 0; // Wrap to leftmost
          }
          mSelectedSubItem = 0; // Reset submenu selection
          
          // Update interface statically (no slide effects)
          DrawHorizontalTabs ();    // Update tab selection
          DisplayCurrentSubmenu (); // Update submenu content statically
          DisplaySelectedTabInfo (); // Update info panel
          break;
          
        case SCAN_UP:
          // Move up in submenu
          if (mSelectedSubItem > 0) {
            mSelectedSubItem--;
          } else {
            mSelectedSubItem = mTotalSubItems - 1; // Wrap to bottom
          }
          
          // Update submenu and info only (static)
          DisplayCurrentSubmenu ();
          DisplaySelectedTabInfo ();
          break;
          
        case SCAN_DOWN:
          // Move down in submenu
          if (mSelectedSubItem < mTotalSubItems - 1) {
            mSelectedSubItem++;
          } else {
            mSelectedSubItem = 0; // Wrap to top
          }
          
          // Update submenu and info only (static)
          DisplayCurrentSubmenu ();
          DisplaySelectedTabInfo ();
          break;
          
        case SCAN_ESC:
          // Exit BIOS setup
          mInBiosSetup = FALSE;
          gST->ConOut->ClearScreen (gST->ConOut);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
          gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 40) / 2, mConsoleRows / 2);
          Print (L"🚪 Exiting BIOS Setup. Shutting down system...");
          gBS->Stall (2000000);
          gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
          return;
          
        default:
          // Check for ENTER key
          if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
            HandleTabSelection ();
            if (!mInBiosSetup) {
              return;
            }
          }
          break;
      }
    } else {
      // Small delay to prevent excessive CPU usage
      gBS->Stall (50000); // 50ms
    }
  }
}

**/

/**
  Monitor keyboard input for F12 key press only.
**/

/**
VOID
EFIAPI
KeyboardMonitor (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  
  // Only monitor after message is shown and not in BIOS setup
  if (!mMessageShown || mInBiosSetup) {
    return;
  }
  
  // Check if key is available
  Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
  
  if (!EFI_ERROR (Status)) {
    // Check for F12 key only
    if (Key.ScanCode == SCAN_F12) {
      DEBUG ((DEBUG_INFO, "F12 key pressed - showing BIOS setup form\n"));
      
      // Cancel timer and keyboard events
      gBS->SetTimer (mKeyMonitorEvent, TimerCancel, 0);
      gBS->SetTimer (mTimerEvent, TimerCancel, 0);
      
      // Display BIOS setup form
      DisplayBiosSetupForm ();
      
      // After exiting BIOS setup, shutdown
      gST->ConOut->ClearScreen (gST->ConOut);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
      gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 40) / 2, mConsoleRows / 2);
      Print (L"✅ BIOS Setup completed. Shutting down system...");
      gBS->Stall (2000000);
      gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
      return;
    }
  }
}

**/

/**
  Timer callback function to handle logo display timing.

  @param Event      The event that is signaled.
  @param Context    The context passed to the event.
**/

/**
VOID
EFIAPI
TimerCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  mTimerCounter++;
  
  // After 5 seconds, show the message below logo and wait for F12
  if (mTimerCounter == 50 && !mMessageShown) { // 50 * 100ms = 5 seconds
    // Clear the entire screen first
    gST->ConOut->ClearScreen (gST->ConOut);
    
    // Get console size for proper centering
    GetConsoleSize ();
    
    // Position cursor below the logo area (center of screen)
    gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 28) / 2, mConsoleRows / 2 + 5);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
    Print (L"Press F12 key for BIOS setup");
    
    mMessageShown = TRUE;
    
    // Reset console input for clean key detection
    gST->ConIn->Reset (gST->ConIn, FALSE);
    
    DEBUG ((DEBUG_INFO, "Message displayed, waiting for F12 key\n"));
  }
}

**/

/**
  Setup timing and keyboard monitoring after logo is displayed.
**/
/**

VOID
SetupLogoTiming (
  VOID
  )
{
  EFI_STATUS  Status;
  
  if (mLogoDisplayed) {
    return;
  }
  
  DEBUG ((DEBUG_INFO, "Setting up logo timing and keyboard monitoring\n"));
  
  // Create timer event for logo timing
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  TimerCallback,
                  NULL,
                  &mTimerEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    Status = gBS->SetTimer (
                    mTimerEvent,
                    TimerPeriodic,
                    1000000  // 100ms in 100ns units
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set timer for mTimerEvent: %r\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to create mTimerEvent: %r\n", Status));
  }
  
  // Create keyboard monitoring event
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  KeyboardMonitor,
                  NULL,
                  &mKeyMonitorEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    Status = gBS->SetTimer (
                    mKeyMonitorEvent,
                    TimerPeriodic,
                    500000  // 50ms in 100ns units
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set timer for mKeyMonitorEvent: %r\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to create mKeyMonitorEvent: %r\n", Status));
  }
  
  mLogoDisplayed = TRUE;
}
**/

/**
  Load a platform logo image and return its data and attributes.
**/

/**
EFI_STATUS
EFIAPI
GetImage (
  IN     EDKII_PLATFORM_LOGO_PROTOCOL        *This,
  IN OUT UINT32                              *Instance,
  OUT EFI_IMAGE_INPUT                        *Image,
  OUT EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE  *Attribute,
  OUT INTN                                   *OffsetX,
  OUT INTN                                   *OffsetY
  )
{
  UINT32      Current;
  EFI_STATUS  Status;

  if ((Instance == NULL) || (Image == NULL) ||
      (Attribute == NULL) || (OffsetX == NULL) || (OffsetY == NULL))
  {
    return EFI_INVALID_PARAMETER;
  }

  Current = *Instance;
  if (Current >= ARRAY_SIZE (mLogos)) {
    return EFI_NOT_FOUND;
  }

  (*Instance)++;
  *Attribute = mLogos[Current].Attribute;
  *OffsetX   = mLogos[Current].OffsetX;
  *OffsetY   = mLogos[Current].OffsetY;
  
  // Get image from HII database
  Status = mHiiImageEx->GetImageEx (mHiiImageEx, mHiiHandle, mLogos[Current].ImageId, Image);
  
  // After first logo is displayed, setup timing
  if (Current == 0 && !EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Logo displayed, setting up timing\n"));
    SetupLogoTiming ();
  }
  
  return Status;
}

EDKII_PLATFORM_LOGO_PROTOCOL  mPlatformLogo = {
  GetImage
};

**/
/**
  Cleanup function called when driver is unloaded.
**/
/**
EFI_STATUS
EFIAPI
LogoDriverUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
  // Close events if they exist
  if (mTimerEvent != NULL) {
    gBS->CloseEvent (mTimerEvent);
    mTimerEvent = NULL;
  }
  
  if (mKeyMonitorEvent != NULL) {
    gBS->CloseEvent (mKeyMonitorEvent);
    mKeyMonitorEvent = NULL;
  }
  
  if (mBiosSetupKeyEvent != NULL) {
    gBS->CloseEvent (mBiosSetupKeyEvent);
    mBiosSetupKeyEvent = NULL;
  }
  
  if (mMouseEvent != NULL) {
    gBS->CloseEvent (mMouseEvent);
    mMouseEvent = NULL;
  }
  
  return EFI_SUCCESS;
}

**/
/**
  Entrypoint of this module.
**/
/**

EFI_STATUS
EFIAPI
InitializeLogo (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                   Status;
  EFI_HII_PACKAGE_LIST_HEADER  *PackageList;
  EFI_HII_DATABASE_PROTOCOL    *HiiDatabase;
  EFI_HANDLE                   Handle;

  DEBUG ((DEBUG_INFO, "Logo Driver: Initializing with blue theme and static horizontal tabs...\n"));

  // Initialize global variables
  mInBiosSetup = FALSE;
  mSelectedMainTab = 0;
  mSelectedSubItem = 0;
  mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;
  mTotalMainTabs = ARRAY_SIZE (mMainTabs);

  // Locate HII Database Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiDatabaseProtocolGuid,
                  NULL,
                  (VOID **)&HiiDatabase
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Database Protocol: %r\n", Status));
    return Status;
  }

  // Locate HII Image Ex Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiImageExProtocolGuid,
                  NULL,
                  (VOID **)&mHiiImageEx
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Image Ex Protocol: %r\n", Status));
    return Status;
  }

  // Retrieve HII package list from ImageHandle
  Status = gBS->OpenProtocol (
                  ImageHandle,
                  &gEfiHiiPackageListProtocolGuid,
                  (VOID **)&PackageList,
                  ImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "HII Image Package with logo not found in PE/COFF resource section\n"));
    return Status;
  }

  // Publish HII package list to HII Database
  Status = HiiDatabase->NewPackageList (
                          HiiDatabase,
                          PackageList,
                          NULL,
                          &mHiiHandle
                          );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to add package list to HII Database: %r\n", Status));
    return Status;
  }

  // Install Platform Logo Protocol
  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEdkiiPlatformLogoProtocolGuid,
                  &mPlatformLogo,
                  NULL
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to install Platform Logo Protocol: %r\n", Status));
    HiiDatabase->RemovePackageList (HiiDatabase, mHiiHandle);
    return Status;
  }

  DEBUG ((DEBUG_INFO, "Logo Driver: Initialized successfully with blue theme static layout\n"));
  
  return EFI_SUCCESS;
}

**/




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




// Fixed BIOS setup utility with NO menu duplication but slide effect comes.

/** @file
  Logo DXE Driver with F12 key BIOS setup functionality.
  Shows logo, waits for F12 key, then displays BIOS setup form with navigation.
  Enhanced with blue theme, horizontal menu tabs and static three-section layout.

Copyright (c) 2016 - 2017, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent
**/
/**

#include <Uefi.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/HiiImageEx.h>
#include <Protocol/PlatformLogo.h>
#include <Protocol/HiiPackageList.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/SimpleTextOut.h>
#include <Protocol/SimplePointer.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>
#include <Library/MemoryAllocationLib.h>

typedef struct {
  EFI_IMAGE_ID                             ImageId;
  EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE    Attribute;
  INTN                                     OffsetX;
  INTN                                     OffsetY;
} LOGO_ENTRY;

// Menu item structure for horizontal tabs
typedef struct {
  CHAR16  *MenuText;
  CHAR16  *Description;
  UINT32  StartCol;    // Column where menu tab starts
  UINT32  EndCol;      // Column where menu tab ends
  UINT32  TabWidth;    // Width of the tab
} MENU_TAB;

// Submenu item structure
typedef struct {
  CHAR16  *MenuText;
  CHAR16  *Description;
  UINT32  StartRow;    // Row where menu item starts
  UINT32  StartCol;    // Column where menu item starts  
  UINT32  EndRow;      // Row where menu item ends
  UINT32  EndCol;      // Column where menu item ends
} SUBMENU_ITEM;

// Mouse state structure
typedef struct {
  BOOLEAN  LeftButtonPressed;
  BOOLEAN  LeftButtonReleased;
  BOOLEAN  WasLeftButtonPressed;
  UINT32   ClickCount;
  UINT64   LastClickTime;
  INT32    LastX;
  INT32    LastY;
  BOOLEAN  Visible;
} MOUSE_STATE;

// Menu level enumeration
typedef enum {
  MENU_LEVEL_MAIN_MENU = 0,
  MENU_LEVEL_MAIN_SUBMENU,
  MENU_LEVEL_SECURITY_SUBMENU,
  MENU_LEVEL_ADVANCED_SUBMENU,
  MENU_LEVEL_UEFI_SUBMENU
} MENU_LEVEL;

// Global variables
EFI_HII_IMAGE_EX_PROTOCOL  *mHiiImageEx;
EFI_HII_HANDLE             mHiiHandle;
EFI_EVENT                  mKeyMonitorEvent;
EFI_EVENT                  mTimerEvent;
EFI_EVENT                  mBiosSetupKeyEvent;
EFI_EVENT                  mMouseEvent;
EFI_SIMPLE_POINTER_PROTOCOL *mSimplePointer = NULL;
BOOLEAN                    mLogoDisplayed = FALSE;
BOOLEAN                    mMessageShown = FALSE;
BOOLEAN                    mInBiosSetup = FALSE;
UINTN                      mTimerCounter = 0;
UINTN                      mSelectedMainTab = 0;
UINTN                      mSelectedSubItem = 0;
UINTN                      mTotalMainTabs = 0;
UINTN                      mTotalSubItems = 0;
MENU_LEVEL                 mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;
MOUSE_STATE                mMouseState;
UINTN                      mConsoleColumns = 100;  // Increased default size
UINTN                      mConsoleRows = 40;      // Increased default size

// Main horizontal tabs (fixed spacing and positioning)
MENU_TAB mMainTabs[] = {
  { L"Main", L"System Information and Basic Settings", 2, 17, 16 },
  { L"Advanced", L"Advanced Configuration Options", 18, 35, 18 },
  { L"Security", L"Security and password Settings", 36, 53, 18 },
  { L"Save & Exit", L"Save changes and exit setup", 54, 75, 22 }
};

// Main submenu items (for Main tab)
SUBMENU_ITEM mMainSubmenu[] = {
  { L"BIOS Information", L"View BIOS version and system information", 5, 5, 5, 35 },
  { L"BIOS Eventlog", L"Display BIOS event log entries", 6, 5, 6, 35 },
  { L"Update system BIOS", L"Update system BIOS firmware", 7, 5, 7, 35 },
  { L"Change Date & Time", L"Configure system date and time", 8, 5, 8, 35 },
};

// Advanced submenu items
SUBMENU_ITEM mAdvancedSubmenu[] = {
  { L"Boot Options", L"Configure boot device priority and settings", 5, 5, 5, 35 },
  { L"Port Options", L"Configure USB, SATA, and other port settings", 6, 5, 6, 35 },
  { L"System Options", L"Configure CPU, memory, and system settings", 7, 5, 7, 35 },
  { L"Power Management", L"Configure power and thermal settings", 8, 5, 8, 35 },
};

// Security submenu items
SUBMENU_ITEM mSecuritySubmenu[] = {
  { L"Administrator Tools", L"Administrative security management tools", 5, 5, 5, 35 },
  { L"Create BIOS Password", L"Set BIOS administrator password protection", 6, 5, 6, 35 },
  { L"TPM Security", L"Configure Trusted Platform Module settings", 7, 5, 7, 35 },
  { L"Secure Boot", L"Enable or disable UEFI Secure Boot", 8, 5, 8, 35 },
};

// Save & Exit submenu items
SUBMENU_ITEM mSaveExitSubmenu[] = {
  { L"Save Changes and Exit", L"Save current settings and exit setup", 5, 5, 5, 35 },
  { L"Discard Changes and Exit", L"Exit setup without saving any changes", 6, 5, 6, 35 },
  { L"Save Changes", L"Save current settings and continue", 7, 5, 7, 35 },
  { L"Discard Changes", L"Reset to previous saved settings", 8, 5, 8, 35 },
  { L"Load Setup Defaults", L"Load factory default settings", 9, 5, 9, 35 }
};

// Function declarations
VOID DisplayBiosSetupForm (VOID);
VOID GetCurrentTime (OUT CHAR16 *TimeString);
VOID HandleBiosSetupNavigation (VOID);
VOID DrawBlueBackground (VOID);
VOID DrawHorizontalTabs (VOID);
VOID DrawThreeSectionLayout (VOID);
VOID DisplayCurrentSubmenu (VOID);
VOID DisplaySelectedTabInfo (VOID);
VOID DisplayNavigationHelp (VOID);
EFI_STATUS InitializeMouse (VOID);
VOID UpdateMouseState (VOID);
UINTN GetTabFromMousePosition (IN INT32 MouseX, IN INT32 MouseY);
UINTN GetSubItemFromMousePosition (IN INT32 MouseX, IN INT32 MouseY);
VOID HandleTabSelection (VOID);
VOID HandleSubItemSelection (VOID);
SUBMENU_ITEM* GetCurrentSubmenuArray (OUT UINTN *ItemCount);
VOID DrawMouseCursor (VOID);
VOID GetConsoleSize (VOID);
VOID CompleteInterfaceRedraw (VOID);  // New function to avoid duplication

LOGO_ENTRY mLogos[] = {
  {
    IMAGE_TOKEN (IMG_LOGO),
    EdkiiPlatformLogoDisplayAttributeCenter,
    0,
    0
  }
};

**/

/**
  Get console size and set to larger size for full screen display.
**/

/**
VOID
GetConsoleSize (
  VOID
  )
{
  EFI_STATUS Status;
  UINTN MaxMode, MaxColumns, MaxRows;
  UINTN BestMode = 0;
  UINTN BestColumns = 80;
  UINTN BestRows = 25;
  
  // Try to find the largest available mode
  MaxMode = gST->ConOut->Mode->MaxMode;
  
  for (UINTN Mode = 0; Mode < MaxMode; Mode++) {
    Status = gST->ConOut->QueryMode (
                            gST->ConOut,
                            Mode,
                            &MaxColumns,
                            &MaxRows
                            );
    
    if (!EFI_ERROR (Status)) {
      // Look for a larger mode that would give us better screen coverage
      if ((MaxColumns >= BestColumns && MaxRows >= BestRows) && 
          (MaxColumns > BestColumns || MaxRows > BestRows)) {
        BestMode = Mode;
        BestColumns = MaxColumns;
        BestRows = MaxRows;
      }
    }
  }
  
  // Set the best mode we found
  if (BestMode != gST->ConOut->Mode->Mode) {
    Status = gST->ConOut->SetMode (gST->ConOut, BestMode);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_WARN, "Failed to set console mode %d: %r\n", BestMode, Status));
    }
  }
  
  // Query the actual current mode
  Status = gST->ConOut->QueryMode (
                          gST->ConOut,
                          gST->ConOut->Mode->Mode,
                          &mConsoleColumns,
                          &mConsoleRows
                          );
  
  if (EFI_ERROR (Status)) {
    // Use larger default values
    mConsoleColumns = 100;
    mConsoleRows = 40;
  }
  
  DEBUG ((DEBUG_INFO, "Console size: %dx%d\n", mConsoleColumns, mConsoleRows));
}

**/

/**
  Get current submenu array based on selected main tab.

  @param ItemCount   Output parameter for number of items in submenu.

  @retval Pointer to current submenu array.
**/

/**
SUBMENU_ITEM*
GetCurrentSubmenuArray (
  OUT UINTN *ItemCount
  )
{
  switch (mSelectedMainTab) {
    case 0: // Main
      *ItemCount = ARRAY_SIZE (mMainSubmenu);
      return mMainSubmenu;
      
    case 1: // Advanced
      *ItemCount = ARRAY_SIZE (mAdvancedSubmenu);
      return mAdvancedSubmenu;
      
    case 2: // Security
      *ItemCount = ARRAY_SIZE (mSecuritySubmenu);
      return mSecuritySubmenu;
      
    case 3: // Save & Exit
      *ItemCount = ARRAY_SIZE (mSaveExitSubmenu);
      return mSaveExitSubmenu;
      
    default:
      *ItemCount = ARRAY_SIZE (mMainSubmenu);
      return mMainSubmenu;
  }
}

**/

/**
  Initialize mouse support by locating Simple Pointer Protocol.

  @retval EFI_SUCCESS   Mouse initialized successfully.
  @retval Others        Failed to initialize mouse.
**/

/**
EFI_STATUS
InitializeMouse (
  VOID
  )
{
  EFI_STATUS Status;
  
  // Initialize mouse state
  mMouseState.LeftButtonPressed = FALSE;
  mMouseState.LeftButtonReleased = FALSE;
  mMouseState.WasLeftButtonPressed = FALSE;
  mMouseState.ClickCount = 0;
  mMouseState.LastClickTime = 0;
  mMouseState.LastX = (INT32)(mConsoleColumns / 2); // Center of screen
  mMouseState.LastY = (INT32)(mConsoleRows / 2);
  mMouseState.Visible = TRUE;
  
  // Try to locate Simple Pointer Protocol
  Status = gBS->LocateProtocol (
                  &gEfiSimplePointerProtocolGuid,
                  NULL,
                  (VOID **)&mSimplePointer
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "Simple Pointer Protocol not available: %r\n", Status));
    mSimplePointer = NULL;
    return Status;
  }
  
  // Reset mouse
  Status = mSimplePointer->Reset (mSimplePointer, TRUE);
  if (EFI_ERROR (Status)) {
    Status = mSimplePointer->Reset (mSimplePointer, FALSE);
    if (EFI_ERROR (Status)) {
      mSimplePointer = NULL;
      return Status;
    }
  }
  
  DEBUG ((DEBUG_INFO, "Mouse initialized successfully\n"));
  return EFI_SUCCESS;
}

**/

/**
  Update mouse state by reading current mouse input.
**/

/**
VOID
UpdateMouseState (
  VOID
  )
{
  EFI_STATUS                Status;
  EFI_SIMPLE_POINTER_STATE  MouseState;
  
  if (mSimplePointer == NULL) {
    return;
  }
  
  Status = mSimplePointer->GetState (mSimplePointer, &MouseState);
  if (EFI_ERROR (Status)) {
    return;
  }
  
  // Handle relative movement
  INT32 DeltaX = MouseState.RelativeMovementX / 65536;
  INT32 DeltaY = MouseState.RelativeMovementY / 65536;
  
  if (DeltaX != 0 || DeltaY != 0) {
    mMouseState.LastX += DeltaX;
    mMouseState.LastY += DeltaY;
    
    // Boundary check
    if (mMouseState.LastX < 0) mMouseState.LastX = 0;
    if (mMouseState.LastY < 0) mMouseState.LastY = 0;
    if (mMouseState.LastX >= (INT32)mConsoleColumns) mMouseState.LastX = (INT32)mConsoleColumns - 1;
    if (mMouseState.LastY >= (INT32)mConsoleRows) mMouseState.LastY = (INT32)mConsoleRows - 1;
  }
  
  // Update button state
  mMouseState.WasLeftButtonPressed = mMouseState.LeftButtonPressed;
  mMouseState.LeftButtonPressed = MouseState.LeftButton;
  
  // Detect button release (click completion)
  if (mMouseState.WasLeftButtonPressed && !mMouseState.LeftButtonPressed) {
    mMouseState.LeftButtonReleased = TRUE;
    mMouseState.ClickCount++;
  } else {
    mMouseState.LeftButtonReleased = FALSE;
  }
}

**/
/**
  Draw mouse cursor at current position.
**/

/**
VOID
DrawMouseCursor (
  VOID
  )
{
  if (!mMouseState.Visible || mSimplePointer == NULL) {
    return;
  }
  
  gST->ConOut->SetCursorPosition (gST->ConOut, mMouseState.LastX, mMouseState.LastY);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
  Print (L"▓");
}
**/


/**
  Get tab index based on mouse position.

  @param MouseX   Mouse X coordinate.
  @param MouseY   Mouse Y coordinate.

  @retval Index   Tab index, or -1 if not over any tab.
**/

/**
UINTN
GetTabFromMousePosition (
  IN INT32 MouseX,
  IN INT32 MouseY
  )
{
  UINTN Index;
  
  // Check if mouse is on tab row (row 2)
  if (MouseY != 2) {
    return (UINTN)-1;
  }
  
  for (Index = 0; Index < mTotalMainTabs; Index++) {
    if (MouseX >= mMainTabs[Index].StartCol && MouseX <= mMainTabs[Index].EndCol) {
      return Index;
    }
  }
  
  return (UINTN)-1;
}

**/

/**
  Get submenu item index based on mouse position.

  @param MouseX   Mouse X coordinate.
  @param MouseY   Mouse Y coordinate.

  @retval Index   Submenu item index, or -1 if not over any item.
**/

/**
UINTN
GetSubItemFromMousePosition (
  IN INT32 MouseX,
  IN INT32 MouseY
  )
{
  UINTN Index;
  SUBMENU_ITEM *CurrentSubmenu;
  UINTN ItemCount;
  
  CurrentSubmenu = GetCurrentSubmenuArray (&ItemCount);
  
  for (Index = 0; Index < ItemCount; Index++) {
    if (MouseY == CurrentSubmenu[Index].StartRow + 1 && // Add offset for header rows
        MouseX >= CurrentSubmenu[Index].StartCol &&
        MouseX <= CurrentSubmenu[Index].EndCol) {
      return Index;
    }
  }
  
  return (UINTN)-1;
}

**/

/**
  Get current time and format it as string.

  @param TimeString   Output buffer for formatted time string.
**/

/**
VOID
GetCurrentTime (
  OUT CHAR16  *TimeString
  )
{
  EFI_TIME    Time;
  EFI_STATUS  Status;
  
  Status = gRT->GetTime (&Time, NULL);
  if (!EFI_ERROR (Status)) {
    UnicodeSPrint (
      TimeString, 
      50 * sizeof(CHAR16),
      L"%02d/%02d/%d  %02d:%02d:%02d",
      Time.Day,
      Time.Month,
      Time.Year,
      Time.Hour,
      Time.Minute,
      Time.Second
    );
  } else {
    UnicodeSPrint (TimeString, 50 * sizeof(CHAR16), L"[Date/Time Not Available]");
  }
}

**/

/**
  Draw blue background with proper headers - NO DUPLICATION.
**/

/**
VOID
DrawBlueBackground (
  VOID
  )
{
  UINTN Index, Col;
  
  // Get actual console size and try to maximize it
  GetConsoleSize ();
  
  // Clear screen completely first
  gST->ConOut->ClearScreen (gST->ConOut);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  
  // Fill entire screen with blue background - ONE TIME ONLY
  for (Index = 0; Index < mConsoleRows; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Index);
    for (Col = 0; Col < mConsoleColumns; Col++) {
      Print (L" ");
    }
  }
  
  // Draw BIOS Setup Utility in top-left corner - ONE TIME ONLY
  gST->ConOut->SetCursorPosition (gST->ConOut, 1, 0);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  Print (L"BIOS Setup Utility");
  
  // Draw CDAC BANGALORE title below center - ONE TIME ONLY
  gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 14) / 2, 1);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
  Print (L"CDAC BANGALORE");
}

**/

/**
  Draw horizontal tabs with COMPLETE clearing to prevent duplication.
**/

/**
VOID
DrawHorizontalTabs (
  VOID
  )
{
  UINTN Index, Col;
  
  // STEP 1: COMPLETELY clear the tab row (row 2) with blue background
  gST->ConOut->SetCursorPosition (gST->ConOut, 0, 2);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  for (Col = 0; Col < mConsoleColumns; Col++) {
    Print (L" ");
  }
  
  // STEP 2: Draw each tab EXACTLY ONCE at correct positions
  for (Index = 0; Index < mTotalMainTabs; Index++) {
    // Position cursor at the exact start of this tab
    gST->ConOut->SetCursorPosition (gST->ConOut, mMainTabs[Index].StartCol, 2);
    
    // Set colors based on selection
    if (Index == mSelectedMainTab) {
      // Selected tab - light gray background with black text
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    } else {
      // Unselected tab - blue background with white text
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
    }
    
    // Print the tab text with padding ONCE ONLY
    Print (L" %s ", mMainTabs[Index].MenuText);
  }
}

**/

/**
  Draw the three-section layout with proper boundaries.
**/

/**
VOID
DrawThreeSectionLayout (
  VOID
  )
{
  UINTN Row, Col;
  UINTN SeparatorCol = (mConsoleColumns * 3) / 4; // 3/4 position for separator
  
  // Clear and fill the main content area (rows 3 to second-to-last)
  for (Row = 3; Row < mConsoleRows - 2; Row++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Row);
    
    // Left section (submenu area) - light gray background - 3/4 of screen
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    for (Col = 0; Col < SeparatorCol; Col++) {
      Print (L" ");
    }
    
    // Vertical separator
    if (SeparatorCol < mConsoleColumns) {
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"│");
    }
    
    // Right section (info area) - light gray background - 1/4 of screen
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    for (Col = SeparatorCol + 1; Col < mConsoleColumns; Col++) {
      Print (L" ");
    }
  }
}

**/

/**
  Display current submenu - COMPLETELY CLEAR FIRST to prevent duplication.
**/

/**
VOID
DisplayCurrentSubmenu (
  VOID
  )
{
  UINTN Index, Col;
  SUBMENU_ITEM *CurrentSubmenu;
  UINTN ItemCount;
  UINTN SeparatorCol = (mConsoleColumns * 3) / 4;
  
  CurrentSubmenu = GetCurrentSubmenuArray (&ItemCount);
  mTotalSubItems = ItemCount;
  
  // STEP 1: COMPLETELY clear the left section area to remove ALL previous content
  for (Index = 4; Index < mConsoleRows - 3; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 1, Index);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    // Clear the entire left section width (3/4 of screen)
    for (Col = 1; Col < SeparatorCol - 1; Col++) {
      Print (L" ");
    }
  }
  
  // STEP 2: Display submenu items EXACTLY ONCE - no duplication
  for (Index = 0; Index < ItemCount && Index < 15; Index++) {
    // Position cursor for this menu item - each item gets its own row
    gST->ConOut->SetCursorPosition (gST->ConOut, 3, Index + 5);
    
    // Set colors based on selection
    if (Index == mSelectedSubItem) {
      // Highlight selected item with blue background
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
    } else {
      // Normal item with light gray background
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    }
    
    // Print menu text EXACTLY ONCE - NO DUPLICATION
    Print (L"%s", CurrentSubmenu[Index].MenuText);
  }
}

**/

/**
  Display selected item information in right section - CLEAR FIRST.
**/

/**
VOID
DisplaySelectedTabInfo (
  VOID
  )
{
  SUBMENU_ITEM *CurrentSubmenu;
  UINTN ItemCount;
  CHAR16 TimeString[100];
  UINTN SeparatorCol = (mConsoleColumns * 3) / 4;
  UINTN RightStart = SeparatorCol + 2;
  UINTN Index, Col;
  
  CurrentSubmenu = GetCurrentSubmenuArray (&ItemCount);
  
  // STEP 1: COMPLETELY clear right section first (1/4 of screen)
  for (Index = 4; Index < mConsoleRows - 3; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, Index);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    for (Col = RightStart; Col < mConsoleColumns - 1; Col++) {
      Print (L" ");
    }
  }
  
  // STEP 2: Display information header
  gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 5);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"Selected Item:");
  
  // Display based on current tab and selected submenu item
  if (mSelectedSubItem < ItemCount) {
    // Show selected item name
    gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 7);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_DARKGRAY | EFI_BACKGROUND_LIGHTGRAY);
    Print (L"%s", CurrentSubmenu[mSelectedSubItem].MenuText);
    
    // Show description with proper multi-line wrapping
    gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 9);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    
    // Calculate available width for right section (1/4)
    UINTN RightWidth = mConsoleColumns - RightStart - 2;
    CHAR16 *Description = CurrentSubmenu[mSelectedSubItem].Description;
    UINTN DescLen = StrLen(Description);
    
    // Multi-line description display
    if (DescLen <= RightWidth) {
      // Single line - fits completely
      Print (L"%s", Description);
    } else {
      // Multi-line - need to wrap text properly
      UINTN CurrentPos = 0;
      UINTN CurrentLine = 9;
      
      while (CurrentPos < DescLen && CurrentLine < mConsoleRows - 6) {
        gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, CurrentLine);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
        
        UINTN CharsToShow = RightWidth;
        if (CurrentPos + CharsToShow > DescLen) {
          CharsToShow = DescLen - CurrentPos;
        }
        
        // Find last space for proper word wrapping
        if (CurrentPos + CharsToShow < DescLen) {
          UINTN LastSpace = CharsToShow;
          for (UINTN i = CharsToShow; i > 0; i--) {
            if (Description[CurrentPos + i - 1] == L' ') {
              LastSpace = i;
              break;
            }
          }
          if (LastSpace < CharsToShow) {
            CharsToShow = LastSpace;
          }
        }
        
        // Print this line portion
        for (UINTN i = 0; i < CharsToShow && CurrentPos < DescLen; i++) {
          Print (L"%c", Description[CurrentPos]);
          CurrentPos++;
        }
        
        // Skip space if we broke at a space
        if (CurrentPos < DescLen && Description[CurrentPos] == L' ') {
          CurrentPos++;
        }
        
        CurrentLine++;
      }
    }
    
    // For Main tab's Change Date & Time item, show additional time info
    if (mSelectedMainTab == 0 && mSelectedSubItem == 3) {
      GetCurrentTime (TimeString);
      gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 12);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"Current:");
      gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 13);
      
      // Time display with wrapping if needed
      UINTN TimeLen = StrLen(TimeString);
      if (TimeLen <= RightWidth) {
        Print (L"%s", TimeString);
      } else {
        Print (L"%.15s", TimeString);
        gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 14);
        Print (L"%s", &TimeString[15]);
      }
      
      gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 16);
      Print (L"Use +/- to");
      gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 17);
      Print (L"change values");
    }
  }
  
  // Show current tab name at bottom of right section
  gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, mConsoleRows - 5);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"Tab: %s", mMainTabs[mSelectedMainTab].MenuText);
}

**/

/**
  Display navigation help in bottom section.
**/

/**
VOID
DisplayNavigationHelp (
  VOID
  )
{
  UINTN Col;
  
  // Bottom blue bars with navigation help
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  
  // Fill bottom two rows with blue background
  for (UINTN Row = mConsoleRows - 2; Row < mConsoleRows; Row++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Row);
    for (Col = 0; Col < mConsoleColumns; Col++) {
      Print (L" ");
    }
  }
  
  // Display navigation instructions on bottom row
  gST->ConOut->SetCursorPosition (gST->ConOut, 2, mConsoleRows - 1);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  Print (L"←→: Select Screen  ↑↓: Select Item  Enter: Select  ESC: Exit");
}
**/


/**
  Complete interface redraw - PREVENTS ALL DUPLICATION.
**/

/**
VOID
CompleteInterfaceRedraw (
  VOID
  )
{
  // Draw the complete BIOS setup interface step by step
  DrawBlueBackground ();       // Includes title headers - clears everything first
  DrawHorizontalTabs ();       // Draw tabs with complete clearing first
  DrawThreeSectionLayout ();   // Draw the static layout
  DisplayCurrentSubmenu ();    // Display submenu items with complete clearing first
  DisplaySelectedTabInfo ();   // Display item info with complete clearing first
  DisplayNavigationHelp ();    // Bottom navigation help
  
  // Draw mouse cursor
  DrawMouseCursor ();
}

**/

/**
  Display BIOS setup form with NO duplication.
**/

/**
VOID
DisplayBiosSetupForm (
  VOID
  )
{
  mSelectedMainTab = 0;
  mSelectedSubItem = 0;
  mTotalMainTabs = ARRAY_SIZE (mMainTabs);
  mInBiosSetup = TRUE;
  
  // Initialize mouse support
  InitializeMouse ();
  
  // Use complete interface redraw to prevent any duplication
  CompleteInterfaceRedraw ();
  
  // Reset console input for clean key detection
  gST->ConIn->Reset (gST->ConIn, FALSE);
  
  // Start navigation handling
  HandleBiosSetupNavigation ();
}

**/

/**
  Handle tab selection based on current selected tab.
**/

/**
VOID
HandleTabSelection (
  VOID
  )
{
  UINTN SeparatorCol = (mConsoleColumns * 3) / 4;
  UINTN RightStart = SeparatorCol + 2;
  
  switch (mSelectedMainTab) {
    case 3: // Save & Exit
      if (mSelectedSubItem == 0) { // Save Changes and Exit
        gST->ConOut->ClearScreen (gST->ConOut);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
        gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 35) / 2, mConsoleRows / 2);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_GREEN | EFI_BACKGROUND_BLACK);
        Print (L"✓ Settings saved. Shutting down system...");
        gBS->Stall (2000000);
        gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
        mInBiosSetup = FALSE;
      } else if (mSelectedSubItem == 1) { // Discard Changes and Exit
        gST->ConOut->ClearScreen (gST->ConOut);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
        gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 40) / 2, mConsoleRows / 2);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_RED | EFI_BACKGROUND_BLACK);
        Print (L"✗ Exiting without saving. Shutting down...");
        gBS->Stall (2000000);
        gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
        mInBiosSetup = FALSE;
      } else if (mSelectedSubItem == 2) { // Save Changes
        // Show save confirmation in right panel
        gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_GREEN | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"✓ Saved");
        gBS->Stall (1500000);
        // Clear the message
        gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"        ");
      } else if (mSelectedSubItem == 3) { // Discard Changes
        // Show discard confirmation in right panel
        gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_RED | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"✗ Discarded");
        gBS->Stall (1500000);
        // Clear the message
        gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"           ");
      } else if (mSelectedSubItem == 4) { // Load Setup Defaults
        // Show defaults loaded confirmation in right panel
        gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"⚡ Defaults");
        gBS->Stall (1500000);
        // Clear the message
        gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"          ");
      }
      break;
      
    default:
      // For other tabs, show a brief action message in right panel
      gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 15);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"⚡ Action...");
      gBS->Stall (800000);
      // Clear the message
      gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 15);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"           ");
      break;
  }
}

**/

/**
  Handle navigation within BIOS setup - NO DUPLICATION.
**/

/**
VOID
HandleBiosSetupNavigation (
  VOID
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  UINTN          MouseTabIndex, MouseSubIndex;
  
  while (mInBiosSetup) {
    // Update mouse state
    UpdateMouseState ();
    
    // Redraw mouse cursor
    DrawMouseCursor ();
    
    // Handle mouse input
    if (mSimplePointer != NULL) {
      MouseTabIndex = GetTabFromMousePosition (mMouseState.LastX, mMouseState.LastY);
      MouseSubIndex = GetSubItemFromMousePosition (mMouseState.LastX, mMouseState.LastY);
      
      // Handle mouse clicks on tabs
      if (mMouseState.LeftButtonReleased && MouseTabIndex != (UINTN)-1) {
        if (MouseTabIndex != mSelectedMainTab) {
          mSelectedMainTab = MouseTabIndex;
          mSelectedSubItem = 0; // Reset submenu selection
          
          // Use complete interface redraw to prevent duplication
          CompleteInterfaceRedraw ();
        }
        mMouseState.LeftButtonReleased = FALSE;
      }
      
      // Handle mouse clicks on submenu items
      if (mMouseState.LeftButtonReleased && MouseSubIndex != (UINTN)-1) {
        if (MouseSubIndex != mSelectedSubItem) {
          mSelectedSubItem = MouseSubIndex;
          
          // Redraw submenu and info only (with proper clearing)
          DisplayCurrentSubmenu ();
          DisplaySelectedTabInfo ();
        } else {
          // Double-click simulation - execute selection
          HandleTabSelection ();
          if (!mInBiosSetup) return;
        }
        mMouseState.LeftButtonReleased = FALSE;
      }
    }
    
    // Handle keyboard input
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
    
    if (!EFI_ERROR (Status)) {
      switch (Key.ScanCode) {
        case SCAN_LEFT:
          // Move left in main tabs
          if (mSelectedMainTab > 0) {
            mSelectedMainTab--;
          } else {
            mSelectedMainTab = mTotalMainTabs - 1; // Wrap to rightmost
          }
          mSelectedSubItem = 0; // Reset submenu selection
          
          // Use complete interface redraw to prevent duplication
          CompleteInterfaceRedraw ();
          break;
          
        case SCAN_RIGHT:
          // Move right in main tabs
          if (mSelectedMainTab < mTotalMainTabs - 1) {
            mSelectedMainTab++;
          } else {
            mSelectedMainTab = 0; // Wrap to leftmost
          }
          mSelectedSubItem = 0; // Reset submenu selection
          
          // Use complete interface redraw to prevent duplication
          CompleteInterfaceRedraw ();
          break;
          
        case SCAN_UP:
          // Move up in submenu
          if (mSelectedSubItem > 0) {
            mSelectedSubItem--;
          } else {
            mSelectedSubItem = mTotalSubItems - 1; // Wrap to bottom
          }
          
          // Update submenu and info only (with proper clearing)
          DisplayCurrentSubmenu ();
          DisplaySelectedTabInfo ();
          break;
          
        case SCAN_DOWN:
          // Move down in submenu
          if (mSelectedSubItem < mTotalSubItems - 1) {
            mSelectedSubItem++;
          } else {
            mSelectedSubItem = 0; // Wrap to top
          }
          
          // Update submenu and info only (with proper clearing)
          DisplayCurrentSubmenu ();
          DisplaySelectedTabInfo ();
          break;
          
        case SCAN_ESC:
          // Exit BIOS setup
          mInBiosSetup = FALSE;
          gST->ConOut->ClearScreen (gST->ConOut);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
          gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 40) / 2, mConsoleRows / 2);
          Print (L"🚪 Exiting BIOS Setup. Shutting down system...");
          gBS->Stall (2000000);
          gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
          return;
          
        default:
          // Check for ENTER key
          if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
            HandleTabSelection ();
            if (!mInBiosSetup) {
              return;
            }
          }
          break;
      }
    } else {
      // Small delay to prevent excessive CPU usage
      gBS->Stall (50000); // 50ms
    }
  }
}

**/

/**
  Monitor keyboard input for F12 key press only.
**/

/**
VOID
EFIAPI
KeyboardMonitor (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  
  // Only monitor after message is shown and not in BIOS setup
  if (!mMessageShown || mInBiosSetup) {
    return;
  }
  
  // Check if key is available
  Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
  
  if (!EFI_ERROR (Status)) {
    // Check for F12 key only
    if (Key.ScanCode == SCAN_F12) {
      DEBUG ((DEBUG_INFO, "F12 key pressed - showing BIOS setup form\n"));
      
      // Cancel timer and keyboard events
      gBS->SetTimer (mKeyMonitorEvent, TimerCancel, 0);
      gBS->SetTimer (mTimerEvent, TimerCancel, 0);
      
      // Display BIOS setup form
      DisplayBiosSetupForm ();
      
      // After exiting BIOS setup, shutdown
      gST->ConOut->ClearScreen (gST->ConOut);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
      gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 40) / 2, mConsoleRows / 2);
      Print (L"✅ BIOS Setup completed. Shutting down system...");
      gBS->Stall (2000000);
      gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
      return;
    }
  }
}

**/

/**
  Timer callback function to handle logo display timing.

  @param Event      The event that is signaled.
  @param Context    The context passed to the event.
**/

/**
VOID
EFIAPI
TimerCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  mTimerCounter++;
  
  // After 5 seconds, show the message below logo and wait for F12
  if (mTimerCounter == 50 && !mMessageShown) { // 50 * 100ms = 5 seconds
    // Clear the entire screen first
    gST->ConOut->ClearScreen (gST->ConOut);
    
    // Get console size for proper centering
    GetConsoleSize ();
    
    // Position cursor below the logo area (center of screen)
    gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 28) / 2, mConsoleRows / 2 + 5);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
    Print (L"Press F12 key for BIOS setup");
    
    mMessageShown = TRUE;
    
    // Reset console input for clean key detection
    gST->ConIn->Reset (gST->ConIn, FALSE);
    
    DEBUG ((DEBUG_INFO, "Message displayed, waiting for F12 key\n"));
  }
}

**/

/**
  Setup timing and keyboard monitoring after logo is displayed.
**/

/**
VOID
SetupLogoTiming (
  VOID
  )
{
  EFI_STATUS  Status;
  
  if (mLogoDisplayed) {
    return;
  }
  
  DEBUG ((DEBUG_INFO, "Setting up logo timing and keyboard monitoring\n"));
  
  // Create timer event for logo timing
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  TimerCallback,
                  NULL,
                  &mTimerEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    Status = gBS->SetTimer (
                    mTimerEvent,
                    TimerPeriodic,
                    1000000  // 100ms in 100ns units
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set timer for mTimerEvent: %r\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to create mTimerEvent: %r\n", Status));
  }
  
  // Create keyboard monitoring event
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  KeyboardMonitor,
                  NULL,
                  &mKeyMonitorEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    Status = gBS->SetTimer (
                    mKeyMonitorEvent,
                    TimerPeriodic,
                    500000  // 50ms in 100ns units
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set timer for mKeyMonitorEvent: %r\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to create mKeyMonitorEvent: %r\n", Status));
  }
  
  mLogoDisplayed = TRUE;
}

**/

/**
  Load a platform logo image and return its data and attributes.
**/

/**
EFI_STATUS
EFIAPI
GetImage (
  IN     EDKII_PLATFORM_LOGO_PROTOCOL        *This,
  IN OUT UINT32                              *Instance,
  OUT EFI_IMAGE_INPUT                        *Image,
  OUT EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE  *Attribute,
  OUT INTN                                   *OffsetX,
  OUT INTN                                   *OffsetY
  )
{
  UINT32      Current;
  EFI_STATUS  Status;

  if ((Instance == NULL) || (Image == NULL) ||
      (Attribute == NULL) || (OffsetX == NULL) || (OffsetY == NULL))
  {
    return EFI_INVALID_PARAMETER;
  }

  Current = *Instance;
  if (Current >= ARRAY_SIZE (mLogos)) {
    return EFI_NOT_FOUND;
  }

  (*Instance)++;
  *Attribute = mLogos[Current].Attribute;
  *OffsetX   = mLogos[Current].OffsetX;
  *OffsetY   = mLogos[Current].OffsetY;
  
  // Get image from HII database
  Status = mHiiImageEx->GetImageEx (mHiiImageEx, mHiiHandle, mLogos[Current].ImageId, Image);
  
  // After first logo is displayed, setup timing
  if (Current == 0 && !EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Logo displayed, setting up timing\n"));
    SetupLogoTiming ();
  }
  
  return Status;
}

EDKII_PLATFORM_LOGO_PROTOCOL  mPlatformLogo = {
  GetImage
};

**/

/**
  Cleanup function called when driver is unloaded.
**/

/**
EFI_STATUS
EFIAPI
LogoDriverUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
  // Close events if they exist
  if (mTimerEvent != NULL) {
    gBS->CloseEvent (mTimerEvent);
    mTimerEvent = NULL;
  }
  
  if (mKeyMonitorEvent != NULL) {
    gBS->CloseEvent (mKeyMonitorEvent);
    mKeyMonitorEvent = NULL;
  }
  
  if (mBiosSetupKeyEvent != NULL) {
    gBS->CloseEvent (mBiosSetupKeyEvent);
    mBiosSetupKeyEvent = NULL;
  }
  
  if (mMouseEvent != NULL) {
    gBS->CloseEvent (mMouseEvent);
    mMouseEvent = NULL;
  }
  
  return EFI_SUCCESS;
}

**/

/**
  Entrypoint of this module.
**/

/**
EFI_STATUS
EFIAPI
InitializeLogo (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                   Status;
  EFI_HII_PACKAGE_LIST_HEADER  *PackageList;
  EFI_HII_DATABASE_PROTOCOL    *HiiDatabase;
  EFI_HANDLE                   Handle;

  DEBUG ((DEBUG_INFO, "Logo Driver: Initializing with NO menu duplication...\n"));

  // Initialize global variables
  mInBiosSetup = FALSE;
  mSelectedMainTab = 0;
  mSelectedSubItem = 0;
  mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;
  mTotalMainTabs = ARRAY_SIZE (mMainTabs);

  // Locate HII Database Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiDatabaseProtocolGuid,
                  NULL,
                  (VOID **)&HiiDatabase
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Database Protocol: %r\n", Status));
    return Status;
  }

  // Locate HII Image Ex Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiImageExProtocolGuid,
                  NULL,
                  (VOID **)&mHiiImageEx
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Image Ex Protocol: %r\n", Status));
    return Status;
  }

  // Retrieve HII package list from ImageHandle
  Status = gBS->OpenProtocol (
                  ImageHandle,
                  &gEfiHiiPackageListProtocolGuid,
                  (VOID **)&PackageList,
                  ImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "HII Image Package with logo not found in PE/COFF resource section\n"));
    return Status;
  }

  // Publish HII package list to HII Database
  Status = HiiDatabase->NewPackageList (
                          HiiDatabase,
                          PackageList,
                          NULL,
                          &mHiiHandle
                          );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to add package list to HII Database: %r\n", Status));
    return Status;
  }

  // Install Platform Logo Protocol
  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEdkiiPlatformLogoProtocolGuid,
                  &mPlatformLogo,
                  NULL
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to install Platform Logo Protocol: %r\n", Status));
    HiiDatabase->RemovePackageList (HiiDatabase, mHiiHandle);
    return Status;
  }

  DEBUG ((DEBUG_INFO, "Logo Driver: Initialized successfully - NO DUPLICATION\n"));
  
  return EFI_SUCCESS;
}

**/


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





// Pure Static BIOS Setup - NO slide effect, NO duplication. this is perfectly work.

/** @file
  Logo DXE Driver with F12 key BIOS setup functionality.
  Shows logo, waits for F12 key, then displays BIOS setup form with navigation.
  Enhanced with blue theme, horizontal menu tabs and PURE STATIC layout.

Copyright (c) 2016 - 2017, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent
**/

/**

#include <Uefi.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/HiiImageEx.h>
#include <Protocol/PlatformLogo.h>
#include <Protocol/HiiPackageList.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/SimpleTextOut.h>
#include <Protocol/SimplePointer.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>
#include <Library/MemoryAllocationLib.h>

typedef struct {
  EFI_IMAGE_ID                             ImageId;
  EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE    Attribute;
  INTN                                     OffsetX;
  INTN                                     OffsetY;
} LOGO_ENTRY;

// Menu item structure for horizontal tabs
typedef struct {
  CHAR16  *MenuText;
  CHAR16  *Description;
  UINT32  StartCol;    // Column where menu tab starts
  UINT32  EndCol;      // Column where menu tab ends
  UINT32  TabWidth;    // Width of the tab
} MENU_TAB;

// Submenu item structure
typedef struct {
  CHAR16  *MenuText;
  CHAR16  *Description;
  UINT32  StartRow;    // Row where menu item starts
  UINT32  StartCol;    // Column where menu item starts  
  UINT32  EndRow;      // Row where menu item ends
  UINT32  EndCol;      // Column where menu item ends
} SUBMENU_ITEM;

// Mouse state structure
typedef struct {
  BOOLEAN  LeftButtonPressed;
  BOOLEAN  LeftButtonReleased;
  BOOLEAN  WasLeftButtonPressed;
  UINT32   ClickCount;
  UINT64   LastClickTime;
  INT32    LastX;
  INT32    LastY;
  BOOLEAN  Visible;
} MOUSE_STATE;

// Menu level enumeration
typedef enum {
  MENU_LEVEL_MAIN_MENU = 0,
  MENU_LEVEL_MAIN_SUBMENU,
  MENU_LEVEL_SECURITY_SUBMENU,
  MENU_LEVEL_ADVANCED_SUBMENU,
  MENU_LEVEL_UEFI_SUBMENU
} MENU_LEVEL;

// Global variables
EFI_HII_IMAGE_EX_PROTOCOL  *mHiiImageEx;
EFI_HII_HANDLE             mHiiHandle;
EFI_EVENT                  mKeyMonitorEvent;
EFI_EVENT                  mTimerEvent;
EFI_EVENT                  mBiosSetupKeyEvent;
EFI_EVENT                  mMouseEvent;
EFI_SIMPLE_POINTER_PROTOCOL *mSimplePointer = NULL;
BOOLEAN                    mLogoDisplayed = FALSE;
BOOLEAN                    mMessageShown = FALSE;
BOOLEAN                    mInBiosSetup = FALSE;
UINTN                      mTimerCounter = 0;
UINTN                      mSelectedMainTab = 0;
UINTN                      mSelectedSubItem = 0;
UINTN                      mTotalMainTabs = 0;
UINTN                      mTotalSubItems = 0;
MENU_LEVEL                 mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;
MOUSE_STATE                mMouseState;
UINTN                      mConsoleColumns = 100;
UINTN                      mConsoleRows = 40;

// STATIC flag to prevent background redraw
BOOLEAN                    mInterfaceInitialized = FALSE;

// Main horizontal tabs (fixed spacing and positioning)
MENU_TAB mMainTabs[] = {
  { L"Main", L"System Information and Basic Settings", 2, 17, 16 },
  { L"Advanced", L"Advanced Configuration Options", 18, 35, 18 },
  { L"Security", L"Security and password Settings", 36, 53, 18 },
  { L"Save & Exit", L"Save changes and exit setup", 54, 75, 22 }
};

// Main submenu items (for Main tab)
SUBMENU_ITEM mMainSubmenu[] = {
  { L"BIOS Information", L"View BIOS version and system information", 5, 5, 5, 35 },
  { L"BIOS Eventlog", L"Display BIOS event log entries", 6, 5, 6, 35 },
  { L"Update system BIOS", L"Update system BIOS firmware", 7, 5, 7, 35 },
  { L"Change Date & Time", L"Configure system date and time", 8, 5, 8, 35 },
};

// Advanced submenu items
SUBMENU_ITEM mAdvancedSubmenu[] = {
  { L"Boot Options", L"Configure boot device priority and settings", 5, 5, 5, 35 },
  { L"Port Options", L"Configure USB, SATA, and other port settings", 6, 5, 6, 35 },
  { L"System Options", L"Configure CPU, memory, and system settings", 7, 5, 7, 35 },
  { L"Power Management", L"Configure power and thermal settings", 8, 5, 8, 35 },
};

// Security submenu items
SUBMENU_ITEM mSecuritySubmenu[] = {
  { L"Administrator Tools", L"Administrative security management tools", 5, 5, 5, 35 },
  { L"Create BIOS Password", L"Set BIOS administrator password protection", 6, 5, 6, 35 },
  { L"TPM Security", L"Configure Trusted Platform Module settings", 7, 5, 7, 35 },
  { L"Secure Boot", L"Enable or disable UEFI Secure Boot", 8, 5, 8, 35 },
};

// Save & Exit submenu items
SUBMENU_ITEM mSaveExitSubmenu[] = {
  { L"Save Changes and Exit", L"Save current settings and exit setup", 5, 5, 5, 35 },
  { L"Discard Changes and Exit", L"Exit setup without saving any changes", 6, 5, 6, 35 },
  { L"Save Changes", L"Save current settings and continue", 7, 5, 7, 35 },
  { L"Discard Changes", L"Reset to previous saved settings", 8, 5, 8, 35 },
  { L"Load Setup Defaults", L"Load factory default settings", 9, 5, 9, 35 }
};

// Function declarations
VOID DisplayBiosSetupForm (VOID);
VOID GetCurrentTime (OUT CHAR16 *TimeString);
VOID HandleBiosSetupNavigation (VOID);
VOID DrawBlueBackgroundOnce (VOID);  // One-time background setup
VOID UpdateTabsOnly (VOID);          // Only update tab selection colors
VOID UpdateSubmenuOnly (VOID);       // Only update submenu selection
VOID UpdateInfoPanelOnly (VOID);     // Only update info panel
VOID DrawInitialInterface (VOID);    // Initial complete setup
EFI_STATUS InitializeMouse (VOID);
VOID UpdateMouseState (VOID);
UINTN GetTabFromMousePosition (IN INT32 MouseX, IN INT32 MouseY);
UINTN GetSubItemFromMousePosition (IN INT32 MouseX, IN INT32 MouseY);
VOID HandleTabSelection (VOID);
VOID HandleSubItemSelection (VOID);
SUBMENU_ITEM* GetCurrentSubmenuArray (OUT UINTN *ItemCount);
VOID DrawMouseCursor (VOID);
VOID GetConsoleSize (VOID);

LOGO_ENTRY mLogos[] = {
  {
    IMAGE_TOKEN (IMG_LOGO),
    EdkiiPlatformLogoDisplayAttributeCenter,
    0,
    0
  }
};

**/

/**
  Get console size and set to larger size for full screen display.
**/

/**
VOID
GetConsoleSize (
  VOID
  )
{
  EFI_STATUS Status;
  UINTN MaxMode, MaxColumns, MaxRows;
  UINTN BestMode = 0;
  UINTN BestColumns = 80;
  UINTN BestRows = 25;
  
  // Try to find the largest available mode
  MaxMode = gST->ConOut->Mode->MaxMode;
  
  for (UINTN Mode = 0; Mode < MaxMode; Mode++) {
    Status = gST->ConOut->QueryMode (
                            gST->ConOut,
                            Mode,
                            &MaxColumns,
                            &MaxRows
                            );
    
    if (!EFI_ERROR (Status)) {
      if ((MaxColumns >= BestColumns && MaxRows >= BestRows) && 
          (MaxColumns > BestColumns || MaxRows > BestRows)) {
        BestMode = Mode;
        BestColumns = MaxColumns;
        BestRows = MaxRows;
      }
    }
  }
  
  // Set the best mode we found
  if (BestMode != gST->ConOut->Mode->Mode) {
    Status = gST->ConOut->SetMode (gST->ConOut, BestMode);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_WARN, "Failed to set console mode %d: %r\n", BestMode, Status));
    }
  }
  
  // Query the actual current mode
  Status = gST->ConOut->QueryMode (
                          gST->ConOut,
                          gST->ConOut->Mode->Mode,
                          &mConsoleColumns,
                          &mConsoleRows
                          );
  
  if (EFI_ERROR (Status)) {
    mConsoleColumns = 100;
    mConsoleRows = 40;
  }
  
  DEBUG ((DEBUG_INFO, "Console size: %dx%d\n", mConsoleColumns, mConsoleRows));
}

**/

/**
  Get current submenu array based on selected main tab.
**/
/**

SUBMENU_ITEM*
GetCurrentSubmenuArray (
  OUT UINTN *ItemCount
  )
{
  switch (mSelectedMainTab) {
    case 0: // Main
      *ItemCount = ARRAY_SIZE (mMainSubmenu);
      return mMainSubmenu;
      
    case 1: // Advanced
      *ItemCount = ARRAY_SIZE (mAdvancedSubmenu);
      return mAdvancedSubmenu;
      
    case 2: // Security
      *ItemCount = ARRAY_SIZE (mSecuritySubmenu);
      return mSecuritySubmenu;
      
    case 3: // Save & Exit
      *ItemCount = ARRAY_SIZE (mSaveExitSubmenu);
      return mSaveExitSubmenu;
      
    default:
      *ItemCount = ARRAY_SIZE (mMainSubmenu);
      return mMainSubmenu;
  }
}
**/


/**
  Initialize mouse support.
**/

/**
EFI_STATUS
InitializeMouse (
  VOID
  )
{
  EFI_STATUS Status;
  
  // Initialize mouse state
  mMouseState.LeftButtonPressed = FALSE;
  mMouseState.LeftButtonReleased = FALSE;
  mMouseState.WasLeftButtonPressed = FALSE;
  mMouseState.ClickCount = 0;
  mMouseState.LastClickTime = 0;
  mMouseState.LastX = (INT32)(mConsoleColumns / 2);
  mMouseState.LastY = (INT32)(mConsoleRows / 2);
  mMouseState.Visible = TRUE;
  
  // Try to locate Simple Pointer Protocol
  Status = gBS->LocateProtocol (
                  &gEfiSimplePointerProtocolGuid,
                  NULL,
                  (VOID **)&mSimplePointer
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "Simple Pointer Protocol not available: %r\n", Status));
    mSimplePointer = NULL;
    return Status;
  }
  
  // Reset mouse
  Status = mSimplePointer->Reset (mSimplePointer, TRUE);
  if (EFI_ERROR (Status)) {
    Status = mSimplePointer->Reset (mSimplePointer, FALSE);
    if (EFI_ERROR (Status)) {
      mSimplePointer = NULL;
      return Status;
    }
  }
  
  DEBUG ((DEBUG_INFO, "Mouse initialized successfully\n"));
  return EFI_SUCCESS;
}

**/

/**
  Update mouse state.
**/

/**
VOID
UpdateMouseState (
  VOID
  )
{
  EFI_STATUS                Status;
  EFI_SIMPLE_POINTER_STATE  MouseState;
  
  if (mSimplePointer == NULL) {
    return;
  }
  
  Status = mSimplePointer->GetState (mSimplePointer, &MouseState);
  if (EFI_ERROR (Status)) {
    return;
  }
  
  // Handle relative movement
  INT32 DeltaX = MouseState.RelativeMovementX / 65536;
  INT32 DeltaY = MouseState.RelativeMovementY / 65536;
  
  if (DeltaX != 0 || DeltaY != 0) {
    mMouseState.LastX += DeltaX;
    mMouseState.LastY += DeltaY;
    
    // Boundary check
    if (mMouseState.LastX < 0) mMouseState.LastX = 0;
    if (mMouseState.LastY < 0) mMouseState.LastY = 0;
    if (mMouseState.LastX >= (INT32)mConsoleColumns) mMouseState.LastX = (INT32)mConsoleColumns - 1;
    if (mMouseState.LastY >= (INT32)mConsoleRows) mMouseState.LastY = (INT32)mConsoleRows - 1;
  }
  
  // Update button state
  mMouseState.WasLeftButtonPressed = mMouseState.LeftButtonPressed;
  mMouseState.LeftButtonPressed = MouseState.LeftButton;
  
  // Detect button release (click completion)
  if (mMouseState.WasLeftButtonPressed && !mMouseState.LeftButtonPressed) {
    mMouseState.LeftButtonReleased = TRUE;
    mMouseState.ClickCount++;
  } else {
    mMouseState.LeftButtonReleased = FALSE;
  }
}

**/

/**
  Draw mouse cursor.
**/

/**
VOID
DrawMouseCursor (
  VOID
  )
{
  if (!mMouseState.Visible || mSimplePointer == NULL) {
    return;
  }
  
  gST->ConOut->SetCursorPosition (gST->ConOut, mMouseState.LastX, mMouseState.LastY);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
  Print (L"▓");
}
**/


/**
  Get tab index based on mouse position.
**/
/**

UINTN
GetTabFromMousePosition (
  IN INT32 MouseX,
  IN INT32 MouseY
  )
{
  UINTN Index;
  
  // Check if mouse is on tab row (row 2)
  if (MouseY != 2) {
    return (UINTN)-1;
  }
  
  for (Index = 0; Index < mTotalMainTabs; Index++) {
    if (MouseX >= mMainTabs[Index].StartCol && MouseX <= mMainTabs[Index].EndCol) {
      return Index;
    }
  }
  
  return (UINTN)-1;
}

**/

/**
  Get submenu item index based on mouse position.
**/
/**

UINTN
GetSubItemFromMousePosition (
  IN INT32 MouseX,
  IN INT32 MouseY
  )
{
  UINTN Index;
  SUBMENU_ITEM *CurrentSubmenu;
  UINTN ItemCount;
  
  CurrentSubmenu = GetCurrentSubmenuArray (&ItemCount);
  
  for (Index = 0; Index < ItemCount; Index++) {
    if (MouseY == CurrentSubmenu[Index].StartRow + 1 && 
        MouseX >= CurrentSubmenu[Index].StartCol &&
        MouseX <= CurrentSubmenu[Index].EndCol) {
      return Index;
    }
  }
  
  return (UINTN)-1;
}
**/


/**
  Get current time and format it as string.
**/
/**

VOID
GetCurrentTime (
  OUT CHAR16  *TimeString
  )
{
  EFI_TIME    Time;
  EFI_STATUS  Status;
  
  Status = gRT->GetTime (&Time, NULL);
  if (!EFI_ERROR (Status)) {
    UnicodeSPrint (
      TimeString, 
      50 * sizeof(CHAR16),
      L"%02d/%02d/%d  %02d:%02d:%02d",
      Time.Day,
      Time.Month,
      Time.Year,
      Time.Hour,
      Time.Minute,
      Time.Second
    );
  } else {
    UnicodeSPrint (TimeString, 50 * sizeof(CHAR16), L"[Date/Time Not Available]");
  }
}

**/

/**
  Draw blue background and headers - ONE TIME ONLY.
**/
/**

VOID
DrawBlueBackgroundOnce (
  VOID
  )
{
  UINTN Index, Col;
  
  // Get actual console size
  GetConsoleSize ();
  
  // Clear screen and set blue background - ONE TIME ONLY
  gST->ConOut->ClearScreen (gST->ConOut);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  
  // Fill entire screen with blue background - ONE TIME ONLY
  for (Index = 0; Index < mConsoleRows; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Index);
    for (Col = 0; Col < mConsoleColumns; Col++) {
      Print (L" ");
    }
  }
  
  // Draw BIOS Setup Utility in top-left corner - ONE TIME ONLY
  gST->ConOut->SetCursorPosition (gST->ConOut, 1, 0);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  Print (L"BIOS Setup Utility");
  
  // Draw CDAC BANGALORE title below center - ONE TIME ONLY
  gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 14) / 2, 1);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
//  Print (L"CDAC BANGALORE");
   Print (L"BHARAT BIOS CDAC ");
}
**/

/**
  Update ONLY tab colors - no background redraw.
**/

/**
VOID
UpdateTabsOnly (
  VOID
  )
{
  UINTN Index, Col;
  
  // Clear ONLY the tab row (row 2) - MINIMAL clearing
  gST->ConOut->SetCursorPosition (gST->ConOut, 0, 2);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  for (Col = 0; Col < mConsoleColumns; Col++) {
    Print (L" ");
  }
  
  // Redraw ONLY the tabs with updated selection colors
  for (Index = 0; Index < mTotalMainTabs; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, mMainTabs[Index].StartCol, 2);
    
    if (Index == mSelectedMainTab) {
      // Selected tab - light gray background with black text
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    } else {
      // Unselected tab - blue background with white text
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
    }
    
    Print (L" %s ", mMainTabs[Index].MenuText);
  }
}

**/

/**
  Update ONLY submenu selection - no background redraw.
**/

/**
VOID
UpdateSubmenuOnly (
  VOID
  )
{
  UINTN Index, Col;
  SUBMENU_ITEM *CurrentSubmenu;
  UINTN ItemCount;
  UINTN SeparatorCol = (mConsoleColumns * 3) / 4;
  
  CurrentSubmenu = GetCurrentSubmenuArray (&ItemCount);
  mTotalSubItems = ItemCount;
  
  // Clear ONLY the left section content area (NOT the entire interface)
  for (Index = 4; Index < mConsoleRows - 3; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 1, Index);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    for (Col = 1; Col < SeparatorCol - 1; Col++) {
      Print (L" ");
    }
  }
  
  // Display ONLY submenu items with correct selection colors
  for (Index = 0; Index < ItemCount && Index < 15; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 3, Index + 5);
    
    if (Index == mSelectedSubItem) {
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
    } else {
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    }
    
    Print (L"%s", CurrentSubmenu[Index].MenuText);
  }
}
**/


/**
  Update ONLY info panel - no background redraw.
**/

/**
VOID
UpdateInfoPanelOnly (
  VOID
  )
{
  SUBMENU_ITEM *CurrentSubmenu;
  UINTN ItemCount;
  CHAR16 TimeString[100];
  UINTN SeparatorCol = (mConsoleColumns * 3) / 4;
  UINTN RightStart = SeparatorCol + 2;
  UINTN Index, Col;
  
  CurrentSubmenu = GetCurrentSubmenuArray (&ItemCount);
  
  // Clear ONLY right section (NOT the entire interface)
  for (Index = 4; Index < mConsoleRows - 3; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, Index);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    for (Col = RightStart; Col < mConsoleColumns - 1; Col++) {
      Print (L" ");
    }
  }
  
  // Display information header
  gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 5);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"Selected Item:");
  
  // Display based on current selection
  if (mSelectedSubItem < ItemCount) {
    // Show selected item name
    gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 7);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_DARKGRAY | EFI_BACKGROUND_LIGHTGRAY);
    Print (L"%s", CurrentSubmenu[mSelectedSubItem].MenuText);
    
    // Show description with proper wrapping
    gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 9);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    
    UINTN RightWidth = mConsoleColumns - RightStart - 2;
    CHAR16 *Description = CurrentSubmenu[mSelectedSubItem].Description;
    UINTN DescLen = StrLen(Description);
    
    // Multi-line description display
    if (DescLen <= RightWidth) {
      Print (L"%s", Description);
    } else {
      // Multi-line text wrapping
      UINTN CurrentPos = 0;
      UINTN CurrentLine = 9;
      
      while (CurrentPos < DescLen && CurrentLine < mConsoleRows - 6) {
        gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, CurrentLine);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
        
        UINTN CharsToShow = RightWidth;
        if (CurrentPos + CharsToShow > DescLen) {
          CharsToShow = DescLen - CurrentPos;
        }
        
        // Find last space for word wrapping
        if (CurrentPos + CharsToShow < DescLen) {
          UINTN LastSpace = CharsToShow;
          for (UINTN i = CharsToShow; i > 0; i--) {
            if (Description[CurrentPos + i - 1] == L' ') {
              LastSpace = i;
              break;
            }
          }
          if (LastSpace < CharsToShow) {
            CharsToShow = LastSpace;
          }
        }
        
        // Print this line
        for (UINTN i = 0; i < CharsToShow && CurrentPos < DescLen; i++) {
          Print (L"%c", Description[CurrentPos]);
          CurrentPos++;
        }
        
        // Skip space if we broke at a space
        if (CurrentPos < DescLen && Description[CurrentPos] == L' ') {
          CurrentPos++;
        }
        
        CurrentLine++;
      }
    }
    
    // Special case for Date & Time
    if (mSelectedMainTab == 0 && mSelectedSubItem == 3) {
      GetCurrentTime (TimeString);
      gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 12);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"Current:");
      gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 13);
      
      UINTN TimeLen = StrLen(TimeString);
      if (TimeLen <= RightWidth) {
        Print (L"%s", TimeString);
      } else {
        Print (L"%.15s", TimeString);
        gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 14);
        Print (L"%s", &TimeString[15]);
      }
      
      gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 16);
      Print (L"Use +/- to");
      gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 17);
      Print (L"change values");
    }
  }
  
  // Show current tab name at bottom
  gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, mConsoleRows - 5);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"Tab: %s", mMainTabs[mSelectedMainTab].MenuText);
}

**/

/**
  Draw initial complete interface - ONE TIME ONLY.
**/

/**
VOID
DrawInitialInterface (
  VOID
  )
{
  UINTN Row, Col;
  UINTN SeparatorCol = (mConsoleColumns * 3) / 4;
  
  // Step 1: Draw background and headers - ONCE ONLY
  DrawBlueBackgroundOnce ();
  
  // Step 2: Draw the three-section layout - ONCE ONLY
  for (Row = 3; Row < mConsoleRows - 2; Row++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Row);
    
    // Left section (3/4 of screen)
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    for (Col = 0; Col < SeparatorCol; Col++) {
      Print (L" ");
    }
    
    // Vertical separator
    if (SeparatorCol < mConsoleColumns) {
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"│");
    }
    
    // Right section (1/4 of screen)
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    for (Col = SeparatorCol + 1; Col < mConsoleColumns; Col++) {
      Print (L" ");
    }
  }
  
  // Step 3: Draw bottom navigation help - ONCE ONLY
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  for (Row = mConsoleRows - 2; Row < mConsoleRows; Row++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Row);
    for (Col = 0; Col < mConsoleColumns; Col++) {
      Print (L" ");
    }
  }
  
  // Display navigation instructions - ONCE ONLY
  gST->ConOut->SetCursorPosition (gST->ConOut, 2, mConsoleRows - 1);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  Print (L"←→: Select Screen  ↑↓: Select Item  Enter: Select  ESC: Exit");
  
  // Step 4: Draw initial tabs and content
  UpdateTabsOnly ();
  UpdateSubmenuOnly ();
  UpdateInfoPanelOnly ();
  
  // Mark interface as initialized to prevent future background redraws
  mInterfaceInitialized = TRUE;
}

**/

/**
  Display BIOS setup form with PURE STATIC approach.
**/

/**
VOID
DisplayBiosSetupForm (
  VOID
  )
{
  mSelectedMainTab = 0;
  mSelectedSubItem = 0;
  mTotalMainTabs = ARRAY_SIZE (mMainTabs);
  mInBiosSetup = TRUE;
  mInterfaceInitialized = FALSE;  // Reset flag
  
  // Initialize mouse support
  InitializeMouse ();
  
  // Draw complete interface ONLY ONCE
  DrawInitialInterface ();
  
  // Reset console input for clean key detection
  gST->ConIn->Reset (gST->ConIn, FALSE);
  
  // Start navigation handling
  HandleBiosSetupNavigation ();
}

**/

/**
  Handle tab selection based on current selected tab.
**/
/**

VOID
HandleTabSelection (
  VOID
  )
{
  UINTN SeparatorCol = (mConsoleColumns * 3) / 4;
  UINTN RightStart = SeparatorCol + 2;
  
  switch (mSelectedMainTab) {
    case 3: // Save & Exit
      if (mSelectedSubItem == 0) { // Save Changes and Exit
        gST->ConOut->ClearScreen (gST->ConOut);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
        gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 35) / 2, mConsoleRows / 2);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_GREEN | EFI_BACKGROUND_BLACK);
        Print (L"✓ Settings saved. Shutting down system...");
        gBS->Stall (2000000);
        gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
        mInBiosSetup = FALSE;
      } else if (mSelectedSubItem == 1) { // Discard Changes and Exit
        gST->ConOut->ClearScreen (gST->ConOut);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
        gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 40) / 2, mConsoleRows / 2);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_RED | EFI_BACKGROUND_BLACK);
        Print (L"✗ Exiting without saving. Shutting down...");
        gBS->Stall (2000000);
        gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
        mInBiosSetup = FALSE;
      } else if (mSelectedSubItem == 2) { // Save Changes
        // Show temporary message in info panel
        gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_GREEN | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"✓ Saved");
        gBS->Stall (1500000);
        // Update info panel to clear message
        UpdateInfoPanelOnly ();
      } else if (mSelectedSubItem == 3) { // Discard Changes
        // Show temporary message in info panel
        gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_RED | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"✗ Discarded");
        gBS->Stall (1500000);
        // Update info panel to clear message
        UpdateInfoPanelOnly ();
      } else if (mSelectedSubItem == 4) { // Load Setup Defaults
        // Show temporary message in info panel
        gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"⚡ Defaults");
        gBS->Stall (1500000);
        // Update info panel to clear message
        UpdateInfoPanelOnly ();
      }
      break;
      
    default:
      // For other tabs, show brief action message
      gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 15);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"⚡ Action...");
      gBS->Stall (800000);
      // Update info panel to clear message
      UpdateInfoPanelOnly ();
      break;
  }
}

**/

/**
  Handle navigation - PURE STATIC (no slide effects).
**/

/**

VOID
HandleBiosSetupNavigation (
  VOID
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  UINTN          MouseTabIndex, MouseSubIndex;
  
  while (mInBiosSetup) {
    // Update mouse state
    UpdateMouseState ();
    
    // Redraw mouse cursor
    DrawMouseCursor ();
    
    // Handle mouse input
    if (mSimplePointer != NULL) {
      MouseTabIndex = GetTabFromMousePosition (mMouseState.LastX, mMouseState.LastY);
      MouseSubIndex = GetSubItemFromMousePosition (mMouseState.LastX, mMouseState.LastY);
      
      // Handle mouse clicks on tabs
      if (mMouseState.LeftButtonReleased && MouseTabIndex != (UINTN)-1) {
        if (MouseTabIndex != mSelectedMainTab) {
          mSelectedMainTab = MouseTabIndex;
          mSelectedSubItem = 0; // Reset submenu selection
          
          // STATIC UPDATE: Only update what changed - NO background redraw
          UpdateTabsOnly ();      // Update only tab colors
          UpdateSubmenuOnly ();   // Update only submenu content
          UpdateInfoPanelOnly (); // Update only info panel
        }
        mMouseState.LeftButtonReleased = FALSE;
      }
      
      // Handle mouse clicks on submenu items
      if (mMouseState.LeftButtonReleased && MouseSubIndex != (UINTN)-1) {
        if (MouseSubIndex != mSelectedSubItem) {
          mSelectedSubItem = MouseSubIndex;
          
          // STATIC UPDATE: Only update submenu and info - NO background redraw
          UpdateSubmenuOnly ();
          UpdateInfoPanelOnly ();
        } else {
          // Execute selection
          HandleTabSelection ();
          if (!mInBiosSetup) return;
        }
        mMouseState.LeftButtonReleased = FALSE;
      }
    }
    
    // Handle keyboard input
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
    
    if (!EFI_ERROR (Status)) {
      switch (Key.ScanCode) {
        case SCAN_LEFT:
          // Move left in main tabs
          if (mSelectedMainTab > 0) {
            mSelectedMainTab--;
          } else {
            mSelectedMainTab = mTotalMainTabs - 1;
          }
          mSelectedSubItem = 0;
          
          // STATIC UPDATE: Only update what changed - NO slide effect
          UpdateTabsOnly ();      // Update only tab colors
          UpdateSubmenuOnly ();   // Update only submenu content  
          UpdateInfoPanelOnly (); // Update only info panel
          break;
          
        case SCAN_RIGHT:
          // Move right in main tabs
          if (mSelectedMainTab < mTotalMainTabs - 1) {
            mSelectedMainTab++;
          } else {
            mSelectedMainTab = 0;
          }
          mSelectedSubItem = 0;
          
          // STATIC UPDATE: Only update what changed - NO slide effect
          UpdateTabsOnly ();      // Update only tab colors
          UpdateSubmenuOnly ();   // Update only submenu content
          UpdateInfoPanelOnly (); // Update only info panel
          break;
          
        case SCAN_UP:
          // Move up in submenu
          if (mSelectedSubItem > 0) {
            mSelectedSubItem--;
          } else {
            mSelectedSubItem = mTotalSubItems - 1;
          }
          
          // STATIC UPDATE: Only update submenu and info - NO background redraw
          UpdateSubmenuOnly ();
          UpdateInfoPanelOnly ();
          break;
          
        case SCAN_DOWN:
          // Move down in submenu
          if (mSelectedSubItem < mTotalSubItems - 1) {
            mSelectedSubItem++;
          } else {
            mSelectedSubItem = 0;
          }
          
          // STATIC UPDATE: Only update submenu and info - NO background redraw
          UpdateSubmenuOnly ();
          UpdateInfoPanelOnly ();
          break;
          
        case SCAN_ESC:
          // Exit BIOS setup
          mInBiosSetup = FALSE;
          gST->ConOut->ClearScreen (gST->ConOut);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
          gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 40) / 2, mConsoleRows / 2);
          Print (L"🚪 Exiting BIOS Setup. Shutting down system...");
          gBS->Stall (2000000);
          gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
          return;
          
        default:
          // Check for ENTER key
          if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
            HandleTabSelection ();
            if (!mInBiosSetup) {
              return;
            }
          }
          break;
      }
    } else {
      // Small delay to prevent excessive CPU usage
      gBS->Stall (50000); // 50ms
    }
  }
}

**/

/**
  Monitor keyboard input for F12 key press only.
**/

/**
VOID
EFIAPI
KeyboardMonitor (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  
  // Only monitor after message is shown and not in BIOS setup
  if (!mMessageShown || mInBiosSetup) {
    return;
  }
  
  // Check if key is available
  Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
  
  if (!EFI_ERROR (Status)) {
    // Check for F12 key only
    if (Key.ScanCode == SCAN_F12) {
      DEBUG ((DEBUG_INFO, "F12 key pressed - showing BIOS setup form\n"));
      
      // Cancel timer and keyboard events
      gBS->SetTimer (mKeyMonitorEvent, TimerCancel, 0);
      gBS->SetTimer (mTimerEvent, TimerCancel, 0);
      
      // Display BIOS setup form
      DisplayBiosSetupForm ();
      
      // After exiting BIOS setup, shutdown
      gST->ConOut->ClearScreen (gST->ConOut);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
      gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 40) / 2, mConsoleRows / 2);
      Print (L"✅ BIOS Setup completed. Shutting down system...");
      gBS->Stall (2000000);
      gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
      return;
    }
  }
}

**/

/**
  Timer callback function to handle logo display timing.
**/

/**
VOID
EFIAPI
TimerCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  mTimerCounter++;
  
  // After 5 seconds, show the message below logo and wait for F12
  if (mTimerCounter == 50 && !mMessageShown) { // 50 * 100ms = 5 seconds
    // Clear the entire screen first
    gST->ConOut->ClearScreen (gST->ConOut);
    
    // Get console size for proper centering
    GetConsoleSize ();
    
    // Position cursor below the logo area (center of screen)
    gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 28) / 2, mConsoleRows / 2 + 5);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
    Print (L"Press F12 key for BIOS setup");
    
    mMessageShown = TRUE;
    
    // Reset console input for clean key detection
    gST->ConIn->Reset (gST->ConIn, FALSE);
    
    DEBUG ((DEBUG_INFO, "Message displayed, waiting for F12 key\n"));
  }
}

**/

/**
  Setup timing and keyboard monitoring after logo is displayed.
**/

/**
VOID
SetupLogoTiming (
  VOID
  )
{
  EFI_STATUS  Status;
  
  if (mLogoDisplayed) {
    return;
  }
  
  DEBUG ((DEBUG_INFO, "Setting up logo timing and keyboard monitoring\n"));
  
  // Create timer event for logo timing
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  TimerCallback,
                  NULL,
                  &mTimerEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    Status = gBS->SetTimer (
                    mTimerEvent,
                    TimerPeriodic,
                    1000000  // 100ms in 100ns units
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set timer for mTimerEvent: %r\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to create mTimerEvent: %r\n", Status));
  }
  
  // Create keyboard monitoring event
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  KeyboardMonitor,
                  NULL,
                  &mKeyMonitorEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    Status = gBS->SetTimer (
                    mKeyMonitorEvent,
                    TimerPeriodic,
                    500000  // 50ms in 100ns units
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set timer for mKeyMonitorEvent: %r\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to create mKeyMonitorEvent: %r\n", Status));
  }
  
  mLogoDisplayed = TRUE;
}

**/

/**
  Load a platform logo image and return its data and attributes.
**/

/**
EFI_STATUS
EFIAPI
GetImage (
  IN     EDKII_PLATFORM_LOGO_PROTOCOL        *This,
  IN OUT UINT32                              *Instance,
  OUT EFI_IMAGE_INPUT                        *Image,
  OUT EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE  *Attribute,
  OUT INTN                                   *OffsetX,
  OUT INTN                                   *OffsetY
  )
{
  UINT32      Current;
  EFI_STATUS  Status;

  if ((Instance == NULL) || (Image == NULL) ||
      (Attribute == NULL) || (OffsetX == NULL) || (OffsetY == NULL))
  {
    return EFI_INVALID_PARAMETER;
  }

  Current = *Instance;
  if (Current >= ARRAY_SIZE (mLogos)) {
    return EFI_NOT_FOUND;
  }

  (*Instance)++;
  *Attribute = mLogos[Current].Attribute;
  *OffsetX   = mLogos[Current].OffsetX;
  *OffsetY   = mLogos[Current].OffsetY;
  
  // Get image from HII database
  Status = mHiiImageEx->GetImageEx (mHiiImageEx, mHiiHandle, mLogos[Current].ImageId, Image);
  
  // After first logo is displayed, setup timing
  if (Current == 0 && !EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Logo displayed, setting up timing\n"));
    SetupLogoTiming ();
  }
  
  return Status;
}

EDKII_PLATFORM_LOGO_PROTOCOL  mPlatformLogo = {
  GetImage
};

**/

/**
  Cleanup function called when driver is unloaded.
**/

/**
EFI_STATUS
EFIAPI
LogoDriverUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
  // Close events if they exist
  if (mTimerEvent != NULL) {
    gBS->CloseEvent (mTimerEvent);
    mTimerEvent = NULL;
  }
  
  if (mKeyMonitorEvent != NULL) {
    gBS->CloseEvent (mKeyMonitorEvent);
    mKeyMonitorEvent = NULL;
  }
  
  if (mBiosSetupKeyEvent != NULL) {
    gBS->CloseEvent (mBiosSetupKeyEvent);
    mBiosSetupKeyEvent = NULL;
  }
  
  if (mMouseEvent != NULL) {
    gBS->CloseEvent (mMouseEvent);
    mMouseEvent = NULL;
  }
  
  return EFI_SUCCESS;
}

**/

/**
  Entrypoint of this module.
**/

/**
EFI_STATUS
EFIAPI
InitializeLogo (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                   Status;
  EFI_HII_PACKAGE_LIST_HEADER  *PackageList;
  EFI_HII_DATABASE_PROTOCOL    *HiiDatabase;
  EFI_HANDLE                   Handle;

  DEBUG ((DEBUG_INFO, "Logo Driver: Pure Static BIOS - No Slide, No Duplication\n"));

  // Initialize global variables
  mInBiosSetup = FALSE;
  mSelectedMainTab = 0;
  mSelectedSubItem = 0;
  mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;
  mTotalMainTabs = ARRAY_SIZE (mMainTabs);
  mInterfaceInitialized = FALSE;

  // Locate HII Database Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiDatabaseProtocolGuid,
                  NULL,
                  (VOID **)&HiiDatabase
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Database Protocol: %r\n", Status));
    return Status;
  }

  // Locate HII Image Ex Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiImageExProtocolGuid,
                  NULL,
                  (VOID **)&mHiiImageEx
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Image Ex Protocol: %r\n", Status));
    return Status;
  }

  // Retrieve HII package list from ImageHandle
  Status = gBS->OpenProtocol (
                  ImageHandle,
                  &gEfiHiiPackageListProtocolGuid,
                  (VOID **)&PackageList,
                  ImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "HII Image Package with logo not found in PE/COFF resource section\n"));
    return Status;
  }

  // Publish HII package list to HII Database
  Status = HiiDatabase->NewPackageList (
                          HiiDatabase,
                          PackageList,
                          NULL,
                          &mHiiHandle
                          );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to add package list to HII Database: %r\n", Status));
    return Status;
  }

  // Install Platform Logo Protocol
  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEdkiiPlatformLogoProtocolGuid,
                  &mPlatformLogo,
                  NULL
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to install Platform Logo Protocol: %r\n", Status));
    HiiDatabase->RemovePackageList (HiiDatabase, mHiiHandle);
    return Status;
  }

  DEBUG ((DEBUG_INFO, "Logo Driver: Pure Static Implementation Complete\n"));
  
  return EFI_SUCCESS;
}

**/




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





// Pure Static BIOS Setup - NO mouse, WITH manual calendar for date/time. This works perfectly.

/** @file
  Logo DXE Driver with F12 key BIOS setup functionality.
  Shows logo, waits for F12 key, then displays BIOS setup form with navigation.
  Enhanced with blue theme, horizontal menu tabs and PURE STATIC layout.

Copyright (c) 2016 - 2017, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent
**/

/**

#include <Uefi.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/HiiImageEx.h>
#include <Protocol/PlatformLogo.h>
#include <Protocol/HiiPackageList.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/SimpleTextOut.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>
#include <Library/MemoryAllocationLib.h>

typedef struct {
  EFI_IMAGE_ID                             ImageId;
  EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE    Attribute;
  INTN                                     OffsetX;
  INTN                                     OffsetY;
} LOGO_ENTRY;

// Menu item structure for horizontal tabs
typedef struct {
  CHAR16  *MenuText;
  CHAR16  *Description;
  UINT32  StartCol;    // Column where menu tab starts
  UINT32  EndCol;      // Column where menu tab ends
  UINT32  TabWidth;    // Width of the tab
} MENU_TAB;

// Submenu item structure
typedef struct {
  CHAR16  *MenuText;
  CHAR16  *Description;
  UINT32  StartRow;    // Row where menu item starts
  UINT32  StartCol;    // Column where menu item starts  
  UINT32  EndRow;      // Row where menu item ends
  UINT32  EndCol;      // Column where menu item ends
} SUBMENU_ITEM;

// Calendar state structure for date/time setting
typedef struct {
  UINT16   Year;
  UINT8    Month;
  UINT8    Day;
  UINT8    Hour;
  UINT8    Minute;
  UINT8    Second;
  UINT8    SelectedField;  // 0=Year, 1=Month, 2=Day, 3=Hour, 4=Minute, 5=Second
  BOOLEAN  InCalendarMode;
} CALENDAR_STATE;

// Menu level enumeration
typedef enum {
  MENU_LEVEL_MAIN_MENU = 0,
  MENU_LEVEL_MAIN_SUBMENU,
  MENU_LEVEL_SECURITY_SUBMENU,
  MENU_LEVEL_ADVANCED_SUBMENU,
  MENU_LEVEL_UEFI_SUBMENU,
  MENU_LEVEL_CALENDAR
} MENU_LEVEL;

// Global variables
EFI_HII_IMAGE_EX_PROTOCOL  *mHiiImageEx;
EFI_HII_HANDLE             mHiiHandle;
EFI_EVENT                  mKeyMonitorEvent;
EFI_EVENT                  mTimerEvent;
EFI_EVENT                  mBiosSetupKeyEvent;
BOOLEAN                    mLogoDisplayed = FALSE;
BOOLEAN                    mMessageShown = FALSE;
BOOLEAN                    mInBiosSetup = FALSE;
UINTN                      mTimerCounter = 0;
UINTN                      mSelectedMainTab = 0;
UINTN                      mSelectedSubItem = 0;
UINTN                      mTotalMainTabs = 0;
UINTN                      mTotalSubItems = 0;
MENU_LEVEL                 mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;
UINTN                      mConsoleColumns = 100;
UINTN                      mConsoleRows = 40;
CALENDAR_STATE             mCalendar;

// STATIC flag to prevent background redraw
BOOLEAN                    mInterfaceInitialized = FALSE;

// Main horizontal tabs (fixed spacing and positioning)
MENU_TAB mMainTabs[] = {
  { L"Main", L"System Information and Basic Settings", 2, 17, 16 },
  { L"Advanced", L"Advanced Configuration Options", 18, 35, 18 },
  { L"Security", L"Security and password Settings", 36, 53, 18 },
  { L"Save & Exit", L"Save changes and exit setup", 54, 75, 22 }
};

// Main submenu items (for Main tab)
SUBMENU_ITEM mMainSubmenu[] = {
  { L"BIOS Information", L"View BIOS version and system information", 5, 5, 5, 35 },
  { L"BIOS Eventlog", L"Display BIOS event log entries", 6, 5, 6, 35 },
  { L"Update system BIOS", L"Update system BIOS firmware", 7, 5, 7, 35 },
  { L"Change Date & Time", L"Configure system date and time manually", 8, 5, 8, 35 },
};

// Advanced submenu items
SUBMENU_ITEM mAdvancedSubmenu[] = {
  { L"Boot Options", L"Configure boot device priority and settings", 5, 5, 5, 35 },
  { L"Port Options", L"Configure USB, SATA, and other port settings", 6, 5, 6, 35 },
  { L"System Options", L"Configure CPU, memory, and system settings", 7, 5, 7, 35 },
  { L"Power Management", L"Configure power and thermal settings", 8, 5, 8, 35 },
};

// Security submenu items
SUBMENU_ITEM mSecuritySubmenu[] = {
  { L"Administrator Tools", L"Administrative security management tools", 5, 5, 5, 35 },
  { L"Create BIOS Password", L"Set BIOS administrator password protection", 6, 5, 6, 35 },
  { L"TPM Security", L"Configure Trusted Platform Module settings", 7, 5, 7, 35 },
  { L"Secure Boot", L"Enable or disable UEFI Secure Boot", 8, 5, 8, 35 },
};

// Save & Exit submenu items
SUBMENU_ITEM mSaveExitSubmenu[] = {
  { L"Save Changes and Exit", L"Save current settings and exit setup", 5, 5, 5, 35 },
  { L"Discard Changes and Exit", L"Exit setup without saving any changes", 6, 5, 6, 35 },
  { L"Save Changes", L"Save current settings and continue", 7, 5, 7, 35 },
  { L"Discard Changes", L"Reset to previous saved settings", 8, 5, 8, 35 },
  { L"Load Setup Defaults", L"Load factory default settings", 9, 5, 9, 35 }
};

// Month names for calendar display
CHAR16 *mMonthNames[] = {
  L"January", L"February", L"March", L"April", L"May", L"June",
  L"July", L"August", L"September", L"October", L"November", L"December"
};

// Function declarations
VOID DisplayBiosSetupForm (VOID);
VOID GetCurrentTime (OUT CHAR16 *TimeString);
VOID HandleBiosSetupNavigation (VOID);
VOID DrawBlueBackgroundOnce (VOID);
VOID UpdateTabsOnly (VOID);
VOID UpdateSubmenuOnly (VOID);
VOID UpdateInfoPanelOnly (VOID);
VOID DrawInitialInterface (VOID);
VOID HandleTabSelection (VOID);
VOID HandleSubItemSelection (VOID);
SUBMENU_ITEM* GetCurrentSubmenuArray (OUT UINTN *ItemCount);
VOID GetConsoleSize (VOID);
VOID InitializeCalendar (VOID);
VOID DisplayCalendar (VOID);
VOID HandleCalendarNavigation (VOID);
VOID UpdateCalendarTime (VOID);
BOOLEAN IsLeapYear (UINT16 Year);
UINT8 GetDaysInMonth (UINT16 Year, UINT8 Month);
VOID SetSystemTime (VOID);

LOGO_ENTRY mLogos[] = {
  {
    IMAGE_TOKEN (IMG_LOGO),
    EdkiiPlatformLogoDisplayAttributeCenter,
    0,
    0
  }
};

**/

/**
  Check if a year is a leap year.
**/

/**
BOOLEAN
IsLeapYear (
  UINT16 Year
  )
{
  return ((Year % 4 == 0) && (Year % 100 != 0)) || (Year % 400 == 0);
}

**/

/**
  Get number of days in a month.
**/

/**
UINT8
GetDaysInMonth (
  UINT16 Year,
  UINT8  Month
  )
{
  UINT8 DaysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  
  if (Month == 2 && IsLeapYear(Year)) {
    return 29;
  }
  
  return DaysInMonth[Month - 1];
}
**/


/**
  Initialize calendar with current system time.
**/
/**

VOID
InitializeCalendar (
  VOID
  )
{
  EFI_TIME    Time;
  EFI_STATUS  Status;
  
  Status = gRT->GetTime (&Time, NULL);
  if (!EFI_ERROR (Status)) {
    mCalendar.Year = Time.Year;
    mCalendar.Month = Time.Month;
    mCalendar.Day = Time.Day;
    mCalendar.Hour = Time.Hour;
    mCalendar.Minute = Time.Minute;
    mCalendar.Second = Time.Second;
  } else {
    // Default values if system time is not available
    mCalendar.Year = 2024;
    mCalendar.Month = 1;
    mCalendar.Day = 1;
    mCalendar.Hour = 0;
    mCalendar.Minute = 0;
    mCalendar.Second = 0;
  }
  
  mCalendar.SelectedField = 0; // Start with Year selected
  mCalendar.InCalendarMode = TRUE;
}

**/

/**
  Set system time from calendar values.
**/

/**

VOID
SetSystemTime (
  VOID
  )
{
  EFI_TIME Time;
  EFI_STATUS Status;
  
  Time.Year = mCalendar.Year;
  Time.Month = mCalendar.Month;
  Time.Day = mCalendar.Day;
  Time.Hour = mCalendar.Hour;
  Time.Minute = mCalendar.Minute;
  Time.Second = mCalendar.Second;
  Time.Nanosecond = 0;
  Time.TimeZone = EFI_UNSPECIFIED_TIMEZONE;
  Time.Daylight = 0;
  
  Status = gRT->SetTime (&Time);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to set system time: %r\n", Status));
  }
}

**/


/**
  Display calendar interface for manual date/time setting.
**/

/**

VOID
DisplayCalendar (
  VOID
  )
{
  UINTN Row, Col;
  UINTN SeparatorCol = (mConsoleColumns * 3) / 4;
  UINTN RightStart = SeparatorCol + 2;
  
  // Clear the interface area
  for (Row = 4; Row < mConsoleRows - 3; Row++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 1, Row);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    for (Col = 1; Col < mConsoleColumns - 1; Col++) {
      Print (L" ");
    }
  }
  
  // Calendar title
  gST->ConOut->SetCursorPosition (gST->ConOut, 3, 5);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"Manual Date & Time Setting");
  
  // Current date/time display
  gST->ConOut->SetCursorPosition (gST->ConOut, 3, 7);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"Current Date & Time:");
  
  // Date fields with selection highlighting
  gST->ConOut->SetCursorPosition (gST->ConOut, 3, 9);
  
  // Year field
  if (mCalendar.SelectedField == 0) {
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  } else {
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  }
  Print (L"[%04d]", mCalendar.Year);
  
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  Print (L" / ");
  
  // Month field
  if (mCalendar.SelectedField == 1) {
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  } else {
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  }
  Print (L"[%02d]", mCalendar.Month);
  
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  Print (L" / ");
  
  // Day field
  if (mCalendar.SelectedField == 2) {
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  } else {
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  }
  Print (L"[%02d]", mCalendar.Day);
  
  // Time fields
  gST->ConOut->SetCursorPosition (gST->ConOut, 3, 11);
  
  // Hour field
  if (mCalendar.SelectedField == 3) {
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  } else {
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  }
  Print (L"[%02d]", mCalendar.Hour);
  
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  Print (L" : ");
  
  // Minute field
  if (mCalendar.SelectedField == 4) {
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  } else {
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  }
  Print (L"[%02d]", mCalendar.Minute);
  
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  Print (L" : ");
  
  // Second field
  if (mCalendar.SelectedField == 5) {
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  } else {
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  }
  Print (L"[%02d]", mCalendar.Second);
  
  // Month name display
  if (mCalendar.Month >= 1 && mCalendar.Month <= 12) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 3, 13);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_DARKGRAY | EFI_BACKGROUND_LIGHTGRAY);
    Print (L"Month: %s", mMonthNames[mCalendar.Month - 1]);
  }
  
  // Instructions
  gST->ConOut->SetCursorPosition (gST->ConOut, 3, 15);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"Instructions:");
  
  gST->ConOut->SetCursorPosition (gST->ConOut, 3, 16);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"Left/Right: Select field");
  
  gST->ConOut->SetCursorPosition (gST->ConOut, 3, 17);
  Print (L"Up/Down: Change value");
  
  gST->ConOut->SetCursorPosition (gST->ConOut, 3, 18);
  Print (L"Enter: Save changes");
  
  gST->ConOut->SetCursorPosition (gST->ConOut, 3, 19);
  Print (L"ESC: Cancel and return");
  
  // Right panel info
  gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 5);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"Calendar Settings");
  
  gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 7);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"Selected Field:");
  
  gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 8);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_DARKGRAY | EFI_BACKGROUND_LIGHTGRAY);
  
  switch (mCalendar.SelectedField) {
    case 0: Print (L"Year"); break;
    case 1: Print (L"Month"); break;
    case 2: Print (L"Day"); break;
    case 3: Print (L"Hour"); break;
    case 4: Print (L"Minute"); break;
    case 5: Print (L"Second"); break;
  }
  
  gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 10);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"Value Ranges:");
  
  gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 11);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_DARKGRAY | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"Year: 1900-2099");
  
  gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 12);
  Print (L"Month: 1-12");
  
  gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 13);
  Print (L"Day: 1-%d", GetDaysInMonth(mCalendar.Year, mCalendar.Month));
  
  gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 14);
  Print (L"Hour: 0-23");
  
  gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 15);
  Print (L"Min/Sec: 0-59");
}

**/

/**
  Handle calendar navigation and value changes.
**/

/**

VOID
HandleCalendarNavigation (
  VOID
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  BOOLEAN        ValueChanged = FALSE;
  UINT8          MaxDays;
  
  while (mCalendar.InCalendarMode && mInBiosSetup) {
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
    
    if (!EFI_ERROR (Status)) {
      switch (Key.ScanCode) {
        case SCAN_LEFT:
          // Move to previous field
          if (mCalendar.SelectedField > 0) {
            mCalendar.SelectedField--;
          } else {
            mCalendar.SelectedField = 5; // Wrap to last field
          }
          DisplayCalendar ();
          break;
          
        case SCAN_RIGHT:
          // Move to next field
          if (mCalendar.SelectedField < 5) {
            mCalendar.SelectedField++;
          } else {
            mCalendar.SelectedField = 0; // Wrap to first field
          }
          DisplayCalendar ();
          break;
          
        case SCAN_UP:
          // Increase value
          ValueChanged = TRUE;
          switch (mCalendar.SelectedField) {
            case 0: // Year
              if (mCalendar.Year < 2099) mCalendar.Year++;
              break;
            case 1: // Month
              if (mCalendar.Month < 12) {
                mCalendar.Month++;
              } else {
                mCalendar.Month = 1;
              }
              break;
            case 2: // Day
              MaxDays = GetDaysInMonth(mCalendar.Year, mCalendar.Month);
              if (mCalendar.Day < MaxDays) {
                mCalendar.Day++;
              } else {
                mCalendar.Day = 1;
              }
              break;
            case 3: // Hour
              if (mCalendar.Hour < 23) {
                mCalendar.Hour++;
              } else {
                mCalendar.Hour = 0;
              }
              break;
            case 4: // Minute
              if (mCalendar.Minute < 59) {
                mCalendar.Minute++;
              } else {
                mCalendar.Minute = 0;
              }
              break;
            case 5: // Second
              if (mCalendar.Second < 59) {
                mCalendar.Second++;
              } else {
                mCalendar.Second = 0;
              }
              break;
          }
          DisplayCalendar ();
          break;
          
        case SCAN_DOWN:
          // Decrease value
          ValueChanged = TRUE;
          switch (mCalendar.SelectedField) {
            case 0: // Year
              if (mCalendar.Year > 1900) mCalendar.Year--;
              break;
            case 1: // Month
              if (mCalendar.Month > 1) {
                mCalendar.Month--;
              } else {
                mCalendar.Month = 12;
              }
              break;
            case 2: // Day
              if (mCalendar.Day > 1) {
                mCalendar.Day--;
              } else {
                MaxDays = GetDaysInMonth(mCalendar.Year, mCalendar.Month);
                mCalendar.Day = MaxDays;
              }
              break;
            case 3: // Hour
              if (mCalendar.Hour > 0) {
                mCalendar.Hour--;
              } else {
                mCalendar.Hour = 23;
              }
              break;
            case 4: // Minute
              if (mCalendar.Minute > 0) {
                mCalendar.Minute--;
              } else {
                mCalendar.Minute = 59;
              }
              break;
            case 5: // Second
              if (mCalendar.Second > 0) {
                mCalendar.Second--;
              } else {
                mCalendar.Second = 59;
              }
              break;
          }
          DisplayCalendar ();
          break;
          
        case SCAN_ESC:
          // Cancel and return to main menu
          mCalendar.InCalendarMode = FALSE;
          mCurrentMenuLevel = MENU_LEVEL_MAIN_SUBMENU;
          DrawInitialInterface ();
          return;
          
        default:
          // Check for ENTER key
          if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
            // Save changes and return
            SetSystemTime ();
            mCalendar.InCalendarMode = FALSE;
            mCurrentMenuLevel = MENU_LEVEL_MAIN_SUBMENU;
            
            // Show confirmation message briefly
            gST->ConOut->SetCursorPosition (gST->ConOut, 3, 21);
            gST->ConOut->SetAttribute (gST->ConOut, EFI_GREEN | EFI_BACKGROUND_LIGHTGRAY);
            Print (L"Date & Time saved successfully!");
            gBS->Stall (1500000); // Wait 1.5 seconds
            
            DrawInitialInterface ();
            return;
          }
          break;
      }
      
      // Validate day when month/year changes
      if (ValueChanged && (mCalendar.SelectedField == 0 || mCalendar.SelectedField == 1)) {
        MaxDays = GetDaysInMonth(mCalendar.Year, mCalendar.Month);
        if (mCalendar.Day > MaxDays) {
          mCalendar.Day = MaxDays;
          DisplayCalendar ();
        }
      }
    } else {
      // Small delay to prevent excessive CPU usage
      gBS->Stall (50000); // 50ms
    }
  }
}

**/

/**
  Get console size and set to larger size for full screen display.
**/

/**

VOID
GetConsoleSize (
  VOID
  )
{
  EFI_STATUS Status;
  UINTN MaxMode, MaxColumns, MaxRows;
  UINTN BestMode = 0;
  UINTN BestColumns = 80;
  UINTN BestRows = 25;
  
  // Try to find the largest available mode
  MaxMode = gST->ConOut->Mode->MaxMode;
  
  for (UINTN Mode = 0; Mode < MaxMode; Mode++) {
    Status = gST->ConOut->QueryMode (
                            gST->ConOut,
                            Mode,
                            &MaxColumns,
                            &MaxRows
                            );
    
    if (!EFI_ERROR (Status)) {
      if ((MaxColumns >= BestColumns && MaxRows >= BestRows) && 
          (MaxColumns > BestColumns || MaxRows > BestRows)) {
        BestMode = Mode;
        BestColumns = MaxColumns;
        BestRows = MaxRows;
      }
    }
  }
  
  // Set the best mode we found
  if (BestMode != gST->ConOut->Mode->Mode) {
    Status = gST->ConOut->SetMode (gST->ConOut, BestMode);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_WARN, "Failed to set console mode %d: %r\n", BestMode, Status));
    }
  }
  
  // Query the actual current mode
  Status = gST->ConOut->QueryMode (
                          gST->ConOut,
                          gST->ConOut->Mode->Mode,
                          &mConsoleColumns,
                          &mConsoleRows
                          );
  
  if (EFI_ERROR (Status)) {
    mConsoleColumns = 100;
    mConsoleRows = 40;
  }
  
  DEBUG ((DEBUG_INFO, "Console size: %dx%d\n", mConsoleColumns, mConsoleRows));
}

**/

/**
  Get current submenu array based on selected main tab.
**/

/**

SUBMENU_ITEM*
GetCurrentSubmenuArray (
  OUT UINTN *ItemCount
  )
{
  switch (mSelectedMainTab) {
    case 0: // Main
      *ItemCount = ARRAY_SIZE (mMainSubmenu);
      return mMainSubmenu;
      
    case 1: // Advanced
      *ItemCount = ARRAY_SIZE (mAdvancedSubmenu);
      return mAdvancedSubmenu;
      
    case 2: // Security
      *ItemCount = ARRAY_SIZE (mSecuritySubmenu);
      return mSecuritySubmenu;
      
    case 3: // Save & Exit
      *ItemCount = ARRAY_SIZE (mSaveExitSubmenu);
      return mSaveExitSubmenu;
      
    default:
      *ItemCount = ARRAY_SIZE (mMainSubmenu);
      return mMainSubmenu;
  }
}

**/

/**
  Get current time and format it as string.
**/

/**
VOID
GetCurrentTime (
  OUT CHAR16  *TimeString
  )
{
  EFI_TIME    Time;
  EFI_STATUS  Status;
  
  Status = gRT->GetTime (&Time, NULL);
  if (!EFI_ERROR (Status)) {
    UnicodeSPrint (
      TimeString, 
      50 * sizeof(CHAR16),
      L"%02d/%02d/%d  %02d:%02d:%02d",
      Time.Day,
      Time.Month,
      Time.Year,
      Time.Hour,
      Time.Minute,
      Time.Second
    );
  } else {
    UnicodeSPrint (TimeString, 50 * sizeof(CHAR16), L"[Date/Time Not Available]");
  }
}

**/

/**
  Draw blue background and headers - ONE TIME ONLY.
**/

/**
VOID
DrawBlueBackgroundOnce (
  VOID
  )
{
  UINTN Index, Col;
  
  // Get actual console size
  GetConsoleSize ();
  
  // Clear screen and set blue background - ONE TIME ONLY
  gST->ConOut->ClearScreen (gST->ConOut);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  
  // Fill entire screen with blue background - ONE TIME ONLY
  for (Index = 0; Index < mConsoleRows; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Index);
    for (Col = 0; Col < mConsoleColumns; Col++) {
      Print (L" ");
    }
  }
  
  // Draw BIOS Setup Utility in top-left corner - ONE TIME ONLY
  gST->ConOut->SetCursorPosition (gST->ConOut, 1, 0);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  Print (L"BIOS Setup Utility");
  
  // Draw CDAC BANGALORE title below center - ONE TIME ONLY
  gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 14) / 2, 1);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
  Print (L"BHARAT BIOS CDAC ");
}

**/

/**
  Update ONLY tab colors - no background redraw.
**/

/**
VOID
UpdateTabsOnly (
  VOID
  )
{
  UINTN Index, Col;
  
  // Clear ONLY the tab row (row 2) - MINIMAL clearing
  gST->ConOut->SetCursorPosition (gST->ConOut, 0, 2);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  for (Col = 0; Col < mConsoleColumns; Col++) {
    Print (L" ");
  }
  
  // Redraw ONLY the tabs with updated selection colors
  for (Index = 0; Index < mTotalMainTabs; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, mMainTabs[Index].StartCol, 2);
    
    if (Index == mSelectedMainTab) {
      // Selected tab - light gray background with black text
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    } else {
      // Unselected tab - blue background with white text
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
    }
    
    Print (L" %s ", mMainTabs[Index].MenuText);
  }
}

**/

/**
  Update ONLY submenu selection - no background redraw.
**/

/**
VOID
UpdateSubmenuOnly (
  VOID
  )
{
  UINTN Index, Col;
  SUBMENU_ITEM *CurrentSubmenu;
  UINTN ItemCount;
  UINTN SeparatorCol = (mConsoleColumns * 3) / 4;
  
  CurrentSubmenu = GetCurrentSubmenuArray (&ItemCount);
  mTotalSubItems = ItemCount;
  
  // Clear ONLY the left section content area (NOT the entire interface)
  for (Index = 4; Index < mConsoleRows - 3; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 1, Index);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    for (Col = 1; Col < SeparatorCol - 1; Col++) {
      Print (L" ");
    }
  }
  
  // Display ONLY submenu items with correct selection colors
  for (Index = 0; Index < ItemCount && Index < 15; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 3, Index + 5);
    
    if (Index == mSelectedSubItem) {
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
    } else {
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    }
    
    Print (L"%s", CurrentSubmenu[Index].MenuText);
  }
}
**/


/**
  Update ONLY info panel - no background redraw.
**/

/**
VOID
UpdateInfoPanelOnly (
  VOID
  )
{
  SUBMENU_ITEM *CurrentSubmenu;
  UINTN ItemCount;
  CHAR16 TimeString[100];
  UINTN SeparatorCol = (mConsoleColumns * 3) / 4;
  UINTN RightStart = SeparatorCol + 2;
  UINTN Index, Col;
  
  CurrentSubmenu = GetCurrentSubmenuArray (&ItemCount);
  
  // Clear ONLY right section (NOT the entire interface)
  for (Index = 4; Index < mConsoleRows - 3; Index++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, Index);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    for (Col = RightStart; Col < mConsoleColumns - 1; Col++) {
      Print (L" ");
    }
  }
  
  // Display information header
  gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 5);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"Selected Item:");
  
  // Display based on current selection
  if (mSelectedSubItem < ItemCount) {
    // Show selected item name
    gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 7);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_DARKGRAY | EFI_BACKGROUND_LIGHTGRAY);
    Print (L"%s", CurrentSubmenu[mSelectedSubItem].MenuText);
    
    // Show description with proper wrapping
    gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 9);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    
    UINTN RightWidth = mConsoleColumns - RightStart - 2;
    CHAR16 *Description = CurrentSubmenu[mSelectedSubItem].Description;
    UINTN DescLen = StrLen(Description);
    
    // Multi-line description display
    if (DescLen <= RightWidth) {
      Print (L"%s", Description);
    } else {
      // Multi-line text wrapping
      UINTN CurrentPos = 0;
      UINTN CurrentLine = 9;
      
      while (CurrentPos < DescLen && CurrentLine < mConsoleRows - 6) {
        gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, CurrentLine);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
        
        UINTN CharsToShow = RightWidth;
        if (CurrentPos + CharsToShow > DescLen) {
          CharsToShow = DescLen - CurrentPos;
        }
        
        // Find last space for word wrapping
        if (CurrentPos + CharsToShow < DescLen) {
          UINTN LastSpace = CharsToShow;
          for (UINTN i = CharsToShow; i > 0; i--) {
            if (Description[CurrentPos + i - 1] == L' ') {
              LastSpace = i;
              break;
            }
          }
          if (LastSpace < CharsToShow) {
            CharsToShow = LastSpace;
          }
        }
        
        // Print this line
        for (UINTN i = 0; i < CharsToShow && CurrentPos < DescLen; i++) {
          Print (L"%c", Description[CurrentPos]);
          CurrentPos++;
        }
        
        // Skip space if we broke at a space
        if (CurrentPos < DescLen && Description[CurrentPos] == L' ') {
          CurrentPos++;
        }
        
        CurrentLine++;
      }
    }
    
    // Special case for Date & Time
    if (mSelectedMainTab == 0 && mSelectedSubItem == 3) {
      GetCurrentTime (TimeString);
      gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 12);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"Current:");
      gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 13);
      
      UINTN TimeLen = StrLen(TimeString);
      if (TimeLen <= RightWidth) {
        Print (L"%s", TimeString);
      } else {
        Print (L"%.15s", TimeString);
        gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 14);
        Print (L"%s", &TimeString[15]);
      }
      
      gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 16);
      Print (L"Press Enter to");
      gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 17);
      Print (L"open calendar");
    }
  }
  
  // Show current tab name at bottom
  gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, mConsoleRows - 5);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
  Print (L"Tab: %s", mMainTabs[mSelectedMainTab].MenuText);
}

**/

/**
  Draw initial complete interface - ONE TIME ONLY.
**/

/**
VOID
DrawInitialInterface (
  VOID
  )
{
  UINTN Row, Col;
  UINTN SeparatorCol = (mConsoleColumns * 3) / 4;
  
  // Step 1: Draw background and headers - ONCE ONLY
  DrawBlueBackgroundOnce ();
  
  // Step 2: Draw the three-section layout - ONCE ONLY
  for (Row = 3; Row < mConsoleRows - 2; Row++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Row);
    
    // Left section (3/4 of screen)
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    for (Col = 0; Col < SeparatorCol; Col++) {
      Print (L" ");
    }
    
    // Vertical separator
    if (SeparatorCol < mConsoleColumns) {
      gST->ConOut->SetAttribute (gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"│");
    }
    
    // Right section (1/4 of screen)
    gST->ConOut->SetAttribute (gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    for (Col = SeparatorCol + 1; Col < mConsoleColumns; Col++) {
      Print (L" ");
    }
  }
  
  // Step 3: Draw bottom navigation help - ONCE ONLY
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  for (Row = mConsoleRows - 2; Row < mConsoleRows; Row++) {
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, Row);
    for (Col = 0; Col < mConsoleColumns; Col++) {
      Print (L" ");
    }
  }
  
  // Display navigation instructions - ONCE ONLY
  gST->ConOut->SetCursorPosition (gST->ConOut, 2, mConsoleRows - 1);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  Print (L"←→: Select Screen  ↑↓: Select Item  Enter: Select  ESC: Exit");
  
  // Step 4: Draw initial tabs and content
  UpdateTabsOnly ();
  UpdateSubmenuOnly ();
  UpdateInfoPanelOnly ();
  
  // Mark interface as initialized to prevent future background redraws
  mInterfaceInitialized = TRUE;
}

**/

/**
  Display BIOS setup form with PURE STATIC approach.
**/
/**

VOID
DisplayBiosSetupForm (
  VOID
  )
{
  mSelectedMainTab = 0;
  mSelectedSubItem = 0;
  mTotalMainTabs = ARRAY_SIZE (mMainTabs);
  mInBiosSetup = TRUE;
  mInterfaceInitialized = FALSE;  // Reset flag
  mCurrentMenuLevel = MENU_LEVEL_MAIN_SUBMENU;
  
  // Draw complete interface ONLY ONCE
  DrawInitialInterface ();
  
  // Reset console input for clean key detection
  gST->ConIn->Reset (gST->ConIn, FALSE);
  
  // Start navigation handling
  HandleBiosSetupNavigation ();
}

**/

/**
  Handle tab selection based on current selected tab.
**/
/**

VOID
HandleTabSelection (
  VOID
  )
{
  UINTN SeparatorCol = (mConsoleColumns * 3) / 4;
  UINTN RightStart = SeparatorCol + 2;
  
  // Special case: Change Date & Time in Main tab
  if (mSelectedMainTab == 0 && mSelectedSubItem == 3) {
    mCurrentMenuLevel = MENU_LEVEL_CALENDAR;
    InitializeCalendar ();
    DisplayCalendar ();
    HandleCalendarNavigation ();
    return;
  }
  
  switch (mSelectedMainTab) {
    case 3: // Save & Exit
      if (mSelectedSubItem == 0) { // Save Changes and Exit
        gST->ConOut->ClearScreen (gST->ConOut);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
        gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 35) / 2, mConsoleRows / 2);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_GREEN | EFI_BACKGROUND_BLACK);
        Print (L"✓ Settings saved. Shutting down system...");
        gBS->Stall (2000000);
        gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
        mInBiosSetup = FALSE;
      } else if (mSelectedSubItem == 1) { // Discard Changes and Exit
        gST->ConOut->ClearScreen (gST->ConOut);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
        gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 40) / 2, mConsoleRows / 2);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_RED | EFI_BACKGROUND_BLACK);
        Print (L"✗ Exiting without saving. Shutting down...");
        gBS->Stall (2000000);
        gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
        mInBiosSetup = FALSE;
      } else if (mSelectedSubItem == 2) { // Save Changes
        // Show temporary message in info panel
        gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_GREEN | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"✓ Saved");
        gBS->Stall (1500000);
        // Update info panel to clear message
        UpdateInfoPanelOnly ();
      } else if (mSelectedSubItem == 3) { // Discard Changes
        // Show temporary message in info panel
        gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_RED | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"✗ Discarded");
        gBS->Stall (1500000);
        // Update info panel to clear message
        UpdateInfoPanelOnly ();
      } else if (mSelectedSubItem == 4) { // Load Setup Defaults
        // Show temporary message in info panel
        gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 15);
        gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_LIGHTGRAY);
        Print (L"⚡ Defaults");
        gBS->Stall (1500000);
        // Update info panel to clear message
        UpdateInfoPanelOnly ();
      }
      break;
      
    default:
      // For other tabs, show brief action message
      gST->ConOut->SetCursorPosition (gST->ConOut, RightStart, 15);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_LIGHTGRAY);
      Print (L"⚡ Action...");
      gBS->Stall (800000);
      // Update info panel to clear message
      UpdateInfoPanelOnly ();
      break;
  }
}
**/


/**
  Handle navigation - PURE STATIC (no slide effects).
**/

/**
VOID
HandleBiosSetupNavigation (
  VOID
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  
  while (mInBiosSetup) {
    // Handle different menu levels
    if (mCurrentMenuLevel == MENU_LEVEL_CALENDAR) {
      HandleCalendarNavigation ();
      continue;
    }
    
    // Handle keyboard input
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
    
    if (!EFI_ERROR (Status)) {
      switch (Key.ScanCode) {
        case SCAN_LEFT:
          // Move left in main tabs
          if (mSelectedMainTab > 0) {
            mSelectedMainTab--;
          } else {
            mSelectedMainTab = mTotalMainTabs - 1;
          }
          mSelectedSubItem = 0;
          
          // STATIC UPDATE: Only update what changed - NO slide effect
          UpdateTabsOnly ();      // Update only tab colors
          UpdateSubmenuOnly ();   // Update only submenu content  
          UpdateInfoPanelOnly (); // Update only info panel
          break;
          
        case SCAN_RIGHT:
          // Move right in main tabs
          if (mSelectedMainTab < mTotalMainTabs - 1) {
            mSelectedMainTab++;
          } else {
            mSelectedMainTab = 0;
          }
          mSelectedSubItem = 0;
          
          // STATIC UPDATE: Only update what changed - NO slide effect
          UpdateTabsOnly ();      // Update only tab colors
          UpdateSubmenuOnly ();   // Update only submenu content
          UpdateInfoPanelOnly (); // Update only info panel
          break;
          
        case SCAN_UP:
          // Move up in submenu
          if (mSelectedSubItem > 0) {
            mSelectedSubItem--;
          } else {
            mSelectedSubItem = mTotalSubItems - 1;
          }
          
          // STATIC UPDATE: Only update submenu and info - NO background redraw
          UpdateSubmenuOnly ();
          UpdateInfoPanelOnly ();
          break;
          
        case SCAN_DOWN:
          // Move down in submenu
          if (mSelectedSubItem < mTotalSubItems - 1) {
            mSelectedSubItem++;
          } else {
            mSelectedSubItem = 0;
          }
          
          // STATIC UPDATE: Only update submenu and info - NO background redraw
          UpdateSubmenuOnly ();
          UpdateInfoPanelOnly ();
          break;
          
        case SCAN_ESC:
          // Exit BIOS setup
          mInBiosSetup = FALSE;
          gST->ConOut->ClearScreen (gST->ConOut);
          gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
          gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 40) / 2, mConsoleRows / 2);
          Print (L"🚪 Exiting BIOS Setup. Shutting down system...");
          gBS->Stall (2000000);
          gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
          return;
          
        default:
          // Check for ENTER key
          if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
            HandleTabSelection ();
            if (!mInBiosSetup) {
              return;
            }
          }
          break;
      }
    } else {
      // Small delay to prevent excessive CPU usage
      gBS->Stall (50000); // 50ms
    }
  }
}
**/


/**
  Monitor keyboard input for F12 key press only.
**/

/**
VOID
EFIAPI
KeyboardMonitor (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  
  // Only monitor after message is shown and not in BIOS setup
  if (!mMessageShown || mInBiosSetup) {
    return;
  }
  
  // Check if key is available
  Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
  
  if (!EFI_ERROR (Status)) {
    // Check for F12 key only
    if (Key.ScanCode == SCAN_F12) {
      DEBUG ((DEBUG_INFO, "F12 key pressed - showing BIOS setup form\n"));
      
      // Cancel timer and keyboard events
      gBS->SetTimer (mKeyMonitorEvent, TimerCancel, 0);
      gBS->SetTimer (mTimerEvent, TimerCancel, 0);
      
      // Display BIOS setup form
      DisplayBiosSetupForm ();
      
      // After exiting BIOS setup, shutdown
      gST->ConOut->ClearScreen (gST->ConOut);
      gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
      gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 40) / 2, mConsoleRows / 2);
      Print (L"✅ BIOS Setup completed. Shutting down system...");
      gBS->Stall (2000000);
      gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
      return;
    }
  }
}

**/

/**
  Timer callback function to handle logo display timing.
**/

/**
VOID
EFIAPI
TimerCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  mTimerCounter++;
  
  // After 5 seconds, show the message below logo and wait for F12
  if (mTimerCounter == 50 && !mMessageShown) { // 50 * 100ms = 5 seconds
    // Clear the entire screen first
    gST->ConOut->ClearScreen (gST->ConOut);
    
    // Get console size for proper centering
    GetConsoleSize ();
    
    // Position cursor below the logo area (center of screen)
    gST->ConOut->SetCursorPosition (gST->ConOut, (mConsoleColumns - 28) / 2, mConsoleRows / 2 + 5);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
    Print (L"Press F12 key for BIOS setup");
    
    mMessageShown = TRUE;
    
    // Reset console input for clean key detection
    gST->ConIn->Reset (gST->ConIn, FALSE);
    
    DEBUG ((DEBUG_INFO, "Message displayed, waiting for F12 key\n"));
  }
}
**/


/**
  Setup timing and keyboard monitoring after logo is displayed.
**/
/**

VOID
SetupLogoTiming (
  VOID
  )
{
  EFI_STATUS  Status;
  
  if (mLogoDisplayed) {
    return;
  }
  
  DEBUG ((DEBUG_INFO, "Setting up logo timing and keyboard monitoring\n"));
  
  // Create timer event for logo timing
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  TimerCallback,
                  NULL,
                  &mTimerEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    Status = gBS->SetTimer (
                    mTimerEvent,
                    TimerPeriodic,
                    1000000  // 100ms in 100ns units
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set timer for mTimerEvent: %r\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to create mTimerEvent: %r\n", Status));
  }
  
  // Create keyboard monitoring event
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  KeyboardMonitor,
                  NULL,
                  &mKeyMonitorEvent
                  );
  
  if (!EFI_ERROR (Status)) {
    Status = gBS->SetTimer (
                    mKeyMonitorEvent,
                    TimerPeriodic,
                    500000  // 50ms in 100ns units
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to set timer for mKeyMonitorEvent: %r\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to create mKeyMonitorEvent: %r\n", Status));
  }
  
  mLogoDisplayed = TRUE;
}
**/


/**
  Load a platform logo image and return its data and attributes.
**/
/**

EFI_STATUS
EFIAPI
GetImage (
  IN     EDKII_PLATFORM_LOGO_PROTOCOL        *This,
  IN OUT UINT32                              *Instance,
  OUT EFI_IMAGE_INPUT                        *Image,
  OUT EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE  *Attribute,
  OUT INTN                                   *OffsetX,
  OUT INTN                                   *OffsetY
  )
{
  UINT32      Current;
  EFI_STATUS  Status;

  if ((Instance == NULL) || (Image == NULL) ||
      (Attribute == NULL) || (OffsetX == NULL) || (OffsetY == NULL))
  {
    return EFI_INVALID_PARAMETER;
  }

  Current = *Instance;
  if (Current >= ARRAY_SIZE (mLogos)) {
    return EFI_NOT_FOUND;
  }

  (*Instance)++;
  *Attribute = mLogos[Current].Attribute;
  *OffsetX   = mLogos[Current].OffsetX;
  *OffsetY   = mLogos[Current].OffsetY;
  
  // Get image from HII database
  Status = mHiiImageEx->GetImageEx (mHiiImageEx, mHiiHandle, mLogos[Current].ImageId, Image);
  
  // After first logo is displayed, setup timing
  if (Current == 0 && !EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Logo displayed, setting up timing\n"));
    SetupLogoTiming ();
  }
  
  return Status;
}

EDKII_PLATFORM_LOGO_PROTOCOL  mPlatformLogo = {
  GetImage
};

**/

/**
  Cleanup function called when driver is unloaded.
**/
/**

EFI_STATUS
EFIAPI
LogoDriverUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
  // Close events if they exist
  if (mTimerEvent != NULL) {
    gBS->CloseEvent (mTimerEvent);
    mTimerEvent = NULL;
  }
  
  if (mKeyMonitorEvent != NULL) {
    gBS->CloseEvent (mKeyMonitorEvent);
    mKeyMonitorEvent = NULL;
  }
  
  if (mBiosSetupKeyEvent != NULL) {
    gBS->CloseEvent (mBiosSetupKeyEvent);
    mBiosSetupKeyEvent = NULL;
  }
  
  return EFI_SUCCESS;
}

**/

/**
  Entrypoint of this module.
**/
/**

EFI_STATUS
EFIAPI
InitializeLogo (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                   Status;
  EFI_HII_PACKAGE_LIST_HEADER  *PackageList;
  EFI_HII_DATABASE_PROTOCOL    *HiiDatabase;
  EFI_HANDLE                   Handle;

  DEBUG ((DEBUG_INFO, "Logo Driver: Pure Static BIOS - No Mouse, With Calendar\n"));

  // Initialize global variables
  mInBiosSetup = FALSE;
  mSelectedMainTab = 0;
  mSelectedSubItem = 0;
  mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;
  mTotalMainTabs = ARRAY_SIZE (mMainTabs);
  mInterfaceInitialized = FALSE;
  
  // Initialize calendar state
  mCalendar.InCalendarMode = FALSE;
  mCalendar.SelectedField = 0;

  // Locate HII Database Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiDatabaseProtocolGuid,
                  NULL,
                  (VOID **)&HiiDatabase
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Database Protocol: %r\n", Status));
    return Status;
  }

  // Locate HII Image Ex Protocol
  Status = gBS->LocateProtocol (
                  &gEfiHiiImageExProtocolGuid,
                  NULL,
                  (VOID **)&mHiiImageEx
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to locate HII Image Ex Protocol: %r\n", Status));
    return Status;
  }

  // Retrieve HII package list from ImageHandle
  Status = gBS->OpenProtocol (
                  ImageHandle,
                  &gEfiHiiPackageListProtocolGuid,
                  (VOID **)&PackageList,
                  ImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "HII Image Package with logo not found in PE/COFF resource section\n"));
    return Status;
  }

  // Publish HII package list to HII Database
  Status = HiiDatabase->NewPackageList (
                          HiiDatabase,
                          PackageList,
                          NULL,
                          &mHiiHandle
                          );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to add package list to HII Database: %r\n", Status));
    return Status;
  }

  // Install Platform Logo Protocol
  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEdkiiPlatformLogoProtocolGuid,
                  &mPlatformLogo,
                  NULL
                  );
  
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to install Platform Logo Protocol: %r\n", Status));
    HiiDatabase->RemovePackageList (HiiDatabase, mHiiHandle);
    return Status;
  }

  DEBUG ((DEBUG_INFO, "Logo Driver: Pure Static Implementation with Calendar Complete\n"));
  
  return EFI_SUCCESS;
}

**/






///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// this is not working for usb details information .it only shows some print messages those are given in code.



/**
#include <Uefi.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/PrintLib.h>
#include <Library/DebugLib.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/UsbIo.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/HiiImageEx.h>
#include <Protocol/PlatformLogo.h>
#include <Protocol/HiiPackageList.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/SimpleTextOut.h>

// USB Authentication Constants
#define FIXED_VENDOR_ID 0x0781
#define FIXED_PRODUCT_ID 0x5595
#define FIXED_SERIAL_NUMBER L"040144d2dd4c4643e76cd30e263d37e6fff7d68d4ecb3ac1374d551837620beec3b2000000000000000000008cd839090083751895558107b628040f"
#define MAX_ATTEMPTS 2

// BIOS Setup Structures and Constants
typedef struct {
  EFI_IMAGE_ID                             ImageId;
  EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE    Attribute;
  INTN                                     OffsetX;
  INTN                                     OffsetY;
} LOGO_ENTRY;

typedef struct {
  CHAR16  *MenuText;
  CHAR16  *Description;
  UINT32  StartCol;
  UINT32  EndCol;
  UINT32  TabWidth;
} MENU_TAB;

typedef struct {
  CHAR16  *MenuText;
  CHAR16  *Description;
  UINT32  StartRow;
  UINT32  StartCol;
  UINT32  EndRow;
  UINT32  EndCol;
} SUBMENU_ITEM;

typedef struct {
  UINT16   Year;
  UINT8    Month;
  UINT8    Day;
  UINT8    Hour;
  UINT8    Minute;
  UINT8    Second;
  UINT8    SelectedField;
  BOOLEAN  InCalendarMode;
} CALENDAR_STATE;

typedef enum {
  MENU_LEVEL_MAIN_MENU = 0,
  MENU_LEVEL_MAIN_SUBMENU,
  MENU_LEVEL_SECURITY_SUBMENU,
  MENU_LEVEL_ADVANCED_SUBMENU,
  MENU_LEVEL_UEFI_SUBMENU,
  MENU_LEVEL_CALENDAR
} MENU_LEVEL;

// Global Variables
EFI_HII_IMAGE_EX_PROTOCOL  *mHiiImageEx;
EFI_HII_HANDLE             mHiiHandle;
EFI_EVENT                  mKeyMonitorEvent;
EFI_EVENT                  mTimerEvent;
EFI_EVENT                  mUsbAuthEvent;
BOOLEAN                    mLogoDisplayed = FALSE;
BOOLEAN                    mMessageShown = FALSE;
BOOLEAN                    mInBiosSetup = FALSE;
BOOLEAN                    mUsbAuthenticated = FALSE;
UINTN                      mTimerCounter = 0;
UINTN                      mSelectedMainTab = 0;
UINTN                      mSelectedSubItem = 0;
UINTN                      mTotalMainTabs = 0;
UINTN                      mTotalSubItems = 0;
MENU_LEVEL                 mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;
UINTN                      mConsoleColumns = 100;
UINTN                      mConsoleRows = 40;
CALENDAR_STATE             mCalendar;
BOOLEAN                    mInterfaceInitialized = FALSE;

// Menu Data
MENU_TAB mMainTabs[] = {
  { L"Main", L"System Information and Basic Settings", 2, 17, 16 },
  { L"Advanced", L"Advanced Configuration Options", 18, 35, 18 },
  { L"Security", L"Security and password Settings", 36, 53, 18 },
  { L"Save & Exit", L"Save changes and exit setup", 54, 75, 22 }
};

SUBMENU_ITEM mMainSubmenu[] = {
  { L"BIOS Information", L"View BIOS version and system information", 5, 5, 5, 35 },
  { L"BIOS Eventlog", L"Display BIOS event log entries", 6, 5, 6, 35 },
  { L"Update system BIOS", L"Update system BIOS firmware", 7, 5, 7, 35 },
  { L"Change Date & Time", L"Configure system date and time manually", 8, 5, 8, 35 },
  { L"USB Authentication", L"Authenticate USB device for secure access", 9, 5, 9, 35 },
};

SUBMENU_ITEM mAdvancedSubmenu[] = {
  { L"Boot Options", L"Configure boot device priority and settings", 5, 5, 5, 35 },
  { L"Port Options", L"Configure USB, SATA, and other port settings", 6, 5, 6, 35 },
  { L"System Options", L"Configure CPU, memory, and system settings", 7, 5, 7, 35 },
  { L"Power Management", L"Configure power and thermal settings", 8, 5, 8, 35 },
};

SUBMENU_ITEM mSecuritySubmenu[] = {
  { L"Administrator Tools", L"Administrative security management tools", 5, 5, 5, 35 },
  { L"Create BIOS Password", L"Set BIOS administrator password protection", 6, 5, 6, 35 },
  { L"TPM Security", L"Configure Trusted Platform Module settings", 7, 5, 7, 35 },
  { L"Secure Boot", L"Enable or disable UEFI Secure Boot", 8, 5, 8, 35 },
};

SUBMENU_ITEM mSaveExitSubmenu[] = {
  { L"Save Changes and Exit", L"Save current settings and exit setup", 5, 5, 5, 35 },
  { L"Discard Changes and Exit", L"Exit setup without saving any changes", 6, 5, 6, 35 },
  { L"Save Changes", L"Save current settings and continue", 7, 5, 7, 35 },
  { L"Discard Changes", L"Reset to previous saved settings", 8, 5, 8, 35 },
  { L"Load Setup Defaults", L"Load factory default settings", 9, 5, 9, 35 }
};

CHAR16 *mMonthNames[] = {
  L"January", L"February", L"March", L"April", L"May", L"June",
  L"July", L"August", L"September", L"October", L"November", L"December"
};

LOGO_ENTRY mLogos[] = {
  {
    IMAGE_TOKEN (IMG_LOGO),
    EdkiiPlatformLogoDisplayAttributeCenter,
    0,
    0
  }
};

// Function Declarations
EFI_STATUS PerformUsbAuthentication(VOID);
VOID DisplayBiosSetupForm(VOID);
VOID GetCurrentTime(OUT CHAR16 *TimeString);
VOID HandleBiosSetupNavigation(VOID);
VOID DrawBlueBackgroundOnce(VOID);
VOID UpdateTabsOnly(VOID);
VOID UpdateSubmenuOnly(VOID);
VOID UpdateInfoPanelOnly(VOID);
VOID DrawInitialInterface(VOID);
VOID HandleTabSelection(VOID);
SUBMENU_ITEM* GetCurrentSubmenuArray(OUT UINTN *ItemCount);
VOID GetConsoleSize(VOID);
VOID InitializeCalendar(VOID);
VOID DisplayCalendar(VOID);
VOID HandleCalendarNavigation(VOID);
BOOLEAN IsLeapYear(UINT16 Year);
UINT8 GetDaysInMonth(UINT16 Year, UINT8 Month);
VOID SetSystemTime(VOID);
VOID SetupLogoTiming(VOID);
VOID EFIAPI KeyboardMonitor(IN EFI_EVENT Event, IN VOID *Context);
VOID EFIAPI TimerCallback(IN EFI_EVENT Event, IN VOID *Context);


**/


//
// USB Authentication Implementation
//

/**
EFI_STATUS
PerformUsbAuthentication(
  VOID
  )
{
  EFI_USB_DEVICE_DESCRIPTOR DevDesc;
  EFI_USB_INTERFACE_DESCRIPTOR IfDesc;
  EFI_USB_IO_PROTOCOL *UsbIo;
  EFI_STATUS Status = EFI_SUCCESS;
  EFI_HANDLE *HandleBuffer = NULL;
  CHAR16 *Manufacturer;
  CHAR16 *Product;
  CHAR16 *SerialNumber;
  UINT16 *LangIdTable;
  UINT16 TableSize;
  UINTN HandleCount;
  UINT8 Attempts = 0;
  BOOLEAN DeviceFound = FALSE;

  DEBUG((DEBUG_INFO, "Starting USB Authentication Process\n"));

  // Clear screen for authentication display
  gST->ConOut->ClearScreen(gST->ConOut);
  gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
  
  Print(L"\n");
  Print(L"=== USB Device Authentication ===\n");
  Print(L"Scanning for authorized USB devices...\n\n");

  // Locate USB devices
  Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiUsbIoProtocolGuid, NULL, &HandleCount, &HandleBuffer);
  if (EFI_ERROR(Status)) {
    Print(L"ERROR: Could not locate USB devices.\n");
    return Status;
  }

  Print(L"Found %d USB device(s)\n", HandleCount);
  Print(L"\n  VendorID  ProductID  Manufacturer/Product/SerialNumber\n");
  Print(L"  -------------------------------------------------------\n\n");

  for (UINT8 Index = 0; Index < HandleCount && Attempts < MAX_ATTEMPTS; Index++) {
    Status = gBS->HandleProtocol(HandleBuffer[Index], &gEfiUsbIoProtocolGuid, (VOID**)&UsbIo);
    if (EFI_ERROR(Status)) {
      Print(L"ERROR: Failed to open USB I/O protocol.\n");
      continue;
    }

    Status = UsbIo->UsbGetDeviceDescriptor(UsbIo, &DevDesc);
    if (EFI_ERROR(Status)) {
      Print(L"ERROR: Failed to get device descriptor.\n");
      continue;
    }

    Status = UsbIo->UsbGetInterfaceDescriptor(UsbIo, &IfDesc);
    if (EFI_ERROR(Status)) {
      Print(L"ERROR: Failed to get interface descriptor.\n");
      continue;
    }

    // Get string descriptors
    TableSize = 0;
    LangIdTable = NULL;
    Status = UsbIo->UsbGetSupportedLanguages(UsbIo, &LangIdTable, &TableSize);
    if (LangIdTable != NULL) {
      FreePool(LangIdTable);
    }

    Status = UsbIo->UsbGetStringDescriptor(UsbIo, 0x0409, DevDesc.StrManufacturer, &Manufacturer);
    if (EFI_ERROR(Status)) {
      Manufacturer = L"Unknown";
    }

    Status = UsbIo->UsbGetStringDescriptor(UsbIo, 0x0409, DevDesc.StrProduct, &Product);
    if (EFI_ERROR(Status)) {
      Product = L"Unknown";
    }

    Status = UsbIo->UsbGetStringDescriptor(UsbIo, 0x0409, DevDesc.StrSerialNumber, &SerialNumber);
    if (EFI_ERROR(Status)) {
      SerialNumber = L"Unknown";
    }

    // Display device information
    Print(L"    %04X      %04X     %s, %s, %s\n",
          DevDesc.IdVendor, DevDesc.IdProduct, Manufacturer, Product, SerialNumber);

    // Check authentication
    if (DevDesc.IdVendor == FIXED_VENDOR_ID && 
        DevDesc.IdProduct == FIXED_PRODUCT_ID && 
        StrCmp(SerialNumber, FIXED_SERIAL_NUMBER) == 0) {
      
      Print(L"\n*** AUTHENTICATION SUCCESSFUL ***\n");
      Print(L"Welcome, Authorized User!\n");
      mUsbAuthenticated = TRUE;
      DeviceFound = TRUE;
      break;
    } else {
      Attempts++;
      if (Attempts >= MAX_ATTEMPTS) {
        Print(L"\n*** AUTHENTICATION FAILED ***\n");
        Print(L"Maximum unauthorized attempts reached.\n");
        Print(L"System will shutdown in 3 seconds...\n");
        gBS->Stall(3000000); // 3 seconds
        FreePool(HandleBuffer);
        return EFI_ACCESS_DENIED;
      }
    }

    // Free string descriptors if they were allocated
    if (Manufacturer != L"Unknown") {
      FreePool(Manufacturer);
    }
    if (Product != L"Unknown") {
      FreePool(Product);
    }
    if (SerialNumber != L"Unknown") {
      FreePool(SerialNumber);
    }
  }

  if (!DeviceFound && Attempts == 0) {
    Print(L"\n*** AUTHENTICATION FAILED ***\n");
    Print(L"No authorized USB device found.\n");
    Print(L"System will shutdown in 3 seconds...\n");
    gBS->Stall(3000000);
    FreePool(HandleBuffer);
    return EFI_ACCESS_DENIED;
  }

  Print(L"\nPress any key to continue to BIOS Setup...\n");
  
  // Wait for key press
  EFI_INPUT_KEY Key;
  while (gST->ConIn->ReadKeyStroke(gST->ConIn, &Key) == EFI_NOT_READY) {
    gBS->Stall(100000); // 100ms
  }

  FreePool(HandleBuffer);
  return EFI_SUCCESS;
}
**/


/**
EFI_STATUS
PerformUsbAuthentication(
  VOID
  )
{
  EFI_USB_DEVICE_DESCRIPTOR DevDesc;
  EFI_USB_INTERFACE_DESCRIPTOR IfDesc;
  EFI_USB_IO_PROTOCOL *UsbIo;
  EFI_STATUS Status = EFI_SUCCESS;
  EFI_HANDLE *HandleBuffer = NULL;
  CHAR16 *Manufacturer = NULL;
  CHAR16 *Product = NULL;
  CHAR16 *SerialNumber = NULL;
  UINT16 *LangIdTable = NULL;
  UINT16 TableSize;
  UINTN HandleCount;
  UINT8 Attempts = 0;
  BOOLEAN DeviceFound = FALSE;

  DEBUG((DEBUG_INFO, "Starting USB Authentication Process\n"));

  // Clear screen for authentication display
  gST->ConOut->ClearScreen(gST->ConOut);
  gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
  
  Print(L"\n");
  Print(L"=== USB Device Authentication ===\n");
  Print(L"Scanning for authorized USB devices...\n\n");

  // Locate USB devices
  Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiUsbIoProtocolGuid, NULL, &HandleCount, &HandleBuffer);
  if (EFI_ERROR(Status)) {
    Print(L"ERROR: Could not locate USB devices.\n");
    Print(L"Status: %r\n", Status);
    return Status;
  }

  Print(L"Found %d USB device(s)\n", HandleCount);
  Print(L"\n  VendorID  ProductID  Manufacturer/Product/SerialNumber\n");
  Print(L"  -------------------------------------------------------\n\n");

  for (UINT8 Index = 0; Index < HandleCount && Attempts < MAX_ATTEMPTS; Index++) {
    // Reset string pointers for each device
    Manufacturer = NULL;
    Product = NULL;
    SerialNumber = NULL;
    
    Status = gBS->HandleProtocol(HandleBuffer[Index], &gEfiUsbIoProtocolGuid, (VOID**)&UsbIo);
    if (EFI_ERROR(Status)) {
      Print(L"ERROR: Failed to open USB I/O protocol for device %d.\n", Index);
      continue;
    }

    Status = UsbIo->UsbGetDeviceDescriptor(UsbIo, &DevDesc);
    if (EFI_ERROR(Status)) {
      Print(L"ERROR: Failed to get device descriptor for device %d.\n", Index);
      continue;
    }

    Status = UsbIo->UsbGetInterfaceDescriptor(UsbIo, &IfDesc);
    if (EFI_ERROR(Status)) {
      Print(L"ERROR: Failed to get interface descriptor for device %d.\n", Index);
      continue;
    }

    // Get supported languages first
    TableSize = 0;
    Status = UsbIo->UsbGetSupportedLanguages(UsbIo, &LangIdTable, &TableSize);
    if (EFI_ERROR(Status) || LangIdTable == NULL) {
      Print(L"WARNING: No language support found for device %d\n", Index);
      // Use default strings
      Manufacturer = L"Unknown";
      Product = L"Unknown";  
      SerialNumber = L"Unknown";
    } else {
      // Try to get string descriptors with first supported language
      UINT16 LangId = LangIdTable[0]; // Use first supported language
      
      // Get Manufacturer string
      if (DevDesc.StrManufacturer != 0) {
        Status = UsbIo->UsbGetStringDescriptor(UsbIo, LangId, DevDesc.StrManufacturer, &Manufacturer);
        if (EFI_ERROR(Status)) {
          Manufacturer = L"Unknown";
        }
      } else {
        Manufacturer = L"No Manufacturer";
      }

      // Get Product string  
      if (DevDesc.StrProduct != 0) {
        Status = UsbIo->UsbGetStringDescriptor(UsbIo, LangId, DevDesc.StrProduct, &Product);
        if (EFI_ERROR(Status)) {
          Product = L"Unknown";
        }
      } else {
        Product = L"No Product";
      }

      // Get Serial Number string
      if (DevDesc.StrSerialNumber != 0) {
        Status = UsbIo->UsbGetStringDescriptor(UsbIo, LangId, DevDesc.StrSerialNumber, &SerialNumber);
        if (EFI_ERROR(Status)) {
          SerialNumber = L"Unknown";
        }
      } else {
        SerialNumber = L"No Serial";
      }
      
      // Free the language table
      if (LangIdTable != NULL) {
        FreePool(LangIdTable);
        LangIdTable = NULL;
      }
    }

    // Display device information - ensure strings are not NULL
    if (Manufacturer == NULL) Manufacturer = L"NULL";
    if (Product == NULL) Product = L"NULL";
    if (SerialNumber == NULL) SerialNumber = L"NULL";

    Print(L"  %04X      %04X     %s / %s / %s\n",
          DevDesc.IdVendor, DevDesc.IdProduct, Manufacturer, Product, SerialNumber);

    // Check authentication
    if (DevDesc.IdVendor == FIXED_VENDOR_ID && 
        DevDesc.IdProduct == FIXED_PRODUCT_ID) {
        
      // For serial number comparison, handle the case where we might not have retrieved it properly
      if (SerialNumber != NULL && 
          StrCmp(SerialNumber, L"Unknown") != 0 && 
          StrCmp(SerialNumber, L"NULL") != 0 &&
          StrCmp(SerialNumber, L"No Serial") != 0 &&
          StrCmp(SerialNumber, FIXED_SERIAL_NUMBER) == 0) {
        
        Print(L"\n*** AUTHENTICATION SUCCESSFUL ***\n");
        Print(L"Welcome, Authorized User!\n");
        mUsbAuthenticated = TRUE;
        DeviceFound = TRUE;
        
        // Free allocated strings before breaking
        if (Manufacturer != NULL && StrCmp(Manufacturer, L"Unknown") != 0 && 
            StrCmp(Manufacturer, L"NULL") != 0 && StrCmp(Manufacturer, L"No Manufacturer") != 0) {
          FreePool(Manufacturer);
        }
        if (Product != NULL && StrCmp(Product, L"Unknown") != 0 && 
            StrCmp(Product, L"NULL") != 0 && StrCmp(Product, L"No Product") != 0) {
          FreePool(Product);
        }
        if (SerialNumber != NULL && StrCmp(SerialNumber, L"Unknown") != 0 && 
            StrCmp(SerialNumber, L"NULL") != 0 && StrCmp(SerialNumber, L"No Serial") != 0) {
          FreePool(SerialNumber);
        }
        
        break;
      } else {
        Print(L"    Vendor/Product ID matches but serial number differs\n");
        Print(L"    Expected: %s\n", FIXED_SERIAL_NUMBER);
        Print(L"    Found: %s\n", SerialNumber);
        Attempts++;
      }
    } else {
      Attempts++;
    }

    // Free string descriptors if they were allocated (not static strings)
    if (Manufacturer != NULL && StrCmp(Manufacturer, L"Unknown") != 0 && 
        StrCmp(Manufacturer, L"NULL") != 0 && StrCmp(Manufacturer, L"No Manufacturer") != 0) {
      FreePool(Manufacturer);
    }
    if (Product != NULL && StrCmp(Product, L"Unknown") != 0 && 
        StrCmp(Product, L"NULL") != 0 && StrCmp(Product, L"No Product") != 0) {
      FreePool(Product);
    }
    if (SerialNumber != NULL && StrCmp(SerialNumber, L"Unknown") != 0 && 
        StrCmp(SerialNumber, L"NULL") != 0 && StrCmp(SerialNumber, L"No Serial") != 0) {
      FreePool(SerialNumber);
    }

    // Check if maximum attempts reached
    if (Attempts >= MAX_ATTEMPTS) {
      Print(L"\n*** AUTHENTICATION FAILED ***\n");
      Print(L"Maximum unauthorized attempts reached.\n");
      Print(L"System will shutdown in 3 seconds...\n");
      gBS->Stall(3000000); // 3 seconds
      FreePool(HandleBuffer);
      return EFI_ACCESS_DENIED;
    }
  }

  if (!DeviceFound && Attempts == 0) {
    Print(L"\n*** AUTHENTICATION FAILED ***\n");
    Print(L"No authorized USB device found.\n");
    Print(L"System will shutdown in 3 seconds...\n");
    gBS->Stall(3000000);
    FreePool(HandleBuffer);
    return EFI_ACCESS_DENIED;
  }

  if (!DeviceFound && Attempts > 0) {
    Print(L"\n*** AUTHENTICATION FAILED ***\n");
    Print(L"USB devices found but none match authorization criteria.\n");
    Print(L"System will shutdown in 3 seconds...\n");
    gBS->Stall(3000000);
    FreePool(HandleBuffer);
    return EFI_ACCESS_DENIED;
  }

  Print(L"\nPress any key to continue to BIOS Setup...\n");
  
  // Wait for key press
  EFI_INPUT_KEY Key;
  while (gST->ConIn->ReadKeyStroke(gST->ConIn, &Key) == EFI_NOT_READY) {
    gBS->Stall(100000); // 100ms
  }

  FreePool(HandleBuffer);
  return EFI_SUCCESS;
}

**/




//
// Calendar Functions
//

/**

BOOLEAN
IsLeapYear(
  UINT16 Year
  )
{
  return ((Year % 4 == 0) && (Year % 100 != 0)) || (Year % 400 == 0);
}

UINT8
GetDaysInMonth(
  UINT16 Year,
  UINT8  Month
  )
{
  UINT8 DaysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  
  if (Month == 2 && IsLeapYear(Year)) {
    return 29;
  }
  
  return DaysInMonth[Month - 1];
}

VOID
InitializeCalendar(
  VOID
  )
{
  EFI_TIME    Time;
  EFI_STATUS  Status;
  
  Status = gRT->GetTime(&Time, NULL);
  if (!EFI_ERROR(Status)) {
    mCalendar.Year = Time.Year;
    mCalendar.Month = Time.Month;
    mCalendar.Day = Time.Day;
    mCalendar.Hour = Time.Hour;
    mCalendar.Minute = Time.Minute;
    mCalendar.Second = Time.Second;
  } else {
    mCalendar.Year = 2024;
    mCalendar.Month = 1;
    mCalendar.Day = 1;
    mCalendar.Hour = 0;
    mCalendar.Minute = 0;
    mCalendar.Second = 0;
  }
  
  mCalendar.SelectedField = 0;
  mCalendar.InCalendarMode = TRUE;
}

VOID
SetSystemTime(
  VOID
  )
{
  EFI_TIME Time;
  EFI_STATUS Status;
  
  Time.Year = mCalendar.Year;
  Time.Month = mCalendar.Month;
  Time.Day = mCalendar.Day;
  Time.Hour = mCalendar.Hour;
  Time.Minute = mCalendar.Minute;
  Time.Second = mCalendar.Second;
  Time.Nanosecond = 0;
  Time.TimeZone = EFI_UNSPECIFIED_TIMEZONE;
  Time.Daylight = 0;
  
  Status = gRT->SetTime(&Time);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "Failed to set system time: %r\n", Status));
  }
}

**/



/**
VOID
DisplayCalendar(
  VOID
  )
{
  UINTN Row, Col;
//  UINTN SeparatorCol = (mConsoleColumns * 3) / 4;
  // Remove the unused RightStart variable
  
  // Clear the interface area
  for (Row = 4; Row < mConsoleRows - 3; Row++) {
    gST->ConOut->SetCursorPosition(gST->ConOut, 1, Row);
    gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    for (Col = 1; Col < mConsoleColumns - 1; Col++) {
      Print(L" ");
    }
  }
  
  // Calendar title
  gST->ConOut->SetCursorPosition(gST->ConOut, 3, 5);
  gST->ConOut->SetAttribute(gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
  Print(L"Manual Date & Time Setting");
  
  // Current date/time display
  gST->ConOut->SetCursorPosition(gST->ConOut, 3, 7);
  gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  Print(L"Current Date & Time:");
  
  // Date fields with selection highlighting
  gST->ConOut->SetCursorPosition(gST->ConOut, 3, 9);
  
  // Year field
  if (mCalendar.SelectedField == 0) {
    gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  } else {
    gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  }
  Print(L"[%04d]", mCalendar.Year);
  
  gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  Print(L" / ");
  
  // Month field
  if (mCalendar.SelectedField == 1) {
    gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  } else {
    gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  }
  Print(L"[%02d]", mCalendar.Month);
  
  gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  Print(L" / ");
  
  // Day field
  if (mCalendar.SelectedField == 2) {
    gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  } else {
    gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  }
  Print(L"[%02d]", mCalendar.Day);
  
  // Time fields
  gST->ConOut->SetCursorPosition(gST->ConOut, 3, 11);
  
  // Hour field
  if (mCalendar.SelectedField == 3) {
    gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  } else {
    gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  }
  Print(L"[%02d]", mCalendar.Hour);
  
  gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  Print(L" : ");
  
  // Minute field
  if (mCalendar.SelectedField == 4) {
    gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  } else {
    gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  }
  Print(L"[%02d]", mCalendar.Minute);
  
  gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  Print(L" : ");
  
  // Second field
  if (mCalendar.SelectedField == 5) {
    gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  } else {
    gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  }
  Print(L"[%02d]", mCalendar.Second);
  
  // Month name display
  if (mCalendar.Month >= 1 && mCalendar.Month <= 12) {
    gST->ConOut->SetCursorPosition(gST->ConOut, 3, 13);
    gST->ConOut->SetAttribute(gST->ConOut, EFI_DARKGRAY | EFI_BACKGROUND_LIGHTGRAY);
    Print(L"Month: %s", mMonthNames[mCalendar.Month - 1]);
  }
  
  // Instructions
  gST->ConOut->SetCursorPosition(gST->ConOut, 3, 15);
  gST->ConOut->SetAttribute(gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
  Print(L"Instructions:");
  
  gST->ConOut->SetCursorPosition(gST->ConOut, 3, 16);
  gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  Print(L"Left/Right: Select field");
  
  gST->ConOut->SetCursorPosition(gST->ConOut, 3, 17);
  Print(L"Up/Down: Change value");
  
  gST->ConOut->SetCursorPosition(gST->ConOut, 3, 18);
  Print(L"Enter: Save changes");
  
  gST->ConOut->SetCursorPosition(gST->ConOut, 3, 19);
  Print(L"ESC: Cancel and return");
}
**/


/**

VOID
HandleCalendarNavigation(
  VOID
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  BOOLEAN        ValueChanged = FALSE;
  UINT8          MaxDays;
  
  while (mCalendar.InCalendarMode && mInBiosSetup) {
    Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
    
    if (!EFI_ERROR(Status)) {
      switch (Key.ScanCode) {
        case SCAN_LEFT:
          if (mCalendar.SelectedField > 0) {
            mCalendar.SelectedField--;
          } else {
            mCalendar.SelectedField = 5;
          }
          DisplayCalendar();
          break;
          
        case SCAN_RIGHT:
          if (mCalendar.SelectedField < 5) {
            mCalendar.SelectedField++;
          } else {
            mCalendar.SelectedField = 0;
          }
          DisplayCalendar();
          break;
          
        case SCAN_UP:
          ValueChanged = TRUE;
          switch (mCalendar.SelectedField) {
            case 0: // Year
              if (mCalendar.Year < 2099) mCalendar.Year++;
              break;
            case 1: // Month
              if (mCalendar.Month < 12) {
                mCalendar.Month++;
              } else {
                mCalendar.Month = 1;
              }
              break;
            case 2: // Day
              MaxDays = GetDaysInMonth(mCalendar.Year, mCalendar.Month);
              if (mCalendar.Day < MaxDays) {
                mCalendar.Day++;
              } else {
                mCalendar.Day = 1;
              }
              break;
            case 3: // Hour
              if (mCalendar.Hour < 23) {
                mCalendar.Hour++;
              } else {
                mCalendar.Hour = 0;
              }
              break;
            case 4: // Minute
              if (mCalendar.Minute < 59) {
                mCalendar.Minute++;
              } else {
                mCalendar.Minute = 0;
              }
              break;
            case 5: // Second
              if (mCalendar.Second < 59) {
                mCalendar.Second++;
              } else {
                mCalendar.Second = 0;
              }
              break;
          }
          DisplayCalendar();
          break;
          
        case SCAN_DOWN:
          ValueChanged = TRUE;
          switch (mCalendar.SelectedField) {
            case 0: // Year
              if (mCalendar.Year > 1900) mCalendar.Year--;
              break;
            case 1: // Month
              if (mCalendar.Month > 1) {
                mCalendar.Month--;
              } else {
                mCalendar.Month = 12;
              }
              break;
            case 2: // Day
              if (mCalendar.Day > 1) {
                mCalendar.Day--;
              } else {
                MaxDays = GetDaysInMonth(mCalendar.Year, mCalendar.Month);
                mCalendar.Day = MaxDays;
              }
              break;
            case 3: // Hour
              if (mCalendar.Hour > 0) {
                mCalendar.Hour--;
              } else {
                mCalendar.Hour = 23;
              }
              break;
            case 4: // Minute
              if (mCalendar.Minute > 0) {
                mCalendar.Minute--;
              } else {
                mCalendar.Minute = 59;
              }
              break;
            case 5: // Second
              if (mCalendar.Second > 0) {
                mCalendar.Second--;
              } else {
                mCalendar.Second = 59;
              }
              break;
          }
          DisplayCalendar();
          break;
          
        case SCAN_ESC:
          mCalendar.InCalendarMode = FALSE;
          mCurrentMenuLevel = MENU_LEVEL_MAIN_SUBMENU;
          DrawInitialInterface();
          return;
          
        default:
          if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
            SetSystemTime();
            mCalendar.InCalendarMode = FALSE;
            mCurrentMenuLevel = MENU_LEVEL_MAIN_SUBMENU;
            
            gST->ConOut->SetCursorPosition(gST->ConOut, 3, 21);
            gST->ConOut->SetAttribute(gST->ConOut, EFI_GREEN | EFI_BACKGROUND_LIGHTGRAY);
            Print(L"Date & Time saved successfully!");
            gBS->Stall(1500000);
            
            DrawInitialInterface();
            return;
          }
          break;
      }
      
      if (ValueChanged && (mCalendar.SelectedField == 0 || mCalendar.SelectedField == 1)) {
        MaxDays = GetDaysInMonth(mCalendar.Year, mCalendar.Month);
        if (mCalendar.Day > MaxDays) {
          mCalendar.Day = MaxDays;
          DisplayCalendar();
        }
      }
    } else {
      gBS->Stall(50000);
    }
  }
}
**/



//
// BIOS Setup Interface Functions
//



/**
VOID
GetConsoleSize(
  VOID
  )
{
  EFI_STATUS Status;
  UINTN MaxMode, MaxColumns, MaxRows;
  UINTN BestMode = 0;
  UINTN BestColumns = 80;
  UINTN BestRows = 25;
  
  MaxMode = gST->ConOut->Mode->MaxMode;
  
  for (UINTN Mode = 0; Mode < MaxMode; Mode++) {
    Status = gST->ConOut->QueryMode(
                            gST->ConOut,
                            Mode,
                            &MaxColumns,
                            &MaxRows
                            );
    
    if (!EFI_ERROR(Status)) {
      if ((MaxColumns >= BestColumns && MaxRows >= BestRows) && 
          (MaxColumns > BestColumns || MaxRows > BestRows)) {
        BestMode = Mode;
        BestColumns = MaxColumns;
        BestRows = MaxRows;
      }
    }
  }
  
  if (BestMode != gST->ConOut->Mode->Mode) {
    Status = gST->ConOut->SetMode(gST->ConOut, BestMode);
    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_WARN, "Failed to set console mode %d: %r\n", BestMode, Status));
    }
  }
  
  Status = gST->ConOut->QueryMode(
                          gST->ConOut,
                          gST->ConOut->Mode->Mode,
                          &mConsoleColumns,
                          &mConsoleRows
                          );
  
  if (EFI_ERROR(Status)) {
    mConsoleColumns = 100;
    mConsoleRows = 40;
  }
  
  DEBUG((DEBUG_INFO, "Console size: %dx%d\n", mConsoleColumns, mConsoleRows));
}

SUBMENU_ITEM*
GetCurrentSubmenuArray(
  OUT UINTN *ItemCount
  )
{
  switch (mSelectedMainTab) {
    case 0: // Main
      *ItemCount = ARRAY_SIZE(mMainSubmenu);
      return mMainSubmenu;
      
    case 1: // Advanced
      *ItemCount = ARRAY_SIZE(mAdvancedSubmenu);
      return mAdvancedSubmenu;
      
    case 2: // Security
      *ItemCount = ARRAY_SIZE(mSecuritySubmenu);
      return mSecuritySubmenu;
      
    case 3: // Save & Exit
      *ItemCount = ARRAY_SIZE(mSaveExitSubmenu);
      return mSaveExitSubmenu;
      
    default:
      *ItemCount = ARRAY_SIZE(mMainSubmenu);
      return mMainSubmenu;
  }
}

VOID
GetCurrentTime(
  OUT CHAR16  *TimeString
  )
{
  EFI_TIME    Time;
  EFI_STATUS  Status;
  
  Status = gRT->GetTime(&Time, NULL);
  if (!EFI_ERROR(Status)) {
    UnicodeSPrint(
      TimeString, 
      50 * sizeof(CHAR16),
      L"%02d/%02d/%d  %02d:%02d:%02d",
      Time.Day,
      Time.Month,
      Time.Year,
      Time.Hour,
      Time.Minute,
      Time.Second
    );
  } else {
    UnicodeSPrint(TimeString, 50 * sizeof(CHAR16), L"[Date/Time Not Available]");
  }
}

VOID
DrawBlueBackgroundOnce(
  VOID
  )
{
  UINTN Index, Col;
  
  GetConsoleSize();
  
  gST->ConOut->ClearScreen(gST->ConOut);
  gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  
  for (Index = 0; Index < mConsoleRows; Index++) {
    gST->ConOut->SetCursorPosition(gST->ConOut, 0, Index);
    for (Col = 0; Col < mConsoleColumns; Col++) {
      Print(L" ");
    }
  }
  
  gST->ConOut->SetCursorPosition(gST->ConOut, 1, 0);
  gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  Print(L"BIOS Setup Utility");
  
  gST->ConOut->SetCursorPosition(gST->ConOut, (mConsoleColumns - 14) / 2, 1);
  gST->ConOut->SetAttribute(gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);
  Print(L"BHARAT BIOS CDAC");
}

VOID
UpdateTabsOnly(
  VOID
  )
{
  UINTN Index, Col;
  
  gST->ConOut->SetCursorPosition(gST->ConOut, 0, 2);
  gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  for (Col = 0; Col < mConsoleColumns; Col++) {
    Print(L" ");
  }
  
  for (Index = 0; Index < mTotalMainTabs; Index++) {
    gST->ConOut->SetCursorPosition(gST->ConOut, mMainTabs[Index].StartCol, 2);
    
    if (Index == mSelectedMainTab) {
      gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    } else {
      gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
    }
    
    Print(L" %s ", mMainTabs[Index].MenuText);
  }
}

VOID
UpdateSubmenuOnly(
  VOID
  )
{
  UINTN Index, Col;
  SUBMENU_ITEM *CurrentSubmenu;
  UINTN ItemCount;
  UINTN SeparatorCol = (mConsoleColumns * 3) / 4;
  
  CurrentSubmenu = GetCurrentSubmenuArray(&ItemCount);
  mTotalSubItems = ItemCount;
  
  for (Index = 4; Index < mConsoleRows - 3; Index++) {
    gST->ConOut->SetCursorPosition(gST->ConOut, 1, Index);
    gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    for (Col = 1; Col < SeparatorCol - 1; Col++) {
      Print(L" ");
    }
  }
  
  for (Index = 0; Index < ItemCount && Index < 15; Index++) {
    gST->ConOut->SetCursorPosition(gST->ConOut, 3, Index + 5);
    
    if (Index == mSelectedSubItem) {
      gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
    } else {
      gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    }
    
    Print(L"%s", CurrentSubmenu[Index].MenuText);
  }
}

VOID
UpdateInfoPanelOnly(
  VOID
  )
{
  SUBMENU_ITEM *CurrentSubmenu;
  UINTN ItemCount;
  CHAR16 TimeString[100];
  UINTN SeparatorCol = (mConsoleColumns * 3) / 4;
  UINTN RightStart = SeparatorCol + 2;
  UINTN Index, Col;
  
  CurrentSubmenu = GetCurrentSubmenuArray(&ItemCount);
  
  for (Index = 4; Index < mConsoleRows - 3; Index++) {
    gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, Index);
    gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    for (Col = RightStart; Col < mConsoleColumns - 1; Col++) {
      Print(L" ");
    }
  }
  
  gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, 5);
  gST->ConOut->SetAttribute(gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
  Print(L"Selected Item:");
  
  if (mSelectedSubItem < ItemCount) {
    gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, 7);
    gST->ConOut->SetAttribute(gST->ConOut, EFI_DARKGRAY | EFI_BACKGROUND_LIGHTGRAY);
    Print(L"%s", CurrentSubmenu[mSelectedSubItem].MenuText);
    
    gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, 9);
    gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    
    UINTN RightWidth = mConsoleColumns - RightStart - 2;
    CHAR16 *Description = CurrentSubmenu[mSelectedSubItem].Description;
    UINTN DescLen = StrLen(Description);
    
    if (DescLen <= RightWidth) {
      Print(L"%s", Description);
    } else {
      UINTN CurrentPos = 0;
      UINTN CurrentLine = 9;
      
      while (CurrentPos < DescLen && CurrentLine < mConsoleRows - 6) {
        gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, CurrentLine);
        gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
        
        UINTN CharsToShow = RightWidth;
        if (CurrentPos + CharsToShow > DescLen) {
          CharsToShow = DescLen - CurrentPos;
        }
        
        if (CurrentPos + CharsToShow < DescLen) {
          UINTN LastSpace = CharsToShow;
          for (UINTN i = CharsToShow; i > 0; i--) {
            if (Description[CurrentPos + i - 1] == L' ') {
              LastSpace = i;
              break;
            }
          }
          if (LastSpace < CharsToShow) {
            CharsToShow = LastSpace;
          }
        }
        
        for (UINTN i = 0; i < CharsToShow && CurrentPos < DescLen; i++) {
          Print(L"%c", Description[CurrentPos]);
          CurrentPos++;
        }
        
        if (CurrentPos < DescLen && Description[CurrentPos] == L' ') {
          CurrentPos++;
        }
        
        CurrentLine++;
      }
    }
    
    // Special case for Date & Time
    if (mSelectedMainTab == 0 && mSelectedSubItem == 3) {
      GetCurrentTime(TimeString);
      gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, 12);
      gST->ConOut->SetAttribute(gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
      Print(L"Current:");
      gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, 13);
      
      UINTN TimeLen = StrLen(TimeString);
      if (TimeLen <= RightWidth) {
        Print(L"%s", TimeString);
      } else {
        Print(L"%.15s", TimeString);
        gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, 14);
        Print(L"%s", &TimeString[15]);
      }
      
      gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, 16);
      Print(L"Press Enter to");
      gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, 17);
      Print(L"open calendar");
    }
    
    // Special case for USB Authentication
    if (mSelectedMainTab == 0 && mSelectedSubItem == 4) {
      gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, 12);
      gST->ConOut->SetAttribute(gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
      Print(L"Status:");
      gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, 13);
      
      if (mUsbAuthenticated) {
        gST->ConOut->SetAttribute(gST->ConOut, EFI_GREEN | EFI_BACKGROUND_LIGHTGRAY);
        Print(L"Authenticated");
      } else {
        gST->ConOut->SetAttribute(gST->ConOut, EFI_RED | EFI_BACKGROUND_LIGHTGRAY);
        Print(L"Not Authenticated");
      }
      
      gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, 15);
      gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
      Print(L"Press Enter to");
      gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, 16);
      Print(L"re-authenticate");
    }
  }
  
  gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, mConsoleRows - 5);
  gST->ConOut->SetAttribute(gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
  Print(L"Tab: %s", mMainTabs[mSelectedMainTab].MenuText);
}

VOID
DrawInitialInterface(
  VOID
  )
{
  UINTN Row, Col;
  UINTN SeparatorCol = (mConsoleColumns * 3) / 4;
  
  DrawBlueBackgroundOnce();
  
  for (Row = 3; Row < mConsoleRows - 2; Row++) {
    gST->ConOut->SetCursorPosition(gST->ConOut, 0, Row);
    
    gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    for (Col = 0; Col < SeparatorCol; Col++) {
      Print(L" ");
    }
    
    if (SeparatorCol < mConsoleColumns) {
      gST->ConOut->SetAttribute(gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
      Print(L"│");
    }
    
    gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    for (Col = SeparatorCol + 1; Col < mConsoleColumns; Col++) {
      Print(L" ");
    }
  }
  
  gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  for (Row = mConsoleRows - 2; Row < mConsoleRows; Row++) {
    gST->ConOut->SetCursorPosition(gST->ConOut, 0, Row);
    for (Col = 0; Col < mConsoleColumns; Col++) {
      Print(L" ");
    }
  }
  
  gST->ConOut->SetCursorPosition(gST->ConOut, 2, mConsoleRows - 1);
  gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
  Print(L"←→: Select Screen  ↑↓: Select Item  Enter: Select  ESC: Exit");
  
  UpdateTabsOnly();
  UpdateSubmenuOnly();
  UpdateInfoPanelOnly();
  
  mInterfaceInitialized = TRUE;
}

VOID
HandleTabSelection(
  VOID
  )
{
  UINTN SeparatorCol = (mConsoleColumns * 3) / 4;
  UINTN RightStart = SeparatorCol + 2;
  EFI_STATUS Status;
  
  // Special case: Change Date & Time
  if (mSelectedMainTab == 0 && mSelectedSubItem == 3) {
    mCurrentMenuLevel = MENU_LEVEL_CALENDAR;
    InitializeCalendar();
    DisplayCalendar();
    HandleCalendarNavigation();
    return;
  }
  
  // Special case: USB Authentication
  if (mSelectedMainTab == 0 && mSelectedSubItem == 4) {
    // Show authentication in progress message
    gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, 18);
    gST->ConOut->SetAttribute(gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_LIGHTGRAY);
    Print(L"Authenticating...");
    
    Status = PerformUsbAuthentication();
    
    if (Status == EFI_SUCCESS) {
      mUsbAuthenticated = TRUE;
    } else {
      mUsbAuthenticated = FALSE;
      // System will shutdown from PerformUsbAuthentication if authentication fails
      return;
    }
    
    // Redraw interface after authentication
    DrawInitialInterface();
    return;
  }
  
  switch (mSelectedMainTab) {
    case 3: // Save & Exit
      if (mSelectedSubItem == 0) { // Save Changes and Exit
        gST->ConOut->ClearScreen(gST->ConOut);
        gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
        gST->ConOut->SetCursorPosition(gST->ConOut, (mConsoleColumns - 35) / 2, mConsoleRows / 2);
        gST->ConOut->SetAttribute(gST->ConOut, EFI_GREEN | EFI_BACKGROUND_BLACK);
        Print(L"Settings saved. Shutting down system...");
        gBS->Stall(2000000);
        gRT->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
        mInBiosSetup = FALSE;
      } else if (mSelectedSubItem == 1) { // Discard Changes and Exit
        gST->ConOut->ClearScreen(gST->ConOut);
        gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
        gST->ConOut->SetCursorPosition(gST->ConOut, (mConsoleColumns - 40) / 2, mConsoleRows / 2);
        gST->ConOut->SetAttribute(gST->ConOut, EFI_RED | EFI_BACKGROUND_BLACK);
        Print(L"Exiting without saving. Shutting down...");
        gBS->Stall(2000000);
        gRT->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
        mInBiosSetup = FALSE;
      } else if (mSelectedSubItem == 2) { // Save Changes
        gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, 15);
        gST->ConOut->SetAttribute(gST->ConOut, EFI_GREEN | EFI_BACKGROUND_LIGHTGRAY);
        Print(L"Saved");
        gBS->Stall(1500000);
        UpdateInfoPanelOnly();
      } else if (mSelectedSubItem == 3) { // Discard Changes
        gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, 15);
        gST->ConOut->SetAttribute(gST->ConOut, EFI_RED | EFI_BACKGROUND_LIGHTGRAY);
        Print(L"Discarded");
        gBS->Stall(1500000);
        UpdateInfoPanelOnly();
      } else if (mSelectedSubItem == 4) { // Load Setup Defaults
        gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, 15);
        gST->ConOut->SetAttribute(gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_LIGHTGRAY);
        Print(L"Defaults");
        gBS->Stall(1500000);
        UpdateInfoPanelOnly();
      }
      break;
      
    default:
      gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, 15);
      gST->ConOut->SetAttribute(gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_LIGHTGRAY);
      Print(L"Action...");
      gBS->Stall(800000);
      UpdateInfoPanelOnly();
      break;
  }
}

VOID
DisplayBiosSetupForm(
  VOID
  )
{
  mSelectedMainTab = 0;
  mSelectedSubItem = 0;
  mTotalMainTabs = ARRAY_SIZE(mMainTabs);
  mInBiosSetup = TRUE;
  mInterfaceInitialized = FALSE;
  mCurrentMenuLevel = MENU_LEVEL_MAIN_SUBMENU;
  
  DrawInitialInterface();
  
  gST->ConIn->Reset(gST->ConIn, FALSE);
  
  HandleBiosSetupNavigation();
}

VOID
HandleBiosSetupNavigation(
  VOID
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  
  while (mInBiosSetup) {
    if (mCurrentMenuLevel == MENU_LEVEL_CALENDAR) {
      HandleCalendarNavigation();
      continue;
    }
    
    Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
    
    if (!EFI_ERROR(Status)) {
      switch (Key.ScanCode) {
        case SCAN_LEFT:
          if (mSelectedMainTab > 0) {
            mSelectedMainTab--;
          } else {
            mSelectedMainTab = mTotalMainTabs - 1;
          }
          mSelectedSubItem = 0;
          
          UpdateTabsOnly();
          UpdateSubmenuOnly();
          UpdateInfoPanelOnly();
          break;
          
        case SCAN_RIGHT:
          if (mSelectedMainTab < mTotalMainTabs - 1) {
            mSelectedMainTab++;
          } else {
            mSelectedMainTab = 0;
          }
          mSelectedSubItem = 0;
          
          UpdateTabsOnly();
          UpdateSubmenuOnly();
          UpdateInfoPanelOnly();
          break;
          
        case SCAN_UP:
          if (mSelectedSubItem > 0) {
            mSelectedSubItem--;
          } else {
            mSelectedSubItem = mTotalSubItems - 1;
          }
          
          UpdateSubmenuOnly();
          UpdateInfoPanelOnly();
          break;
          
        case SCAN_DOWN:
          if (mSelectedSubItem < mTotalSubItems - 1) {
            mSelectedSubItem++;
          } else {
            mSelectedSubItem = 0;
          }
          
          UpdateSubmenuOnly();
          UpdateInfoPanelOnly();
          break;
          
        case SCAN_ESC:
          mInBiosSetup = FALSE;
          gST->ConOut->ClearScreen(gST->ConOut);
          gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
          gST->ConOut->SetCursorPosition(gST->ConOut, (mConsoleColumns - 40) / 2, mConsoleRows / 2);
          Print(L"Exiting BIOS Setup. Shutting down system...");
          gBS->Stall(2000000);
          gRT->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
          return;
          
        default:
          if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
            HandleTabSelection();
            if (!mInBiosSetup) {
              return;
            }
          }
          break;
      }
    } else {
      gBS->Stall(50000);
    }
  }
}
**/




//
// Logo and Event Handling Functions
//


/**
VOID
EFIAPI
KeyboardMonitor(
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  
  if (!mMessageShown || mInBiosSetup) {
    return;
  }
  
  Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
  
  if (!EFI_ERROR(Status)) {
    if (Key.ScanCode == SCAN_F12) {
      DEBUG((DEBUG_INFO, "F12 key pressed - showing BIOS setup form\n"));
      
      gBS->SetTimer(mKeyMonitorEvent, TimerCancel, 0);
      gBS->SetTimer(mTimerEvent, TimerCancel, 0);
      
      DisplayBiosSetupForm();
      
      gST->ConOut->ClearScreen(gST->ConOut);
      gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
      gST->ConOut->SetCursorPosition(gST->ConOut, (mConsoleColumns - 40) / 2, mConsoleRows / 2);
      Print(L"BIOS Setup completed. Shutting down system...");
      gBS->Stall(2000000);
      gRT->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
      return;
    }
  }
}

VOID
EFIAPI
TimerCallback(
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  mTimerCounter++;
  
  if (mTimerCounter == 50 && !mMessageShown) { // 50 * 100ms = 5 seconds
    gST->ConOut->ClearScreen(gST->ConOut);
    
    GetConsoleSize();
    
    gST->ConOut->SetCursorPosition(gST->ConOut, (mConsoleColumns - 28) / 2, mConsoleRows / 2 + 5);
    gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
    Print(L"Press F12 key for BIOS setup");
    
    mMessageShown = TRUE;
    
    gST->ConIn->Reset(gST->ConIn, FALSE);
    
    DEBUG((DEBUG_INFO, "Message displayed, waiting for F12 key\n"));
  }
}

VOID
SetupLogoTiming(
  VOID
  )
{
  EFI_STATUS  Status;
  
  if (mLogoDisplayed) {
    return;
  }
  
  DEBUG((DEBUG_INFO, "Setting up logo timing and keyboard monitoring\n"));
  
  Status = gBS->CreateEvent(
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  TimerCallback,
                  NULL,
                  &mTimerEvent
                  );
  
  if (!EFI_ERROR(Status)) {
    Status = gBS->SetTimer(
                    mTimerEvent,
                    TimerPeriodic,
                    1000000  // 100ms in 100ns units
                    );
    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_ERROR, "Failed to set timer for mTimerEvent: %r\n", Status));
    }
  } else {
    DEBUG((DEBUG_ERROR, "Failed to create mTimerEvent: %r\n", Status));
  }
  
  Status = gBS->CreateEvent(
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  KeyboardMonitor,
                  NULL,
                  &mKeyMonitorEvent
                  );
  
  if (!EFI_ERROR(Status)) {
    Status = gBS->SetTimer(
                    mKeyMonitorEvent,
                    TimerPeriodic,
                    500000  // 50ms in 100ns units
                    );
    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_ERROR, "Failed to set timer for mKeyMonitorEvent: %r\n", Status));
    }
  } else {
    DEBUG((DEBUG_ERROR, "Failed to create mKeyMonitorEvent: %r\n", Status));
  }
  
  mLogoDisplayed = TRUE;
}
**/



//
// Platform Logo Protocol Implementation
//
/**

EFI_STATUS
EFIAPI
GetImage(
  IN     EDKII_PLATFORM_LOGO_PROTOCOL        *This,
  IN OUT UINT32                              *Instance,
  OUT EFI_IMAGE_INPUT                        *Image,
  OUT EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE  *Attribute,
  OUT INTN                                   *OffsetX,
  OUT INTN                                   *OffsetY
  )
{
  UINT32      Current;
  EFI_STATUS  Status;

  if ((Instance == NULL) || (Image == NULL) ||
      (Attribute == NULL) || (OffsetX == NULL) || (OffsetY == NULL))
  {
    return EFI_INVALID_PARAMETER;
  }

  Current = *Instance;
  if (Current >= ARRAY_SIZE(mLogos)) {
    return EFI_NOT_FOUND;
  }

  (*Instance)++;
  *Attribute = mLogos[Current].Attribute;
  *OffsetX   = mLogos[Current].OffsetX;
  *OffsetY   = mLogos[Current].OffsetY;
  
  Status = mHiiImageEx->GetImageEx(mHiiImageEx, mHiiHandle, mLogos[Current].ImageId, Image);
  
  if (Current == 0 && !EFI_ERROR(Status)) {
    DEBUG((DEBUG_INFO, "Logo displayed, setting up timing\n"));
    SetupLogoTiming();
  }
  
  return Status;
}

EDKII_PLATFORM_LOGO_PROTOCOL  mPlatformLogo = {
  GetImage
};


//
// Driver Cleanup
//
EFI_STATUS
EFIAPI
LogoDriverUnload(
  IN EFI_HANDLE  ImageHandle
  )
{
  if (mTimerEvent != NULL) {
    gBS->CloseEvent(mTimerEvent);
    mTimerEvent = NULL;
  }
  
  if (mKeyMonitorEvent != NULL) {
    gBS->CloseEvent(mKeyMonitorEvent);
    mKeyMonitorEvent = NULL;
  }
  
  if (mUsbAuthEvent != NULL) {
    gBS->CloseEvent(mUsbAuthEvent);
    mUsbAuthEvent = NULL;
  }
  
  return EFI_SUCCESS;
}

//
// Driver Entry Point
//
EFI_STATUS
EFIAPI
InitializeLogo(
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                   Status;
  EFI_HII_PACKAGE_LIST_HEADER  *PackageList;
  EFI_HII_DATABASE_PROTOCOL    *HiiDatabase;
  EFI_HANDLE                   Handle;

  DEBUG((DEBUG_INFO, "Combined UEFI Driver: USB Authentication + BIOS Setup\n"));

  // Initialize global variables
  mInBiosSetup = FALSE;
  mSelectedMainTab = 0;
  mSelectedSubItem = 0;
  mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;
  mTotalMainTabs = ARRAY_SIZE(mMainTabs);
  mInterfaceInitialized = FALSE;
  mUsbAuthenticated = FALSE;
  
  // Initialize calendar state
  mCalendar.InCalendarMode = FALSE;
  mCalendar.SelectedField = 0;

  // Locate HII Database Protocol
  Status = gBS->LocateProtocol(
                  &gEfiHiiDatabaseProtocolGuid,
                  NULL,
                  (VOID **)&HiiDatabase
                  );
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "Failed to locate HII Database Protocol: %r\n", Status));
    return Status;
  }

  // Locate HII Image Ex Protocol
  Status = gBS->LocateProtocol(
                  &gEfiHiiImageExProtocolGuid,
                  NULL,
                  (VOID **)&mHiiImageEx
                  );
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "Failed to locate HII Image Ex Protocol: %r\n", Status));
    return Status;
  }

  // Retrieve HII package list from ImageHandle
  Status = gBS->OpenProtocol(
                  ImageHandle,
                  &gEfiHiiPackageListProtocolGuid,
                  (VOID **)&PackageList,
                  ImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "HII Image Package with logo not found in PE/COFF resource section\n"));
    return Status;
  }

  // Publish HII package list to HII Database
  Status = HiiDatabase->NewPackageList(
                          HiiDatabase,
                          PackageList,
                          NULL,
                          &mHiiHandle
                          );
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "Failed to add package list to HII Database: %r\n", Status));
    return Status;
  }

  // Install Platform Logo Protocol
  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces(
                  &Handle,
                  &gEdkiiPlatformLogoProtocolGuid,
                  &mPlatformLogo,
                  NULL
                  );
  
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "Failed to install Platform Logo Protocol: %r\n", Status));
    HiiDatabase->RemovePackageList(HiiDatabase, mHiiHandle);
    return Status;
  }

  DEBUG((DEBUG_INFO, "Combined UEFI Driver: USB Auth + BIOS Setup Complete\n"));
  
  return EFI_SUCCESS;
}

**/




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////









#include <Uefi.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/PrintLib.h>
#include <Library/DebugLib.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/UsbIo.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/HiiImageEx.h>
#include <Protocol/PlatformLogo.h>
#include <Protocol/HiiPackageList.h>

#include <Protocol/SimpleTextIn.h>

#include <Protocol/SimpleTextOut.h>


// Add these includes at the top of your file (after existing includes)
#include <Protocol/MpService.h>
#include <Library/CpuLib.h>
#include <Register/Intel/Cpuid.h>

// USB Authentication Constants

#define FIXED_VENDOR_ID 0x0781

#define FIXED_PRODUCT_ID 0x5595

#define FIXED_SERIAL_NUMBER L"040144d2dd4c4643e76cd30e263d37e6fff7d68d4ecb3ac1374d551837620beec3b2000000000000000000008cd839090083751895558107b628040f"

#define MAX_ATTEMPTS 2


// Add these structures for CPU information
typedef struct {
  CHAR16  VendorString[13];
  CHAR16  BrandString[49];
  UINT32  MaxBasicCpuIdIndex;
  UINT32  MaxExtendedCpuIdIndex;
  UINT32  Family;
  UINT32  Model;
  UINT32  Stepping;
  UINT32  ProcessorType;
  UINT32  CacheLineSize;
  UINT32  MaxLogicalProcessors;
  UINTN   NumberOfProcessors;
  UINTN   NumberOfEnabledProcessors;
  UINT64  ProcessorFrequency;
  BOOLEAN HyperThreadingSupported;
  BOOLEAN MultiCoreSupported;
} CPU_INFO;

// Global CPU information variable
CPU_INFO mCpuInfo;


// BIOS Setup Structures and Constants

typedef struct {

  EFI_IMAGE_ID                             ImageId;

  EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE    Attribute;

  INTN                                     OffsetX;

  INTN                                     OffsetY;

} LOGO_ENTRY;



typedef struct {

  CHAR16  *MenuText;

  CHAR16  *Description;

  UINT32  StartCol;

  UINT32  EndCol;

  UINT32  TabWidth;

} MENU_TAB;



typedef struct {

  CHAR16  *MenuText;

  CHAR16  *Description;

  UINT32  StartRow;

  UINT32  StartCol;

  UINT32  EndRow;

  UINT32  EndCol;

} SUBMENU_ITEM;



typedef struct {

  UINT16   Year;

  UINT8    Month;

  UINT8    Day;

  UINT8    Hour;

  UINT8    Minute;

  UINT8    Second;

  UINT8    SelectedField;

  BOOLEAN  InCalendarMode;

} CALENDAR_STATE;



typedef enum {

  MENU_LEVEL_MAIN_MENU = 0,

  MENU_LEVEL_MAIN_SUBMENU,

  MENU_LEVEL_SECURITY_SUBMENU,

  MENU_LEVEL_ADVANCED_SUBMENU,

  MENU_LEVEL_UEFI_SUBMENU,

  MENU_LEVEL_CALENDAR,
  
  MENU_LEVEL_CPU_INFO        // Add this new level

} MENU_LEVEL;



// Global Variables

EFI_HII_IMAGE_EX_PROTOCOL  *mHiiImageEx;

EFI_HII_HANDLE             mHiiHandle;

EFI_EVENT                  mKeyMonitorEvent;

EFI_EVENT                  mTimerEvent;

EFI_EVENT                  mUsbAuthEvent;

BOOLEAN                    mLogoDisplayed = FALSE;

BOOLEAN                    mMessageShown = FALSE;

BOOLEAN                    mInBiosSetup = FALSE;

BOOLEAN                    mUsbAuthenticated = FALSE;

UINTN                      mTimerCounter = 0;

UINTN                      mSelectedMainTab = 0;

UINTN                      mSelectedSubItem = 0;

UINTN                      mTotalMainTabs = 0;

UINTN                      mTotalSubItems = 0;

MENU_LEVEL                 mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;

UINTN                      mConsoleColumns = 100;

UINTN                      mConsoleRows = 40;

CALENDAR_STATE             mCalendar;

BOOLEAN                    mInterfaceInitialized = FALSE;



// Menu Data

MENU_TAB mMainTabs[] = {

  { L"Main", L"System Information and Basic Settings", 2, 17, 16 },

  { L"Advanced", L"Advanced Configuration Options", 18, 35, 18 },

  { L"Security", L"Security and password Settings", 36, 53, 18 },

  { L"Save & Exit", L"Save changes and exit setup", 54, 75, 22 }

};



SUBMENU_ITEM mMainSubmenu[] = {

  { L"BIOS Information", L"View BIOS version and system information", 5, 5, 5, 35 },
   { L"System Information", L"View detailed system specifications and hardware info", 6, 5, 6, 35 }, // Add this

  { L"BIOS Eventlog", L"Display BIOS event log entries", 7, 5, 7, 35  },

  { L"Update system BIOS", L"Update system BIOS firmware", 8, 5, 8, 35 },

  { L"Change Date & Time", L"Configure system date and time manually",  9, 5, 9, 35 },

  { L"USB Authentication", L"Authenticate USB device for secure access", 10, 5, 10, 35 },

};



SUBMENU_ITEM mAdvancedSubmenu[] = {

  { L"Boot Options", L"Configure boot device priority and settings", 5, 5, 5, 35 },

  { L"Port Options", L"Configure USB, SATA, and other port settings", 6, 5, 6, 35 },

  { L"System Options", L"Configure CPU, memory, and system settings", 7, 5, 7, 35 },

  { L"Power Management", L"Configure power and thermal settings", 8, 5, 8, 35 },

};



SUBMENU_ITEM mSecuritySubmenu[] = {

  { L"Administrator Tools", L"Administrative security management tools", 5, 5, 5, 35 },

  { L"Create BIOS Password", L"Set BIOS administrator password protection", 6, 5, 6, 35 },

  { L"TPM Security", L"Configure Trusted Platform Module settings", 7, 5, 7, 35 },

  { L"Secure Boot", L"Enable or disable UEFI Secure Boot", 8, 5, 8, 35 },

};



SUBMENU_ITEM mSaveExitSubmenu[] = {

  { L"Save Changes and Exit", L"Save current settings and exit setup", 5, 5, 5, 35 },

  { L"Discard Changes and Exit", L"Exit setup without saving any changes", 6, 5, 6, 35 },

  { L"Save Changes", L"Save current settings and continue", 7, 5, 7, 35 },

  { L"Discard Changes", L"Reset to previous saved settings", 8, 5, 8, 35 },

  { L"Load Setup Defaults", L"Load factory default settings", 9, 5, 9, 35 }

};



CHAR16 *mMonthNames[] = {

  L"January", L"February", L"March", L"April", L"May", L"June",

  L"July", L"August", L"September", L"October", L"November", L"December"

};



LOGO_ENTRY mLogos[] = {

  {

    IMAGE_TOKEN (IMG_LOGO),

    EdkiiPlatformLogoDisplayAttributeCenter,

    0,

    0

  }

};



// Function Declarations

EFI_STATUS PerformUsbAuthentication(VOID);

VOID DisplayBiosSetupForm(VOID);

VOID GetCurrentTime(OUT CHAR16 *TimeString);

VOID HandleBiosSetupNavigation(VOID);

VOID DrawBlueBackgroundOnce(VOID);

VOID UpdateTabsOnly(VOID);

VOID UpdateSubmenuOnly(VOID);

VOID UpdateInfoPanelOnly(VOID);

VOID DrawInitialInterface(VOID);

VOID HandleTabSelection(VOID);

SUBMENU_ITEM* GetCurrentSubmenuArray(OUT UINTN *ItemCount);

VOID GetConsoleSize(VOID);

VOID InitializeCalendar(VOID);

VOID DisplayCalendar(VOID);

VOID HandleCalendarNavigation(VOID);

BOOLEAN IsLeapYear(UINT16 Year);

UINT8 GetDaysInMonth(UINT16 Year, UINT8 Month);

VOID SetSystemTime(VOID);

VOID SetupLogoTiming(VOID);

VOID EFIAPI KeyboardMonitor(IN EFI_EVENT Event, IN VOID *Context);

VOID EFIAPI TimerCallback(IN EFI_EVENT Event, IN VOID *Context);


// Add these function declarations with  existing ones:
VOID CollectCpuInformation(VOID);
VOID DisplayCpuInformation(VOID);
VOID HandleCpuInfoNavigation(VOID);
VOID GetCpuVendorString(OUT CHAR16 *VendorString);
VOID GetCpuBrandString(OUT CHAR16 *BrandString);
VOID GetCpuFamilyModelStepping(OUT UINT32 *Family, OUT UINT32 *Model, OUT UINT32 *Stepping, OUT UINT32 *ProcessorType);
EFI_STATUS GetProcessorCount(OUT UINTN *NumberOfProcessors, OUT UINTN *NumberOfEnabledProcessors);
BOOLEAN IsHyperThreadingSupported(VOID);
UINT32 GetCacheLineSize(VOID);





// Function to get CPU vendor string
VOID
GetCpuVendorString(
  OUT CHAR16 *VendorString
  )
{
  UINT32 RegEbx, RegEcx, RegEdx;
  CHAR8  VendorStr[13];
  
  // CPUID leaf 0 returns vendor string in EBX, EDX, ECX
  AsmCpuid(0, NULL, &RegEbx, &RegEcx, &RegEdx);
  
  // Copy vendor string
  CopyMem(&VendorStr[0], &RegEbx, 4);
  CopyMem(&VendorStr[4], &RegEdx, 4);
  CopyMem(&VendorStr[8], &RegEcx, 4);
  VendorStr[12] = '\0';
  
  // Convert to Unicode
  AsciiStrToUnicodeStrS(VendorStr, VendorString, 13);
}

// Function to get CPU brand string
VOID
GetCpuBrandString(
  OUT CHAR16 *BrandString
  )
{
  UINT32 RegEax, RegEbx, RegEcx, RegEdx;
  CHAR8  BrandStr[49];
  UINT32 Index;
  
  // Check if brand string is supported
  AsmCpuid(0x80000000, &RegEax, NULL, NULL, NULL);
  if (RegEax < 0x80000004) {
    UnicodeSPrint(BrandString, 49 * sizeof(CHAR16), L"Brand String Not Supported");
    return;
  }
  
  // Get brand string from CPUID leaves 0x80000002, 0x80000003, 0x80000004
  Index = 0;
  for (UINT32 Leaf = 0x80000002; Leaf <= 0x80000004; Leaf++) {
    AsmCpuid(Leaf, &RegEax, &RegEbx, &RegEcx, &RegEdx);
    CopyMem(&BrandStr[Index], &RegEax, 4);
    CopyMem(&BrandStr[Index + 4], &RegEbx, 4);
    CopyMem(&BrandStr[Index + 8], &RegEcx, 4);
    CopyMem(&BrandStr[Index + 12], &RegEdx, 4);
    Index += 16;
  }
  BrandStr[48] = '\0';
  
  // Convert to Unicode and trim whitespace
  AsciiStrToUnicodeStrS(BrandStr, BrandString, 49);
  
  // Trim leading spaces
  CHAR16 *TrimmedStart = BrandString;
  while (*TrimmedStart == L' ') {
    TrimmedStart++;
  }
  if (TrimmedStart != BrandString) {
    StrCpyS(BrandString, 49, TrimmedStart);
  }
}

// Function to get CPU family, model, stepping information
VOID
GetCpuFamilyModelStepping(
  OUT UINT32 *Family,
  OUT UINT32 *Model,
  OUT UINT32 *Stepping,
  OUT UINT32 *ProcessorType
  )
{
  UINT32 RegEax;
  
  AsmCpuid(1, &RegEax, NULL, NULL, NULL);
  
  *Stepping = RegEax & 0xF;
  *Model = (RegEax >> 4) & 0xF;
  *Family = (RegEax >> 8) & 0xF;
  *ProcessorType = (RegEax >> 12) & 0x3;
  
  // Handle extended family and model for newer processors
  if (*Family == 0xF) {
    *Family += (RegEax >> 20) & 0xFF;
  }
  if (*Family == 0x6 || *Family == 0xF) {
    *Model += ((RegEax >> 16) & 0xF) << 4;
  }
}

// Function to get processor count using MP Services Protocol

EFI_STATUS
GetProcessorCount(
  OUT UINTN *NumberOfProcessors,
  OUT UINTN *NumberOfEnabledProcessors
  )
{
  EFI_STATUS                Status;
  EFI_MP_SERVICES_PROTOCOL  *MpServices;
  
  Status = gBS->LocateProtocol(
                  &gEfiMpServiceProtocolGuid,
                  NULL,
                  (VOID **)&MpServices
                  );
  
  if (!EFI_ERROR(Status)) {
    Status = MpServices->GetNumberOfProcessors(
                           MpServices,
                           NumberOfProcessors,
                           NumberOfEnabledProcessors
                           );
  } else {
    // Fallback: Use CPUID to get basic processor info
    UINT32 RegEbx;
    AsmCpuid(1, NULL, &RegEbx, NULL, NULL);
    
    // Extract logical processor count from CPUID.01H:EBX[23:16]
    UINT32 LogicalProcessors = (RegEbx >> 16) & 0xFF;
    
    if (LogicalProcessors > 0) {
      *NumberOfProcessors = LogicalProcessors;
      *NumberOfEnabledProcessors = LogicalProcessors;
    } else {
      // Ultimate fallback: assume single processor
      *NumberOfProcessors = 1;
      *NumberOfEnabledProcessors = 1;
    }
    
    Status = EFI_SUCCESS;
  }
  
  return Status;
}


// Function to detect Hyper-Threading support
BOOLEAN
IsHyperThreadingSupported(
  VOID
  )
{
  UINT32 RegEax, RegEbx, RegEcx, RegEdx;
  
  // Check CPUID leaf 1, EDX bit 28 for Hyper-Threading
  AsmCpuid(1, &RegEax, &RegEbx, &RegEcx, &RegEdx);
  
  return (RegEdx & BIT28) ? TRUE : FALSE;
}

// Function to get cache line size
UINT32
GetCacheLineSize(
  VOID
  )
{
  UINT32 RegEax, RegEbx, RegEcx, RegEdx;
  
  AsmCpuid(1, &RegEax, &RegEbx, &RegEcx, &RegEdx);
  
  // Cache line size is in EBX bits 15:8, multiply by 8
  return ((RegEbx >> 8) & 0xFF) * 8;
}

// Main function to collect all CPU information
VOID
CollectCpuInformation(
  VOID
  )
{
  UINT32 RegEax;
  
  DEBUG((DEBUG_INFO, "Collecting CPU Information...\n"));
  
  // Get basic CPUID info
  AsmCpuid(0, &mCpuInfo.MaxBasicCpuIdIndex, NULL, NULL, NULL);
  AsmCpuid(0x80000000, &mCpuInfo.MaxExtendedCpuIdIndex, NULL, NULL, NULL);
  
  // Get vendor string
  GetCpuVendorString(mCpuInfo.VendorString);
  
  // Get brand string
  GetCpuBrandString(mCpuInfo.BrandString);
  
  // Get family, model, stepping
  GetCpuFamilyModelStepping(
    &mCpuInfo.Family,
    &mCpuInfo.Model,
    &mCpuInfo.Stepping,
    &mCpuInfo.ProcessorType
  );
  
  // Get processor count
  GetProcessorCount(
    &mCpuInfo.NumberOfProcessors,
    &mCpuInfo.NumberOfEnabledProcessors
  );
  
  // Get cache line size
  mCpuInfo.CacheLineSize = GetCacheLineSize();
  
  // Check Hyper-Threading support
  mCpuInfo.HyperThreadingSupported = IsHyperThreadingSupported();
  
  // Check if multi-core (more than 1 processor)
  mCpuInfo.MultiCoreSupported = (mCpuInfo.NumberOfProcessors > 1) ? TRUE : FALSE;
  
  // Get max logical processors per package
  if (mCpuInfo.MaxBasicCpuIdIndex >= 1) {
    AsmCpuid(1, NULL, &RegEax, NULL, NULL);
    mCpuInfo.MaxLogicalProcessors = (RegEax >> 16) & 0xFF;
  } else {
    mCpuInfo.MaxLogicalProcessors = 1;
  }
  
  DEBUG((DEBUG_INFO, "CPU Info Collection Complete\n"));
}

// Function to display CPU information in BIOS setup
VOID
DisplayCpuInformation(
  VOID
  )
{
  UINTN Row = 5;
  UINTN Col = 3;
 
  
  // Clear the interface area first
  for (UINTN ClearRow = 4; ClearRow < mConsoleRows - 3; ClearRow++) {
    gST->ConOut->SetCursorPosition(gST->ConOut, 1, ClearRow);
    gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    for (UINTN ClearCol = 1; ClearCol < mConsoleColumns - 1; ClearCol++) {
      Print(L" ");
    }
  }
  
  // Title
  gST->ConOut->SetCursorPosition(gST->ConOut, Col, Row);
  gST->ConOut->SetAttribute(gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
  Print(L"CPU Information");
  Row += 2;
  
  // CPU Vendor
  gST->ConOut->SetCursorPosition(gST->ConOut, Col, Row);
  gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
  Print(L"Vendor: %s", mCpuInfo.VendorString);
  Row++;
  
  // CPU Brand String
  gST->ConOut->SetCursorPosition(gST->ConOut, Col, Row);
  Print(L"Processor: %s", mCpuInfo.BrandString);
  Row++;
  
  // Family, Model, Stepping
  gST->ConOut->SetCursorPosition(gST->ConOut, Col, Row);
  Print(L"Family: %02X, Model: %02X, Stepping: %02X", 
        mCpuInfo.Family, mCpuInfo.Model, mCpuInfo.Stepping);
  Row++;
  
  // Processor Type
  gST->ConOut->SetCursorPosition(gST->ConOut, Col, Row);
  CHAR16 *ProcessorTypeStr;
  switch (mCpuInfo.ProcessorType) {
    case 0: ProcessorTypeStr = L"Original OEM Processor"; break;
    case 1: ProcessorTypeStr = L"Intel OverDrive Processor"; break;
    case 2: ProcessorTypeStr = L"Dual Processor"; break;
    default: ProcessorTypeStr = L"Reserved"; break;
  }
  Print(L"Type: %s", ProcessorTypeStr);
  Row++;
  
  // Number of Processors/Cores
  gST->ConOut->SetCursorPosition(gST->ConOut, Col, Row);
  Print(L"Cores: %d (Enabled: %d)", mCpuInfo.NumberOfProcessors, mCpuInfo.NumberOfEnabledProcessors);
  Row++;
  
  // Max Logical Processors
  gST->ConOut->SetCursorPosition(gST->ConOut, Col, Row);
  Print(L"Max Logical Processors: %d", mCpuInfo.MaxLogicalProcessors);
  Row++;
  
  // Features
  gST->ConOut->SetCursorPosition(gST->ConOut, Col, Row);
  Print(L"Hyper-Threading: %s", mCpuInfo.HyperThreadingSupported ? L"Supported" : L"Not Supported");
  Row++;
  
  gST->ConOut->SetCursorPosition(gST->ConOut, Col, Row);
  Print(L"Multi-Core: %s", mCpuInfo.MultiCoreSupported ? L"Yes" : L"No");
  Row++;
  
  // Cache Line Size
  gST->ConOut->SetCursorPosition(gST->ConOut, Col, Row);
  Print(L"Cache Line Size: %d bytes", mCpuInfo.CacheLineSize);
  Row++;
  
  // CPUID Support Levels
  gST->ConOut->SetCursorPosition(gST->ConOut, Col, Row);
  Print(L"Max Basic CPUID: %08X", mCpuInfo.MaxBasicCpuIdIndex);
  Row++;
  
  gST->ConOut->SetCursorPosition(gST->ConOut, Col, Row);
  Print(L"Max Extended CPUID: %08X", mCpuInfo.MaxExtendedCpuIdIndex);
  Row += 2;
  
  // Instructions
  gST->ConOut->SetCursorPosition(gST->ConOut, Col, Row);
  gST->ConOut->SetAttribute(gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
  Print(L"Press ESC to return to main menu");
}

// Function to handle CPU information navigation
VOID
HandleCpuInfoNavigation(
  VOID
  )
{
  EFI_STATUS     Status;
  EFI_INPUT_KEY  Key;
  
  while (mInBiosSetup) {
    Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
    
    if (!EFI_ERROR(Status)) {
      if (Key.ScanCode == SCAN_ESC) {
        mCurrentMenuLevel = MENU_LEVEL_MAIN_SUBMENU;
        DrawInitialInterface();
        return;
      }
    } else {
      gBS->Stall(50000); // 50ms delay
    }
  }
}



//
// USB Authentication Implementation
//


EFI_STATUS
PerformUsbAuthentication(
  VOID
  )
{
  EFI_USB_DEVICE_DESCRIPTOR DevDesc;
  EFI_USB_INTERFACE_DESCRIPTOR IfDesc;
  EFI_USB_IO_PROTOCOL *UsbIo;
  EFI_STATUS Status = EFI_SUCCESS;
  EFI_HANDLE *HandleBuffer = NULL;
  CHAR16 *Manufacturer;
  CHAR16 *Product;
  CHAR16 *SerialNumber;

  UINT16 *LangIdTable;

  UINT16 TableSize;

  UINTN HandleCount;
  UINT8 Attempts = 0;
  BOOLEAN DeviceFound = FALSE;



  DEBUG((DEBUG_INFO, "Starting USB Authentication Process\n"));



  // Clear screen for authentication display

  gST->ConOut->ClearScreen(gST->ConOut);

  gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);

  

  Print(L"\n");

  Print(L"=== USB Device Authentication ===\n");

  Print(L"Scanning for authorized USB devices...\n\n");


  // Locate USB devices

  Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiUsbIoProtocolGuid, NULL, &HandleCount, &HandleBuffer);

  if (EFI_ERROR(Status)) {

    Print(L"ERROR: Could not locate USB devices.\n");

    return Status;

  }



  Print(L"Found %d USB device(s)\n", HandleCount);

  Print(L"\n  VendorID  ProductID  Manufacturer/Product/SerialNumber\n");

  Print(L"  -------------------------------------------------------\n\n");



  for (UINT8 Index = 0; Index < HandleCount && Attempts < MAX_ATTEMPTS; Index++) {
    Status = gBS->HandleProtocol(HandleBuffer[Index], &gEfiUsbIoProtocolGuid, (VOID**)&UsbIo);

    if (EFI_ERROR(Status)) {

      Print(L"ERROR: Failed to open USB I/O protocol.\n");

      continue;

    }



    Status = UsbIo->UsbGetDeviceDescriptor(UsbIo, &DevDesc);

    if (EFI_ERROR(Status)) {

      Print(L"ERROR: Failed to get device descriptor.\n");
      continue;

    }


    Status = UsbIo->UsbGetInterfaceDescriptor(UsbIo, &IfDesc);

    if (EFI_ERROR(Status)) {

      Print(L"ERROR: Failed to get interface descriptor.\n");

      continue;

    }



    // Get string descriptors
    TableSize = 0;

    LangIdTable = NULL;

    Status = UsbIo->UsbGetSupportedLanguages(UsbIo, &LangIdTable, &TableSize);

    if (LangIdTable != NULL) {

      FreePool(LangIdTable);

    }



    Status = UsbIo->UsbGetStringDescriptor(UsbIo, 0x0409, DevDesc.StrManufacturer, &Manufacturer);

    if (EFI_ERROR(Status)) {

      Manufacturer = L"Unknown";

    }



    Status = UsbIo->UsbGetStringDescriptor(UsbIo, 0x0409, DevDesc.StrProduct, &Product);

    if (EFI_ERROR(Status)) {

      Product = L"Unknown";

    }



    Status = UsbIo->UsbGetStringDescriptor(UsbIo, 0x0409, DevDesc.StrSerialNumber, &SerialNumber);
    if (EFI_ERROR(Status)) {

      SerialNumber = L"Unknown";

    }


    // Display device information

    Print(L"    %04X      %04X     %s, %s, %s\n",

          DevDesc.IdVendor, DevDesc.IdProduct, Manufacturer, Product, SerialNumber);



    // Check authentication

    if (DevDesc.IdVendor == FIXED_VENDOR_ID && 

        DevDesc.IdProduct == FIXED_PRODUCT_ID && 

        StrCmp(SerialNumber, FIXED_SERIAL_NUMBER) == 0) {

      

      Print(L"\n*** AUTHENTICATION SUCCESSFUL ***\n");

      Print(L"Welcome, Authorized User!\n");

      mUsbAuthenticated = TRUE;
      DeviceFound = TRUE;

      break;

    } else {
      Attempts++;

      if (Attempts >= MAX_ATTEMPTS) {

        Print(L"\n*** AUTHENTICATION FAILED ***\n");

        Print(L"Maximum unauthorized attempts reached.\n");

        Print(L"System will shutdown in 3 seconds...\n");
        gBS->Stall(3000000); // 3 seconds

        FreePool(HandleBuffer);

        return EFI_ACCESS_DENIED;

      }

    }



    // Free string descriptors if they were allocated

    if (Manufacturer != L"Unknown") {

      FreePool(Manufacturer);

    }

    if (Product != L"Unknown") {

      FreePool(Product);

    }

    if (SerialNumber != L"Unknown") {

      FreePool(SerialNumber);

    }
  }



  if (!DeviceFound && Attempts == 0) {

    Print(L"\n*** AUTHENTICATION FAILED ***\n");

    Print(L"No authorized USB device found.\n");

    Print(L"System will shutdown in 3 seconds...\n");

    gBS->Stall(3000000);

    FreePool(HandleBuffer);

    return EFI_ACCESS_DENIED;
  }


  Print(L"\nPress any key to continue to BIOS Setup...\n");

  

  // Wait for key press

  EFI_INPUT_KEY Key;

  while (gST->ConIn->ReadKeyStroke(gST->ConIn, &Key) == EFI_NOT_READY) {

    gBS->Stall(100000); // 100ms

  }



  FreePool(HandleBuffer);

  return EFI_SUCCESS;

}



/**
EFI_STATUS
PerformUsbAuthentication(
  VOID
  )
{
  EFI_USB_DEVICE_DESCRIPTOR DevDesc;
  EFI_USB_INTERFACE_DESCRIPTOR IfDesc;
  EFI_USB_IO_PROTOCOL *UsbIo;
  EFI_STATUS Status = EFI_SUCCESS;
  EFI_HANDLE *HandleBuffer = NULL;
  CHAR16 *Manufacturer = NULL;
  CHAR16 *Product = NULL;
  CHAR16 *SerialNumber = NULL;
  UINT16 *LangIdTable = NULL;
  UINT16 TableSize;
  UINTN HandleCount;
  UINT8 Attempts = 0;
  BOOLEAN DeviceFound = FALSE;

  DEBUG((DEBUG_INFO, "Starting USB Authentication Process\n"));

  // Clear screen for authentication display
  gST->ConOut->ClearScreen(gST->ConOut);
  gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);
  
  Print(L"\n");
  Print(L"=== USB Device Authentication ===\n");
  Print(L"Scanning for authorized USB devices...\n\n");

  // Locate USB devices
  Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiUsbIoProtocolGuid, NULL, &HandleCount, &HandleBuffer);
  if (EFI_ERROR(Status)) {
    Print(L"ERROR: Could not locate USB devices.\n");
    Print(L"Status: %r\n", Status);

    return Status;

  }


  Print(L"Found %d USB device(s)\n", HandleCount);

  Print(L"\n  VendorID  ProductID  Manufacturer/Product/SerialNumber\n");

  Print(L"  -------------------------------------------------------\n\n");



  for (UINT8 Index = 0; Index < HandleCount && Attempts < MAX_ATTEMPTS; Index++) {
    // Reset string pointers for each device

    Manufacturer = NULL;

    Product = NULL;

    SerialNumber = NULL;

    

    Status = gBS->HandleProtocol(HandleBuffer[Index], &gEfiUsbIoProtocolGuid, (VOID**)&UsbIo);
    if (EFI_ERROR(Status)) {

      Print(L"ERROR: Failed to open USB I/O protocol for device %d.\n", Index);

      continue;

    }



    Status = UsbIo->UsbGetDeviceDescriptor(UsbIo, &DevDesc);

    if (EFI_ERROR(Status)) {

      Print(L"ERROR: Failed to get device descriptor for device %d.\n", Index);

      continue;

    }



    Status = UsbIo->UsbGetInterfaceDescriptor(UsbIo, &IfDesc);
    if (EFI_ERROR(Status)) {

      Print(L"ERROR: Failed to get interface descriptor for device %d.\n", Index);

      continue;
    }


    // Get supported languages first

    TableSize = 0;

    Status = UsbIo->UsbGetSupportedLanguages(UsbIo, &LangIdTable, &TableSize);
    if (EFI_ERROR(Status) || LangIdTable == NULL) {

      Print(L"WARNING: No language support found for device %d\n", Index);

      // Use default strings

      Manufacturer = L"Unknown";

      Product = L"Unknown";  

      SerialNumber = L"Unknown";

    } else {

      // Try to get string descriptors with first supported language
      UINT16 LangId = LangIdTable[0]; // Use first supported language

      

      // Get Manufacturer string

      if (DevDesc.StrManufacturer != 0) {

        Status = UsbIo->UsbGetStringDescriptor(UsbIo, LangId, DevDesc.StrManufacturer, &Manufacturer);
        if (EFI_ERROR(Status)) {

          Manufacturer = L"Unknown";

        }

      } else {

        Manufacturer = L"No Manufacturer";

      }



      // Get Product string  

      if (DevDesc.StrProduct != 0) {

        Status = UsbIo->UsbGetStringDescriptor(UsbIo, LangId, DevDesc.StrProduct, &Product);

        if (EFI_ERROR(Status)) {

          Product = L"Unknown";

        }

      } else {

        Product = L"No Product";

      }


      // Get Serial Number string

      if (DevDesc.StrSerialNumber != 0) {

        Status = UsbIo->UsbGetStringDescriptor(UsbIo, LangId, DevDesc.StrSerialNumber, &SerialNumber);

        if (EFI_ERROR(Status)) {
          SerialNumber = L"Unknown";

        }
      } else {

        SerialNumber = L"No Serial";

      }

      

      // Free the language table

      if (LangIdTable != NULL) {

        FreePool(LangIdTable);
        LangIdTable = NULL;

      }

    }



    // Display device information - ensure strings are not NULL

    if (Manufacturer == NULL) Manufacturer = L"NULL";

    if (Product == NULL) Product = L"NULL";

    if (SerialNumber == NULL) SerialNumber = L"NULL";


    Print(L"  %04X      %04X     %s / %s / %s\n",

          DevDesc.IdVendor, DevDesc.IdProduct, Manufacturer, Product, SerialNumber);



    // Check authentication
    if (DevDesc.IdVendor == FIXED_VENDOR_ID && 

        DevDesc.IdProduct == FIXED_PRODUCT_ID) {

        

      // For serial number comparison, handle the case where we might not have retrieved it properly

      if (SerialNumber != NULL && 

          StrCmp(SerialNumber, L"Unknown") != 0 && 

          StrCmp(SerialNumber, L"NULL") != 0 &&

          StrCmp(SerialNumber, L"No Serial") != 0 &&

          StrCmp(SerialNumber, FIXED_SERIAL_NUMBER) == 0) {
        

        Print(L"\n*** AUTHENTICATION SUCCESSFUL ***\n");
        Print(L"Welcome, Authorized User!\n");

        mUsbAuthenticated = TRUE;

        DeviceFound = TRUE;

        

        // Free allocated strings before breaking

        if (Manufacturer != NULL && StrCmp(Manufacturer, L"Unknown") != 0 && 

            StrCmp(Manufacturer, L"NULL") != 0 && StrCmp(Manufacturer, L"No Manufacturer") != 0) {

          FreePool(Manufacturer);

        }

        if (Product != NULL && StrCmp(Product, L"Unknown") != 0 && 

            StrCmp(Product, L"NULL") != 0 && StrCmp(Product, L"No Product") != 0) {

          FreePool(Product);
        }

        if (SerialNumber != NULL && StrCmp(SerialNumber, L"Unknown") != 0 && 

            StrCmp(SerialNumber, L"NULL") != 0 && StrCmp(SerialNumber, L"No Serial") != 0) {

          FreePool(SerialNumber);

        }

        

        break;

      } else {

        Print(L"    Vendor/Product ID matches but serial number differs\n");

        Print(L"    Expected: %s\n", FIXED_SERIAL_NUMBER);

        Print(L"    Found: %s\n", SerialNumber);

        Attempts++;
      }

    } else {

      Attempts++;
    }



    // Free string descriptors if they were allocated (not static strings)

    if (Manufacturer != NULL && StrCmp(Manufacturer, L"Unknown") != 0 && 

        StrCmp(Manufacturer, L"NULL") != 0 && StrCmp(Manufacturer, L"No Manufacturer") != 0) {

      FreePool(Manufacturer);

    }

    if (Product != NULL && StrCmp(Product, L"Unknown") != 0 && 

        StrCmp(Product, L"NULL") != 0 && StrCmp(Product, L"No Product") != 0) {

      FreePool(Product);

    }

    if (SerialNumber != NULL && StrCmp(SerialNumber, L"Unknown") != 0 && 

        StrCmp(SerialNumber, L"NULL") != 0 && StrCmp(SerialNumber, L"No Serial") != 0) {

      FreePool(SerialNumber);

    }


    // Check if maximum attempts reached

    if (Attempts >= MAX_ATTEMPTS) {

      Print(L"\n*** AUTHENTICATION FAILED ***\n");

      Print(L"Maximum unauthorized attempts reached.\n");

      Print(L"System will shutdown in 3 seconds...\n");

      gBS->Stall(3000000); // 3 seconds

      FreePool(HandleBuffer);

      return EFI_ACCESS_DENIED;

    }

  }


  if (!DeviceFound && Attempts == 0) {

    Print(L"\n*** AUTHENTICATION FAILED ***\n");

    Print(L"No authorized USB device found.\n");

    Print(L"System will shutdown in 3 seconds...\n");

    gBS->Stall(3000000);

    FreePool(HandleBuffer);

    return EFI_ACCESS_DENIED;

  }


  if (!DeviceFound && Attempts > 0) {
    Print(L"\n*** AUTHENTICATION FAILED ***\n");

    Print(L"USB devices found but none match authorization criteria.\n");

    Print(L"System will shutdown in 3 seconds...\n");

    gBS->Stall(3000000);

    FreePool(HandleBuffer);

    return EFI_ACCESS_DENIED;

  }


  Print(L"\nPress any key to continue to BIOS Setup...\n");

  

  // Wait for key press

  EFI_INPUT_KEY Key;

  while (gST->ConIn->ReadKeyStroke(gST->ConIn, &Key) == EFI_NOT_READY) {

    gBS->Stall(100000); // 100ms

  }



  FreePool(HandleBuffer);

  return EFI_SUCCESS;

}


**/




//
// Calendar Functions
//



BOOLEAN
IsLeapYear(
  UINT16 Year
  )
{
  return ((Year % 4 == 0) && (Year % 100 != 0)) || (Year % 400 == 0);
}


UINT8

GetDaysInMonth(

  UINT16 Year,

  UINT8  Month

  )

{

  UINT8 DaysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  

  if (Month == 2 && IsLeapYear(Year)) {

    return 29;

  }

  

  return DaysInMonth[Month - 1];

}



VOID

InitializeCalendar(

  VOID

  )

{

  EFI_TIME    Time;

  EFI_STATUS  Status;

  

  Status = gRT->GetTime(&Time, NULL);

  if (!EFI_ERROR(Status)) {

    mCalendar.Year = Time.Year;

    mCalendar.Month = Time.Month;

    mCalendar.Day = Time.Day;

    mCalendar.Hour = Time.Hour;

    mCalendar.Minute = Time.Minute;

    mCalendar.Second = Time.Second;

  } else {

    mCalendar.Year = 2024;

    mCalendar.Month = 1;

    mCalendar.Day = 1;

    mCalendar.Hour = 0;

    mCalendar.Minute = 0;

    mCalendar.Second = 0;

  }

  

  mCalendar.SelectedField = 0;

  mCalendar.InCalendarMode = TRUE;

}



VOID

SetSystemTime(

  VOID

  )

{

  EFI_TIME Time;

  EFI_STATUS Status;

  

  Time.Year = mCalendar.Year;

  Time.Month = mCalendar.Month;

  Time.Day = mCalendar.Day;

  Time.Hour = mCalendar.Hour;

  Time.Minute = mCalendar.Minute;

  Time.Second = mCalendar.Second;

  Time.Nanosecond = 0;

  Time.TimeZone = EFI_UNSPECIFIED_TIMEZONE;

  Time.Daylight = 0;

  

  Status = gRT->SetTime(&Time);

  if (EFI_ERROR(Status)) {

    DEBUG((DEBUG_ERROR, "Failed to set system time: %r\n", Status));

  }

}







VOID
DisplayCalendar(
  VOID
  )
{
  UINTN Row, Col;
//  UINTN SeparatorCol = (mConsoleColumns * 3) / 4;
  // Remove the unused RightStart variable
  
  // Clear the interface area
  for (Row = 4; Row < mConsoleRows - 3; Row++) {
    gST->ConOut->SetCursorPosition(gST->ConOut, 1, Row);
    gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
    for (Col = 1; Col < mConsoleColumns - 1; Col++) {
      Print(L" ");
    }
  }
  
  // Calendar title
  gST->ConOut->SetCursorPosition(gST->ConOut, 3, 5);
  gST->ConOut->SetAttribute(gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
  Print(L"Manual Date & Time Setting");
  
  // Current date/time display

  gST->ConOut->SetCursorPosition(gST->ConOut, 3, 7);

  gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);

  Print(L"Current Date & Time:");

  

  // Date fields with selection highlighting

  gST->ConOut->SetCursorPosition(gST->ConOut, 3, 9);

  

  // Year field

  if (mCalendar.SelectedField == 0) {

    gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);

  } else {

    gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);

  }

  Print(L"[%04d]", mCalendar.Year);

  

  gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);

  Print(L" / ");

  

  // Month field

  if (mCalendar.SelectedField == 1) {

    gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);

  } else {

    gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);

  }

  Print(L"[%02d]", mCalendar.Month);

  

  gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);

  Print(L" / ");

  

  // Day field

  if (mCalendar.SelectedField == 2) {

    gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);

  } else {

    gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);

  }

  Print(L"[%02d]", mCalendar.Day);

  

  // Time fields

  gST->ConOut->SetCursorPosition(gST->ConOut, 3, 11);

  

  // Hour field

  if (mCalendar.SelectedField == 3) {

    gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);

  } else {

    gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);

  }

  Print(L"[%02d]", mCalendar.Hour);

  

  gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);

  Print(L" : ");

  

  // Minute field

  if (mCalendar.SelectedField == 4) {

    gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);

  } else {

    gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);

  }

  Print(L"[%02d]", mCalendar.Minute);

  

  gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);

  Print(L" : ");

  

  // Second field

  if (mCalendar.SelectedField == 5) {

    gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);

  } else {

    gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);

  }

  Print(L"[%02d]", mCalendar.Second);

  

  // Month name display

  if (mCalendar.Month >= 1 && mCalendar.Month <= 12) {

    gST->ConOut->SetCursorPosition(gST->ConOut, 3, 13);

    gST->ConOut->SetAttribute(gST->ConOut, EFI_DARKGRAY | EFI_BACKGROUND_LIGHTGRAY);

    Print(L"Month: %s", mMonthNames[mCalendar.Month - 1]);

  }

  

  // Instructions

  gST->ConOut->SetCursorPosition(gST->ConOut, 3, 15);

  gST->ConOut->SetAttribute(gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);

  Print(L"Instructions:");

  

  gST->ConOut->SetCursorPosition(gST->ConOut, 3, 16);

  gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);

  Print(L"Left/Right: Select field");

  

  gST->ConOut->SetCursorPosition(gST->ConOut, 3, 17);

  Print(L"Up/Down: Change value");

  

  gST->ConOut->SetCursorPosition(gST->ConOut, 3, 18);

  Print(L"Enter: Save changes");

  

  gST->ConOut->SetCursorPosition(gST->ConOut, 3, 19);

  Print(L"ESC: Cancel and return");

}






VOID
HandleCalendarNavigation(
  VOID
  )
{

  EFI_STATUS     Status;

  EFI_INPUT_KEY  Key;
  BOOLEAN        ValueChanged = FALSE;
  UINT8          MaxDays;

  

  while (mCalendar.InCalendarMode && mInBiosSetup) {

    Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);

    

    if (!EFI_ERROR(Status)) {

      switch (Key.ScanCode) {

        case SCAN_LEFT:

          if (mCalendar.SelectedField > 0) {

            mCalendar.SelectedField--;

          } else {

            mCalendar.SelectedField = 5;

          }

          DisplayCalendar();

          break;

          

        case SCAN_RIGHT:

          if (mCalendar.SelectedField < 5) {

            mCalendar.SelectedField++;

          } else {

            mCalendar.SelectedField = 0;

          }

          DisplayCalendar();

          break;

          

        case SCAN_UP:

          ValueChanged = TRUE;

          switch (mCalendar.SelectedField) {

            case 0: // Year

              if (mCalendar.Year < 2099) mCalendar.Year++;

              break;

            case 1: // Month

              if (mCalendar.Month < 12) {

                mCalendar.Month++;

              } else {

                mCalendar.Month = 1;

              }

              break;

            case 2: // Day

              MaxDays = GetDaysInMonth(mCalendar.Year, mCalendar.Month);

              if (mCalendar.Day < MaxDays) {

                mCalendar.Day++;

              } else {

                mCalendar.Day = 1;

              }

              break;

            case 3: // Hour

              if (mCalendar.Hour < 23) {

                mCalendar.Hour++;

              } else {

                mCalendar.Hour = 0;

              }

              break;

            case 4: // Minute

              if (mCalendar.Minute < 59) {

                mCalendar.Minute++;

              } else {

                mCalendar.Minute = 0;

              }

              break;

            case 5: // Second

              if (mCalendar.Second < 59) {

                mCalendar.Second++;

              } else {

                mCalendar.Second = 0;

              }

              break;

          }

          DisplayCalendar();

          break;

          

        case SCAN_DOWN:

          ValueChanged = TRUE;

          switch (mCalendar.SelectedField) {

            case 0: // Year

              if (mCalendar.Year > 1900) mCalendar.Year--;

              break;

            case 1: // Month

              if (mCalendar.Month > 1) {

                mCalendar.Month--;

              } else {

                mCalendar.Month = 12;

              }

              break;

            case 2: // Day

              if (mCalendar.Day > 1) {

                mCalendar.Day--;

              } else {

                MaxDays = GetDaysInMonth(mCalendar.Year, mCalendar.Month);

                mCalendar.Day = MaxDays;

              }

              break;

            case 3: // Hour

              if (mCalendar.Hour > 0) {

                mCalendar.Hour--;

              } else {

                mCalendar.Hour = 23;

              }

              break;

            case 4: // Minute

              if (mCalendar.Minute > 0) {

                mCalendar.Minute--;

              } else {

                mCalendar.Minute = 59;

              }

              break;

            case 5: // Second

              if (mCalendar.Second > 0) {

                mCalendar.Second--;

              } else {

                mCalendar.Second = 59;

              }

              break;

          }

          DisplayCalendar();

          break;

          

        case SCAN_ESC:

          mCalendar.InCalendarMode = FALSE;

          mCurrentMenuLevel = MENU_LEVEL_MAIN_SUBMENU;

          DrawInitialInterface();

          return;

          

        default:

          if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {

            SetSystemTime();

            mCalendar.InCalendarMode = FALSE;

            mCurrentMenuLevel = MENU_LEVEL_MAIN_SUBMENU;

            

            gST->ConOut->SetCursorPosition(gST->ConOut, 3, 21);

            gST->ConOut->SetAttribute(gST->ConOut, EFI_GREEN | EFI_BACKGROUND_LIGHTGRAY);

            Print(L"Date & Time saved successfully!");

            gBS->Stall(1500000);

            

            DrawInitialInterface();

            return;

          }

          break;

      }

      

      if (ValueChanged && (mCalendar.SelectedField == 0 || mCalendar.SelectedField == 1)) {

        MaxDays = GetDaysInMonth(mCalendar.Year, mCalendar.Month);

        if (mCalendar.Day > MaxDays) {

          mCalendar.Day = MaxDays;

          DisplayCalendar();

        }

      }

    } else {

      gBS->Stall(50000);

    }

  }

}





//
// BIOS Setup Interface Functions
//




VOID
GetConsoleSize(
  VOID
  )
{
  EFI_STATUS Status;
  UINTN MaxMode, MaxColumns, MaxRows;
  UINTN BestMode = 0;
  UINTN BestColumns = 80;
  UINTN BestRows = 25;
  
  MaxMode = gST->ConOut->Mode->MaxMode;
  
  for (UINTN Mode = 0; Mode < MaxMode; Mode++) {
    Status = gST->ConOut->QueryMode(
                            gST->ConOut,
                            Mode,
                            &MaxColumns,

                            &MaxRows

                            );

    

    if (!EFI_ERROR(Status)) {

      if ((MaxColumns >= BestColumns && MaxRows >= BestRows) && 

          (MaxColumns > BestColumns || MaxRows > BestRows)) {

        BestMode = Mode;

        BestColumns = MaxColumns;

        BestRows = MaxRows;

      }

    }

  }

  

  if (BestMode != gST->ConOut->Mode->Mode) {

    Status = gST->ConOut->SetMode(gST->ConOut, BestMode);

    if (EFI_ERROR(Status)) {

      DEBUG((DEBUG_WARN, "Failed to set console mode %d: %r\n", BestMode, Status));

    }

  }

  

  Status = gST->ConOut->QueryMode(

                          gST->ConOut,

                          gST->ConOut->Mode->Mode,

                          &mConsoleColumns,

                          &mConsoleRows

                          );

  

  if (EFI_ERROR(Status)) {

    mConsoleColumns = 100;

    mConsoleRows = 40;

  }

  

  DEBUG((DEBUG_INFO, "Console size: %dx%d\n", mConsoleColumns, mConsoleRows));

}



SUBMENU_ITEM*

GetCurrentSubmenuArray(

  OUT UINTN *ItemCount

  )

{

  switch (mSelectedMainTab) {

    case 0: // Main

      *ItemCount = ARRAY_SIZE(mMainSubmenu);

      return mMainSubmenu;

      

    case 1: // Advanced

      *ItemCount = ARRAY_SIZE(mAdvancedSubmenu);

      return mAdvancedSubmenu;

      

    case 2: // Security

      *ItemCount = ARRAY_SIZE(mSecuritySubmenu);

      return mSecuritySubmenu;

      

    case 3: // Save & Exit

      *ItemCount = ARRAY_SIZE(mSaveExitSubmenu);

      return mSaveExitSubmenu;

      

    default:

      *ItemCount = ARRAY_SIZE(mMainSubmenu);

      return mMainSubmenu;

  }

}



VOID

GetCurrentTime(

  OUT CHAR16  *TimeString

  )

{

  EFI_TIME    Time;

  EFI_STATUS  Status;

  

  Status = gRT->GetTime(&Time, NULL);

  if (!EFI_ERROR(Status)) {

    UnicodeSPrint(

      TimeString, 

      50 * sizeof(CHAR16),

      L"%02d/%02d/%d  %02d:%02d:%02d",

      Time.Day,

      Time.Month,

      Time.Year,

      Time.Hour,

      Time.Minute,

      Time.Second

    );

  } else {

    UnicodeSPrint(TimeString, 50 * sizeof(CHAR16), L"[Date/Time Not Available]");

  }

}



VOID

DrawBlueBackgroundOnce(

  VOID

  )

{

  UINTN Index, Col;

  

  GetConsoleSize();

  

  gST->ConOut->ClearScreen(gST->ConOut);

  gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);

  

  for (Index = 0; Index < mConsoleRows; Index++) {

    gST->ConOut->SetCursorPosition(gST->ConOut, 0, Index);

    for (Col = 0; Col < mConsoleColumns; Col++) {

      Print(L" ");

    }

  }

  

  gST->ConOut->SetCursorPosition(gST->ConOut, 1, 0);

  gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);

  Print(L"BIOS Setup Utility");

  

  gST->ConOut->SetCursorPosition(gST->ConOut, (mConsoleColumns - 14) / 2, 1);

  gST->ConOut->SetAttribute(gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_BLUE);

  Print(L"BHARAT BIOS CDAC");

}



VOID

UpdateTabsOnly(

  VOID

  )

{

  UINTN Index, Col;

  

  gST->ConOut->SetCursorPosition(gST->ConOut, 0, 2);

  gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);

  for (Col = 0; Col < mConsoleColumns; Col++) {

    Print(L" ");

  }

  

  for (Index = 0; Index < mTotalMainTabs; Index++) {

    gST->ConOut->SetCursorPosition(gST->ConOut, mMainTabs[Index].StartCol, 2);

    

    if (Index == mSelectedMainTab) {

      gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);

    } else {

      gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);

    }

    

    Print(L" %s ", mMainTabs[Index].MenuText);

  }

}



VOID

UpdateSubmenuOnly(

  VOID

  )

{

  UINTN Index, Col;

  SUBMENU_ITEM *CurrentSubmenu;

  UINTN ItemCount;

  UINTN SeparatorCol = (mConsoleColumns * 3) / 4;

  

  CurrentSubmenu = GetCurrentSubmenuArray(&ItemCount);

  mTotalSubItems = ItemCount;

  

  for (Index = 4; Index < mConsoleRows - 3; Index++) {

    gST->ConOut->SetCursorPosition(gST->ConOut, 1, Index);

    gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);

    for (Col = 1; Col < SeparatorCol - 1; Col++) {

      Print(L" ");

    }

  }

  

  for (Index = 0; Index < ItemCount && Index < 15; Index++) {

    gST->ConOut->SetCursorPosition(gST->ConOut, 3, Index + 5);

    

    if (Index == mSelectedSubItem) {

      gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);

    } else {

      gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);

    }

    

    Print(L"%s", CurrentSubmenu[Index].MenuText);

  }

}



VOID

UpdateInfoPanelOnly(

  VOID

  )

{

  SUBMENU_ITEM *CurrentSubmenu;

  UINTN ItemCount;

  CHAR16 TimeString[100];

  UINTN SeparatorCol = (mConsoleColumns * 3) / 4;

  UINTN RightStart = SeparatorCol + 2;

  UINTN Index, Col;

  

  CurrentSubmenu = GetCurrentSubmenuArray(&ItemCount);

  

  for (Index = 4; Index < mConsoleRows - 3; Index++) {

    gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, Index);

    gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);

    for (Col = RightStart; Col < mConsoleColumns - 1; Col++) {

      Print(L" ");

    }

  }

  

  gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, 5);

  gST->ConOut->SetAttribute(gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);

  Print(L"Selected Item:");

  

  if (mSelectedSubItem < ItemCount) {

    gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, 7);

    gST->ConOut->SetAttribute(gST->ConOut, EFI_DARKGRAY | EFI_BACKGROUND_LIGHTGRAY);

    Print(L"%s", CurrentSubmenu[mSelectedSubItem].MenuText);

    

    gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, 9);

    gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);

    

    UINTN RightWidth = mConsoleColumns - RightStart - 2;

    CHAR16 *Description = CurrentSubmenu[mSelectedSubItem].Description;

    UINTN DescLen = StrLen(Description);

    

    if (DescLen <= RightWidth) {

      Print(L"%s", Description);

    } else {

      UINTN CurrentPos = 0;

      UINTN CurrentLine = 9;

      

      while (CurrentPos < DescLen && CurrentLine < mConsoleRows - 6) {

        gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, CurrentLine);

        gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);

        

        UINTN CharsToShow = RightWidth;

        if (CurrentPos + CharsToShow > DescLen) {

          CharsToShow = DescLen - CurrentPos;

        }

        

        if (CurrentPos + CharsToShow < DescLen) {

          UINTN LastSpace = CharsToShow;

          for (UINTN i = CharsToShow; i > 0; i--) {

            if (Description[CurrentPos + i - 1] == L' ') {

              LastSpace = i;

              break;

            }

          }

          if (LastSpace < CharsToShow) {

            CharsToShow = LastSpace;

          }

        }

        

        for (UINTN i = 0; i < CharsToShow && CurrentPos < DescLen; i++) {

          Print(L"%c", Description[CurrentPos]);

          CurrentPos++;

        }

        

        if (CurrentPos < DescLen && Description[CurrentPos] == L' ') {

          CurrentPos++;

        }

        

        CurrentLine++;

      }

    }

     // Special case for CPU Information preview
    if (mSelectedMainTab == 0 && mSelectedSubItem == 1) {
      gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, 12);
      gST->ConOut->SetAttribute(gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);
      Print(L"Quick Info:");
      gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, 13);
      gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);
      Print(L"Vendor: %s", mCpuInfo.VendorString);
      gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, 14);
      Print(L"Cores: %d", mCpuInfo.NumberOfProcessors);
      gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, 16);
      Print(L"Press Enter for");
      gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, 17);
      Print(L"detailed view");
    }

    // Special case for Date & Time

    if (mSelectedMainTab == 0 && mSelectedSubItem == 4) {

      GetCurrentTime(TimeString);

      gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, 12);

      gST->ConOut->SetAttribute(gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);

      Print(L"Current:");

      gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, 13);

      

      UINTN TimeLen = StrLen(TimeString);

      if (TimeLen <= RightWidth) {

        Print(L"%s", TimeString);

      } else {

        Print(L"%.15s", TimeString);

        gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, 14);

        Print(L"%s", &TimeString[15]);

      }

      

      gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, 16);

      Print(L"Press Enter to");

      gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, 17);

      Print(L"open calendar");

    }

    

    // Special case for USB Authentication

    if (mSelectedMainTab == 0 && mSelectedSubItem == 5) {

      gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, 12);

      gST->ConOut->SetAttribute(gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);

      Print(L"Status:");

      gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, 13);

      

      if (mUsbAuthenticated) {

        gST->ConOut->SetAttribute(gST->ConOut, EFI_GREEN | EFI_BACKGROUND_LIGHTGRAY);

        Print(L"Authenticated");

      } else {

        gST->ConOut->SetAttribute(gST->ConOut, EFI_RED | EFI_BACKGROUND_LIGHTGRAY);

        Print(L"Not Authenticated");

      }

      

      gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, 15);

      gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);

      Print(L"Press Enter to");

      gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, 16);

      Print(L"re-authenticate");

    }

  }

  

  gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, mConsoleRows - 5);

  gST->ConOut->SetAttribute(gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);

  Print(L"Tab: %s", mMainTabs[mSelectedMainTab].MenuText);

}



VOID

DrawInitialInterface(

  VOID

  )

{

  UINTN Row, Col;

  UINTN SeparatorCol = (mConsoleColumns * 3) / 4;

  

  DrawBlueBackgroundOnce();

  

  for (Row = 3; Row < mConsoleRows - 2; Row++) {

    gST->ConOut->SetCursorPosition(gST->ConOut, 0, Row);

    

    gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);

    for (Col = 0; Col < SeparatorCol; Col++) {

      Print(L" ");

    }

    

    if (SeparatorCol < mConsoleColumns) {

      gST->ConOut->SetAttribute(gST->ConOut, EFI_BLUE | EFI_BACKGROUND_LIGHTGRAY);

      Print(L"│");

    }

    

    gST->ConOut->SetAttribute(gST->ConOut, EFI_BLACK | EFI_BACKGROUND_LIGHTGRAY);

    for (Col = SeparatorCol + 1; Col < mConsoleColumns; Col++) {

      Print(L" ");

    }

  }

  

  gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);

  for (Row = mConsoleRows - 2; Row < mConsoleRows; Row++) {

    gST->ConOut->SetCursorPosition(gST->ConOut, 0, Row);

    for (Col = 0; Col < mConsoleColumns; Col++) {

      Print(L" ");

    }

  }

  

  gST->ConOut->SetCursorPosition(gST->ConOut, 2, mConsoleRows - 1);

  gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);

  Print(L"←→: Select Screen  ↑↓: Select Item  Enter: Select  ESC: Exit");

  

  UpdateTabsOnly();

  UpdateSubmenuOnly();

  UpdateInfoPanelOnly();

  

  mInterfaceInitialized = TRUE;

}



VOID

HandleTabSelection(

  VOID

  )

{

  UINTN SeparatorCol = (mConsoleColumns * 3) / 4;

  UINTN RightStart = SeparatorCol + 2;

  EFI_STATUS Status;

  
   // Special case: CPU Information
  if (mSelectedMainTab == 0 && mSelectedSubItem == 1) { // Index 1 for CPU Information
    mCurrentMenuLevel = MENU_LEVEL_CPU_INFO;
    DisplayCpuInformation();
    HandleCpuInfoNavigation();
    return;
  }
  
  

  // Special case: Change Date & Time

  if (mSelectedMainTab == 0 && mSelectedSubItem == 4) {

    mCurrentMenuLevel = MENU_LEVEL_CALENDAR;

    InitializeCalendar();

    DisplayCalendar();

    HandleCalendarNavigation();

    return;

  }

  

  // Special case: USB Authentication

  if (mSelectedMainTab == 0 && mSelectedSubItem == 5) {

    // Show authentication in progress message

    gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, 18);

    gST->ConOut->SetAttribute(gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_LIGHTGRAY);

    Print(L"Authenticating...");

    

    Status = PerformUsbAuthentication();

    

    if (Status == EFI_SUCCESS) {

      mUsbAuthenticated = TRUE;

    } else {

      mUsbAuthenticated = FALSE;

      // System will shutdown from PerformUsbAuthentication if authentication fails

      return;

    }

    

    // Redraw interface after authentication

    DrawInitialInterface();

    return;

  }

  

  switch (mSelectedMainTab) {

    case 3: // Save & Exit

      if (mSelectedSubItem == 0) { // Save Changes and Exit

        gST->ConOut->ClearScreen(gST->ConOut);

        gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);

        gST->ConOut->SetCursorPosition(gST->ConOut, (mConsoleColumns - 35) / 2, mConsoleRows / 2);

        gST->ConOut->SetAttribute(gST->ConOut, EFI_GREEN | EFI_BACKGROUND_BLACK);

        Print(L"Settings saved. Shutting down system...");

        gBS->Stall(2000000);

        gRT->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);

        mInBiosSetup = FALSE;

      } else if (mSelectedSubItem == 1) { // Discard Changes and Exit

        gST->ConOut->ClearScreen(gST->ConOut);

        gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);

        gST->ConOut->SetCursorPosition(gST->ConOut, (mConsoleColumns - 40) / 2, mConsoleRows / 2);

        gST->ConOut->SetAttribute(gST->ConOut, EFI_RED | EFI_BACKGROUND_BLACK);

        Print(L"Exiting without saving. Shutting down...");

        gBS->Stall(2000000);

        gRT->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);

        mInBiosSetup = FALSE;

      } else if (mSelectedSubItem == 2) { // Save Changes

        gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, 15);

        gST->ConOut->SetAttribute(gST->ConOut, EFI_GREEN | EFI_BACKGROUND_LIGHTGRAY);

        Print(L"Saved");

        gBS->Stall(1500000);

        UpdateInfoPanelOnly();

      } else if (mSelectedSubItem == 3) { // Discard Changes

        gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, 15);

        gST->ConOut->SetAttribute(gST->ConOut, EFI_RED | EFI_BACKGROUND_LIGHTGRAY);

        Print(L"Discarded");

        gBS->Stall(1500000);

        UpdateInfoPanelOnly();

      } else if (mSelectedSubItem == 4) { // Load Setup Defaults

        gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, 15);

        gST->ConOut->SetAttribute(gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_LIGHTGRAY);

        Print(L"Defaults");

        gBS->Stall(1500000);

        UpdateInfoPanelOnly();

      }

      break;

      

    default:

      gST->ConOut->SetCursorPosition(gST->ConOut, RightStart, 15);

      gST->ConOut->SetAttribute(gST->ConOut, EFI_YELLOW | EFI_BACKGROUND_LIGHTGRAY);

      Print(L"Action...");

      gBS->Stall(800000);

      UpdateInfoPanelOnly();

      break;

  }

}



VOID

DisplayBiosSetupForm(

  VOID

  )

{

  mSelectedMainTab = 0;

  mSelectedSubItem = 0;

  mTotalMainTabs = ARRAY_SIZE(mMainTabs);

  mInBiosSetup = TRUE;

  mInterfaceInitialized = FALSE;

  mCurrentMenuLevel = MENU_LEVEL_MAIN_SUBMENU;

  

  DrawInitialInterface();

  

  gST->ConIn->Reset(gST->ConIn, FALSE);

  

  HandleBiosSetupNavigation();

}



VOID

HandleBiosSetupNavigation(

  VOID

  )

{

  EFI_STATUS     Status;

  EFI_INPUT_KEY  Key;

  

  while (mInBiosSetup) {

    if (mCurrentMenuLevel == MENU_LEVEL_CALENDAR) {

      HandleCalendarNavigation();

      continue;

    }

 if (mCurrentMenuLevel == MENU_LEVEL_CPU_INFO) {
      HandleCpuInfoNavigation();
      continue;
    }
    

    Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);

    

    if (!EFI_ERROR(Status)) {

      switch (Key.ScanCode) {

        case SCAN_LEFT:

          if (mSelectedMainTab > 0) {

            mSelectedMainTab--;

          } else {

            mSelectedMainTab = mTotalMainTabs - 1;

          }

          mSelectedSubItem = 0;

          

          UpdateTabsOnly();

          UpdateSubmenuOnly();

          UpdateInfoPanelOnly();

          break;

          

        case SCAN_RIGHT:

          if (mSelectedMainTab < mTotalMainTabs - 1) {

            mSelectedMainTab++;

          } else {

            mSelectedMainTab = 0;

          }

          mSelectedSubItem = 0;

          

          UpdateTabsOnly();

          UpdateSubmenuOnly();

          UpdateInfoPanelOnly();

          break;

          

        case SCAN_UP:

          if (mSelectedSubItem > 0) {

            mSelectedSubItem--;

          } else {

            mSelectedSubItem = mTotalSubItems - 1;

          }

          

          UpdateSubmenuOnly();

          UpdateInfoPanelOnly();

          break;

          

        case SCAN_DOWN:

          if (mSelectedSubItem < mTotalSubItems - 1) {

            mSelectedSubItem++;

          } else {

            mSelectedSubItem = 0;

          }

          

          UpdateSubmenuOnly();

          UpdateInfoPanelOnly();

          break;

          

        case SCAN_ESC:

          mInBiosSetup = FALSE;

          gST->ConOut->ClearScreen(gST->ConOut);

          gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);

          gST->ConOut->SetCursorPosition(gST->ConOut, (mConsoleColumns - 40) / 2, mConsoleRows / 2);

          Print(L"Exiting BIOS Setup. Shutting down system...");

          gBS->Stall(2000000);

          gRT->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);

          return;

          

        default:

          if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {

            HandleTabSelection();

            if (!mInBiosSetup) {

              return;

            }

          }

          break;

      }

    } else {

      gBS->Stall(50000);

    }

  }

}






//
// Logo and Event Handling Functions
//



VOID
EFIAPI
KeyboardMonitor(
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{

  EFI_STATUS     Status;

  EFI_INPUT_KEY  Key;

  
  if (!mMessageShown || mInBiosSetup) {
    return;
  }
  

  Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);

  

  if (!EFI_ERROR(Status)) {

    if (Key.ScanCode == SCAN_F12) {

      DEBUG((DEBUG_INFO, "F12 key pressed - showing BIOS setup form\n"));

      

      gBS->SetTimer(mKeyMonitorEvent, TimerCancel, 0);

      gBS->SetTimer(mTimerEvent, TimerCancel, 0);

      

      DisplayBiosSetupForm();

      

      gST->ConOut->ClearScreen(gST->ConOut);

      gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);

      gST->ConOut->SetCursorPosition(gST->ConOut, (mConsoleColumns - 40) / 2, mConsoleRows / 2);

      Print(L"BIOS Setup completed. Shutting down system...");

      gBS->Stall(2000000);

      gRT->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);

      return;

    }

  }

}



VOID

EFIAPI

TimerCallback(

  IN EFI_EVENT  Event,

  IN VOID       *Context

  )

{

  mTimerCounter++;

  

  if (mTimerCounter == 50 && !mMessageShown) { // 50 * 100ms = 5 seconds

    gST->ConOut->ClearScreen(gST->ConOut);

    

    GetConsoleSize();

    

    gST->ConOut->SetCursorPosition(gST->ConOut, (mConsoleColumns - 28) / 2, mConsoleRows / 2 + 5);

    gST->ConOut->SetAttribute(gST->ConOut, EFI_WHITE | EFI_BACKGROUND_BLACK);

    Print(L"Press F12 key for BIOS setup");

    

    mMessageShown = TRUE;

    

    gST->ConIn->Reset(gST->ConIn, FALSE);

    

    DEBUG((DEBUG_INFO, "Message displayed, waiting for F12 key\n"));

  }

}



VOID

SetupLogoTiming(

  VOID

  )

{

  EFI_STATUS  Status;

  

  if (mLogoDisplayed) {

    return;

  }

  

  DEBUG((DEBUG_INFO, "Setting up logo timing and keyboard monitoring\n"));

  

  Status = gBS->CreateEvent(

                  EVT_TIMER | EVT_NOTIFY_SIGNAL,

                  TPL_CALLBACK,

                  TimerCallback,

                  NULL,

                  &mTimerEvent

                  );

  

  if (!EFI_ERROR(Status)) {

    Status = gBS->SetTimer(

                    mTimerEvent,

                    TimerPeriodic,

                    1000000  // 100ms in 100ns units

                    );

    if (EFI_ERROR(Status)) {

      DEBUG((DEBUG_ERROR, "Failed to set timer for mTimerEvent: %r\n", Status));

    }

  } else {

    DEBUG((DEBUG_ERROR, "Failed to create mTimerEvent: %r\n", Status));

  }

  

  Status = gBS->CreateEvent(

                  EVT_TIMER | EVT_NOTIFY_SIGNAL,

                  TPL_CALLBACK,

                  KeyboardMonitor,

                  NULL,

                  &mKeyMonitorEvent

                  );

  

  if (!EFI_ERROR(Status)) {

    Status = gBS->SetTimer(

                    mKeyMonitorEvent,

                    TimerPeriodic,

                    500000  // 50ms in 100ns units

                    );

    if (EFI_ERROR(Status)) {

      DEBUG((DEBUG_ERROR, "Failed to set timer for mKeyMonitorEvent: %r\n", Status));

    }

  } else {

    DEBUG((DEBUG_ERROR, "Failed to create mKeyMonitorEvent: %r\n", Status));

  }

  

  mLogoDisplayed = TRUE;

}





//
// Platform Logo Protocol Implementation
//


EFI_STATUS
EFIAPI
GetImage(
  IN     EDKII_PLATFORM_LOGO_PROTOCOL        *This,

  IN OUT UINT32                              *Instance,

  OUT EFI_IMAGE_INPUT                        *Image,

  OUT EDKII_PLATFORM_LOGO_DISPLAY_ATTRIBUTE  *Attribute,

  OUT INTN                                   *OffsetX,

  OUT INTN                                   *OffsetY

  )

{

  UINT32      Current;

  EFI_STATUS  Status;



  if ((Instance == NULL) || (Image == NULL) ||

      (Attribute == NULL) || (OffsetX == NULL) || (OffsetY == NULL))

  {

    return EFI_INVALID_PARAMETER;

  }



  Current = *Instance;

  if (Current >= ARRAY_SIZE(mLogos)) {

    return EFI_NOT_FOUND;

  }



  (*Instance)++;

  *Attribute = mLogos[Current].Attribute;

  *OffsetX   = mLogos[Current].OffsetX;

  *OffsetY   = mLogos[Current].OffsetY;

  

  Status = mHiiImageEx->GetImageEx(mHiiImageEx, mHiiHandle, mLogos[Current].ImageId, Image);

  

  if (Current == 0 && !EFI_ERROR(Status)) {

    DEBUG((DEBUG_INFO, "Logo displayed, setting up timing\n"));

    SetupLogoTiming();

  }

  

  return Status;

}



EDKII_PLATFORM_LOGO_PROTOCOL  mPlatformLogo = {

  GetImage

};



//
// Driver Cleanup
//
EFI_STATUS
EFIAPI
LogoDriverUnload(
  IN EFI_HANDLE  ImageHandle
  )
{
  if (mTimerEvent != NULL) {
    gBS->CloseEvent(mTimerEvent);
    mTimerEvent = NULL;
  }
  
  if (mKeyMonitorEvent != NULL) {
    gBS->CloseEvent(mKeyMonitorEvent);

    mKeyMonitorEvent = NULL;

  }

  

  if (mUsbAuthEvent != NULL) {

    gBS->CloseEvent(mUsbAuthEvent);

    mUsbAuthEvent = NULL;

  }

  

  return EFI_SUCCESS;

}



//

// Driver Entry Point

//

EFI_STATUS

EFIAPI

InitializeLogo(

  IN EFI_HANDLE        ImageHandle,

  IN EFI_SYSTEM_TABLE  *SystemTable

  )

{

  EFI_STATUS                   Status;

  EFI_HII_PACKAGE_LIST_HEADER  *PackageList;

  EFI_HII_DATABASE_PROTOCOL    *HiiDatabase;

  EFI_HANDLE                   Handle;



  DEBUG((DEBUG_INFO, "Combined UEFI Driver: USB Authentication + BIOS Setup\n"));



  // Initialize global variables

  mInBiosSetup = FALSE;

  mSelectedMainTab = 0;

  mSelectedSubItem = 0;

  mCurrentMenuLevel = MENU_LEVEL_MAIN_MENU;

  mTotalMainTabs = ARRAY_SIZE(mMainTabs);

  mInterfaceInitialized = FALSE;

  mUsbAuthenticated = FALSE;

  

  // Initialize calendar state

  mCalendar.InCalendarMode = FALSE;

  mCalendar.SelectedField = 0;


 // Initialize CPU information - ADD THIS PART
  ZeroMem(&mCpuInfo, sizeof(CPU_INFO));
  CollectCpuInformation();
  DEBUG((DEBUG_INFO, "CPU Information Collected: %s\n", mCpuInfo.VendorString));


  // Locate HII Database Protocol

  Status = gBS->LocateProtocol(

                  &gEfiHiiDatabaseProtocolGuid,

                  NULL,

                  (VOID **)&HiiDatabase

                  );

  if (EFI_ERROR(Status)) {

    DEBUG((DEBUG_ERROR, "Failed to locate HII Database Protocol: %r\n", Status));

    return Status;

  }



  // Locate HII Image Ex Protocol

  Status = gBS->LocateProtocol(

                  &gEfiHiiImageExProtocolGuid,

                  NULL,

                  (VOID **)&mHiiImageEx

                  );

  if (EFI_ERROR(Status)) {

    DEBUG((DEBUG_ERROR, "Failed to locate HII Image Ex Protocol: %r\n", Status));

    return Status;

  }



  // Retrieve HII package list from ImageHandle

  Status = gBS->OpenProtocol(

                  ImageHandle,

                  &gEfiHiiPackageListProtocolGuid,

                  (VOID **)&PackageList,

                  ImageHandle,

                  NULL,

                  EFI_OPEN_PROTOCOL_GET_PROTOCOL

                  );

  if (EFI_ERROR(Status)) {

    DEBUG((DEBUG_ERROR, "HII Image Package with logo not found in PE/COFF resource section\n"));

    return Status;

  }



  // Publish HII package list to HII Database

  Status = HiiDatabase->NewPackageList(

                          HiiDatabase,

                          PackageList,

                          NULL,

                          &mHiiHandle

                          );

  if (EFI_ERROR(Status)) {

    DEBUG((DEBUG_ERROR, "Failed to add package list to HII Database: %r\n", Status));

    return Status;

  }



  // Install Platform Logo Protocol

  Handle = NULL;

  Status = gBS->InstallMultipleProtocolInterfaces(

                  &Handle,

                  &gEdkiiPlatformLogoProtocolGuid,

                  &mPlatformLogo,

                  NULL

                  );

  

  if (EFI_ERROR(Status)) {

    DEBUG((DEBUG_ERROR, "Failed to install Platform Logo Protocol: %r\n", Status));

    HiiDatabase->RemovePackageList(HiiDatabase, mHiiHandle);

    return Status;

  }



  DEBUG((DEBUG_INFO, "Combined UEFI Driver: USB Auth + BIOS Setup Complete\n"));

  

  return EFI_SUCCESS;

}









