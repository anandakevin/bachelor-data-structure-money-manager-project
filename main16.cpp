#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#define UP		0x48
#define DOWN	0x50
#define LEFT	0x4B
#define RIGHT	0x4D
#define ENTER	0xD
#define SPACE	0x20
#define ESC 	27
#define SP		32

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <ctype.h>
#include <conio.h>

const int maxttlLines = 100;

FILE *fp;

struct date{
	int dd, mm, yy;
};

struct acc{
    char name[20];
    float currBal;
    acc *prev, *next;
}*firstAcc, *lastAcc; //First and Last Account

struct ctg{
	char name[20], ID[3];
	char baseEff;
	//This is to determine whether if this is plus,
	//then the base of saldo for this category is plus, and vice versa
	ctg *next, *prev;
}*ctghead, *ctgtail; // Type of transaction category

struct transaction{
	char name[36], ID[11], trsType[20], account[20];
	date tgl;
	float amount;
	transaction *prev, *next;
}*trshead, *trstail; //List of Transactions

struct note{
	char content[10000];
	note *next, *prev;
}*notehead, *notetail;

void warna(int color){
	//http://www.cplusplus.com/forum/beginner/103397/
	HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hCon,color);
}

acc * pushFrontAcc(const char name[], float currBal) {
	acc *newAcc = (acc *)malloc(sizeof(acc));
	newAcc->prev = newAcc->next = NULL;
	strcpy(newAcc->name, name);
	newAcc->currBal = currBal;
	if (!firstAcc) firstAcc = lastAcc = newAcc;
	else {
		firstAcc->prev = newAcc;
		newAcc->next = firstAcc;
		firstAcc = newAcc;
	}
	return newAcc;
}

acc * pushMidAcc(const char name[], float currBal, acc **curr) {
	acc *newAcc = (acc *)malloc(sizeof(acc));
	newAcc->prev = newAcc->next = NULL;
	strcpy(newAcc->name, name);
	newAcc->currBal = currBal;
	(*curr)->prev->next = newAcc;
	newAcc->prev = (*curr)->prev;
	(*curr)->prev = newAcc;
	newAcc->next = (*curr);
	return newAcc;
}

acc * pushBackAcc(const char name[], float bal) {
	acc *newAcc = (acc *)malloc(sizeof(acc));
	newAcc->prev = newAcc->next = NULL;
	strcpy(newAcc->name, name);
	newAcc->currBal = bal;
	if (!firstAcc) firstAcc = lastAcc = newAcc;
	else {
		lastAcc->next = newAcc;
		newAcc->prev = lastAcc;
		lastAcc = newAcc;
	}
	return newAcc;
}

acc * pushAccByName(const char name[], float bal) {
	acc *newAcc = NULL;
	if (firstAcc) {
		if (strcmpi(name, firstAcc->name) < 0) newAcc = pushFrontAcc(name,bal);
		else if (strcmpi(name, lastAcc->name) > 0) newAcc = pushBackAcc(name, bal);
		else {
			acc *curr = firstAcc;
			while (curr != NULL && strcmpi(name, curr->name) > 0) curr = curr->next;
			newAcc = pushMidAcc(name, bal, &curr);
		}
	}
	else pushFrontAcc(name, bal);
	return newAcc;
}

ctg * pushFrontCtg(const char name[], const char id[], char eff) {
	ctg *newCtg = (ctg *)malloc(sizeof(ctg));
	newCtg->prev = newCtg->next = NULL;
	strcpy(newCtg->name, name);
	strcpy(newCtg->ID, id);
	newCtg->baseEff = eff;
	if (!ctghead) ctghead = ctgtail = newCtg;
	else {
		ctghead->prev = newCtg;
		newCtg->next = ctghead;
		ctghead = newCtg;
	}
	return newCtg;
}

ctg * pushMidCtg(const char name[], const char id[], ctg **curr, char eff) {
	ctg *newCtg = (ctg *)malloc(sizeof(ctg));
	newCtg->prev = newCtg->next = NULL;
	strcpy(newCtg->name, name);
	strcpy(newCtg->ID, id);
	newCtg->baseEff = eff;
	(*curr)->prev->next = newCtg;
	newCtg->prev = (*curr)->prev;
	(*curr)->prev = newCtg;
	newCtg->next = (*curr);
	return newCtg;
}

ctg * pushBackCtg(const char name[], const char id[], char eff) {
	ctg *newCtg = (ctg *)malloc(sizeof(ctg));
	newCtg->prev = newCtg->next = NULL;
	strcpy(newCtg->name, name);
	strcpy(newCtg->ID, id);
	newCtg->baseEff = eff;
	if (!ctghead) ctghead = ctgtail = newCtg;
	else {
		ctgtail->next = newCtg;
		newCtg->prev = ctgtail;
		ctgtail = newCtg;
	}
	return newCtg;
}

ctg * pushCtgByName(const char name[], const char id[], char eff) {
	ctg *newCtg = NULL;
	if (ctghead) {
		if (strcmpi(name, ctghead->name) < 0) newCtg = pushFrontCtg(name, id, eff);
		else if (strcmpi(name, ctgtail->name) > 0) newCtg = pushBackCtg(name, id, eff);
		else {
			ctg *curr = ctghead;
			while (curr != NULL && strcmpi(name, curr->name) > 0) curr = curr->next;
			newCtg = pushMidCtg(name, id, &curr, eff);
		}
	}
	else pushFrontCtg(name, id, eff);
	return newCtg;
}

transaction * pushBackTrs(const char name[], struct date tgl, char type[], float amount, char ID[], char account[]) {
	transaction *newTrs = (transaction *)malloc(sizeof(transaction));
	newTrs->prev = newTrs->next = NULL;
	strcpy(newTrs->name, name);
	strcpy(newTrs->trsType, type);
	strcpy(newTrs->ID, ID);
	newTrs->tgl = tgl;
	newTrs->amount = amount;
	strcpy(newTrs->account, account);
	if (!trshead) trshead = trstail = newTrs;
	else {
		trstail->next = newTrs;
		newTrs->prev = trstail;
		trstail = newTrs;
	}
	return newTrs;
}

void pushBackNote(char content[]) {
	note *newNote = (note *)malloc(sizeof(note));
	newNote->prev = newNote->next = NULL;
	strcpy(newNote->content, content);
	if (!notehead) notehead = notetail = newNote;
	else {
		notetail->next = newNote;
		newNote->prev = notetail;
		notetail = newNote;
	}
}

transaction *copyTrsValue(transaction *curr) {
	transaction *temp = (transaction *)malloc(sizeof(transaction));
	strcpy(temp->name, curr->name);
	strcpy(temp->trsType, curr->trsType);
	strcpy(temp->ID, curr->ID);
	temp->tgl = curr->tgl;
	temp->amount = curr->amount;
	strcpy(temp->account, curr->account);
	return temp;
}

transaction * popTrsHead() {
	transaction *temp = copyTrsValue(trshead);
	if (trshead == trstail) {
		free(trshead); trshead = trstail = NULL;
	}
	else {
		trshead = trshead->next;
		free(trshead->prev);
		trshead->prev = NULL;
	}
	return temp;
}

transaction * popTrsMid(transaction **curr) {
	transaction *temp = copyTrsValue(*curr);
	(*curr)->prev->next = (*curr)->next;
	(*curr)->next->prev = (*curr)->prev;
	free(*curr);
	return temp;
}

transaction * popTrsTail() {
	transaction *temp = copyTrsValue(trstail);
	if (trshead == trstail) {
		free(trshead); trshead = trstail = NULL;
	}
	else {
		trstail = trstail->prev;
		free(trstail->next);
		trstail->next = NULL;
	}
	return temp;
}

ctg * copyCtgValue(ctg *curr) {
	ctg * temp = (ctg *)malloc(sizeof(ctg));
	strcpy(temp->ID, curr->ID);
	strcpy(temp->name, curr->name);
	return temp;
}

ctg * popCtgHead() {
	ctg *temp = copyCtgValue(ctghead);
	if (ctghead == ctgtail) {
		free(ctghead); ctghead = ctgtail = NULL;
	}
	else {
		ctghead = ctghead->next;
		free(ctghead->prev);
		ctghead->prev = NULL;
	}
	return temp;
}

ctg * popCtgMid(int idx) {
	ctg *curr = ctghead;
	for (int i = 1; i < idx; i++) curr = curr->next;
	ctg *temp = copyCtgValue(curr);
	curr->prev->next = curr->next;
	curr->next->prev = curr->prev;
	curr->next = curr->prev = NULL;
	free(curr);
	return temp;
}

