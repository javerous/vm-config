/*
 *  SMVMwareNVRAMTests.m
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

#import "SMVMwareNVRAM.h"
#import "SMVMwareNVRAMHelper.h"

#import "SMTestsTools.h"


/*
** Types
*/
#pragma mark - Types

typedef NS_ENUM(NSUInteger, SMModificationPhase)
{
	SMModificationPhaseOriginal,
	SMModificationPhaseReopen1,
	SMModificationPhaseReopen2
};


/*
** SMVMwareNVRAMTests
*/
#pragma mark - SMVMwareNVRAMTests

@interface SMVMwareNVRAMTests : XCTestCase

@end

@implementation SMVMwareNVRAMTests

#pragma mark - XCTestCase

- (void)setUp
{
	self.continueAfterFailure = NO;
}


#pragma mark - Tests

- (void)testBasic1Parsing
{
	// Parse file.
	SMError			*error = NULL;
	SMVMwareNVRAM	*nvram = [self nvramForFile:@"basic-1" error:&error];
	
	XCTAssert(nvram, @"failed to parse file: %s", SMErrorGetUserInfo(error));
	
	_onExit {
		SMVMwareNVRAMFree(nvram);
	};
	
	// Check content.
	XCTAssertEqual(SMVMwareNVRAMEntriesCount(nvram), 3);
	
	// > Entry 1.
	SMVMwareNVRAMEntry	*entry1 = SMVMwareNVRAMGetEntryAtIndex(nvram, 0);
	size_t 				size1 = SIZE_MAX;
	
	XCTAssertEqualStrings(SMVMwareNVRAMEntryGetName(entry1), "KEY1");
	XCTAssertEqualStrings(SMVMwareNVRAMEntryGetSubname(entry1), "SUB1");
	XCTAssertEqual(SMVMwareNVRAMEntryGetType(entry1), SMVMwareNVRAMEntryTypeGeneric);
	XCTAssertNotEqual(SMVMwareNVRAMEntryGetContentBytes(entry1, &size1), NULL);
	XCTAssertEqual(size1, 0);
	
	// > Entry 2.
	SMVMwareNVRAMEntry	*entry2 = SMVMwareNVRAMGetEntryAtIndex(nvram, 1);
	uint8_t				bytes2_ref[] = { 0xCA, 0xFE, 0xBA, 0xBE };
	const void			*bytes2 = NULL;
	size_t 				size2 = SIZE_MAX;

	XCTAssertEqualStrings(SMVMwareNVRAMEntryGetName(entry2), "KEY2");
	XCTAssertEqualStrings(SMVMwareNVRAMEntryGetSubname(entry2), "SUB2");
	XCTAssertEqual(SMVMwareNVRAMEntryGetType(entry2), SMVMwareNVRAMEntryTypeGeneric);
	
	bytes2 = SMVMwareNVRAMEntryGetContentBytes(entry2, &size2);
	
	XCTAssertEqual(size2, sizeof(bytes2_ref));
	XCTAssertEqual(memcmp(bytes2, bytes2_ref, size2), 0);

	// > Entry 3.
	SMVMwareNVRAMEntry *entry3 = SMVMwareNVRAMGetEntryAtIndex(nvram, 2);
	
	XCTAssertEqualStrings(SMVMwareNVRAMEntryGetName(entry3), "EFI_");
	XCTAssertEqualStrings(SMVMwareNVRAMEntryGetSubname(entry3), "NV");
	XCTAssertEqual(SMVMwareNVRAMEntryGetType(entry3), SMVMwareNVRAMEntryTypeEFIVariables);
	
	XCTAssertEqual(SMVMwareNVRAMEntryVariablesCount(entry3), 2);
	
	// >> Variable 1
	SMVMwareNVRAMEFIVariable	*var1 = SMVMwareNVRAMEntryGetVariableAtIndex(entry3, 0);
	efi_guid_t					guid1_ref = { 0x4CE8598D, 0xD539, 0x8043, { 0xB6, 0xCB, 0x76, 0x0E, 0xA6, 0x66, 0x55, 0x2E } };
	uint8_t						name1_ref[] = { 0x50, 0x00, 0x52, 0x00, 0x4F, 0x00, 0x50, 0x00, 0x31, 0x00 };
	uint8_t						value1_ref[] = { 0x56, 0x00, 0x41, 0x00, 0x4C, 0x00, 0x55, 0x00, 0x45, 0x00, 0x31, 0x00 };
	
	efi_guid_t guid1 = SMVMwareNVRAMVariableGetGUID(var1);
	
	size_t		name1_size = SIZE_MAX;
	const void	*name1 = SMVMwareNVRAMVariableGetName(var1, &name1_size);
	
	size_t		value1_size = SIZE_MAX;
	const void	*value1 = SMVMwareNVRAMVariableGetValue(var1, &value1_size);
	
	XCTAssertEqual(memcmp(&guid1, &guid1_ref, sizeof(efi_guid_t)), 0);
	XCTAssertEqual(SMVMwareNVRAMVariableGetAttributes(var1), 0x42);
	
	XCTAssertEqual(name1_size, sizeof(name1_ref));
	XCTAssertEqual(memcmp(name1, name1_ref, sizeof(name1_ref)), 0);
	
	XCTAssertEqual(value1_size, sizeof(value1_ref));
	XCTAssertEqual(memcmp(value1, value1_ref, sizeof(value1_ref)), 0);
	
	XCTAssertEqualStrings(SMVMwareNVRAMVariableGetUTF8Name(var1, NULL), "PROP1");

	
	// >> Variable 2
	SMVMwareNVRAMEFIVariable	*var2 = SMVMwareNVRAMEntryGetVariableAtIndex(entry3, 1);
	efi_guid_t					guid2_ref = { 0x311EEF73, 0x98C3, 0x924C, { 0xB0, 0x94, 0x2A, 0x97, 0x3F, 0xB1, 0xD3, 0xA8 } };
	uint8_t						name2_ref[] = { 0x48, 0x00, 0x45, 0x00, 0x4C, 0x00, 0x4C, 0x00, 0x4F, 0x00, 0x57, 0x00, 0x4F, 0x00, 0x52, 0x00, 0x4C, 0x00, 0x44, 0x00 };
	uint8_t						value2_ref[] = { 0x57, 0x00, 0x45, 0x00, 0x4C, 0x00, 0x43, 0x00, 0x4F, 0x00, 0x4D, 0x00, 0x45, 0x00 };
	
	efi_guid_t guid2 = SMVMwareNVRAMVariableGetGUID(var2);
	
	size_t		name2_size = SIZE_MAX;
	const void	*name2 = SMVMwareNVRAMVariableGetName(var2, &name2_size);
	
	size_t		value2_size = SIZE_MAX;
	const void	*value2 = SMVMwareNVRAMVariableGetValue(var2, &value2_size);
	
	XCTAssertEqual(memcmp(&guid2, &guid2_ref, sizeof(efi_guid_t)), 0);
	XCTAssertEqual(SMVMwareNVRAMVariableGetAttributes(var2), 0x51);
	
	XCTAssertEqual(name2_size, sizeof(name2_ref));
	XCTAssertEqual(memcmp(name2, name2_ref, sizeof(name2_ref)), 0);
	
	XCTAssertEqual(value2_size, sizeof(value2_ref));
	XCTAssertEqual(memcmp(value2, value2_ref, sizeof(value2_ref)), 0);
	
	XCTAssertEqualStrings(SMVMwareNVRAMVariableGetUTF8Name(var2, NULL), "HELLOWORLD");
}

