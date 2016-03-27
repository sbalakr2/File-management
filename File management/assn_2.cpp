#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <iomanip>
using namespace std;

void getUserCommand(fstream &file, fstream &index, fstream &avlist, string order);
void addObjectToIndexArr(int sid, long offset);
int binarySearch(int s, int l, int r);
void writeToDiskAndExit(fstream &file, fstream &index, fstream &avlist);
void sortAvailList(int size, long offset, string order);
void readListToArray(fstream &index, fstream &avail);
void insertIntoAvailArrayOnOrder(int size, long offset, string order);
int binarySearchAvail(int s, long off, int l, int r, string order);

struct index_S {
	int key;
	long off;
};

struct avail_S {
	int siz;
	long off;
};

index_S *ixarr = new index_S[10000];
avail_S *avarr = new avail_S[10000];
int indexCnt = 0;
int availCnt = 0;

void addObjectToIndexArr(int key, long offset) {
	if (indexCnt == 0) {
		ixarr[0].key = key;
		ixarr[0].off = offset;
		indexCnt = 1;
	}
	else {
		int i = binarySearch(key, 0, indexCnt - 1);
		if (i != -1) {
			ixarr[i].key = key;
			ixarr[i].off = offset;
			indexCnt++;
		}
	}
}

void addObjectToAvailArr(int size, long offset, string order) {
	if (availCnt == 0) {
		avarr[0].siz = size;
		avarr[0].off = offset;
		availCnt = 1;
	}
	else {
		insertIntoAvailArrayOnOrder(size, offset, order);
	}
}

void insertIntoAvailArrayOnOrder(int size, long offset, string order) {
	if (order == "--first-fit") {
		avarr[availCnt].siz = size;
		avarr[availCnt].off = offset;
		availCnt++;
	}
	else {
		int i = -1;
		// insert object in asc order
		i = binarySearchAvail(size, offset, 0, availCnt - 1, order);
		if (i != -1) {
			avarr[i].siz = size;
			avarr[i].off = offset;
			availCnt++;
		}
	}
}

int binarySearch(int s, int l, int r) {
	int n = r - l + 1;
	if (l == r) {
		// case when there is only one element
		if (s < ixarr[l].key) {
			// shift the elements to the right by one position
			for (int k = indexCnt; k > l; k--) {
				ixarr[k] = ixarr[k - 1];
			}
			return l;
		}
		else {
			// shift the elements to the right by one position
			for (int k = indexCnt; k > (l + 1); k--) {
				ixarr[k] = ixarr[k - 1];
			}
			return l + 1;
		}
	}
	else {
		if (n < 0) { return -1; }
		else if (n == 0) { return r + 1; }
		else {
			int c = (int)(l + floor(n / 2));
			if (s < ixarr[c].key) {
				return binarySearch(s, l, c - 1);
			}
			else {
				if ((c + 1) > r) { c = r - 1; }
				return binarySearch(s, c + 1, r);
			}
		}
	}
}

int binarySearchAvail(int s, long off, int l, int r, string order) {
	int n = r - l + 1;
	if (l == r) {
		// case when there is only one element
		bool con = (order == "--best-fit") ? (s < avarr[l].siz) : (s > avarr[l].siz);
		if (s == avarr[l].siz) {
			con = (off < avarr[l].off);
		}
		if (con) {
			// shift the elements to the right by one position
			for (int k = availCnt; k > l; k--) {
				avarr[k] = avarr[k - 1];
			}
			return l;
		}
		else {
			// shift the elements to the right by one position
			for (int k = availCnt; k > (l + 1); k--) {
				avarr[k] = avarr[k - 1];
			}
			return l + 1;
		}
	}
	else {
		if (n < 0) { return -1; }
		else if (n == 0) { return r + 1; }
		else {
			int c = (int)(l + floor(n / 2));
			bool con = (order == "--best-fit") ? (s < avarr[c].siz) : (s > avarr[c].siz);
			if (s == avarr[c].siz) {
				con = (off < avarr[c].off);
			}
			if (con) {
				return binarySearchAvail(s, off, l, c - 1, order);
			}
			else {
				return binarySearchAvail(s, off, c + 1, r, order);
			}
		}
	}
}

int binarySearchForKey(int s, int l, int r) {
	int n = r - l + 1;
	if (n <= 0) { return -1; }
	else {
		int c = (int)(l + floor(n / 2));
		if (s == ixarr[c].key) {
			return c;
		}
		else if (s < ixarr[c].key) {
			return binarySearchForKey(s, l, c - 1);
		}
		else {
			return binarySearchForKey(s, c + 1, r);
		}
	}
}

