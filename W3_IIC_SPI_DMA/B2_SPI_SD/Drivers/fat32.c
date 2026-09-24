#include "fat32.h"
#include "sd.h"


/* =========================================================
 * Sector buffer
 * ========================================================= */

static unsigned char sector[512];


/* =========================================================
 * FAT32 information
 * ========================================================= */

static unsigned long fat_start;
static unsigned long data_start;

static unsigned long sectors_per_cluster;
static unsigned long root_cluster;

static unsigned long partition_start;


/* =========================================================
 * Read little-endian 16 bit
 * ========================================================= */

static unsigned int Read16(
    unsigned char *p
)
{
    return p[0] |
           ((unsigned int)p[1] << 8);
}


/* =========================================================
 * Read little-endian 32 bit
 * ========================================================= */

static unsigned long Read32(
    unsigned char *p
)
{
    return ((unsigned long)p[0]) |
           ((unsigned long)p[1] << 8) |
           ((unsigned long)p[2] << 16) |
           ((unsigned long)p[3] << 24);
}


/* =========================================================
 * Write little-endian 32 bit
 * ========================================================= */

static void Write32(
    unsigned char *p,
    unsigned long value
)
{
    p[0] = value;
    p[1] = value >> 8;
    p[2] = value >> 16;
    p[3] = value >> 24;
}


/* =========================================================
 * Cluster -> Sector
 * ========================================================= */

static unsigned long ClusterToSector(
    unsigned long cluster
)
{
    return data_start +
           (cluster - 2) * sectors_per_cluster;
}


/* =========================================================
 * Read FAT entry
 * ========================================================= */

static unsigned long FAT32_GetNextCluster(
    unsigned long cluster
)
{
    unsigned long fat_sector;
    unsigned int offset;


    fat_sector =
        fat_start +
        (cluster * 4) / 512;


    offset =
        (cluster * 4) % 512;


    if (SD_ReadBlock(
            fat_sector,
            sector
        ) != SD_OK)
    {
        return 0xFFFFFFFF;
    }


    return Read32(
        &sector[offset]
    ) & 0x0FFFFFFF;
}


/* =========================================================
 * Write FAT entry
 * ========================================================= */

static unsigned char FAT32_SetCluster(
    unsigned long cluster,
    unsigned long value
)
{
    unsigned long fat_sector;
    unsigned int offset;


    fat_sector =
        fat_start +
        (cluster * 4) / 512;


    offset =
        (cluster * 4) % 512;


    if (SD_ReadBlock(
            fat_sector,
            sector
        ) != SD_OK)
    {
        return FAT32_ERROR;
    }


    Write32(
        &sector[offset],
        value
    );


    if (SD_WriteBlock(
            fat_sector,
            sector
        ) != SD_OK)
    {
        return FAT32_ERROR;
    }


    return FAT32_OK;
}


/* =========================================================
 * Find free cluster
 * ========================================================= */

static unsigned long FAT32_FindFreeCluster(void)
{
    unsigned long cluster;
    unsigned long value;


    /*
     * Bắt đầu từ cluster 2.
     */

    for (
        cluster = 2;
        cluster < 0x100000;
        cluster++
    )
    {
        value =
            FAT32_GetNextCluster(cluster);


        if (value == 0)
        {
            return cluster;
        }
    }


    return 0;
}


/* =========================================================
 * Convert filename -> FAT 8.3 name
 *
 * TEST.TXT
 *
 * ->
 *
 * TEST    TXT
 * ========================================================= */

static void FAT32_Name83(
    char *filename,
    unsigned char *name
)
{
    unsigned int i;
    unsigned int j;


    /*
     * Fill with spaces
     */

    for (i = 0; i < 11; i++)
    {
        name[i] = ' ';
    }


    /*
     * Filename
     */

    i = 0;
    j = 0;


    while (
        filename[i] != 0 &&
        filename[i] != '.' &&
        j < 8
    )
    {
        if (
            filename[i] >= 'a' &&
            filename[i] <= 'z'
        )
        {
            name[j] =
                filename[i] - 'a' + 'A';
        }
        else
        {
            name[j] =
                filename[i];
        }


        i++;
        j++;
    }


    /*
     * Find '.'
     */

    while (
        filename[i] != 0 &&
        filename[i] != '.'
    )
    {
        i++;
    }


    /*
     * Extension
     */

    if (filename[i] == '.')
    {
        i++;
    }


    j = 0;


    while (
        filename[i] != 0 &&
        j < 3
    )
    {
        if (
            filename[i] >= 'a' &&
            filename[i] <= 'z'
        )
        {
            name[8 + j] =
                filename[i] - 'a' + 'A';
        }
        else
        {
            name[8 + j] =
                filename[i];
        }


        i++;
        j++;
    }
}


