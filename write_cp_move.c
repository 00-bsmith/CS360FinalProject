/*write_cp_move.c*/

//this file contains the write, my_write, cp, and mv functions needed for level 2



/*
int write_file()
{
  1. Preprations:
     ask for a fd   and   a text string to write;

  2. verify fd is indeed opened for WR or RW or APPEND mode

  3. copy the text string into a buf[] and get its length as nbytes.

     return(mywrite(fd, buf, nbytes));
}


*/
int my_write(int fd, char buf[ ], int nbytes);

int write_file(){//this one is called by main.c, nothing is passed into it
	int fd;
	char buf[128];//just going to use 128 as thats teh size used everywhere else
	char line[128];	
	printf("Please give the fd you wish to access\n");
	
	fgets(line, 128, stdin);
    line[strlen(line)-1] = 0;

   
      
    buf[0]=0;

   // sscanf(line, "%s %s", cmd, pathname);-------Original scanf
   sscanf(line, "%s", buf);
   fd=atoi(buf);
    printf("Attempting to write to %d\n", fd);
	
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
	// mode = 0|1|2|3 for R|W|RW|APPEND
	//1, 2, or 3 are all viable
	
	if(oftp->mode==0||oftp->mode>3){
		printf("File not opened with Write permissions\nOpen with: %d", oftp->mode);
		return 0;
	}
	
	MINODE *mip=oftp->mptr;
	printf("Ready to write to file, please input what you wish to write:\n");
	line[0]=0;
	fgets(line, 128, stdin);
    line[strlen(line)-1] = 0;

 
	return my_write(fd, line, strlen(line));
	
	
}

/*
int mywrite(int fd, char buf[ ], int nbytes) 
{
  while (nbytes > 0 ){

     compute LOGICAL BLOCK (lbk) and the startByte in that lbk:

          lbk       = oftp->offset / BLKSIZE;
          startByte = oftp->offset % BLKSIZE;

    // I only show how to write DIRECT data blocks, you figure out how to 
    // write indirect and double-indirect blocks.

     if (lbk < 12){                         // direct block
        if (ip->INODE.i_block[lbk] == 0){   // if no data block yet

           mip->INODE.i_block[lbk] = balloc(mip->dev);// MUST ALLOCATE a block
        }
        blk = mip->INODE.i_block[lbk];      // blk should be a disk block now
     }
     else if (lbk >= 12 && lbk < 256 + 12){ // INDIRECT blocks 
              // HELP INFO:
              if (i_block[12] == 0){
                  allocate a block for it;
                  zero out the block on disk !!!!
     // NOTE: you may modify balloc() to zero out the allocated block on disk
              }
              get i_block[12] into an int ibuf[256];
              blk = ibuf[lbk - 12];
              if (blk==0){
                 allocate a disk block;
                 record it in i_block[12];
              }
              .......
     }
     else{
            // double indirect blocks 
     }

      all cases come to here : write to the data block 
     get_block(mip->dev, blk, wbuf);   // read disk block into wbuf[ ]  
     char *cp = wbuf + startByte;      // cp points at startByte in wbuf[]
     remain = BLKSIZE - startByte;     // number of BYTEs remain in this block

     while (remain > 0){               // write as much as remain allows  
           *cp++ = *cq++;              // cq points at buf[ ]
           nbytes--; remain--;         // dec counts
           oftp->offset++;             // advance offset
           if (offset > INODE.i_size)  // especially for RW|APPEND mode
               mip->INODE.i_size++;    // inc file size (if offset > fileSize)
           if (nbytes <= 0) break;     // if already nbytes, break
     }
     put_block(mip->dev, blk, wbuf);   // write wbuf[ ] to disk
     
     // loop back to outer while to write more .... until nbytes are written
  }

  mip->dirty = 1;       // mark mip dirty for iput() 
  printf("wrote %d char into file descriptor fd=%d\n", nbytes, fd);           
  return nbytes;
}


*/






