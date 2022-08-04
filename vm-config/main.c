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
#include <getopt.h>
#include <errno.h>

#include <uuid/uuid.h>

#include <sys/param.h>

#include "SMError.h"
#include "SMStringHelper.h"

#include "SMVMwareNVRAM.h"
#include "SMVMwareNVRAMHelper.h"

#include "SMVMwareVMX.h"
#include "SMVMwareVMXHelper.h"


/*
** Types
*/
#pragma mark - Types

#ifndef _UUID_STRING_T
typedef char uuid_string_t[37];
#endif


/*
** Prototypes
*/
#pragma mark - Prototypes

// Sub-mains.
static int main_show(int argc, const char * argv[]);
static int main_change(int argc, const char * argv[]);

// VMware.
static SMVMwareVMX *	SMGetVMXFromVM(const char *vm_path, SMVMwareVMX **inoutVMX, SMError **error);
static SMVMwareNVRAM *	SMGetNVRAMFromVM(const char *vm_path, SMVMwareVMX **inoutVMX, SMVMwareNVRAM **inoutNVRAM, SMError **error);

// Output.
static void SMDumpBytes(const void *bytes, size_t size, size_t padding);


/*
** Main
*/
#pragma mark - Main

int main(int argc, const char * argv[])
{
	if (argc < 2)
	{
		fprintf(stderr, "Usage: %s <verb>\n", getprogname());
		fprintf(stderr, "\n");
		fprintf(stderr, "verbs:\n");
		fprintf(stderr, "  show [vm.vmwarevm] <content>\n");
		fprintf(stderr, "    --all                            show all possible content\n");
		fprintf(stderr, "    --vmx                            show vmx file content\n");
		fprintf(stderr, "    --nvram                          show nvram file content\n");
		fprintf(stderr, "    --nvram-efi-variables            show nvram efi variables\n");
		fprintf(stderr, "    --nvram-efi-variable <name>      show nvram efi the variable with this name\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "  change [vm.vmwarevm] <operations>\n");
		fprintf(stderr, "    --boot-args <string>             set boot arguments\n");
		fprintf(stderr, "    --csr-enable                     similar to 'csrutil enable'\n");
		fprintf(stderr, "    --csr-enable-version <version>   similar to 'csrutil enable' for a specific macOS version\n");
		fprintf(stderr, "    --csr-disable                    similar to 'csrutil disable'\n");
		fprintf(stderr, "    --csr-disable-version <version>  similar to 'csrutil disable' for a specific macOS version\n");
		fprintf(stderr, "    --csr-flags <flags>              set Configurable Security Restrictions flags\n");
		fprintf(stderr, "    --machine-uuid <uuid>            set machine UUID\n");
		fprintf(stderr, "    --screen-resolution <WxH>        set screen resolution, width x height, i.e. '1920x1080'\n");

		return 1;
	}
	
	const char *verb = argv[1];
	
	if (strcmp(verb, "show") == 0)
		return main_show(argc - 1, argv + 1);
	else if (strcmp(verb, "change") == 0)
		return main_change(argc - 1, argv + 1);
	else
	{
		fprintf(stderr, "Unknow verb '%s'.\n", verb);
		return 1;
	}
}

#pragma mark > Show

static int main_show(int argc, const char * argv[])
{
	int 			result = EXIT_SUCCESS;

	SMVMwareVMX		*g_vmx = NULL;
	SMVMwareNVRAM	*g_nvram = NULL;
	SMError			*error = NULL;
	
	// Check & extract vm path.
	const char *vm_path;
	
	if (argc < 2)
	{
		fprintf(stderr, "Error: missing vm path.\n");
		return EXIT_FAILURE;
	}
	
	vm_path = argv[1];
	
	argc--;
	argv++;
	
	// Handle options.
	int ch;
	
	bool show_vmx = false;
	bool show_nvram = false;
	bool show_nvram_efi_variables = false;
	char *show_nvram_efi_variable = NULL;

	typedef enum
	{
		SMMainShowAll,
		
		SMMainShowVMX,
		
		SMMainShowNVRAM,
		SMMainShowNVRAMEFIVariables,
		SMMainShowNVRAMEFIVariable,
	} SMMainShow;
	
	static struct option longopts[] = {
		{ "all",      				no_argument,		NULL,	SMMainShowAll },
		
		{ "vmx",					no_argument,		NULL,	SMMainShowVMX },
		
		{ "nvram",					no_argument,		NULL,	SMMainShowNVRAM },
		{ "nvram-efi-variables",	no_argument,		NULL,	SMMainShowNVRAMEFIVariables },
		{ "nvram-efi-variable",		required_argument,	NULL,	SMMainShowNVRAMEFIVariable },

		{ NULL,         0,                      NULL,           0 }
	};
		
	while ((ch = getopt_long(argc, (char * const *)argv, "", longopts, NULL)) != -1)
	{
		switch (ch)
		{
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
				free(show_nvram_efi_variable);
				show_nvram_efi_variable = strdup(optarg);
				break;
			}
				
			default:
				fprintf(stderr, "Error: invalid option - check usage.\n");
				goto fail;
		}
	}
	
	argc -= optind;
	argv += optind;
	
	if (argc != 0)
	{
		fprintf(stderr, "Error: invalid extra parameters - check usage.\n");
		goto fail;
	}
	
	
	// Print VMX.
	if (show_vmx)
	{
		SMVMwareVMX *vmx = SMGetVMXFromVM(vm_path, &g_vmx, &error);
		
		if (!vmx)
			goto fail;
						
		size_t count = SMVMwareVMXEntriesCount(vmx);
		
		fprintf(stdout, "-- VMX (%lu entries) --\n", count);
		
		for (size_t i = 0; i < count; i++)
		{
			SMVMwareVMXEntry *entry = SMVMwareVMXGetEntryAtIndex(vmx, i);
			
			switch (SMVMwareVMXEntryGetType(entry))
			{
				case SMVMwareVMXEntryTypeEmpty:
					fprintf(stdout, "Type: empty-line\n");
					continue;
					
				case SMVMwareVMXEntryTypeComment:
				{
					const char *comment = SMVMwareVMXEntryGetComment(entry, &error);
					
					if (!comment)
						goto fail;
					
					fprintf(stdout, "Type: comment\n");
					fprintf(stdout, "  > value = '%s'\n", comment);

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
					
					fprintf(stdout, "Type: key-value\n");
					
					fprintf(stdout, "  > key   = %s\n", key);
					fprintf(stdout, "  > value = %s\n", value);

					break;
				}
			}
			
			fprintf(stdout, "\n");
		}
	}
	
	// Print NVRAM.
	if (show_nvram || show_nvram_efi_variables || show_nvram_efi_variable)
	{
		SMVMwareNVRAM *nvram = SMGetNVRAMFromVM(vm_path, &g_vmx, &g_nvram, &error);
		
		if (!nvram)
			goto fail;
		
		size_t count = SMVMwareNVRAMEntriesCount(nvram);
		
		if (show_nvram)
			fprintf(stdout, "-- NVRAM (%lu entries) --\n", count);
		else if (show_nvram_efi_variables)
			fprintf(stdout, "-- NVRAM EFI Variables --\n");
		
		for (size_t i = 0; i < count; i++)
		{
			SMVMwareNVRAMEntry 	*entry = SMVMwareNVRAMGetEntryAtIndex(nvram, i);
			const char			*name = SMVMwareNVRAMEntryGetName(entry);
			const char			*subname = SMVMwareNVRAMEntryGetSubname(entry);
			
			if (show_nvram)
			{
				if (strlen(subname) > 0)
					fprintf(stdout, "%s - %s\n", name, subname);
				else
					fprintf(stdout, "%s\n", name);
			}
			
			size_t var_count = SMVMwareNVRAMEntryVariablesCount(entry);
			
			// > Show variables.
			if ((show_nvram_efi_variables || show_nvram_efi_variable) && var_count > 0)
			{
				if (show_nvram_efi_variables)
					fprintf(stdout, " > Variables: %lu\n", var_count);

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

					fprintf(stdout, "    GUID %s\n", guid_str);

					// > Attributes.
					fprintf(stdout, "       Attributes: 0x%x\n", attributes);
					
					if (attributes & EFI_VARIABLE_NON_VOLATILE)								fprintf(stdout, "           efi-variable-non-volatile\n");
					if (attributes & EFI_VARIABLE_BOOTSERVICE_ACCESS)						fprintf(stdout, "           efi-variable-bootservice-access\n");
					if (attributes & EFI_VARIABLE_RUNTIME_ACCESS)							fprintf(stdout, "           efi-variable-runtime-access\n");
					if (attributes & EFI_VARIABLE_HARDWARE_ERROR_RECORD)					fprintf(stdout, "           efi-variable-hardware-error-record\n");
					if (attributes & EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS)				fprintf(stdout, "           efi-variable-authenticated-write-access\n");
					if (attributes & EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS)	fprintf(stdout, "           efi-variable-time-based-authenticated-write-access\n");
					if (attributes & EFI_VARIABLE_APPEND_WRITE)								fprintf(stdout, "           efi-variable-append-write\n");

					fprintf(stdout, "\n");

					// > UTF-8 name.
					if (utf8_name)
					{
						bool	valid_utf = true;
						size_t	len = strlen(utf8_name);

						for (size_t i = 0; i < len && valid_utf; i++)
							valid_utf = isprint(utf8_name[i]);

						if (valid_utf)
							fprintf(stdout, "       Name (UTF-8): %s\n\n", utf8_name);
					}

					// > Specific keys handling.
					if (memcmp(&guid, &apple_nvram_variable_guid, sizeof(guid)) == 0 ||
						memcmp(&guid, &apple_screen_resolution_guid, sizeof(guid)) == 0 ||
						memcmp(&guid, &dhcpv6_service_binding_guid, sizeof(guid)) == 0)
					{
						if (utf8_name && strcmp(utf8_name, "csr-active-config") == 0 && value_size == 4)
						{
							const uint32_t	*csr_ptr = value;
							uint32_t 		csr = *csr_ptr;

							fprintf(stdout, "       CSR: 0x%x\n", csr);

							if (csr & CSR_ALLOW_UNTRUSTED_KEXTS)			fprintf(stdout, "           allow-untrusted-kexts\n");
							if (csr & CSR_ALLOW_UNRESTRICTED_FS)			fprintf(stdout, "           allow-unrestricted-fs\n");
							if (csr & CSR_ALLOW_TASK_FOR_PID)				fprintf(stdout, "           allow-task-for-pid\n");
							if (csr & CSR_ALLOW_KERNEL_DEBUGGER)			fprintf(stdout, "           allow-kernel-debugger\n");
							if (csr & CSR_ALLOW_APPLE_INTERNAL)				fprintf(stdout, "           allow-apple-internal\n");
							if (csr & CSR_ALLOW_UNRESTRICTED_DTRACE)		fprintf(stdout, "           allow-unrestricted-dtrace\n");
							if (csr & CSR_ALLOW_UNRESTRICTED_NVRAM)			fprintf(stdout, "           allow-unrestricted-nvram\n");
							if (csr & CSR_ALLOW_DEVICE_CONFIGURATION)		fprintf(stdout, "           allow-device-configuration\n");
							if (csr & CSR_ALLOW_ANY_RECOVERY_OS)			fprintf(stdout, "           allow-any-recovery-os\n");
							if (csr & CSR_ALLOW_UNAPPROVED_KEXTS)			fprintf(stdout, "           allow-unapproved-kexts\n");
							if (csr & CSR_ALLOW_EXECUTABLE_POLICY_OVERRIDE)	fprintf(stdout, "           allow-executable-policy-override\n");
							if (csr & CSR_ALLOW_UNAUTHENTICATED_ROOT)		fprintf(stdout, "           allow-unauthenticated-root\n");

							fprintf(stdout, "\n");
						}
					}

					// > Name.
					fprintf(stdout, "       Name:\n");
					SMDumpBytes(name, name_size, 11);
					fprintf(stdout, "\n");

					// > Value.
					fprintf(stdout, "       Value:\n");
					SMDumpBytes(value, value_size, 11);

					fprintf(stdout, "\n\n");
				}

				fprintf(stdout, "\n");
			}

			// > Show nvram sections content.
			else if (show_nvram)
			{
				size_t		bytes_size = 0;
				const void	*bytes = SMVMwareNVRAMEntryGetContentBytes(entry, &bytes_size);
				
				fprintf(stdout, " > Size : %lu\n", bytes_size);

				if (bytes_size > 0)
				{
					fprintf(stdout, " > Bytes:\n");

					SMDumpBytes(bytes, bytes_size, 3);
				}

				fprintf(stdout, "\n");
			}
		}
	}
	
	// Done.
	goto clean;
	