/* =========================================================
 * Compare FAT 8.3 name
 * ========================================================= */

static unsigned char FAT32_NameMatch(
    unsigned char *entry,
    unsigned char *name
)
{
    unsigned int i;


    for (i = 0; i < 11; i++)
    {
        if (entry[i] != name[i])
        {
            return 0;
        }
    }


    return 1;
}


/* =========================================================
 * FAT32 Mount
 * ========================================================= */

unsigned char FAT32_Mount(void)
{
    unsigned int reserved;
    unsigned int fat_count;
    unsigned long sectors_per_fat;

    unsigned long root_sector;


    /*
     * Read sector 0
     */

    if (SD_ReadBlock(
            0,
            sector
        ) != SD_OK)
    {
        return FAT32_ERROR;
    }


    /*
     * Boot signature
     */

    if (
        sector[510] != 0x55 ||
        sector[511] != 0xAA
    )
    {
        return FAT32_ERROR;
    }


    /*
     * Check MBR partition
     *
     * First partition:
     * offset 446
     *
     * Partition type:
     * offset 450
     */

    if (
        sector[450] == 0x0B ||
        sector[450] == 0x0C
    )
    {
        /*
         * FAT32 partition
         */

        partition_start =
            Read32(
                &sector[454]
            );
    }
    else
    {
        /*
         * Sector 0 may itself
         * be FAT32 boot sector
         */

        partition_start = 0;
    }


    /*
     * Read FAT32 Boot Sector
     */

    if (SD_ReadBlock(
            partition_start,
            sector
        ) != SD_OK)
    {
        return FAT32_ERROR;
    }


    /*
     * Boot signature
     */

    if (
        sector[510] != 0x55 ||
        sector[511] != 0xAA
    )
    {
        return FAT32_ERROR;
    }


    /*
     * Bytes per sector
     */

    if (
        Read16(&sector[11]) != 512
    )
    {
        return FAT32_ERROR;
    }


    /*
     * Sectors per cluster
     */

    sectors_per_cluster =
        sector[13];


    if (
        sectors_per_cluster == 0
    )
    {
        return FAT32_ERROR;
    }


    /*
     * Reserved sectors
     */

    reserved =
        Read16(&sector[14]);


    /*
     * Number of FATs
     */

    fat_count =
        sector[16];


    /*
     * FAT32 sectors per FAT
     */

    sectors_per_fat =
        Read32(&sector[36]);


    /*
     * Root directory cluster
     */

    root_cluster =
        Read32(&sector[44]);


    if (
        fat_count == 0 ||
        sectors_per_fat == 0 ||
        root_cluster < 2
    )
    {
        return FAT32_ERROR;
    }


    /*
     * FAT start
     */

    fat_start =
        partition_start +
        reserved;


    /*
     * Data start
     */

    data_start =
        fat_start +
        fat_count * sectors_per_fat;


    /*
     * Root directory sector
     */

    root_sector =
        ClusterToSector(
            root_cluster
        );


    /*
     * Test root directory
     */

    if (SD_ReadBlock(
            root_sector,
            sector
        ) != SD_OK)
    {
        return FAT32_ERROR;
    }


    return FAT32_OK;
}


/* =========================================================
 * Find file in root directory
 *
 * Return:
 *
 * 0       = file not found
 * != 0    = first cluster
 *
 * Also return:
 *
 * entry_sector
 * entry_offset
 * ========================================================= */

