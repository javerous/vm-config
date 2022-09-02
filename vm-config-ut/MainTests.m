/*
 *  MainTests.m
 *
 *  Copyright 2022 Avérous Julien-Pierre
 *
 *  This file is part of vm-config.
 *
 *  vm-config is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  vm-config is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with vm-config.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#import <XCTest/XCTest.h>

#import <os/lock.h>

#import "main.h"

#import "SMTestsTools.h"

#import "SMVMwareVMX.h"
#import "SMVMwareVMXHelper.h"

#import "SMVMwareNVRAM.h"
#import "SMVMwareNVRAMHelper.h"

#import "SMVMwareVMXHelper.h"


/*
** Defines
*/
#pragma mark - Defines

#define XCTAssertDefaultMain(ExpectedExit) 															\
	SMDeclareDefaultFiles;																			\
	XCTAssertEqual(internal_main(sizeof(argv) / sizeof(*argv), argv, fout, ferr), ExpectedExit);	\
	fflush(fout); 																					\
	fflush(ferr)


/*
** Types
*/
#pragma mark - Types

typedef struct
{
	const char *key;
	const char *value;
} SMVMXEntryTest;

typedef struct
{
	efi_guid_t guid;
	
	const char *name;
	
	uint8_t	value[100];
	size_t	value_size;
} SMNVRAMEFIVariableTest;




/*
** MainTests
*/
#pragma mark - MainTests

@interface MainTests : XCTestCase
{
	NSString *_testDirectory;
}

@end

@implementation MainTests

#pragma mark - Setup

- (void)setUp
{
	NSString *tempDirectory = [NSString stringWithFormat:@"%@-vm-config-ut", [NSUUID UUID].UUIDString];
	
	_testDirectory = [NSTemporaryDirectory() stringByAppendingPathComponent:tempDirectory];
	
	NSAssert([[NSFileManager defaultManager] createDirectoryAtPath:_testDirectory withIntermediateDirectories:YES attributes:nil error:nil], @"cannot create temp directory");
	
	self.continueAfterFailure = NO;
}

- (void)tearDown
{
	[[NSFileManager defaultManager] removeItemAtPath:_testDirectory error:nil];
}


#pragma mark - Tests

- (void)testNotArguments
{
	// Test main.
	const char *argv[] = {
		"ut-main"
	};
		
	XCTAssertDefaultMain(SMMainExitInvalidArgs);
	
	// Check output.
	XCTAssertEqual(sout, 0);
	XCTAssertGreaterThan(serr, 20);
}

- (void)testVersion
{
	// Test main.
	const char *argv[] = {
		"ut-main",
		"version"
	};
	
	XCTAssertDefaultMain(SMMainExitSuccess);
	
	// Check output.
	XCTAssertContainString(*bout, sout, "version");
	XCTAssertEqual(serr, 0);
}

- (void)testInvalidVerb
{
	// Test main.
	const char *argv[] = {
		"ut-main",
		"invalid-verb"
	};
	
	XCTAssertDefaultMain(SMMainExitInvalidArgs);
	
	// Check output.
	XCTAssertEqual(sout, 0);
	XCTAssertContainString(*berr, serr, "Error");
}


#pragma mark > Show

- (void)testShowError
{
	// Test main.
	const char *argv[] = {
		"ut-main",
		"show",
	};
	
	XCTAssertDefaultMain(SMMainExitInvalidArgs);
	
	// Check output.
	XCTAssertEqual(sout, 0);
	XCTAssertContainString(*berr, serr, "Error");
}

- (void)testShowExtraArgs
{
	// Test main.
	const char *argv[] = {
		"ut-main",
		"show",
		_testDirectory.fileSystemRepresentation,
		"--all",
		"blah",
	};
	
	XCTAssertDefaultMain(SMMainExitInvalidArgs);
	
	// Check output.
	XCTAssertEqual(sout, 0);
	XCTAssertContainString(*berr, serr, "Error");
}

