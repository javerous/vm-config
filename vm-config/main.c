/*
 *  main.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <assert.h>

#include <uuid/uuid.h>

#include <sys/param.h>

#include "main.h"

#include "SMCommandLineOptions.h"

#include "SMError.h"
#include "SMStringHelper.h"

#include "SMVMwareNVRAM.h"
#include "SMVMwareNVRAMHelper.h"

#include "SMVMwareVMX.h"
#include "SMVMwareVMXHelper.h"


/*
** Defines
*/
#pragma mark - Defines

#define SMStringify(a) xSMStringify(a)
#define xSMStringify(a) #a


/*
** Types
*/
#pragma mark - Types

#ifndef _UUID_STRING_T
typedef char uuid_string_t[37];
#endif

typedef enum
{
	SMMainVerbVersion,
	SMMainVerbShow,
	SMMainVerbChange
} SMMainVerb;

typedef enum
{
	SMMainShowVM,

	SMMainShowAll,
	
	SMMainShowVMX,
	
	SMMainShowNVRAM,
	SMMainShowNVRAMEFIVariables,
	SMMainShowNVRAMEFIVariable,
} SMMainShow;

typedef enum
{
	SMMainChangeVM,
	
	SMMainChangeBootArgs,
	
	SMMainChangeCSREnable,
	SMMainChangeCSREnableVersion,

	SMMainChangeCSRDisable,
	SMMainChangeCSRDisableVersion,
	
	SMMainChangeCSRFlags,
	
	SMMainChangeMachineUUID,
	
	SMMainChangeScreenResolution,
} SMMainChange;


/*
** Prototypes
*/
#pragma mark - Prototypes

// Sub-mains.
static int main_show(SMCLOptionsResult *opt_result, FILE *fout, FILE *ferr);
static int main_change(SMCLOptionsResult *opt_result, FILE *fout, FILE *ferr);

// Information.
static void show_version(FILE *output);

// VMware.
static SMVMwareVMX *	SMGetVMXFromVM(const char *vm_path, SMVMwareVMX **inoutVMX, SMError **error);
static SMVMwareNVRAM *	SMGetNVRAMFromVM(const char *vm_path, SMVMwareVMX **inoutVMX, SMVMwareNVRAM **inoutNVRAM, SMError **error);

// Output.
static void SMDumpBytes(const void *bytes, size_t size, size_t padding, FILE *output);


/*
** Main
*/
#pragma mark - Main

int main(int argc, const char * argv[])
{
	return internal_main(argc, argv, stdout, stderr);
}

