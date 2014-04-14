#include "GlobalOpenFileTable.h"

int compareName(GlobalOpenFileData* gofd, char* name);
int comparePid(PerProcessOpenFileData* ppofd, int pid);

GlobalOpenFileData* newGlobalOpenFileData(MetaDataNode* mdn) {
	GlobalOpenFileData* gofd = malloc(sizeof *gofd);
	strcpy(gofd->name, mdn->name);
	gofd->mdn = mdn;
	gofd->dataLocation = mdn->dataIndex;
	gofd->fileOpenCount = 1;

	return gofd;
}

GlobalOpenFileData* closeGlobal(GlobalOpenFileData* gofd) {
	if (--gofd->fileOpenCount == 0) {
		free(gofd);
		return NULL;
	}

	return gofd;
}

PerProcessOpenFileData* newPerProcessOpenFileData(unsigned int pid, int uid, int gid,  MetaDataNode* mdn, GlobalOpenFileData* gofd) {
	// make sure we have permission to open
	Byte fileMode = 0;

	// get user access
	if (mdn->uid == uid)
		fileMode = (mdn->fileMode & 0700) >> 6;

	// get group access
	if (mdn->gid == gid)
		fileMode = (mdn->fileMode & 0070) >> 3;

	// get other access
	fileMode = mdn->fileMode & 0007;

	if (!fileMode)
		return NULL;

	PerProcessOpenFileData* ppofd = malloc(sizeof *ppofd);
	ppofd->pid = pid;
	ppofd->fileMode = fileMode;
	ppofd->position = 0;
	ppofd->gofd = gofd;

	return ppofd;
}

void* findByName(GList* goft, const char* name) {
	int compareName(GlobalOpenFileData* gofd, const char* other) {
		return strcmp(gofd->name, other);
	}

	GList* l = g_list_find_custom(goft, name, (void*) compareName);
	if (!l)
		return NULL;

	return l->data;
}

void* findByPidAndName(GList* ppoft, int pid, const char* name) {
	for (GList* l = ppoft; l != NULL; l = l->next)
		if (((PerProcessOpenFileData*) l->data)->pid == pid && !strcmp(((PerProcessOpenFileData*) l->data)->gofd->name, name))
			return l->data;

	return NULL;
}

void freeGlobalOpenFileTable(GList* goft) {
	g_list_free_full(goft, free);
}