- (void)testFail1Parsing
{
	// Parse file.
	SMError			*error = NULL;
	SMVMwareNVRAM	*nvram = [self nvramForFile:@"fail-1" error:&error];
	
	XCTAssertEqual(nvram, NULL, "succeeded in parsing an invalid nvram");
	
	NSLog(@"Result error: '%s'.\n", SMErrorGetUserInfo(error));
	
	_onExit {
		SMVMwareNVRAMFree(nvram);
	};
}

- (void)testFail2Parsing
{
	// Parse file.
	SMError			*error = NULL;
	SMVMwareNVRAM	*nvram = [self nvramForFile:@"fail-2" error:&error];

	XCTAssertEqual(nvram, NULL, "succeeded in parsing an invalid nvram");

	NSLog(@"Result error: '%s'.\n", SMErrorGetUserInfo(error));

	_onExit {
		SMVMwareNVRAMFree(nvram);
	};
}

- (void)testFail3Parsing
{
	// Parse file.
	SMError			*error = NULL;
	SMVMwareNVRAM	*nvram = [self nvramForFile:@"fail-3" error:&error];

	XCTAssertEqual(nvram, NULL, "succeeded in parsing an invalid nvram");

	NSLog(@"Result error: '%s'.\n", SMErrorGetUserInfo(error));

	_onExit {
		SMVMwareNVRAMFree(nvram);
	};
}