ctg * popCtgTail() {
	ctg *temp = copyCtgValue(ctgtail);
	if (ctghead == ctgtail) {
		free(ctghead); ctghead = ctgtail = NULL;
	}
	else {
		ctgtail = ctgtail->prev;
		free(ctgtail->next);
		ctgtail->next = NULL;
	}
	return temp;
}

ctg * popCtg(int idx, int ttl) {
	ctg *temp;
	if (idx == 1) temp = popCtgHead();
	else if (idx == ttl) temp = popCtgTail();
	else temp = popCtgMid(idx);
	return temp;
}

acc * copyAccValue(acc *curr) {
	acc * temp = (acc *)malloc(sizeof(acc));
	strcpy(temp->name, curr->name);
	temp->currBal = curr->currBal;
	return temp;
}

acc * popAccHead() {
	acc *temp = copyAccValue(firstAcc);
	if (firstAcc == lastAcc) {
		free(firstAcc); firstAcc = lastAcc = NULL;
	}
	else {
		firstAcc = firstAcc->next;
		free(firstAcc->prev);
		firstAcc->prev = NULL;
	}
	return temp;
}

acc * popAccMid(int idx) {
	acc *curr = firstAcc;
	for (int i = 1; i < idx; i++) curr = curr->next;
	acc *temp = copyAccValue(curr);
	curr->prev->next = curr->next;
	curr->next->prev = curr->prev;
	curr->next = curr->prev = NULL;
	free(curr);
	return temp;
}

acc * popAccTail() {
	acc *temp = copyAccValue(lastAcc);
	if (firstAcc == lastAcc) {
		free(firstAcc);
		firstAcc = lastAcc = NULL;
	}
	else {
		lastAcc = lastAcc->prev;
		free(lastAcc->next);
		lastAcc->next = NULL;
	}
	return temp;
}

acc * popAcc(int idx, int ttl) {
	acc *temp = NULL;
	if (idx == 1) temp = popAccHead();
	else if (idx == ttl) temp = popAccTail();
	else temp = popAccMid(idx);
	return temp;
}

void popNoteHead() {
	if (notehead == notetail) {
		free(notehead); notehead = notetail = NULL;
	}
	else {
		notehead = notehead->next;
		free(notehead->prev);
		notehead->prev = NULL;
	}
}

void popNoteMid(int idx) {
	note *curr = notehead;
	for (int i = 1; i < idx; i++) curr = curr->next;
	curr->prev->next = curr->next;
	curr->next->prev = curr->prev;
	curr->next = curr->prev = NULL;
	free(curr);
}

void popNoteTail() {
	if (notehead == notetail) {
		free(notehead);
		notehead=notetail=NULL;
	}
	else {
		notetail = notetail->prev;
		free(notetail->next);
		notetail->next = NULL;
	}
}

void popNote(int idx, int ttl) {
	if (idx == 1) popNoteHead();
	else if (idx == ttl) popNoteTail();
	else popNoteMid(idx);
}

void readAccData() {
	fp = fopen("Accounts.txt", "r");
	if (fp != NULL) {
		char name[20];
		float currBal;
		int size;
		fseek(fp, 0, SEEK_END);
		size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		if(size != 0) {
			while (!feof(fp)) {
				fscanf(fp, "%[^-]-%f\n", name, &currBal);
				pushAccByName(name, currBal);
			}
		}
	}
	else return;
	fclose(fp);
}

void readCtgData() {
	fp = fopen("Categories.txt", "r");
	if (fp != NULL) {
		char name[20], id[3], eff;
		int size;
		fseek(fp, 0, SEEK_END);
		size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		if(size != 0) {
			while (!feof(fp)) {
				fscanf(fp, "%[^-]-%[^-]-%c\n", name, id, &eff);
				pushCtgByName(name, id, eff);
			}
		}
	}
	else return;
	fclose(fp);
}

void readTrsData() {
	fp = fopen("Transactions.txt", "r");
	if (fp != NULL) {
		char name[36], id[11], trsType[20], account[20];
		float amount = 0;
		date tgl;
		tgl.dd = tgl.mm = tgl.yy = 0;
		int size;
		fseek(fp, 0, SEEK_END);
		size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		if (size != 0) {
			while (!feof(fp)) {
				fscanf(fp, "%[^-]-%[^-]-%d-%d-%d-%[^-]-%f-%[^\n]\n", name, id, &tgl.dd, &tgl.mm, &tgl.yy, trsType, &amount, account);
				pushBackTrs(name, tgl, trsType, amount, id, account);
			}
		}
	}
	else return;
	fclose(fp);
}

void readNoteData() {
	fp = fopen("Notes.txt", "r");
	if (fp != NULL) {
		char note[10000];
		int size;
		fseek(fp, 0, SEEK_END);
		size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		if (size != 0) {
			while (!feof(fp)) {
				fscanf(fp, "%[^\n]\n", note);
				pushBackNote(note);
			}
		}
	}
	else return;
	fclose(fp);
}

void writeAccData() {
	fp = fopen("Accounts.txt", "w");
	if (firstAcc) {
		for (acc *curr = firstAcc; curr != NULL; curr = curr->next) {
			fprintf(fp, "%s-%f\n", curr->name, curr->currBal);
		}
	}
	fclose(fp);
}

void writeCtgData() {
	fp = fopen("Categories.txt", "w");
	if (ctghead) {
		for (ctg *curr = ctghead; curr != NULL; curr = curr->next) {
			fprintf(fp, "%s-%s-%c\n", curr->name, curr->ID, curr->baseEff);
		}
	}
	fclose(fp);
}

void writeTrsData() {
	fp = fopen("Transactions.txt", "w");
	for (transaction *curr = trshead; curr != NULL; curr = curr->next) {
			fprintf(fp, "%s-%s-%d-%d-%d-%s-%.2f-%s\n",
				curr->name, curr->ID, curr->tgl.dd, curr->tgl.mm,
				curr->tgl.yy, curr->trsType, curr->amount, curr->account);
	}
	fclose(fp);
}

void writeNoteData() {
	fp = fopen("Notes.txt", "w");
	if (notehead) {
		for (note *curr = notehead; curr != NULL; curr = curr->next) {
			fprintf(fp, "%s\n", curr->content);
		}
	}
	fclose(fp);
}

void title() {
	printf("\n\n");

	printf("\t\t888b     d888                                     888      d8b          888    \n");
	printf("\t\t8888b   d8888                                     888      Y8P          888    \n");
	printf("\t\t88888b.d88888                                     888                   888    \n");
	printf("\t\t888Y88888P888  .d88b.  88888b.   .d88b.  888  888 888      888 .d8888b  888888 \n");
	printf("\t\t888 Y888P 888 d88\"\"88b 888 \"88b d8P  Y8b 888  888 888      888 88K      888    \n");
	printf("\t\t888  Y8P  888 888  888 888  888 88888888 888  888 888      888 \"Y8888b. 888    \n");
	printf("\t\t888   \"   888 Y88..88P 888  888 Y8b.     Y88b 888 888      888      X88 Y88b.  \n");
	printf("\t\t888       888  \"Y88P\"  888  888  \"Y8888   \"Y88888 88888888 888  88888P'  \"Y888 \n");
	printf("\t\t                                              888                              \n");
	printf("\t\t                                         Y8b d88P                              \n");
	printf("\t\t                                          \"Y88P\"                               \n");

	printf("\n\t\t\t\t\t\t\t - An easy way to manage your money -");
}

void titleAdd() {
	printf("\n\n");

	printf("\t\t                  _      _    _   __  __                            \n");
	printf("\t\t  ___ ___ ___    /_\\  __| |__| | |  \\/  |___ _ _ _  _   ___ ___ ___ \n");
	printf("\t\t |___|___|___|  / _ \\/ _` / _` | | |\\/| / -_) ' \\ || | |___|___|___|\n");
	printf("\t\t               /_/ \\_\\__,_\\__,_| |_|  |_\\___|_||_\\_,_|              \n");
}

void titleView() {
	printf("\n\n");

	printf("\t\t               __   ___              __  __                            \n");
	printf("\t\t  ___ ___ ___  \\ \\ / (_)_____ __ __ |  \\/  |___ _ _ _  _   ___ ___ ___ \n");
	printf("\t\t |___|___|___|  \\ V /| / -_) V  V / | |\\/| / -_) ' \\ || | |___|___|___|\n");
	printf("\t\t                 \\_/ |_\\___|\\_/\\_/  |_|  |_\\___|_||_\\_,_|              \n");
}

void titleEdit(){
	printf("\n\n");

	printf("\t\t                ___    _ _ _     __  __                            \n");
	printf("\t\t  ___ ___ ___  | __|__| (_) |_  |  \\/  |___ _ _ _  _   ___ ___ ___ \n");
	printf("\t\t |___|___|___| | _|/ _` | |  _| | |\\/| / -_) ' \\ || | |___|___|___|\n");
	printf("\t\t               |___\\__,_|_|\\__| |_|  |_\\___|_||_\\_,_|              \n");
}

