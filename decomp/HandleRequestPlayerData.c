/* WorldServer.exe  HandleRequestPlayerData @ 0x00403640   [V; guild-status details I]
 * Reconstructed pseudo-code, not original source. FILE mode only. */

void CWorldLog::RequestPlayerData(int iClientH, char *pData)
{
    char cCharName[11] = {0}, cAccount[11] = {0}, cFileAccount[11] = {0};
    char cFn[256] = {0}, cTmp[256];
    int  iLevel; char cMap[11];
    FILE *pFile;

    if (m_pClientList[iClientH] == NULL) return;
    memcpy(cCharName, pData + 6, 10);           /* MSGID_REQUEST_PLAYERDATA: name 10 */
    memcpy(cAccount,  pData + 16, 10);          /* account 10; password (+26) and IP (+36) unused */
    if (strlen(cCharName) == 0) PutLogList("(X) CharName NULL!");
    if (strlen(cAccount)  == 0) PutLogList("(X) AccountName NULL!");

    /* reads account-name, character-loc-map, character-LEVEL from the file */
    ReadCharOwnerLevelMapFromFile(cCharName, cFileAccount, cMap, &iLevel);   /* 0x40aff0 */

    strcat(cFn, "Character"); strcat(cFn, "\\");
    wsprintf(cTmp, "AscII%d", (unsigned char)cCharName[0]); strcat(cFn, cTmp);
    strcat(cFn, "\\"); strcat(cFn, cCharName); strcat(cFn, ".txt");

    pFile = fopen(cFn, "rt");
    if (pFile != NULL && strcmp(cAccount, cFileAccount) == 0) {
        DWORD dwFileSize = filelength(pFile);
        char  cGuildStatus = CheckGuildStatusForPlayerData(pFile, dwFileSize);  /* 0x403b10: 0 = guild gone */
        char *cp = new char[dwFileSize + 1000];
        memset(cp, 0, dwFileSize + 1000);
        *(DWORD *)cp       = MSGID_RESPONSE_PLAYERDATA;
        *(WORD  *)(cp + 4) = DEF_LOGRESMSGTYPE_CONFIRM;      /* 0x0F14 */
        memcpy(cp + 6, cCharName, 10);
        cp[16] = 2;                                          /* account status (ignored by 2.20) */
        cp[17] = cGuildStatus;
        fread(cp + 18, dwFileSize, 1, pFile);                /* character file text */
        m_pClientList[iClientH]->m_pXSock->iSendMsg(cp, dwFileSize + 19, 0);   /* +1 NUL */
        delete[] cp;
        PutLogList("(TestLog) Send character(%s) data...");
    }
    else {
        char cRej[16] = {0};
        *(DWORD *)cRej = MSGID_RESPONSE_PLAYERDATA;
        *(WORD *)(cRej + 4) = DEF_LOGRESMSGTYPE_REJECT;      /* 0x0F15 */
        memcpy(cRej + 6, cCharName, 10);
        PutLogList("(X) RequestPlayerData Error! Fn(%s) Account(%s, %s)");
        m_pClientList[iClientH]->m_pXSock->iSendMsg(cRej, 16, 0);
    }
    if (pFile) fclose(pFile);
    /* on socket error: GameServerConnectionLost(iClientH) and delete the client */
}