static unsigned long FAT32_FindFile(
    char *filename,
    unsigned long *entry_sector,
    unsigned int *entry_offset
)
{
    unsigned char name[11];

    unsigned long cluster;
    unsigned long next;

    unsigned long s;

    unsigned int offset;
    unsigned char attribute;


    FAT32_Name83(
        filename,
        name
    );


    cluster =
        root_cluster;


    while (
        cluster >= 2 &&
        cluster < 0x0FFFFFF8
    )
    {
        for (
            s = 0;
            s < sectors_per_cluster;
            s++
        )
        {
            if (SD_ReadBlock(
                    ClusterToSector(cluster) + s,
                    sector
                ) != SD_OK)
            {
                return 0;
            }


            for (
                offset = 0;
                offset < 512;
                offset += 32
            )
            {
                /*
                 * End of directory
                 */

                if (
                    sector[offset] == 0x00
                )
                {
                    return 0;
                }


                /*
                 * Deleted
                 */

                if (
                    sector[offset] == 0xE5
                )
                {
                    continue;
                }


                /*
                 * Long filename
                 */

                if (
                    (sector[offset + 11] & 0x0F)
                    == 0x0F
                )
                {
                    continue;
                }


                /*
                 * Attribute
                 */

                attribute =
                    sector[offset + 11];


                /*
                 * Volume label
                 */

                if (
                    attribute & 0x08
                )
                {
                    continue;
                }


                /*
                 * Directory
                 */

                if (
                    attribute & 0x10
                )
                {
                    continue;
                }


                /*
                 * Compare filename
                 */

                if (
                    FAT32_NameMatch(
                        &sector[offset],
                        name
                    )
                )
                {
                    *entry_sector =
                        ClusterToSector(cluster) + s;


                    *entry_offset =
                        offset;


                    /*
                     * First cluster:
                     *
                     * High 16 bits:
                     * offset 20,21
                     *
                     * Low 16 bits:
                     * offset 26,27
                     */

                    return
                        ((unsigned long)sector[offset + 20] << 16) |
                        ((unsigned long)sector[offset + 21] << 24) |
                        sector[offset + 26] |
                        ((unsigned long)sector[offset + 27] << 8);
                }
            }
        }


        /*
         * Next directory cluster
         */

        next =
            FAT32_GetNextCluster(
                cluster
            );


        if (
            next == 0xFFFFFFFF
        )
        {
            return 0;
        }


        cluster =
            next;
    }


    return 0;
}


/* =========================================================
 * Create file
 * ========================================================= */

static unsigned char FAT32_CreateFile(
    char *filename,
    unsigned long *entry_sector,
    unsigned int *entry_offset
)
{
    unsigned char name[11];

    unsigned long cluster;
    unsigned long next;

    unsigned long s;

    unsigned int offset;
    unsigned int i;


    FAT32_Name83(
        filename,
        name
    );


    cluster =
        root_cluster;


    while (
        cluster >= 2 &&
        cluster < 0x0FFFFFF8
    )
    {
        for (
            s = 0;
            s < sectors_per_cluster;
            s++
        )
        {
            if (SD_ReadBlock(
                    ClusterToSector(cluster) + s,
                    sector
                ) != SD_OK)
            {
                return FAT32_ERROR;
            }


            for (
                offset = 0;
                offset < 512;
                offset += 32
            )
            {
                /*
                 * Free directory entry
                 */

                if (
                    sector[offset] == 0x00 ||
                    sector[offset] == 0xE5
                )
                {
                    /*
                     * Name
                     */

                    for (
                        i = 0;
                        i < 11;
                        i++
                    )
                    {
                        sector[offset + i] =
                            name[i];
                    }


                    /*
                     * Attribute:
                     * Archive
                     */

                    sector[offset + 11] =
                        0x20;


                    /*
                     * First cluster = 0
                     */

                    sector[offset + 20] = 0;
                    sector[offset + 21] = 0;

                    sector[offset + 26] = 0;
                    sector[offset + 27] = 0;


                    /*
                     * File size = 0
                     */

                    sector[offset + 28] = 0;
                    sector[offset + 29] = 0;
                    sector[offset + 30] = 0;
                    sector[offset + 31] = 0;


                    /*
                     * Write directory sector
                     */

                    if (SD_WriteBlock(
                            ClusterToSector(cluster) + s,
                            sector
                        ) != SD_OK)
                    {
                        return FAT32_ERROR;
                    }


                    *entry_sector =
                        ClusterToSector(cluster) + s;


                    *entry_offset =
                        offset;


                    return FAT32_OK;
                }
            }
        }


        next =
            FAT32_GetNextCluster(
                cluster
            );


        if (
            next == 0xFFFFFFFF
        )
        {
            return FAT32_ERROR;
        }


        cluster =
            next;
    }


    return FAT32_ERROR;
}


/* =========================================================
 * Free old cluster chain
 * ========================================================= */