int my_write(int fd, char buf[ ], int nbytes){
	OFT *oftp=running->fd[fd];
	
	char  write_buf[1024];
	MINODE *mip=oftp->mptr;
	INODE *ip=&mip->INODE;//getting the basics setup
	
	int new_size=nbytes;
	
	int lbk;
	int startByte;
	int blk;
	
	int * int_p;
	
	int ind_blk;//ind block location
	int ind_blk_ofs;//offset of ind blk
	
	int remain=0, counter=0;
	char *cp;
	char *cq=buf;
	
	
	
	if(oftp->mode==3){
		//apend mode, so add onto file, dont overwrite
		new_size+=ip->i_size;
	}
	//double check file is good to go, check MIP and IP & oftp
	
	if (oftp == 0){
		printf("oftp is null\n"); 
		return;
	}
	else if (mip == 0){
		printf("mip is null\n"); 
		return;
	}
	else if (ip == 0){
		printf("ip is null\n"); 
		return;
	}
	
	printf("Entering loop to write to file!\n");
	//starting logic from above
	while(nbytes>0){
		// compute LOGICAL BLOCK (lbk) and the startByte in that lbk:
		lbk  = oftp->offset / BLKSIZE;
        startByte = oftp->offset % BLKSIZE;
		
		
		if (lbk < 12){                         // direct block
		printf("Direct Block Writing\n");
        if (mip->INODE.i_block[lbk] == 0){   // if no data block yet
		
           mip->INODE.i_block[lbk] = balloc(mip->dev);// MUST ALLOCATE a block
        }
        blk = mip->INODE.i_block[lbk];      // blk should be a disk block now
     }
	 
	 
     else if (lbk >= 12 && lbk < 256 + 12){ // INDIRECT blocks 
          printf("Indirect Block Writing\n");
		  if(!ip->i_block[12]){//no block at 12
			  ip->i_block[12]=balloc(mip->dev);//allocate
			  //set to 0
			  get_block(mip->dev, ip->i_block[12], write_buf);
			  
			  for(int i=0;i<BLKSIZE;i++){
				  write_buf[i]=0;
			  }
			  put_block(mip->dev, ip->i_block[12], write_buf);
		  }
		  //get the block thats there or the one we wrote
		  
		  
		  
		  get_block(mip->dev, ip->i_block[12], write_buf);
		  /* get i_block[12] into an int ibuf[256];
              blk = ibuf[lbk - 12];
              if (blk==0){
                 allocate a disk block;
                 record it in i_block[12];
				 */
		  
		  
		  
		  int_p=(int*)write_buf+lbk-12;
		  blk=*int_p;
		  
		  //allocate if blank
		  if(blk==0){
			  *int_p=balloc(mip->dev);
			  blk=*int_p;
			  put_block(mip->dev, blk, write_buf);//may not be needed
		  }
     }
	 
	
     else{
            // double indirect blocks 
			//this is block 13, but works a bit dif from 12
			//should follow same principal as read
			if(ip->i_block[13]==0){
				//allocate
				ip->i_block[13]=balloc(mip->dev);
				get_block(mip->dev, ip->i_block[13], write_buf);
				for(int i=0; i<BLKSIZE; i++){
					write_buf[i]=0;
				}
				put_block(mip->dev, ip->i_block[13], write_buf);
			}
			//get the block thats there or we made
			get_block(mip->dev, ip->i_block[13], write_buf);
			
			ind_blk=(lbk-256-12)/256;
			ind_blk_ofs=(lbk-256-12)%256;
			
			int_p=(int*)write_buf+ind_blk;
			blk=*int_p;
			
			if(blk==0){
				//need to allocate if missing
				int_p=balloc(mip->dev);
				blk = *int_p;
				put_block(mip->dev, blk, write_buf);//might be unneeded
			}
			
     }
		
		/*
		  all cases come to here : write to the data block 
     get_block(mip->dev, blk, wbuf);   // read disk block into wbuf[ ]  
     char *cp = wbuf + startByte;      // cp points at startByte in wbuf[]
     remain = BLKSIZE - startByte;     // number of BYTEs remain in this block

     while (remain > 0){               // write as much as remain allows  
           *cp++ = *cq++;              // cq points at buf[ ]
           nbytes--; remain--;         // dec counts
           oftp->offset++;             // advance offset
           if (offset > INODE.i_size)  // especially for RW|APPEND mode
               mip->INODE.i_size++;    // inc file size (if offset > fileSize)
           if (nbytes <= 0) break;     // if already nbytes, break
     }
     put_block(mip->dev, blk, wbuf);   // write wbuf[ ] to disk
     
     // loop back to outer while to write more .... until nbytes are written
		
		*/
		get_block(mip->dev, blk, write_buf);
		cp = write_buf + startByte;
		remain = BLKSIZE - startByte;
		
		while(remain > 0)
		{
			*cp++ = *cq++;
			nbytes--;
			counter++;
			remain--;
			oftp->offset++;
			if(oftp->offset > mip->INODE.i_size)
				mip->INODE.i_size++;
			if(nbytes <= 0)
				break;
		}
		
		put_block(mip->dev, blk, write_buf);
		
		
	}
	
/*	 mip->dirty = 1;       // mark mip dirty for iput() 
  printf("wrote %d char into file descriptor fd=%d\n", nbytes, fd);           
  return nbytes;
	*/
	mip->dirty=1;
	printf("wrote %d char into file descriptor fd=%d\n", nbytes, fd);           
  return nbytes;
	
	
}



