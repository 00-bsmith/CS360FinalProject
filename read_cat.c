/////////////////////read_cat.c


my_read(int fd,char* buf,int nbytes);
int cat_file(char* filename);

//notes:
//read seems to crash on the larger files, unsure as tp the reason. but tiny and small both read, althougj
//small does get a stack smash error
//the others just core dump after verifying read rights

//CAT works but openning and reading does not...






int read_file()
{
  /*Preparations: 
    ASSUME: file is opened for RD or RW;
    ask for a fd  and  nbytes to read;
    verify that fd is indeed opened for RD or RW;
    return(myread(fd, buf, nbytes));
	*/
	char line[128];
	printf("Please give an fd and nbytes to read:\n");
	 fgets(line, 128, stdin);
    line[strlen(line)-1] = 0;

    if (line[0]==0){
		printf("No input given\n");
	}
       
   char fd_c[10];
   char n_c[10];

   // sscanf(line, "%s %s", cmd, pathname);-------Original scanf
   sscanf(line, "%s %s", fd_c, n_c);
    printf("fd=%s, n=%s\n", fd_c, n_c);
	
	int fd= atoi(fd_c);
	int nbytes=(n_c);
	if(fd!=0&&fd!=1&&fd!=2&&fd!=3){
		printf("No FD given\n");
		return 0;
	}
	if(nbytes==0){
		printf("No bytes given!\n");
		return 0;
	}
	OFT *oftp;
//	printf("After checks\n");
	int i;
	//char buf[nbytes+1];//the file will be read into this buffer to be stored, then printed from here!
	char buf[1024];//a dynamic string is causing crashes
//	printf("before ifs\n");
	if(fd>=NFD||fd<0){
		printf("Error: fd out of bounds!\n");
		return 0;
	}
//	printf("if 1\n");
	oftp=running->fd[fd];
	if(oftp==0){
		printf("Not an active fd!\n");
		return 0;
	}
	//printf("if 2\n");
	//now we know oftp is valid, lets check the rights!
	//oftp->mode = mode;      // mode = 0|1|2|3 for R|W|RW|APPEND 
	//we need 0, or 2 for read permissions!
	if(oftp->mode==0||oftp->mode==2){
		printf("You have read permissions!\n");
	}
	
	else{
		printf("No perms to read\n");
		return 0;
	}
	//now we can do the return, and move on to my_read
//	int count=my_read(fd, buf, nbytes);
	//I want to print the file contents here
	//im going to try slapping a null ptr on the end of the buffer
	//buf[1024]="\0";
//	printf("%s\n", buf);
	
	//return(count);

//CAT works and uses
/*
	while(n=my_read(fd, mybuf, 1024)){
		mybuf[n]=0;
		printf("%s", mybuf);
	}
*/
int count;
	while(count=my_read(fd, buf, nbytes)){
		buf[count]=0;
		printf("%s", buf);
	}
return count;
}