int internal_main(int argc, const char * argv[], FILE *fout, FILE *ferr)
{
	// Configuration options parsing.
	SMCLOptions *options = SMCLOptionsCreate();
	
	// > Version.
	SMCLOptionsVerb *version_verb = SMCLOptionsAddVerb(options,	SMMainVerbVersion, "version", "Show version of this command line");

	(void)version_verb;
	
	// > show.
	SMCLOptionsVerb *show_verb = SMCLOptionsAddVerb(options, SMMainVerbShow, "show", "Show configuration of a virtual machine bundle");

	SMCLOptionsVerbAddValue(show_verb,				SMMainShowVM,								"vmwarevm",															"Path to the virtual machine .vmwarevm bundle");
	SMCLOptionsVerbAddOption(show_verb, 			SMMainShowAll, 						true,	"all", 					0,											"Show all possible content");
	SMCLOptionsVerbAddOption(show_verb, 			SMMainShowVMX, 						true,	"vmx", 					0,											"Show vmx file content");
	SMCLOptionsVerbAddOption(show_verb, 			SMMainShowNVRAM,					true,	"nvram", 				0,											"Show nvram file content");
	SMCLOptionsVerbAddOption(show_verb, 			SMMainShowNVRAMEFIVariables,		true,	"nvram-efi-variables",	0,											"Show nvram efi variables");
	SMCLOptionsVerbAddOptionWithArgument(show_verb,	SMMainShowNVRAMEFIVariable,			true,	"nvram-efi-variable",	0, SMCLValueTypeString,		"name",			"Show nvram efi variable with this name");
	
	// > change.
	SMCLOptionsVerb *change_verb = SMCLOptionsAddVerb(options, SMMainVerbChange, "change", "Change configuration of a virtual machine bundle");

	SMCLOptionsVerbAddValue(change_verb,				SMMainChangeVM,							"vmwarevm",															"Path to the virtual machine .vmwarevm bundle");
	SMCLOptionsVerbAddOptionWithArgument(change_verb,	SMMainChangeBootArgs, 			true,	"boot-args", 			0,  SMCLValueTypeString,	"key=value",	"Set boot arguments");
	SMCLOptionsVerbAddOption(change_verb,				SMMainChangeCSREnable, 			true,	"csr-enable", 			0,											"Similar to 'csrutil enable'");
	SMCLOptionsVerbAddOptionWithArgument(change_verb,	SMMainChangeCSREnableVersion, 	true,	"csr-enable-version", 	0,  SMCLValueTypeString,	"version",		"Similar to 'csrutil enable' for a specific macOS version");
	SMCLOptionsVerbAddOption(change_verb,				SMMainChangeCSRDisable, 		true,	"csr-disable", 			0, 											"Similar to 'csrutil disable'");
	SMCLOptionsVerbAddOptionWithArgument(change_verb,	SMMainChangeCSRDisableVersion, 	true,	"csr-disable-version", 	0,  SMCLValueTypeString,	"version",		"Similar to 'csrutil disable' for a specific macOS version");
	SMCLOptionsVerbAddOptionWithArgument(change_verb,	SMMainChangeCSRFlags, 			true,	"csr-flags", 			0,  SMCLValueTypeUInt32,	"flags",		"Set Configurable Security Restrictions flags");
	SMCLOptionsVerbAddOptionWithArgument(change_verb,	SMMainChangeMachineUUID, 		true,	"machine-uuid", 		0,  SMCLValueTypeString,	"uuid",			"Set machine UUID");
	SMCLOptionsVerbAddOptionWithArgument(change_verb,	SMMainChangeScreenResolution, 	true,	"screen-resolution",	0,  SMCLValueTypeString,	"WxH",			"Set screen resolution, width x height, e.g. '1920x1080'");
	
	// Parse options.
	SMError				*error = NULL;
	SMCLOptionsResult	*result = SMCLOptionsParse(options, argc, argv, &error);
	
	if (!result)
	{
		fprintf(ferr, "Error: %s\n", SMErrorGetSentencizedUserInfo(error));
		fprintf(ferr, "\n");
		SMCLOptionsPrintUsage(options, ferr);
		
		SMCLOptionsFree(options);

		return SMMainExitInvalidArgs;
	}
	
	// Dispatch verb handing.
	int exit_code = 0;
	
	switch ((SMMainVerb)SMCLOptionsResultVerbIdentifier(result))
	{
		case SMMainVerbVersion:
			show_version(fout);
			break;
			
		case SMMainVerbShow:
			exit_code = main_show(result, fout, ferr);
			break;
			
		case SMMainVerbChange:
			exit_code = main_change(result, fout, ferr);
			break;
	}
	
	SMCLOptionsResultFree(result);
	SMCLOptionsFree(options);

	return exit_code;
}


#pragma mark > Show

