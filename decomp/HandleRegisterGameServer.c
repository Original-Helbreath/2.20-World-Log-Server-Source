/* WorldServer.exe  HandleRegisterGameServer @ 0x00402510   [V]
 * Reconstructed pseudo-code, not original source.
 * Layout = 2.20 MSGID_REQUEST_REGISTERGAMESERVER, but the WLS dispatches it as MsgID 0x0512A3E3. */

void CWorldLog::RegisterGameServer(int iClientH, char *pData)
{
    char  cName[11]  = {0};                     /* +6,  10 */
    char  cAddr[17]  = {0};                     /* +16, 16 */
    WORD  wPort      = *(WORD *)(pData + 32);
    int   iTotalMaps = (int)*(char *)(pData + 34);
    char *cpMaps     = pData + 35;              /* iTotalMaps x 11 */
    BOOL  bMapsOK    = TRUE;
    int   i, iGS;

    if (m_pClientList[iClientH] == NULL) return;
    memcpy(cName, pData + 6, 10);
    memcpy(cAddr, pData + 16, 16);

    for (i = 0; i < DEF_MAXGAMESERVERS; i++)                     /* 100 */
        if (m_pGameList[i] != NULL && memcmp(m_pGameList[i]->m_cName, cName, 10) == 0) {
            SendRegisterGameServerResponse(iClientH, FALSE);
            PutLogList("(!) Game Server Registration - fail : Server-Name(%s) already exists.");
            return;
        }
    for (iGS = 0; iGS < DEF_MAXGAMESERVERS && m_pGameList[iGS] != NULL; iGS++) ;
    if (iGS == DEF_MAXGAMESERVERS) { SendRegisterGameServerResponse(iClientH, FALSE); return; }

    m_pGameList[iGS] = new CGameServer(iClientH, cName, cAddr, wPort);  /* 0x4011b0 */
    for (i = 0; i < iTotalMaps; i++, cpMaps += 11)
        if (RegisterMap(iGS, cpMaps) == FALSE) bMapsOK = FALSE;       /* 0x402a80, 100 maps max */
    if (bMapsOK == FALSE) {
        PutLogList("(X) Game Server(%s) Registration - Fail: Map Duplicated!");
        SendRegisterGameServerResponse(iClientH, FALSE);
        return;
    }

    /* configs first, CONFIRM last */
    if (SendConfigFile(iClientH, "Npc.cfg")    && SendConfigFile(iClientH, "Item.cfg")   &&
        SendConfigFile(iClientH, "Item2.cfg")  && SendConfigFile(iClientH, "Magic.cfg")  &&
        SendConfigFile(iClientH, "Skill.cfg")  && SendConfigFile(iClientH, "Item3.cfg")  &&
        SendConfigFile(iClientH, "Quest.cfg")  && SendConfigFile(iClientH, "BuildItem.cfg") &&
        SendConfigFile(iClientH, "DupItemID.cfg") && SendConfigFile(iClientH, "Noticement.txt") &&
        SendConfigFile(iClientH, "Potion.cfg")) {
        SendConfigFile(iClientH, "GameData\\AresdenOccupyFlag.txt");   /* result ignored */
        SendConfigFile(iClientH, "GameData\\ElvineOccupyFlag.txt");
        SendRegisterGameServerResponse(iClientH, TRUE);   /* MSGID_RESPONSE_REGISTERGAMESERVER, 0x0F14 */
        m_pClientList[iClientH]->m_cMode = 2;             /* client+4: game server */

        /* forward to MLS: world 30, name 10, addr 16, port u32, map count u32, maps 11 each */
        char cBuf[0x40 + 11 * 100] = {0};
        memcpy(cBuf,       m_cWorldName, 30);
        memcpy(cBuf + 30,  cName, 10);
        memcpy(cBuf + 40,  cAddr, 16);
        *(int *)(cBuf + 56) = wPort;
        *(int *)(cBuf + 60) = iTotalMaps;
        memcpy(cBuf + 64,  pData + 35, iTotalMaps * 11);
        SendMsgToMLS(MSGID_REGISTER_WORLDSERVER_GAMESERVER, DEF_MSGTYPE_CONFIRM, cBuf, 0x40 + iTotalMaps * 11, -1);
        return;
    }
    PutLogList("(X) Game Server(%s) Registration - Fail: Cannot send configuration data!");
    SendRegisterGameServerResponse(iClientH, FALSE);
}

/* SendRegisterGameServerResponse @0x4029c0: 6-byte message, MsgID 0x0512A3F5,
 * MsgType 0x0F14 (TRUE) or 0x0F15 (FALSE), key 0. On send failure: GameServerConnectionLost. */
