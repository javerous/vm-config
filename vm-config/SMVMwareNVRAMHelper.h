/*
 *  SMVMwareNVRAMHelper.h
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

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <uuid/uuid.h>

#include "SMVMwareNVRAM.h"

#include "SMError.h"

#include "SMVersion.h"


/*
** Defines
*/
#pragma mark - Defines

// EFI GUIDs.
#define Efi_Global_Variable_Guid		(efi_guid_t){ 0x8BE4DF61, 0x93CA, 0x11d2, { 0xAA, 0x0D, 0x00, 0xE0, 0x98, 0x03, 0x2B, 0x8C } }
#define Apple_Vendor_Guid				(efi_guid_t){ 0xAC39C713, 0x7E50, 0x423D, { 0x88, 0x9D, 0x27, 0x8F, 0xCC, 0x34, 0x22, 0xB6 } }
#define Apple_Firmware_Variable_Guid	(efi_guid_t){ 0x4D1EDE05, 0x38C7, 0x4A6A, { 0x9C, 0xC6, 0x4B, 0xCC, 0xA8, 0xB3, 0x8C, 0x14 } }
#define Apple_NVRAM_Variable_Guid		(efi_guid_t){ 0x7C436110, 0xAB2A, 0x4BBB, { 0xA8, 0x80, 0xFE, 0x41, 0x99, 0x5C, 0x9F, 0x82 } }
#define Apple_Screen_Resolution_Guid	(efi_guid_t){ 0xAC20C489, 0xDD86, 0x4E99, { 0x99, 0x2C, 0xB7, 0xC7, 0x42, 0xC1, 0xDD, 0xA9 } }

#define DHCPv6_Service_Binding_Guid		(efi_guid_t){ 0x9FB9A8A1, 0x2F4A, 0x43A6, { 0x88, 0x9C, 0xD0, 0xF7, 0xB6, 0xC4, 0x7A, 0xD5 } }


// EFI Variables Names.
// > 7C436110-AB2A-4BBB-A880-FE41995C9F82 / Apple_Vendor_Guid.
#define SMEFIAppleNVRAMVarEFIApplePayload0DataName				"efi-apple-payload0-data"			// Bytes (opaque).
#define SMEFIAppleNVRAMVarEFIApplePayload0Name					"efi-apple-payload0"				// C-String (Property List).
#define SMEFIAppleNVRAMVarEFIApplePayload1DataName				"efi-apple-payload1-data"			// Bytes (opaque).
#define SMEFIAppleNVRAMVarEFIApplePayload1Name					"efi-apple-payload1" 				// C-String (Property List).
#define SMEFIAppleNVRAMVarEFIApplePayload2DataName				"efi-apple-payload2-data"			// Bytes (opaque).
#define SMEFIAppleNVRAMVarEFIApplePayload3Name 					"efi-apple-payload2"				// C-String (Property List)
#define SMEFIAppleNVRAMVarEFIAppleRecoveryName					"efi-apple-recovery"				// C-String (Property List).
#define SMEFIAppleNVRAMVarEFIBackupBootDeviceDataName			"efi-backup-boot-device-data"		// Bytes (opaque).
#define SMEFIAppleNVRAMVarEFIBackupBootDeviceName				"efi-backup-boot-device"			// String (Property List).
#define SMEFIAppleNVRAMVarEFIBootDeviceDataName					"efi-boot-device-data"				// Bytes (opaque).
#define SMEFIAppleNVRAMVarEFIBootDeviceName						"efi-boot-device"					// C-String (Property List).

#define SMEFIAppleNVRAMVarHWBootDataName						"HW_BOOT_DATA"						// Bytes (opaque).

#define SMEFIAppleNVRAMVarAutoBootName							"auto-boot"							// String ("true" / "false").
#define SMEFIAppleNVRAMVarBacklightLevelName					"backlight-level"					// uint16_t ?
#define SMEFIAppleNVRAMVarBootArgsName							"boot-args"							// C-String.
#define SMEFIAppleNVRAMVarCSRActiveConfigName					"csr-active-config"					// uint32_t flags.
#define SMEFIAppleNVRAMVarEOSFDRCacheUUIDName					"eos-fdr-cache-uuid"				// String (UUID).
#define SMEFIAppleNVRAMVarEOSRestoreFailureUUIDsName			"eos-restore-failure-uuids"			// String (UUID).
#define SMEFIAppleNVRAMVarFMMComputerName						"fmm-computer-name"					// String (Find My Mac Computer Name).
#define SMEFIAppleNVRAMVarGPUPolicyName							"gpu-policy"						// uint8_t ?
#define SMEFIAppleNVRAMVarKDPIPStrName							"_kdp_ipstr"						// String.
#define SMEFIAppleNVRAMVarMultiAdapaterStateName				"multiupdater-state"				// Bytes (opaque).
#define SMEFIAppleNVRAMVarMultiAdapterRetryLimitsName			"multiupdater-retry-limits"			// uin64_t ?
#define SMEFIAppleNVRAMVarOTAControllerVersionName				"ota-controllerVersion"				// C-String.
#define SMEFIAppleNVRAMVarOTAUpdateTypeName						"ota-updateType"					// C-String.
#define SMEFIAppleNVRAMVarPlatformUUIDName						"platform-uuid"						// Bytes (UUID)
#define SMEFIAppleNVRAMVarPreferredCountName					"preferred-count"					// uint8_t ?
#define SMEFIAppleNVRAMVarPreferredNetworksName					"preferred-networks"				// Bytes (opaque).
#define SMEFIAppleNVRAMVarPrevLangDiagsName						"prev-lang-diags:kbd"				// C-String (ISO 639-1 ?).
#define SMEFIAppleNVRAMVarPrevLangName  						"prev-lang:kbd"						// String (ISO 639-1 ? e.g. "en-GB:1").
	
