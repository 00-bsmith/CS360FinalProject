/************* mkdir_creat.c file **************/

void my_mkdir(MINODE* mip, char* child_name);
int enter_name(MINODE *pip, int myino, char *myname);
void creat_file(char *pathname);
void my_creat(MINODE *pmip, char* child_name);

//current notes:
//as of 4/8/20, the mkdir works, but when i cdnto a dir and try to make another it loops and spams 0's at me
//doing mkdir dir1/dir2 seems to work, but when i cd in and ls, i get a weird loop..
//this uses the same exact ls program that worked fine previously
//only thing i can think of is my mkdir doesnt work correctly when inside something other than root...
//will need to investigate

//update: I added some lines i forgot to go back and add. Here is how it works now.

//if i make dir 1, then go into dir1, and make a new dir, it works. I can also ls.
//but if i make dir 1 and dir2, then cd into dir1, mkdir and ls fail.
//when this fails, dir2, breaks and stops being a dir.
//i can go down as much as i want and it works.
//as soon as a file has a sibling, only the oldest sibling will work correctly.
//any older files get corrupted in some manner
//perhaps my bits are off slightly in the definitions
//if i make a dir3, while 1&2 are siblings. 2 stops being a dir, yet 1 remains a dir.
//same with 4 and onwards.
//using ls to examine 2&3 shows -------- as perms, and 0's for all other information
//time is listed as Dec 31, 16:00:00 1969
//this is definitely some sort of corruption in the data. perhaps end bits are being overwritten
//this is using pacific time, UTC-8, linux is based off jan 1 1970 00:00 UTC.


//if a dir is created, and a new file is then created after it, a similar thing happens to if two dirs are made back to back
//only this time the filess aren't messed with as of now.
//i think the dir complecity makes them easier to break.



//If 2 files are made in series, the oldest file gets corrupted into a dir somehow?








int make_dir(char * pathname)
{
 /*  MINODE *start;		     
1. pahtname = "/a/b/c" start = root;         dev = root->dev;
            =  "a/b/c" start = running->cwd; dev = running->cwd->dev;

2. Let  
     parent = dirname(pathname);   parent= "/a/b" OR "a/b"
     child  = basename(pathname);  child = "c"

   WARNING: strtok(), dirname(), basename() destroy pathname

3. Get minode of parent:

       pino  = getino(parent);
       pip   = iget(dev, pino); 

   Verify : (1). parent INODE is a DIR (HOW?)   AND
            (2). child does NOT exists in the parent directory (HOW?);
               
4. call mymkdir(pip, child);

5. inc parent inodes's link count by 1; 
   touch its atime and mark it DIRTY

6. iput(pip);*/

	if(strcmp(pathname, "")==0){//verify pathname given is not empty
		printf("No path given for mkdir\nPlease Try Again\n");
		return 1;
	}
		int i, ino;
		
		MINODE* mip; //start
		
		char parent_name[1024], child_name[1024], buf[1024], temp1[1024], temp2[1024];
		
		//basename and dirname https://linux.die.net/man/3/dirname
		
		strcpy(temp1, pathname);
		strcpy(temp2, pathname);
		
		strcpy( parent_name, dirname(temp1));
		strcpy(child_name, basename(temp2));
		//temps are used as dirname and basename destroy the original like strtok
		printf("Parent: %s, \nChild: %s\n", parent_name, child_name);
		//get the ino
		ino=getino(parent_name);//now we can check if parent exists, which it should
		mip=iget(dev, ino);
		INODE *ip;
		ip=&mip->INODE;
		
		//check if its real
		if(!mip){//this should only happen if you try to make a dir with a pathname
		//making a dir within cwd should always work
			printf("ERROR: NO PARENT\n");
			return;
			
		}
	
		
		
		//make sure its a directory
		if(!S_ISDIR(ip->i_mode)){
			printf("The parent is not a directory\nTry again\n");
			return;
		}
		//make sure youre not making 2 of the same directories
		if(search((running->cwd), pathname)!=0){//search returns 0 if not found, so if !0 then theres a duplicate
			printf("ERROR! Cannot make duplicate dir!\n");
			return;
			
		}
		//if all these come through clean, move on to the helper program mymkdir
		if(mip==root){
			printf("The parent is root!\n");
		}
		
		my_mkdir(mip, child_name);
		
		//4/8/2020 forgot these lines, lets hope this is the fix!
		ip->i_links_count++;
		ip->i_atime=time(0L);
		mip->dirty=1;
		
		iput(mip);
		return;
		
		
}

