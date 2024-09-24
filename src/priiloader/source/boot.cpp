/*
 * Copyright (C) 2017 FIX94
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
//modified in 2024 for HAI-Priiloader
#include "boot.h"
#include "font.h"
#include "video.h"
#include <stdio.h>
#include <ogcsys.h>
#include <locale.h>
#include <ogc/isfs.h>
#include <string.h>
#include <malloc.h>
#include "fs.h"
u32 AppEntrypoint;
//used externally
u8 *tmdBuffer = NULL;
u32 tmdSize = 0;


#define ALIGN32(x) (((x) + 31) & ~31)

static fstats stats ATTRIBUTE_ALIGN(32);

//PrintFormat does not work when in a seperate file
//moving here is easier than resolving an auxiliary bug
u8 *ISFS_GetFile(const char *path, u32 *size, s32 length)
{
	*size = 0;
	//printf("ISFS_GetFile %s", path);
	//ISFS_Open did not work on HAI-IOS for some reason; trying IOS_Open instead
	s32 fd = ISFS_Open(path, ISFS_OPEN_READ);
	//s32 fd = IOS_Open(path, 0);
	u8 *buf = NULL;
	sleep(3);
	if (fd >= 0)
	{
		memset(&stats, 0, sizeof(fstats));
		s32 temp = ISFS_GetFileStats(fd, &stats);
		if(temp >= 0)
		{
			if(length <= 0)
				length = stats.file_length;
			if(length > 0)
				buf = (u8*)memalign(32, ALIGN32(length));
			if(buf)
			{
				*size = stats.file_length;
				temp = ISFS_Read(fd, (char*)buf, length);
				if( temp != length)
				{
					PrintFormat(0,16,32,"ISFS_Read returned %d", temp);
					sleep(3);
					*size = 0;
					free(buf);
				}
			}
			
			else
			{
				PrintFormat(0,16,32, "buf is 0");
				sleep(3);
			}
		}
		else
		{
			//currently returns -102 for permission denied
			PrintFormat(0,16,32, "ISFS_GetFileStats returned %d", temp);
			sleep(3);
		}
		ISFS_Close(fd);
	}
	else
	{
		PrintFormat(0,16,32, "IOS_Open returned file descriptor %d", fd);
	}
	//if(*size > 0)
	//	printf(" succeed!\n");
	//else
	//	printf(" failed!\n");
	return buf;
}

static inline bool apply_patch(const char *name, const u8 *old, const u8 *patch, u32 size)
{
	u8 i;
	u32 found = 0;
	u8 *ptr = (u8*)0x93400000;

	u32 level = IRQ_Disable();
	while((u32)ptr < (u32)0x94000000)
	{
		if(memcmp(ptr, old, size) == 0)
		{
			for(i = 0; i < size; ++i)
				*(vu8*)(ptr+i) = *(vu8*)(patch+i);
			found++;
		}
		ptr++;
	}
	IRQ_Restore(level);

	//printf("patched %s %lu times.\n", name, found);
	return (found > 0);
}



extern "C" { extern void __exception_closeall(); };

int bootChannel(u64 title, bool boot4by3)
{
	
    
    static const u8 isfs_perm_wiivc_old[] = { 0x42, 0x9F, 0xD1, 0x03, 0x20, 0x00, 0xBD, 0xF0, 0x09, 0x8B, 0xE7, 0xF8, 0x20, 0x66 };
	static const u8 isfs_perm_wiivc_patch[] = { 0x42, 0x9F, 0x46, 0xC0, 0x20, 0x00, 0xBD, 0xF0, 0x09, 0x8B, 0xE7, 0xF8, 0x20, 0x66 };
	static const u8 setuid_old[] = { 0xD1, 0x2A, 0x1C, 0x39 };
	static const u8 setuid_patch[] = { 0x46, 0xC0, 0x1C, 0x39 };
	static const u8 es_identify_old[] = { 0x28, 0x03, 0xD1, 0x23 };
	static const u8 es_identify_patch[] = { 0x28, 0x03, 0x00, 0x00 };

	//patches fail to be applied dynamically
	//they are instead statically applied to fw.img
	
	bool success = apply_patch("isfs_permissions_wiivc", isfs_perm_wiivc_old, isfs_perm_wiivc_patch, sizeof(isfs_perm_wiivc_patch));
	if (!success)
	{
		PrintFormat(0,16,48,"failed to patch isfs_permissions_wiivc");
		sleep(3);
	}
	//PrintFormat( 0, 16, rmode->viHeight-72, "patched isfs permissions ");
	success = apply_patch("es_setuid", setuid_old, setuid_patch, sizeof(setuid_patch));
	if (!success)
	{
		PrintFormat(0,16,48,"failed to patch es_setuid");
		sleep(3);
	}
	//PrintFormat( 0, 16, rmode->viHeight-72, "patched setuid ");
	//this is running as the system menu so es_identify should nopt be necessary
	success = apply_patch("es_identify", es_identify_old, es_identify_patch, sizeof(es_identify_patch));
	if (!success)
	{
		PrintFormat(0,16,48,"failed to patch es_identify");
		sleep(3);
	}
	//PrintFormat( 0, 16, rmode->viHeight-72, "patched es_identify ");

	//ISFS_Initialize();
	
	u8 reqIOS = 0;

	char tmd[ISFS_MAXPATH] ATTRIBUTE_ALIGN(32);
	u32 title_upper = (u32)((title) >> 32);
	u32 title_lower = (u32)(title & 0xffffffff);
	sprintf(tmd, "/title/%08lx/%08lx/content/title.tmd", title_upper, title_lower);
	PrintFormat( 0, 16, rmode->viHeight-90, tmd);
	tmdBuffer = ISFS_GetFile(tmd, &tmdSize, -1);

	/*
	if(tmdBuffer == NULL)
	{
		int maxAttempts = 10;
		int i;
		for (i = 1; i < maxAttempts; i++)
		{
			PrintFormat( 0, 16, rmode->viHeight-108, "No Title TMD! on attempt %d %d", i, exists);
			//printf("No Title TMD!\n");
			tmdBuffer = ISFS_GetFile(tmd, &tmdSize, -1);
			if (tmdBuffer != NULL)
			exists = IOS_Open(tmd, 1);
			IOS_Close(exists);
			ISFS_Deinitialize();
			s32 r = ISFS_Initialize();
			if( r < 0 )
			{
				*(vu32*)0xCD8000C0 |= 0x20;
				PrintFormat( 0, 16, rmode->viHeight-116, "FS failed to initialize");
			}
			sleep(1);
		}
		if (i == 10)
		{
			ISFS_Initialize();
			return 0;
		}
		
	}
	*/
	sleep(1);
	if(tmdSize < 0x1E4)
	{
		PrintFormat( 0, 16, rmode->viHeight-108, "Too small Title TMD!");
		free(tmdBuffer);
		printf("Too small Title TMD!\n");
		ISFS_Deinitialize();
		sleep(3);
		return 0;
	}

	reqIOS = tmdBuffer[0x18B];
	//printf("Requested Game IOS: %i\n", reqIOS);
	IOS_ReloadIOS(reqIOS);

	//this is running as the system menu so es_identify is not needed
	/*
	if(!DoESIdentify())
	{
		//free(tmdBuffer);
		printf("DoESIdentify failed!\n");
		PrintFormat( 0, 16, rmode->viHeight-108, "DoESIdentify failed! ");
		//ISFS_Deinitialize();
		sleep(3);
		//return 0;
	}
	*/
	

	AppEntrypoint = LoadChannel(title, false);
	PrintFormat( 0, 16, rmode->viHeight-126, "Entrypoint: %08lx AHBPROT: %08lx\n", AppEntrypoint, *(vu32*)0xCD800064);
	//printf("Entrypoint: %08lx AHBPROT: %08lx\n", AppEntrypoint, *(vu32*)0xCD800064);
	sleep(3);
	free(tmdBuffer);

    if (boot4by3)
	{
		write32(0xd8006a0, 0x30000002);
		mask32(0xd8006a8, 0, 2);
	}
    
    //no video configuration within priiloader
	/* Set black and flush */
    
	VIDEO_SetBlack(TRUE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();
	else while(VIDEO_GetNextField())
		VIDEO_WaitVSync();
    
	/* Shutdown IOS subsystems */
	ISFS_Deinitialize();
	u32 level = IRQ_Disable();
	__IOS_ShutdownSubsystems();
	__exception_closeall();

	/* Originally from tueidj - taken from NeoGamma (thx) */
	*(vu32*)0xCC003024 = 1;

	if(AppEntrypoint == 0x3400)
	{
		asm volatile (
			"isync\n"
			"lis %r3, AppEntrypoint@h\n"
			"ori %r3, %r3, AppEntrypoint@l\n"
			"lwz %r3, 0(%r3)\n"
			"mtsrr0 %r3\n"
			"mfmsr %r3\n"
			"li %r4, 0x30\n"
			"andc %r3, %r3, %r4\n"
			"mtsrr1 %r3\n"
			"rfi\n"
		);
	}
	else
	{
		asm volatile (
			"lis %r3, AppEntrypoint@h\n"
			"ori %r3, %r3, AppEntrypoint@l\n"
			"lwz %r3, 0(%r3)\n"
			"mtlr %r3\n"
			"blr\n"
		);
	}
	IRQ_Restore(level);

	return 0;
}