#define SMEFIAppleNVRAMVarBluetoothActiveControllerInfoName		"bluetoothActiveControllerInfo"		// Bytes (opaque).
#define SMEFIAppleNVRAMVarBluetoothExternalDongleFailedName		"bluetoothExternalDongleFailed" 	// uint8_t (boolean ?).
#define SMEFIAppleNVRAMVarBluetoothInternalControllerInfoName	"bluetoothInternalControllerInfo"	// Bytes (opaque).
#define SMEFIAppleNVRAMVarLocationServicesEnabledName			"LocationServicesEnabled"			// uint8_t (boolean ?)
#define SMEFIAppleNVRAMVarPanicInfoLogName						"AAPL,PanicInfoLog"					// Bytes (opaque)
#define SMEFIAppleNVRAMVarPanicInfoPrefixName					"AAPL,PanicInfo"					// Bytes (opaque) (name is a prefix, they are formated like "AAPL,PanicInfo0000", "AAPL,PanicInfo0001", etc.
#define SMEFIAppleNVRAMVarSystemAudioVolumeDBName				"SystemAudioVolumeDB"				// uint8_t ?
#define SMEFIAppleNVRAMVarSystemAudioVolumeName					"SystemAudioVolume"					// uint8_t (ASCII of the digit of the volume, between '0' and '9' ?)
#define SMEFIAppleNVRAMVarThorUpdateResultName 					"ThorUpdateResult"					// uint64_t ?

// > 9FB9A8A1-2F4A-43A6-889C-D0F7B6C47AD5 / DHCPv6_Service_Binding_Guid.
#define SMEFIVarDHCPv6ClientIDName	"ClientId"

// > AC20C489-DD86-4E99-992C-B7C742C1DDA9 / Apple_Screen_Resolution_Guid.
#define SMEFIVarAppleScreenResolutionHeightName	"height"	// uin32_t (pixels).
#define SMEFIVarAppleScreenResolutionWidthName	"width"		// uin32_t (pixels).


// Configurable Security Restrictions (CSR) flags - from bsd/sys/csr.h.
#define CSR_ALLOW_UNTRUSTED_KEXTS               (1 << 0)
#define CSR_ALLOW_UNRESTRICTED_FS               (1 << 1)
#define CSR_ALLOW_TASK_FOR_PID                  (1 << 2)
#define CSR_ALLOW_KERNEL_DEBUGGER               (1 << 3)
#define CSR_ALLOW_APPLE_INTERNAL                (1 << 4)
#define CSR_ALLOW_DESTRUCTIVE_DTRACE            (1 << 5) /* name deprecated */
#define CSR_ALLOW_UNRESTRICTED_DTRACE           (1 << 5)
#define CSR_ALLOW_UNRESTRICTED_NVRAM            (1 << 6)
#define CSR_ALLOW_DEVICE_CONFIGURATION          (1 << 7)
#define CSR_ALLOW_ANY_RECOVERY_OS               (1 << 8)
#define CSR_ALLOW_UNAPPROVED_KEXTS              (1 << 9)
#define CSR_ALLOW_EXECUTABLE_POLICY_OVERRIDE    (1 << 10)
#define CSR_ALLOW_UNAUTHENTICATED_ROOT          (1 << 11)


/*
** Functions
*/
#pragma mark - Functions

// CSR Get/Set.
bool SMVMwareNVRAMGetAppleCSRActiveConfig(SMVMwareNVRAM *nvram, uint32_t *csr, SMError **error);
bool SMVMwareNVRAMSetAppleCSRActiveConfig(SMVMwareNVRAM *nvram, uint32_t csr, SMError **error);

// CSR Activation.
bool SMVMwareNVRAMSetAppleCSRActivation(SMVMwareNVRAM *nvram, SMVersion macos_version, bool enable, SMError **error); // Mimate "csrutil enable" / "csrutil disable".

// UUID.
bool SMVMwareNVRAMGetApplePlatformUUID(SMVMwareNVRAM *nvram, uuid_t uuid, SMError **error);
bool SMVMwareNVRAMSetApplePlatformUUID(SMVMwareNVRAM *nvram, uuid_t uuid, SMError **error);

bool SMVMwareNVRAMSetAppleMachineUUID(SMVMwareNVRAM *nvram, uuid_t uuid, SMError **error);

// Boot Args.
bool SMVMwareNVRAMSetBootArgs(SMVMwareNVRAM *nvram, const char *boot_args, SMError **error);
