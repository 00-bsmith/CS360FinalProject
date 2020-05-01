/************* cd_ls_pwd.c file **************/

int chdir(char *pathname)   
{
	 if(strcmp(pathname,"")==0){
	printf("Can't change dir to nothing\nTaking you back to root!\n");
	 iput(running->cwd);
	  running->cwd=root;
 }
 else{
  printf("chdir %s\n", pathname);
  //printf("under construction READ textbook HOW TO chdir!!!!\n");
  // READ Chapter 11.7.3 HOW TO chdir
  //the below lines are straight from the book
  int ino=getino(pathname);
  MINODE *mip=iget(dev, ino);
  if(S_ISDIR(mip->INODE.i_mode)){
	  printf("%s is a dir\n", pathname);
	  iput(running->cwd);
	  running->cwd=mip;
  }
  else{
	  printf("%s is not a dir\nPlease retry with a dir\n", pathname);
  }
  
  
 }
  
}

int ls_file(MINODE *mip, char *name)
{
	char *t1 = "xwrxwrxwr-------";
	char *t2 = "----------------";	
	//From 8.6.7
	//these were used in previous LS runs and seems to work well
	/*
	f ((sp->st_mode & 0xF000) == 0x8000) // if (S_ISREG())
		printf("%c",'-');
	if ((sp->st_mode & 0xF000) == 0x4000) // if (S_ISDIR())
		printf("%c",'d');
	if ((sp->st_mode & 0xF000) == 0xA000) // if (S_ISLNK())
		printf("%c",'l');
	for (i=8; i >= 0; i--){
		if (sp->st_mode & (1 << i)) // print r|w|x
		printf("%c", t1[i]);
		else
		printf("%c", t2[i]); // or print -
	}
	printf("%4d ",sp->st_nlink); // link count
	printf("%4d ",sp->st_gid); // gid
	printf("%4d ",sp->st_uid); // uid
	printf("%8d ",sp->st_size); // file size
	*/
	
	u16 mode;
	u16 link;
	u16 uid;
	u16 gid;
	u32 size;
	
	INODE inodep= mip->INODE;
	
	mode=inodep.i_mode;
	link=inodep.i_links_count;
	uid=inodep.i_uid;
	gid=inodep.i_gid;
	size=inodep.i_size;
	
	
	
	//now we do the big if chain that is shown up top
	if((mode&0xF000)==0x8000){
		printf("%c",'-');
	}
	if((mode&0xF000)==0x4000){
		printf("%c",'d');
	}
	if((mode&0xF000)==0xA000){
		printf("%c",'l');
	}
	//straight pulled from 8.6.7 and edited
	int i=0;
	for (i=8; i >= 0; i--){
		if (mode & (1 << i)) // print r|w|x
		printf("%c", t1[i]);
		else
		printf("%c", t2[i]); // or print -
	}
	printf("%4d ",link); // link count
	printf("%4d ",gid); // gid
	printf("%4d ",uid); // uid
	printf("%8d ",size); // file size
	
	char* nodeTime=ctime((time_t*)&inodep.i_mtime);
	nodeTime[strlen(nodeTime)-1]=0; // get rid of newline
	printf("%s ", nodeTime);
	printf("%s", name);
	printf("\n");
	
	
}

int ls_dir(MINODE *mip)
{
 // printf("ls_dir: list CWD's file names; YOU do it for ls -l\n");

  char buf[BLKSIZE], temp[256];
  DIR *dp;
  char *cp;
//  char *t1 = "xwrxwrxwr-------";
//	char *t2 = "----------------";
  
  
  // Assume DIR has only one data block i_block[0]
  get_block(dev, mip->INODE.i_block[0], buf); 
  dp = (DIR *)buf;
  cp = buf;
  MINODE *temp_mip;
  
  while (cp < buf + BLKSIZE){
     strncpy(temp, dp->name, dp->name_len);
     temp[dp->name_len] = 0;
	
	temp_mip=iget(dev, dp->inode);//Edit, i never realized that I hadn't added a way to get a new mip
	if(temp_mip){
		ls_file(temp_mip,temp);
	}
     //printf("[%d %s]  ", dp->inode, temp); // print [inode# name]
	//lets just call our ls_file instead since it has all the fancy formatting!
	//ls_file(mip,temp);


     cp += dp->rec_len;
     dp = (DIR *)cp;
  }
  printf("\n");
}

