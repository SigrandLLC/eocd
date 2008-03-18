#ifndef EOC_PCI_H
#define EOC_PCI_H

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <generic/EOC_generic.h>

int pci_iflist(DIR *dir,char *dpath,char *pdev,char iflist[MOD_MAX_DEVS][IF_NAME_LEN]);
char *pci2dname(int pcislot,int pcidev);
int dname2pci(char *name,int &pcislot,int &pcidev);

#endif