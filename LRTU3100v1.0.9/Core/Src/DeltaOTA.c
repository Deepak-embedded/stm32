#include "main.h"
#include "Lora_AT_Types.h"
#include "janpatch.h"

static unsigned char source_buf[SECTOR_SIZE];
static unsigned char target_buf[SECTOR_SIZE];
static unsigned char patch_buf[SECTOR_SIZE];

size_t exFlash_read(void *ptr, size_t size, size_t count, exFlash_stream_t *stream);
size_t exFlash_write(const void *ptr, size_t size, size_t count, exFlash_stream_t *stream); 
int exFlash_seek(exFlash_stream_t *stream, long int offset, int origin); 

janpatch_ctx ctx = {
    // fread/fwrite buffers for every file, minimum size is 1 byte
    // when you run on an embedded system with block size flash, set it to the size of a block for best performance
    { source_buf, SECTOR_SIZE },
    { patch_buf, SECTOR_SIZE },
    { target_buf, SECTOR_SIZE },

    // functions which can perform basic file IO
    &exFlash_read,
    &exFlash_write,
    &exFlash_seek,

    NULL, // ftell not implemented
    NULL, // progress callback not implemented
};


size_t exFlash_read(void *ptr, size_t size, size_t count, exFlash_stream_t *stream) 
{
    if (stream->offset + count > stream->size) 
	{
        count = stream->size - stream->offset;
    }
    if(MX25L_ReadRaw(ptr, size * count, stream->offset) == HAL_OK)
    {
		osDelay(1);
		return count * size;
    }
    else
    	return 0;
}

size_t exFlash_write(const void *ptr, size_t size, size_t count, exFlash_stream_t *stream) 
{
    if((stream->offset + count) > stream->size)
    {
        count = stream->size - stream->offset;
    }

	if((size * count) == SECTOR_SIZE)
	{
		if(MX25L_OK == MX25L_Erase_Write_One_Sector(ptr, SECTOR_SIZE, stream->offset))
		{
			osDelay(10);
			MX25L_Erase_Write_One_Sector(ptr, SECTOR_SIZE, stream->offset);
		}
	}
	else if((size * count) < SECTOR_SIZE)
	{
		WriteLog(1, "extFlashWrite less than a sector size\r\n", 1);
		if(stream->offset % SECTOR_SIZE == 0)	// check if this is start for sector then erase
			Erase_RECsector(stream->offset, 1);
		if(W25Q_OK != W25Q_Write_continous(ptr, size * count, stream->offset))
		{
			osDelay(10);
			W25Q_Write_continous(ptr, size * count, stream->offset);
		}
	}
	else
		count = 0;
	osDelay(1);
    return count * size;
}

int exFlash_seek(exFlash_stream_t *stream, long int offset, int origin) 
{
    if(origin == SEEK_SET)
	{
		if (offset > stream->size)
		{
			WriteLog(1, "End of Stream\r\n", 1);
			return -1;
		}
		else if(offset != 0)
			stream->offset = offset;
		else
			offset = stream->offset;	// if offset is zero then set current offset as base address
	}
	else
		return -1;

	return 0;
}

char copyTargetToSource(uint32_t FileSize)
{
	size_t copy_len = 0;

	while(copy_len < FileSize)
	{
		if(MX25L_ReadRaw(target_buf, SECTOR_SIZE, TGT_HEX_FILE_START_ADDRESS+copy_len) == HAL_OK)
		{
			osDelay(1);
			if(MX25L_Erase_Write_One_Sector(target_buf, SECTOR_SIZE, HEX_FILE_START_ADDRESS+copy_len) == MX25L_OK)
			{
				osDelay(1);
				copy_len += SECTOR_SIZE;
			}
		}
	}
	return 1;
}