long getOffsetToWrite(int size, fstream &file, string order) {
	file.seekg(0, ios::end);
	long offset = (long)file.tellg();  // EOF as offset if a hole is not found
	// search the avail list array for a hole
	if (availCnt > 0) {
		for (int i = 0; i < availCnt; i++) {
			int curholeSize = avarr[i].siz;
			if (curholeSize >= size) {
				offset = avarr[i].off;
				for (int j = i; j < availCnt; j++) {
					avarr[j].siz = avarr[j + 1].siz;
					avarr[j].off = avarr[j + 1].off;
				}
				availCnt--;
				// add to avail list in case of internal fragmentation
				int newsize = curholeSize - size;
				long newoffset = offset + size;
				addObjectToAvailArr(newsize, newoffset, order);
				break;
			}
		}
	}
	return offset;
}

void writeRecToFileOffset(fstream &file, long offset, int size, string rec) {
	char *buffer = new char[size];
	memcpy(buffer, rec.c_str(), size);
	file.seekg(offset, ios::beg);
	file.write(reinterpret_cast<const char*>(&size), sizeof(size));	
	file.write(buffer, size);
	delete[] buffer;
}

void addRecord(fstream &file, fstream &index, fstream &avlist, string data, string order) {
	int pos = data.find(" ");
	string sub = data.substr(0, pos);
	int sid = atoi(sub.c_str());
	string rec = data.substr(pos + 1, data.size());
	int recsize = rec.size();
	int length = sizeof(recsize);
	int byteCount = recsize + length;
	if (file.is_open()) {
		// check the index if primary key exists
		if (binarySearchForKey(sid, 0, indexCnt - 1) == -1) {
			long offset = getOffsetToWrite(byteCount, file, order);
			writeRecToFileOffset(file, offset, recsize, rec);
			// write to index file
			addObjectToIndexArr(sid, offset);
		}
		else {
			cout << "Record with SID=" << sid << " exists" << endl;
			// get next user input
			getUserCommand(file, index, avlist, order);
		}
	}
	else {
		cout << "Unable to open data file" << endl;
	}
	// get next user input
	getUserCommand(file, index, avlist, order);
}

void findRecord(fstream &file, fstream &index, fstream &avlist, string data, string order) {
	// binary search for key in index file and get offset
	int sid = atoi(data.c_str());
	if (binarySearchForKey(sid, 0, indexCnt - 1) == -1) {
		cout << "No record with SID=" << sid << " exists" << endl;
	}
	else {
		int index = binarySearchForKey(sid, 0, indexCnt - 1);
		int offset = ixarr[index].off;
		file.seekg(offset, ios::beg);
		int *recsz = new int[1];
		file.read(reinterpret_cast<char*>(&recsz[1]), sizeof(int));
		int sz = *(recsz + 1);
		char *rdbuf = new char[sz];
		file.read(rdbuf, sz);
		for (int i = 0; i < sz; i++) {
			cout << *(rdbuf + i);
		}
		cout << endl;
		delete[] rdbuf;
	}
	// get next user input
	getUserCommand(file, index, avlist, order);
}

void delRecord(fstream &file, fstream &index, fstream &avlist, string data, string order) {
	// binary search for key in index file and get offset
	int sid = atoi(data.c_str());
	if (binarySearchForKey(sid, 0, indexCnt - 1) == -1) {
		cout << "No record with SID=" << sid << " exists" << endl;
	}
	else {
		int index = binarySearchForKey(sid, 0, indexCnt - 1);
		int offset = ixarr[index].off;
		file.seekg(offset, ios::beg);
		int *recsz = new int[1];
		file.read(reinterpret_cast<char*>(&recsz[1]), sizeof(int));
		int sz = *(recsz + 1);

		// update avail list
		int hole_size = sz + sizeof(int);  // total size of space created by deletion
		addObjectToAvailArr(hole_size, offset, order);

		// remove entry from index
		for (int i = 0; i < indexCnt; i++) {
			if (ixarr[i].key == sid) {
				for (int j = i; j < indexCnt; j++) {
					ixarr[j].key = ixarr[j + 1].key;
					ixarr[j].off = ixarr[j + 1].off;
				}
				indexCnt--;
			}
		}
	}
	// get next user input
	getUserCommand(file, index, avlist, order);
}

