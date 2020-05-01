/************* rmdir.c file **************/


//notes
//Due to the mkdir issues causing corruption, I have no way of testing the first and last cases
//for the rmdir rm_child functions
//this means you will only ever be able to remove the last directory
//this removal does work however
//as far as I can tell, this SHOULD work, but I have no way to test until I fix mkdir




//note 2: My empty check works if the dir has another dir nested inside
//I need to go back and fix it so it will work with a file...








int rm_child(MINODE *parent, char *name){
	

	INODE* pip=&parent->INODE;
	
	DIR *dp;
	DIR* prev;
	DIR* last;
	
	char buf[1024];
	char *cp;
	char temp[1024];
	char *cp_last;
	
	int start;
	int end;
	int i;
	
	printf("parent start size: %d\n", pip->i_size);
	
	for(i=0;i<12;i++){
		if(pip->i_block[i]==0){
			return;//null block, dont need to do anything
		}
		
		get_block(dev, pip->i_block[i],buf);
		cp=buf;
		dp=(DIR*)buf;
		
		
		while(cp<buf+BLKSIZE){
			
			strncpy(temp, dp->name, dp->name_len);
			temp[dp->name_len]=0;
			printf("At %s\n", temp);//remove later
			
			if(strcmp(temp, name)==0){
				//we found the child we are looking for
				printf("Found\n");
				
				if((cp==buf)&&(cp+dp->rec_len==buf+BLKSIZE)){
					//first
					printf("It's the first");
					bdalloc( dev, pip->i_block[i]);
					
					pip->i_size-=BLKSIZE;
					
					//shift
					printf("Before while\n");
					while((pip->i_block[i+1])&&(i+1<12)){//the next block exists and our counter isnt more than 11
						printf("In while\n");
						i++;
						get_block(dev, pip->i_block[i], buf);
						put_block(dev, pip->i_block[i-1], buf);
						
					}
					printf("After While\n");
					
					
				}
				
				else if(cp+dp->rec_len==buf+BLKSIZE){
					//last entry
					printf("It's the last one\n");
					
					prev->rec_len+=dp->rec_len;
					put_block(dev, pip->i_block[i], buf);
					
					
				}
				else{
					//middle
					printf("Middle\n");
					
					last=(DIR*)buf;
					cp_last=buf;
					printf("Before while\n");
					while(cp_last+last->rec_len<buf+BLKSIZE){
						printf("In while\n");
						cp_last+=last->rec_len;
						last = (DIR*)cp_last;
						
						
					}
					printf("After while\n");
					last->rec_len+=dp->rec_len;
					start=cp+dp->rec_len;
					end=buf+BLKSIZE;
					
					memmove(cp, start, end-start);
					
					put_block(dev, pip->i_block[1], buf);
					
				}
				parent->dirty=1;
				iput(parent);
				return;//no need to keep looping
				
			}
			
			
			
			
			
			
			
			
			prev = dp;
			cp += dp->rec_len;
			dp = (DIR*)cp;
		}
		
		
	}
	return;
	/*
	
   1. Search parent INODE's data block(s) for the entry of name

   2. Erase name entry from parent directory by
    
  (1). if LAST entry in block{
                                         |remove this entry   |
          -----------------------------------------------------
          xxxxx|INO rlen nlen NAME |yyy  |zzz                 | 
          -----------------------------------------------------

                  becomes:
          -----------------------------------------------------
          xxxxx|INO rlen nlen NAME |yyy (add zzz len to yyy)  |
          -----------------------------------------------------

      }
    */
	
	
	/*
  (2). if (first entry in a data block){
          deallocate the data block; modify parent's file size;

          -----------------------------------------------
          |INO Rlen Nlen NAME                           | 
          -----------------------------------------------
          
          Assume this is parent's i_block[i]:
          move parent's NONZERO blocks upward, i.e. 
               i_block[i+1] becomes i_block[i]
               etc.
          so that there is no HOLEs in parent's data block numbers
      }
*/

/*
  (3). if in the middle of a block{
          move all entries AFTER this entry LEFT;
          add removed rec_len to the LAST entry of the block;
          no need to change parent's fileSize;

               | remove this entry   |
          -------------------------------------------------------------------
          xxxxx|INO rlen nlen NAME   |yyy  |zzz                             | 
          -----|---------------------|---------------------------------------
               cp                    cp+dp->rec_len
               dp                    | size = buf+BLKSIZE - (cp+dp->rec_len)|

          memcpy(cp, cp+dp->rec_len, size);

                  becomes:
          -------------------------------------------------------------------
          xxxxx|yyy |zzz (rec_len INC by rlen)                              |
          -------------------------------------------------------------------

      }
    */
	
	/*
  3. Write the parent's data block back to disk;
     mark parent minode DIRTY for write-back
	
	
	
	*/
	
}







