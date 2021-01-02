/*  kernel.c - the C part of the kernel */
/*  Copyright (C) 1999, 2010  Free Software Foundation, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "multiboot2.h"
#include "picture.c"

/*  Macros. */

/*  Some screen stuff. */
/*  The number of columns. */
#define COLUMNS                 80
/*  The number of lines. */
#define LINES                   24
/*  The attribute of a character. */
#define ATTRIBUTE               7
/*  The video memory address. */
#define VIDEO                   0xB8000

#define false 0
#define true 1

/*  Variables. */
/*  Save the X position. */
static int xpos;
/*  Save the Y position. */
static int ypos;
/*  Point to the video memory. */
static volatile unsigned char *video;

/*  Forward declarations. */
void cmain (unsigned long magic, unsigned long addr);
static void cls (void);
static void itoa (char *buf, int base, int d);
static void putchar (int c);
void printf (const char *format, ...);

void drawimgRGBA32(struct multiboot_tag_framebuffer *tagfb, struct picture *picture);
void sleep(unsigned x);

/*  Kernel main */
void cmain (unsigned long magic, unsigned long addr)
{
	/*  Clear the screen. */
	cls ();

	/*  Am I booted by a Multiboot-compliant boot loader? */
	if (magic != MULTIBOOT2_BOOTLOADER_MAGIC)
	{
		printf ("Invalid magic number: 0x%x\n", (unsigned) magic);
		return;
	}

	if (addr & 7)
	{
		printf ("Unaligned mbi: 0x%x\n", addr);
		return;
	}
	
	struct multiboot_tag *tag;
	for (tag = (struct multiboot_tag *) (addr + 8);
		tag->type != MULTIBOOT_TAG_TYPE_END;
		tag = (struct multiboot_tag *) ((multiboot_uint8_t *) tag + ((tag->size + 7) & ~7)))
	{
		if (tag->type == MULTIBOOT_TAG_TYPE_FRAMEBUFFER)
		{		
			struct multiboot_tag_framebuffer *tagfb = (struct multiboot_tag_framebuffer *) tag;

			if (tagfb->common.framebuffer_type == MULTIBOOT_FRAMEBUFFER_TYPE_RGB)
			{
				
				if (tagfb->common.framebuffer_bpp == 32)
				{
					int k = 0;
					while (true)
					{
						k = (k+1)%4;
						drawimgRGBA32(tagfb, &pictures[k]);
						sleep(1);
					}
				}
				else
				{
					printf("Wrong color depth, cannot continue !\n");
				}
			}
			else
			{
				printf("Wrong framebuffer type, cannot continue !\n");
			}
		}
	}
}

/*  Draws RGBA, 4bpp image to framebuffer  */
void drawimgRGBA32(struct multiboot_tag_framebuffer *tagfb, struct picture *picture) {
	void *fb = (void *) (unsigned long) tagfb->common.framebuffer_addr;
	for (unsigned i = 0; i < picture->height; i++)
	{
		for (unsigned j = 0; j < picture->width; j++)
		{
			multiboot_uint32_t *pixel = fb + 4 * (i*tagfb->common.framebuffer_width + j);
			*pixel = picture->subpixels[4 * (i*picture->width + j)] << 16
				| picture->subpixels[4 * (i*picture->width + j) + 1] << 8
				| picture->subpixels[4 * (i*picture->width + j) + 2]
				| picture->subpixels[4 * (i*picture->width + j) + 3] << 24;
		}
	}
}


/*  Pause for x seconds  */
void sleep(unsigned x) {
	for (unsigned long long i = 0; i < x*700000000; i++) i=i;
}

/*  Clear the screen and initialize VIDEO, XPOS and YPOS. */
static void cls (void)
{
	int i;

	video = (unsigned char *) VIDEO;
	
	for (i = 0; i < COLUMNS * LINES * 2; i++)
		*(video + i) = 0;

	xpos = 0;
	ypos = 0;
}

/*  Convert the integer D to a string and save the string in BUF. If
   BASE is equal to ’d’, interpret that D is decimal, and if BASE is
   equal to ’x’, interpret that D is hexadecimal. */
static void itoa (char *buf, int base, int d)
{
	char *p = buf;
	char *p1, *p2;
	unsigned long ud = d;
	int divisor = 10;
	
  /*  If %d is specified and D is minus, put ‘-’ in the head. */
	if (base == 'd' && d < 0)
	{
		*p++ = '-';
		buf++;
		ud = -d;
	}
	else if (base == 'x')
		divisor = 16;

  /*  Divide UD by DIVISOR until UD == 0. */
	do
	{
		int remainder = ud % divisor;
		
		*p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
	}
	while (ud /= divisor);

  /*  Terminate BUF. */
	*p = 0;
	
  /*  Reverse BUF. */
	p1 = buf;
	p2 = p - 1;
	while (p1 < p2)
	{
		char tmp = *p1;
		*p1 = *p2;
		*p2 = tmp;
		p1++;
		p2--;
	}
}

/*  Put the character C on the screen. */
static void putchar (int c)
{
	if (c == '\n' || c == '\r')
	{
		newline:
		xpos = 0;
		ypos++;
		if (ypos >= LINES)
			ypos = 0;
		return;
	}

	*(video + (xpos + ypos * COLUMNS) * 2) = c & 0xFF;
	*(video + (xpos + ypos * COLUMNS) * 2 + 1) = ATTRIBUTE;

	xpos++;
	if (xpos >= COLUMNS)
		goto newline;
}

/*  Format a string and print it on the screen, just like the libc
   function printf. */
void printf (const char *format, ...)
{
	char **arg = (char **) &format;
	int c;
	char buf[20];

	arg++;
	
	while ((c = *format++) != 0)
	{
		if (c != '%')
		{
			putchar (c);
		}
		else
		{
			char *p, *p2;
			int pad0 = 0, pad = 0;
			
			c = *format++;
			if (c == '0')
			{
				pad0 = 1;
				c = *format++;
			}

			if (c >= '0' && c <= '9')
			{
				pad = c - '0';
				c = *format++;
			}

			switch (c)
			{
				case 'd':
				case 'u':
				case 'x':
				itoa (buf, c, *((int *) arg++));
				p = buf;
				goto string;
				break;

				case 's':
				p = *arg++;
				if (! p)
					p = "(null)";

				string:
				for (p2 = p; *p2; p2++);
					for (; p2 < p + pad; p2++)
						putchar (pad0 ? '0' : ' ');
					while (*p)
						putchar (*p++);
					break;

				default:
				putchar (*((int *) arg++));
				break;
			}
		}
	}
}
