/** @file
  This sample application bases on HelloWorld PCD setting
  to print "UEFI Hello World!" to the UEFI Console.

  Copyright (c) 2006 - 2018, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/




/**

#include <Uefi.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/PrintLib.h>
#include <Protocol/SimpleTextIn.h>

**/


//
// String token ID of help message text.
// Shell supports to find help message in the resource section of an application image if
// .MAN file is not found. This global variable is added to make build tool recognizes
// that the help string is consumed by user and then build tool will add the string into
// the resource section. Thus the application can use '-?' option to show help message in
// Shell.
//


  //     GLOBAL_REMOVE_IF_UNREFERENCED EFI_STRING_ID  mStringHelpTokenId = STRING_TOKEN (STR_HELLO_WORLD_HELP_INFORMATION);


/**
  The user Entry Point for Application. The user code starts with this function
  as the real entry point for the application.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
/**

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  UINT32  Index;

  Index = 0;
  EFI_INPUT_KEY Key;
  UINTN EventIndex;

  //
  // Clear the screen
  //
  SystemTable->ConOut->ClearScreen(SystemTable->ConOut);



  //
  // Three PCD type (FeatureFlag, UINT32 and String) are used as the sample.
  //
  if (FeaturePcdGet (PcdHelloWorldPrintEnable)) {
    for (Index = 0; Index < PcdGet32 (PcdHelloWorldPrintTimes); Index++) {
      //
      // Use UefiLib Print API to print string to UEFI console
      //
      Print ((CHAR16 *)PcdGetPtr (PcdHelloWorldPrintString));
    }
  }

// Print press the key message
  Print(L"Press the Key\n");

  // wait for a key press
  SystemTable->ConIn->Reset(SystemTable->ConIn,FALSE);
  SystemTable->BootServices->WaitForEvent(1, &(SystemTable->ConIn->WaitForKey), &EventIndex);  SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key);

  // Print key press detected message
  Print(L"key press detected: %c %d \n",Key.UnicodeChar, Key.UnicodeChar);

  // Shutdown the system if 's' key pressed
  if (Key.UnicodeChar =='s') 
  
  gRT->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0,NULL);

 return EFI_SUCCESS;
}

**/



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/** @file
  This sample application bases on HelloWorld PCD setting
  to print "UEFI Hello World!" to the UEFI Console.

  Copyright (c) 2006 - 2018, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
/**

#include <Uefi.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/PrintLib.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/UsbIo.h>



#define UTILITY_VERSION L"20190408"
#undef DEBUG

#define FIXED_VENDOR_ID 0x0781      // Replace with fixed vendor id
#define FIXED_PRODUCT_ID 0x5595     // Replace with fixed product id
#define FIXED_SERIAL_NUMBER L"040144d2dd4c4643e76cd30e263d37e6fff7d68d4ecb3ac1374d551837620beec3b2000000000000000000008cd839090083751895558107b628040f"    // Replace with fixed serial number
#define MAX_ATTEMPTS 2

**/
//
// String token ID of help message text.
// Shell supports to find help message in the resource section of an application image if
// .MAN file is not found. This global variable is added to make build tool recognizes
// that the help string is consumed by user and then build tool will add the string into
// the resource section. Thus the application can use '-?' option to show help message in
// Shell.
//



// GLOBAL_REMOVE_IF_UNREFERENCED EFI_STRING_ID mStringHelpTokenId = STRING_TOKEN (STR_HELLO_WORLD_HELP_INFORMATION);







/**
  The user Entry Point for Application. The user code starts with this function
  as the real entry point for the application.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

 **/

