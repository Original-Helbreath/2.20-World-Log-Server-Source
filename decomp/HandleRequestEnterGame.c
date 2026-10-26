/* WorldServer.exe  HandleRequestEnterGame @ 0x00405440   [V]
 * Reconstructed pseudo-code, not original source.
 * Client after a server change (kind NEW_TOWLSBUTMLS); the kind in MsgType is never read. */

void CWorldLog::RequestEnterGame(int iClientH, char *pData)
{
    char cCharName[11] = {0}, cMapName[11] = {0}, cAccount[11] = {0};
    char cAddr[17] = {0}, cPw[11]; int iPort, iLevel;
    char cMsg[256] = {0};
    int  iSize;

    if (m_pClientList[iClientH] == NULL) return;
    memcpy(cCharName, pData + 6, 10);
    memcpy(cMapName,  pData + 16, 10);
    memcpy(cAccount,  pData + 26, 10);          /* password, level, world, token: not read */

    char cRet = FindGameServerByMap(cMapName, cAddr, &iPort);   /* 0x4056c0: -1 not found, 0 ok */
    PutLogList("(TestLog) ReqEnterGame: Accnt(%s) Char(%s) Map(%s) Addr(%s) Port(%d) Code(%d)");

    *(DWORD *)cMsg = MSGID_RESPONSE_ENTERGAME;
    if (cRet == -1) {
        *(WORD *)(cMsg + 4) = DEF_ENTERGAMERESTYPE_REJECT;   /* 0x0F21 */
        cMsg[6] = 3;                                         /* game server not running */
        iSize = 7;
    }
    else if (cRet == -2) {                                   /* unreachable: 0x4056c0 never returns -2 */
        *(WORD *)(cMsg + 4) = DEF_ENTERGAMERESTYPE_REJECT;
        cMsg[6] = 5;
        iSize = 7;
    }
    else {
        if (GetAccountStatus(cAccount, cPw, &iLevel) == 0) { /* no INIT/WAIT/PLAYING entry */
            delete m_pClientList[iClientH]; m_pClientList[iClientH] = NULL;
            PutLogList("(TestLog) ReqEnterGame Reject! Account(%s) status NULL");
            return;                                          /* no reply */
        }
        *(WORD *)(cMsg + 4) = DEF_ENTERGAMERESTYPE_CONFIRM;  /* 0x0F22 */
        memcpy(cMsg + 6, cAddr, 16);
        *(WORD *)(cMsg + 22) = (WORD)iPort;
        iSize = 24;                                          /* no game-server name (2.20 client reads 20 more) */
    }
    if (m_pClientList[iClientH]->m_pXSock->iSendMsg(cMsg, iSize, 0) < 0) {
        delete m_pClientList[iClientH]; m_pClientList[iClientH] = NULL;
    }
}