void titleNotes(){
	printf("\n\n");

	printf("\t\t                _  _     _            __  __                            \n");
	printf("\t\t  ___ ___ ___  | \\| |___| |_ ___ ___ |  \\/  |___ _ _ _  _   ___ ___ ___ \n");
	printf("\t\t |___|___|___| | .` / _ \\  _/ -_|_-< | |\\/| / -_) ' \\ || | |___|___|___|\n");
	printf("\t\t               |_|\\_\\___/\\__\\___/__/ |_|  |_\\___|_||_\\_,_|              \n");
}

void titleEditTrans(){
	printf("\n\n");

	printf("\t                ___    _ _ _     _____                          _   _                        \n");
	printf("\t  ___ ___ ___  | __|__| (_) |_  |_   _| _ __ _ _ _  ___ __ _ __| |_(_)___ _ _    ___ ___ ___ \n");
	printf("\t |___|___|___| | _|/ _` | |  _|   | || '_/ _` | ' \\(_-</ _` / _|  _| / _ \\ ' \\  |___|___|___|\n");
	printf("\t               |___\\__,_|_|\\__|   |_||_| \\__,_|_||_/__/\\__,_\\__|\\__|_\\___/_||_|              \n");
}

void titleEditCtg(){
	printf("\n\n");

	printf("\t                ___    _ _ _      ___      _                                       \n");
	printf("\t  ___ ___ ___  | __|__| (_) |_   / __|__ _| |_ ___ __ _ ___ _ _ _  _   ___ ___ ___ \n");
	printf("\t |___|___|___| | _|/ _` | |  _| | (__/ _` |  _/ -_) _` / _ \\ '_| || | |___|___|___|\n");
	printf("\t               |___\\__,_|_|\\__|  \\___\\__,_|\\__\\___\\__, \\___/_|  \\_, |              \n");
	printf("\t                                                  |___/         |__/               ");
}

void titleEditAcc(){
	printf("\n\n");

	printf("\t                ___    _ _ _       _                      _                 \n");
	printf("\t  ___ ___ ___  | __|__| (_) |_    /_\\  __ __ ___ _  _ _ _| |_   ___ ___ ___ \n");
	printf("\t |___|___|___| | _|/ _` | |  _|  / _ \\/ _/ _/ _ \\ || | ' \\  _| |___|___|___|\n");
	printf("\t               |___\\__,_|_|\\__| /_/ \\_\\__\\__\\___/\\_,_|_||_\\__|              \n");
}

void loadingForSaveFile() {
	for (int i = 0; i < 11; i++) {
		system("cls");
		printf("\n\n\n\n\t\t\t\t\t\t Thank you for using our app!!");
		printf("\n\n\t\t\t\t\t\tThis app is brought to you by:\n");
		printf("\n\t\t\t\t\t\t   Ananda Kevin - 2101724320\n");
		printf("\n\t\t\t\t\t\t\t\t&\n");
		printf("\n\t\t\t\t\t\tJessica Clarisa H. - 2101724541\n\n\n\n\n\n");
		printf("\n\n\t\t\t\t\t     Saving your files, please wait...");
		printf("\n\n\t\t\t\t\t           ");
		for (int j = (11 - i); j<11; j++) printf("%c ", 254);
		printf("\n\n\t\t\t\t\t\t\t   %d %c", (i * 10), 37);
		Sleep(rand() % 101 + 100);
	}
}

void loadingForLoadFile() {
	for (int i = 0; i < 11; i++) {
		system("cls");
		printf("\n\n\n\n\t\t\t\t     Loading your files, please wait...");
		printf("\n\n\t\t\t\t            ");
		for (int j = (11 - i); j<11; j++) printf("%c ", 254);
		printf("\n\n\t\t\t\t\t\t  %d %c", (i * 10), 37);
		Sleep(rand() % 101 + 100);
	}
}

void loading() {
	for (int i = 1; i < 11; i++) {
		system("cls");
		printf("\n\n\n\n\t\t\t\t     LOADING, please wait...");
		printf("\n\n\t\t\t\t         ");
		for (int j = (11 - i); j<11; j++) {
			printf("%2c", 254);
		}
		printf("\n\n\t\t\t\t\t\t  %d %c", (i * 10), 37);
		Sleep(rand() % 101 + 100);
	}
}

void lineAtas() {
	printf("\n%c", 201);
	for (int i = 0; i<9; i++) {
		printf("%c", 205);
	}
	printf("%c", 203);
	for (int i = 0; i<37; i++) {
		printf("%c", 205);
	}
	printf("%c", 203);
	for (int i = 0; i<13; i++) {
		printf("%c", 205);
	}
	printf("%c", 203);
	for (int i = 0; i<22; i++) {
		printf("%c", 205);
	}
	printf("%c", 203);
	for (int i = 0; i<17; i++) {
		printf("%c", 205);
	}
	printf("%c\n", 187);

}

void linePembatas() {
	printf("\n%c", 204);
	for (int i = 0; i<9; i++) {
		printf("%c", 205);
	}
	printf("%c", 206);
	for (int i = 0; i<37; i++) {
		printf("%c", 205);
	}
	printf("%c", 206);
	for (int i = 0; i<13; i++) {
		printf("%c", 205);
	}
	printf("%c", 206);
	for (int i = 0; i<22; i++) {
		printf("%c", 205);
	}
	printf("%c", 206);
	for (int i = 0; i<17; i++) {
		printf("%c", 205);
	}
	printf("%c\n", 185);
}

void lineBawah() {
	printf("%c", 200);
	for (int i = 0; i<9; i++) {
		printf("%c", 205);
	}
	printf("%c", 202);
	for (int i = 0; i<37; i++) {
		printf("%c", 205);
	}
	printf("%c", 202);
	for (int i = 0; i<13; i++) {
		printf("%c", 205);
	}
	printf("%c", 202);
	for (int i = 0; i<22; i++) {
		printf("%c", 205);
	}
	printf("%c", 202);
	for (int i = 0; i<17; i++) {
		printf("%c", 205);
	}
	printf("%c\n", 188);
}

void declareDefAcc(){
    pushAccByName("Cash", 0);
    pushAccByName("Debit", 0);
}

void declareDefTransactionType() {
	pushCtgByName("Food and Beverages", "FB", '-');
	pushCtgByName("Medicine", "ME", '-');
	pushCtgByName("Transport", "TR", '-');
	pushCtgByName("Home Needs", "HN", '-');
	pushCtgByName("Laundry", "LY", '-');
	pushCtgByName("Flights", "FL", '-');
	pushCtgByName("Bills", "BL", '-');
	pushCtgByName("Fruits", "FR", '-');
	pushCtgByName("Income", "IN", '+');
	pushCtgByName("Others", "OR", '-');
}

char *capitaliseText(char text[]) {
	text[0] = toupper(text[0]);
	for (int i = 0; i < (int)strlen(text); i++) if (isspace(text[i])) text[i + 1] = toupper(text[i + 1]);
	return text;
}

//This function is to create type id
char *generateTypeID(char text[]) {
	static char id[3];
	for (int i = 0, j = 0; i < (int) strlen(text); i++) {
		if (i == 0 || isspace(text[i - 1])) id[j++] = toupper(text[i]);
	}
	return id;
}

bool validateName(char name[]) {
	if (strlen(name) > 35) return false;
	else return true;
}//Validate Name

bool checkAccount(char name[]) {
	acc *curr = firstAcc;
	//Move the currently pointed node to the node which is being currently searched
	while (strcmpi(name, curr->name) != 0 && curr->next != NULL) curr = curr->next;
	if (strcmpi(name, curr->name) == 0)  return true;
	else return false;
}//Function to check the currently inputted account

bool validateAcc(char name[]) {
	if (strlen(name) > 20) return false;
	if (checkAccount(name) == true) return false; //If there is already an account with the same name, it will return false
	else return true;
}//Validate Account Name

bool checkCategory(char type[]) {
	ctg *curr = ctghead;
	//Move the currently pointed node to the node which is being currently searched
	while (strcmpi(type, curr->name) != 0 && curr->next != NULL) curr = curr->next;
	if (strcmpi(type, curr->name) == 0)  return true;
	else return false;
}//Function to check the currently inputted category

ctg *getCtgInfo(char type[]) {
	ctg *curr = ctghead;
	//Move the currently pointed node to the node which is being currently searched
	while (strcmpi(type, curr->name) != 0 && curr->next != NULL) curr = curr->next;
	if (strcmpi(type, curr->name) == 0)  return curr;
}