void my_mkdir(MINODE* pmip, char* child_name){
	 MINODE *mip;
	 
	 int ino = ialloc(dev);    
     int bno = balloc(dev);
	 printf("ino: %d\nbno: %d\n", ino, bno);
	 
	 mip=iget(dev,ino);
	 
	 INODE *ip;
	 ip=&mip->INODE;
	 //below from https://eecs.wsu.edu/~cs360/mkdir_creat.html
	ip->i_mode = 0x41ED;		// OR 040755: DIR type and permissions
	ip->i_uid  = running->uid;	// Owner uid 
	ip->i_gid  = running->gid;	// Group Id
	ip->i_size = BLKSIZE;		// Size in bytes 
	ip->i_links_count = 2;	        // Links count=2 because of . and ..
	ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);  // set to current time
	ip->i_blocks = 2;                	// LINUX: Blocks count in 512-byte chunks 
	ip->i_block[0] = bno;             // new DIR has one data block   
	//ip->i_block[1] to i_block[14] = 0;
	
	for(int i=1;i<15;i++){
		ip->i_block[i]=0;
	}
 
  mip->dirty = 1;               // mark minode dirty
  iput(mip);                    // write INODE to disk
	 
	
	//create data for . and .. (aka self and parent)
	char* buf[1024];
	get_block(dev,bno,buf);
	
	DIR *dp = (DIR*) buf; //cast the buffer as a directory
	char * temp = buf;
	//dp is meeeeeee
	dp->inode=ino;
	dp->rec_len=12;
	dp->name_len=strlen(".");// useing 1 didnt work
	dp->file_type=(u8)EXT2_FT_DIR;
	//strcpy(dp->name, '.');
	dp->name[0]='.';
	
	temp+=dp->rec_len;
	dp=(DIR*)temp;
	
	//parent aka ..
	dp->inode=pmip->ino;
	dp->rec_len = 1012; // 1024-12 is 1012
	dp->name_len=strlen("..");
	dp->file_type=(u8)EXT2_FT_DIR;
	dp->name[0]='.';
	dp->name[1]='.';
	
	//now we add the data
	put_block(dev,bno,buf);
	
	enter_name(pmip,ino,child_name);
	
	
}

int enter_name(MINODE *pip, int myino, char *myname){
	INODE *parent=&pip->INODE;
	
	char buf[1024];
	char *copy;
	DIR* dp;
	
	int block_size=1024;
	int bno=0;
	int length=0;
	
	for(int i=0; i<parent->i_size/BLKSIZE;i++){
		if(parent->i_block[i]==0){break;}//done in loop
		
		bno=parent->i_block[i];
		
		get_block(dev,bno,buf);
		
		dp=(DIR*)buf;
		copy=buf;
		
		
		length=4*((8+strlen(myname)+3)/4);//This length needs achieved
		printf("Length=%d\n", length);
		
		while(copy+dp->rec_len < buf+BLKSIZE){//get to the end of the entry list
			copy+=dp->rec_len;
			dp=(DIR*)copy;
		}
		printf("Last: %s\n", dp->name);
		copy=(char*)dp;
		
		
		int ideal = 4*((8+dp->name_len+3)/4);
		printf("Ideal: %d\n", ideal);
		int remain=dp->rec_len-ideal;
		printf("Remain: %d\n", remain);
		
		if(remain >=length){
			dp->rec_len=ideal;
			copy+=dp->rec_len;
			dp=(DIR*)copy;
			
			dp->inode=myino;
			dp->rec_len= block_size- ((u32)copy-(u32)buf);
			printf("rec_len: %d\n", dp->rec_len);
			
			dp->name_len=strlen(myname);
			dp->file_type=EXT2_FT_DIR;
			strcpy(dp->name, myname);
			
			put_block(dev, bno, buf);
			
			return;
			
		}
		
		
	}
	
	
	
}



////////////////////////////
void creat_file(char* pathname){//creat is supposed to be nearly hte same as for dir
//inode mode is REGULAR
//rw-r--r--
//no data, size=0
//links_count=1
//dont increment parent link

		
	
	
	if(strcmp(pathname, "")==0){//verify pathname given is not empty
		printf("No path given for mkdir\nPlease Try Again\n");
		return 1;
	}
		int i, ino;
		
		MINODE* mip; //start
		
		char parent_name[1024], child_name[1024], buf[1024], temp1[1024], temp2[1024];
		
		//basename and dirname https://linux.die.net/man/3/dirname
		
		strcpy(temp1, pathname);
		strcpy(temp2, pathname);
		
		strcpy( parent_name, dirname(temp1));
		strcpy(child_name, basename(temp2));
		//temps are used as dirname and basename destroy the original like strtok
		printf("Parent: %s, \nChild: %s\n", parent_name, child_name);
		//get the ino
		ino=getino(parent_name);//now we can check if parent exists, which it should
		mip=iget(dev, ino);
		INODE *ip;
		ip=&mip->INODE;
		
		//check if its real
		if(!mip){//this should only happen if you try to make a dir with a pathname
		//making a dir within cwd should always work
			printf("ERROR: NO PARENT\n");
			return;
			
		}
	
	
		//make sure its a directory
		if(!S_ISDIR(ip->i_mode)){
			printf("The parent is not a directory\nTry again\n");
			return;
		}
		//make sure youre not making 2 of the same directories
		if(search((running->cwd), pathname)!=0){//search returns 0 if not found, so if !0 then theres a duplicate
			printf("ERROR! Cannot make duplicate dir!\n");
			return;
			
		}
		
		
		my_creat(mip, child_name);
		
	
	
		ip->i_atime=time(0L);
		mip->dirty=1;
		
		iput(mip);
		return;
	
	
	
}



void my_creat(MINODE* pmip, char* child_name){
	
	 int ino = ialloc(dev);   
	  MINODE* mip=iget(dev,ino);
	  INODE *ip=&mip->INODE;
	  printf("Creat ino: %d\n", ino);
	  
	ip->i_mode = 0x81A4;		// REGULAR
	ip->i_uid  = running->uid;	// Owner uid 
	ip->i_gid  = running->gid;	// Group Id
	ip->i_size = 0;		// Size in bytes 
	ip->i_links_count = 1;	        // Links count=1 because only itself
	ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);  // set to current time
	ip->i_blocks = 0;                	// LINUX: Blocks count in 512-byte chunks 
	
	//ip->i_block[1] to i_block[14] = 0;
	
	for(int i=1;i<15;i++){
		ip->i_block[i]=0;
	}
	mip->dirty=1;
	iput(mip);
	enter_name(pmip,ino,child_name);
	return;
	
	
	
}