fail:
	result = EXIT_FAILURE;
	
	if (error)
		fprintf(stderr, "Error: %s.\n", SMErrorGetUserInfo(error));
	
clean:
	free(show_nvram_efi_variable);
	
	SMErrorFree(error);
	SMVMwareVMXFree(g_vmx);
	SMVMwareNVRAMFree(g_nvram);
	
	return result;
}

#pragma mark > Change

static int main_change(int argc, const char * argv[])
{
	int 			result = EXIT_SUCCESS;

	SMVMwareVMX		*g_vmx = NULL;
	SMVMwareNVRAM	*g_nvram = NULL;
	SMError			*error = NULL;
	
	char			*vmx_path_tmp_path = NULL;
	char			*nvram_path_tmp_path = NULL;
	
	// Check & extract vm path.
	const char *vm_path;
	
	if (argc < 2)
	{
		fprintf(stderr, "Error: missing vm path.\n");
		return EXIT_FAILURE;
	}
	
	vm_path = argv[1];
	
	argc--;
	argv++;

	// Handle options.
	int ch;

	typedef enum
	{
		SMMainChangeBootArgs,
		
		SMMainChangeCSREnable,
		SMMainChangeCSREnableVersion,

		SMMainChangeCSRDisable,
		SMMainChangeCSRDisableVersion,
		
		SMMainChangeCSRFlags,
		
		SMMainChangeMachineUUID,
		
		SMMainChangeScreenResolution,
	} SMMainChange;
	
	static struct option longopts[] = {
		{ "boot-args",      		required_argument,	NULL,	SMMainChangeBootArgs },
		
		{ "csr-enable",				no_argument,		NULL,	SMMainChangeCSREnable },
		{ "csr-enable-version",		required_argument,	NULL,	SMMainChangeCSREnableVersion },
		
		{ "csr-disable",			no_argument,		NULL,	SMMainChangeCSRDisable },
		{ "csr-disable-version",	required_argument,	NULL,	SMMainChangeCSRDisableVersion },
		
		{ "csr-flags",   			required_argument,	NULL,	SMMainChangeCSRFlags },
		
		{ "machine-uuid",   		required_argument,	NULL,	SMMainChangeMachineUUID },
		
		{ "screen-resolution",   	required_argument,	NULL,	SMMainChangeScreenResolution },

		{ NULL,         0,                      NULL,           0 }
	};
	
	while ((ch = getopt_long(argc, (char * const *)argv, "", longopts, NULL)) != -1)
	{
		switch (ch)
		{
			case SMMainChangeBootArgs:
			{
				// Change NVRAM.
				SMVMwareNVRAM *nvram = SMGetNVRAMFromVM(vm_path, &g_vmx, &g_nvram, &error);

				if (!nvram)
					goto fail;

				if (!SMVMwareNVRAMSetBootArgs(nvram, optarg, &error))
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
				if (ch == SMMainChangeCSREnableVersion || ch == SMMainChangeCSRDisableVersion)
				{
					macos_version = SMVersionFromString(optarg, &error);

					if (SMVersionIsEqual(macos_version, SMVersionInvalid))
						goto fail;
				}

				// Try to extract version from VMX.
				else if (ch == SMMainChangeCSREnable || ch == SMMainChangeCSRDisable)
				{
					// Get VMX.
					SMVMwareVMX *vmx = SMGetVMXFromVM(vm_path, &g_vmx, &error);

					if (!vmx)
						goto fail;

					// Extract macOS version.
					macos_version = SMVMwareVMXExtractMacOSVersion(vmx);

					if (SMVersionIsEqual(macos_version, SMVersionInvalid))
						fprintf(stderr, "Warning: can't detected macosVersion - use acceptable csr default behavior.\n");
					else
						fprintf(stderr, "Info: macOS version %d.%d.%d detected - use this version for csr behavior.\n", macos_version.major_version, macos_version.minor_version, macos_version.patch_version);
				}

				// Get NVRAM.
				SMVMwareNVRAM *nvram = SMGetNVRAMFromVM(vm_path, &g_vmx, &g_nvram, &error);
				
				if (!nvram)
					goto fail;

				// Change NVRAM.
				bool enable = (ch == SMMainChangeCSREnable || ch == SMMainChangeCSREnableVersion);
					
				if (!SMVMwareNVRAMSetAppleCSRActivation(nvram, macos_version, enable, &error))
					goto fail;
				
				break;
			}
				
			case SMMainChangeCSRFlags:
			{
				errno = 0;
				
				// Parse integer. Why those libc functions have to be so... chaotic bad...
				// I think I handled all errors possible from what I understand of the man page, but who know.
				char				*endp = NULL;
				unsigned long long	result = strtoull(optarg, &endp, 16);
				uint32_t 			new_csr = 0;
				
				if ((*optarg == 0) || !endp || *endp != 0 || ((result == ULONG_MAX || result == 0) && errno != 0) || result > UINT32_MAX)
				{
					fprintf(stderr, "Error: invalid value '%s'.\n", optarg);
					goto fail;
				}
				
				new_csr = (uint32_t)result;
				
				// Change NVRAM.
				SMVMwareNVRAM *nvram = SMGetNVRAMFromVM(vm_path, &g_vmx, &g_nvram, &error);
				
				if (!nvram)
					goto fail;
					
				if (!SMVMwareNVRAMSetAppleCSRActiveConfig(nvram, new_csr, &error))
					goto fail;
				
				break;
			}
				
			case SMMainChangeMachineUUID:
			{
				// Parse UUID.
				const char	*uuid_str = optarg;
				uuid_t		uuid;
				
				if (uuid_parse(uuid_str, uuid) == -1)
				{
					fprintf(stderr, "Error: invalid UUID '%s'.\n", uuid_str);
					goto fail;
				}
				
				// Change VMX.
				SMVMwareVMX *vmx = SMGetVMXFromVM(vm_path, &g_vmx, &error);
				
				if (!vmx)
					goto fail;
				
				if (!SMVMwareVMXSetMachineUUID(vmx, uuid, &error))
					goto fail;

				// Change NVRAM.
				SMVMwareNVRAM *nvram = SMGetNVRAMFromVM(vm_path, &g_vmx, &g_nvram, &error);
				
				if (!nvram)
					goto fail;
				
				if (!SMVMwareNVRAMSetAppleMachineUUID(nvram, uuid, &error))
					goto fail;
				
				break;
			}
				
			case SMMainChangeScreenResolution:
			{
				// Parse argument.
				unsigned int	width = 0, height = 0;
				size_t			optarg_len = strlen(optarg);
				int				scan_len = 0;
				int				sresult = sscanf(optarg, "%ux%u%n", &width, &height, &scan_len);
				
				if (sresult != 2 || scan_len != optarg_len)
				{
					fprintf(stderr, "Error: invalid screen resolution - check usage.\n");
					goto fail;
				}
				
				// Apply some arbitrary constraints, something which doesn't look too crazy or risky.
				// This software is open-source, anyone can change them if they don't agree...
				// Anyway, VMware Fusion (or macOS ?) fix them if they are not valid.
				if (width < 100 || width > 50000)
				{
					fprintf(stderr, "Error: width should be in the range of [100;50000].\n");
					goto fail;
				}
				
				if (height < 100 || height > 50000)
				{
					fprintf(stderr, "Error: height should be in the range of [100;50000].\n");
					goto fail;
				}
				
				// Change NVRAM.
				SMVMwareNVRAM *nvram = SMGetNVRAMFromVM(vm_path, &g_vmx, &g_nvram, &error);
				
				if (!nvram)
					goto fail;
				
				if (!SMVMwareNVRAMSetScreenResolution(nvram, width, height, &error))
					goto fail;
				
				break;
			}
				
			default:
				fprintf(stderr, "Error: invalid option - check usage.\n");
				goto fail;
		}
	}
	
	argc -= optind;
	argv += optind;
	
	if (argc != 0)
	{
		fprintf(stderr, "Error: invalid extra parameters - check usage.\n");
		goto fail;
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
		fprintf(stderr, "Error: failed to replace vmx file ('%s' -> '%s').\n", vmx_path_tmp_path, vmx_path);
		goto fail;
	}
	
	if (nvram_path && nvram_path_tmp_path && rename(nvram_path_tmp_path, nvram_path) == -1)
	{
		fprintf(stderr, "Error: failed to replace nvram file ('%s' -> '%s').\n", nvram_path_tmp_path, nvram_path);
		goto fail;
	}
	
	// Finish.
	fprintf(stderr, "Virtual machine configuration changed with success.\n");
	
	goto clean;
	
fail:
	result = EXIT_FAILURE;
	
	if (error)
		fprintf(stderr, "Error: %s.\n", SMErrorGetUserInfo(error));
	
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
		SMSetErrorPtr(error, "main", -1, "can't open vm bundle");
		goto finish;
	}
	
	// Search vmx file.
	struct dirent *dp;
	
	while ((dp = readdir(dir)) != NULL)
	{
		if (!SMStringPathHasExtension(dp->d_name, "vmx"))
			continue;
		
		char *path = SMStringPathAppendComponent(vm_path, dp->d_name);
		
		
		result = SMVMwareVMXCreate(path, error);
		
		free(path);
		
		break;
	}
	
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
		
	result = SMVMwareNVRAMCreate(path, error);
	
	free(path);
	
