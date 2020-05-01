/*********** util.c file ****************/

int get_block(int dev, int blk, char *buf)
{
   lseek(dev, (long)blk*BLKSIZE, 0);
   read(dev, buf, BLKSIZE);
}   
int put_block(int dev, int blk, char *buf)
{
   lseek(dev, (long)blk*BLKSIZE, 0);
   write(dev, buf, BLKSIZE);
}   

int tokenize(char *pathname)
{
  int i;
  char *s;
  printf("tokenize %s\n", pathname);

  strcpy(gpath, pathname);   // tokens are in global gpath[ ]
  n = 0;

  s = strtok(gpath, "/");
  while(s){
    name[n] = s;
    n++;
    s = strtok(0, "/");
  }

  for (i= 0; i<n; i++)
    printf("%s  ", name[i]);
  printf("\n");
}

// return minode pointer to loaded INODE
MINODE *iget(int dev, int ino)
{
  int i;
  MINODE *mip;
  char buf[BLKSIZE];
  int blk, offset;
  INODE *ip;

  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->dev == dev && mip->ino == ino){
       mip->refCount++;
       //printf("found [%d %d] as minode[%d] in core\n", dev, ino, i);
       return mip;
    }
  }
    
  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount == 0){
       //printf("allocating NEW minode[%d] for [%d %d]\n", i, dev, ino);
       mip->refCount = 1;
       mip->dev = dev;
       mip->ino = ino;

       // get INODE of ino into buf[ ]    
       blk    = (ino-1)/8 + inode_start;
       offset = (ino-1) % 8;

       //printf("iget: ino=%d blk=%d offset=%d\n", ino, blk, offset);

       get_block(dev, blk, buf);
       ip = (INODE *)buf + offset;
       // copy INODE to mp->INODE
       mip->INODE = *ip;
       return mip;
    }
  }   
  printf("PANIC: no more free minodes\n");
  return 0;
}

void iput(MINODE *mip)
{
 int i, block, offset;
 char buf[BLKSIZE];
 INODE *ip;

 mip->refCount--;
 
 if (mip->refCount > 0)  // minode is still in use
    return;
 if (!mip->dirty)        // INODE has not changed; no need to write back
    return;
 
 /* write INODE back to disk */
 /***** NOTE *******************************************
  For mountroot, we never MODIFY any loaded INODE
                 so no need to write it back
  FOR LATER WROK: MUST write INODE back to disk if refCount==0 && DIRTY

  Write YOUR code here to write INODE back to disk
 ********************************************************/
} 

int search(MINODE *mip, char *name)
{
   char *cp, c, sbuf[BLKSIZE], temp[256];
   DIR *dp;
   INODE *ip;

   printf("search for %s in MINODE = [%d, %d]\n", name, mip->dev, mip->ino);
   ip = &(mip->INODE);

   /*** search for name in mip's data blocks: ASSUME i_block[0] ONLY ***/

   get_block(dev, ip->i_block[0], sbuf);
   dp = (DIR *)sbuf;
   cp = sbuf;
   printf("  ino   rlen  nlen  name\n");

   while (cp < sbuf + BLKSIZE){
     strncpy(temp, dp->name, dp->name_len);
     temp[dp->name_len] = 0;
     printf("%4d  %4d  %4d    %s\n", 
           dp->inode, dp->rec_len, dp->name_len, temp);
     if (strcmp(temp, name)==0){
        printf("found %s : ino = %d\n", temp, dp->inode);
        return dp->inode;
     }
     cp += dp->rec_len;
     dp = (DIR *)cp;
   }
   return 0;
}

int getino(char *pathname)
{
  int i, ino, blk, disp;
  char buf[BLKSIZE];
  INODE *ip;
  MINODE *mip;

  printf("getino: pathname=%s\n", pathname);
  if (strcmp(pathname, "/")==0)
      return 2;
  
  // starting mip = root OR CWD
  if (pathname[0]=='/')
     mip = root;
  else
     mip = running->cwd;

  mip->refCount++;         // because we iput(mip) later
  
  tokenize(pathname);

  for (i=0; i<n; i++){
      printf("===========================================\n");
      printf("getino: i=%d name[%d]=%s\n", i, i, name[i]);
 
      ino = search(mip, name[i]);

      if (ino==0){
         iput(mip);
         printf("name %s does not exist\n", name[i]);
         return 0;
      }
      iput(mip);                // release current mip
      mip = iget(dev, ino);     // get next mip
   }

  iput(mip);                   // release mip  
   return ino;
}