acc *getAccInfo(char accName[]) {
	acc *curr = firstAcc;
	while (strcmpi(accName, curr->name) != 0 && curr->next != NULL) curr = curr->next;
	if (strcmpi(accName, curr->name) == 0)  return curr;
}

bool validateTrsType(char type[]) {
	if (strlen(type) > 20) return false;
	else
		for (int i = 1, words = 1; i < (int)strlen(type); i++) {
			if (isspace(type[i - 1])) ++words;
			if (words > 3) return false;
		}
	if(checkCategory(type) == true) return false; //If there is already a category with the same name, it will return false
	else return true;
}//Validate Transaction Type

bool validateCtgID(char id[]) {
	if (strlen(id) > 3) return false;
	else {
		for (int i = 0; i < (int)strlen(id); i++) if (!isupper(id[i])) return false;
	}
	return true;
}

bool validateTrsID(char id[]) {
	if (strlen(id) != 10) return false;
	else {
		for (int i = 0; i < 2; i++) if (!isupper(id[i])) return false;
		for (int i = 2; i < (int)strlen(id); i++) if (!isdigit(id[i])) return false;
	}
	return true;
}

bool valPM(char plusminus) {
	if (plusminus != '+' && plusminus != '-') return false;
	else return true;
}

void syncAccTrsAdd(float amount, char accName[], char plusminus) {
	acc * curr = getAccInfo(accName);
	if (plusminus == '+') curr->currBal += amount;
	else curr->currBal -= amount;
}

void revertSyncAccTrs(float amount, char accName[], char pm) {
	acc * curr = getAccInfo(accName);
	if (pm == '+') curr->currBal -= amount;
	else curr->currBal += amount;
}

void syncAccwithTrsEdit(char accNameBefore[], const char accNameAfter[]) {
	for (transaction *curr = trshead; curr != NULL; curr = curr->next) {
		if (strcmpi(curr->account, accNameBefore) == 0) strcpy(curr->account, accNameAfter);
	}
}

void syncAccwithTrsDelete(char name[]) {
	for (transaction *curr = trshead; curr != NULL; curr = curr->next) {
		if (strcmpi(curr->account, name) == 0) {
			if (curr == trshead) popTrsHead();
			else if (curr == trstail) popTrsTail();
			else popTrsMid(&curr);
		}
	}
}

void syncCtgwithTrsEdit(char typeBefore[], const char typeAfter[], const char id[]) {
	for (transaction *curr = trshead; curr != NULL; curr = curr->next) {
		if (strcmpi(curr->trsType, typeBefore) == 0) {
			strcpy(curr->trsType, typeAfter);
			curr->ID[0] = id[0];
			curr->ID[1] = id[1];
		}
	}
}

void syncCtgwithTrsDelete(char type[]) {
	ctg *currCtg = getCtgInfo(type);
	for (transaction *curr = trshead; curr != NULL; curr = curr->next) {
		if (strcmpi(curr->trsType, type) == 0) {
            revertSyncAccTrs(curr->amount, curr->account, currCtg->baseEff);
			if (curr == trshead) popTrsHead();
			else if (curr == trstail) popTrsTail();
			else popTrsMid(&curr);
		}
	}
}

void calUsePercentage(char type[], char eff) {
	if (eff == '-') {
		float ttlAll = 0, ttlCurr = 0;
		for (transaction *curr = trshead; curr != NULL; curr = curr->next) {
			ctg *currCtg = getCtgInfo(curr->trsType);
			if (currCtg->baseEff == '-') {
				ttlAll += curr->amount;
				if (strcmpi(type, curr->trsType) == 0) ttlCurr += curr->amount;
			}
		}
		float percentage = (ttlCurr / ttlAll) * 100;
		if(percentage != 0){
			printf("%-20s : %.2f%c\n", type, percentage, 37);
			printf("Total of Usage on this category: Rp%.2f\n\n", ttlCurr);
		}
	}
	else if(eff == '+'){
		float ttlAll = 0, ttlCurr = 0;
		for (transaction *curr = trshead; curr != NULL; curr = curr->next) {
			ctg *currCtg = getCtgInfo(curr->trsType);
			if (currCtg->baseEff == '+') {
				ttlAll += curr->amount;
				if (strcmpi(type, curr->trsType) == 0) ttlCurr += curr->amount;
			}
		}
		float percentage = (ttlCurr / ttlAll) * 100;
		if(percentage != 0){
			printf("%-20s : %.2f%c\n", type, percentage, 37);
			printf("Total of Surplus on this category: Rp%.2f\n\n", ttlCurr);
		}
	}
}

void printPercentageofUse() {
	if (trshead) {
		printf("Summary of Money Use\n\n");
		for (ctg *curr = ctghead; curr != NULL; curr = curr->next) { 
			calUsePercentage(curr->name, curr->baseEff);
		}
	}
}

void viewCurrSaldo() {
	printf("Current Accounts' Saldo\n");
	for (int i = 0; i < 30; i++) printf("-");
	printf("\n");
	if (firstAcc)
		for (acc *curr = firstAcc; curr != NULL; curr = curr->next)
			printf("%-20s Rp %-10.2f\n", curr->name, curr->currBal);
	else printf("No Account Yet...");
}

void printCurrData(struct transaction **curr) {
	while ((*curr) != NULL /*|| ttlLines <= maxttlLines*/) {
		printf("%c  %02d/%02d  %c %-35s %c %-11s %c %-20s %c Rp.%-12.2f %c\n", 186, (*curr)->tgl.dd, (*curr)->tgl.mm, 186,
			(*curr)->name, 186, (*curr)->ID, 186, (*curr)->trsType, 186, (*curr)->amount, 186);
		(*curr) = (*curr)->next;
		//ttlLines += 1;
	}
}

int printAvalableCategories() {
	printf("\nAvailable Categories:\n");
	int i = 0;
	for (ctg * curr = ctghead; curr != NULL; curr = curr->next)
		printf("%02d. %s\n", ++i, curr->name);
	printf("\n");
	return i;
}

int printAvalableAccounts() {
	printf("\nAvailable Accounts:\n");
	int i = 0;
	for (acc * curr = firstAcc; curr != NULL; curr = curr->next)
		printf("%02d. %s\t Rp.%.2f\n", ++i, curr->name, curr->currBal);
	printf("\n");
	return i;
}

void printTransactionData(struct transaction *trs,/* int pagenum,*/ char type[]) {
	system("cls");
	printAvalableAccounts();
	printf("\n\n");
	if (trshead) {
		transaction *curr = trshead;
		printf("Transaction for Year %d\n", curr->tgl.yy);
		lineAtas();
		printf("%c %-7s %c %-35s %c %-11s %c %-20s %c %-15s %c", 186, "Date", 186, "Name", 186, "ID", 186, "Category Type", 186, "Amount", 186);
		linePembatas();
		//int ttlLines = 0;
		while (curr != NULL /*|| ttlLines <= maxttlLines*/)
			printCurrData(&curr); //ttlLines += 1;
								  //if (curr != NULL) viewAllTransaction(curr, ++pagenum);
		lineBawah();
		printPercentageofUse();
	}
	else printf("No Transaction...yet\n\n");
}

