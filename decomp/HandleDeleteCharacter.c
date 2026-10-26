/* WorldServer.exe  HandleDeleteCharacter @ 0x00407cc0   [V strings and reply ID; field 26 I]
 * Reconstructed pseudo-code, not original source. Request relayed by the MLS, which has already
 * removed the name from the account file. */

void CWorldLog::DeleteCharacter(int iClientH, char *pData)
{
    char  cName[11] = {0}, cAccount[11] = {0}, cFileAccount[11] = {0}, cFn[256], cMap[11];
    int   iLevel;
    DWORD dwCookie   = *(DWORD *)(pData + 26);  /* [I] */
    int   iTotalChar = (int)*(char *)(pData + 30);
    char *cpNames    = pData + 31;              /* remaining names, 11 bytes each */

    memcpy(cName, pData + 6, 10);
    memcpy(cAccount, pData + 16, 10);
    ReadCharOwnerLevelMapFromFile(cName, cFileAccount, cMap, &iLevel);     /* 0x40aff0 */
    if (strcmp(cAccount, cFileAccount) != 0) {
        PutLogList("(TestLog) Error! Account mismatch(%s - %s)! Cannot delete character(%s).");
        /* reply RESPONSE_LOG with a failure type */
        return;
    }
    BuildCharFileName(cFn, cName);              /* Character\AscII<n>\<name>.txt */
    DeleteFile(cFn);
    PutLogList("(O) Character(%s) deleted! Account(%s)-ID(%d)-Path(%s)");

    /* reply via MLS: account 10, cookie, RESPONSE_LOG / DEF_LOGRESMSGTYPE_CHARACTERDELETED (0x0F1F),
       then the 2.20 payload: account status u8, count u8, 65-byte slots for the remaining names */
    SendMsgToMLS(MSGID_RESPONSE_LOG, DEF_MSGTYPE_CONFIRM, cReply, iReplySize, -1);
}
