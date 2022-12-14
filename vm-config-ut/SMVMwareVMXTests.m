/*
 *  SMVMwareVMXTests.m
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

#import "SMVMwareVMX.h"
#import "SMVMwareVMXHelper.h"

#import "SMTestsTools.h"
#import "SMTestCase.h"


/*
** Types
*/
#pragma mark - Types

typedef struct
{
	SMVMwareVMXEntryType type;
	const char *key;
	const char *value;
} SMVMXEntryTest;


/*
** SMVMwareVMXTests
*/
#pragma mark - SMVMwareVMXTests

@interface SMVMwareVMXTests : SMTestCase

@end

@implementation SMVMwareVMXTests

#pragma mark - XCTestCase

- (void)setUp
{
	[super setUp];
	
	self.continueAfterFailure = NO;
}


#pragma mark - Tests

- (void)testBasic1Parsing
{
	// Parse file.
	SMError		*error = NULL;
	SMVMwareVMX *vmx = [self vmxForFile:@"basic-1" error:&error];
	
	XCTAssert(vmx, @"failed to parse file: %s", SMErrorGetUserInfo(error));
	
	_onExit {
		SMVMwareVMXFree(vmx);
	};
	
	// Validate content.
	SMVMXEntryTest testEntries[] = {
		{ .type = SMVMwareVMXEntryTypeKeyValue, .key = ".encoding", .value = "UTF-8" },
		{ .type = SMVMwareVMXEntryTypeKeyValue, .key = "displayName", .value = "macOS 10.15" },
		{ .type = SMVMwareVMXEntryTypeKeyValue, .key = "config.version", .value = "8" },
		{ .type = SMVMwareVMXEntryTypeKeyValue, .key = "pciBridge0.present", .value = "TRUE" },
		{ .type = SMVMwareVMXEntryTypeEmpty },
		{ .type = SMVMwareVMXEntryTypeEmpty },
		{ .type = SMVMwareVMXEntryTypeKeyValue, .key = "guestOS", .value = "darwin19-64" },
		{ .type = SMVMwareVMXEntryTypeEmpty },
		{ .type = SMVMwareVMXEntryTypeComment, .value = "A comment" },
		{ .type = SMVMwareVMXEntryTypeKeyValue, .key = "firmware", .value = "efi" },
	};
	
	[self validateEntriesOfVMX:vmx withTestEntries:testEntries count:sizeof(testEntries) / sizeof(*testEntries)];
}