void addTrs() {//Add new Transaction
	system("cls");
	//These variables will be used to record temporary data
	char name[36], ID[11], type[20], accName[20], idtype[3];
	float amount;
	struct date tgl;
	int totdays;
	int kabisat = 0;
	printf("\t\t\t--- Add new transaction ---\n\n");
	//Insert the name of the transaction or the event here
	do {
		printf("Enter Your Transaction/Item/Event Name [1..35 characters|0 to cancel]: ");
		scanf("%[^\n]", name); getchar();
		if (strcmp(name, "0") == 0) return;
	} while (validateName(name) == false);
	printAvalableCategories(); //Prints all available categories at the moment
	//Categorise the type of the transaction here
	do {
		printf("Enter Your Transaction Category: ");
		scanf("%[^\n]", type); getchar();
	} while (checkCategory(type) == false);
	ctg *currCtg = getCtgInfo(type);
	strcpy(idtype, currCtg->ID); // Get the current type id
	printAvalableAccounts();
	do {
		printf("Enter Which Account Type will this transaction be inputted\t\t\t: ");
		scanf("%[^\n]", accName); getchar();
	} while (checkAccount(accName) == false);
	//Record date when this transaction happened
	//Separate the struct of tgl with the transaction to make it easier to record
	do {
		printf("Enter Your Transaction Year [2000-2100]\t: ");
		scanf("%d", &tgl.yy); getchar();
		if (tgl.yy % 4 == 0 && tgl.yy % 100 != 0) kabisat = 1;
		//to determine the total date of february
	} while (tgl.yy < 2000 || tgl.yy > 2100);
	//Input the month of the transaction
	do {
		totdays = 0;
		printf("Enter Your Transaction Month [in digit]\t: ");
		scanf("%d", &tgl.mm); getchar();
		//Determining the maximum number of days for that month
		if (tgl.mm == 2){
			if (kabisat == 1) totdays = 29;
			else totdays = 28;
		}
		else if ((tgl.mm % 2 == 0 && tgl.mm < 8) || (tgl.mm % 2 == 1 && tgl.mm > 8)) {
			totdays = 30;
		}
		else {
			totdays = 31;
		}
	}while (tgl.mm<1 || tgl.mm>12);
	do {
		printf("Enter Your Transaction Date [in digit]\t: ");
		scanf("%d", &tgl.dd); getchar();
	} while (tgl.dd < 1 || tgl.dd > totdays);
	do {
		printf("Enter Your Transaction Amount\t\t: Rp.");
		scanf("%f", &amount); getchar();
	} while (amount<0);
	//Get the current time
	time_t t = time(NULL);
	tm tm = *localtime(&t);
	int z = rand() % (99 - 10 + 1) + 10;
	int dd = tm.tm_mday;
	int mm = tm.tm_mon + 1;
	int yy = tm.tm_year - 100;
	sprintf(ID, "%2s%d%.02d%.02d%.02d", idtype, z, dd, mm, yy);
	strcpy(name, capitaliseText(name));
	strcpy(type, capitaliseText(type));
	printf("\n\n\n--- Add Transaction/Event Success! ---");
	printf("\nName\t: %s", name);
	printf("\nAmount\t: Rp.%.2f", amount);
	printf("\nType\t: %s", type);
	printf("\nId\t: %s", ID);
	pushBackTrs(name, tgl, type, amount, ID, capitaliseText(accName));
	syncAccTrsAdd(amount, accName, currCtg->baseEff);
	printf("\n\n\n");
	system("Pause");
}

void addTrsCtg() {
	system("cls");
	char type[20], id[3], eff;
	printf("\t\t\t--- Add Category ---\n\n");
	printAvalableCategories();
	do {
		printf("Enter Your Transaction/Item/Event Type Name [1..20 characters || 3 words maximum|0 to cancel]: ");
		scanf("%[^\n]", type); getchar();
		if (strcmp(type, "0") == 0) return;
	} while (validateTrsType(type) == false);
	do {
		printf("Enter ID of Current Transaction/Item/Event Type [2 Uppercase Letter Only]\t : ");
		scanf("%[^\n]", id); getchar();
	} while (validateCtgID(id) == false);
	do {
		eff = '\0';
		printf("Is this transaction a plus or minus for the amount of the current Account?[+/-] : ");
		scanf("%c", &eff); getchar();
	} while (eff != '+' && eff != '-');
	strcpy(type, capitaliseText(type));
	ctg *temp = pushCtgByName(type, id, eff);
	printf("\n\n\n--- Add Category Success!---\n");
	printf("\nCategory Type\t: %s", temp->name);
	printf("\nCategory ID\t: %s\n\n\n", temp->ID);
	system("pause");
}

void addAccType() {
	system("cls");
	char name[20];
	printf("\t\t\t--- Add Account---\t\t\t");
	printAvalableAccounts();
	do {
		printf("Enter your new account name [1..20 characters|0 to cancel]: ");
		scanf("%[^\n]", name); getchar();
		if (strcmp(name, "0") == 0) return;
	} while (validateAcc(name) == false);
	acc *temp = pushAccByName(name, 0);
	printf("\n\n\n---Add Account Type Success!---\n");
	printf("\nAccount Name\t\t: %s", temp->name);
	printf("\nAccount Current Balance\t: %.2f\n\n\n", temp->currBal);
	system("pause");
}

void viewSpecificTrsbyCtg(char name[], char eff) {
	int found = 0;
	printf("\n\n%s\n", name);
	for (transaction *curr = trshead; curr != NULL; curr = curr->next) {
		if (strcmpi(name, curr->trsType) == 0) {
			if (found == 0) {
				printf("Transaction for Year %d\n", curr->tgl.yy);
				lineAtas();
				printf("%c %-7s %c %-35s %c %-11s %c %-20s %c %-15s %c", 186, "Date", 186, "Name", 186, "ID", 186, "Category Type", 186, "Amount", 186);
				linePembatas();
				found = 1;
			}
			printf("%c  %02d/%02d  %c %-35s %c %-11s %c %-20s %c Rp. %-11.2f %c\n", 186, curr->tgl.dd, curr->tgl.mm, 186,
				curr->name, 186, curr->ID, 186, curr->trsType, 186, curr->amount, 186);
		}
	}
	if (found == 1) {
		lineBawah();
		calUsePercentage(name, eff);
	}
	else printf("No transaction data in this category\n");
}

void viewCtg() {
	system("cls");
	printf("\n\n--- View by Category ---\n\n");
	printAvalableAccounts();
	if (trshead) {
		for (ctg *temp = ctghead; temp != NULL; temp = temp->next)
			viewSpecificTrsbyCtg(temp->name, temp->baseEff);
	}
	else printf("No Transaction yet\n");
	system("pause");
}

void viewSpecificCtg() {
	system("cls");
	if (trshead) {
		char type[20];
		printf("\n--- View by Specific Category ---\n");
		printAvalableCategories();
		do {
			printf("\nChoose Which Category Do You Would Like to View: ");
			scanf("%[^\n]", type); getchar();
		} while (checkCategory(type) == false);
		ctg *currCtg = getCtgInfo(type);
		viewSpecificTrsbyCtg(type, currCtg->baseEff);
	}
	else printf("\n\nNo Transaction yet\n\n");
	system("pause");
}

void editAcc(){
	system("cls");
	char name[20];
	printAvalableAccounts();
	if (firstAcc) {
		printf("\t\t\t--- Edit Account ---\n\n");
		do {
			printf("Enter account's name to edit: ");
			scanf("%[^\n]", name); getchar();
		} while (strlen(name)>20);
		printf("\n\n");
		acc *temp = firstAcc;
		while (temp) {
			if(strcmpi(name, temp->name)==0){
				do {
					printf("Enter your new account's name to edit: ");
					scanf("%[^\n]", name); getchar();
				} while (strlen(name) > 20);
				syncAccwithTrsEdit(temp->name, name);
				strcpy(temp->name, name);
				printf("\n\n\n---Edit Account Type Success!---\n");
				printf("\nAccount Name\t\t: %s", temp->name);
				printf("\nAccount Current Balance\t: %.2f\n\n\n", temp->currBal);
				break;
			}
			else temp = temp->next;
			if(temp == NULL) printf("\n\nAccount was not found!");
		}
	}
	else printf("No account yet");
	printf("\n\n");
	system("pause");
}

void delAcc(){
	system("cls");
	int ttl = printAvalableAccounts();
	printf("\t\t\t--- Delete Account ---\n\n");
	printf("[WARNING!!! All transactions paid with deleted account will be deleted too.]\n\n");
	int idx;
	if (firstAcc) {
		do {
			printf("Enter your account's index to delete [1..%d|0 to cancel]: ", ttl);
			scanf("%d", &idx); getchar();
			if (idx == 0) return;
		} while (idx < 1 || idx > ttl);
		acc *temp = firstAcc;
		for (int i = 1; i < idx; i++) temp = temp->next;
		syncAccwithTrsDelete(temp->name);
		temp = popAcc(idx, ttl);
		printf("\n\nSuccess deleting %s account!", temp->name);
	}
	else printf("No account yet");
	printf("\n\n");
	system("pause");
}