- (void)testShowInvalidArgs
{
	// Test main.
	const char *argv[] = {
		"ut-main",
		"show",
		_testDirectory.fileSystemRepresentation,
		"--unknow-arg",
	};
	
	XCTAssertDefaultMain(SMMainExitInvalidArgs);
	
	// Check output.
	XCTAssertEqual(sout, 0);
	XCTAssertContainString(*berr, serr, "Error");
}

- (void)testShowInvalidVM
{
	// Test main.
	const char *argv[] = {
		"ut-main",
		"show",
		_testDirectory.fileSystemRepresentation,
		"--all",
	};
	
	XCTAssertDefaultMain(SMMainExitInvalidVM);
	
	// Check output.
	XCTAssertEqual(sout, 0);
	XCTAssertContainString(*berr, serr, "Error");
}

- (void)testShowAll
{
	// Generate test vm.
	NSString *vmPath = [self generateVMwareVMWithResultingVMXFilePath:nil resultingNVRAMFilePath:nil];
	
	// Test main.
	const char *argv[] = {
		"ut-main",
		"show",
		vmPath.fileSystemRepresentation,
		"--all",
	};
	
	XCTAssertDefaultMain(SMMainExitSuccess);

	// Check output.
	XCTAssertGreaterThan(sout, 20);
	XCTAssertContainString(*bout, sout, "VMX");
	XCTAssertContainString(*bout, sout, "entries");
	XCTAssertContainString(*bout, sout, ".encoding");
	XCTAssertContainString(*bout, sout, "Type");
	
	XCTAssertContainString(*bout, sout, "NVRAM");
	XCTAssertContainString(*bout, sout, "KEY1");
	XCTAssertContainString(*bout, sout, "SUB1");
	XCTAssertContainString(*bout, sout, "EFI_");
	XCTAssertContainString(*bout, sout, "GUID");
	XCTAssertContainString(*bout, sout, "4CE8598D-D539-8043-B6CB-760EA666552E");
	XCTAssertContainString(*bout, sout, "HELLOWORLD");
	XCTAssertContainString(*bout, sout, "Attributes");
	XCTAssertContainString(*bout, sout, "Name");

	XCTAssertEqual(serr, 0);
}

- (void)testShowVMX
{
	// Generate test vm.
	NSString *vmPath = [self generateVMwareVMWithResultingVMXFilePath:nil resultingNVRAMFilePath:nil];
	
	// Test main.
	const char *argv[] = {
		"ut-main",
		"show",
		vmPath.fileSystemRepresentation,
		"--vmx",
	};
	
	XCTAssertDefaultMain(SMMainExitSuccess);
	
	// Check output.
	XCTAssertContainString(*bout, sout, "VMX");
	XCTAssertContainString(*bout, sout, "entries");
	XCTAssertContainString(*bout, sout, ".encoding");
	
	XCTAssertNotContainString(*bout, sout, "NVRAM");
	XCTAssertNotContainString(*bout, sout, "KEY1");
	XCTAssertNotContainString(*bout, sout, "SUB1");
	XCTAssertNotContainString(*bout, sout, "EFI_");
	XCTAssertNotContainString(*bout, sout, "GUID");
	XCTAssertNotContainString(*bout, sout, "4CE8598D-D539-8043-B6CB-760EA666552E");
	XCTAssertNotContainString(*bout, sout, "HELLOWORLD");
	XCTAssertNotContainString(*bout, sout, "Attributes");
	
	XCTAssertEqual(serr, 0);
}

