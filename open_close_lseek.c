/*********** open_close_lseek.c file ****************/



int truncate(MINODE* mip);
int close_file(int fd);
int lseek(int fd, int position);
int pfd();
int dup(int fd);
int dup2(int fd, int gd);





int open_file(char *filename, char *third){
	//1. ask for a pathname and mode to open:
      //   You may use mode = 0|1|2|3 for R|W|RW|APPEND
	
	int mode =-1;
	int ino=0;
	MINODE *mip;
	INODE* ip;
	
	
	
	if(strcmp(filename,"")==0){
		printf("Error with filename\n");
		return 0;
	}
	if(strcmp(third,"")==0){
		printf("Error with mode\n");
		return 0;
	}
	
	if(strcmp(third,"0")!=0&&strcmp(third,"1")!=0&&strcmp(third,"2")!=0&&strcmp(third,"3")!=0){
		printf("Incorrect mode\n");
		return 0;
	}
	mode=atoi(third);
	//now mode holds our int value and filename should be the file to open
	
	
	/*
	  2. get pathname's inumber:
         if (pathname[0]=='/') dev = root->dev;          // root INODE's dev
         else                  dev = running->cwd->dev;  
         ino = getino(pathname); 
	
	*/
	 if (filename[0]=='/') {
		 dev = root->dev;
		 }          // root INODE's dev
         else {
			 dev = running->cwd->dev;
		 }
         ino = getino(filename); 
	
	printf("File INO: %d\n", ino);
	if(ino==0){
		printf("Error with ino, no file\n");
		return 0;
	}
	
	/*
	  3. get its Minode pointer
         mip = iget(dev, ino);
	*/
	mip = iget(dev, ino);
	ip=&mip->INODE;
	
	//4. check mip->INODE.i_mode to verify it's a REGULAR file and permission OK.
	
	if(!S_ISREG(ip->i_mode)){
		printf("Error: %s is not a file\n", filename);
		return 0;
	}
	
	//make sure its not open
	
	//can't find any information about this in:
	//http://man7.org/linux/man-pages/man7/inode.7.html
	
	/*
	running is a PROC* and proc has the following
	typedef struct proc{
  struct proc *next;
  int          pid;
  int          status;
  int          uid, gid;
  MINODE      *cwd;
  OFT         *fd[NFD];
}PROC;
	
	fd goes from 0-15, as NFD is 16, this may be what we need to check.
	fd is an OFT*, this contains
	
	typedef struct oft{
  int  mode;
  int  refCount;
  MINODE *mptr;
  int  offset;
}OFT;
	
	
	*/
for(int i=0;i<NFD;i++){
	if(running->fd[i]!=0){//make sure something is here worth checking
		if(running->fd[i]->mptr==mip){// if our mip matches an mip in running, then its open! I think...
			printf("Error: File already open\n");
			return 0;
		}
	}
}

	/*
	5. allocate a FREE OpenFileTable (OFT) and fill in values:
 
         oftp->mode = mode;      // mode = 0|1|2|3 for R|W|RW|APPEND 
         oftp->refCount = 1;
         oftp->minodePtr = mip;  // point at the file's minode[]
	
	*/
	
OFT* oftp = (OFT*)malloc(sizeof(OFT));
oftp->mode=mode;
oftp->refCount=1;
oftp->mptr=mip;//type.h has it listed differently!
	
	/*
	6. Depending on the open mode 0|1|2|3, set the OFT's offset accordingly:

      switch(mode){
         case 0 : oftp->offset = 0;     // R: offset = 0
                  break;
         case 1 : truncate(mip);        // W: truncate file to 0 size
                  oftp->offset = 0;
                  break;
         case 2 : oftp->offset = 0;     // RW: do NOT truncate file
                  break;
         case 3 : oftp->offset =  mip->INODE.i_size;  // APPEND mode
                  break;
         default: printf("invalid mode\n");
                  return(-1);
      }
	
	
	*/
	switch(mode){
         case 0 : oftp->offset = 0;     // R: offset = 0
                  break;
         case 1 : truncate(mip);        // W: truncate file to 0 size
                  oftp->offset = 0;
                  break;
         case 2 : oftp->offset = 0;     // RW: do NOT truncate file
                  break;
         case 3 : oftp->offset =  mip->INODE.i_size;  // APPEND mode
                  break;
         default: printf("invalid mode\n");
                  return(-1);
      }
	
		/*
		7. find the SMALLEST i in running PROC's fd[ ] such that fd[i] is NULL
      Let running->fd[i] point at the OFT entry
		*/
	int i=0;
	for(i=0;i<NFD;i++){
		if(running->fd[i]==0){//we hit an empty spot
		running->fd[i]=oftp;
		break;
		}
	}
	if(i==NFD){
		printf("Error: reached end of FD, not adding file\n");
		free(oftp);
		return 0;
	}
	/*
	8. update INODE's time field
         for R: touch atime. 
         for W|RW|APPEND mode : touch atime and mtime
      mark Minode[ ] dirty
	*/
	  //   You may use mode = 0|1|2|3 for R|W|RW|APPEND
	  if(mode==0){
		  ip->i_atime=time(0L);
	  }
	  else{
		  ip->i_atime=time(0L);
		  ip->i_mtime=time(0L);
	  }
	  mip->dirty=1;
	//no mention of iput()
	//9. return i as the file descriptor
	
	printf("All done opening\n");
	return i;
	
	
}
	