- (void)testChaotic1Parsing
{
	// Parse file.
	SMError		*error = NULL;
	SMVMwareVMX *vmx = [self vmxForFile:@"chaotic-1" error:&error];
	
	XCTAssert(vmx, @"failed to parse file: %s", SMErrorGetUserInfo(error));
	
	_onExit {
		SMVMwareVMXFree(vmx);
	};
	
	// Validate content.
	SMVMXEntryTest testEntries[] = {
		{ .type = SMVMwareVMXEntryTypeKeyValue, .key = "key1", .value = "value1" },
		{ .type = SMVMwareVMXEntryTypeKeyValue, .key = "key2", .value = "value2" },
		{ .type = SMVMwareVMXEntryTypeKeyValue, .key = "key3", .value = "value3" },
		{ .type = SMVMwareVMXEntryTypeKeyValue, .key = "key4", .value = "   value4   " },
		{ .type = SMVMwareVMXEntryTypeKeyValue, .key = "key5", .value = "   value5   " },
		{ .type = SMVMwareVMXEntryTypeEmpty },
		{ .type = SMVMwareVMXEntryTypeKeyValue, .key = "key6", .value = "value\"hello" },
		{ .type = SMVMwareVMXEntryTypeKeyValue, .key = "key6", .value = "\"valuehello" },
		{ .type = SMVMwareVMXEntryTypeKeyValue, .key = "key6", .value = "valuehello\"" },
		{ .type = SMVMwareVMXEntryTypeEmpty },
		{ .type = SMVMwareVMXEntryTypeKeyValue, .key = "key7", .value = "content with space" },
		{ .type = SMVMwareVMXEntryTypeKeyValue, .key = "key8", .value = " content with space" },
		{ .type = SMVMwareVMXEntryTypeKeyValue, .key = "key9", .value = "content with space " },
		{ .type = SMVMwareVMXEntryTypeKeyValue, .key = "key10", .value = " content with space " },
		{ .type = SMVMwareVMXEntryTypeEmpty },
		{ .type = SMVMwareVMXEntryTypeKeyValue, .key = "guestOS.detailed.data", .value = "bitness='64' buildNumber='19H512' distroName='Mac OS X' distroVersion='10.15.7' familyName='Darwin' kernelVersion='19.6.0'" },
		{ .type = SMVMwareVMXEntryTypeEmpty },
		{ .type = SMVMwareVMXEntryTypeKeyValue, .key = "empty1", .value = "" },
		{ .type = SMVMwareVMXEntryTypeKeyValue, .key = "empty2", .value = "" },
		{ .type = SMVMwareVMXEntryTypeKeyValue, .key = "empty3", .value = "" },
		{ .type = SMVMwareVMXEntryTypeEmpty },
		{ .type = SMVMwareVMXEntryTypeComment, .value = "A comment" },
		{ .type = SMVMwareVMXEntryTypeComment, .value = "" },
		{ .type = SMVMwareVMXEntryTypeComment, .value = "" },
		{ .type = SMVMwareVMXEntryTypeComment, .value = "Another comment" },
		{ .type = SMVMwareVMXEntryTypeComment, .value = "Stick comment" },
	};
	
	[self validateEntriesOfVMX:vmx withTestEntries:testEntries count:sizeof(testEntries) / sizeof(*testEntries)];
}

- (void)testFail1Parsing
{
	// Parse file.
	SMError		*error = NULL;
	SMVMwareVMX *vmx = [self vmxForFile:@"fail-1" error:&error];
	
	XCTAssertEqual(vmx, NULL, "succeeded in parsing an invalid vmx (%s)", SMErrorGetUserInfo(error));
		
	// Clean.
	SMVMwareVMXFree(vmx);
	SMErrorFree(error);
}

- (void)testFail2Parsing
{
	// Parse file.
	SMError		*error = NULL;
	SMVMwareVMX *vmx = [self vmxForFile:@"fail-2" error:&error];
	
	XCTAssertEqual(vmx, NULL, "succeeded in parsing an invalid vmx (%s)", SMErrorGetUserInfo(error));
		
	// Clean.
	SMVMwareVMXFree(vmx);
	SMErrorFree(error);
}

- (void)testFail3Parsing
{
	// Parse file.
	SMError		*error = NULL;
	SMVMwareVMX *vmx = [self vmxForFile:@"fail-3" error:&error];
	
	XCTAssertEqual(vmx, NULL, "succeeded in parsing an invalid vmx (%s)", SMErrorGetUserInfo(error));
		
	// Clean.
	SMVMwareVMXFree(vmx);
	SMErrorFree(error);
}

- (void)testFail4Parsing
{
	// Parse file.
	SMError		*error = NULL;
	SMVMwareVMX *vmx = [self vmxForFile:@"fail-4" error:&error];
	
	XCTAssertEqual(vmx, NULL, "succeeded in parsing an invalid vmx (%s)", SMErrorGetUserInfo(error));
		
	// Clean.
	SMVMwareVMXFree(vmx);
	SMErrorFree(error);
}