- (void)testShowNVRAM
{
	// Generate test vm.
	NSString *vmPath = [self generateVMwareVMWithResultingVMXFilePath:nil resultingNVRAMFilePath:nil];
	
	// Test main.
	const char *argv[] = {
		"ut-main",
		"show",
		vmPath.fileSystemRepresentation,
		"--nvram",
	};
	
	XCTAssertDefaultMain(SMMainExitSuccess);
	
	// Check output.
	XCTAssertNotContainString(*bout, sout, "VMX");
	XCTAssertNotContainString(*bout, sout, ".encoding");
	
	XCTAssertContainString(*bout, sout, "NVRAM");
	XCTAssertContainString(*bout, sout, "KEY1");
	XCTAssertContainString(*bout, sout, "SUB1");
	XCTAssertContainString(*bout, sout, "EFI_");
	XCTAssertContainString(*bout, sout, "GUID");
	XCTAssertContainString(*bout, sout, "4CE8598D-D539-8043-B6CB-760EA666552E");
	XCTAssertContainString(*bout, sout, "HELLOWORLD");
	XCTAssertContainString(*bout, sout, "Attributes");
	XCTAssertContainString(*bout, sout, "Name");

	XCTAssertEqual(serr, 0);
}

- (void)testShowEFIVariables
{
	// Generate test vm.
	NSString *vmPath = [self generateVMwareVMWithResultingVMXFilePath:nil resultingNVRAMFilePath:nil];
	
	// Test main.
	const char *argv[] = {
		"ut-main",
		"show",
		vmPath.fileSystemRepresentation,
		"--nvram-efi-variables",
	};
	
	XCTAssertDefaultMain(SMMainExitSuccess);
	
	// Check output.
	XCTAssertNotContainString(*bout, sout, "VMX");
	XCTAssertNotContainString(*bout, sout, ".encoding");
	
	XCTAssertContainString(*bout, sout, "NVRAM");
	XCTAssertNotContainString(*bout, sout, "KEY1");
	XCTAssertNotContainString(*bout, sout, "SUB1");
	XCTAssertNotContainString(*bout, sout, "EFI_");
	XCTAssertContainString(*bout, sout, "4CE8598D-D539-8043-B6CB-760EA666552E");
	XCTAssertContainString(*bout, sout, "HELLOWORLD");
	XCTAssertContainString(*bout, sout, "Attributes");
	XCTAssertContainString(*bout, sout, "Name");
}

- (void)testShowEFIVariable
{
	// Generate test vm.
	NSString *vmPath = [self generateVMwareVMWithResultingVMXFilePath:nil resultingNVRAMFilePath:nil];
	
	// Test main.
	const char *argv[] = {
		"ut-main",
		"show",
		vmPath.fileSystemRepresentation,
		"--nvram-efi-variable",
		"PROP1"
	};
	
	XCTAssertDefaultMain(SMMainExitSuccess);
	
	// Check output.
	XCTAssertNotContainString(*bout, sout, "VMX");
	XCTAssertNotContainString(*bout, sout, ".encoding");
	
	XCTAssertNotContainString(*bout, sout, "NVRAM");
	XCTAssertNotContainString(*bout, sout, "KEY1");
	XCTAssertNotContainString(*bout, sout, "SUB1");
	XCTAssertNotContainString(*bout, sout, "EFI_");
	XCTAssertNotContainString(*bout, sout, "311EEF73-98C3-924C-B094-2A973FB1D3A8");
	XCTAssertNotContainString(*bout, sout, "HELLOWORLD");
	XCTAssertContainString(*bout, sout, "PROP1");
	XCTAssertContainString(*bout, sout, "Attributes");
	XCTAssertContainString(*bout, sout, "Name");
	XCTAssertContainString(*bout, sout, "Value");
}


#pragma mark > Change

- (void)testChangeBootArgs
{
	// Generate test vm.
	NSString *vmPath = [self generateVMwareVMWithResultingVMXFilePath:nil resultingNVRAMFilePath:nil];

	// Test main.
	const char *argv[] = {
		"ut-main",
		"change",
		vmPath.fileSystemRepresentation,
		"--boot-args",
		"hello-world"
	};
	
	XCTAssertDefaultMain(SMMainExitSuccess);

	// Check output.
	XCTAssertEqual(serr, 0);

	// Validate change.
	SMVMXEntryTest vmxEntries[] = {
	};
	
	SMNVRAMEFIVariableTest nvramVariables[] = {
		{ .guid = Apple_NVRAM_Variable_Guid, .name = SMEFIAppleNVRAMVarBootArgsName, .value = { 0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x2D, 0x77, 0x6F, 0x72, 0x6C, 0x64, 0x00 }, .value_size = 12 }
	};
	
	[self validateChangeOnVMAtPath:vmPath vmxEntries:vmxEntries vmxCount:sizeof(vmxEntries) / sizeof(*vmxEntries) nvramVariables:nvramVariables nvramCount:sizeof(nvramVariables) / sizeof(*nvramVariables)];
}

