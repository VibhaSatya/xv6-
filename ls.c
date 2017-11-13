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
int match(char *string, char *pattern) {
    //printf(1,"\nstr %s pat %s\n",string,pattern);
    if (pattern[0] == '\0' && string[0] == '\0') 
        {return 1;}

    if (pattern[0] == '*' && pattern[1] != '\0' && string[0] == '\0')
        return match(string, pattern+1);

    if (pattern[0] == '?' || pattern[0] == string[0])
        return match(string+1, pattern+1);

    if (pattern[0] == '*')
        return match(string, pattern+1) || match(string+1, pattern);
    return 0;
}




void
ls(char *path)
{
  //printf(1,"Inside ls");
  char buf[512], *p;
  int fd,index;
  struct dirent de;
  struct stat st;
//######################################################################################################
//in case input has * 
  int flag=0;
  for(index=strlen(path)-1; index>=0; index--)
        {
         if(path[index]=='*'||path[index]=='?') {flag=1;break;}
        }
 
  if(*path=='*') {path[0]='.';path[1]='\0';flag=0;}
  if(flag)
 {
        //printf(1,"Inside if");
        int k=0;
  	
        char name[512];
        strcpy(name,fmtname(path));
        //printf(1,"\nbefore loop\n");
        int check=0;
	for(index=strlen(path)-1; index>=0 ; index--)
        {
         if(path[index]=='/') {check=1;break;}
         k++;
         //printf(1," %d ",k);
        }
        if(check==0) 
          {
           path[0]='.';
           path[1]='\0';
           } 
       // printf(1,"\nbefore null\n");
        else
        *(path+(strlen(path)-k-1))='\0';
        //printf(1,"\nafter loop\n");
 	// printf(1,"\n path %s\n",(path));
   	if((fd = open(path, 0)) < 0){
    	    printf(2, "ls: cannot open %s\n", path);
    	    return;}
  	if(fstat(fd, &st) < 0){
    	    printf(2, "ls: cannot stat %s\n", path);
    	    close(fd);
    	    return;}

    	if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      	    printf(1, "ls: path too long\n");
            return;}
    
    strcpy(buf, path);
    p = buf+strlen(buf);
    
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      //printf(1,"\n%s  %s\n",name,de.name);
      //printf(1," %d ",strlen(de.name));
      int i;
      for(i=0;i<strlen(name);i++) 
      { 
	if(name[i]==' ') {name[i]='\0';break;}
	//printf(1,"%c",name[i]);
      }

      if(match(de.name,name)) printf(1,"\n%s\n",de.name);
      if(stat(buf, &st) < 0){
        printf(1, "ls: cannot stat %s\n", buf);
        continue;
      }
      //printf(1, "%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
     
     
  }

  close(fd);
  return;
 }

//######################################################################################################
//no wildcard
else
  {
  //printf(1,"\npath %s\n\n",path);
  //printf(1,"Inside else");
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
//######################################################################################################
 
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
    ls(argv[i]);
  exit();
}
