/* WorldServer.exe  HandleMainLogSocketEvent @ 0x00401d40 (connect branch),
 *                  SendRegisterWorldServer @ 0x00404240, SendRegisterExtraMLSSocket @ 0x004042e0,
 *                  SendRegisterAllGameServersToMLS @ 0x0040ae50, SendMsgToMLS @ 0x00403fd0   [V]
 * Reconstructed pseudo-code, not original source. */

/* 10 sockets to main-log-server-address:port, window messages 0x464..0x46D (InitializeWorldServer @0x4014e0) */
void CWorldLog::OnMainLogSockConnected(int iIndex)
{
    PutLogList("(!) Main-log-socket(%d) connected.");
    m_iMainLogSockActiveCount++;
    if (m_bIsWorldRegistered == FALSE) {          /* this+0x3d53c */
        m_bIsWorldRegistered = TRUE;
        SendRegisterWorldServer(iIndex);          /* MSGID_REGISTER_WORLDSERVER */
        SendRegisterAllGameServersToMLS();        /* MSGID_REGISTER_WORLDSERVER_GAMESERVER x N */
    }
    else SendRegisterExtraMLSSocket(iIndex);      /* MSGID_REGISTER_WORLDSERVERSOCKET */
}

void CWorldLog::SendRegisterWorldServer(int iIndex)      /* and ..._WORLDSERVERSOCKET: same body */
{
    char cBuf[100] = {0};
    memcpy(cBuf,      m_cWorldName,    30);       /* world-server-name */
    memcpy(cBuf + 30, m_cWorldAddress, 16);       /* world-server-address */
    *(int *)(cBuf + 46) = m_iWorldServerPort;     /* world-server-port (client port) */
    SendMsgToMLS(MSGID_REGISTER_WORLDSERVER, DEF_MSGTYPE_CONFIRM, cBuf, 50, iIndex);
}

/* MLS answers MSGID_RESPONSE_REGISTER_WORLDSERVERSOCKET on each socket; only then
 * m_bIsMainLogSockAvailable[i] = TRUE and SendMsgToMLS may use it. */

BOOL CWorldLog::SendMsgToMLS(DWORD dwMsgID, WORD wMsgType, char *pData, DWORD dwSize, int iIndex)
{
    if (iIndex == -1) {                           /* round robin over available sockets */
        for (int n = 0; n < 10 && !m_bIsMainLogSockAvailable[m_iCurMLSock]; n++)
            m_iCurMLSock = (m_iCurMLSock + 1) % 10;
    } else m_iCurMLSock = iIndex;

    char *cp = m_cMLSBuffer;                      /* this+0x20005 */
    memset(cp, 0, 30001);
    *(DWORD *)cp = dwMsgID; *(WORD *)(cp + 4) = wMsgType;
    memcpy(cp + 6, pData, dwSize);

    if (m_bIsMainLogSockAvailable[m_iCurMLSock] || iIndex != -1) {
        if (m_pMainLogSock[m_iCurMLSock]->iSendMsg(cp, dwSize + 6, (char)rand()) < 0) {
            PutLogList("(X) Main-log-socket(%d) send error!");
            /* recreate that socket and reconnect; mark unavailable */
            return FALSE;
        }
        return TRUE;
    }
    PutLogList("(X) No MLS-Socket Available!  MsgID(%x)");
    return FALSE;
}
