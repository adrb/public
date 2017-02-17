/*
 * Adrian Brzezinski (2017) <adrbxx at gmail.com>
 * License: GPLv2+
 *
 * Compilation: gcc -Wall fiemap.c -o fiemap
 * Usage: ./fiemap -h
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <assert.h>
#include <getopt.h>
#include <stdbool.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/fs.h>
#include <linux/fiemap.h>

#define FIEMAP_FLAG_ONDISK      0x00000008      // new flag definition, just in case

int MAX_EXTENTS_NUM = 128;

void help( char *prog ) {

        printf("\nUsage: %s [options] <path>\n\n", prog);
        printf("\t  -o          query FIEMAP with FIEMAP_FLAG_ONDISK flag set\n");
        printf("\t  -p          print file extent map\n");
        printf("\t  -n number   number of extents queried by single ioctl call\n");
        printf("\t  -h          print this help and exit\n\n");
}

__u64 fiemap_size( char *path, bool ondisk_size, bool print_extents) {

	__u64 extents_size = 0;
	__u64 extents_count = 0;
	__u64 extents_encoded = 0;
	__u64 extents_shared = 0;

	int fd = 0;
	if ((fd = open(path, O_RDONLY)) < 0) {
        	fprintf(stderr, "ERROR: Cannot open %s\n", path);
        	return (__u64)-1;
    	}

	struct stat statinfo;
	if (fstat(fd, &statinfo) < 0) {
        	fprintf(stderr, "ERROR: Cannot stat %s\n", path);
        	close(fd);
        	return (__u64)-1;
    	}

	struct fiemap *p_fiemap = (struct fiemap *)malloc( offsetof(struct fiemap, fm_extents[0])
                                                                + sizeof(struct fiemap_extent) * MAX_EXTENTS_NUM);
	assert(p_fiemap);

	__u64 start = 0;
        __u64 end = statinfo.st_size;
	__u64 last = 0;
        __u64 last_phy = (__u64)-1;

        bool eoe = false;
	while ( !eoe ) {
        	p_fiemap->fm_start  = start;
        	p_fiemap->fm_length = end;
        	p_fiemap->fm_flags  = FIEMAP_FLAG_SYNC;
        	p_fiemap->fm_extent_count = MAX_EXTENTS_NUM;

                if ( ondisk_size ) p_fiemap->fm_flags |= FIEMAP_FLAG_ONDISK;

        	if ( ioctl(fd, FS_IOC_FIEMAP, p_fiemap) == -1 ) {
			fprintf(stderr, "ERROR: ioctl failed: %s\n", strerror(errno));
                        extents_size = -1;
			break;
		}

                int i = 0;
		for ( ; i < p_fiemap->fm_mapped_extents ; i++) {

			struct fiemap_extent extent;
			extent = p_fiemap->fm_extents[i];

                        /*
                         * if we read with ONDISK flag, then we just don't know
                         * "logical" extent size, so we may get last extent
                         * from previous loop as first one
                         */
                        if ( i == 0 && p_fiemap->fm_flags & FIEMAP_FLAG_ONDISK &&
                             last_phy == extent.fe_physical )
                                continue;

			if ( extent.fe_flags & FIEMAP_EXTENT_SHARED ) extents_shared++;
                        if ( extent.fe_flags & FIEMAP_EXTENT_ENCODED ) extents_encoded++;
		
			extents_size += extent.fe_length;

                        if ( print_extents ) {
                                printf("Extent: %-6llu logical=%-14llu phy=%-14llu len=%-10llu flags=%x\n", extents_count,
                                        (__u64)extent.fe_logical, (__u64)extent.fe_physical,
                                        (__u64)extent.fe_length, extent.fe_flags);
                        }

        		extents_count++;

                        if (extent.fe_flags & FIEMAP_EXTENT_LAST) {
                                eoe = true;
                                break;
                        }
      
            		last = extent.fe_logical + extent.fe_length;
                        last_phy = extent.fe_physical;
        	}

		start = last;
	}

        if ( print_extents )
	        printf("Number of extents: %llu (encoded %llu, shared %llu)\n", extents_count, extents_encoded, extents_shared);

	free(p_fiemap);
	close(fd);

return extents_size;
}

int main( int argc, char *argv[] ) {

        char *path = 0;
        bool ondisk_size = false;
        bool print_extents = false;
        __u64 size = 0;

        int opt;
        while ( (opt=getopt(argc,argv,"hn:op")) != -1 ) {
                switch( opt ) {
                case 'n':
                        MAX_EXTENTS_NUM = atoi(optarg);
                break;
                case 'o':
                        ondisk_size = true;
                break;
                case 'p':
                        print_extents = true;
                break;
                case 'h':
                case '?':
                case ':':
                        help(argv[0]);
                        exit(EXIT_FAILURE);
                }
        }

        if (optind >= argc) {
                fprintf(stderr, "ERROR: Expected path\n");
                help(argv[0]);
                exit(EXIT_FAILURE);
        }

        path = argv[optind];
        if ( (size = fiemap_size(path, ondisk_size, print_extents)) != (__u64)-1 )
                printf("Size: %llu bytes\n", size);

return EXIT_SUCCESS;
}