static int main_show(SMCLOptionsResult *opt_result, FILE *fout, FILE *ferr)
{
	int 			result = SMMainExitSuccess;

	SMVMwareVMX		*g_vmx = NULL;
	SMVMwareNVRAM	*g_nvram = NULL;
	SMError			*error = NULL;
	
	// Handle options.
	const char	*vm_path;
	bool		show_vmx = false;
	bool		show_nvram = false;
	bool		show_nvram_efi_variables = false;
	const char	*show_nvram_efi_variable = NULL;

	for (size_t i = 0; i < SMCLOptionsResultParametersCount(opt_result); i++)
	{
		SMMainShow mainShowOp = (SMMainShow)SMCLOptionsResultParameterIdentifierAtIndex(opt_result, i);
		
		switch (mainShowOp)
		{
			case SMMainShowVM:
			{
				vm_path = SMCLOptionsResultParameterStringValueAtIndex(opt_result, i);
				break;
			}
				
			case SMMainShowAll:
			{
				show_vmx = true;
				show_nvram = true;
				show_nvram_efi_variables = true;
				
				break;
			}
				
			case SMMainShowVMX:
			{
				show_vmx = true;
				break;
			}
				
			case SMMainShowNVRAM:
			{
				show_nvram = true;
				show_nvram_efi_variables = true;
				
				break;
			}
				
			case SMMainShowNVRAMEFIVariables:
			{
				show_nvram_efi_variables = true;
				break;
			}
				
			case SMMainShowNVRAMEFIVariable:
			{
				show_nvram_efi_variable = SMCLOptionsResultParameterStringValueAtIndex(opt_result, i);
				break;
			}
		}
	}
	
	// Print VMX.
	if (show_vmx)
	{
		// > Open VMX file.
		SMVMwareVMX *vmx = SMGetVMXFromVM(vm_path, &g_vmx, &error);
		
		if (!vmx)
		{
			result = SMMainExitInvalidVM;
			goto fail;
		}
						
		// > List & print entries.
		size_t count = SMVMwareVMXEntriesCount(vmx);
		
		fprintf(fout, "-- VMX (%lu entries) --\n", count);
		
		for (size_t i = 0; i < count; i++)
		{
			SMVMwareVMXEntry *entry = SMVMwareVMXGetEntryAtIndex(vmx, i);
			
			switch (SMVMwareVMXEntryGetType(entry))
			{
				case SMVMwareVMXEntryTypeEmpty:
					fprintf(fout, "Type: empty-line\n");
					continue;
					
				case SMVMwareVMXEntryTypeComment:
				{
					const char *comment = SMVMwareVMXEntryGetComment(entry, &error);
					
					if (!comment)
						goto fail;
					
					fprintf(fout, "Type: comment\n");
					fprintf(fout, "  > value = '%s'\n", comment);

					break;
				}
					
				case SMVMwareVMXEntryTypeKeyValue:
				{
					const char *key = SMVMwareVMXEntryGetKey(entry, &error);
					
					if (!key)
						goto fail;
					
					const char *value = SMVMwareVMXEntryGetValue(entry, &error);

					if (!value)
						goto fail;
					
					fprintf(fout, "Type: key-value\n");
					
					fprintf(fout, "  > key   = %s\n", key);
					fprintf(fout, "  > value = %s\n", value);

					break;
				}
			}
			
			fprintf(fout, "\n");
		}
	}
	
	// Print NVRAM.
	if (show_nvram || show_nvram_efi_variables || show_nvram_efi_variable)
	{
		// > Open NVRAM file.
		SMVMwareNVRAM *nvram = SMGetNVRAMFromVM(vm_path, &g_vmx, &g_nvram, &error);
		
		if (!nvram)
		{
			result = SMMainExitInvalidVM;
			goto fail;
		}
		
		// > List & print entries.
		size_t count = SMVMwareNVRAMEntriesCount(nvram);
		
		if (show_nvram)
			fprintf(fout, "-- NVRAM (%lu entries) --\n", count);
		else if (show_nvram_efi_variables)
			fprintf(fout, "-- NVRAM EFI Variables --\n");
		
		for (size_t i = 0; i < count; i++)
		{
			SMVMwareNVRAMEntry 	*entry = SMVMwareNVRAMGetEntryAtIndex(nvram, i);
			const char			*name = SMVMwareNVRAMEntryGetName(entry);
			const char			*subname = SMVMwareNVRAMEntryGetSubname(entry);
			
			if (show_nvram)
			{
				if (strlen(subname) > 0)
					fprintf(fout, "%s - %s\n", name, subname);
				else
					fprintf(fout, "%s\n", name);
			}
			
			size_t var_count = SMVMwareNVRAMEntryVariablesCount(entry);
			
			// > Show variables.
			if ((show_nvram_efi_variables || show_nvram_efi_variable) && var_count > 0)
			{
				if (show_nvram_efi_variables)
					fprintf(fout, " > Variables: %lu\n", var_count);

				efi_guid_t apple_nvram_variable_guid = Apple_NVRAM_Variable_Guid;
				efi_guid_t apple_screen_resolution_guid = Apple_Screen_Resolution_Guid;
				efi_guid_t dhcpv6_service_binding_guid = DHCPv6_Service_Binding_Guid;

				// > Handle vars if any.
				for (size_t j = 0; j < var_count; j++)
				{
					SMVMwareNVRAMEFIVariable *var = SMVMwareNVRAMEntryGetVariableAtIndex(entry, j);

					uint32_t	attributes = SMVMwareNVRAMVariableGetAttributes(var);

					size_t		name_size = 0;
					const void	*name = SMVMwareNVRAMVariableGetName(var, &name_size);

					size_t		value_size = 0;
					const void	*value = SMVMwareNVRAMVariableGetValue(var, &value_size);
					
					const char	*utf8_name = SMVMwareNVRAMVariableGetUTF8Name(var, NULL);
										
					// > Check if we are interested by this variable.
					if (!show_nvram_efi_variables && show_nvram_efi_variable)
					{
						if (utf8_name)
						{
							if (strcmp(utf8_name, show_nvram_efi_variable) != 0)
								continue;
						}
						else
							continue;
					}

					// > GUID.
					efi_guid_t	guid = SMVMwareNVRAMVariableGetGUID(var);
					char		guid_str[SMVMwareGUIDStringSize + 1];

					SMVMwareNVRAMGUIDToGUIDString(&guid, guid_str);

					fprintf(fout, "    GUID %s\n", guid_str);

					// > Attributes.
					fprintf(fout, "       Attributes: 0x%x\n", attributes);
					
					if (attributes & EFI_VARIABLE_NON_VOLATILE)								fprintf(fout, "           efi-variable-non-volatile\n");
					if (attributes & EFI_VARIABLE_BOOTSERVICE_ACCESS)						fprintf(fout, "           efi-variable-bootservice-access\n");
					if (attributes & EFI_VARIABLE_RUNTIME_ACCESS)							fprintf(fout, "           efi-variable-runtime-access\n");
					if (attributes & EFI_VARIABLE_HARDWARE_ERROR_RECORD)					fprintf(fout, "           efi-variable-hardware-error-record\n");
					if (attributes & EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS)				fprintf(fout, "           efi-variable-authenticated-write-access\n");
					if (attributes & EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS)	fprintf(fout, "           efi-variable-time-based-authenticated-write-access\n");
					if (attributes & EFI_VARIABLE_APPEND_WRITE)								fprintf(fout, "           efi-variable-append-write\n");

					fprintf(fout, "\n");

					// > Name.
					fprintf(fout, "       Name:\n");
					SMDumpBytes(name, name_size, 11, fout);
					fprintf(fout, "\n");
					
					// > UTF-8 name.
					if (utf8_name)
					{
						bool	valid_utf = true;
						size_t	len = strlen(utf8_name);

						for (size_t i = 0; i < len && valid_utf; i++)
							valid_utf = isprint(utf8_name[i]);

						if (valid_utf)
							fprintf(fout, "       Name (UTF-8): %s\n\n", utf8_name);
					}
					
					// > Value.
					fprintf(fout, "       Value:\n");
					SMDumpBytes(value, value_size, 11, fout);
					
					fprintf(fout, "\n");

					// > Specific value handling.
					if (memcmp(&guid, &apple_nvram_variable_guid, sizeof(guid)) == 0)
					{
						if (utf8_name && strcmp(utf8_name, "csr-active-config") == 0 && value_size == 4)
						{
							const uint32_t	*csr_ptr = value;
							uint32_t 		csr = *csr_ptr;
							
							fprintf(fout, "       Value (Configurable Security Restrictions): 0x%x\n", csr);
							
							if (csr & CSR_ALLOW_UNTRUSTED_KEXTS)			fprintf(fout, "           allow-untrusted-kexts\n");
							if (csr & CSR_ALLOW_UNRESTRICTED_FS)			fprintf(fout, "           allow-unrestricted-fs\n");
							if (csr & CSR_ALLOW_TASK_FOR_PID)				fprintf(fout, "           allow-task-for-pid\n");
							if (csr & CSR_ALLOW_KERNEL_DEBUGGER)			fprintf(fout, "           allow-kernel-debugger\n");
							if (csr & CSR_ALLOW_APPLE_INTERNAL)				fprintf(fout, "           allow-apple-internal\n");
							if (csr & CSR_ALLOW_UNRESTRICTED_DTRACE)		fprintf(fout, "           allow-unrestricted-dtrace\n");
							if (csr & CSR_ALLOW_UNRESTRICTED_NVRAM)			fprintf(fout, "           allow-unrestricted-nvram\n");
							if (csr & CSR_ALLOW_DEVICE_CONFIGURATION)		fprintf(fout, "           allow-device-configuration\n");
							if (csr & CSR_ALLOW_ANY_RECOVERY_OS)			fprintf(fout, "           allow-any-recovery-os\n");
							if (csr & CSR_ALLOW_UNAPPROVED_KEXTS)			fprintf(fout, "           allow-unapproved-kexts\n");
							if (csr & CSR_ALLOW_EXECUTABLE_POLICY_OVERRIDE)	fprintf(fout, "           allow-executable-policy-override\n");
							if (csr & CSR_ALLOW_UNAUTHENTICATED_ROOT)		fprintf(fout, "           allow-unauthenticated-root\n");
							
							fprintf(fout, "\n");
						}
						else if (utf8_name && strcmp(utf8_name, "platform-uuid") == 0 && value_size == sizeof(uuid_t))
						{
							uuid_string_t uuid_str;
							
							uuid_unparse(value, uuid_str);
							
							fprintf(fout, "       Value (UUID): %s\n", uuid_str);
							fprintf(fout, "\n");
						}
						else if (utf8_name && strcmp(utf8_name, "fmm-computer-name") == 0 && value_size > 0)
						{
							fprintf(fout, "       Value (Find My Mac Computer Name): ");
							
							if (((const char *)value)[value_size - 1] != 0)
							{
								fwrite(value, value_size, 1, fout);
								fprintf(fout, "\n\n");
							}
							else
								fprintf(fout, "%s\n\n", (const char *)value);
						}
					}
					else if (memcmp(&guid, &apple_screen_resolution_guid, sizeof(guid)) == 0)
					{
						if (utf8_name && strcmp(utf8_name, "width") == 0 && value_size == 4)
						{
							const uint32_t *width = value;

							fprintf(fout, "       Value (screen width): %u pixels\n", *width);
							fprintf(fout, "\n");
						}
						else if (utf8_name && strcmp(utf8_name, "height") == 0 && value_size == 4)
						{
							const uint32_t *height = value;

							fprintf(fout, "       Value (screen height): %u pixels\n", *height);
							fprintf(fout, "\n");
						}
					}
					else if (memcmp(&guid, &dhcpv6_service_binding_guid, sizeof(guid)) == 0)
					{
						if (utf8_name && strcmp(utf8_name, "ClientId") == 0 && value_size == 4 + sizeof(uuid_t))
						{
							const uint32_t	*unknown = value;
							uuid_string_t	uuid_str;
							
							uuid_unparse(value + 4, uuid_str);
							
							fprintf(fout, "       Value (DHCPv6 Service Binding - Client ID):\n");
							fprintf(fout, "           Unknown value: 0x%x (%u)\n", *unknown, *unknown);
							fprintf(fout, "           UUID:          %s\n", uuid_str);
							fprintf(fout, "\n");
						}
					}
				}

				fprintf(fout, "\n");
			}

			// > Show nvram sections content.
			else if (show_nvram)
			{
				size_t		bytes_size = 0;
				const void	*bytes = SMVMwareNVRAMEntryGetContentBytes(entry, &bytes_size);
				
				fprintf(fout, " > Size : %lu\n", bytes_size);

				if (bytes_size > 0)
				{
					fprintf(fout, " > Bytes:\n");

					SMDumpBytes(bytes, bytes_size, 3, fout);
				}

				fprintf(fout, "\n");
			}
		}
	}
	
	// Done.
	goto clean;
	
fail:
	
	if (result == SMMainExitSuccess)
		result = SMMainExitUnknowError;
	
	if (error)
		fprintf(ferr, "Error: %s\n", SMErrorGetSentencizedUserInfo(error));
	
clean:
	SMErrorFree(error);
	SMVMwareVMXFree(g_vmx);
	SMVMwareNVRAMFree(g_nvram);
	
	return result;
}