finish:
	
	if (result)
		*inoutNVRAM = result;
	
	return result;
}


#pragma mark > Output

static void SMDumpBytes(const void *bytes, size_t size, size_t padding)
{
#define SMDumpLineBytes 25

	const uint8_t *ubytes = bytes;

	// Forge padding string.
	char *padding_str;
	
	padding_str = malloc(padding + 1);
	memset(padding_str, ' ', padding);
	padding_str[padding] = 0;
		
	// Print lines.
	size_t total_bytes_handled = 0;

	while (total_bytes_handled < size)
	{
		size_t line_bytes_size = MIN(SMDumpLineBytes, size - total_bytes_handled);
		
		// Print padding.
		fputs(padding_str, stdout);
		
		// Print bytes.
		for (size_t i = 0; i < line_bytes_size; i++)
			fprintf(stdout, "%02x ", ubytes[total_bytes_handled + i]);
		
		// Align bytes.
		for (size_t i = 0; i < SMDumpLineBytes - line_bytes_size ; i++)
			fputs("   ", stdout);

		fprintf(stdout, "| ");
		
		// Print ASCII.
		for (size_t i = 0; i < line_bytes_size; i++)
		{
			uint8_t byte = ubytes[total_bytes_handled + i];
			
			fprintf(stdout, "%c", isprint(byte) ? byte : '.' );
		}
		
		fprintf(stdout, "\n");
		
		total_bytes_handled += line_bytes_size;
	}
	
	// Clean.
	free(padding_str);
}
