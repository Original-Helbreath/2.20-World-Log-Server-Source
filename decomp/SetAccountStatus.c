/* WorldServer.exe  SetAccountStatus @ 0x0040a210   [V]
 * Reconstructed pseudo-code, not original source.
 * Table this+0xa06c: 5000 x CAccountStatus (0x30 bytes):
 *   +0x04 cState, +0x05 cAccount[11], +0x10 cPassword[11], +0x1b cGameServer[11],
 *   +0x28 iLevel, +0x2c dwTime (timeGetTime) */

int CWorldLog::SetAccountStatus(int iMode, char *pAccount, char *pPassword, int iLevel, char *pGameServer)
{
    int i = FindAccountStatus(pAccount);         /* -1 if absent */

    switch (iMode) {
    case DEF_ACCSTATUS_NULL:                     /* 0: delete */
        if (i == -1) return 0;
        PutLogList("(TestLog) Set account(%s) connection status NULL...");
        delete m_pAccountStatus[i]; m_pAccountStatus[i] = NULL;
        return 1;

    case DEF_ACCSTATUS_INIT:                     /* 1: from MLS SETACCOUNTINITSTATUS */
        if (i != -1 && m_pAccountStatus[i]->m_cState == DEF_ACCSTATUS_PLAYING) {
            /* already playing: update password/level and tell the owning game server */
            strcpy(m_pAccountStatus[i]->m_cPassword, pPassword);
            m_pAccountStatus[i]->m_iLevel = iLevel;
            int iGS = FindGameServerByName(m_pAccountStatus[i]->m_cGameServer);
            if (iGS != -1 && m_pGameList[iGS]->m_iClientH != -1 && m_pClientList[m_pGameList[iGS]->m_iClientH]) {
                char cMsg[30] = {0};
                *(DWORD *)cMsg = MSGID_REQUEST_CHECKACCOUNTPASSWORD;   /* 0x2654203A */
                *(WORD *)(cMsg + 4) = DEF_MSGTYPE_CONFIRM;
                memcpy(cMsg + 6,  pAccount, 10);
                memcpy(cMsg + 16, pPassword, 10);
                *(int *)(cMsg + 26) = iLevel;
                m_pClientList[m_pGameList[iGS]->m_iClientH]->m_pXSock->iSendMsg(cMsg, 30, 0);
            }
            return 1;
        }
        if (i == -1) i = NewAccountStatus(pAccount);   /* new CAccountStatus(account, timeGetTime()) */
        m_pAccountStatus[i]->m_cState = DEF_ACCSTATUS_INIT;
        m_pAccountStatus[i]->m_dwTime = timeGetTime();
        strcpy(m_pAccountStatus[i]->m_cPassword, pPassword);
        m_pAccountStatus[i]->m_iLevel = iLevel;
        PutLogList("(TestLog) Set account(%s)-Level(%d) connection status INIT...");
        return 1;

    case DEF_ACCSTATUS_WAIT:                     /* 2: from game server SETACCOUNTWAITSTATUS */
        if (i == -1) i = NewAccountStatus(pAccount);
        m_pAccountStatus[i]->m_cState = DEF_ACCSTATUS_WAIT;
        m_pAccountStatus[i]->m_iLevel = iLevel;
        m_pAccountStatus[i]->m_dwTime = timeGetTime();
        PutLogList("(TestLog) Set account(%s) connection status WAIT...");
        return 1;

    case DEF_ACCSTATUS_PLAYING:                  /* 3: from ENTERGAMECONFIRM */
        if (i == -1) {
            i = NewAccountStatus(pAccount);
            m_pAccountStatus[i]->m_cState = DEF_ACCSTATUS_PLAYING;
            memcpy(m_pAccountStatus[i]->m_cGameServer, pGameServer, 10);
            PutLogList("(TestLog) Set account(%s) connection status PLAYING(create & set)...");
            return 2;
        }
        m_pAccountStatus[i]->m_cState = DEF_ACCSTATUS_PLAYING;
        memcpy(m_pAccountStatus[i]->m_cGameServer, pGameServer, 10);
        PutLogList("(TestLog) Set account(%s) connection status PLAYING...");
        return 1;
    }
    return 0;
}

/* CheckAccountStatusTimeout @0x40aa30, every 10th call:
 *   INIT or WAIT older than 20000 ms -> SendMsgToMLS(MSGID_REQUEST_CLEARACCOUNTSTATUS, account 10)
 *   and SetAccountStatus(NULL). */
