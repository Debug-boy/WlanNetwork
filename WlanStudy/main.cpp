#include <iostream>
#include<windows.h>

#include <wlanapi.h>
#include<objbase.h>

#pragma comment(lib,"wlanapi.lib")
#pragma comment(lib,"ole32.lib")

DWORD g_ClientVersion = 1;	//�����Ŀͻ��汾
DWORD g_NowClientVersion = -1;	//��ǰʹ�õĿͻ��汾,�����Ķ��پͷ��ض���

HANDLE g_ClientHandle = NULL;	//���ú����ر��Ŀͻ����

PWLAN_INTERFACE_INFO_LIST g_WlanDriverList = NULL;	//���������豸��Ϣ�б�;
PWLAN_INTERFACE_INFO g_MyUseDriverObj = NULL;//ʹ��ĳ�����������豸����

PWLAN_BSS_LIST g_pBssList = NULL;//�洢����AP�б�

//����������ú󷵻صĴ�����Ϣ
void Wlan_OutResultMsg(const char *funName,DWORD dwResult) {
	switch (dwResult)
	{
	case ERROR_SUCCESS:
		printf("%s : SUCCESS!\n", funName);
		break;

	default:
		printf("%s : ERROR is %u!\n", funName, dwResult);
		break;
	}
}

//��ȡ��������״̬��Ϣ
char* Wlan_GetInterfaceMsg(WLAN_INTERFACE_STATE state) {
	static char InterfaceMsg[1024];
	memset(InterfaceMsg, 0, sizeof(InterfaceMsg));
	switch (state)
	{
	case wlan_interface_state_not_ready:
		sscanf("Not ready", "%[^\n]%s", InterfaceMsg);//��û��׼����
		break;
	case wlan_interface_state_connected:
		sscanf("Connected", "%[^\n]%s", InterfaceMsg);//���ӵ�״̬
		break;
	case wlan_interface_state_ad_hoc_network_formed:
		sscanf("First node in a ad hoc network", "%[^\n]%s", InterfaceMsg);//�����еĵ�һ���ڵ�
		break;
	case wlan_interface_state_disconnecting:
		sscanf("Disconnecting", "%[^\n]%s", InterfaceMsg);//�Ͽ�����
		break;
	case wlan_interface_state_disconnected:
		sscanf("Not connected", "%[^\n]%s", InterfaceMsg);//�ѶϿ�����
		break;
	case wlan_interface_state_associating:
		sscanf("Attempting to associate with a network", "%[^\n]%s", InterfaceMsg);//�������������
		break;
	case wlan_interface_state_discovering:
		sscanf("Auto configuration is discovering settings for the network", "%[^\n]%s", InterfaceMsg);//�Զ��������ڷ������������
		break;
	case wlan_interface_state_authenticating:
		sscanf("In process of authenticating", "%[^\n]%s", InterfaceMsg);//���ڽ��������֤
		break;
	default:
		sscanf("Unknown state", "%[^\n]%s", InterfaceMsg);//δ֪״̬
		break;
	}
	return InterfaceMsg;
}


//�������������±�,ѡ��ʹ�õ���������,����ѡ���±���0��ʼ
DWORD Wlan_setMyuseDriver(DWORD index) {
	DWORD dwResultCode = WlanEnumInterfaces(g_ClientHandle, NULL, &g_WlanDriverList);//���һ������Ŀ���һ����ָ��
	if (dwResultCode == ERROR_SUCCESS)
		g_MyUseDriverObj = (PWLAN_INTERFACE_INFO)(&g_WlanDriverList->InterfaceInfo[index]);
	return dwResultCode;
}

DWORD Wlan_GetDriverInfo(unsigned int isOutMsg) {
	DWORD dwResultCode = WlanEnumInterfaces(g_ClientHandle, NULL, &g_WlanDriverList);//���һ������Ŀ���һ����ָ��
	if (dwResultCode != ERROR_SUCCESS) {
		Wlan_OutResultMsg("WlanEnumInterfaces", dwResultCode);
	}
	else {
		DWORD i;
		WCHAR GuidBuffer[40] = { 0 };//��GUID BYTE ת�����ַ����Ļ�����
		PWLAN_INTERFACE_INFO pDriverInfo = NULL;//��ʼ���豸ΪNULL
		printf("\n");
		//printf("WlanDriver Number->%d\n", g_WlanDriverList->dwNumberOfItems);
		for (i = 0; i < g_WlanDriverList->dwNumberOfItems; i++) {
			pDriverInfo = (PWLAN_INTERFACE_INFO)(&g_WlanDriverList->InterfaceInfo[i]);
			if (isOutMsg) {
				printf("------------------------index:%d------------------------\n", i);
				wprintf(L"Diriver->Name:%ws\n", pDriverInfo->strInterfaceDescription);
				StringFromGUID2(pDriverInfo->InterfaceGuid, (LPOLESTR)&GuidBuffer, 39);
				wprintf(L"\tDiriver->Guid:%ws\n", GuidBuffer);
				printf("\tDiriver->State:%s\n", Wlan_GetInterfaceMsg(pDriverInfo->isState));
				printf("--------------------------------------------------------\n");
			}
		}
		int index = 0;
		if (g_MyUseDriverObj == NULL) {
			printf("input Please select DriverObject:");
			scanf("%d", &index);
			Wlan_setMyuseDriver(index);//��������ʹ�ö���
		}
	}
	return  dwResultCode;
}


