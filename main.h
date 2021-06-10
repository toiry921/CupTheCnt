#include <inttypes.h>

#define CUPLIST_ENTRIES (0x100)

typedef struct
{
	uint32_t offset;
	uint32_t offset_end;
} Contents_entry;

typedef struct
{
	uint8_t magic[4]; //the word is "CONT"
	uint8_t unknown[0xBFC];
	Contents_entry entries[CUPLIST_ENTRIES];
} Contents_header;