#pragma mark > Change

static int main_change(SMCLOptionsResult *opt_result, FILE *fout, FILE *ferr)
{
	int 			result = SMMainExitSuccess;

	SMVMwareVMX		*g_vmx = NULL;
	SMVMwareNVRAM	*g_nvram = NULL;
	SMError			*error = NULL;
	
	// Handle options.
	const char		*vm_path;
	char			*vmx_path_tmp_path = NULL;
	char			*nvram_path_tmp_path = NULL;
	
	for (size_t i = 0; i < SMCLOptionsResultParametersCount(opt_result); i++)
	{
		SMMainChange mainChangeOp = (SMMainChange)SMCLOptionsResultParameterIdentifierAtIndex(opt_result, i);
		
		switch (mainChangeOp)
		{
			case SMMainChangeVM:
			{
				vm_path = SMCLOptionsResultParameterStringValueAtIndex(opt_result, i);
				break;
			}
				
			case SMMainChangeBootArgs:
			{
				// Open NVRAM file.
				SMVMwareNVRAM *nvram = SMGetNVRAMFromVM(vm_path, &g_vmx, &g_nvram, &error);

				if (!nvram)
				{
					result = SMMainExitInvalidVM;
					goto fail;
				}

				// Set boot arguments.
				if (!SMVMwareNVRAMSetBootArgs(nvram, SMCLOptionsResultParameterStringValueAtIndex(opt_result, i), &error))
					goto fail;
				
				break;
			}
				
			case SMMainChangeCSREnable:
			case SMMainChangeCSREnableVersion:
			case SMMainChangeCSRDisable:
			case SMMainChangeCSRDisableVersion:
			{
				SMVersion macos_version = SMVersionInvalid;

				// Parse version.
				if (mainChangeOp == SMMainChangeCSREnableVersion || mainChangeOp == SMMainChangeCSRDisableVersion)
				{
					macos_version = SMVersionFromString(SMCLOptionsResultParameterStringValueAtIndex(opt_result, i), &error);

					if (SMVersionIsEqual(macos_version, SMVersionInvalid))
						goto fail;
				}

				// Try to extract version from VMX.
				else if (mainChangeOp == SMMainChangeCSREnable || mainChangeOp == SMMainChangeCSRDisable)
				{
					// Get VMX.
					SMVMwareVMX *vmx = SMGetVMXFromVM(vm_path, &g_vmx, &error);
					
					if (!vmx)
					{
						result = SMMainExitInvalidVM;
						goto fail;
					}
					
					// Extract macOS version.
					macos_version = SMVMwareVMXExtractMacOSVersion(vmx);
					
					if (SMVersionIsEqual(macos_version, SMVersionInvalid))
						fprintf(ferr, "Warning: Unable to detected macOS version. Using an acceptable CSR default behavior.\n");
					else
						fprintf(ferr, "Info: macOS version %d.%d.%d detected. Using this version for CSR behavior.\n", macos_version.major_version, macos_version.minor_version, macos_version.patch_version);
					
					fprintf(ferr, "\n");
				}

				// Open NVRAM file.
				SMVMwareNVRAM *nvram = SMGetNVRAMFromVM(vm_path, &g_vmx, &g_nvram, &error);
				
				if (!nvram)
				{
					result = SMMainExitInvalidVM;
					goto fail;
				}

				// Change CSR active configuration.
				bool enable = (mainChangeOp == SMMainChangeCSREnable || mainChangeOp == SMMainChangeCSREnableVersion);
					
				if (!SMVMwareNVRAMSetAppleCSRActivation(nvram, macos_version, enable, &error))
					goto fail;
				
				break;
			}
				
			case SMMainChangeCSRFlags:
			{
				uint32_t new_csr = SMCLOptionsResultParameterUInt32ValueAtIndex(opt_result, i);
				
				// Open NVRAM file.
				SMVMwareNVRAM *nvram = SMGetNVRAMFromVM(vm_path, &g_vmx, &g_nvram, &error);
				
				if (!nvram)
				{
					result = SMMainExitInvalidVM;
					goto fail;
				}
					
				// Change CSR active configuration.
				if (!SMVMwareNVRAMSetAppleCSRActiveConfig(nvram, new_csr, &error))
					goto fail;
				
				break;
			}
				
			case SMMainChangeMachineUUID:
			{
				// Parse UUID.
				const char	*uuid_str = SMCLOptionsResultParameterStringValueAtIndex(opt_result, i);;
				uuid_t		uuid;
				
				if (uuid_parse(uuid_str, uuid) == -1)
				{
					fprintf(ferr, "Error: Invalid UUID '%s'.\n", uuid_str);
					goto fail;
				}
				
				// Open VMX.
				SMVMwareVMX *vmx = SMGetVMXFromVM(vm_path, &g_vmx, &error);
				
				if (!vmx)
				{
					result = SMMainExitInvalidVM;
					goto fail;
				}
				
				// Change machine UUID.
				if (!SMVMwareVMXSetMachineUUID(vmx, uuid, &error))
					goto fail;

				// Open NVRAM file.
				SMVMwareNVRAM *nvram = SMGetNVRAMFromVM(vm_path, &g_vmx, &g_nvram, &error);
				
				if (!nvram)
				{
					result = SMMainExitInvalidVM;
					goto fail;
				}
				
				// Change machine UUID.
				if (!SMVMwareNVRAMSetAppleMachineUUID(nvram, uuid, &error))
					goto fail;
				
				break;
			}
				
			case SMMainChangeScreenResolution:
			{
				// Parse screen resolution.
				const char		*value = SMCLOptionsResultParameterStringValueAtIndex(opt_result, i);
				unsigned int	width = 0, height = 0;
				size_t			optarg_len = strlen(value);
				int				scan_len = 0;
				int				sresult = sscanf(value, "%ux%u%n", &width, &height, &scan_len);
				
				if (sresult != 2 || scan_len != optarg_len)
				{
					fprintf(ferr, "Error: Invalid screen resolution.\n");
					goto fail;
				}
				
				// Open NVRAM file.
				SMVMwareNVRAM *nvram = SMGetNVRAMFromVM(vm_path, &g_vmx, &g_nvram, &error);
				
				if (!nvram)
				{
					result = SMMainExitInvalidVM;
					goto fail;
				}
				
				// Change screen resolution.
				if (!SMVMwareNVRAMSetScreenResolution(nvram, width, height, &error))
					goto fail;
				
				break;
			}
		}
	}

	// Write files.
	const char	*vmx_path = NULL;
	const char	*nvram_path = NULL;
	
	// > Generate tmp uuid.
	uuid_t			tmp_uuid = { 0 };
	uuid_string_t	tmp_uuid_str = { 0 };
	
	uuid_generate(tmp_uuid);
	uuid_unparse(tmp_uuid, tmp_uuid_str);
	
	// > Write modified VMX.
	if (g_vmx)
	{
		// > Fetch original file path.
		vmx_path = SMVMwareVMXGetPath(g_vmx);
		
		// > Generate temp path.
		char vmx_path_bck_name[sizeof(uuid_string_t) + 10];

		snprintf(vmx_path_bck_name, sizeof(vmx_path_bck_name), "._%s.vmx", tmp_uuid_str);
		vmx_path_tmp_path = SMStringPathAppendComponent(vm_path, vmx_path_bck_name);
				
		// > Write to tmp path.
		if (!SMVMwareVMXWriteToFile(g_vmx, vmx_path_tmp_path, &error))
			goto fail;
	}
	
	// > Write modified NVRAM.
	if (g_nvram)
	{
		// > Fetch original file path.
		nvram_path = SMVMwareNVRAMGetPath(g_nvram);
		
		// > Generate temp path.
		char nvram_path_bck_name[sizeof(uuid_string_t) + 10];
		
		snprintf(nvram_path_bck_name, sizeof(nvram_path_bck_name), "._%s.nvram", tmp_uuid_str);
		nvram_path_tmp_path = SMStringPathAppendComponent(vm_path, nvram_path_bck_name);
				
		// > Write to tmp path.
		if (!SMVMwareNVRAMWriteToFile(g_nvram, nvram_path_tmp_path, &error))
			goto fail;
	}
	
	// > Stage files.
	if (vmx_path && vmx_path_tmp_path && rename(vmx_path_tmp_path, vmx_path) == -1)
	{
		fprintf(ferr, "Error: Failed to replace vmx file ('%s' -> '%s') - %d (%s).\n", vmx_path_tmp_path, vmx_path, errno, strerror(errno));
		goto fail;
	}
	
	if (nvram_path && nvram_path_tmp_path && rename(nvram_path_tmp_path, nvram_path) == -1)
	{
		fprintf(ferr, "Error: Failed to replace nvram file ('%s' -> '%s') - %d (%s).\n", nvram_path_tmp_path, nvram_path, errno, strerror(errno));
		goto fail;
	}
	
	// Finish.
	fprintf(fout, "Virtual machine configuration changed with success.\n");
	
	goto clean;
	
fail:
	if (result == SMMainExitSuccess)
		result = SMMainExitUnknowError;
	
	if (error)
		fprintf(ferr, "Error: %s\n", SMErrorGetSentencizedUserInfo(error));
	
clean:
	SMErrorFree(error);
	SMVMwareVMXFree(g_vmx);
	SMVMwareNVRAMFree(g_nvram);
	
	if (vmx_path_tmp_path)
	{
		unlink(vmx_path_tmp_path);
		free(vmx_path_tmp_path);
	}
	
	if (nvram_path_tmp_path)
	{
		unlink(nvram_path_tmp_path);
		free(nvram_path_tmp_path);
	}
	
	return result;
}