- (void)testParserStability
{
	// Parse file.
	SMError		*error = NULL;
	SMVMwareVMX *vmx = [self vmxForFile:@"basic-1" error:&error];
	
	XCTAssert(vmx, @"failed to parse file: %s", SMErrorGetUserInfo(error));
	
	SMErrorFree(error);
	error = NULL;
	
	// Write parsed file.
	NSString *tempOutput = SMGenerateTemporaryTestPath();
	
	XCTAssertTrue(SMVMwareVMXWriteToFile(vmx, tempOutput.fileSystemRepresentation, &error), @"failed to write file: %s", SMErrorGetUserInfo(error));
	
	// Compare.
	const char	*cResourcePath = SMVMwareVMXGetPath(vmx);
	NSString	*resourcePath = [[NSFileManager defaultManager] stringWithFileSystemRepresentation:cResourcePath length:strlen(cResourcePath)];
		
	NSData *refData = [NSData dataWithContentsOfFile:resourcePath];
	NSData *writtenData = [NSData dataWithContentsOfFile:tempOutput];
	
	XCTAssertNotNil(refData);
	XCTAssertNotNil(writtenData);
	
	XCTAssertEqualObjects(refData, writtenData);
	
	// Clean.
	SMVMwareVMXFree(vmx);
	SMErrorFree(error);
}

- (void)testModifications
{
	SMError *error = NULL;

	// Parse file.
	SMVMwareVMX *vmxOriginal = [self vmxForFile:@"basic-1" error:&error];
	
	XCTAssert(vmxOriginal, @"failed to parse file: %s", SMErrorGetUserInfo(error));
	
	SMErrorFree(error);
	error = NULL;
	
	// Fetch entries.
	SMVMwareVMXEntry *entry1 = SMVMwareVMXGetEntryForKey(vmxOriginal, ".encoding");
	SMVMwareVMXEntry *entry2 = SMVMwareVMXGetEntryForKey(vmxOriginal, "displayName");
	SMVMwareVMXEntry *entry3 = SMVMwareVMXGetEntryForKey(vmxOriginal, "config.version");
	SMVMwareVMXEntry *entry4 = SMVMwareVMXGetEntryForKey(vmxOriginal, "pciBridge0.present");
	SMVMwareVMXEntry *entry5 = SMVMwareVMXGetEntryAtIndex(vmxOriginal, 8);

	XCTAssertNotEqual(entry1, NULL);
	XCTAssertNotEqual(entry2, NULL);
	XCTAssertNotEqual(entry3, NULL);
	XCTAssertNotEqual(entry4, NULL);
	
	// Check entries.
	XCTAssertEqual(SMVMwareVMXEntryGetType(entry1), SMVMwareVMXEntryTypeKeyValue);
	XCTAssertEqual(SMVMwareVMXEntryGetType(entry2), SMVMwareVMXEntryTypeKeyValue);
	XCTAssertEqual(SMVMwareVMXEntryGetType(entry3), SMVMwareVMXEntryTypeKeyValue);
	XCTAssertEqual(SMVMwareVMXEntryGetType(entry4), SMVMwareVMXEntryTypeKeyValue);
	XCTAssertEqual(SMVMwareVMXEntryGetType(entry5), SMVMwareVMXEntryTypeComment);

	// Modify entries.
	XCTAssertTrue(SMVMwareVMXEntrySetKey(entry1, "key1", &error), "failed to set key: %s", SMErrorGetUserInfo(error));
	XCTAssertTrue(SMVMwareVMXEntrySetValue(entry2, "simplevalue", &error), "failed to set value: %s", SMErrorGetUserInfo(error));
	XCTAssertTrue(SMVMwareVMXEntrySetValue(entry3, "value with spaces", &error), "failed to set value: %s", SMErrorGetUserInfo(error));
	XCTAssertTrue(SMVMwareVMXEntrySetValue(entry4, "test \"a value\"", &error), "failed to set value: %s", SMErrorGetUserInfo(error));
	XCTAssertTrue(SMVMwareVMXEntrySetComment(entry5, "Hello World !", &error), "failed to set comment: %s", SMErrorGetUserInfo(error));

	// Write modified file.
	NSString *modifiedFile = SMGenerateTemporaryTestPath();
	
	XCTAssertTrue(SMVMwareVMXWriteToFile(vmxOriginal, modifiedFile.fileSystemRepresentation, &error), "failed to write file: %s", SMErrorGetUserInfo(error));
	
	SMVMwareVMXFree(vmxOriginal);
	vmxOriginal = NULL;
	
	// Open modified file.
	SMVMwareVMX *vmxReopen = SMVMwareVMXOpen(modifiedFile.fileSystemRepresentation, &error);
	
	XCTAssertNotEqual(vmxReopen, NULL, "failed to re-open modified file: %s", SMErrorGetUserInfo(error));
	
	// Validate content
	SMVMXEntryTest testEntries[] = {
		{ .type = SMVMwareVMXEntryTypeKeyValue, .key = "key1", .value = "UTF-8" },
		{ .type = SMVMwareVMXEntryTypeKeyValue, .key = "displayName", .value = "simplevalue" },
		{ .type = SMVMwareVMXEntryTypeKeyValue, .key = "config.version", .value = "value with spaces" },
		{ .type = SMVMwareVMXEntryTypeKeyValue, .key = "pciBridge0.present", .value = "test \"a value\"" },
		{ .type = SMVMwareVMXEntryTypeEmpty },
		{ .type = SMVMwareVMXEntryTypeEmpty },
		{ .type = SMVMwareVMXEntryTypeKeyValue, .key = "guestOS", .value = "darwin19-64" },
		{ .type = SMVMwareVMXEntryTypeEmpty },
		{ .type = SMVMwareVMXEntryTypeComment, .value = "Hello World !" },
		{ .type = SMVMwareVMXEntryTypeKeyValue, .key = "firmware", .value = "efi" },
	};
	
	[self validateEntriesOfVMX:vmxReopen withTestEntries:testEntries count:sizeof(testEntries) / sizeof(*testEntries)];
	
	// Clean.
	SMVMwareVMXFree(vmxReopen);
	SMErrorFree(error);
	
	[[NSFileManager defaultManager] removeItemAtPath:modifiedFile error:nil];
}