//csdn����
void charTowchar(const char* chr, wchar_t* wchar, int size){
	MultiByteToWideChar(CP_ACP, 0, chr,
		strlen(chr) + 1, wchar, size / sizeof(wchar[0]));
}

//�����������ɨ����Ϣ
DWORD Wlan_GetBssInfo() {
	if (g_MyUseDriverObj == NULL)
		Wlan_GetDriverInfo(true);
	PWLAN_BSS_ENTRY pEntry = NULL;//ѡ������AP����ʵ��
	DWORD dwResultCode = WlanGetNetworkBssList(g_ClientHandle, (&g_MyUseDriverObj->InterfaceGuid), NULL, dot11_BSS_type_any, true, NULL, &g_pBssList);
	printf("\n");
	printf("--------------------------------------------------------------------------------------------\n");
	printf("INDEX\t\t\tSSID\t\t\tMAC_ADDR\t\t\tSIGNAL_DB\n");
	for (DWORD i = 0; i < g_pBssList->dwNumberOfItems; i++) {
		printf("%d\t\t\t", i);
		pEntry = (PWLAN_BSS_ENTRY)(&g_pBssList->wlanBssEntries[i]);
		printf("%s\t\t", pEntry->dot11Ssid.ucSSID);
		printf("[%02X:%02X:%02X:%02X:%02X:%02X]\t\t",
			pEntry->dot11Bssid[0], pEntry->dot11Bssid[1], pEntry->dot11Bssid[2],
			pEntry->dot11Bssid[3], pEntry->dot11Bssid[4], pEntry->dot11Bssid[5]
		);
		printf("%d\n", pEntry->lRssi);
	}
	printf("--------------------------------------------------------------------------------------------\n");
	return dwResultCode;
}

DWORD Wlan_Connect() {
	DWORD dwResultCode = 0;
	Wlan_GetBssInfo();
	printf("input index Connection object:");
	unsigned int index = 0;
	WCHAR strProfileBuffer[64] = { 0 };
	scanf("%d", &index);
	PWLAN_CONNECTION_PARAMETERS pConnectConfig = NULL;
	WLAN_CONNECTION_PARAMETERS wlanConnPara;
	wlanConnPara.wlanConnectionMode = wlan_connection_mode_profile;
	charTowchar((char*)g_pBssList->wlanBssEntries[index].dot11Ssid.ucSSID, strProfileBuffer, 63);
	wlanConnPara.strProfile = strProfileBuffer;
	wlanConnPara.pDot11Ssid = NULL;         // SET SSID NULL 
	wlanConnPara.dot11BssType = dot11_BSS_type_infrastructure;
	wlanConnPara.pDesiredBssidList = NULL;
	wlanConnPara.dwFlags = WLAN_CONNECTION_HIDDEN_NETWORK;
	pConnectConfig = &wlanConnPara;
	if (g_MyUseDriverObj != NULL)
		dwResultCode = WlanConnect(g_ClientHandle, (&g_MyUseDriverObj->InterfaceGuid), pConnectConfig, NULL);
	if (dwResultCode != ERROR_SUCCESS) {
		Wlan_OutResultMsg("WlanConnect", dwResultCode);
	}
	else {
		printf("Out:Connect is Success!\n");
	}
	return dwResultCode;
}

int main(int argc, char* argv[]) {

	DWORD dwResultCode = 0;
	dwResultCode = WlanOpenHandle(g_ClientVersion, NULL, &g_NowClientVersion, &g_ClientHandle);

	if (dwResultCode != ERROR_SUCCESS) {
		Wlan_OutResultMsg("WlanOpenHandle", dwResultCode);
		return EXIT_FAILURE;
	}
	//printf("g_ClientHandle->%08X\n", g_ClientHandle);

	unsigned int funCode;
	printf("------------------------------\n");
	printf("index\t\t\tFUNCTION\n");
	printf("1\t\t\tOut Driver INFO\n");
	printf("2\t\t\tOut WLAN_BSS INFO\n");
	printf("3\t\t\tConnect BssWlan\n");
	printf("------------------------------\n");
	printf("inputf index Function:");
	scanf("%d", &funCode);

	switch (funCode)
	{
	case 1:
		Wlan_GetDriverInfo(true);
		break;

	case 2:
		Wlan_GetBssInfo();
		break;

	case 3:
		Wlan_Connect();
		break;

	default:
		break;
	}

	WlanCloseHandle(g_ClientHandle, NULL);

	return EXIT_SUCCESS;
}