- (void)testChangeCSREnableVersion
{
	// Generate test vm.
	NSString *vmPath = [self generateVMwareVMWithResultingVMXFilePath:nil resultingNVRAMFilePath:nil];

	// Test main.
	const char *argv[] = {
		"ut-main",
		"change",
		vmPath.fileSystemRepresentation,
		"--csr-enable-version",
		"10.15.0"
	};
	
	XCTAssertDefaultMain(SMMainExitSuccess);

	// Check output.
	XCTAssertEqual(serr, 0);
	
	// Validate change.
	SMVMXEntryTest vmxEntries[] = {
	};
	
	SMNVRAMEFIVariableTest nvramVariables[] = {
		{ .guid = Apple_NVRAM_Variable_Guid, .name = SMEFIAppleNVRAMVarCSRActiveConfigName, .value = { 0x10, 0x00, 0x00, 0x00 }, .value_size = 4 }
	};
	
	[self validateChangeOnVMAtPath:vmPath vmxEntries:vmxEntries vmxCount:sizeof(vmxEntries) / sizeof(*vmxEntries) nvramVariables:nvramVariables nvramCount:sizeof(nvramVariables) / sizeof(*nvramVariables)];
}

- (void)testChangeCSRDisable
{
	// Generate test vm.
	NSString *vmPath = [self generateVMwareVMWithResultingVMXFilePath:nil resultingNVRAMFilePath:nil];

	// Test main.
	const char *argv[] = {
		"ut-main",
		"change",
		vmPath.fileSystemRepresentation,
		"--csr-disable-version",
		"10.15.0"
	};
	
	XCTAssertDefaultMain(SMMainExitSuccess);

	// Check output.
	XCTAssertEqual(serr, 0);
	
	// Validate change.
	SMVMXEntryTest vmxEntries[] = {
	};
	
	SMNVRAMEFIVariableTest nvramVariables[] = {
		{ .guid = Apple_NVRAM_Variable_Guid, .name = SMEFIAppleNVRAMVarCSRActiveConfigName, .value = { 0x77, 0x00, 0x00, 0x00 }, .value_size = 4 }
	};
	
	[self validateChangeOnVMAtPath:vmPath vmxEntries:vmxEntries vmxCount:sizeof(vmxEntries) / sizeof(*vmxEntries) nvramVariables:nvramVariables nvramCount:sizeof(nvramVariables) / sizeof(*nvramVariables)];
}

- (void)testChangeCSRFlags
{
	// Generate test vm.
	NSString *vmPath = [self generateVMwareVMWithResultingVMXFilePath:nil resultingNVRAMFilePath:nil];

	// Test main.
	const char *argv[] = {
		"ut-main",
		"change",
		vmPath.fileSystemRepresentation,
		"--csr-flags",
		"0x42"
	};
	
	XCTAssertDefaultMain(SMMainExitSuccess);

	// Check output.
	XCTAssertEqual(serr, 0);
	
	// Validate change.
	SMVMXEntryTest vmxEntries[] = {
	};
	
	SMNVRAMEFIVariableTest nvramVariables[] = {
		{ .guid = Apple_NVRAM_Variable_Guid, .name = SMEFIAppleNVRAMVarCSRActiveConfigName, .value = { 0x42, 0x00, 0x00, 0x00 }, .value_size = 4 }
	};
	
	[self validateChangeOnVMAtPath:vmPath vmxEntries:vmxEntries vmxCount:sizeof(vmxEntries) / sizeof(*vmxEntries) nvramVariables:nvramVariables nvramCount:sizeof(nvramVariables) / sizeof(*nvramVariables)];
}