int truncate(MINODE* mip){
	/*1. release mip->INODE's data blocks;
     a file may have 12 direct blocks, 256 indirect blocks and 256*256
     double indirect data blocks. release them all.
  2. update INODE's time field

  3. set INODE's size to 0 and mark Minode[ ] dirty
	*/
	int i=0;
	while(mip->INODE.i_block[i]!=0){//im assuming there inst data, a 0 then more data
		mip->INODE.i_block[i]=0;//i dont know the size limit, so I have to make this assumption
		i++;
		}
		
		/*create file has the size at 14, this may need to be used later
		for(int i=1;i<15;i++){
		ip->i_block[i]=0;
		}
		*/
		mip->INODE.i_size=0;
		mip->dirty=1;
		mip->INODE.i_atime=time(0L);
		mip->INODE.i_mtime=time(0L);
	}

	
	int close_file(int fd)
{
  /*1. verify fd is within range.

  2. verify running->fd[fd] is pointing at a OFT entry

  3. The following code segments should be fairly obvious:
     oftp = running->fd[fd];
     running->fd[fd] = 0;
     oftp->refCount--;
     if (oftp->refCount > 0) return 0;

     // last user of this OFT entry ==> dispose of the Minode[]
     mip = oftp->inodeptr;
     iput(mip);

     return 0; */
	 printf("Attempting to close: %d\n", fd);
	 if(fd>=NFD||fd<0){
		 printf("ERROR: Outside of range\n");
		 return 0;
	 }
	 if(running->fd[fd]==0){
		 printf("Error: not active file\n");
		 return 0;
		 
	 }
	 OFT *oftp;
	 oftp = running->fd[fd];
     running->fd[fd] = 0;
     oftp->refCount--;
if (oftp->refCount > 0) {return 0;}

MINODE * mip=oftp->mptr;//type.h has MINODE *mptr, this may need altered(IT DID)
iput(mip);
printf("Done with closing\n");
	 return 0;
}



int my_lseek(int fd, int position)//using lseek as the fn name was causing a weird core dump after loading the file
{
 /* From fd, find the OFT entry. 

  change OFT entry's offset to position but make sure NOT to over run either end
  of the file.

  return originalPosition*/
  printf("LSEEK\n");
  printf("Moving from %d to %d\n", fd, position);
  OFT* oftp=running->fd[fd];
  if(position>=NFD||position<0){//using the same logic as shown in close!
	  printf("Error: Wrong position given\n");
	  return 0;
	  
  }
  oftp->offset=position;
  printf("Seccess!\n");
  return 0;
}



int pfd()
{
  /*This function displays the currently opened files as follows:

        fd     mode    offset    INODE
       ----    ----    ------   --------
         0     READ    1234   [dev, ino]  
         1     WRITE      0   [dev, ino]
      --------------------------------------
  to help the user know what files has been opened.*/
  
  printf("fd     mode    offset    INODE\n");
  printf("-------------------------------------------\n");
  
  
  OFT *oftp;
  int i=0;
  
  char mode[10];
  for(i=0;i<NFD;i++){
	  oftp=running->fd[i];
	  if(oftp==0){
		  continue;
	  }
	  if(oftp->refCount>=1){
		  if(oftp->mode==0){
			  strcpy(mode, "READ");
		  }
		  else if(oftp->mode==1){
			  strcpy(mode, "WRITE");
		  }
		  else if(oftp->mode==2){
			  strcpy(mode, "RW");
		  }
		  else if(oftp->mode==3){
			  strcpy(mode, "APPEND");
		  }
		  
		  printf("%d    %s    %.4d    [%d, %d]\n", i, mode, oftp->offset, oftp->mptr->dev, oftp->mptr->ino);
		  
		  
		  
	  }
	  
	  
  }
    printf("-------------------------------------------\n");
}




dup(int fd) 
{
 /* verify fd is an opened descriptor;
  duplicates (copy) fd[fd] into FIRST empty fd[ ] slot;
  increment OFT's refCount by 1;
  */
  OFT* oftp;
  if(fd<NFD||fd>=0){
	  if(running->fd[fd]!=0){
		  //good to go
		  int i=0;
		  while(i<NFD&&running->fd[i]!=0){
			  i++;
		  }
		  if(i>=NFD){
			  printf("No dup\n");
			  return 0;
		  }
		  if(running->fd[i]==0){
			  running->fd[i]=running->fd[fd];
			  oftp=running->fd[i];
			  oftp->refCount++;
			  return 0;
		  }
		  else{
			  printf("No dup, case 2\n");
			  return 0;
		  }
	  }
  }
  
}

dup2(int fd, int gd)
{
	/*
  CLOSE gd fisrt if it's already opened;
  duplicates fd[fd] into fd[gd]; 
  */
  if(running->fd[fd]!=0){
  OFT* temp=running->fd[gd];
  OFT* new=running->fd[fd];
  
  if(temp!=0){
	free(temp);
  }
  running->fd[gd]=new;
  new->refCount++;
  }
  else{
	  printf("Can't dup2\n");
  }
  return 0;
}
















	

