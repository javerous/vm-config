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
#import "SMVMwareNVRAM.h"

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
	
	NSLog(@"-> '%@'", _testDirectory);
}

- (void)tearDown
{
	[[NSFileManager defaultManager] removeItemAtPath:_testDirectory error:nil];
}

#pragma mark - Tests

- (void)testNotArguments
{
	const char *argv[] = {
		"ut-main"
	};
		
	XCTAssertDefaultMain(SMMainExitInvalidArgs);
	
	XCTAssertEqual(sout, 0);
	XCTAssertGreaterThan(serr, 20);
}

- (void)testVersion
{
	const char *argv[] = {
		"ut-main",
		"version"
	};
	
	XCTAssertDefaultMain(SMMainExitSuccess);
	
	XCTAssertContainString(*bout, sout, "version");
	XCTAssertEqual(serr, 0);
}

- (void)testInvalidVerb
{
	const char *argv[] = {
		"ut-main",
		"invalid-verb"
	};
	
	XCTAssertDefaultMain(SMMainExitInvalidArgs);
	
	XCTAssertEqual(sout, 0);
	XCTAssertContainString(*berr, serr, "Error");
}


#pragma mark > Show

- (void)testShowError
{
	const char *argv[] = {
		"ut-main",
		"show",
	};
	
	XCTAssertDefaultMain(SMMainExitInvalidArgs);
	
	XCTAssertEqual(sout, 0);
	XCTAssertContainString(*berr, serr, "Error");
}

- (void)testShowExtraArgs
{
	const char *argv[] = {
		"ut-main",
		"show",
		_testDirectory.fileSystemRepresentation,
		"--all",
		"blah",
	};
	
	XCTAssertDefaultMain(SMMainExitInvalidArgs);
	
	XCTAssertEqual(sout, 0);
	XCTAssertContainString(*berr, serr, "Error");
}

- (void)testShowInvalidArgs
{
	const char *argv[] = {
		"ut-main",
		"show",
		_testDirectory.fileSystemRepresentation,
		"--unknow-arg",
	};
	
	XCTAssertDefaultMain(SMMainExitInvalidArgs);
	
	XCTAssertEqual(sout, 0);
	XCTAssertContainString(*berr, serr, "Error");
}

- (void)testShowInvalidVM
{
	const char *argv[] = {
		"ut-main",
		"show",
		_testDirectory.fileSystemRepresentation,
		"--all",
	};
	
	XCTAssertDefaultMain(SMMainExitInvalidVM);
	
	XCTAssertEqual(sout, 0);
	XCTAssertContainString(*berr, serr, "Error");
}

- (void)testShowAll
{
	NSString *vmPath = [self generateVMwareVMWithResultingVMXFilePath:nil resultingNVRAMFilePath:nil];
	
	const char *argv[] = {
		"ut-main",
		"show",
		vmPath.fileSystemRepresentation,
		"--all",
	};
	
	XCTAssertDefaultMain(SMMainExitSuccess);

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
	NSString *vmPath = [self generateVMwareVMWithResultingVMXFilePath:nil resultingNVRAMFilePath:nil];
	
	const char *argv[] = {
		"ut-main",
		"show",
		vmPath.fileSystemRepresentation,
		"--vmx",
	};
	
	XCTAssertDefaultMain(SMMainExitSuccess);
	
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
	NSString *vmPath = [self generateVMwareVMWithResultingVMXFilePath:nil resultingNVRAMFilePath:nil];
	
	const char *argv[] = {
		"ut-main",
		"show",
		vmPath.fileSystemRepresentation,
		"--nvram",
	};
	
	XCTAssertDefaultMain(SMMainExitSuccess);
	
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
	NSString *vmPath = [self generateVMwareVMWithResultingVMXFilePath:nil resultingNVRAMFilePath:nil];
	
	const char *argv[] = {
		"ut-main",
		"show",
		vmPath.fileSystemRepresentation,
		"--nvram-efi-variables",
	};
	
	XCTAssertDefaultMain(SMMainExitSuccess);
	
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
	NSString *vmPath = [self generateVMwareVMWithResultingVMXFilePath:nil resultingNVRAMFilePath:nil];
	
	const char *argv[] = {
		"ut-main",
		"show",
		vmPath.fileSystemRepresentation,
		"--nvram-efi-variable",
		"PROP1"
	};
	
	XCTAssertDefaultMain(SMMainExitSuccess);
	
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

// FIXME: add  tests.

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

@end