- (void)testChangeMachineUUID
{
	// Generate test vm.
	NSString *vmPath = [self generateVMwareVMWithResultingVMXFilePath:nil resultingNVRAMFilePath:nil];

	// Test main.
	const char *argv[] = {
		"ut-main",
		"change",
		vmPath.fileSystemRepresentation,
		"--machine-uuid",
		"EBE8D0F9-994A-4E3D-8D3D-C2C85EF23BC9"
	};
	
	XCTAssertDefaultMain(SMMainExitSuccess);

	// Check output.
	XCTAssertEqual(serr, 0);
	
	// Validate change.
	SMVMXEntryTest vmxEntries[] = {
		{ .key = SMVMwareVMXUUIDBiosKey, .value = "eb e8 d0 f9 99 4a 4e 3d-8d 3d c2 c8 5e f2 3b c9" },
		{ .key = SMVMwareVMXUUIDLocationKey, .value = "eb e8 d0 f9 99 4a 4e 3d-8d 3d c2 c8 5e f2 3b c9" },
	};
	
	SMNVRAMEFIVariableTest nvramVariables[] = {
		{ .guid = Apple_NVRAM_Variable_Guid, .name = SMEFIAppleNVRAMVarPlatformUUIDName, .value = { 0xEB, 0xE8, 0xD0, 0xF9, 0x99, 0x4A, 0x4E, 0x3D, 0x8D, 0x3D, 0xC2, 0xC8, 0x5E, 0xF2, 0x3B, 0xC9 }, .value_size = 16 }
	};
	
	[self validateChangeOnVMAtPath:vmPath vmxEntries:vmxEntries vmxCount:sizeof(vmxEntries) / sizeof(*vmxEntries) nvramVariables:nvramVariables nvramCount:sizeof(nvramVariables) / sizeof(*nvramVariables)];
}

- (void)testChangeScreenResolution
{
	// Generate test vm.
	NSString *vmPath = [self generateVMwareVMWithResultingVMXFilePath:nil resultingNVRAMFilePath:nil];

	// Test main.
	const char *argv[] = {
		"ut-main",
		"change",
		vmPath.fileSystemRepresentation,
		"--screen-resolution",
		"400x500"
	};
	
	XCTAssertDefaultMain(SMMainExitSuccess);

	// Check output.
	XCTAssertEqual(serr, 0);
	
	// Validate change.
	SMVMXEntryTest vmxEntries[] = {
	};
	
	SMNVRAMEFIVariableTest nvramVariables[] = {
		{ .guid = Apple_Screen_Resolution_Guid, .name = SMEFIVarAppleScreenResolutionWidthName, .value = { 0x90, 0x01, 0x00, 0x00 }, .value_size = 4 },
		{ .guid = Apple_Screen_Resolution_Guid, .name = SMEFIVarAppleScreenResolutionHeightName, .value = { 0xF4, 0x01, 0x00, 0x00 }, .value_size = 4 }
	};
	
	[self validateChangeOnVMAtPath:vmPath vmxEntries:vmxEntries vmxCount:sizeof(vmxEntries) / sizeof(*vmxEntries) nvramVariables:nvramVariables nvramCount:sizeof(nvramVariables) / sizeof(*nvramVariables)];
}