int findmyname(MINODE *parent, u32 myino, char *myname) 
{
  // WRITE YOUR code here:
  // search parent's data block for myino;
  // copy its name STRING to myname[ ];
 // printf("name 1\n");
  char buffer[BLKSIZE];
  char name[256];
  DIR *dp;
	char *cp;
  //get_block(dev, wd->INODE.i_block[0], buffer); 
  get_block(dev,parent->INODE.i_block[0],buffer);
  dp = (DIR *)buffer;
	cp = buffer;
	//printf("name 2\n");
	//This while loop breaks for some reason, adding a counter to limit it to 100 loops;
	int counter=0;
	while((cp<(buffer+BLKSIZE))&&(counter<100)){
	//	printf("Name while\nCount=%d\n",counter);
		if(dp->inode==myino){
		//	printf("Name while if\n");
			strncpy(name,dp->name,dp->name_len);//this uses the same concept as the other while loop, only now we KNOW the ino, so we can just find the TRUE name vs . or ..
	//		printf("after copy\n");
		//	printf("Name: %s\n",name);
				name[dp->name_len]=0;
		//		printf("Name 2: %s", name);
				strcpy(myname, name);
		//		printf("My name: %s", myname);
				//myname=name;
			//	printf("After copy 2\n");
				return 1;//could return anything, we jsut want to return, may need to use return 0 as a fail
			}
			cp += dp->rec_len;
				dp = (DIR *)cp;
				counter++;
		}
		return 0;
	
  
}

int findino(MINODE *mip, u32 *myino) // myino = ino of . return ino of .. aka parent
{
  char buf[BLKSIZE], *cp;   
  DIR *dp;

  get_block(mip->dev, mip->INODE.i_block[0], buf);
  cp = buf; 
  dp = (DIR *)buf;
  *myino = dp->inode;
  cp += dp->rec_len;
  dp = (DIR *)cp;
  return dp->inode;
}


/////////////////////Below given for mkdir_creat

int tst_bit(char buf[], int bit) // chapter 11.3.1
{
	return buf[bit/8] & (1 << (bit%8));
}
int set_bit(char buf[], int bit)//chapter 11.3.2
{
	return buf[bit/8] |= (1 << (bit%8));
}

int ialloc(int dev)  // allocate an inode number from inode_bitmap
{
  int  i;
  char buf[BLKSIZE];

// read inode_bitmap block
  get_block(dev, imap, buf);

  for (i=0; i < ninodes; i++){
    if (tst_bit(buf, i)==0){
        set_bit(buf, i);
        put_block(dev, imap, buf);
        printf("allocated ino = %d\n", i+1); // bits count from 0; ino from 1
        return i+1;
    }
  }
  return 0;
}


int balloc(int dev){ // the same as ialloc but change imap to bmap and ninode to nblocks
  int  i;
  char buf[BLKSIZE];

// read inode_bitmap block
  get_block(dev, bmap, buf);

  for (i=0; i < nblocks; i++){
    if (tst_bit(buf, i)==0){
        set_bit(buf, i);
        put_block(dev, bmap, buf);
        //printf("allocated ino = %d\n", i+1); // bits count from 0; ino from 1
        return i+1;
    }
  }
	printf("FS PANIC: out of INODES\n");
	return 0;
}



////////////////////////Below given for rmdir

int idalloc(int dev, int ino)  // deallocate an ino number
{
  int i;  
  char buf[BLKSIZE];

  if (ino > ninodes){
    printf("inumber %d out of range\n", ino);
    return 0;
  }

  // get inode bitmap block
  get_block(dev, imap, buf);
  clr_bit(buf, ino-1);

  // write buf back
  put_block(dev, imap, buf);
}
//clrbit is from page 338 in textbook
int clr_bit(char *buf, int bit){
	return buf[bit/8] &= ~(1<<(bit %8));
}


int bdalloc(int dev, int blk) // deallocate a blk number
{
  int i;
  char buf[BLKSIZE];
  
  get_block(dev, bmap, buf);
  clr_bit(buf, blk-1);
  put_block(dev, bmap, buf);
  
  
  
}

