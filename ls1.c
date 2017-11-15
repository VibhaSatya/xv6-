#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

char* fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}

char *getFileName(char *name) 
{
	int len = strlen(name);
	static char finalName[DIRSIZ+1];
  int i, lc = 0;
  
	for(i = 0; i < len; i++) {
		if(name[i] == '/') {
			lc = 0;
			memset(finalName, 0, DIRSIZ+1);
		}
		else {
			finalName[lc] = name[i];
			lc = lc + 1;
		}
	}
	return(finalName);
}

int wildcardMatch(char *string, char *pattern) 
{
  if (pattern[0] == '\0' && string[0] == '\0') 
    return 1;

  if (pattern[0] == '*' && pattern[1] != '\0' && string[0] == '\0')
    return wildcardMatch(string, pattern+1);

  if (pattern[0] == '?' && string[0] == '\0')
    return 0;

  if (pattern[0] == '?' || pattern[0] == string[0])
    return wildcardMatch(string+1, pattern+1);

  if (pattern[0] == '*')
    return wildcardMatch(string, pattern+1) || wildcardMatch(string+1, pattern);
  return 0;
}

void
ls(char *path)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

    if((fd = open(path, 0)) < 0){
      printf(2, "ls: cannot open %s\n", path);
      return;
    }

    if(fstat(fd, &st) < 0){
      printf(2, "ls: cannot stat %s\n", path);
      close(fd);
      return;
    }

  
  switch(st.type){
    case T_FILE:
      printf(1, "%s %d %d %d\n", fmtname(path), st.type, st.ino, st.size);
      break;

    case T_DIR:
      if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
        printf(1, "ls: path too long\n");
        break;
      }
      strcpy(buf, path);
      p = buf+strlen(buf);
      *p++ = '/';
      while(read(fd, &de, sizeof(de)) == sizeof(de)){
        if(de.inum == 0)
          continue;
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;
        if(stat(buf, &st) < 0){
          printf(1, "ls: cannot stat %s\n", buf);
          continue;
        }
        printf(1, "%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
      }
      break;
  }
  close(fd);
  return;
}

void
lsWildcard(char *path)
{
  char buf[512], *p;
  int fd, index, i;
  struct dirent de;
  struct stat st;

  int k=0, check=0;
  char name[512];
  strcpy(name, getFileName(path));

  for(index=strlen(path)-1; index>=0 ; index--) {
    if(path[index]=='/') {
      check=1;
      break;
    }
    k++;
  }

  if(check==0) {
    path[0]='.';
    path[1]='\0';
  }
  else {
    *(path+(strlen(path)-k-1))='\0';
  }

  if((fd = open(path, 0)) < 0) {
    printf(2, "ls: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    printf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  strcpy(buf, path);
  p = buf+strlen(buf);
  *p++ = '/';

  while(read(fd, &de, sizeof(de)) == sizeof(de)){
    if(de.inum == 0)
      continue;

    memmove(p, de.name, DIRSIZ);
    p[DIRSIZ] = 0;

    for(i=0; i < strlen(name); i++) { 
      if(name[i]==' ') {
        name[i]='\0';
        break;
      }
    }

    if(wildcardMatch(getFileName(buf),name)) {
      if(stat(buf, &st) < 0){
        printf(1, "ls: cannot stat %s\n", buf);
        continue;
      }
      if(st.type == T_DIR && getFileName(buf)[0]!='.') {
	printf(1, "%s\n", getFileName(buf));
        ls(buf);
        printf(1, "\n");
      }
      else
        printf(1, "%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
    }
  }
  close(fd);
  return;
}

int
main(int argc, char *argv[])
{
  int i;

  if(argc < 2){
    ls(".");
    exit();
  }
  for(i=1; i<argc; i++)
    if(strchr(argv[i], '*') || strchr(argv[i], '?')) {
      lsWildcard(argv[i]);
    }
    else {
      ls(argv[i]);
    }
  exit();
}
