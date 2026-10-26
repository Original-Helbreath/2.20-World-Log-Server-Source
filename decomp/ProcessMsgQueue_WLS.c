/* WorldServer.exe  ProcessMsgQueue_WLS  @ 0x004021b0   [V]
 * Reconstructed pseudo-code, not original source.
 * Drains the 10000-entry message queue (filled by EnqueueMsgFromSocket @0x401ff0).
 * Source (1 = accepted socket, 2 = MLS socket) is popped but NOT used for dispatch. */

void CWorldLog::ProcessMsgQueue()
{
    char  cFrom, cKey;
    DWORD dwSize;
    int   iSockIndex;
    char *pData = m_pMsgBuffer;                 /* this+0x18ad4, 30001 bytes */

    memset(pData, 0, 30001);
    while (bGetMsgQuene(&cFrom, pData, &dwSize, &iSockIndex, &cKey) == TRUE) {
        DWORD dwMsgID = *(DWORD *)pData;
        switch (dwMsgID) {
        case 0x0512A3E3:  /* game-server registration; 2.20 uses MSGID_REQUEST_REGISTERGAMESERVER 0x0512A3F4 */
            RegisterGameServer(iSockIndex, pData);                 break; /* 0x402510 */
        case MSGID_REQUEST_PLAYERDATA:            RequestPlayerData(iSockIndex, pData);          break; /* 0x403640 */
        case MSGID_REQUEST_SAVEPLAYERDATALOGOUT:  SavePlayerData(iSockIndex, pData, dwSize, TRUE,  FALSE); break;
        case MSGID_REQUEST_SAVEPLAYERDATA:        SavePlayerData(iSockIndex, pData, dwSize, FALSE, FALSE); break;
        case MSGID_REQUEST_SAVEPLAYERDATA_REPLY:  SavePlayerData(iSockIndex, pData, dwSize, TRUE,  TRUE);  break;
        case MSGID_REQUEST_NOSAVELOGOUT:          NoSaveLogout(iSockIndex, pData);               break; /* 0x405780 */
        case MSGID_REQUEST_CREATENEWCHARACTER:    CreateNewCharacter(iSockIndex, pData);         break; /* 0x405850, from MLS */
        case MSGID_REQUEST_ENTERGAME:             RequestEnterGame(iSockIndex, pData);           break; /* 0x405440, from client */
        case MSGID_REQUEST_DELETECHARACTER:       DeleteCharacter(iSockIndex, pData);            break; /* 0x407cc0, from MLS */
        case MSGID_REQUEST_CREATENEWGUILD:        CreateNewGuild(iSockIndex, pData);             break;
        case MSGID_REQUEST_DISBANDGUILD:          DisbandGuild(iSockIndex, pData);               break;
        case MSGID_REQUEST_UPDATEGUILDINFO_NEWGUILDSMAN: GuildAddGuildsman(iSockIndex, pData);   break;
        case MSGID_REQUEST_UPDATEGUILDINFO_DELGUILDSMAN: GuildDelGuildsman(iSockIndex, pData);   break;
        case MSGID_ENTERGAMECONFIRM:              EnterGameConfirm(iSockIndex, pData);           break; /* 0x4033f0 */
        case MSGID_GAMESERVERSHUTDOWNED:          GameServerShutdowned(iSockIndex);              break;
        case MSGID_REQUEST_FORCEDISCONECTACCOUNT: ForceDisconnectAccount(iSockIndex, pData);     break; /* from MLS */
        case MSGID_REQUEST_SAVEARESDENOCCUPYFLAGDATA: SaveOccupyFlagData(iSockIndex, pData, 1);  break;
        case MSGID_REQUEST_SAVEELVINEOCCUPYFLAGDATA:  SaveOccupyFlagData(iSockIndex, pData, 2);  break;
        case MSGID_SENDSERVERSHUTDOWNMSG:         SendServerShutdownToMLS();                     break;
        case MSGID_GAMEMASTERLOG:                 WriteGameMasterLog(pData, dwSize);             break;
        case MSGID_GAMEITEMLOG:                   WriteGameItemLog(pData, dwSize);               break;
        case MSGID_GAMECRUSADELOG:                WriteCrusadeLog(pData, dwSize);                break;
        case MSGID_REQUEST_CHARINFOLIST:          RequestCharInfoList(iSockIndex, pData);        break; /* from MLS */
        case MSGID_REQUEST_SETACCOUNTINITSTATUS:  /* from MLS: account 10, password 10, level i32 */
            SetAccountStatus(DEF_ACCSTATUS_INIT, pData + 6, pData + 16, *(int *)(pData + 26), NULL); break;
        case MSGID_REQUEST_SETACCOUNTWAITSTATUS:  /* from game server: account 10, level i32 */
            SetAccountStatus(DEF_ACCSTATUS_WAIT, pData + 6, NULL, *(int *)(pData + 16), NULL);       break;
        case MSGID_RESPONSE_REGISTER_WORLDSERVERSOCKET:
            m_bIsMainLogSockAvailable[iSockIndex] = TRUE;          /* this+0x27560[i] */          break;
        default: break;                                            /* unknown: dropped */
        }
    }
    UpdateScreen();                                                /* 0x40c420 */
}

/* Account states used by SetAccountStatus @0x40a210 (names from its log strings):
 * 0 NULL (entry deleted), 1 INIT, 2 WAIT, 3 PLAYING */
