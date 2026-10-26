/* WorldServer.exe  HandleSavePlayerData @ 0x00404e80   [V]
 * Reconstructed pseudo-code, not original source. FILE mode only (no DB-mode block parser).
 * Called for SAVEPLAYERDATA (logout=0,reply=0), SAVEPLAYERDATALOGOUT (1,0), SAVEPLAYERDATA_REPLY (1,1). */

void CWorldLog::SavePlayerData(int iClientH, char *pData, DWORD dwMsgSize, BOOL bLogout, BOOL bReply)
{
    char cCharName[11] = {0}, cAccount[11] = {0}, cFn[256] = {0}, cTmp[256];
    char cCountFlag = *(pData + 36);            /* 2.20 "counting flag" */
    FILE *pFile;

    if (m_pClientList[iClientH] == NULL) return;
    memcpy(cCharName, pData + 6, 10);
    memcpy(cAccount,  pData + 16, 10);          /* password at +26 is not checked */

    strcat(cFn, "Character"); strcat(cFn, "\\");
    wsprintf(cTmp, "AscII%d", (unsigned char)cCharName[0]); strcat(cFn, cTmp);
    strcat(cFn, "\\"); strcat(cFn, cCharName); strcat(cFn, ".txt");

    CreateFolderForFile(cFn);                   /* 0x4052f0: creates Character\AscIIn\ */
    if (dwMsgSize - 37 == 0)
        PutLogList("(X) Character data body empty: Cannot create & save data file.");
    else if ((pFile = fopen(cFn, "wt")) == NULL)
        PutLogList("(X) Cannot create character data file : Name(%s)");
    else {
        PutLogList("(O) Character data file saved : Name(%s)");
        fwrite(pData + 37, dwMsgSize - 36, 1, pFile);   /* text from +37 */
        fclose(pFile);
    }

    if (bLogout == TRUE && cCountFlag == 1) {
        PutLogList("(TestLog) Account(%s) log out request to MLS...");
        SendMsgToMLS(MSGID_REQUEST_CLEARACCOUNTSTATUS, DEF_MSGTYPE_CONFIRM, cAccount, 10, -1);
        SetAccountStatus(DEF_ACCSTATUS_NULL, cAccount, NULL, 0, NULL);
    }
    if (bReply == TRUE) {
        char cMsg[16] = {0};
        *(DWORD *)cMsg = MSGID_RESPONSE_SAVEPLAYERDATA_REPLY;     /* 0x0C152212 */
        *(WORD *)(cMsg + 4) = DEF_MSGTYPE_CONFIRM;
        memcpy(cMsg + 6, cCharName, 10);
        m_pClientList[iClientH]->m_pXSock->iSendMsg(cMsg, 16, 0);
    }
}
