#include <iostream>
#include <vector>
#include <windows.h>
#include <TlHelp32.h>  
#include <winternl.h>  

// process name'den process id'yi alıyor
DWORD ProcessIdAl(const char* UygulamaIsim) {
    DWORD ProcessId = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 ProcessEntry;
        ProcessEntry.dwSize = sizeof(ProcessEntry);

        if (Process32First(hSnap, &ProcessEntry)) {
            do {
                if (!_strcmpi(ProcessEntry.szExeFile, UygulamaIsim)) {
                    ProcessId = ProcessEntry.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnap, &ProcessEntry));
        }
    }
    CloseHandle(hSnap);
    return ProcessId;
}

// pointer zinciri okuma
uintptr_t PointerZinciriOku(HANDLE hProc, DWORD64 ptr, const std::vector<unsigned int>& offsets) {
    uintptr_t addr = ptr;
    for (unsigned int i = 0; i < offsets.size(); ++i) {
        if (!ReadProcessMemory(hProc, (BYTE*)addr, &addr, sizeof(DWORD64), nullptr)) {
            return 0; // okuma başarısızsa kapat
        }
        addr += offsets[i];
    }
    return addr;
}

int main() {
    const char* OyunIsim = "RedLock-Win64-Shipping.exe";

    DWORD ProcessID = ProcessIdAl(OyunIsim);
    if (ProcessID == 0) {
        std::cout << "Process ID alimi basarisiz!" << std::endl;
        return 1;
    }
    std::cout << "Process ID: " << ProcessID << std::endl;

    HANDLE OyunHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessID);
    if (!OyunHandle) {
        std::cout << "handle erisimi basarisiz!" << std::endl;
        return 1;
    }

    // base addresi alir
    PROCESS_BASIC_INFORMATION pbi;
    ULONG Size = 0;
    typedef NTSTATUS(NTAPI* pNtQueryInfo)(HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);
    pNtQueryInfo NtQueryInformationProcess =
        (pNtQueryInfo)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryInformationProcess");
    if (NtQueryInformationProcess(OyunHandle, ProcessBasicInformation, &pbi, sizeof(pbi), &Size) != 0) {
        std::cout << "NtQueryInformationProcess basarisiz! (bunun ne oldugun hala bilmiyom xd)" << std::endl;
        return 1;
    }

    PPEB PEB = pbi.PebBaseAddress;
    DWORD64 BaseAdress = 0;
    DWORD64 HedefAdress = (DWORD64)PEB + 0x10;
    ReadProcessMemory(OyunHandle, (LPCVOID)HedefAdress, &BaseAdress, sizeof(DWORD64), nullptr);

    std::cout << "Base Address: 0x" << std::hex << BaseAdress << std::endl;

    // Pointer zinciri
    std::vector<unsigned int> offsets = { 0xB8, 0x20, 0x20, 0xA0, 0x2A0, 0x370, 0x108 };
    uintptr_t finalAddr = PointerZinciriOku(OyunHandle, BaseAdress + 0x0928AA38, offsets); // 0x123456 örnek offset

    if (finalAddr == 0) {
        std::cout << "Pointer zinciri okunamadi!" << std::endl;
        return 1;
    }

    float stamina = 0.f;
    ReadProcessMemory(OyunHandle, (LPCVOID)finalAddr, &stamina, sizeof(stamina), nullptr);
    std::cout << "Stamina: " << stamina << std::endl;

    // sta değiştirme
    float yeniStamina = 9999.f;
    std::cout << "aga stamina hilesi V tusuna basildiginda aciliyor her basisinda fulleniyo speed" << std::endl;

    while (true) {
        if (GetAsyncKeyState('V') & 0x8000) {
            WriteProcessMemory(OyunHandle, (LPVOID)finalAddr, &yeniStamina, sizeof(yeniStamina), nullptr);
            std::cout << "stamina heck basarili!!" << std::endl;


            }
          
    }
        
        
        

    CloseHandle(OyunHandle);
    return 0;
}
// By Lyarths
// Discord: 9.9.9.9.9