- (void)testChangeCombined
{
	// Generate test vm.
	NSString *vmPath = [self generateVMwareVMWithResultingVMXFilePath:nil resultingNVRAMFilePath:nil];

	// Test main.
	const char *argv[] = {
		"ut-main",
		"change",
		vmPath.fileSystemRepresentation,
		"--machine-uuid",
		"EBE8D0F9-994A-4E3D-8D3D-C2C85EF23BC9",
		"--boot-args",
		"hello-world"
	};
	
	XCTAssertDefaultMain(SMMainExitSuccess);

	// Check output.
	XCTAssertEqual(serr, 0);
	
	// Validate change.
	SMVMXEntryTest vmxEntries[] = {
		{ .key = SMVMwareVMXUUIDBiosKey, .value = "eb e8 d0 f9 99 4a 4e 3d-8d 3d c2 c8 5e f2 3b c9" },
		{ .key = SMVMwareVMXUUIDLocationKey, .value = "eb e8 d0 f9 99 4a 4e 3d-8d 3d c2 c8 5e f2 3b c9" },
	};
	
	SMNVRAMEFIVariableTest nvramVariables[] = {
		{ .guid = Apple_NVRAM_Variable_Guid, .name = SMEFIAppleNVRAMVarPlatformUUIDName, .value = { 0xEB, 0xE8, 0xD0, 0xF9, 0x99, 0x4A, 0x4E, 0x3D, 0x8D, 0x3D, 0xC2, 0xC8, 0x5E, 0xF2, 0x3B, 0xC9 }, .value_size = 16 },
		{ .guid = Apple_NVRAM_Variable_Guid, .name = SMEFIAppleNVRAMVarBootArgsName, .value = { 0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x2D, 0x77, 0x6F, 0x72, 0x6C, 0x64, 0x00 }, .value_size = 12 }
	};
	
	[self validateChangeOnVMAtPath:vmPath vmxEntries:vmxEntries vmxCount:sizeof(vmxEntries) / sizeof(*vmxEntries) nvramVariables:nvramVariables nvramCount:sizeof(nvramVariables) / sizeof(*nvramVariables)];
}


#pragma mark - Helpers

- (NSString *)generateVMwareVMWithResultingVMXFilePath:(NSString **)vmxFilePath resultingNVRAMFilePath:(NSString **)nvramFilePath
{
	SMVMwareVMX		**pvmx = NULL;
	SMVMwareNVRAM	**pnvram = NULL;
	SMVMwareVMX		*vmx = NULL;
	SMVMwareNVRAM	*nvram = NULL;
	
	if (vmxFilePath)
		pvmx = &vmx;
	
	if (nvramFilePath)
		pnvram = &nvram;
	
	NSString *vmPath = [self generateVMwareVMWithResultingVMX:pvmx resultingNVRAM:pnvram];
	
	if (vmxFilePath)
	{
		*vmxFilePath = @(SMVMwareVMXGetPath(vmx));
		SMVMwareVMXFree(vmx);
	}
	
	if (nvramFilePath)
	{
		*nvramFilePath = @(SMVMwareNVRAMGetPath(nvram));
		SMVMwareNVRAMFree(nvram);
	}
	
	return vmPath;
}