/*
cp src dest:

1. fd = open src for READ;

2. gd = open dst for WR|CREAT; 

   NOTE:In the project, you may have to creat the dst file first, then open it 
        for WR, OR  if open fails due to no file yet, creat it and then open it
        for WR.

3. while( n=read(fd, buf[ ], BLKSIZE) ){
       write(gd, buf, n);  // notice the n in write()
   }


*/

int cp(char *pathname, char * destname){//this will use all 3 from main.c
	int fdSrc=0;
	int fdDest=0;
	
	char buf[1024], src[1024], dest[1024];
	
	MINODE *mip;
	INODE * ip;
	
	printf("Copying from %s, to %s\n", pathname, destname);
	
	//verify valid inputs!
	if(strcmp(pathname,"")==0){
		printf("Error: Null pathname, retyr\n");
		return 0;
	}
	if(strcmp(destname, "")==0){
		printf("Error: Null destname, please retry\n");
		return 0;
	}
	//so a simple check to see if a file exists
	//getino should return an ino if a file of the name exists
	//if it return 0, no file exists, if this is the case, we need to call creat to make one
	if(getino(pathname)==0){
		printf("Error, %s DNE\n", pathname);
		return 0;
	}
	
	if(getino(destname)==0){
		//dest needs to be made
		creat_file(destname);
	}
	//now both should be valid, lets open them
	
	fdSrc=open_file(pathname, "0");//read is mode 0
	fdDest=open_file(destname, "2");//mode 2 is RW, this should allow full copy.
	
	printf("Openned Src: %d\nOpenned Dest: %d\nBeginning Copy\n", fdSrc, fdDest);
	pfd();
	int i=0;
	while(i=my_read(fdSrc, buf, BLKSIZE)){
		my_write(fdDest, buf, i);
	}//this loop will run until read stops returning values!
	
	close_file(fdSrc);
	close_file(fdDest);
	printf("Done Copying!\n");
	return;
	
}


/*
mv src dest:

1. verify src exists; get its INODE in ==> you already know its dev
2. check whether src is on the same dev as src

              CASE 1: same dev:
3. Hard link dst with src (i.e. same INODE number)
4. unlink src (i.e. rm src name from its parent directory and reduce INODE's
               link count by 1).
                
              CASE 2: not the same dev:
3. cp src to dst
4. unlink src

*/



int mv(char *pathname, char * destname){//this will use all 3 from main.c

int fdSrc=0;
	int fdDest=0;
	
	char buf[1024], src[1024], dest[1024];
	
	MINODE *mip;
	INODE * ip;
	
	printf("Moving from %s, to %s\n", pathname, destname);
	
	//verify valid inputs!
	if(strcmp(pathname,"")==0){
		printf("Error: Null pathname, retry\n");
		return 0;
	}
	if(strcmp(destname, "")==0){
		printf("Error: Null destname, please retry\n");
		return 0;
	}
	//so a simple check to see if a file exists
	//getino should return an ino if a file of the name exists
	//if it return 0, no file exists, if this is the case, we need to call creat to make one
	if(getino(pathname)==0){
		printf("Error, %s DNE\n", pathname);
		return 0;
	}
	//valid dest and valid source, lets check if dest exists
// i dont think i use seperate dev at any point, so no need to check

//int link(char*pathname, char* third)
printf("About to move %s into %s\n", pathname, destname);

link(pathname, destname);
unlink(pathname);
printf("Done\n");
//note to self, if above doesnt work, just call cp(path, dest), then unlink source
//note, unlink is refusing to work now, even though its identical to what was used in level 1 sub
}