static unsigned char FAT32_FreeClusterChain(
    unsigned long cluster
)
{
    unsigned long next;


    while (
        cluster >= 2 &&
        cluster < 0x0FFFFFF8
    )
    {
        next =
            FAT32_GetNextCluster(
                cluster
            );


        if (
            next == 0xFFFFFFFF
        )
        {
            return FAT32_ERROR;
        }


        /*
         * Mark current cluster free
         */

        if (
            FAT32_SetCluster(
                cluster,
                0
            ) != FAT32_OK
        )
        {
            return FAT32_ERROR;
        }


        /*
         * End of chain
         */

        if (
            next >= 0x0FFFFFF8
        )
        {
            break;
        }


        cluster =
            next;
    }


    return FAT32_OK;
}


/* =========================================================
 * Write file
 *
 * filename:
 *     TEST.TXT
 *
 * data:
 *     data buffer
 *
 * size:
 *     number of bytes
 * ========================================================= */

unsigned char FAT32_WriteFile(
    char *filename,
    unsigned char *data,
    unsigned long size
)
{
    unsigned long entry_sector;
    unsigned int entry_offset;

    unsigned long first_cluster;
    unsigned long old_cluster;

    unsigned long cluster;
    unsigned long new_cluster;

    unsigned long required_clusters;
    unsigned long written;

    unsigned long i;
    unsigned long s;

    unsigned long data_index;

    unsigned int count;


    /*
     * Find existing file
     */

    first_cluster =
        FAT32_FindFile(
            filename,
            &entry_sector,
            &entry_offset
        );


    /*
     * File does not exist
     */

    if (
        first_cluster == 0
    )
    {
        if (
            FAT32_CreateFile(
                filename,
                &entry_sector,
                &entry_offset
            ) != FAT32_OK
        )
        {
            return FAT32_ERROR;
        }
    }
    else
    {
        /*
         * File already exists.
         *
         * Free old cluster chain.
         */

        old_cluster =
            first_cluster;


        if (
            FAT32_FreeClusterChain(
                old_cluster
            ) != FAT32_OK
        )
        {
            return FAT32_ERROR;
        }
    }


    /*
     * Empty file
     */

    if (
        size == 0
    )
    {
        if (SD_ReadBlock(
                entry_sector,
                sector
            ) != SD_OK)
        {
            return FAT32_ERROR;
        }


        /*
         * First cluster = 0
         */

        sector[entry_offset + 20] = 0;
        sector[entry_offset + 21] = 0;

        sector[entry_offset + 26] = 0;
        sector[entry_offset + 27] = 0;


        /*
         * File size = 0
         */

        Write32(
            &sector[entry_offset + 28],
            0
        );


        if (SD_WriteBlock(
                entry_sector,
                sector
            ) != SD_OK)
        {
            return FAT32_ERROR;
        }


        return FAT32_OK;
    }


    /*
     * Calculate required clusters
     *
     * cluster size =
     * 512 * sectors_per_cluster
     */

    required_clusters =
        (
            size +
            (512 * sectors_per_cluster) -
            1
        ) /
        (512 * sectors_per_cluster);


    /*
     * Find first free cluster
     */

    first_cluster =
        FAT32_FindFreeCluster();


    if (
        first_cluster == 0
    )
    {
        return FAT32_ERROR;
    }


    cluster =
        first_cluster;


    written =
        0;


    /*
     * Write clusters
     */

    for (
        i = 0;
        i < required_clusters;
        i++
    )
    {
        /*
         * Write all sectors
         * inside this cluster
         */

        for (
            s = 0;
            s < sectors_per_cluster;
            s++
        )
        {
            /*
             * Clear sector buffer
             */

            for (
                count = 0;
                count < 512;
                count++
            )
            {
                sector[count] = 0;
            }


            /*
             * Copy data
             */

            for (
                count = 0;
                count < 512 &&
                written < size;
                count++
            )
            {
                sector[count] =
                    data[written];

                written++;
            }


            /*
             * Write sector
             */

            if (
                SD_WriteBlock(
                    ClusterToSector(cluster) + s,
                    sector
                ) != SD_OK
            )
            {
                return FAT32_ERROR;
            }
        }


        /*
         * More clusters required
         */

        if (
            i + 1 < required_clusters
        )
        {
            /*
             * Find next free cluster
             */

            new_cluster =
                FAT32_FindFreeCluster();


            if (
                new_cluster == 0
            )
            {
                return FAT32_ERROR;
            }


            /*
             * Current -> next
             */

            if (
                FAT32_SetCluster(
                    cluster,
                    new_cluster
                ) != FAT32_OK
            )
            {
                return FAT32_ERROR;
            }


            cluster =
                new_cluster;
        }
        else
        {
            /*
             * End Of File
             */

            if (
                FAT32_SetCluster(
                    cluster,
                    0x0FFFFFFF
                ) != FAT32_OK
            )
            {
                return FAT32_ERROR;
            }
        }
    }


    /*
     * Read directory entry
     */

    if (
        SD_ReadBlock(
            entry_sector,
            sector
        ) != SD_OK
    )
    {
        return FAT32_ERROR;
    }


    /*
     * First cluster - high 16 bits
     */

    sector[entry_offset + 20] =
        (first_cluster >> 16) & 0xFF;


    sector[entry_offset + 21] =
        (first_cluster >> 24) & 0xFF;


    /*
     * First cluster - low 16 bits
     */

    sector[entry_offset + 26] =
        first_cluster & 0xFF;


    sector[entry_offset + 27] =
        (first_cluster >> 8) & 0xFF;


    /*
     * File size
     */

    Write32(
        &sector[entry_offset + 28],
        size
    );


    /*
     * Write directory entry
     */

    if (
        SD_WriteBlock(
            entry_sector,
            sector
        ) != SD_OK
    )
    {
        return FAT32_ERROR;
    }


    return FAT32_OK;
}