int ls(char *pathname)  
{
  printf("ls %s\n", pathname);
 // printf("ls CWD only! YOU do it for ANY pathname\n");
 if(strcmp(pathname,"")==0){
	ls_dir(running->cwd);
 }
 else{//we should be able to jsut pass the pathname directly
	// ls_dir(pathname);
	 int myIno=getino(pathname);//getino should return an int that we can send along
	 //now we need to get a MINODE
	 MINODE* myInode=iget(dev,myIno);//starting with dev to see how it goes
	 int check=search(running->cwd,pathname);//this should tell us if it exists
	 if(check==0){//search returns 0 if its not found
		 printf("can't access %s\n",pathname);
	 }
	 else{
		 //found information regarding S_ISDIR, will try to implement
		 printf("%s was found, now trying to print\n", pathname);
		 if(S_ISDIR(myInode->INODE.i_mode)){
			 printf("ISDIR says its a directory\n");
			  ls_dir(myInode);
		 }
		 else{
			 ls_file(myInode,pathname);
		 }
	
	 }
	 
 }
}


void rpwd(MINODE *wd){
	  if(wd==root){}
	  else{
		  printf("In else of rec\n");
		  int my_ino;
		  int parent_ino;
		  
		  //need to use the block to get the inos above
		  //see get_block being used elsewhere, perhaps this can give us some insight
		  char buffer[BLKSIZE];
		  get_block(dev, wd->INODE.i_block[0], buffer);
		  char* temp[256];
		  
		  DIR *dp;
			char *cp;

			// Assume DIR has only one data block i_block[0]
			get_block(dev, wd->INODE.i_block[0], buffer); 
			dp = (DIR *)buffer;
			cp = buffer;
			printf("Buffer: %s\n",buffer);
			// the "." directory is current directory and ".." is the parent directory
			//this is useful..... how?
		  //we need a loop to find these two
		  //if we have found them, then we can set ino and parent ino
		  //they should be the first two directories in a block, so counter shouldn't need to go above 2
			//will add a counter if there is a need for it!
			//adding a counter statement to speed things up
			int counter =0;
			printf("After decs\n");
		  while((cp<buffer+BLKSIZE)&&(counter<2)){
			  printf("In while\n");
			strncpy(temp, dp->name, dp->name_len);
			temp[dp->name_len] = 0;
			  
			//now strcmp against . and .. to see if its parent or us
			if(strcmp(temp,".")==0){
				printf("First if\n");
				my_ino=dp->inode;
				
			}
			if(strcmp(temp,"..")==0){
				printf("second if\n");
				parent_ino=dp->inode;
				}
			  cp += dp->rec_len;
				dp = (DIR *)cp;
				counter++;
		  }
		  printf("No more while\n");
			//this while statement is adapted from the ls_dir that was given to us
		  MINODE* pip=iget(dev,parent_ino);//step 3
		  //from pip->INODE.i_block[]: get my_name string by my_ino as local
		  //this means I need to write findmyname
		  //char* my_name;
		  //lets try allocating space to stop the seg fault?
		  char my_name[256];
		 printf("About to call name\n");
		  findmyname(pip, my_ino, my_name);
		  printf("Left my name\n");
		  if(pip){
		  rpwd(pip);
		  }
		  printf("After rec\n");//Can never reach this point and i dont know why
		  printf("/%s", my_name);
	  }
	  
  }












char *pwd(MINODE *wd)
{
  //printf("pwd: READ HOW TO pwd in textbook!!!!\n");
  printf("Printing working directory!\n");
  if (wd == root){
    printf("/\n");
    return;
  }
  else{
	  printf("Entering recursive\n");
	 rpwd(wd);  
	 printf("\n");
  }
  
  
}