void editTrans() {
	system("cls");
	printf("\t\t\t--- Edit Transaction ---\n\n");
	if (trshead) {
		printTransactionData(trshead, '\0');
		//These variables will be used to record temporary data
		char name[36], ID[11], type[20], accName[20], plusminus;
		float amount;
		struct date tgl;
		int totdays, benar;
		int kabisat = 0;
		do {
			printf("Enter Transaction ID [0 to cancel]: ");
			scanf("%[^\n]", ID); getchar();
			if (strcmp(ID, "0") == 0) return;
		} while (validateTrsID(ID) == false);
		transaction *temp = trshead;
		while (temp) {
			if (strcmp(ID, temp->ID) == 0) {
				ctg *currCtg = getCtgInfo(temp->trsType);
				revertSyncAccTrs(temp->amount, temp->account, currCtg->baseEff);
				int chc;
				printf("Which part do you want to change? [0..6]\n");
				printf("0. Cancel\n");
				printf("1. Title\n");
				printf("2. Category\n");
				printf("3. Account\n");
				printf("4. Date\n");
				printf("5. Amount\n");
				printf("6. All\n");
				do {
					printf(">> "); scanf("%d", &chc); getchar();
				} while (chc < 0|| chc > 6);
				//Insert the name of the transaction or the event here
				if (chc == 1 || chc == 6) {//Title
					do {
						printf("Enter Your Transaction/Item/Event Name [1..35 characters]: ");
						scanf("%[^\n]", name); getchar();
					} while (validateName(name) == false);
					strcpy(name, capitaliseText(name));
					strcpy(temp->name, name);
				}
				if (chc == 2 || chc == 6) {//Category
					//Prints all available categories at the moment
					printAvalableCategories();
					//Categorise the type of the transaction here
					do {
						printf("Enter Your Transaction Category: ");
						scanf("%[^\n]", type); getchar();
					} while (checkCategory(type) == false);
					currCtg = getCtgInfo(type);
					capitaliseText(type);
					strcpy(temp->trsType, type);
					temp->ID[0] = currCtg->ID[0];
					temp->ID[1] = currCtg->ID[1];
				}
				if (chc == 3 || chc == 6) {//Account
					printAvalableAccounts();
					do {
						printf("Enter Which Account Type will this transaction be inputted: ");
						scanf("%[^\n]", accName); getchar();
					} while (checkAccount(accName) == false);
					strcpy(temp->account, accName);
				}
				//Record date when this transaction happened
				//Separate the struct of tgl with the transaction to make it easier to record
				if (chc == 4 || chc == 6) {
					do {
						printf("Enter Your Transaction Year\t\t: ");
						scanf("%d", &tgl.yy); getchar();
						if (tgl.yy % 4 == 0 && tgl.yy % 100 != 0) kabisat = 1;
						//to determine the total date of february
					} while (tgl.yy<1);
					//Input the month of the transaction
					do {
						totdays = 0;
						printf("Enter Your Transaction Month [in digit]\t: ");
						scanf("%d", &tgl.mm); getchar();
						//Determining the maximum number of days for that month
						if (tgl.mm == 2) {
							if (kabisat == 1) totdays = 29;
							else totdays = 28;
						}
						else if ((tgl.mm % 2 == 0 && tgl.mm < 8) || (tgl.mm % 2 == 1 && tgl.mm > 8)) {
							totdays = 30;
						}
						else {
							totdays = 31;
						}
					} while (tgl.mm<1 || tgl.mm>12);
					do {
						printf("Enter Your Transaction Date [in digit]\t: ");
						scanf("%d", &tgl.dd); getchar();
					} while (tgl.dd < 1 || tgl.dd > totdays);
					temp->tgl = tgl;
				}
				if (chc == 5 || chc == 6) {
					do {
						printf("Enter Your Transaction Amount\t\t: Rp.");
						scanf("%f", &amount); getchar();
					} while (amount<0);
					temp->amount = amount;
				}
				syncAccTrsAdd(temp->amount, temp->account, currCtg->baseEff);
				printf("Data has been updated!");
				printf("\nName\t: %s", temp->name);
				printf("\nAmount\t: Rp.%.2f", temp->amount);
				printf("\nType\t: %s", temp->trsType);
				printf("\nId\t: %s", temp->ID);
				break;
			}
			else temp = temp->next;
		}
		if (temp == NULL) printf("\n\nID is not found\n");
	}
	else printf("No transaction record yet\n");
	printf("\n");
	system("pause");
	//getchar();
}

void delTrans() {
	system("cls");
	printTransactionData(trshead, '\0');
	//These variables will be used to record temporary data
	char ID[11];
	printf("\n\n\t\t\t--- Delete Transaction ---\n\n");
	if (trshead) {
		do {
			printf("Enter Transaction ID[0 to cancel]: ");
			scanf("%[^\n]", ID); getchar();
			if (strcmp(ID, "0") == 0) return;
		} while (validateTrsID(ID) == false);
		transaction *curr = trshead;
		while (curr != NULL && strcmp(curr->ID, ID) != 0) curr = curr->next;
		if (curr != NULL && strcmp(curr->ID, ID) == 0) {
			ctg *currCtg = getCtgInfo(curr->trsType);
			revertSyncAccTrs(curr->amount, curr->account, currCtg->baseEff);
			if (curr == trshead) popTrsHead();
			else if (curr == trstail) popTrsTail;
			else popTrsMid(&curr);
			printf("\n --- Success deleting choosen transaction! ---\n\n\n");
		}
		else printf("\n\nTransaction was not found\n\n\n");
	}
	else printf("\n\nNo transaction record yet\n\n\n");
	system("pause");
}

void editCtg(){
	system("cls");
	char type[20], id[3], eff;
	printAvalableCategories();
	if (ctghead) {
		printf("\t\t\t--- Edit Category ---\n\n");
		do {
			printf("Enter Your Transaction/Item/Event Type Name to edit [0 to cancel]: ");
			scanf("%[^\n]", type); getchar();
			if (strcmp(type, "0") == 0) return;
		} while (strlen(type)>20);
		printf("\n\n");
		ctg *temp = ctghead;
		while (temp) {
			if(strcmpi(type, temp->name)==0){
				do {
					printf("Enter Your New Transaction/Item/Event Type Name [1..20 characters || 3 words maximum]: ");
					scanf("%[^\n]", type); getchar();
				} while (validateTrsType(type) == false);
				do {
					printf("Enter New ID of Current Transaction/Item/Event Type [2 Uppercase Letter Only]\t     : ");
					scanf("%[^\n]", id); getchar();
				} while (validateCtgID(id) == false);
				do {
					eff = '\0';
					printf("Is this transaction a plus or minus for the amount of the current Account?[+/-] : ");
					scanf("%c", &eff); getchar();
				} while (eff != '+' && eff != '-');
				syncCtgwithTrsEdit(temp->name, type, id);
				strcpy(type, capitaliseText(type));
				strcpy(temp->name, type);
				strcpy(temp->ID, id);
				temp->baseEff = eff;
				printf("\n\n\n--- Edit Category Success!---\n");
				printf("\nCategory Type\t: %s", temp->name);
				printf("\nCategory ID\t: %s\n\n\n", temp->ID);
				break;
			}
			else temp = temp->next;
			if(temp == NULL) printf("\nType is not found!\n");
		}
	}
	else printf("No category yet\n");
	system("pause");
}

void delCtg(){
	system("cls");
	if (ctghead) {
		int idx;
		int ttl = printAvalableCategories();
		printf("\t\t\t--- Delete Category ---\n\n");
		printf("[WARNING!!! All transactions with deleted category will be deleted too.]\n\n");
		do {
			printf("Enter your Category index to delete [1..%d|0 to cancel]: ", ttl);
			scanf("%d", &idx); getchar();
			if (idx == 0) return;
		} while (idx < 1 || idx > ttl);
		ctg *temp = ctghead;
		for (int i = 1; i < idx; i++) temp = temp->next;
		printf("\n\n%s Category has been deleted", temp->name);
		syncCtgwithTrsDelete(temp->name);
		temp = popCtg(idx, ttl);
	}
	else printf("No category yet");
	printf("\n\n");
	system("pause");
}

void menuAdd() {
	int temp;
	do {
		temp = 0;
		int baris = 0;
		int flag = 0;
		char listAdd[4][100] = { ("            Add New Account"), ("      Add New Transaction's Type"),
			("       Add New Transaction/Event"), ("         Go Back to Main Menu") };
		char move = '\0';
		char ch = '\0';
		listAdd[0][3] = '>';
		listAdd[0][34] = '<';

		for (;;) {
			system("cls");
			titleAdd();
			printf("\n\n\n\n\n");
			for (int k = 0; k<4; k++) {
				printf("\n\t\t\t\t");
				for (int j = 0; j<35; j++) printf("%c", listAdd[k][j]);
				printf("\n");
			}

			printf("\n\n\n\n\n\t\t\t\t      [Press Enter to select...]");
			ch = getch();
			if (ch == DOWN) {
				if (baris<3) baris++;
				listAdd[baris][3] = '>';
				listAdd[baris - 1][3] = '\0';
				listAdd[baris][34] = '<';
				listAdd[baris - 1][34] = '\0';
			}
			else if (ch == UP) {
				if (baris>0) baris--;
				listAdd[baris][3] = '>';
				listAdd[baris + 1][3] = '\0';
				listAdd[baris][34] = '<';
				listAdd[baris + 1][34] = '\0';
			}

			if (ch == ENTER) {
				flag = baris + 1;
				break;
			}
		}
		switch (flag) {
		case 1:
			addAccType();
			break;
		case 2:
			addTrsCtg();
			break;
		case 3:
			addTrs();
			break;
		case 4:
			temp = 1;
			return;
			break;
		}
	} while (temp != 1);
}