/* =========================================================
 * Read file
 *
 * Return:
 *
 * 0       = error
 * > 0     = number of bytes read
 * ========================================================= */

unsigned long FAT32_ReadFile(
    char *filename,
    unsigned char *buffer,
    unsigned long max_size
)
{
    unsigned long entry_sector;
    unsigned int entry_offset;

    unsigned long cluster;

    unsigned long file_size;
    unsigned long read_size;

    unsigned long current_sector;

    unsigned long sector_index;
    unsigned long cluster_index;

    unsigned long offset;

    unsigned long count;


    /*
     * Buffer size must be > 0
     */

    if (
        max_size == 0
    )
    {
        return 0;
    }


    /*
     * Find file
     */

    cluster =
        FAT32_FindFile(
            filename,
            &entry_sector,
            &entry_offset
        );


    if (
        cluster == 0
    )
    {
        return 0;
    }


    /*
     * Read directory entry
     */

    if (
        SD_ReadBlock(
            entry_sector,
            sector
        ) != SD_OK
    )
    {
        return 0;
    }


    /*
     * File size
     */

    file_size =
        Read32(
            &sector[entry_offset + 28]
        );


    /*
     * Nothing to read
     */

    if (
        file_size == 0
    )
    {
        buffer[0] = 0;

        return 0;
    }


    /*
     * Keep one byte for '\0'
     */

    if (
        file_size >= max_size
    )
    {
        read_size =
            max_size - 1;
    }
    else
    {
        read_size =
            file_size;
    }


    offset =
        0;


    /*
     * Read cluster chain
     */

    while (
        cluster >= 2 &&
        cluster < 0x0FFFFFF8 &&
        offset < read_size
    )
    {
        /*
         * Read sectors inside cluster
         */

        for (
            sector_index = 0;
            sector_index < sectors_per_cluster;
            sector_index++
        )
        {
            if (
                offset >= read_size
            )
            {
                break;
            }


            current_sector =
                ClusterToSector(cluster) +
                sector_index;


            /*
             * Read SD sector
             */

            if (
                SD_ReadBlock(
                    current_sector,
                    sector
                ) != SD_OK
            )
            {
                return 0;
            }


            /*
             * Number of bytes to copy
             */

            count =
                read_size - offset;


            if (
                count > 512
            )
            {
                count = 512;
            }


            /*
             * Copy to user buffer
             */

            for (
                cluster_index = 0;
                cluster_index < count;
                cluster_index++
            )
            {
                buffer[offset] =
                    sector[cluster_index];

                offset++;
            }
        }


        /*
         * Get next cluster
         */

        cluster =
            FAT32_GetNextCluster(
                cluster
            );


        if (
            cluster == 0xFFFFFFFF
        )
        {
            return 0;
        }
    }


    /*
     * String terminator
     */

    buffer[offset] = 0;


    return offset;
}
