
#include <stdio.h>
#include <string.h>
#include <config.h>
#include <miscdef.h>
#include <usart.h>
#include <api.h>
#include <protocolapi.h>
#include <ymodem.h>
#include <iflash.h>
#ifndef dbg_msg
#error mis-define dbg_msg()
#endif

#ifndef dbg_msgv
#error mis-define dbg_msgv()
#endif

#if (0 == ENABLE_UPGRADE_MSG)
#undef dbg_msg
#undef dbg_msgv
#define dbg_msg(dev, str)	
#define dbg_msgv(dev, fmt, arg...)
#endif

#define RECV_INTV_US			10000

#define YSTART_RETRY_CNT		3

static u8 buf_1k[1024];

// according to buf_1k size & flash erase method
#if (FLASH_PAGE_SIZE % 1024)
#error Not supported flash size!
#endif

static u32 byte_recv(device_info_t *dev, message_t *dmsg, u8 *c, u32 us) // 11B/ms @ 115200bps, 1B/100us
{
	u32 pos;
	usart_rx_block_t *rb = usart_get_rx_block(dev->comm_uart);

	pos = rb->read;

	do
	{
		watchdog_feed(dev);
	
		dbg_uart_msg_process(dev, dmsg);

		if(TRUE == dbg_usart_read_byte(rb, c, rb->write, &pos))
		{
			rb->read = pos;
			return 0;
		}
		
		udelay(1);
		
	}while(us--);
	
	return 1;
}

static void byte_send(device_info_t *dev, u8 c)
{
	usart_data_post(dev->comm_uart, &c, 1);
}

static u16 calc_crc(u8 *ptr, u32 count)
{
	u32 crc, i;

	crc	= 0;
	//while (--count >= 0)
	while(count--)
	{
		crc = crc ^ ((u32)*ptr++ << 8);
		
		for (i = 0; i < 8; ++i)
		{
			if (crc & 0x8000)
			{
				crc = (crc <<	1) ^ 0x1021;
			}
			else
			{
				crc = crc <<	1;
			}
		}
	}
	
	return (u16)crc;
}

static u32 c2d(u8 c)
{
	if(c >= '0' && c <= '9')
	{
		return c - '0';
	}

	if(c >= 'a' && c <= 'z')
	{
		return c - 'a' + 10;
	}

	return c - 'A' + 10;
}

#if ENABLE_IFLASH_UPDATE
static void upgrade_error(device_info_t *dev)
{

	systick_stop(); // led related
	

	mdelay(15000);

	comm_uart_deinit(dev);

	dbg_uart_deinit(dev);


	software_reset();
}
#endif

