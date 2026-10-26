/* WorldServer.exe  HandleEnterGameConfirm @ 0x004033f0   [V]
 * Reconstructed pseudo-code, not original source. */

void CWorldLog::EnterGameConfirm(int iClientH, char *pData)
{
    char cAccount[11] = {0}, cPassword[11] = {0}, cGameServer[11] = {0};
    char cStoredPw[11] = {0};
    int  iLevel = *(int *)(pData + 52), iStoredLevel;

    if (m_pClientList[iClientH] == NULL) return;
    memcpy(cAccount,    pData + 6,  10);
    memcpy(cPassword,   pData + 16, 10);
    memcpy(cGameServer, pData + 26, 10);         /* client IP at +36 unused */

    int iState = GetAccountStatus(cAccount, cStoredPw, &iStoredLevel);   /* 0x40a8f0 */
    if (iState == DEF_ACCSTATUS_INIT || iState == DEF_ACCSTATUS_WAIT) {
        if (strcmp(cPassword, cStoredPw) != 0 || iLevel != iStoredLevel) {
            PutLogList("(TestLog) ERROR! Account(%s) Password or level mismatch. Request Game Server to disconnect without data save...");
            char cMsg[16] = {0};
            *(DWORD *)cMsg = MSGID_REQUEST_FORCEDISCONECTACCOUNT;      /* 0x15000000 */
            *(WORD *)(cMsg + 4) = DEF_MSGTYPE_CONFIRM;                 /* read as count by 2.20 */
            memcpy(cMsg + 6, cAccount, 10);
            m_pClientList[iClientH]->m_pXSock->iSendMsg(cMsg, 16, 0);
            return;
        }
    }
    /* INIT/WAIT with matching data, or no entry at all: PLAYING on this game server */
    SetAccountStatus(DEF_ACCSTATUS_PLAYING, cAccount, NULL, 0, cGameServer);
}