/*
** Information
*/
#pragma mark - Information

static void show_version(FILE *output)
{
#ifndef PROJ_VERSION
#  error Project version not defined
#endif
	
	fprintf(output, "vm-config version " SMStringify(PROJ_VERSION) "\n");
}


/*
** Helpers
*/
#pragma mark - Helpers

#pragma mark > VMware

static SMVMwareVMX * SMGetVMXFromVM(const char *vm_path, SMVMwareVMX **inoutVMX, SMError **error)
{
	if (*inoutVMX)
		return *inoutVMX;
		
	SMVMwareVMX *result = NULL;
	
	// Open vm directory bundle.
	DIR *dir = opendir(vm_path);
	
	if (!dir)
	{
		SMSetErrorPtr(error, "main", -1, "can't open virtual machine bundle (%d - %s)", errno, strerror(errno));
		goto finish;
	}
	
	// Search vmx file.
	struct dirent 	*dp;
	bool			found_vmx = false;
	
	while ((dp = readdir(dir)) != NULL)
	{
		if (!SMStringPathHasExtension(dp->d_name, "vmx"))
			continue;
		
		char *path = SMStringPathAppendComponent(vm_path, dp->d_name);
		
		found_vmx = true;
		result = SMVMwareVMXOpen(path, error);
		
		free(path);
		
		break;
	}
	
	if (!found_vmx)
		SMSetErrorPtr(error, "main", -1, "can't find VMX file in virtual machine bundle");
	
finish:
	if (dir)
		closedir(dir);
	
	if (result)
		*inoutVMX = result;
	
	return result;
	
}

