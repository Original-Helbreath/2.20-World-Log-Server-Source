/* WorldServer.exe  HandleCreateNewCharacter @ 0x00405850   [V field reads 0x4058fd-0x4059fe,
 *                   reply IDs 0x405c9f, 0x407a0f; file template strings 0x417ce0-0x418404; HP/MP/SP I]
 * Reconstructed pseudo-code, not original source. Ghidra's decompile of this function is broken
 * (very large stack frame); this version follows the disassembly. Request relayed by the MLS. */

void CWorldLog::CreateNewCharacter(int iClientH, char *pData)
{
    char cName[11] = {0}, cAccount[11] = {0}, cPassword[11] = {0}, cFn[256] = {0};
    char *cp = pData + 66;
    char cSex = cp[0], cSkin = cp[1], cHairStyle = cp[2], cHairColor = cp[3], cUnderwear = cp[4];
    char cStr = cp[5], cVit = cp[6], cDex = cp[7], cInt = cp[8], cMag = cp[9], cChr = cp[10];
    DWORD dwCookie   = *(DWORD *)(pData + 77);
    int   iTotalChar = (int)*(char *)(pData + 81);       /* existing names, 11 bytes each at +82 */
    short sAppr1     = (cHairStyle << 8) | (cHairColor << 4) | cUnderwear;

    memcpy(cName, pData + 6, 10); memcpy(cAccount, pData + 16, 10); memcpy(cPassword, pData + 26, 10);
    PutLogList("(TestLog) Request create new character(%s). Account(%s) Password(%s)");
    BuildCharFileName(cFn, cName);              /* Character\AscII<n>\<name>.txt */

    char cReply[0x14 + 65 * 4 + 16] = {0};
    memcpy(cReply, cAccount, 10);
    *(DWORD *)(cReply + 10) = dwCookie;
    *(DWORD *)(cReply + 14) = MSGID_RESPONSE_LOG;

    if (FileExists(cFn)) {
        PutLogList("(X) Cannot create new character - Already existing character(%s)");
        *(WORD *)(cReply + 18) = DEF_LOGRESMSGTYPE_ALREADYEXISTINGCHARACTER;   /* 0x0F1E */
        SendMsgToMLS(MSGID_RESPONSE_LOG, DEF_LOGRESMSGTYPE_ALREADYEXISTINGCHARACTER, cReply, 0x14, -1);
        return;
    }
    FILE *pFile = fopen(cFn, "wt");
    if (pFile == NULL) {
        PutLogList("(!) Cannot create new character - cannot create file : Name(%s)");
        *(WORD *)(cReply + 18) = DEF_LOGRESMSGTYPE_NEWCHARACTERFAILED;         /* 0x0F1D */
        SendMsgToMLS(MSGID_RESPONSE_LOG, DEF_LOGRESMSGTYPE_NEWCHARACTERFAILED, cReply, 0x14, -1);
        return;
    }
    fprintf(pFile, "Character-updated:  Time(%d/%d/%d/%d/%d)\n", ...);
    fprintf(pFile, "[NAME-ACCOUNT]\ncharacter-name     = %s\naccount-name       = %s\n"
                   "account-password   = %s\n", cName, cAccount, cPassword);   /* plaintext */
    fprintf(pFile, "[STATUS]\ncharacter-location   = NONE\ncharacter-guild-name = NONE\n"
                   "character-guild-rank = -1\ncharacter-loc-map  = default\n"
                   "character-loc-x   = -1\ncharacter-loc-y   = -1\n");
    fprintf(pFile, "character-HP       = %d\ncharacter-MP       = %d\ncharacter-SP       = %d\n"
                   "character-DefenseRatio = %d\n", iHP, iMP, iSP, iDR);      /* formulas not recovered */
    fprintf(pFile, "character-LEVEL    = 1\ncharacter-STR      = %d\ncharacter-INT      = %d\n"
                   "character-VIT      = %d\ncharacter-DEX      = %d\ncharacter-MAG      = %d\n"
                   "character-CHARISMA = %d\ncharacter-LUCK     = 10\ncharacter-EXP      = 0\n"
                   "character-LU_Pool   = 0\ncharacter-EK-Count  = 0\ncharacter-PK-Count  = 0\n",
                   cStr, cInt, cVit, cDex, cMag, cChr);
    fprintf(pFile, "sex-status       = %d\nskin-status      = %d\nhairstyle-status = %d\n"
                   "haircolor-status = %d\nunderwear-status = %d\n"
                   "appr1 = %d\nappr2 = 0\nappr3 = 0\nappr4 = 0\n",
                   cSex, cSkin, cHairStyle, cHairColor, cUnderwear, sAppr1);
    fprintf(pFile, "[ITEMLIST]\n"
                   "character-item = Dagger               1 0 0 0 0 0 0 0 0 300 0\n"
                   "character-item = RecallScroll         1 0 0 0 0 0 0 0 0 300 0\n"
                   "character-item = RedPotion            1 0 0 0 0 0 0 0 0 300 0\n"
                   "character-item = BluePotion           1 0 0 0 0 0 0 0 0 300 0\n"
                   "character-item = GreenPotion          1 0 0 0 0 0 0 0 0 300 0\n"
                   "character-item = KneeTrousers(M)      1 0 0 0 0 0  0 0 0 300 0\n");
    if (cSex == 1) fprintf(pFile, "character-item = Shirt(M)             1 0 0 0 0 %d 0 0 0 300 0\n", iColor);
    else           fprintf(pFile, "character-item = Chemise(W)           1 0 0 0 0 %d 0 0 0 300 0\n", iColor);
    fprintf(pFile, "[MAGIC-SKILL-MASTERY]\nmagic-mastery     = %s\nskill-mastery     = ...\nskill-SSN     = ...\n",
            "0000...0000" /* 100 */);
    fprintf(pFile, "[ITEM-EQUIP-STATUS]\nitem-equip-status = %s\n[EOF]\n",
            (cSex == 1) ? "00000110000000000000000000000000000000000000000000"
                        : "00000100000000000000000000000000000000000000000000");
    fclose(pFile);
    PutLogList("(O) New character(%s) created.");

    /* reply 0x0F1C: name 10, count, then a 65-byte slot per character (existing names + new one),
       filled like RequestCharInfoList */
    *(WORD *)(cReply + 18) = DEF_LOGRESMSGTYPE_NEWCHARACTERCREATED;
    memcpy(cReply + 20, cName, 10);
    /* ... slots via GetCharacterInfoFromFile (0x404710) ... */
    SendMsgToMLS(MSGID_RESPONSE_LOG, DEF_LOGRESMSGTYPE_NEWCHARACTERCREATED, cReply,
                 iSlots * 65 + 0x20, -1);
}
