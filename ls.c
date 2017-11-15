#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

char*
fmtname(char *path)
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

char *formatName(char *name) {
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

char *folderName(char *name) {
	int len = strlen(name);
	int secondLayer = 0;
	static char finalName[DIRSIZ+1];
	int i, lc = 0;
	for(i = 0; i < len; i++) {
		if(name[i] == '/' && secondLayer != 0) {
			break;
		}
		if(name[i] == '/' && secondLayer == 0) {
			secondLayer = 1;
		}
		finalName[lc] = name[i];
		lc = lc + 1;
	}
	return(finalName);
}

int wildcardStr(char *string, char *pattern) {
    if (pattern[0] == '\0' && string[0] == '\0') 
        return 1;

    if (pattern[0] == '*' && pattern[1] != '\0' && string[0] == '\0')
        return wildcardStr(string, pattern+1);

    if (pattern[0] == '?' && string[0] == '\0')
        return 0;

    if (pattern[0] == '?' || pattern[0] == string[0])
        return wildcardStr(string+1, pattern+1);

    if (pattern[0] == '*')
        return wildcardStr(string, pattern+1) || wildcardStr(string+1, pattern);
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
  // This case displays the ls when the argument is a file name
  case T_FILE:
    printf(1, "%s %d %d %d\n", fmtname(path), st.type, st.ino, st.size);
    break;

  // This case displays the ls when the argument is a folder name
  case T_DIR:
    printf(1, "FOLDER NAME\n");
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf(1, "ls: path too long\n");
      break;
    }
    strcpy(buf, path);
    //printf(1, "Buf: %s\n", buf);
    p = buf+strlen(buf);
    
    *p++ = '/';
    printf(1, "P: %s\n");
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      printf(1, "FILE NAME: %s\n",buf);
      if(stat(buf, &st) < 0){
        printf(1, "ls: cannot stat %s\n", buf);
        continue;
      }
      //printf(1, "P : %s\n", p);
      //printf(1, "Buf: %s Length: %d\n",buf, strlen(buf));
      printf(1, "%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
    }
    break;
  }
  close(fd);
}

void
lsWildCard(char *specifiedPath) {
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  char* path = folderName(specifiedPath);
  
  char* whatIEntered = specifiedPath;

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
  // This case displays the ls when the argument is a file name
  case T_FILE:
    //printf(1, "Buf: %s\t wie: %s\n",buf, whatIEntered);
    if(wildcardStr(formatName(buf), whatIEntered))
      printf(1, "%s %d %d %d\n", fmtname(path), st.type, st.ino, st.size);
    break;

  // This case displays the ls when the argument is a folder name
  case T_DIR:
    //printf(1, "FOLDER NAME 2\n");
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf(1, "ls: path too long\n");
      break;
    }
    strcpy(buf, path);
    //cd xvprintf(1, "Buf: %s\n", buf);
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
      //printf(1, "Buf: %s\t wie: %s\n",buf, whatIEntered);
      if(wildcardStr(formatName(buf), whatIEntered))
	ls(formatName(buf));       
	//printf(1, "%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
    }
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  int i;

  if(argc < 2){
    ls(".");
    exit();
  }
  for(i=1; i<argc; i++) {
    // Used strchr which is a function defined in ulib.c
    // strchr returns the position at which the character (*)
    // is present in the string (argv[i]) else returns 0
    if(strchr(argv[i], '*') || strchr(argv[i], '?')) {
      lsWildCard(argv[i]);
    }
    else {
      ls(argv[i]);
    }
  }
  exit();
}
