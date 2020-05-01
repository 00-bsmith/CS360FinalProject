/////////////////////link_unlink_symlink

int unlink(char* pathname){
	
	int ino, pino, i;
	MINODE * mip;
	MINODE * pmip;
	
	INODE * ip;
	INODE * pip;
	
	printf("Beginning unlink\n");
	if(strcmp(pathname,"")==0){
		printf("Error: No path given\n");
		return 1;
	}
	ino=getino(pathname);
	mip=iget(dev,ino);
		if(!mip){
			printf("Error: no file\n");
			return 1;
		}
	//right file type, aka not a dir
	ip=&mip->INODE;
	
	if(S_ISDIR(ip->i_mode)){
		printf("Can't unlink a dir\n");
		return 1;
	}
	
	printf("Passed Unlink Checking\n");
	
	printf("Old Links: %d", ip->i_links_count);//if a file has aany data in it, this crashes
	ip->i_links_count--;
	printf("New Links: %d", ip->i_links_count);
	
	//block dealloc
	for(i=0;i<12&&ip->i_block[i]!=0; i++){
		bdalloc(dev, ip->i_block);
	}
	printf("After loop\n");
	
	//inode dealloc
	
	idalloc(dev, ino);
	
	char temp[128], dir[128], base[128];
	
	strcpy(temp, pathname);
	strcpy(base, basename(temp));
	strcpy(temp, pathname);
	strcpy(dir,dirname(temp));
	
	pino=getino(dir);
	pmip=iget(dev, pino);
	pip=&pmip->INODE;
	
	printf("Removal checkpoint\n");//remove this in final
	
	rm_child(pmip, base);//from rmdir.c
	pip->i_atime=time(0L);
	pip->i_mtime=time(0L);
	pmip->dirty=1;
	mip->dirty=1;
	
	iput(pmip);
	iput(mip);
	
	
	
}



int link(char*pathname, char* third){
	
	if(strcmp(pathname,"")==0){
		printf("Error: No path given\n");
		return 0;
	}
	if(strcmp(third,"")==0){
		printf("Error: No Tertiary given\n");
		return 0;
	}
	
	char old[128];//i want to store the names locally just in case i need the originals
	char new[128];
	char temp[128];
	
	char l_parent[128], l_child[128];
	
	int ino, pino, i;
	
	MINODE *mip;
	MINODE *pmip;
	
	INODE * ip;
	INODE* pip;
	
	
	/////////////////////////////////////
	printf("Beginning Linking\n");
	strcpy(old, pathname);
	strcpy(new, third);
	
	ino=getino(old);
	mip=iget(dev, ino);
	
	if(!mip){
		printf("Error: %s DNE\n", old);
		return 0;
	}
	if(S_ISDIR(mip->INODE.i_mode)){
		printf("Error: Can't link to directories!\n");
		return 0;
	}
	////cleared checks
	//start linking
	
	if(strcmp(new,"/")==0){
		strcpy(l_parent, "/");
	}
	else{
		//for dirname
		//https://linux.die.net/man/3/dirname
		strcpy(temp, new);//dirname destroys teh string like strtok
		strcpy(l_parent, dirname(temp));
		
	}
	//new's basename
	strcpy(temp, new);
	strcpy(l_child, basename(temp));
	
	//get the new parent ino
	pino=getino(l_parent);
	pmip=iget(dev,pino);
	
	
	if(!pmip){
		printf("Error: No parent\n");
		return 0;
	}
	//make sure the child isnt already in existence
	//no dupes
	if(getino(new)){
		printf("Error: %s already exists\n", new);
		return 0;
	}
	
	//parent HAS to be a directory
	if(!S_ISDIR(pmip->INODE.i_mode)){//this should never run, but i have the check elsewhere
		printf("Error: somehow your parent isnt a dir?\n");
		return 0;
	}
	
	printf("After Link Checking\n");
	//this uses the enter_name from mkdir_creat
	enter_name(pmip, ino, l_child);
	
	
	ip=&mip->INODE;
	ip->i_links_count++;
	mip->dirty=1;
	
	pip=&pmip->INODE;
	pip->i_atime=time(0L);
	pmip->dirty=1;
	
	iput(pmip);
	iput(mip);
	printf("Done Linking\n");
	
}


/*
 symlink oldNAME  newNAME    e.g. symlink /a/b/c /x/y/z

   ASSUME: oldNAME has <= 60 chars, inlcuding the ending NULL byte.

(1). verify oldNAME exists (either a DIR or a REG file)
(2). creat a FILE /x/y/z
(3). change /x/y/z's type to LNK (0120000)=(1010.....)=0xA...
(4). write the string oldNAME into the i_block[ ], which has room for 60 chars.
    (INODE has 24 unused bytes after i_block[]. So, up to 84 bytes for oldNAME) 

     set /x/y/z file size = number of chars in oldName

(5). write the INODE of /x/y/z back to disk.
  



*/

int symlink(char *pathname, char* third){
	printf("In Symlink\n");
	
	int old_ino=getino(pathname);
	if(old_ino==0){
		printf("Error: %s DNE\n", pathname);
		return 0;
	}
	MINODE * old_mip=iget(dev, old_ino);
	if(!old_mip){
		printf("Error: No MIP\n");
		return 0;
	}
	if(strcmp(third, "")==0){
		printf("Error, no third\n");
		return 0;
	}
	
	char temp[128];
	strcpy(temp, third);
	char parent[128], child[128];
	strcpy(parent, dirname(temp));
	strcpy(child, basename(temp));
	
	printf("Parent: %s, Child: %s\n", parent, child);
	
	int parent_ino=getino(parent);
	MINODE * parent_mip=iget(dev, parent_ino);
	
	if(!parent_mip){
		printf("Error: No parent MIP\n");
		return 0;
		
	}
	
	if(!S_ISDIR(parent_mip->INODE.i_mode))
	{
		printf("ERROR: parent is not a directory\n");
		return 0;
	}
	if(getino(child) > 0)
	{
		printf("ERROR: %s already exists\n", child);
		return;
	}
	//okay we should be good to move on
	printf("Cleared checks\n");
	
	//(2). creat a FILE /x/y/z
	creat_file(child);
	//get the ino
	int ino=getino(child);
	
	MINODE * mip=iget(dev, ino);
	INODE *ip=&mip->INODE;
	//3). change /x/y/z's type to LNK (0120000)=(1010.....)=0xA...
	ip->i_mode=0120000;
	/*
	(4). write the string oldNAME into the i_block[ ], which has room for 60 chars.
    (INODE has 24 unused bytes after i_block[]. So, up to 84 bytes for oldNAME) 

     set /x/y/z file size = number of chars in oldName
	*/
	
	strcpy(temp, pathname);
	char old_name[128];
	strcpy(old_name, basename(temp));
	ip->i_size=strlen(old_name);
	
//	(5). write the INODE of /x/y/z back to disk.

mip->dirty = 1;
iput(mip);
//iput(old_mip);
enter_name(parent_mip, ino, child);
printf("Done symlink\n");
return;
}
