void getUserCommand(fstream &file, fstream &index, fstream &avlist, string order) {
	string str;
	getline(cin, str);
	int pos = str.find(" ");
	string cmd = str.substr(0, pos);
	string data = str.substr(pos + 1, str.size());

	if (cmd.compare("add") == 0) {
		addRecord(file, index, avlist, data, order);
	}
	else if (cmd.compare("find") == 0) {
		findRecord(file, index, avlist, data, order);
	}
	else if (cmd.compare("del") == 0) {
		delRecord(file, index, avlist, data, order);
	}
	else if (cmd.compare("end") == 0) {
		writeToDiskAndExit(file, index, avlist);
	}
	else {
		cout << "Enter a valid command: add, find, del or end" << endl;
		getUserCommand(file, index, avlist, order);
	}
}

void writeToDiskAndExit(fstream &file, fstream &index, fstream &avlist) {
	// clear file contents before writing
	index.close();
	avlist.close();
	index.open("index.bin", ios::out | ios::in | ios::binary | ios::trunc);
	avlist.open("avail.bin", ios::out | ios::in | ios::binary | ios::trunc);

	index.seekg(0, ios::beg);
	for (int a = 0; a < indexCnt; a++) {
		index.write(reinterpret_cast<char*>(&ixarr[a].key), sizeof(int));
		index.write(reinterpret_cast<char*>(&ixarr[a].off), sizeof(long));
	}
	avlist.seekg(0, ios::beg);
	for (int c = 0; c < availCnt; c++) {
		avlist.write(reinterpret_cast<char*>(&avarr[c].siz), sizeof(int));
		avlist.write(reinterpret_cast<char*>(&avarr[c].off), sizeof(long));
	}

	cout << "Index:" << endl;
	for (int x = 0; x < indexCnt; x++) {
		cout << "key=" << ixarr[x].key << ": offset=" << ixarr[x].off << endl;
	}
	cout << "Availability:" << endl;
	for (int y = 0; y < availCnt; y++) {
		cout << "size=" << avarr[y].siz << ": offset=" << avarr[y].off << endl;
	}

	int sum_siz = 0;
	for (int z = 0; z < availCnt; z++) {
		sum_siz += avarr[z].siz;
	}

	cout << "Number of holes: " << availCnt << endl;
	cout << "Hole space: " << sum_siz <<endl;

	delete[] ixarr;
	delete[] avarr;
	file.close();
	index.close();
	avlist.close();
	exit(0);
}

void readListToArray(fstream &index, fstream &avail) {
	index.seekg(0, ios::end);
	int len = (int)(index.tellg());
	index.seekg(0, ios::beg);
	indexCnt = len / (sizeof(int) + sizeof(long));

	for (int b = 0; b < indexCnt; b++) {
		index.read(reinterpret_cast<char*>(&ixarr[b].key), sizeof(int));
		index.read(reinterpret_cast<char*>(&ixarr[b].off), sizeof(long));
	}

	avail.seekg(0, ios::end);
	int avlen = (int)(avail.tellg());
	avail.seekg(0, ios::beg);
	availCnt = avlen / (sizeof(int) + sizeof(long));

	for (int c = 0; c < availCnt; c++) {
		avail.read(reinterpret_cast<char*>(&avarr[c].siz), sizeof(int));
		avail.read(reinterpret_cast<char*>(&avarr[c].off), sizeof(long));
	}
}


int main(int argc, char *argv[]) {
	if (argc != 3) {
		cout << "\n Enter the right number of parameters of the format: avail-list-order studentfile-name" << endl;
	}
	else {
		string availListOrder = argv[1];
		string studentFileName = argv[2];
		ifstream file(studentFileName.c_str());
		if (!file) {
			fstream fp(studentFileName.c_str(), ios::out);
			fstream ixfp("index.bin", ios::out);
			fstream avfp("avail.bin", ios::out);
			fp.close();
			ixfp.close();
			avfp.close();
		}
		fstream fp(studentFileName.c_str(), ios::out | ios::in | ios::binary);
		fstream ixfp("index.bin", ios::out | ios::in | ios::binary);
		fstream avfp("avail.bin", ios::out | ios::in | ios::binary);

		readListToArray(ixfp, avfp);

		if (availListOrder.compare("--first-fit") == 0 || availListOrder.compare("--best-fit") == 0
			|| availListOrder.compare("--worst-fit") == 0) {
			getUserCommand(fp, ixfp, avfp, availListOrder);
		}
		else {
			cout << "\n The availability list order entered is invalid" << endl;
		}
	}
	return 0;
}
