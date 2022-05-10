/*
    NASA/TRMM, Code 910.1.
    This is the TRMM Office Radar Software Library.
    Copyright (C) 1996, 1997
            John H. Merritt
            Space Applications Corporation
            Vienna, Virginia

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#define _USE_BSD
#include <sys/types.h>
#include <signal.h>
#include <zlib.h>
#ifdef _WIN32
#include <fileapi.h>
#endif

#define CHUNK 16384

/* Prototype definitions within this file. */
int no_command (char *cmd);
FILE *uncompress_pipe (FILE *fp);
FILE *compress_pipe (FILE *fp);

/**
 * We need to have a portable way between windows and linux to keep track of
 * tmpfiles. Unfortunatelly this means that we need to also ensure that temp
 * file is removed somehow. Preferrably when closing file. On windows we use
 * +TD as file modifiers. On windows we just run unlink immediately after
 * opening file.
 */
FILE* create_temporary_file(void)
{
  char buff[L_tmpnam];
  char* nam = NULL;
  char tFilename[1024];

  FILE* result = NULL;

  memset(buff, 0, sizeof(char)*L_tmpnam);
  nam = tmpnam(buff);

#ifdef _WIN32
  /* tmpfile on windows creates temporary file under C:\ but since that folder might have write permissions it might
   * not be possible to create them. Instead we need to use a different path for windows and combine temp path with
   * tmpname. If tmpname begins with \, we can concatenate it.  */
  DWORD nBufferLength = 1024;
  LPSTR pathBuffer[1024];
  DWORD len = GetTempPathA(nBufferLength, pathBuffer);
  pathBuffer[len] = '\0';
  if (nam != NULL && nam[0] == '\\' ) {
    if ((strlen(pathBuffer) + strlen(nam) + 1) < 1024) {
      strcat(pathBuffer, nam);
      strncpy(tFilename, pathBuffer, 1024);
    }
  }
  result = fopen(tFilename, "wb+TD");
#else
  strcpy(tFilename, nam);
  result = fopen(tFilename, "wb+");
  unlink(tFilename);
#endif
  return result;
}

/* Avoids the 'Broken pipe' message by reading the rest of the stream. */
void rsl_readflush(FILE *fp)
{
#ifndef _WIN32
  if (fork() == 0) { /* Child */
	char buf[1024];
	while(fread(buf, sizeof(char), sizeof(buf), fp)) continue;
	exit(0);
  }
#endif
}
	
int rsl_pclose(FILE *fp)
{
  int rc;
  if ((rc=pclose(fp)) == EOF) {
    //perror ("pclose");  /* This or fclose do the job. */
    if ((rc=fclose(fp)) == EOF) {
      //perror ("fclose");  /* This or fclose do the job. */
    }
  }
  return rc;
}

int no_command (char *cmd)
{
  int rc;
  /* Return 0 if there is the command 'cmd' on the system. */
  /* Return !0 otherwise. */
  rc = system(cmd);
  if (rc == 0) return rc;
  else return !0;
}

#ifdef NO_UNZIP_PIPE
FILE *uncompress_pipe (FILE *fp)
{
  FILE *retfp = NULL, *result = NULL;
  char buffer[CHUNK];
  int totallen = 0;
  gzFile gzfp = gzdopen(dup(fileno(fp)), "r");

  if (gzfp == Z_NULL) {
    return NULL;
  }

  retfp = create_temporary_file();

  if (retfp == NULL) {
    goto done;
  }

  for (;;) {
    int len = gzread(gzfp, buffer, sizeof(buffer));
    if (len <= 0)
      break;
    totallen += len;
    fwrite(buffer, 1, len, retfp);
  }

  fseek(retfp, 0, SEEK_SET);
  result = retfp;
  retfp = NULL;

done:
  if (retfp != NULL) {
    fclose(retfp);
  }
  fclose(fp);
  gzclose(gzfp);
  return result;
}
#else
FILE *uncompress_pipe (FILE *fp)
{
  /* Pass the file pointed to by 'fp' through the gzip pipe. */
  FILE *fpipe;
  int save_fd;
  if (no_command("gzip --version > /dev/null 2>&1")) return fp;
  save_fd = dup(0);
  close(0); /* Redirect stdin for gzip. */
  (void)dup(fileno(fp));
  fpipe = popen("gzip -q -d -f --stdout", "r");
  if (fpipe == NULL) perror("uncompress_pipe");
  close(0);
  (void)dup(save_fd);
  close(save_fd);
  return fpipe;
}
#endif

FILE *compress_pipe (FILE *fp)
{
  /* Pass the file pointed to by 'fp' through the gzip pipe. */

  FILE *fpipe;
  int save_fd;

  if (no_command("gzip --version > /dev/null 2>&1")) return fp;
  fflush(NULL); /* Flush all buffered output before opening this pipe. */
  save_fd = dup(1);
  close(1); /* Redirect stdout for gzip. */
  (void)dup(fileno(fp));

  fpipe = popen("gzip -q -1 -c", "w");
  if (fpipe == NULL) perror("compress_pipe");
  close(1);
  (void)dup(save_fd);
  return fpipe;
}