- (void)testFail4Parsing
{
	// Parse file.
	SMError			*error = NULL;
	SMVMwareNVRAM	*nvram = [self nvramForFile:@"fail-4" error:&error];

	XCTAssertEqual(nvram, NULL, "succeeded in parsing an invalid nvram");

	NSLog(@"Result error: '%s'.\n", SMErrorGetUserInfo(error));

	_onExit {
		SMVMwareNVRAMFree(nvram);
	};
}

- (void)testParserStability
{
	// Parse file.
	SMError			*error = NULL;
	SMVMwareNVRAM	*nvram = [self nvramForFile:@"basic-1" error:&error];

	XCTAssert(nvram, @"failed to parse file: %s", SMErrorGetUserInfo(error));
	
	_onExit {
		SMVMwareNVRAMFree(nvram);
	};
	
	// Write parsed file.
	NSString *tempOutput = SMGenerateTemporaryTestPath();
	
	XCTAssertTrue(SMVMwareNVRAMWriteToFile(nvram, tempOutput.fileSystemRepresentation, &error), @"failed to write file: %s", SMErrorGetUserInfo(error));
	
	// Compare.
	const char	*cResourcePath = SMVMwareNVRAMGetPath(nvram);
	NSString	*resourcePath = [[NSFileManager defaultManager] stringWithFileSystemRepresentation:cResourcePath length:strlen(cResourcePath)];
		
	NSData *refData = [NSData dataWithContentsOfFile:resourcePath];
	NSData *writtenData = [NSData dataWithContentsOfFile:tempOutput];
	
	XCTAssertNotNil(refData);
	XCTAssertNotNil(writtenData);
	
	XCTAssertEqualObjects(refData, writtenData);
}

- (void)testModificationsBootArgs
{
	[self handleTestModificationOfNVRAMFile:@"basic-1" phaseBlock:^(SMModificationPhase phase, SMVMwareNVRAM *nvram) {
		
		uint8_t ref1[] = { 0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x3D, 0x77, 0x6F, 0x72, 0x6C, 0x64, 0x00 };
		uint8_t ref2[] = { 0x6B, 0x65, 0x79, 0x3D, 0x76, 0x61, 0x6C, 0x75, 0x65, 0x00};

		switch (phase)
		{
			case SMModificationPhaseOriginal:
			{
				XCTAssertTrue(SMVMwareNVRAMSetBootArgs(nvram, "hello=world", NULL));
				
				[self validateEFIVariableOfNVRAM:nvram guid:Apple_NVRAM_Variable_Guid name:SMEFIAppleNVRAMVarBootArgsName value:ref1 size:sizeof(ref1)];

				break;
			}
				
			case SMModificationPhaseReopen1:
			{
				[self validateEFIVariableOfNVRAM:nvram guid:Apple_NVRAM_Variable_Guid name:SMEFIAppleNVRAMVarBootArgsName value:ref1 size:sizeof(ref1)];
				
				XCTAssertTrue(SMVMwareNVRAMSetBootArgs(nvram, "key=value", NULL));
				
				[self validateEFIVariableOfNVRAM:nvram guid:Apple_NVRAM_Variable_Guid name:SMEFIAppleNVRAMVarBootArgsName value:ref2 size:sizeof(ref2)];

				break;
			}
				
			case SMModificationPhaseReopen2:
			{
				[self validateEFIVariableOfNVRAM:nvram guid:Apple_NVRAM_Variable_Guid name:SMEFIAppleNVRAMVarBootArgsName value:ref2 size:sizeof(ref2)];

				break;
			}
		}
	}];
}