/**
EFI_STATUS
	EFIAPI
UefiMain (
		IN EFI_HANDLE        ImageHandle,
		IN EFI_SYSTEM_TABLE  *SystemTable
	 )
{
	EFI_USB_DEVICE_DESCRIPTOR DevDesc;
	EFI_USB_INTERFACE_DESCRIPTOR IfDesc;
	EFI_USB_IO_PROTOCOL *UsbIo;
	EFI_STATUS Status = EFI_SUCCESS;
	EFI_HANDLE *HandleBuffer = NULL;
	// BOOLEAN LangFound;
	CHAR16 *Manufacturer;
	CHAR16 *Product;
	CHAR16 *SerialNumber;
	UINT16 *LangIdTable;
	UINT16 TableSize;
	UINTN HandleCount;
	UINT8 Attempts = 0;   // Track the number of attempts

       
       

        


	// Locate USB devices
	Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiUsbIoProtocolGuid, NULL, &HandleCount, &HandleBuffer);
	if (EFI_ERROR(Status)) 
	{
		Print(L"ERROR: LocateHandleBuffer.\n");
		return Status;
	}

	Print(L"\n");
	Print(L"  VendorID  ProductID  Manufacturer/Product/SerialNumber\n");
	Print(L"  -------------------------------------------------------\n");
	Print(L"\n");

	for (UINT8 Index = 0; Index < HandleCount && Attempts < MAX_ATTEMPTS; Index++) 
	{
		Status = gBS->HandleProtocol(HandleBuffer[Index], &gEfiUsbIoProtocolGuid, (VOID**)&UsbIo);
		if (EFI_ERROR(Status)) 
		{
			Print(L"ERROR: Open UsbIo.\n");
			FreePool(HandleBuffer);
			return Status;
		}

		Status = UsbIo->UsbGetDeviceDescriptor(UsbIo, &DevDesc);
		if (EFI_ERROR(Status)) 
		{
			Print(L"ERROR: UsbGetDeviceDescriptor.\n");
			FreePool(HandleBuffer);
			return Status;
		}

		Status = UsbIo->UsbGetInterfaceDescriptor(UsbIo, &IfDesc);
		if (EFI_ERROR(Status)) 
		{
			Print(L"ERROR: UsbGetInterfaceDescriptor.\n");
			FreePool(HandleBuffer);
			return Status;
		}

		TableSize = 0;
		LangIdTable = NULL;
		Status = UsbIo->UsbGetSupportedLanguages(UsbIo, &LangIdTable, &TableSize);
		FreePool(LangIdTable);

		Status = UsbIo->UsbGetStringDescriptor(UsbIo, 0x0409, DevDesc.StrManufacturer, &Manufacturer);
		if (EFI_ERROR(Status)) 
		{
			Manufacturer = L"Unknown";
		}

		Status = UsbIo->UsbGetStringDescriptor(UsbIo, 0x0409, DevDesc.StrProduct, &Product);
		if (EFI_ERROR(Status)) 
		{
			Product = L"Unknown";
		}

		Status = UsbIo->UsbGetStringDescriptor(UsbIo, 0x0409, DevDesc.StrSerialNumber, &SerialNumber);
		if (EFI_ERROR(Status)) 
		{
			SerialNumber = L"Unknown";
		}

		// Compare the vendor ID, product ID, and serial number
		if (DevDesc.IdVendor == FIXED_VENDOR_ID && DevDesc.IdProduct == FIXED_PRODUCT_ID && 
				StrCmp(SerialNumber, FIXED_SERIAL_NUMBER) == 0) 
		{
			Print(L"hello anand authorised access\n");
			
		      }
	       	else 
		{
			Print(L"unauthorised access\n");
			Attempts++;     // Increment attempts counter
			if (Attempts >= MAX_ATTEMPTS) 
			{
				Print(L"Maximum unauthorised attempts reached.\n");
				FreePool(HandleBuffer);
				return EFI_ACCESS_DENIED;
				
			}
		}
	

		Print(L"    %04X      %04X     %s, %s, %s\n",
				DevDesc.IdVendor, DevDesc.IdProduct, Manufacturer, Product, SerialNumber);
	}			
	
	Print(L"\n");

	FreePool(HandleBuffer);

	return EFI_SUCCESS;
}

**/



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//serial communication code 


#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/SerialIo.h>

EFI_STATUS EFIAPI UefiMain(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable) {
    EFI_STATUS Status;
    EFI_HANDLE *Handles = NULL;
    UINTN HandleCount = 0;

    Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiSerialIoProtocolGuid, NULL, &HandleCount, &Handles);
    if (EFI_ERROR(Status) || HandleCount == 0) {
        Print(L"No serial devices found.\n");
        return Status;
    }

    Print(L"Found %u serial devices:\n", HandleCount);

    // Dump all available serial handles
    for (UINTN i = 0; i < HandleCount; ++i) {
        EFI_SERIAL_IO_PROTOCOL *Tmp;
        gBS->HandleProtocol(Handles[i], &gEfiSerialIoProtocolGuid, (VOID**)&Tmp);
        Print(L"[%u] Handle=%p, BaudRate=%u\n", i, Handles[i], Tmp->Mode->BaudRate);
    }

    // 🟡 Pick a handle manually – yahan index 0 select kiya gaya hai (aap modify bhi kar sakte ho)
    UINTN SelectedIndex = 0;
    EFI_SERIAL_IO_PROTOCOL *SerialIo;
    Status = gBS->HandleProtocol(Handles[SelectedIndex], &gEfiSerialIoProtocolGuid, (VOID**)&SerialIo);
    if (EFI_ERROR(Status)) {
        Print(L"Failed to get SerialIO protocol for selected handle.\n");
        return Status;
    }

    // Free handles memory
    FreePool(Handles);

    // Configure serial port
    Status = SerialIo->SetAttributes(
        SerialIo,
        115200,       // BaudRate
        0,            // ReceiveFifoDepth (0 = default)
        100000,       // Timeout in 100ns units (100000 = 10ms)
        DefaultParity,
        8,            // DataBits
        OneStopBit    // StopBits
    );
    if (EFI_ERROR(Status)) {
        Print(L"SetAttributes failed: %r\n", Status);
        return Status;
    }

    Print(L"Waiting for serial input on selected port...\n");

    // Start polling serial input
    CHAR8 Buffer[128];
    UINTN Size;

    while (TRUE) {
        Size = sizeof(Buffer) - 1;
        Status = SerialIo->Read(SerialIo, &Size, Buffer);

        if (Status == EFI_TIMEOUT || Status == EFI_NOT_READY) {
            gBS->Stall(1000); // 1ms sleep
            continue;
        }

        if (EFI_ERROR(Status)) {
            Print(L"Serial Read Error: %r\n", Status);
            break;
        }

        if (Size > 0) {
            Buffer[Size] = '\0'; // Null terminate
            Print(L"Received (%u bytes): %a\n", Size, Buffer);
        }
    }

    return EFI_SUCCESS;
}





