int isEmptyDir(MINODE * mip){
	/*
	 HOW TO check whether a DIR is empty:
     First, check link count (links_count > 2 means not empty);
     However, links_count = 2 may still have FILEs, so go through its data 
     block(s) to see whether it has any entries in addition to . and ..
	*/
	
	char buf[BLKSIZE];
	
	INODE *ip=&mip->INODE;
	
	char* copy;
	char name[1024];
	DIR* dp;
	
	//link count
	if(ip->i_links_count > 2){
		return 1;
	}
	else if(ip->i_links_count==2){
		//go through data
		//just check the first block
		if(ip->i_block[1]){
			get_block(dev, ip->i_block[1], buf);
			
			copy=buf;
			dp=(DIR*)buf;
			
			while(copy<buf+BLKSIZE){//keep my eyes on this
				strncpy(name, dp->name, dp->name_len);
				name[dp->name_len]=0;//need to append the end with null for strncpy
				
				//check if the name is anything other than . or ..
				//if the name is different, then it is a new file, and its not empty
				if(strcmp(name, ".")!=0){
					return 1;
				}
				if(strcmp(name, "..")!=0){
					return 1;
				}
				
				copy+=dp->rec_len;
				dp=(DIR*)copy;
				
			}
			
			}
		
	}
	else{ return 0;}//it is empty
	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void remove_dir(char* pathname){
	//do not remove . or .., so lets check that first as well as empty
	
	if(strcmp(pathname,"")==0){
	printf("Can't remove nothing\nTry again\n");
	return;
	}
	if(strcmp(pathname, ".")==0){
		printf("You shouldn't be removing yourself\n");
		return;
	}
	if(strcmp(pathname,"..")==0){
		printf("You shouldn't be removing your parent\n");
		return;
	}
	//now for the workhorse
	
	MINODE * pmip;
	MINODE* mip;
	INODE * ip;
	INODE * pip;
	
	int i;
	int ino;
	int parent_ino;
	
	char temp1[1024], temp2[1024], child[1024];
	
	strcpy(temp1, pathname);
	strcpy(child, basename(temp1));//basename destroys the original;
	
	//get ino
	ino=getino(pathname);
	printf("RMDIR INO: %d\n", ino);
	printf("Path: %s\n", pathname);
	
	mip=iget(dev, ino);
	
	if(!mip){
			printf("ERROR: NO DIR\n");
			return;
			
		}
		ip=&mip->INODE;
		//make sure its a directory
	if(!S_ISDIR(ip->i_mode)){
			printf("ERROR: NOT A DIR\n %s\n", pathname);
			return;
		}
	//we need to make a function to check if the file is empty
	if(isEmptyDir(mip)){//isEmptyDir will return 0 if it is empty
		printf("ERROR: %s is not empty\n", pathname);
		return;
	}
	//check if its busy/being used
	if(mip->refCount>1){
		printf("ERROR: %s is busy\n", pathname);
	}
	
	
	
	//now we can remove
	
	//deallocate blocks and inode
	
	for (i=0; i<12; i++){
         if (mip->INODE.i_block[i]==0)
             continue;
         bdalloc(mip->dev, mip->INODE.i_block[i]);
     }
     idalloc(mip->dev, mip->ino);
     iput(mip);// (which clears mip->refCount = 0);
	
	//now we mess with parent link count
	//pip = iget(mip->dev, parent's ino); 
	parent_ino=findino(mip, &ino);
	pmip=iget(dev, parent_ino);//parent MINODE
	pip=&pmip->INODE; //parent INODE 
	
	/*
	8. remove child's entry from parent directory by

        rm_child(MINODE *pip, char *name);
           
        pip->parent Minode, name = entry to remove
	*/
	rm_child(pmip, child);
	
	/*
  9. decrement pip's link_count by 1; 
     touch pip's atime, mtime fields;
     mark pip dirty;
     iput(pip);
     return SUCCESS;
	 */
	pip->i_links_count--;
	pip->i_atime=time(0L);
	pip->i_mtime=time(0L);
	pmip->dirty=1;
	
	iput(pmip);//put changes to parent directory
	
	//mip->dirty=1; //this may not be needed
//	iput(mip);//put changes to deleted directory
	
	return;
	
}