- (void)testModificationsCSRRaw
{
	[self handleTestModificationOfNVRAMFile:@"basic-1" phaseBlock:^(SMModificationPhase phase, SMVMwareNVRAM *nvram) {

		switch (phase)
		{
			case SMModificationPhaseOriginal:
			{
				uint32_t csr = 0;
				
				XCTAssertFalse(SMVMwareNVRAMGetAppleCSRActiveConfig(nvram, &csr, NULL));
				XCTAssertTrue(SMVMwareNVRAMSetAppleCSRActiveConfig(nvram, 0x42, NULL));
				
				XCTAssertTrue(SMVMwareNVRAMGetAppleCSRActiveConfig(nvram, &csr, NULL));
				XCTAssertEqual(csr, 0x42);

				break;
			}
				
			case SMModificationPhaseReopen1:
			{
				uint32_t csr = 0;
				
				XCTAssertTrue(SMVMwareNVRAMGetAppleCSRActiveConfig(nvram, &csr, NULL));
				XCTAssertEqual(csr, 0x42);

				XCTAssertTrue(SMVMwareNVRAMSetAppleCSRActiveConfig(nvram, 0x51, NULL));
				
				XCTAssertTrue(SMVMwareNVRAMGetAppleCSRActiveConfig(nvram, &csr, NULL));
				XCTAssertEqual(csr, 0x51);

				break;
			}
				
			case SMModificationPhaseReopen2:
			{
				uint32_t csr = 0;
				
				XCTAssertTrue(SMVMwareNVRAMGetAppleCSRActiveConfig(nvram, &csr, NULL));
				XCTAssertEqual(csr, 0x51);

				break;
			}
		}
	}];
}

- (void)testModificationsCSRActivation
{
	[self handleTestModificationOfNVRAMFile:@"basic-1" phaseBlock:^(SMModificationPhase phase, SMVMwareNVRAM *nvram) {

		switch (phase)
		{
			case SMModificationPhaseOriginal:
			{
				uint32_t	csr = 0;
				SMVersion	version = SMVersionFromComponents(13, 0, 0);
				
				XCTAssertFalse(SMVMwareNVRAMGetAppleCSRActiveConfig(nvram, &csr, NULL));
				XCTAssertTrue(SMVMwareNVRAMSetAppleCSRActivation(nvram, version, true, NULL));
				
				XCTAssertTrue(SMVMwareNVRAMGetAppleCSRActiveConfig(nvram, &csr, NULL));
				XCTAssertNotEqual(csr, 0);

				return;
			}
				
			case SMModificationPhaseReopen1:
			{
				uint32_t	csr = 0;
				SMVersion	version = SMVersionFromComponents(12, 0, 0);
				
				XCTAssertTrue(SMVMwareNVRAMGetAppleCSRActiveConfig(nvram, &csr, NULL));
				XCTAssertNotEqual(csr, 0);
				
				XCTAssertTrue(SMVMwareNVRAMSetAppleCSRActivation(nvram, version, false, NULL));
				
				XCTAssertTrue(SMVMwareNVRAMGetAppleCSRActiveConfig(nvram, &csr, NULL));
				XCTAssertNotEqual(csr, 0);
				
				break;
			}
				
			case SMModificationPhaseReopen2:
			{
				uint32_t csr = 0;

				XCTAssertTrue(SMVMwareNVRAMGetAppleCSRActiveConfig(nvram, &csr, NULL));
				XCTAssertNotEqual(csr, 0);

				break;
			}
		}
	}];
}