- (void)testMacOSVersion1
{
	SMError *error = NULL;

	// Parse file.
	SMVMwareVMX *vmx = [self vmxForFile:@"guestos-1" error:&error];
	
	XCTAssert(vmx, @"failed to parse file: %s", SMErrorGetUserInfo(error));
	
	SMVersion version = SMVMwareVMXExtractMacOSVersion(vmx);
	SMVersion version_ref = SMVersionFromComponents(10, 15, 0);
	
	XCTAssertTrue(SMVersionIsEqual(version, version_ref));
	
	// Clean.
	SMVMwareVMXFree(vmx);
	SMErrorFree(error);
}

- (void)testMacOSVersion2
{
	SMError *error = NULL;

	// Parse file.
	SMVMwareVMX *vmx = [self vmxForFile:@"guestos-2" error:&error];
	
	XCTAssert(vmx, @"failed to parse file: %s", SMErrorGetUserInfo(error));
	
	SMVersion version = SMVMwareVMXExtractMacOSVersion(vmx);
	SMVersion version_ref = SMVersionFromComponents(10, 15, 7);
	
	XCTAssertTrue(SMVersionIsEqual(version, version_ref));
	
	// Clean.
	SMVMwareVMXFree(vmx);
	SMErrorFree(error);
}

- (void)testSetMachineUUID
{
	SMError *error = NULL;
	
	// Parse file.
	SMVMwareVMX *vmxOriginal = [self vmxForFile:@"empty-1" error:&error];
	
	XCTAssert(vmxOriginal, @"failed to parse file: %s", SMErrorGetUserInfo(error));
	
	SMErrorFree(error);
	error = NULL;

	// Modify content.
	uuid_t uuid1 = { 0xC8, 0x62, 0xD7, 0x75, 0x62, 0x3D, 0x42, 0x99, 0x82, 0x77, 0x96, 0xA6, 0x4A, 0x69, 0x6F, 0xD5 };
	const char *uuidStr1 = "c8 62 d7 75 62 3d 42 99-82 77 96 a6 4a 69 6f d5";
	SMVMXEntryTest testEntries1[] = {
		{ .type = SMVMwareVMXEntryTypeKeyValue, .key = ".encoding", .value = "UTF-8" },
		{ .type = SMVMwareVMXEntryTypeKeyValue, .key = SMVMwareVMXUUIDBiosKey, .value = uuidStr1 },
		{ .type = SMVMwareVMXEntryTypeKeyValue, .key = SMVMwareVMXUUIDLocationKey, .value = uuidStr1 },
	};
	
	XCTAssertTrue(SMVMwareVMXSetMachineUUID(vmxOriginal, uuid1, NULL));
	
	// Validate content.
	[self validateEntriesOfVMX:vmxOriginal withTestEntries:testEntries1 count:sizeof(testEntries1) / sizeof(*testEntries1)];
	
	// Write modified file.
	NSString *modifiedFile = SMGenerateTemporaryTestPath();
	
	XCTAssertTrue(SMVMwareVMXWriteToFile(vmxOriginal, modifiedFile.fileSystemRepresentation, &error), "failed to write file: %s", SMErrorGetUserInfo(error));
	
	SMVMwareVMXFree(vmxOriginal);
	vmxOriginal = NULL;
	
	// Open modified file.
	SMVMwareVMX *vmxReopen = SMVMwareVMXOpen(modifiedFile.fileSystemRepresentation, &error);
	
	XCTAssertNotEqual(vmxReopen, NULL, "failed to re-open modified file: %s", SMErrorGetUserInfo(error));
	
	// Validate re-open.
	[self validateEntriesOfVMX:vmxReopen withTestEntries:testEntries1 count:sizeof(testEntries1) / sizeof(*testEntries1)];
	
	// Change existing.
	uuid_t uuid2 = { 0x84, 0xDE, 0x67, 0x52, 0xFB, 0xAA, 0x4C, 0x61, 0x9C, 0x05, 0xDF, 0x09, 0x92, 0x4F, 0x58, 0x36 };
	const char *uuidStr2 = "84 de 67 52 fb aa 4c 61-9c 05 df 09 92 4f 58 36";
	SMVMXEntryTest testEntries2[] = {
		{ .type = SMVMwareVMXEntryTypeKeyValue, .key = ".encoding", .value = "UTF-8" },
		{ .type = SMVMwareVMXEntryTypeKeyValue, .key = SMVMwareVMXUUIDBiosKey, .value = uuidStr2 },
		{ .type = SMVMwareVMXEntryTypeKeyValue, .key = SMVMwareVMXUUIDLocationKey, .value = uuidStr2 },
	};
	
	XCTAssertTrue(SMVMwareVMXSetMachineUUID(vmxReopen, uuid2, NULL));
	
	[self validateEntriesOfVMX:vmxReopen withTestEntries:testEntries2 count:sizeof(testEntries2) / sizeof(*testEntries2)];
	
	// Clean.
	SMVMwareVMXFree(vmxReopen);
	SMErrorFree(error);
	
	[[NSFileManager defaultManager] removeItemAtPath:modifiedFile error:nil];
}