void menuView() {
	int temp;
	do {
		temp = 0;
		int baris = 0;
		int flag = 0;
		char listView[5][100] = { ("          View All Transactions"), ("      View Transactions by Category"), ("  View Specific Category of Transactions"),
			 ("   View Currently Available Categories"), ("           Go Back to Main Menu") };
		char move = '\0';
		char ch = '\0';
		listView[0][0] = '>';
		listView[0][41] = '<';

		for (;;) {
			system("cls");
			titleView();
			printf("\n\n\n\n\n");
			for (int k = 0; k<5; k++) {
				printf("\n\t\t\t        ");
				for (int j = 0; j<42; j++) {
					printf("%c", listView[k][j]);
				}
				printf("\n");
			}

			printf("\n\n\n\n\n\t\t\t\t        [Press Enter to select...]");
			ch = getch();
			if (ch == DOWN) {
				if (baris<4) baris++;
				listView[baris][0] = '>';
				listView[baris - 1][0] = '\0';
				listView[baris][41] = '<';
				listView[baris - 1][41] = '\0';
			}
			else if (ch == UP) {
				if (baris>0) baris--;
				listView[baris][0] = '>';
				listView[baris + 1][0] = '\0';
				listView[baris][41] = '<';
				listView[baris + 1][41] = '\0';
			}

			if (ch == ENTER) {
				flag = baris + 1;
				break;
			}
		}
		switch (flag) {
		case 1:
			printTransactionData(trshead, '\0');
			system("pause");
			break;
		case 2:
			viewCtg();
			break;
		case 3:
			viewSpecificCtg();
			break;
		case 4:
			system("cls");
			printAvalableCategories();
			printf("\n");
			system("pause");
			break;
		case 5:
			temp = 1;
			return;
			break;
		}
	} while (temp != 1);
}

void addNotes() {
	system("cls");
	char content[10000] ="\0";
	printf("--- Add New Notes ---\n\n\n");
	do {
		printf("Enter the content of the notes here: ");
		scanf("%[^\n]", content); getchar();
	} while (strcmp(content, "\0") == 0);
	pushBackNote(content);
	printf("\n\nSuccess adding new note!\n\n\n");
	system("pause");
}

int printNotes() {
	if (notehead) {
		printf("Current Notes: \n");
		int i = 0;
		for (note *curr = notehead; curr != NULL; curr = curr->next) {
			printf("\n\t%-02d. %s\n", ++i, curr->content);
		}
		return i;
	}
	else printf("No Notes yet");
}

void editNotes() {
	system("cls");
	printf("--- Edit Notes ---\n\n\n");
	if (notehead) {
		int ttl = printNotes();
		int idx;
		char inp[7];
		printf("\n\n");
		do {
			printf("Which note do you wish to edit? [1..%d|0 to cancel]: ", ttl);
			scanf("%d", &idx); getchar();
			if (idx == 0) return;
		} while (idx < 1 || idx > ttl);
		do {
			strcmp(inp, "\0");
			printf("Do you want to edit or delete? [edit|delete]\t  : ");
			scanf("%[^\n]", inp); getchar();
		} while (strcmp(inp, "edit") != 0 && strcmp(inp, "delete") != 0);
		if (strcmp(inp, "delete") == 0) popNote(idx, ttl);
		else {
			char temp[10000];
			printf("\nEnter the content of the notes here\t\t  : ");
			scanf("%[^\n]", temp);
			note *curr = notehead;
			for (int i = 1; i < idx; i++)
				if(curr != NULL) curr = curr->next;
			strcpy(curr->content, temp);
		}
	}
	else printf("No note yet");
	printf("\n\n\n");
	system("pause");
}

void menuNotes(){
	int temp;
	do{
		temp = 0;
		int baris=0;
		int flag = 0;
		char listNotes[3][100]={("     Add New Notes"), ("     Change Details"),
		("  Go Back to Main Menu")};
		char move='\0';
		char ch='\0';
		listNotes[0][0]='>';
		listNotes[0][23]='<';

		for(;;){
			system("cls");
			titleNotes();
			printf("\n\n\n\n\n");
			for(int k=0; k<3; k++){
				printf("\n\t\t\t\t\t");
				for(int j=0; j<24; j++) printf("%c", listNotes[k][j]);
				printf("\n");
			}

			printf("\n\n\n\n\n\t\t\t\t       [Press Enter to select...]");
			ch=getch();
			if(ch==DOWN){
				if(baris<2){
					baris++;
				}
				listNotes[baris][0]='>';
				listNotes[baris-1][0]='\0';
				listNotes[baris][23]='<';
				listNotes[baris-1][23]='\0';
			}
			else if (ch==UP){
				if(baris>0){
					baris--;
				}
				listNotes[baris][0]='>';
				listNotes[baris+1][0]='\0';
				listNotes[baris][23]='<';
				listNotes[baris+1][23]='\0';
			}

			if(ch==ENTER){
				flag = baris + 1;
				break;
			}
		}
		switch(flag){
			case 1:
				addNotes();
				break;
			case 2:
				editNotes();
				break;
			case 3:
				temp = 1;
		}
	}while(temp!=1);
}

void help() {
	system("cls");
	printf("\n\n");

	printf("\t\t\t\t  _  _     _      \n");
	printf("\t\t\t\t | || |___| |_ __ \n");
	printf("\t\t\t\t | __ / -_) | '_ \\\n");
	printf("\t\t\t\t |_||_\\___|_| .__/\n");
	printf("\t\t\t\t            |_|   \n\n\n\n");

	printf("\tThis is an app to help manage your account.\n");
	printf("\tFirst, you input your account's saldo, either into \"Bank\" or \"Cash\" \n\t\twhich are the standard account from this app.\n");
	printf("\tNext, you can input new account type (NB: This part is not necessary).\n");
	printf("\tThen, you can manage your expends and income by adding into \"Add New Transaction/Event\" Menu.\n");
	printf("\tThere has already \"Income\" Category as a default Category in this app.\n");
	printf("\t\t(It will automatically add up your account's' saldo)\n");
	printf("\t\t(For other categories, account's saldo will be reduced automatically)\n");
	printf("\t\t(You can also add new category and set it as income or expend)\n");
	printf("\t\t(If there's a mistake when you input, you can choose \"Edit\" Menu for correction)\n\n");

	printf("\tADD MENU:\n");
	printf("\t1. Add New Cash Account\n");
	printf("\t\t-> to make new account\n");
	printf("\t\t   \"Debit\" and \"Cash\" are the standard accounts\n");
	printf("\t2. Add New Trancation's Type\n");
	printf("\t\t-> to add new category for your transaction's detail\n");
	printf("\t3. Add New Transaction/Event\n");
	printf("\t\t-> to input your transaction's detail\n\n\n");

	printf("\tVIEW MENU:\n");
	printf("\t1. View All Transactions\n");
	printf("\t\t-> view all of your transactions' details\n");
	printf("\t2. View Transactions by Category\n");
	printf("\t\t-> view all of your transactions' details by category\n");
	printf("\t3. View currently available categories\n");
	printf("\t\t-> view all of your available categories\n\n\n");

	printf("\tEDIT MENU:\n");
	printf("\t[this menu are divided into three submenus: \n");
	printf("\t\t1. Edit Transaction\n");
	printf("\t\t2. Edit Account\n");
	printf("\t\t3. Edit Category\n");
	printf("\twhich contents are \"Change Details\" and \"Delete\"]\n");
	printf("\t1. Change Details\n");
	printf("\t\t-> change the details of your choosen transaction/category/account\n");
	printf("\t2. Delete\n");
	printf("\t\t-> delete your choosen transaction/category/account\n\n\n");

	printf("\tNOTES MENU\n");
	printf("\t1. Add New Notes\n");
	printf("\t\t-> to make new note (this note act as a reminder)\n");
	printf("\t2. Change Details\n");
	printf("\t\t-> change the details of your choosen notes or\n");
	printf("\t\t   delete your choosen notes\n\n\n\n");

	system("pause");
}

void credits() {
	system("cls");
	printf("\n\n");

	printf("\t\t\t\t   ___            _ _ _      \n");
	printf("\t\t\t\t  / __|_ _ ___ __| (_) |_ ___\n");
	printf("\t\t\t\t | (__| '_/ -_) _` | |  _(_-<\n");
	printf("\t\t\t\t  \\___|_| \\___\\__,_|_|\\__/__/\n\n\n\n");
	printf("\n\n\t\t\t\t This app is brought to you by:\n");
	printf("\n\t\t\t\t   Ananda Kevin - 2101724320\n");
	printf("\n\t\t\tHe does most of the logic stuff of the app\n");
	printf("\n\t\t\t\t\t\t&\n");
	printf("\n\t\t\t\tJessica Clarisa H. - 2101724541\n");
	printf("\n\t\t\tShe does most of the interface of the app\n");
	printf("\n\t\t\tHere are references of the files we used here\n\n");
	printf("\n\t\t\tMusic: https://www.youtube.com/watch?v=WZkOWliNa9o \n\n\n\n\n\n");
	system("pause");
}

