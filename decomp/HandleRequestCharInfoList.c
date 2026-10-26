/* WorldServer.exe  HandleRequestCharInfoList @ 0x00404370   [V]
 * Reconstructed pseudo-code, not original source.
 * The WLS builds the complete client login-CONFIRM message and the MLS only forwards it. */

void CWorldLog::RequestCharInfoList(int iClientH, char *pData)
{
    char  cBuf[2000] = {0};
    char *cp;
    int   i, iTotalChar = (int)*(char *)(pData + 37);
    char *cpNames = pData + 46;                 /* iTotalChar x 10 */

    PutLogList("(TestLog) Getting character data in account(%s)-ID(%d)...");
    memcpy(cBuf, pData + 6, 10);                /* account */
    *(DWORD *)(cBuf + 10) = *(DWORD *)(pData + 16);       /* MLS cookie */
    *(DWORD *)(cBuf + 14) = MSGID_RESPONSE_LOG;           /* inner client message */
    *(WORD  *)(cBuf + 18) = DEF_LOGRESMSGTYPE_CONFIRM;
    memcpy(cBuf + 20, pData + 20, 18);          /* version, status, 6 dates (17 bytes) + count */
    cp = cBuf + 38;

    for (i = 0; i < iTotalChar; i++, cpNames += 10) {
        CHARINFO ci;
        memcpy(cp, cpNames, 10);
        if (GetCharacterInfoFromFile(cpNames, &ci) == 1) {          /* 0x404710 */
            cp[10] = 1;                                              /* valid */
            *(short *)(cp + 11) = ci.sAppr1;  *(short *)(cp + 13) = ci.sAppr2;
            *(short *)(cp + 15) = ci.sAppr3;  *(short *)(cp + 17) = ci.sAppr4;
            *(short *)(cp + 19) = ci.sSex;    *(short *)(cp + 21) = ci.sSkin;
            *(short *)(cp + 23) = ci.sLevel;  *(int   *)(cp + 25) = ci.iExp;
            *(short *)(cp + 29) = ci.sStr;    *(short *)(cp + 31) = ci.sVit;
            *(short *)(cp + 33) = ci.sDex;    *(short *)(cp + 35) = ci.sInt;
            *(short *)(cp + 37) = ci.sMag;    *(short *)(cp + 39) = ci.sChr;
            *(int   *)(cp + 41) = ci.iApprColor;
            *(short *)(cp + 45) = ci.sYear;   *(short *)(cp + 47) = ci.sMonth;
            *(short *)(cp + 49) = ci.sDay;    *(short *)(cp + 51) = ci.sHour;
            *(short *)(cp + 53) = ci.sMinute;
            memcpy(cp + 55, ci.cMapName, 10);
            cp += 65;
        } else {
            cp[10] = 0;                                              /* damaged: 50-byte slot */
            cp += 50;
        }
    }
    *(int *)cp       = *(int *)(pData + 38);    /* account time left */
    *(int *)(cp + 4) = *(int *)(pData + 42);    /* IP time left */

    SendMsgToMLS(MSGID_RESPONSE_CHARINFOLIST, DEF_MSGTYPE_CONFIRM, cBuf, iTotalChar * 65 + 0x2e, -1);
}