- (void)testDetailedDataParsing
{
	// Valid 1.
	const char		*data1 = "architecture='X86' bitness='64' buildNumber='21G72' distroName='Mac OS X' distroVersion='10.16' familyName='Darwin' kernelVersion='21.6.0'";
	NSDictionary	*refdict1 = @{ @"architecture" : @"X86", @"bitness": @"64", @"buildNumber": @"21G72", @"distroName": @"Mac OS X", @"distroVersion": @"10.16", @"familyName": @"Darwin", @"kernelVersion": @"21.6.0" };
	NSDictionary	*parsedict1 = [self dictionaryFromFields:SMDetailedFieldsFromString(data1) freeFields:YES];
	
	XCTAssertEqualObjects(parsedict1, refdict1);
	
	// Valid 2.
	const char		*data2 = "architecture='X86' bitness='64'";
	NSDictionary	*refdict2 = @{ @"architecture" : @"X86", @"bitness": @"64" };
	NSDictionary	*parsedict2 = [self dictionaryFromFields:SMDetailedFieldsFromString(data2) freeFields:YES];
	
	XCTAssertEqualObjects(parsedict2, refdict2);
	
	// Valid 3.
	const char		*data3 = "architecture='X86' bitness=''";
	NSDictionary	*refdict3 = @{ @"architecture" : @"X86", @"bitness": @"" };
	NSDictionary	*parsedict3 = [self dictionaryFromFields:SMDetailedFieldsFromString(data3) freeFields:YES];
	
	XCTAssertEqualObjects(parsedict3, refdict3);
	
	// Valid 4.
	const char		*data4 = "architecture='X86'";
	NSDictionary	*refdict4 = @{ @"architecture" : @"X86" };
	NSDictionary	*parsedict4 = [self dictionaryFromFields:SMDetailedFieldsFromString(data4) freeFields:YES];
	
	XCTAssertEqualObjects(parsedict4, refdict4);
		
	// Valid 5.
	const char		*data5 = "architecture=''";
	NSDictionary	*refdict5 = @{ @"architecture" : @"" };
	NSDictionary	*parsedict5 = [self dictionaryFromFields:SMDetailedFieldsFromString(data5) freeFields:YES];
	
	XCTAssertEqualObjects(parsedict5, refdict5);
	
	// Valid 6.
	const char		*data6 = "architecture='X86' bitness='64' buildNumber='21G72' distroName='Mac\\' OS X' distroVersion='10.16' familyName='D\\\\arwin' kernelVersion='21.6.0'";
	NSDictionary	*refdict6 = @{ @"architecture" : @"X86", @"bitness": @"64", @"buildNumber": @"21G72", @"distroName": @"Mac' OS X", @"distroVersion": @"10.16", @"familyName": @"D\\arwin", @"kernelVersion": @"21.6.0" };
	NSDictionary	*parsedict6 = [self dictionaryFromFields:SMDetailedFieldsFromString(data6) freeFields:YES];
	
	XCTAssertEqualObjects(parsedict6, refdict6);
	
	// Valid 7.
	const char		*data7 = "architecture='X86' ";
	NSDictionary	*refdict7 = @{ @"architecture" : @"X86" };
	NSDictionary	*parsedict7 = [self dictionaryFromFields:SMDetailedFieldsFromString(data7) freeFields:YES];
	
	XCTAssertEqualObjects(parsedict7, refdict7);
	
	// Valid 8.
	const char		*data8 = "a='b' ";
	NSDictionary	*refdict8 = @{ @"a" : @"b" };
	NSDictionary	*parsedict8 = [self dictionaryFromFields:SMDetailedFieldsFromString(data8) freeFields:YES];
	
	XCTAssertEqualObjects(parsedict8, refdict8);
	
	// Invalid 1.
	const char		*idata1 = "architecture='X86' bitness='64' buildNumber='21G72' distroName='Mac OS X' distroVersion='10.16' familyName='Darwin' kernelVersion='21.6.0";
	NSDictionary	*irefdict1 = @{ @"architecture" : @"X86", @"bitness": @"64", @"buildNumber": @"21G72", @"distroName": @"Mac OS X", @"distroVersion": @"10.16", @"familyName": @"Darwin" };
	NSDictionary	*iparsedict1 = [self dictionaryFromFields:SMDetailedFieldsFromString(idata1) freeFields:YES];
	
	XCTAssertEqualObjects(iparsedict1, irefdict1);
	
	// Invalid 2.
	const char		*idata2 = "architecture='X86' bitness='";
	NSDictionary	*irefdict2 = @{ @"architecture" : @"X86" };
	NSDictionary	*iparsedict2 = [self dictionaryFromFields:SMDetailedFieldsFromString(idata2) freeFields:YES];
	
	XCTAssertEqualObjects(iparsedict2, irefdict2);
	
	// Invalid 3.
	const char		*idata3 = "architecture='X86' bitness=";
	NSDictionary	*irefdict3 = @{ @"architecture" : @"X86" };
	NSDictionary	*iparsedict3 = [self dictionaryFromFields:SMDetailedFieldsFromString(idata3) freeFields:YES];
	
	XCTAssertEqualObjects(iparsedict3, irefdict3);
	
	// Invalid 3.
	const char		*idata4 = "architecture='X86' bitness";
	NSDictionary	*irefdict4 = @{ @"architecture" : @"X86" };
	NSDictionary	*iparsedict4 = [self dictionaryFromFields:SMDetailedFieldsFromString(idata4) freeFields:YES];
	
	XCTAssertEqualObjects(iparsedict4, irefdict4);

	// Invalid 5.
	const char		*idata5 = "";
	NSDictionary	*irefdict5 = @{ };
	NSDictionary	*iparsedict5 = [self dictionaryFromFields:SMDetailedFieldsFromString(idata5) freeFields:YES];

	XCTAssertEqualObjects(iparsedict5, irefdict5);
}