void upgrading_process(device_info_t *dev, message_t *dmsg)
{
	#if 1//UPGRADE_YMODEM
	u16 calc, fail = 0;
	union16_t crc;
	u8 *buf, start, sn = 0, sn1, sn2, first[2] = {SOH, STX};
	u32 times, i, count[2] = {128, 1024}, base, fsize, ret, *wd, code_base;
	
	code_base = get_boot_base();

	while(HOST_RUNNING == dev->ci->state)
	{
		watchdog_feed(dev);

		buf = buf_1k;

		if(fail >= 10)
		{
			dbg_msg(dev, "Too many ymodem errors, abort\r\n");
			goto exit1;
		}

		// ONLY file info packet & first file data packet need START
		if(sn < 2)
		{
			// launch a transfer
			byte_send(dev, YM_START);
			dbg_msg(dev, "send C£¬ready to receive YMODEM\r\n");
		}

		// wait for STX
		times = YSTART_RETRY_CNT;
		start = 0;
		while(times)
		{
			watchdog_feed(dev);
		
			times--;
			
			ret = byte_recv(dev, dmsg, &start, RECV_INTV_US * 5);

			if(!ret)
			{
				break;
			}
		}

		if(sn && (EOT == start))
		{
			dbg_msg(dev, "\r\nFile transfer finished.\r\n");
			byte_send(dev, YACK);

			// wait for next file OR end packet
			sn = 0;
			continue;
		}

		if(!times || start ^ first[sn > 0])
		{
			dbg_msgv(dev, "ERROR: retry %d time(s), start=%02x\r\n", YSTART_RETRY_CNT - times, start);

			if(!times)
			{
				if(start ^ first[sn > 0])
				{
					dbg_msg(dev, "No data received\r\n");
					continue;
				}
			}
			else // count for data mismatch only
			{
				fail++;
				continue;
			}
		}

		// wait for sn1
		ret = byte_recv(dev, dmsg, &sn1, RECV_INTV_US);

		if(ret)
		{
			dbg_msg(dev, "ERROR: sn1 timeout\r\n");
			if(sn) // ingore non-ymodem msg
			{
				fail++;
				byte_send(dev, YNAK);
			}
			continue;
		}

		// wait for sn2
		ret = byte_recv(dev, dmsg, &sn2, RECV_INTV_US);

		if(ret)
		{
			dbg_msg(dev, "ERROR: sn2 timeout\r\n");
			if(sn) // ingore non-ymodem msg
			{
				fail++;
				byte_send(dev, YNAK);
			}
			continue;
		}

		// check SN
		sn2 = ~sn2;

		if((sn1 ^ sn2) || (sn ^ sn1))
		{
			dbg_msgv(dev, "ERROR: sn=%02x:%02x:%02x\r\n", sn, sn1, sn2);
			if(sn) // ingore non-ymodem msg
			{
				fail++;
				byte_send(dev, YNAK);
			}
			continue;
		}

		// wait for DATA
		for(i = 0; i < count[sn > 0]; i++)
		{		
			ret = byte_recv(dev, dmsg, buf++, RECV_INTV_US);
		
			if(ret)
			{
				dbg_msgv(dev, "ERROR: data byte %d recv timeout\r\n", i);
				break;
			}
		}

		if(i ^ count[sn > 0])
		{
			fail++;
			byte_send(dev, YNAK);
			continue;
		}

		// wait for CRC
		for(i = 0; i < 2; i++)
		{		
			watchdog_feed(dev);
			ret = byte_recv(dev, dmsg, crc.b + 1 - i, RECV_INTV_US); // switch order
			//ret = Receive_Byte(crc.b + i, RECV_INTV_US); // not switch order
		
			if(ret)
			{
				dbg_msgv(dev, "ERROR: CRC byte %d recv timeout\r\n", i);
				fail++;
				byte_send(dev, YNAK);
				break;
			}
		}

		if(i ^ 2)
		{
			continue;
		}

		// check CRC
		calc = calc_crc(buf_1k, count[sn > 0]);

		if(calc ^ crc.h)
		{
			dbg_msgv(dev, "ERROR: CRC check failed %04x %04x\r\n", calc, crc.h);
			fail++;
			byte_send(dev, YNAK);
			continue;
		}

		// dump file info & program FLASH
		if(!sn)
		{
			// first packet, file info packet
			wd = (u32 *)buf_1k;

			if(!wd[0]) // End packet
			{
				dbg_msg(dev, "All file(s) transfer finished!\r\n");
			
				byte_send(dev, YACK);

				break;
			}
		
			buf = buf_1k + strlen((char *)buf_1k) + 1;
			//sscanf((const char *)buf, "0x%x ", &fsize);
			// as code size is too large for sscanf, so change

			buf += 2;
			i = 0;
			fsize = 0;
			while(buf[i] && buf[i] ^ ' ')
			{
				watchdog_feed(dev);
				fsize <<= 4;
				fsize |= c2d(buf[i++]);
			}
			// file info
			dbg_msgv(dev, "File:%s, size:%d\r\n", buf_1k, fsize);
		}
		else
		{
			base = code_base + (sn - 1) * count[sn > 0];
	
		#if ENABLE_IFLASH_UPDATE
			wd = (u32 *)buf_1k;

			if(!((base - code_base) % FLASH_PAGE_SIZE))
			{
				//dbg_msgv(dev, "Erase from %08x\r\n", base);
				watchdog_feed(dev);
				
				if(FLASH_COMPLETE ^ iflash_page_erase(base))
				{
					dbg_msgv(dev, "ERROR: Flash page %d erase failed", sn - 1);
					goto exit0;
				}
			}
			
			for(i = 0; i < count[sn > 0]; i += 4)
			{
				watchdog_feed(dev);
			
				if(FLASH_COMPLETE ^ iflash_word_program(base + i, *wd))
				{
					dbg_msgv(dev, "ERROR: Flash addr %d prog failed", base + i);
					goto exit0;
				}
				wd++;
			}

			dbg_msg(dev, "=   ");
		#else
			if(!((base - code_base) % count[sn > 0]))
			{
				dbg_msgv(dev, "sn:%04d, erasing from %08x\r\n", sn, base);
			}
		#endif
		}

		sn++;

		fail = 0;

		// ACK is sent after prepared
		byte_send(dev, YACK);
		dbg_msgv(dev, "sn:%02d, send ACK\r\n", sn);

	}

	#if 0//ENABLE_IFLASH_UPDATE
	if(UPD_TYPE_APP == type)
	{
		dbg_msg(dev, "\r\n>>> RESTORE APP FLAG PAGE...\r\n");
		
		i = item_test_operation(dev, NULL);

		#if ENABLE_DEBUG_UART
		iflash_operation_result(dev, i);
		#endif
	}
	#endif
	
	mdelay(3000);

	dbg_msg(dev, "\r\nupgrade end, reboot\r\n");

exit1:

	comm_uart_deinit(dev);

	dbg_uart_deinit(dev);

	software_reset(); // never return

#if ENABLE_IFLASH_UPDATE
exit0:
	byte_send(dev, CA); // abort

	upgrade_error(dev); // never return
#endif
#endif
}
