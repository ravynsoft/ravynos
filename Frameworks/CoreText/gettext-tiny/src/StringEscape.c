#include <stddef.h>
#include <assert.h>
#include <stdlib.h>

//FIXME out gets silently truncated if outsize is too small

size_t escape(char* in, char* out, size_t outsize) {
	size_t l = 0;
	while(*in && l + 3 < outsize) {
		switch(*in) {
			case '\n':
				*out++ = '\\';
				l++;
				*out = 'n';
				break;
			case '\r':
				*out++ = '\\';
				l++;
				*out = 'r';
				break;
			case '\t':
				*out++ = '\\';
				l++;
				*out = 't';
				break;
			case '\\':
				*out++ = '\\';
				l++;
				*out = '\\';
				break;
			case '"':
				*out++ = '\\';
				l++;
				*out = '"';
				break;
			case '\v':
				*out++ = '\\';
				l++;
				*out = 'v';
				break;
			case '\?':
				*out++ = '\\';
				l++;
				*out = '?';
				break;
			case '\f':
				*out++ = '\\';
				l++;
				*out = 'f';
				break;
			case '\b':
				*out++ = '\\';
				l++;
				*out = 'b';
				break;
			case '\a':
				*out++ = '\\';
				l++;
				*out = 'a';
				break;
			default:
				*out = *in;
		}
		in++;
		out++;
		l++;
	}
	*out = 0;
	return l;
}

size_t unescape(char* in, char *out, size_t outsize) {
	size_t l = 0;
	while(*in && l + 1 < outsize) {
		switch (*in) {
			case '\\':
				++in;
				assert(*in);
				switch(*in) {
					case 'n':
						*out='\n';
						break;
					case 'r':
						*out='\r';
						break;
					case 't':
						*out='\t';
						break;
					case '\\':
						*out='\\';
						break;
					case '"':
						*out='"';
						break;
					case 'v':
						*out='\v';
						break;
					case '?':
						*out = '\?';
						break;
					case 'f':
						*out = '\f';
						break;
					case '\'':
						*out = '\'';
						break;
					case 'b':
						*out = '\b';
						break;
					case 'a':
						*out = '\a';
						break;
					// FIXME add handling of hex and octal
					default:
						abort();
				}
				break;
			default:
				*out=*in;
		}
		in++;
		out++;
		l++;
	}
	*out = 0;
	return l;
}