static SMVMwareNVRAM * SMGetNVRAMFromVM(const char *vm_path, SMVMwareVMX **inoutVMX, SMVMwareNVRAM **inoutNVRAM, SMError **error)
{
	if (*inoutNVRAM)
		return *inoutNVRAM;
	
	SMVMwareNVRAM *result = NULL;
	
	// Get VMX.
	SMVMwareVMX *vmx = SMGetVMXFromVM(vm_path, inoutVMX, error);
	
	if (!vmx)
		return NULL;
	
	// Get NVRAM entry.
	SMVMwareVMXEntry *entry = SMVMwareVMXGetEntryForKey(vmx, SMVMwareVMXNVRAMFileKey);
	
	if (!entry)
	{
		SMSetErrorPtr(error, "main", -1, "can't find nvram file key");
		goto finish;
	}
	
	// Get NVRAM file name.
	const char *name = SMVMwareVMXEntryGetValue(entry, error);
	
	if (!name)
		goto finish;
	
	// Forge NVRAM path.
	char *path = SMStringPathAppendComponent(vm_path, name);
		
	result = SMVMwareNVRAMOpen(path, error);
	
	free(path);
	
finish:
	
	if (result)
		*inoutNVRAM = result;
	
	return result;
}


#pragma mark > Output

static void SMDumpBytes(const void *bytes, size_t size, size_t padding, FILE *output)
{
#define SMDumpLineBytes 25

	const uint8_t *ubytes = bytes;

	// Forge padding string.
	char *padding_str = malloc(padding + 1);
	
	assert(padding_str);
	
	memset(padding_str, ' ', padding);
	padding_str[padding] = 0;
		
	// Print lines.
	size_t total_bytes_handled = 0;

	while (total_bytes_handled < size)
	{
		size_t line_bytes_size = MIN(SMDumpLineBytes, size - total_bytes_handled);
		
		// Print padding.
		fputs(padding_str, output);
		
		// Print bytes.
		for (size_t i = 0; i < line_bytes_size; i++)
			fprintf(output, "%02x ", ubytes[total_bytes_handled + i]);
		
		// Align bytes.
		for (size_t i = 0; i < SMDumpLineBytes - line_bytes_size ; i++)
			fputs("   ", output);

		fprintf(output, "| ");
		
		// Print ASCII.
		for (size_t i = 0; i < line_bytes_size; i++)
		{
			uint8_t byte = ubytes[total_bytes_handled + i];
			
			fprintf(output, "%c", isprint(byte) ? byte : '.' );
		}
		
		fprintf(output, "\n");
		
		total_bytes_handled += line_bytes_size;
	}
	
	// Clean.
	free(padding_str);
}