- (NSString *)generateVMwareVMWithResultingVMX:(SMVMwareVMX **)outVMX resultingNVRAM:(SMVMwareNVRAM **)outNVRAM
{
	NSString *vmName = [NSString stringWithFormat:@"%@.vmwarevm", [NSUUID UUID].UUIDString];
	
	// Forge base path.
	NSString	*vmPath = [_testDirectory stringByAppendingPathComponent:vmName];
	NSString	*vmxPath = [vmPath stringByAppendingPathComponent:@"root.vmx"];
	NSString	*nvramPath = [vmPath stringByAppendingPathComponent:@"root.nvram"];
	
	NSAssert([[NSFileManager defaultManager] createDirectoryAtPath:vmPath withIntermediateDirectories:YES attributes:nil error:nil], @"cannot create temp vm bundle");
	
	// Copy base files.
	NSBundle *bundle = [NSBundle bundleForClass:self.class];
	NSString *originVMXPath = [bundle pathForResource:@"empty-1" ofType:@"vmx"];
	NSString *originNVRAMPath = [bundle pathForResource:@"basic-1" ofType:@"nvram"];
	
	NSAssert(originVMXPath, @"cannot find empty VMX file");
	NSAssert(originNVRAMPath, @"cannot find basic NVRAM file");

	NSAssert([[NSFileManager defaultManager] copyItemAtPath:originVMXPath toPath:vmxPath error:nil], @"cannot copy VMX file");
	NSAssert([[NSFileManager defaultManager] copyItemAtPath:originNVRAMPath toPath:nvramPath error:nil], @"cannot copy NVRAM file");

	// Edit VMX.
	SMError		*error;
	SMVMwareVMX	*vmx = SMVMwareVMXOpen(vmxPath.fileSystemRepresentation, &error);

	NSAssert(vmx, @"cannot open VMX file: %s", SMErrorGetSentencizedUserInfo(error));
	NSAssert(SMVMwareVMXAddEntryKeyValue(vmx, SMVMwareVMXNVRAMFileKey, "root.nvram", &error), @"cannot edit VMX file: %s", SMErrorGetSentencizedUserInfo(error));
	
	[[NSFileManager defaultManager] removeItemAtPath:vmxPath error:nil];
	NSAssert(SMVMwareVMXWriteToFile(vmx, vmxPath.fileSystemRepresentation, &error), @"cannot write VMX file: %s",  SMErrorGetSentencizedUserInfo(error));
	
	// Give back items.
	if (outVMX)
		*outVMX = vmx;
	else
		SMVMwareVMXFree(vmx);
	
	if (outNVRAM)
	{
		SMVMwareNVRAM *nvram = SMVMwareNVRAMOpen(nvramPath.fileSystemRepresentation, &error);
		
		NSAssert(nvram, @"cannot open NVRAM file: %s", SMErrorGetSentencizedUserInfo(error));

		*outNVRAM = nvram;
	}
	
	// Return.
	return vmPath;
}

- (void)validateChangeOnVMAtPath:vmPath vmxEntries:(const SMVMXEntryTest *)vmxEntries vmxCount:(size_t)vmxCount nvramVariables:(const SMNVRAMEFIVariableTest *)nvramVariables nvramCount:(size_t)nvramCount
{
	SMError		*error;
	NSString	*vmxPath = [vmPath stringByAppendingPathComponent:@"root.vmx"];
	NSString	*nvramPath = [vmPath stringByAppendingPathComponent:@"root.nvram"];
	
	// Check VMX.
	SMVMwareVMX	*vmx = SMVMwareVMXOpen(vmxPath.fileSystemRepresentation, &error);
	
	XCTAssertNotEqual(vmx, NULL);
	
	for (size_t i = 0; i < vmxCount; i++)
	{
		const SMVMXEntryTest	*test_entry = &vmxEntries[i];
		SMVMwareVMXEntry		*vmx_entry = SMVMwareVMXGetEntryForKey(vmx, test_entry->key);
		
		XCTAssertNotEqual(vmx_entry, NULL);
		XCTAssertEqual(SMVMwareVMXEntryGetType(vmx_entry), SMVMwareVMXEntryTypeKeyValue);
		
		XCTAssertEqualStrings(SMVMwareVMXEntryGetValue(vmx_entry, NULL), test_entry->value);
	}
	
	// Check NVRAM.
	SMVMwareNVRAM *nvram = SMVMwareNVRAMOpen(nvramPath.fileSystemRepresentation, &error);

	XCTAssertNotEqual(nvram, NULL);
	
	for (size_t i = 0; i < nvramCount; i++)
	{
		const SMNVRAMEFIVariableTest	*test_variable = &nvramVariables[i];
		SMVMwareNVRAMEFIVariable		*nvram_variable = SMVMwareNVRAMVariableForGUIDAndName(nvram, &test_variable->guid, test_variable->name, NULL);
		
		XCTAssertNotEqual(nvram_variable, NULL);
		
		size_t		value_size = SIZE_T_MAX;
		const char	*value = SMVMwareNVRAMVariableGetValue(nvram_variable, &value_size);
		
		XCTAssertEqual(value_size, test_variable->value_size);
		XCTAssertEqual(memcmp(value, test_variable->value, value_size), 0);
	}
}

@end