my_read(int fd,char* buf,int nbytes){
	/*
	1. int count = 0;
    avil = fileSize - OFT's offset // number of bytes still available in file.
    char *cq = buf;                // cq points at buf[ ]

 2. while (nbytes && avil){

       Compute LOGICAL BLOCK number lbk and startByte in that block from offset;

             lbk       = oftp->offset / BLKSIZE;
             startByte = oftp->offset % BLKSIZE;
     
       // I only show how to read DIRECT BLOCKS. YOU do INDIRECT and D_INDIRECT
 
       if (lbk < 12){                     // lbk is a direct block
           blk = mip->INODE.i_block[lbk]; // map LOGICAL lbk to PHYSICAL blk
       }
       else if (lbk >= 12 && lbk < 256 + 12) { 
            //  indirect blocks 
       }
       else{ 
            //  double indirect blocks
       } 

       // get the data block into readbuf[BLKSIZE] 
       get_block(mip->dev, blk, readbuf);

       // copy from startByte to buf[ ], at most remain bytes in this block 
       char *cp = readbuf + startByte;   
       remain = BLKSIZE - startByte;   // number of bytes remain in readbuf[]

       while (remain > 0){
            *cq++ = *cp++;             // copy byte from readbuf[] into buf[]
             oftp->offset++;           // advance offset 
             count++;                  // inc count as number of bytes read
             avil--; nbytes--;  remain--;
             if (nbytes <= 0 || avil <= 0) 
                 break;
       }
 
       // if one data block is not enough, loop back to OUTER while for more ...

   }
   printf("myread: read %d char from file descriptor %d\n", count, fd);  
   return count;   // count is the actual number of bytes read
	
	
	
	*/
	int count = 0;
	char *cq=buf;
	int avil;//need to access minodes for remaining space
	
	
	
	
	OFT* oftp=running->fd[fd];
	MINODE *mip=oftp->mptr;
	INODE* ip=&mip->INODE;//better than mip->INODE personally
	
	avil=ip->i_size-oftp->offset;
	
	
	int lbk, startByte;
	int blk=0;
	int remain;
	
	
	
	int* int_p;
	int ind_blk;//ind block location
	int ind_blk_ofs;//offset of ind blk
	
	
	
	
	char readBuf[1024];
	
	//move to step 2
	while (nbytes && avil){
		//PAGE 234 EXPLAINS DIRECT INDIRECT AND DOUBLE INDIRECT BLOCKS
		
		lbk  = oftp->offset / BLKSIZE;
        startByte = oftp->offset % BLKSIZE;
		
		if (lbk < 12){                     // lbk is a direct block
		//printf("Direct\n");
           blk = mip->INODE.i_block[lbk]; // map LOGICAL lbk to PHYSICAL blk
       }
	   else if (lbk >= 12 && lbk < 256 + 12) { 
	 //  printf("Indirect\n");
            //  indirect blocks 
			get_block(mip->dev, ip->i_block[12], readBuf);
			int_p=(int*)readBuf+lbk-12;
		//	printf("int_p : %d", int_p);
			blk=*int_p;
       }
       else{ 
	  // printf("Indirect x2\n");
            //  double indirect blocks
			get_block(mip->dev, ip->i_block[13], readBuf);
			
			ind_blk=(lbk-256-12)/256;
			ind_blk_ofs=(lbk-256-12)%256;
			int_p=(int*)readBuf+ind_blk;
			
			get_block(mip->dev, *int_p, readBuf);
			
			int_p=(int*)readBuf+ind_blk_ofs;
			blk=*int_p;
       } 
	    // get the data block into readbuf[BLKSIZE] 
       get_block(mip->dev, blk, readBuf);
	   
	   // copy from startByte to buf[ ], at most remain bytes in this block 
       char *cp = readBuf + startByte;   
       remain = BLKSIZE - startByte;   // number of bytes remain in readbuf[]

       while (remain > 0){
            *cq++ = *cp++;             // copy byte from readbuf[] into buf[]
             oftp->offset++;           // advance offset 
             count++;                  // inc count as number of bytes read
             avil--; nbytes--;  remain--;
             if (nbytes <= 0 || avil <= 0) 
                 break;
	   }
	   
	   
	   
		
	}
	printf("--------------------------------------------------------------------\n");
	printf("my_read: read %d char from file descriptor %d\n", count, fd);  
	printf("--------------------------------------------------------------------\n");
   return count;   // count is the actual number of bytes read
	
}


/*
cat filename:

   char mybuf[1024], dummy = 0;  // a null char at end of mybuf[ ]
   int n;

1. int fd = open filename for READ;
2. while( n = read(fd, mybuf[1024], 1024)){
       mybuf[n] = 0;             // as a null terminated string
       // printf("%s", mybuf);   <=== THIS works but not good
       spit out chars from mybuf[ ] but handle \n properly;
   } 
3. close(fd);
*/


int cat_file(char* filename){
	char mybuf[1024], dummy = 0;
	int n;
	
	int fd=open_file(filename, "0");
	while(n=my_read(fd, mybuf, 1024)){
		mybuf[n]=0;
		printf("%s", mybuf);
	}
	
	close_file(fd);
}





















