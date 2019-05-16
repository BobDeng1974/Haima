#include <common.h>
#include <usb.h>
#include <config.h>
#include <part.h>
#include <ext4fs.h>
#include <fat.h>
#include <fs.h>
#include <sandboxfs.h>
#include <asm/io.h>
#include <div64.h>
#include <linux/math64.h>
#include <mmc.h>
#include <cli.h>
#include <command.h>
#include <environment.h>
#include <search.h>
#include <errno.h>
#include <malloc.h>
#include <linux/stddef.h>
#include <asm/byteorder.h>
#include <asm/io.h>


#define AU_FIRMWARE "/UPDATE/uboot_update/u-boot.imx"   
#define AU_KERNEL   "/UPDATE/uboot_update/zImage"
#define AU_DTB      "/UPDATE/uboot_update/imx6q-sabresd.dtb"
#define AU_ROOTFS   "/UPDATE/uboot_update/rootfs.ext4"

#define tAU_FIRMWARE "u-boot.imx"   
#define tAU_KERNEL   "zImage"
#define tAU_DTB      "imx6q-sabresd.dtb"
#define tAU_ROOTFS   "rootfs.ext4"

typedef enum
{
    IDX_KERNEL = 0,
    IDX_DTB,
    IDX_ROOTFS,
    IDX_FIRMWARE,
    IDX_MAXFILES,
}IMAGE_INDEX;

char *aufile[IDX_MAXFILES] = {
    
    AU_KERNEL,
    AU_DTB,
    AU_ROOTFS,
    AU_FIRMWARE,
};

char *filename[IDX_MAXFILES] = {
    
    tAU_KERNEL,
    tAU_DTB,
    tAU_ROOTFS,
    tAU_FIRMWARE,
};




#define LOADE_ADDR     0x12000000      //文件下载到内存的地址
#define BLOCK_SIZE          5*1024*1024     //一次读�?MB
#define ROOTFS_ADDR_BASE    1228800         //文件系统分区的起始地址

static int curr_device = 2;
static struct mmc *init_mmc_device(int dev, bool force_init)
{
	struct mmc *mmc;
	mmc = find_mmc_device(dev);
	if (!mmc) {
		printf("no mmc device at slot %x\n", dev);
		return NULL;
	}
	if (force_init)
		mmc->has_init = 0;
	if (mmc_init(mmc))
		return NULL;
	return mmc;
}

int emmc_erase(u32 block, u32 count)
{
    struct mmc *mmc;
    u32 blk, cnt, n;

    blk = block;
    cnt = count;
    mmc = init_mmc_device(curr_device, false);
    if (!mmc)
        return CMD_RET_FAILURE;

    printf("\nMMC erase: dev # %d, block # %d, count %d ... ",curr_device, blk, cnt);

    if (mmc_getwp(mmc) == 1) 
    {
        printf("Error: card is write protected!\n");
        return CMD_RET_FAILURE;
     
    }
    n = mmc->block_dev.block_erase(curr_device, blk, cnt);
    printf("%d blocks erased: %s\n", n, (n == cnt) ? "OK" : "ERROR");                   
        return (n == cnt) ? CMD_RET_SUCCESS : CMD_RET_FAILURE;
}

static int emmc_write(ulong load_addr, u32 block, u32 count)
{
	struct mmc *mmc;
	u32 blk, cnt, n;
	void *addr;

	addr = (void *)load_addr;
	blk = block ;
	cnt = count;

	mmc = init_mmc_device(curr_device, false);
	if (!mmc)
		return CMD_RET_FAILURE;

	printf("\nMMC write: dev # %d, block # %d, count %d ... ",
	       curr_device, blk, cnt);

	if (mmc_getwp(mmc) == 1) {
		printf("Error: card is write protected!\n");
		return CMD_RET_FAILURE;
	}
	n = mmc->block_dev.block_write(curr_device, blk, cnt, addr);
	printf("%d blocks written: %s\n", n, (n == cnt) ? "OK" : "ERROR");

	return (n == cnt) ? CMD_RET_SUCCESS : CMD_RET_FAILURE;
}