- (void)testModificationsPlatformUUID
{
	[self handleTestModificationOfNVRAMFile:@"basic-1" phaseBlock:^(SMModificationPhase phase, SMVMwareNVRAM *nvram) {
		
		uuid_t uuid1 = { 0xC8, 0x62, 0xD7, 0x75, 0x62, 0x3D, 0x42, 0x99, 0x82, 0x77, 0x96, 0xA6, 0x4A, 0x69, 0x6F, 0xD5 };
		uuid_t uuid2 = { 0x84, 0xDE, 0x67, 0x52, 0xFB, 0xAA, 0x4C, 0x61, 0x9C, 0x05, 0xDF, 0x09, 0x92, 0x4F, 0x58, 0x36 };

		switch (phase)
		{
			case SMModificationPhaseOriginal:
			{
				uuid_t uuid = { 0 };
				
				XCTAssertFalse(SMVMwareNVRAMGetApplePlatformUUID(nvram, uuid, NULL));
				XCTAssertTrue(SMVMwareNVRAMSetApplePlatformUUID(nvram, uuid1, NULL));
				
				XCTAssertTrue(SMVMwareNVRAMGetApplePlatformUUID(nvram, uuid, NULL));
				
				XCTAssertEqual(memcmp(uuid, uuid1, sizeof(uuid)), 0);

				break;
			}
				
			case SMModificationPhaseReopen1:
			{
				uuid_t uuid = { 0 };

				XCTAssertTrue(SMVMwareNVRAMGetApplePlatformUUID(nvram, uuid, NULL));
				XCTAssertEqual(memcmp(uuid, uuid1, sizeof(uuid1)), 0);
				
				XCTAssertTrue(SMVMwareNVRAMSetApplePlatformUUID(nvram, uuid2, NULL));
				XCTAssertTrue(SMVMwareNVRAMGetApplePlatformUUID(nvram, uuid, NULL));
				
				XCTAssertEqual(memcmp(uuid, uuid2, sizeof(uuid)), 0);
								
				break;
			}
				
			case SMModificationPhaseReopen2:
			{
				uuid_t uuid = { 0 };

				XCTAssertTrue(SMVMwareNVRAMGetApplePlatformUUID(nvram, uuid, NULL));
				XCTAssertEqual(memcmp(uuid, uuid2, sizeof(uuid2)), 0);

				break;
			}
		}
	}];
}

- (void)testModificationsScreenResolution
{
	[self handleTestModificationOfNVRAMFile:@"basic-1" phaseBlock:^(SMModificationPhase phase, SMVMwareNVRAM *nvram) {
		
		uint32_t width1 = 50, height1 = 40;
		uint32_t width2 = 1024, height2 = 860;

		switch (phase)
		{
			case SMModificationPhaseOriginal:
			{
				XCTAssertTrue(SMVMwareNVRAMSetScreenResolution(nvram, width1, height1, NULL));
				
				[self validateEFIVariableOfNVRAM:nvram guid:Apple_Screen_Resolution_Guid name:SMEFIVarAppleScreenResolutionWidthName value:&width1 size:sizeof(width1)];
				[self validateEFIVariableOfNVRAM:nvram guid:Apple_Screen_Resolution_Guid name:SMEFIVarAppleScreenResolutionHeightName value:&height1 size:sizeof(height1)];

				break;
			}
				
			case SMModificationPhaseReopen1:
			{
				[self validateEFIVariableOfNVRAM:nvram guid:Apple_Screen_Resolution_Guid name:SMEFIVarAppleScreenResolutionWidthName value:&width1 size:sizeof(width1)];
				[self validateEFIVariableOfNVRAM:nvram guid:Apple_Screen_Resolution_Guid name:SMEFIVarAppleScreenResolutionHeightName value:&height1 size:sizeof(height1)];
				
				XCTAssertTrue(SMVMwareNVRAMSetScreenResolution(nvram, width2, height2, NULL));

				[self validateEFIVariableOfNVRAM:nvram guid:Apple_Screen_Resolution_Guid name:SMEFIVarAppleScreenResolutionWidthName value:&width2 size:sizeof(width2)];
				[self validateEFIVariableOfNVRAM:nvram guid:Apple_Screen_Resolution_Guid name:SMEFIVarAppleScreenResolutionHeightName value:&height2 size:sizeof(height2)];
				
				break;
			}
				
			case SMModificationPhaseReopen2:
			{
				[self validateEFIVariableOfNVRAM:nvram guid:Apple_Screen_Resolution_Guid name:SMEFIVarAppleScreenResolutionWidthName value:&width2 size:sizeof(width2)];
				[self validateEFIVariableOfNVRAM:nvram guid:Apple_Screen_Resolution_Guid name:SMEFIVarAppleScreenResolutionHeightName value:&height2 size:sizeof(height2)];
				
				break;
			}
		}
	}];
}


