/*
 This software is licensed under the MIT License
 read https://opensource.org/licenses/MIT for more information
*/

#include <stdarg.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>

#include "main.h"

void die(int code, char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);

	fprintf(stderr, "Usage: ");
	vfprintf(stderr, fmt, va);

	va_end(va);

	exit(code);
}

int main(int argc, char *argv[])
{
	FILE *cuplist;
	int entries_used;
	uint64_t cuplist_entries[CUPLIST_ENTRIES];

	printf("Cup the Cunt\n");
	if (argc != 3)
		die(EXIT_FAILURE, "%s <CupList> <contents.cnt>\n", argv[0]);

	cuplist = fopen(argv[1], "rb");
	entries_used = 0;

	if (!cuplist)
		die(EXIT_FAILURE, "couldn't open %s.\n", argv[1]);

	printf("Reading %s...\n", argv[1]);
	if (fread(cuplist_entries, CUPLIST_ENTRIES * sizeof(uint64_t), 1, cuplist) < 1)
		die(EXIT_FAILURE, "Invalid CupList (size must be %d bytes).\n", CUPLIST_ENTRIES * sizeof(uint64_t));
	fclose(cuplist);

	for (entries_used = 0; entries_used <= CUPLIST_ENTRIES && cuplist_entries[entries_used] != 0; entries_used++);
	printf("number of entries in CupList: %d\n", entries_used);

	mkdir("updates");
	if (extract_cias(cuplist_entries, entries_used, argv[2], "updates") < 0)
		die(EXIT_FAILURE, "Couldn't extract update CIAs.\n");

	return 0;
}

int extract_cias(uint64_t *cuplist, int entries, const char *contents_path, const char *dir)
{
	FILE *contents, *cia;
	char *contents_buf, out_str[PATH_MAX + 1] = {0};
	Contents_header *cont_hdr;
	size_t cia_size;

	if (entries <= 0)
		return 0;

	contents = fopen(contents_path, "rb");
	if (!contents) 
		die(EXIT_FAILURE, "couldn't open %s.\n", contents_path);

	cont_hdr = (Contents_header*)malloc(sizeof(Contents_header));
	fseek(contents, 0, SEEK_SET);
	fread(cont_hdr, sizeof(Contents_header), 1, contents);

	if (memcmp(cont_hdr->magic, (uint8_t[]){'C','O','N','T'}, 4))
		die(EXIT_FAILURE, "invalid contents file (badmagic).\n");

	mkdir("updates");

	for (int i = 0; i < entries; i++)
	{
		snprintf(out_str, PATH_MAX, "%s/%016"PRIx64".cia", dir, cuplist[i]);
		cia = fopen(out_str, "wb");
		if (!cia)
		{
			printf("ERROR opening %s!\n", out_str);
			continue;
		}

		cia_size = cont_hdr->entries[i].offset_end - cont_hdr->entries[i].offset;
		contents_buf = malloc(cia_size);

		fseek(contents, cont_hdr->entries[i].offset + sizeof(Contents_header), SEEK_SET);
		if (fread(contents_buf, cia_size, 1, contents) <= 0)
		{
			printf("Reached CIA EOF\n");
			break;
		}

		printf("Writing %s (%"PRIi64" KB)\n", out_str, cia_size / 1024);
		if (fwrite(contents_buf, cia_size, 1, cia) <= 0)
			printf("ERROR writing to %s!\n", out_str);

		fclose(cia);
		free(contents_buf);
	}	

	free(cont_hdr);
	return 0;
}