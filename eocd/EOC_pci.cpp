#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

#include <generic/EOC_generic.h>
#include <eoc_debug.h>

int
pci_iflist(DIR *dir,char *dpath,char *pdev,char iflist[MOD_MAX_DEVS][IF_NAME_LEN])
{
	struct dirent *ent;
	char ifaces[MOD_MAX_DEVS][IF_NAME_LEN],*ifaces_s[MOD_MAX_DEVS];
	int cnt,ifcnt = 0;

	// Get all interfaces supported by driver
	while( ent = readdir(dir) ){
		char tmp_path[PATH_SIZE];
		char buf[PATH_SIZE];
		if( strstr(ent->d_name,"dsl") ){
			snprintf(tmp_path,PATH_SIZE,"%s/%s",dpath,ent->d_name);
			if( (cnt = readlink(tmp_path,buf,PATH_SIZE) ) >= 0 ){
				buf[cnt] = '\0';
				if( strstr(buf,pdev) ){
					strncpy(ifaces[ifcnt],ent->d_name,IF_NAME_LEN);
					ifcnt++;
				}
			}
		}
	}
	
	// Bubble sort
	for(int i=0;i<ifcnt;i++){
		ifaces_s[i] = ifaces[i];
	}
	int flag = 1;
	for(int i=ifcnt-1;i>=0 && flag;i--){
		flag = 0;
		for(int j=0;j<i;j++){
			if( strncmp(ifaces_s[j],ifaces_s[j+1],IF_NAME_LEN)>0 ){
				char *tmp = ifaces_s[j];
				ifaces_s[j] = ifaces_s[j+1];
				ifaces_s[j+1] = tmp;
				flag = 1;
			}
		}
	}
	for(int i=0;i<ifcnt;i++){
		strncpy(iflist[i],ifaces_s[i],IF_NAME_LEN);
	}
	
	return ifcnt;
}


char *
pci2dname(int pcislot,int pcidev)
{
	DIR *dir;
	char drvpath[PATH_SIZE];
	char device[PCI_NAME_LEN];
	char ifaces[MOD_MAX_DEVS][IF_NAME_LEN];
	int ifcnt = 0;
	int type = 0;
	
	PDEBUG(DERR,"Start");
	snprintf(drvpath,PATH_SIZE,DEVICE_PATH "/0000:00:%02d.%d/driver",pcislot,pcidev);
	snprintf(device,PCI_NAME_LEN,"0000:00:%02d.%d",pcislot,pcidev);
	PDEBUG(DERR,"Driver path= %s\n\t\tdevice=%s",drvpath,device);
	if( !(dir = opendir(drvpath)) ){
		snprintf(drvpath,PATH_SIZE,DEVICE_PATH "/0000:00:%02d.0/driver",pcislot);
		snprintf(device,PCI_NAME_LEN,"0000:00:%02d.0",pcislot);
		PDEBUG(DERR,"Err, Driver path= %s\n\t\tdevice=%s",drvpath,device);

		if( !(dir = opendir(drvpath)) ){
			PDEBUG(DERR,"Full error");
			return NULL;
		}
		type = 1;
	}

	PDEBUG(DERR,"Get interfaces list");
	rewinddir(dir);	
	ifcnt = pci_iflist(dir,drvpath,device,ifaces);
	closedir(dir);

// DEBUG - to delete
	PDEBUG(DERR,"Reading result:");
	for(int i=0;i<ifcnt;i++){
		PDEBUG(DERR,"\t%s",ifaces[i]);
	}
// ----------------------
				
	switch( type ){
	case 0:
		return strdup(ifaces[0]);
	case 1:
		if( pcidev >= ifcnt ){
			return NULL;
		}
		return strdup(ifaces[pcidev]);
	}
	return NULL;
}


int
dname2pci(char *name,int &pcislot,int &pcidev)
{
	DIR *dir;
	struct dirent *ent;
	char drvpath[PATH_SIZE];
	char pdev[PCI_NAME_LEN] = "\0",pdev_buf[PCI_NAME_LEN];
	char ifaces[MOD_MAX_DEVS][IF_NAME_LEN];
	int ifcnt = 0;
	int type = 0;

	PDEBUG(DERR,"Start");
	// Get device associated with interfase name
	do{
		char buf1[PATH_SIZE], *ptr, *tok, *saveptr;
		char buf[PATH_SIZE];
		char tmp_path[PATH_SIZE];
		int cnt;
		snprintf(tmp_path,PATH_SIZE,OS_IF_PATH "/%s/device",name);
		PDEBUG(DERR,"Read Link: %s",tmp_path);
		if( (cnt = readlink(tmp_path,buf,PATH_SIZE) ) >= 0 ){
			// Save PCI device parameters
			buf[cnt] = '\0';
			PDEBUG(DERR,"Result: %s",buf);
			ptr = buf;
			for(;;ptr = NULL){
				if( !(tok = strtok_r(ptr,"/",&saveptr)) ){
					break;
				}else{
					strncpy(buf1,tok,256);
				}
			} 
			strncpy(pdev,buf1,PCI_NAME_LEN);				
			break;
		}else{
			// No device symlink => something wrong
			return -1;
		}
	}while(0);

	snprintf(drvpath,PATH_SIZE,OS_IF_PATH "/%s/device/driver",name);
	PDEBUG(DERR,"Read %s dir",drvpath);
	if( !(dir = opendir(drvpath)) ){
			return -1;
	}
	PDEBUG(DERR,"Read %s - success",drvpath);
	ifcnt = pci_iflist(dir,drvpath,pdev,ifaces);
	closedir(dir);

	
// DEBUG - to delete
	PDEBUG(DERR,"Reading result:");
	for(int i=0;i<ifcnt;i++){
		PDEBUG(DERR,"\t%s",ifaces[i]);
	}
// ----------------------
		
	if( !ifcnt ){
		return -1;	
	}

	// Parse device name
	char *ptr = pdev,*tok,*saveptr;
	for(;;ptr = NULL){
		if( !(tok = strtok_r(ptr,":",&saveptr)) ){
				break;
		}else{
			strncpy(pdev_buf,tok,PCI_NAME_LEN);
		}
	} 
	ptr = pdev_buf;
	if( !(tok = strtok_r(ptr,".",&saveptr)) )
		return -1;
	pcislot = atoi(tok);
	if( !(tok = strtok_r(NULL,".",&saveptr)) )
		return -1;
	pcidev = atoi(tok);
	
	
	PDEBUG(DERR,"PCIslot=%d,dev=%d",pcislot,pcidev);
	if( ifcnt == 1)
		return 0;

	for(int i=0;i<ifcnt;i++){
		if( !strncmp(name,ifaces[i],IF_NAME_LEN) ){
			pcidev = i;
			PDEBUG(DERR,"Correction: PCIslot=%d,dev=%d",pcislot,pcidev);
			return 0;
		}
	}
	return -1;
}