#if 0
static int env_id = 1;
static int env_set(int flag, int argc, char * const argv[])
{
	int   i, len;
	char  *name, *value, *s;
	ENTRY e, *ep;
	int env_flag = H_INTERACTIVE;

	printf("Initial value for argc=%d\n", argc);
	debug("Initial value for argc=%d\n", argc);
	while (argc > 1 && **(argv + 1) == '-') {
		char *arg = *++argv;

		--argc;
		while (*++arg) {
			switch (*arg) {
			case 'f':		/* force */
				env_flag |= H_FORCE;
				break;
			default:
				return CMD_RET_USAGE;
			}
		}
	}
	printf("[liuh>>>>>>>>]Final value for argc=%d\n", argc);
    printf("[liuh>>>>>>>>]argv[0] = %s, argv[1] = %s, argv[2] = %s\n", argv[0], argv[1], argv[2]);
	debug("Final value for argc=%d\n", argc);
	name = argv[1];
	value = argv[2];
    printf("argv[1] = %s, argv[2] = %s\n", argv[1], argv[2]);

	if (strchr(name, '=')) {
		printf("## Error: illegal character '='"
		       "in variable name \"%s\"\n", name);
		return 1;
	}

	env_id++;

	/* Delete only ? */
	if (argc < 3 || argv[2] == NULL) {
		int rc = hdelete_r(name, &env_htab, env_flag);
		return !rc;
	}

	/*
	 * Insert / replace new value
	 */
	for (i = 2, len = 0; i < argc; ++i)
		len += strlen(argv[i]) + 1;

	value = malloc(len);
	if (value == NULL) {
		printf("## Can't malloc %d bytes\n", len);
		return 1;
	}
	for (i = 2, s = value; i < argc; ++i) {
		char *v = argv[i];

		while ((*s++ = *v++) != '\0')
			;
		*(s - 1) = ' ';
	}
	if (s != value)
		*--s = '\0';

	e.key	= name;
	e.data	= value;
	hsearch_r(e, ENTER, &ep, &env_htab, env_flag);
	free(value);
	if (!ep) {
		printf("## Error inserting \"%s\" variable, errno=%d\n",
			name, errno);
		return 1;
	}

	return 0;
}
static inline int mmc_set_env_part(struct mmc *mmc) {return 0; };
static int init_mmc_for_env(struct mmc *mmc)
{
	if (!mmc) {
		puts("No MMC card found\n");
		return -1;
	}

	if (mmc_init(mmc)) {
		puts("MMC init failed\n");
		return -1;
	}

	return mmc_set_env_part(mmc);
}

static void fini_mmc_for_env(struct mmc *mmc)
{
}
static inline int write_env(struct mmc *mmc, unsigned long size,
			    unsigned long offset, const void *buffer)
{
	uint blk_start, blk_cnt, n;

	blk_start	= ALIGN(offset, mmc->write_bl_len) / mmc->write_bl_len;
	blk_cnt		= ALIGN(size, mmc->write_bl_len) / mmc->write_bl_len;

	n = mmc->block_dev.block_write(mmc_get_env_devno(), blk_start,
					blk_cnt, (u_char *)buffer);

	return (n == blk_cnt) ? 0 : -1;
}
int auto_saveenv(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(env_t, env_new, 1);
	int mmc_env_devno = mmc_get_env_devno();
	struct mmc *mmc = find_mmc_device(mmc_env_devno);
	u32	offset;
	int	ret, copy = 0;

	if (init_mmc_for_env(mmc))
		return 1;

	ret = env_export(env_new);
	if (ret)
		goto fini;

#ifdef CONFIG_ENV_OFFSET_REDUND
	env_new->flags	= ++env_flags; /* increase the serial */

	if (gd->env_valid == 1)
		copy = 1;
#endif

	if (mmc_get_env_addr(mmc, copy, &offset)) {
		ret = 1;
		goto fini;
	}

	printf("Writing to %sMMC(%d)... ", copy ? "redundant " : "",
	       mmc_env_devno);
	if (write_env(mmc, CONFIG_ENV_SIZE, offset, (u_char *)env_new)) {
		puts("failed\n");
		ret = 1;
		goto fini;
	}

	puts("done\n");
	ret = 0;

#ifdef CONFIG_ENV_OFFSET_REDUND
	gd->env_valid = gd->env_valid == 2 ? 1 : 2;
#endif

fini:
	fini_mmc_for_env(mmc);
	return ret;
}
#endif

int checkout_usb(void)
{
    int usb_dev = -1;
    bootstage_mark_name(BOOTSTAGE_ID_USB_START, "usb_start");
    if (usb_init() < 0)
        return -1;
    usb_dev = usb_stor_scan(1);
    return usb_dev;
}

int auto_fat_fsload(const char* ifname, const char* dev_part_str, const char* paddr,  const char* filename, loff_t pos, loff_t bytes)
{
	unsigned long addr;
	const char *file;
	loff_t len_read;
	int ret;
	char *ep;

	if (fs_set_blk_dev(ifname, dev_part_str, FS_TYPE_FAT))
		return 1;

	addr = simple_strtoul(paddr, &ep, 16);
	file = filename;

	ret = fs_read(file, addr, pos, bytes, &len_read);
	if (ret < 0)
		return -1;

	return len_read;
}


static int auto_file_size(const char* ifname, const char*dev_part_str, const char* filename)
{
    loff_t size;
    if (fs_set_blk_dev(ifname, dev_part_str, FS_TYPE_FAT))
        return -1;

    if (fs_size(filename, &size) < 0) 
        return -1;

    return size;
}