#pragma mark - Helpers

- (SMVMwareNVRAM *)nvramForFile:(NSString *)file error:(SMError **)error
{
	NSBundle *bundle = [NSBundle bundleForClass:self.class];
	NSString *path = [bundle pathForResource:file ofType:@"nvram"];
	
	NSAssert(path, @"cannot find NVRAM file '%@'", file);

	return SMVMwareNVRAMOpen(path.fileSystemRepresentation, error);
}

- (void)validateEFIVariableOfNVRAM:(SMVMwareNVRAM *)nvram guid:(efi_guid_t)guid name:(const char *)name value:(const void *)value size:(size_t)size
{
	// Fetch variable.
	SMVMwareNVRAMEFIVariable *var = SMVMwareNVRAMVariableForGUIDAndName(nvram, &guid, name, NULL);
	
	XCTAssertNotEqual(var, NULL);
	
	// Check content.
	size_t			fsize = 0;
	const uint8_t	*fvalue = SMVMwareNVRAMVariableGetValue(var, &fsize);
	
	XCTAssertEqualStrings(name, SMVMwareNVRAMVariableGetUTF8Name(var, NULL));
	
	XCTAssertEqual(fsize, size);
	XCTAssertEqual(memcmp(fvalue, value, size), 0);
}

- (void)handleTestModificationOfNVRAMFile:(NSString *)file phaseBlock:(void (^)(SMModificationPhase phase, SMVMwareNVRAM *nvram))block
{
	// Parse file.
	SMError			*error = NULL;
	SMVMwareNVRAM	*nvramOriginal = [self nvramForFile:file error:&error];
	
	XCTAssert(nvramOriginal, @"failed to parse file: %s", SMErrorGetUserInfo(error));
	
	_onExit {
		SMVMwareNVRAMFree(nvramOriginal);
	};
	
	// Modify.
	block(SMModificationPhaseOriginal, nvramOriginal);
	
	// Write modified file 1.
	NSString *modifiedFile1 = SMGenerateTemporaryTestPath();
	
	XCTAssertTrue(SMVMwareNVRAMWriteToFile(nvramOriginal, modifiedFile1.fileSystemRepresentation, &error), "failed to write file 1: %s", SMErrorGetUserInfo(error));
	
	// Open modified file1.
	SMVMwareNVRAM *nvramReopen1 = SMVMwareNVRAMOpen(modifiedFile1.fileSystemRepresentation, &error);
	
	XCTAssertNotEqual(nvramReopen1, NULL, "failed to re-open modified file 1: %s", SMErrorGetUserInfo(error));
	
	_onExit {
		SMVMwareNVRAMFree(nvramReopen1);
		[[NSFileManager defaultManager] removeItemAtPath:modifiedFile1 error:nil];
	};
	
	// Validate modified.
	block(SMModificationPhaseReopen1, nvramReopen1);
	
	// Write modified file 2.
	NSString *modifiedFile2 = SMGenerateTemporaryTestPath();
	
	XCTAssertTrue(SMVMwareNVRAMWriteToFile(nvramReopen1, modifiedFile2.fileSystemRepresentation, &error), "failed to write file 2: %s", SMErrorGetUserInfo(error));
	
	// Open modified file2.
	SMVMwareNVRAM *nvramReopen2 = SMVMwareNVRAMOpen(modifiedFile2.fileSystemRepresentation, &error);
	
	XCTAssertNotEqual(nvramReopen2, NULL, "failed to re-open modified file 2: %s", SMErrorGetUserInfo(error));
	
	_onExit {
		SMVMwareNVRAMFree(nvramReopen2);
		[[NSFileManager defaultManager] removeItemAtPath:modifiedFile2 error:nil];
	};
	
	
	block(SMModificationPhaseReopen2, nvramReopen2);
}

@end
