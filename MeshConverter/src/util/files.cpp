#include "files.h"

void ReadString(FILE *fp, std::string &buffer, int fixedLength)
{
	char c;

	if (fixedLength > 0)
	{
		for (int i = 0; i < fixedLength; ++i)
		{
			fread(&c, 1, 1, fp);
			if (c != '\0')
				buffer += c;
		}
	}
	else
	{
		do
		{
			fread(&c, 1, 1, fp);
			if (c != '\0')
				buffer += c;
		} while (c != '\0');
	}
}