void menuEditTrans(){
	int temp;
	do{
		temp = 0;
		int baris=0;
		int flag = 0;
		char listEditTrans[3][100]={("     Change Details"), ("         Delete"),
		("  Go Back to Edit Menu")};
		char move='\0';
		char ch='\0';
		listEditTrans[0][0]='>';
		listEditTrans[0][23]='<';

		for(;;){
			system("cls");
			titleEditTrans();
			printf("\n\n\n\n\n");
			for(int k=0; k<3; k++){
				printf("\n\t\t\t\t         ");
				for(int j=0; j<24; j++) printf("%c", listEditTrans[k][j]);
				printf("\n");
			}

			printf("\n\n\n\n\n\t\t\t\t        [Press Enter to select...]");
			ch=getch();
			if(ch==DOWN){
				if(baris<2) baris++;
				listEditTrans[baris][0]='>';
				listEditTrans[baris-1][0]='\0';
				listEditTrans[baris][23]='<';
				listEditTrans[baris-1][23]='\0';
			}
			else if (ch==UP){
				if(baris>0) baris--;
				listEditTrans[baris][0]='>';
				listEditTrans[baris+1][0]='\0';
				listEditTrans[baris][23]='<';
				listEditTrans[baris+1][23]='\0';
			}

			if(ch==ENTER){
				flag = baris + 1;
				break;
			}
		}
		switch(flag){
			case 1:
				editTrans();
				break;
			case 2:
				delTrans();
				break;
			case 3:
				temp = 1;
		}
	}while(temp!=1);
}

void menuEditCtg(){
	int temp;
	do{
		temp = 0;
		int baris=0;
		int flag = 0;
		char listEditCtg[3][100]={("     Change Details"), ("         Delete"),
		("  Go Back to Main Menu")};
		char move='\0';
		char ch='\0';
		listEditCtg[0][0]='>';
		listEditCtg[0][23]='<';

		for(;;){
			system("cls");
			titleEditCtg();
			printf("\n\n\n\n\n");
			for(int k=0; k<3; k++){
				printf("\n\t\t\t\t     ");
				for(int j=0; j<24; j++) printf("%c", listEditCtg[k][j]);
				printf("\n");
			}

			printf("\n\n\n\n\n\t\t\t\t    [Press Enter to select...]");
			ch=getch();
			if(ch==DOWN){
				if(baris<2) baris++;
				listEditCtg[baris][0]='>';
				listEditCtg[baris-1][0]='\0';
				listEditCtg[baris][23]='<';
				listEditCtg[baris-1][23]='\0';
			}
			else if (ch==UP){
				if(baris>0) baris--;
				listEditCtg[baris][0]='>';
				listEditCtg[baris+1][0]='\0';
				listEditCtg[baris][23]='<';
				listEditCtg[baris+1][23]='\0';
			}

			if(ch==ENTER){
				flag = baris + 1;
				break;
			}
		}
		switch(flag){
			case 1:
				editCtg();
				break;
			case 2:
				delCtg();
				break;
			case 3:
				temp = 1;
		}
	}while(temp!=1);
}

void menuEditAcc(){
	int temp;
	do{
		temp = 0;
		int baris=0;
		int flag = 0;
		char listEditAcc[3][100]={("     Change Details"), ("         Delete"),
		("  Go Back to Edit Menu")};
		char move='\0';
		char ch='\0';
		listEditAcc[0][0]='>';
		listEditAcc[0][23]='<';

		for(;;){
			system("cls");
			titleEditAcc();
			printf("\n\n\n\n\n");
			for(int k=0; k<3; k++){
				printf("\n\t\t\t\t  ");
				for(int j=0; j<24; j++){
					printf("%c", listEditAcc[k][j]);
				}
				printf("\n");
			}

			printf("\n\n\n\n\n\t\t\t\t  [Press Enter to select...]");
			ch=getch();
			if(ch==DOWN){
				if(baris<2) baris++;
				listEditAcc[baris][0]='>';
				listEditAcc[baris-1][0]='\0';
				listEditAcc[baris][23]='<';
				listEditAcc[baris-1][23]='\0';
			}
			else if (ch==UP){
				if(baris>0) baris--;
				listEditAcc[baris][0]='>';
				listEditAcc[baris+1][0]='\0';
				listEditAcc[baris][23]='<';
				listEditAcc[baris+1][23]='\0';
			}

			if(ch==ENTER){
				flag = baris + 1;
				break;
			}
		}
		switch(flag){
			case 1:
				editAcc();
				break;
			case 2:
				delAcc();
				break;
			case 3:
				temp = 1;
		}
	}while(temp!=1);
}

void menuEdit(){
	int temp;
	do{
		temp = 0;
		int baris=0;
		int flag = 0;
		char listEdit[4][100]={("    Edit Transaction"), ("      Edit Account"), ("     Edit Category"),
		("  Go Back to Main Menu")};
		char move='\0';
		char ch='\0';
		listEdit[0][0]='>';
		listEdit[0][23]='<';

		for(;;){
			system("cls");
			titleEdit();
			printf("\n\n\n\n\n");
			for(int k=0; k<4; k++){
				printf("\n\t\t\t\t       ");
				for(int j=0; j<35; j++){
					printf("%c", listEdit[k][j]);
				}
				printf("\n");
			}

			printf("\n\n\n\n\n\t\t\t\t       [Press Enter to select...]");
			ch=getch();
			if(ch==DOWN){
				if(baris<3) baris++;
				listEdit[baris][0]='>';
				listEdit[baris-1][0]='\0';
				listEdit[baris][23]='<';
				listEdit[baris-1][23]='\0';
			}
			else if (ch==UP){
				if(baris>0) baris--;
				listEdit[baris][0]='>';
				listEdit[baris+1][0]='\0';
				listEdit[baris][23]='<';
				listEdit[baris+1][23]='\0';
			}

			if(ch==ENTER){
				flag = baris + 1;
				break;
			}
		}
		switch(flag){
			case 1:
				menuEditTrans();
				break;
			case 2:
				menuEditAcc();
				break;
			case 3:
				menuEditCtg();
				break;
			case 4:
				temp = 1;
		}
	}while(temp!=1);
}

void menu(){
	int temp;
	do{
		temp = 0;
		int baris=0, flag = 0, ttl = 0;
		char list[7][100]={("      Add"), ("      View"), ("      Edit"), ("      Notes"), ("      Help"),
		("     Credits"), ("  Save and Exit")};
		char move='\0';
		char ch='\0';
		list[0][0]='>';
		list[0][16]='<';

		for(;;){
			ttl = 0;
			system("cls");
			title();
			printf("\n\n\n\t");
			printNotes();
			printf("\n\n");
			for(int k=0; k<7; k++){
				printf("\n\t\t\t\t\t");
				for(int j=0; j<17; j++) printf("%c", list[k][j]);
				printf("\n");
			}

			printf("\n\n\n\n\n\t\t\t\t    [Press Enter to select...]");
			ch=getch();
			if(ch==DOWN){
				if(baris<6) baris++;
				list[baris][0]='>';
				list[baris-1][0]='\0';
				list[baris][16]='<';
				list[baris-1][16]='\0';
			}
			else if (ch==UP){
				if(baris>0) baris--;
				list[baris][0]='>';
				list[baris+1][0]='\0';
				list[baris][16]='<';
				list[baris+1][16]='\0';
			}

			if(ch==ENTER){
				flag = baris + 1;
				break;
			}
		}
		switch(flag){
			case 1:
				menuAdd();
				break;
			case 2:
				menuView();
				break;
			case 3:
				menuEdit();
				break;
			case 4:
				menuNotes();
				break;
			case 5:
				help();
				break;
			case 6:
				credits();
				break;
			case 7:
				writeAccData();
				writeCtgData();
				writeTrsData();
				writeNoteData();
				loadingForSaveFile();
				temp = 1;
				printf("\n\n");
				system("taskkill /IM nircmd.exe /F");
				//return;
		}
	}while(temp!=1);
}

int main(){
	system("color 1f");
	srand(time(NULL));
	readAccData();
	readCtgData();
	readTrsData();
	readNoteData();
	if (firstAcc || notehead || trshead || ctghead) loadingForLoadFile();
	if(!firstAcc) declareDefAcc();
	if(!ctghead) declareDefTransactionType();
	system("start nircmd mediaplay 100000000 main.mp3");
	menu();
	return 0;
}