static int auto_fat_fswrite(const char* ifname, const char* dev_part_str, const char* paddr,  const char* filename, unsigned long  len)
{
	loff_t size;
	int ret;
	unsigned long addr;
	unsigned long count;
	block_dev_desc_t *dev_desc = NULL;
	disk_partition_t info;
	int dev = 0;
	int part = 1;
	void *buf;

	part = get_device_and_partition(ifname, dev_part_str, &dev_desc, &info, 1);
	if (part < 0)
		return 1;
	dev = dev_desc->dev;

	if (fat_set_blk_dev(dev_desc, &info) != 0) {
		printf("\n** Unable to use %s %d:%d for fatwrite **\n",
			ifname, dev, part);
		return 1;
	}
	addr = simple_strtoul(paddr, NULL, 16);
	count = len;

	buf = map_sysmem(addr, count);
	ret = file_fat_write(filename, buf, 0, count, &size);
	unmap_sysmem(buf);
	if (ret < 0) {
		printf("\n** Unable to write \"%s\" from %s %d:%d **\n",
			filename, ifname, dev, part);
		return 1;
	}

	printf("%llu bytes written\n", size);

	return size;
}

int file_size_to_blk(int size)
{
    return size/(5*1024*1024) + (size%(5*1024*1024) == 0? 0:1);
}

int do_auto_update(void)
{
    int i = 0;
    int j = 0;
    int ret = -1;
    int usb_stor_curr_dev = -1;
    int write_len = 0;
    int file_size = 0;
    int file_blk = 0;
    int pos = 0;
    int emmc_addr = ROOTFS_ADDR_BASE;
    int emmc_blk_cnt = BLOCK_SIZE / 512;
    usb_stor_curr_dev = checkout_usb();
    if(usb_stor_curr_dev != 0)
    {
        printf("not found usb\n");
        return -1;
    }
    
    for(i=0; i<IDX_MAXFILES; i++)
    {
        if(i == IDX_ROOTFS)
        {	
        	
            file_size = auto_file_size("usb", "0", aufile[i]);
			if(file_size < 102400)                                                                                                                                                                                
			{   
			     printf("not found %s\n", filename[i]);
			     continue;
			}
			if(run_command("mmc dev 2 0",0) != 0){  //访问emmc UDA分区
				printf("switch mcc part failed!\n");
				continue;
			};
            file_blk = file_size_to_blk(file_size);
            //printf("[liuh>>>>>>>>>>]file_sie = %d, file_blk = %d\n", file_size, file_blk);
            for(j=0; j<file_blk; j++)
            {
                if(j == (file_blk-1))//最后一包
                {	
                	loff_t bytes = file_size%BLOCK_SIZE;
                	if(bytes == 0){
						bytes = BLOCK_SIZE;
					}
					
                    auto_fat_fsload("usb", "0", "12000000", aufile[i], pos, bytes);

                    emmc_write(LOADE_ADDR, emmc_addr, emmc_blk_cnt);

                }
                else
                {
                    auto_fat_fsload("usb", "0", "12000000", aufile[i], pos, BLOCK_SIZE);
                    pos += BLOCK_SIZE;

                    emmc_write(LOADE_ADDR, emmc_addr, emmc_blk_cnt);
                    emmc_addr += emmc_blk_cnt;
                }
            }
        }
		else if(i ==  IDX_FIRMWARE){

			file_size = auto_file_size("usb", "0", aufile[i]);
			if(file_size <= 0){
				printf("not found %s\n", filename[i]);
				continue;
			}
			printf("%s size is %d\n", filename[i], file_size);
			
			auto_fat_fsload("usb", "0", "12000000", aufile[i], 0, file_size);
         
			if(run_command("mmc dev 2 1",0) != 0){  //访问emmc boot0分区
				printf("switch mcc part failed!\n");
				continue;
			};
			char buffer[50];
			sprintf(buffer,"mmc write 12000000 2 %x",file_size/512 + ((file_size%512)?1:0));
			run_command(buffer,0);
		}
        else {
			
            file_size = auto_file_size("usb", "0", aufile[i]);
			if(file_size <= 0){
				printf("not found %s\n", filename[i]);
				continue;
			}
			printf("%s size is %d\n", filename[i], file_size);
			
			auto_fat_fsload("usb", "0", "12000000", aufile[i], 0, file_size);
			if(run_command("mmc dev 2 0",0) != 0){  //访问emmc UDA分区
				printf("switch mcc part failed!\n");
				continue;
			};
            write_len = auto_fat_fswrite("mmc", "2", "12000000", filename[i], file_size);
            if(write_len != file_size)
            {
                printf("write %s fail\n", filename[i]);
            }
        }
    }
    usb_stop();
    return ret;
}
