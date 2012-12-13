#include <stdio.h>
#include <string.h>

//#define EOC_DEBUG
#include <eoc_debug.h>
#include <utils/hash_table.h>

hash_table::hash_table(int mhash_name) {
	max_hash_name = mhash_name;
	for (int i = 0; i < HASH_SIZE; i++)
		table[i].clear();
	// clear sequential list
	head = NULL;
	tail = NULL;
}

hash_table::~hash_table() {
	for (int i = 0; i < HASH_SIZE; i++) {
		while (table[i].size()) {
			hash_elem *ptr = *table[i].begin();
			table[i].pop_front();
			free(ptr->name);
			delete ptr;
		}
	}
}

int hash_table::_hash(const char *name) {
	unsigned int i = 0, sum = 0;

	while (name[i] != '\0') {
		sum += name[i];
		i++;
	}
	return (sum % HASH_SIZE);
}

hash_elem *
hash_table::find(const char *name, int nsize) {
	PDEBUG(DFULL, "name=%s, nsize=%d", name, nsize);
	int i = _hash(name);

	PDEBUG(DFULL, "_hash=%d, table[i].size=%d", i, table[i].size());

	if (!table[i].size()) { // hash list is empty
		PDEBUG(DFULL, "return NULL");
		return NULL;
	}

	PDEBUG(DFULL, "Continue to search");

	list<hash_elem *>::iterator p = table[i].begin();
	PDEBUG(DFULL, "start for");
	for (; p != table[i].end(); p++) {
		PDEBUG(DFULL, "iter start");
		hash_elem *ptr = *p;
		PDEBUG(DFULL, "inspect p->name=%s, search=%s",ptr->name,name);
		int len = (ptr->nsize > nsize) ? nsize : ptr->nsize;
		if (!strncmp(ptr->name, name, len)) {
			PDEBUG(DFULL,"return");
			return ptr;
		}
		PDEBUG(DFULL, "iter end");
	}
	return NULL;
}

// vy moska maladec))))

int hash_table::add(hash_elem *el) {
	ASSERT(el);
	int len = (el->nsize < max_hash_name) ? el->nsize : max_hash_name;
	int i = _hash(el->name);

	el->next = el->prev = NULL;
	hash_elem *exist = find(el->name, el->nsize);
	if (exist)
		return -1;
	// add to hash table
	table[i].push_back(el);

	// add to sequential list
	if (!head) {
		head = el;
		tail = el;
	} else {
		tail->next = el;
		el->prev = tail;
		tail = el;
	}

	return 0;
}

hash_elem *
hash_table::del(const char *name, int nsize) {

	PDEBUG(DFULL,"start");
	int i = _hash(name);
	PDEBUG(DFULL,"hash=%d",i);
	if (!table[i].size()) // hash list is empty
		return NULL;
	PDEBUG(DFULL,"hash list is not empty");
	list<hash_elem *>::iterator p;
	for (p = table[i].begin(); p != table[i].end(); p++) {
		hash_elem *ptr = *p;
		int len = (ptr->nsize > nsize) ? nsize : ptr->nsize;
		if (!strncmp(ptr->name, name, len)) {
			// delete from hash-table
			PDEBUG(DFULL,"delete element \"%s\" from list",name);
			table[i].erase(p);
			// delete from sequential list
			PDEBUG(DFULL,"delete element \"%s\" from sequental list",name);
			if (ptr->prev && ptr->next) {
				PDEBUG(DFULL,"element is in midddle");
				ptr->prev->next = ptr->next;
				ptr->next->prev = ptr->prev;
			} else {
				if (!ptr->prev) {
					PDEBUG(DFULL,"element is in head");
					ASSERT(head == ptr);
					head = ptr->next;
					if (head)
						head->prev = NULL;
				}
				if (!ptr->next) {
					PDEBUG(DFULL,"element is in tail");
					ASSERT(tail == ptr);
					tail = ptr->prev;
					if (tail)
						tail->next = NULL;
				}
			}
			hash_elem *nx = ptr->next;
			PDEBUG(DFULL,"Cur=%p, name=%s; prev=%p, name=%s",ptr,ptr->name,
				ptr->prev,ptr->prev ? ptr->prev->name : "NULL");
			PDEBUG(DFULL,"next element is \"%s\"",nx ? nx->name : "NULL");
			return ptr;
		}
	}
	return NULL;
}

void hash_table::clear(void (*del_func)(hash_elem *el)) {
	hash_elem *el = first();
	while (el) {
		hash_elem *tmp;
		PDEBUG(DFULL, "Process %s entry", el->name);
		if (!el->is_updated) {
			tmp = el;
			el = next(el->name,el->nsize);
			PDEBUG(DFULL, "Delete row: %s", tmp->name);
			tmp = del(tmp->name, tmp->nsize);

			if( tmp ){
				PDEBUG(DFULL,"delete elem=%s",tmp->name);
				del_func(tmp);
			}
			PDEBUG(DFULL, "Next is: %s", el ? el->name : "NULL");
		} else {
			PDEBUG(DFULL, "Keep profile %s", el->name);
			el->is_updated = 0;
			el = next(el->name, el->nsize);
		}

	}
	// DEBUG PRINT
	el = first();
	PDEBUG(DFULL, "Print new table");
	while (el) {
		PDEBUG(DFULL, "\t%s", el->name);
		el = next(el->name, el->nsize);
	}
}

// BUBLE sort. TODO: use more optimal
void hash_table::sort() {
	int flag = 1;

	while (flag) {
		flag = 0;
		hash_elem *el = first();
		while (el && el->next) {
			hash_elem *nel = el->next;
			int len = (el->nsize > nel->nsize) ? nel->nsize : el->nsize;
			if (strncmp(el->name, nel->name, len) > 0) {
				flag = 1;
				// Change sequence in list
				if (el == head && nel == tail) { // Have only 2 elements in list
					tail = el;
					head = nel;
					el->prev = nel;
					el->next = NULL;
					nel->next = el;
					nel->prev = NULL;
				} else if (el == head) {
					head = nel;
					el->next = nel->next;
					el->next->prev = el;
					el->prev = nel;
					nel->next = el;
					nel->prev = NULL;
				} else if (nel == tail) {
					tail = el;
					nel->next = el;
					nel->prev = el->prev;
					el->prev->next = nel;
					el->prev = nel;
					el->next = NULL;
				} else {
					el->prev->next = nel;
					nel->next->prev = el;
					el->next = nel->next;
					nel->prev = el->prev;
					el->prev = nel;
					nel->next = el;
				}

			} else {
				el = nel;
			}
		}
	}
}