#pragma mark - Helpers

- (SMVMwareVMX *)vmxForFile:(NSString *)file error:(SMError **)error
{
	NSBundle *bundle = [NSBundle bundleForClass:self.class];
	NSString *path = [bundle pathForResource:file ofType:@"vmx"];
	
	NSAssert(path, @"cannot find VMX file '%@'", file);
	
	return SMVMwareVMXOpen(path.fileSystemRepresentation, error);
}

- (NSDictionary *)dictionaryFromFields:(SMDetailedField *)fields freeFields:(BOOL)freeFields
{
	if (!fields)
		return @{ };
	
	NSMutableDictionary *result = [NSMutableDictionary dictionary];
	
	for (size_t i = 0; ; i++)
	{
		if (!fields[i].key || !fields[i].value)
			break;
		
		result[@(fields[i].key)] = @(fields[i].value);
	}
	
	if (freeFields)
		SMDetailedFieldsFree(fields);
	
	return result;
}

- (void)validateEntriesOfVMX:(SMVMwareVMX *)vmx withTestEntries:(SMVMXEntryTest *)testEntries count:(size_t)count
{
	// Check number of entries.
	XCTAssertEqual(SMVMwareVMXEntriesCount(vmx), count, "count mistmatch");
	
	// Check content.
	for (size_t i = 0; i < SMVMwareVMXEntriesCount(vmx); i++)
	{
		SMVMwareVMXEntry	*entry = SMVMwareVMXGetEntryAtIndex(vmx, i);
		SMVMXEntryTest		*testEntry = &testEntries[i];
		SMError				*error = NULL;
		
		XCTAssertEqual(SMVMwareVMXEntryGetType(entry), testEntry->type);
		
		switch (testEntry->type)
		{
			case SMVMwareVMXEntryTypeEmpty:
				break;
				
			case SMVMwareVMXEntryTypeComment:
			{
				const char *comment = SMVMwareVMXEntryGetComment(entry, &error);
				
				XCTAssertTrue(comment != NULL, @"failed to get comment: %s", SMErrorGetUserInfo(error));
				SMErrorFree(error);
				
				XCTAssertEqualStrings(comment, testEntry->value);
				
				break;
			}
				
			case SMVMwareVMXEntryTypeKeyValue:
			{
				const char *key = SMVMwareVMXEntryGetKey(entry, &error);
				
				XCTAssertTrue(key != NULL, @"failed to get key: %s", SMErrorGetUserInfo(error));
				SMErrorFree(error);

				const char *value = SMVMwareVMXEntryGetValue(entry, &error);
				
				XCTAssertTrue(value != NULL, @"failed to get value: %s", SMErrorGetUserInfo(error));
				SMErrorFree(error);

				
				XCTAssertEqualStrings(key, testEntry->key);
				XCTAssertEqualStrings(value, testEntry->value);
				
				break;
			}
		}
	}
	
}

@end
